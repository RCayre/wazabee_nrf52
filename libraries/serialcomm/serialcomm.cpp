#include "serialcomm.h"

// Global instance of SerialComm
SerialComm* SerialComm::instance = NULL;

SerialComm::SerialComm(int baudrate) {
	this->baudrate = baudrate;
	Serial.begin(baudrate);
	instance = this;
}

void SerialComm::flush() {
	Serial.flush();
}

void SerialComm::sendPacketToHost(uint8_t *frame, uint8_t size,int valid_fcs) {
	uint8_t packet[2+1+1+size+1];
	packet[0] = PREAMBLE_START;
	packet[1] = PREAMBLE_END;
	packet[2] = FRAME_TYPE_PACKET;
	packet[3] = size;
	memcpy(&packet[4],frame,size);
	packet[4+size] = valid_fcs ? 0x01 : 0x00;
	Serial.write(packet,size+5);
	Serial.flush();
}
void SerialComm::sendResponseToHost(CommandType type, uint8_t *parameter, uint8_t parameter_length) {
	uint8_t response[2+1+1+parameter_length];
	response[0] = PREAMBLE_START;
	response[1] = PREAMBLE_END;
	response[2] = FRAME_TYPE_RESPONSE;
	response[3] = (uint8_t)type;
	if (parameter_length > 0) {
		memcpy(&response[4],parameter,parameter_length);
	}
	Serial.write(response,parameter_length+4);
	Serial.flush();
}

CommandType SerialComm::waitCommandFromHost() {
	CommandType command = None;
	while (command == None) {

		if (Serial.available() > 3) {
			uint8_t next = Serial.read();
			if (next == PREAMBLE_START) {
				if (Serial.read() == PREAMBLE_END) {
					uint8_t type = Serial.read();
					if (type == 0) {
						uint8_t commandField = Serial.read();
						if (commandField == 0x00) command = GetFirmwareVersion;
						else if (commandField == 0x01) command = GetFCSMode;
						else if (commandField == 0x02) command = SetFCSMode;
						else if (commandField == 0x03) command = GetChannel;
						else if (commandField == 0x04) command = SetChannel;
						else if (commandField == 0x05) command = SendPacket;
						else if (commandField == 0x06) command = Reset;
						else command = Unknown;
					}
				}
				
			}
		}
	}
	return command;
}

FCSMode SerialComm::getFCSModeFromHost() {
	while (Serial.available() == 0);
	uint8_t mode = Serial.read();
	FCSMode fcsMode;
	if (mode == 0x00) fcsMode = ALL;
	else if (mode == 0x01) fcsMode = VALID_FCS_ONLY;
	else fcsMode = UNKNOWN;
	return fcsMode;
}

uint8_t SerialComm::getChannelFromHost() {
	while (Serial.available() == 0);
	uint8_t channel = Serial.read();
	return channel;
}
bool SerialComm::getCalcFCSFromHost() {
	while (Serial.available() == 0);
	uint8_t calc_fcs = Serial.read();
	return (calc_fcs == 0x01);
}
uint8_t SerialComm::getPacketSizeFromHost() {
	while (Serial.available() == 0);
	uint8_t size = Serial.read();
	return size;
}

void SerialComm::getPacketFromHost(uint8_t *packet, uint8_t size) {
	while (Serial.available() < size);
	for (int i=0;i<size;i++) {
		packet[i] = Serial.read();
	}
}
