#include "../common.h"
#include "client_common.h"
#include "client_func.h"

int executeCommand(int command)
{
		int	endFlag = 1;
#ifndef NDEBUG
		/*
		printf("#####\n");
		printf("ExecuteCommand()\n");
		*/
#endif
		switch(command){
				case END_COMMAND:
						endFlag = 0;
						break;
				default:	
						break;
		}
		return endFlag;
}

void sendEndCommand(void)
{
		unsigned char	data[MAX_DATA];
		int	dataSize;

#ifndef NDEBUG
		printf("#####\n");
		printf("sendEndCommand()\n");
#endif

		dataSize = 0;

		sendData(data, dataSize);
}
