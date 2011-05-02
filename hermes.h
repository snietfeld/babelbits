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


// Function pointer-user points this pointer to the function to be
// called whenever a message is finished being read in.
void (*p_processMsg)(char* p_msg, unsigned int msgLen);


void processChar(unsigned char c);
