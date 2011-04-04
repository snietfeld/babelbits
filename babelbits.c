/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    babelbits.c
 *
 * Description: Contains message parsing code
 *    
 *
 * Test configuration: uncomment "#define UNIT_TEST", compile and run
 *--------------------------------------------------------------------*/

#define UNIT_TEST

#ifdef UNIT_TEST
#include <stdio.h>
#endif

#include "babelbits.h"

void processChar(unsigned char c)
{
  static unsigned char synced = 0;
  static unsigned char headerReceived = 0;

  static unsigned short msgLen = 0;
  static unsigned short msgType = 0;
  static unsigned int count = 0;


  if (synced)
    {
      G_msg[count] = c;
      ++count;

      if (headerReceived)
	{
	  if (count >= msgLen)
	    {
	      printf("Finished reading message.\n");
	      //Validate message
	      //Pass message to user
	      processMessage(&G_msg[HEADERLEN], msgLen-HEADERLEN-CHECKLEN);
	      
	      //Done with this message, reset variables for next one
	      synced = 0;
	      headerReceived = 0;
	      count = 0;
	      msgType = 0;
	      msgLen = 0;
	    }
	}
    

      else if (count == HEADERLEN)  //Header hasn't been read yet
	{
	  printf("Processing header...\n");
	  msgType = G_msg[1];   //Parse message type
	  msgLen  = (G_msg[2] << 8) + G_msg[3];   //Parse message length
	  printf("msgType: %d \t msgLen: %d\n", msgType, msgLen);

	  headerReceived = 1;
	}
    }

  else if (c == SYNCBYTE)   //Check for syncbyte to start message
    {
      printf("Synced.\n");
      synced = 1;
      G_msg[count] = c;
      ++count;
    }
}

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
  unsigned char c;

  processChar('$');
  processChar(0x01);
  processChar(0x00);
  processChar(0x08);
  processChar('o');
  processChar('k');
  processChar(1);
  processChar(2);

  c = getchar();

  while (c != 'q')
    {
      processChar(c);
      c = getchar();
    }

}

#endif
