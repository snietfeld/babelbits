/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    babelbits.h
 *
 * Description: Header file for babelbits.c
 *    
 *
 * Test configuration: See babelbits.c
 *--------------------------------------------------------------------*/

#define SYNCBYTE '$'
#define HEADERLEN 4
#define CHECKLEN  2
#define MAX_MSGLEN 256

unsigned char G_msg[MAX_MSGLEN];

void processChar(unsigned char c);
void processMessage(char* p_msg, unsigned int msgLen);
