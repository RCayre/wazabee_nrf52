#include <nrf.h>
#include "radio.h"

#define VERSION_MAJOR		0x01
#define VERSION_MINOR		0x00

#define NRF_DEVICE		0x0A
#define TI_DEVICE		0x0B

#define DEVICE_TYPE		NRF_DEVICE


int main()
{

	SerialComm serialComm(115200);
	Radio radio;
	
	radio.enable();

	while (1) {
		CommandType command = serialComm.waitCommandFromHost();
		if (command == GetFirmwareVersion) {
			uint8_t version[3];
			version[0] = VERSION_MAJOR;
			version[1] = VERSION_MINOR;
			version[2] = DEVICE_TYPE;
			serialComm.sendResponseToHost(GetFirmwareVersion,version,3);
		}
		else if (command == GetFCSMode) {
			uint8_t mode[1];
			mode[0] = (uint8_t)radio.getFCSMode();
			serialComm.sendResponseToHost(GetFCSMode,mode,1);
		}
		else if (command == SetFCSMode) {
			FCSMode fcsMode = serialComm.getFCSModeFromHost();
			uint8_t status[1];
			if (fcsMode != UNKNOWN) {
				radio.setFCSMode(fcsMode);
				status[0] = 0;
			}
			else status[0] = 1;
			serialComm.sendResponseToHost(SetFCSMode,status,1);
		}
		else if (command == GetChannel) {
			uint8_t channel[1];
			channel[0] = radio.getChannel();
			serialComm.sendResponseToHost(GetChannel,channel,1);
		}
		else if (command == SetChannel) {
			uint8_t newChannel = serialComm.getChannelFromHost();
			uint8_t status[1];
			bool success = radio.setChannel(newChannel);
			status[0] = success ? 0x00 : 0x01;
			serialComm.sendResponseToHost(SetChannel,status,1);
		}
		else if (command == SendPacket) {
			bool calcFcs = serialComm.getCalcFCSFromHost();
			uint8_t size = serialComm.getPacketSizeFromHost();

			if (calcFcs) {
				uint8_t packet[size+2];
				serialComm.getPacketFromHost(packet, size);
				uint16_t fcs = calculate_fcs_dot15d4(packet,size+2);
				packet[size] = fcs & 0xFF;
				packet[size+1] = (fcs >> 8) & 0xFF;
				radio.send(packet,size+2);
			}
			else {
				uint8_t packet[size];
				serialComm.getPacketFromHost(packet, size);
				radio.send(packet,size);
			}
			serialComm.sendResponseToHost(SendPacket,NULL,0);
		}
		else if (command == Reset) {
			radio.reset();
			serialComm.sendResponseToHost(Reset,NULL,0);
		}
		serialComm.flush();
	}
}



