#include "helpers.h"

uint32_t bytewise_bit_swap(uint32_t inp)
{
    inp = (inp & 0xF0F0F0F0) >> 4 | (inp & 0x0F0F0F0F) << 4;
    inp = (inp & 0xCCCCCCCC) >> 2 | (inp & 0x33333333) << 2;
    return (inp & 0xAAAAAAAA) >> 1 | (inp & 0x55555555) << 1;
}

int channel_to_frequency(int channel) {
	return 5+5*(channel-11);
}

int hamming(uint8_t *demod_buffer,uint8_t *pattern) {
	int count = 0;
	for (int i=0;i<4;i++) {
		for (int j=0;j<8;j++) {
			if (((pattern[i] >> (7-j)) & 0x01) != (((demod_buffer[i] & (i==0 && j==0 ? 0x7F : 0xFF)) >> (7-j)) & 0x01)) {
				count++;
			}
		}
	}
	return count;
}

void shift_buffer(uint8_t *demod_buffer,int size) {
	for (int i=0;i<size;i++) {
		if (i != 0) {
			demod_buffer[i-1]=((demod_buffer[i] & 0x80) >> 7) | demod_buffer[i-1];
		}
		demod_buffer[i] = demod_buffer[i] << 1;
	}
}


uint16_t update_fcs_dot15d4(uint8_t byte, uint16_t fcs) {
	uint16_t q = (fcs ^ byte) & 15;
	fcs = (fcs / 16) ^ (q * 4225);
	q = (fcs ^ (byte / 16)) & 15;
	fcs = (fcs / 16) ^ (q * 4225);	
	return fcs;	
}

uint16_t calculate_fcs_dot15d4(uint8_t *data, int size) {
	uint16_t fcs = 0;
	for (int i=2; i<size-2;i++) {
		fcs = update_fcs_dot15d4(data[i],fcs);
	}
	return fcs;
}

int check_fcs_dot15d4(uint8_t *data,int size) {
	uint16_t fcs = calculate_fcs_dot15d4(data,size);
	return ((fcs & 0xFF) == data[size-2]) && (((fcs >> 8) & 0xFF) == data[size-1]);
}
