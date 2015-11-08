#include"server_common.h"
#include"server_func.h"

static int numCommand;

int executeCommand(int command, int pos)
{
		int			endFlag = 1;

		assert(0<=pos && pos<MAX_CLIENTS);

		int i;
#ifndef NDEBUG
		printf("#####\n");
		printf("ExecuteCommand()\n");
#endif

		switch(command){
				case END_COMMAND:
						endFlag = 0;
						sendData(ALL_CLIENTS, END_COMMAND, sizeof(int));
						break;
				default:
						fprintf(stderr,"0x%02x is not command!\n", command);
						break;
		}
		return endFlag;
}
