/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    babelbits.c
 *
 * Description: Contains message parsing code
 *
 * Packet format:
 *
 *       | sync | msgType | packetLen | inner message | checksum |
 *
 * packetLen - Length of entire packet, including sync, type, len, msg, & checksum
 *    msgLen - Length of inner message ONLY. No sync, type, len, or checksum
 *    
 * send/receiveMsg() - wraps simple message in packet, err checks and parses msg out
 *                     returns p_msg, msgLen
 * processTMmsg() - Gets msgType, either updates protocol or processes data
 *
 * Test configuration: uncomment "#define UNIT_TEST"
 *                     gcc -o babelbits.exe babelbits.c
 *                     run
 *--------------------------------------------------------------------*/

#define UNIT_TEST

#ifdef UNIT_TEST
#include <stdio.h>
#endif

#include "babelbits.h"


void processMessage(char* p_msg, unsigned int msgLen)
{
  int i;

  printf("Message received: \"");
  for (i = 0; i < msgLen; ++i)
    printf("%c", p_msg[i]);
  printf("\"\n");
}

#ifdef UNIT_TEST



void main()
{


}

#endif
