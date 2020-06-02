#include "radio.h"

// Global instance of Radio
Radio* Radio::instance = NULL;

Radio::Radio() {
	this->channel = 12;
	this->ready = false;
	this->state = RX;
	this->fcsMode = ALL;
	instance = this;
}

void Radio::reset() {
	this->disable();
	this->channel = 12;
	this->ready = false;
	this->state = RX;
	this->fcsMode = ALL;
	this->enable();
}

RadioState Radio::getState() {
	return this->state;
}

FCSMode Radio::getFCSMode() {
	return this->fcsMode;
}

void Radio::setFCSMode(FCSMode mode) {
	this->fcsMode = mode;
}

bool Radio::disable() {
	bool success = false;
	if (NRF_RADIO->STATE > 0)
	{
		NVIC_ClearPendingIRQ(RADIO_IRQn);
		NVIC_DisableIRQ(RADIO_IRQn);
		NRF_RADIO->EVENTS_DISABLED = 0;
		NRF_RADIO->TASKS_DISABLE = 1;
		while (NRF_RADIO->EVENTS_DISABLED == 0);
		success = true;
	}
	return success;
}

bool Radio::enable() {
	this->disable();
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
	while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);

	NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);

	NRF_RADIO->FREQUENCY = channel_to_frequency(this->channel);

	NRF_RADIO->MODE = (RADIO_MODE_MODE_Ble_2Mbit << RADIO_MODE_MODE_Pos);

	NRF_RADIO->TXADDRESS = 0;
	NRF_RADIO->RXADDRESSES = 1;

	NRF_RADIO->BASE0 = bytewise_bit_swap(
		((uint32_t)SYMBOL_TO_CHIP_MAPPING[0][2])<<24 |
		((uint32_t)0x00) |
		((uint32_t)SYMBOL_TO_CHIP_MAPPING[0][1])<<16  |
		((uint32_t)SYMBOL_TO_CHIP_MAPPING[0][0])<<8
		);
	NRF_RADIO->PREFIX0 = bytewise_bit_swap(SYMBOL_TO_CHIP_MAPPING[0][3]);

	// LFLEN=0 bits, S0LEN=0, S1LEN=0
	NRF_RADIO->PCNF0 = 0x00000000;

	// STATLEN=255, MAXLEN=255, BALEN=3, ENDIAN=1 (big), WHITEEN=0
	NRF_RADIO->PCNF1 = 	(RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
	       			(RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)  |
	      			(3 << RADIO_PCNF1_BALEN_Pos) |
				(255 << RADIO_PCNF1_STATLEN_Pos) |
				(255 << RADIO_PCNF1_MAXLEN_Pos);

	// Disable CRC
	NRF_RADIO->CRCCNF = 0x0;
	NRF_RADIO->CRCINIT = 0xFFFF;
	NRF_RADIO->CRCPOLY = 0x11021;

	NRF_RADIO->PACKETPTR = (uint32_t)(this->rxBuffer);

	// Configure interrupts
	NRF_RADIO->INTENSET = 0x00000008;
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);

	NRF_RADIO->SHORTS = 0;

	// Enable receiver
	NRF_RADIO->EVENTS_READY = 0;
	NRF_RADIO->TASKS_RXEN = 1;
	while (NRF_RADIO->EVENTS_READY == 0);

	NRF_RADIO->EVENTS_END = 0;
	NRF_RADIO->TASKS_START = 1;
	this->state = RX;
	return true;
}


bool Radio::setChannel(int channel) {
	bool success = false;
	if (channel >= 11 and channel <= 26) {
		this->channel = channel;
		this->enable();
		success = true;
	}
	return success;
}

int Radio::getChannel() {
	return this->channel;
}

bool Radio::send(uint8_t *data,int size) {

	int total_size = 4  + size;
	
	uint8_t buffer[total_size];
	
	for (int i=0;i<4;i++) buffer[i] = 0;
	memcpy(buffer+4,data,size);

	
	uint8_t output_buffer[MAX_PACKET_SIZE];
	for (int i=0;i<MAX_PACKET_SIZE;i++) output_buffer[i] = 0;
	

	
	for (int i=0;i<total_size;i++) {
		uint8_t msb = buffer[i] >> 4;
		uint8_t lsb = buffer[i] & 0x0F;

		output_buffer[1+i*8+0] = (SYMBOL_TO_CHIP_MAPPING[lsb][0]);
		output_buffer[1+i*8+1] = (SYMBOL_TO_CHIP_MAPPING[lsb][1]);
		output_buffer[1+i*8+2] = (SYMBOL_TO_CHIP_MAPPING[lsb][2]);
		output_buffer[1+i*8+3] = (SYMBOL_TO_CHIP_MAPPING[lsb][3]);

		output_buffer[1+i*8+4] = (SYMBOL_TO_CHIP_MAPPING[msb][0]);
		output_buffer[1+i*8+5] = (SYMBOL_TO_CHIP_MAPPING[msb][1]);
		output_buffer[1+i*8+6] = (SYMBOL_TO_CHIP_MAPPING[msb][2]);
		output_buffer[1+i*8+7] = (SYMBOL_TO_CHIP_MAPPING[msb][3]);


	}
	output_buffer[0] = 0xFF;

	

	// Disable interrupt
	NVIC_DisableIRQ(RADIO_IRQn);
	/* Configure shorts. */
	NRF_RADIO->SHORTS = 0;

	NRF_RADIO->EVENTS_DISABLED = 0;
	NRF_RADIO->TASKS_DISABLE = 1;
	while(NRF_RADIO->EVENTS_DISABLED == 0);

	NRF_RADIO->FREQUENCY = channel_to_frequency(this->channel);
	NRF_RADIO->PACKETPTR = (uint32_t)output_buffer;

	// Turn on the transmitter, and wait for it to signal that it's ready to use.
	this->state = TX;
	NRF_RADIO->EVENTS_READY = 0;
	NRF_RADIO->TASKS_TXEN = 1;
	while (NRF_RADIO->EVENTS_READY == 0);

	// Start transmission and wait for end of packet.
	NRF_RADIO->TASKS_START = 1;
	NRF_RADIO->EVENTS_END = 0;
	while (NRF_RADIO->EVENTS_END == 0);

	NRF_RADIO->PACKETPTR = (uint32_t)this->rxBuffer;

        // Turn off the transmitter.
	NRF_RADIO->EVENTS_DISABLED = 0;
	NRF_RADIO->TASKS_DISABLE = 1;
	while (NRF_RADIO->EVENTS_DISABLED == 0);

	this->state = RX;
	NRF_RADIO->EVENTS_READY = 0;
	NRF_RADIO->TASKS_RXEN = 1;
	while (NRF_RADIO->EVENTS_READY == 0);
	NRF_RADIO->EVENTS_END = 0;
	NRF_RADIO->TASKS_START = 1;

	/* Cleaning and resuming IRQ handler. */
	NRF_RADIO->SHORTS = 0;
	NRF_RADIO->INTENSET = 0x00000008;

	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);	
	return false;
}

extern "C" void RADIO_IRQHandler(void)
{

	if (NRF_RADIO->EVENTS_READY) {
		NRF_RADIO->EVENTS_READY = 0;
		NRF_RADIO->TASKS_START = 1;
	}

	if (NRF_RADIO->EVENTS_END) {
		NRF_RADIO->EVENTS_END = 0;
		
		// buffer of Zigbee symbols
		uint8_t output_buffer[50];
		// buffer of FSK symbols
		uint8_t buffer[MAX_PACKET_SIZE];
		// index of the current Zigbee symbol
		int index = 0;
		// indicator of current 4 bits position (1 = 4 MSB, 0 = 4 LSB)
		int part = 1;
		// indicator of start frame delimiter
		int sfd = 0;
		// Hamming distance
		int hamming_dist = 0;
		// Thresold Hamming distance
		int hamming_thresold = 5;

		// Reset output buffer
		for (int i=0;i<50;i++) output_buffer[i] = 0;

		// Copy the received packet to buffer
		memcpy(buffer,Radio::instance->rxBuffer,MAX_PACKET_SIZE);


		// Align the buffer with the SFD
		output_buffer[0] |= (0x0F & 0x07);
		hamming_dist = 32;
		while (hamming_dist > hamming_thresold) {
			hamming_dist = hamming(buffer,SYMBOL_TO_CHIP_MAPPING[0]);
			if (hamming_dist > hamming_thresold) {
				shift_buffer(buffer,255);
			}
		}

		hamming_dist = 0;
		while (hamming_dist <= hamming_thresold) {
			int symbol = -1;
			// Compute the hamming distance for every zigbee symbol
			for (int i=0;i<16;i++) {
				hamming_dist = hamming(buffer,SYMBOL_TO_CHIP_MAPPING[i]);
				if (hamming_dist <= hamming_thresold) {
					symbol = i;
					break;
				}
			}

			// If a zigbee symbol has been found ...
			if (symbol != -1) {
				// If it matches the SFD next symbol, start the frame decoding
				if (sfd == 0 && symbol == 10) {
					sfd = 1;
				}
				
				// If we are in the frame decoding state ...
				if (sfd == 1) {
					// Fill the output buffer with the selected symbol
					output_buffer[index] |= (symbol & 0x0F) << 4*part;

					// Select the next 4 bits free space in the output buffer
					part = part == 1 ? 0 : 1;
					if (part == 0) index++;
				}
				// Shift the buffer (31 bits shift)
				for (int i=0;i<32;i++) shift_buffer(buffer,MAX_PACKET_SIZE);
			}
		}

		// Transmits the frame to Host
		int valid_fcs = check_fcs_dot15d4(output_buffer, index);
		if ((Radio::instance->getFCSMode() == ALL) || (Radio::instance->getFCSMode() == VALID_FCS_ONLY && valid_fcs)) { 
			SerialComm::instance->sendPacketToHost(output_buffer, index, valid_fcs);
		}
		
		NRF_RADIO->TASKS_START = 1;
	}
}
