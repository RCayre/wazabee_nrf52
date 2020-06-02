#include "helpers.h"
#include "serialcomm.h"

#define MAX_PACKET_SIZE 256

typedef enum {
	RX,
	TX
} RadioState;

class Radio
{
	private:
		int channel;
		bool ready;
		RadioState state;
		FCSMode fcsMode;
	public:
		static Radio *instance;
		uint8_t rxBuffer[MAX_PACKET_SIZE];
		uint8_t txBuffer[MAX_PACKET_SIZE];

		Radio();

		void reset();

		RadioState getState();

		FCSMode getFCSMode();
		void setFCSMode(FCSMode mode);

		bool disable();
		bool enable();

		bool setChannel(int channel);
		int getChannel();

		bool send(uint8_t *data,int size);
};
