/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    hermes.h
 *
 * Description: Header file for hermes.c
 *    
 *
 * Test configuration: See hermes.c
 *--------------------------------------------------------------------*/

#define SYNCBYTE '$'
#define HEADERLEN 4
#define CHECKLEN  2
#define MAX_MSGLEN 256
#define MAX_VARNAMELEN 30
#define MAX_NUMTMVARS 64


unsigned char G_msg[MAX_MSGLEN];
unsigned char G_outMsg[MAX_MSGLEN];

void processChar(unsigned char c);
void processMessage(char* p_msg, unsigned int msgLen);
