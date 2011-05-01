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

//#define CHECKSUM32(p_msg, accum, sum) (while(*p_msg != 0) accum += *p_msg; if(accui

unsigned char checksum8(unsigned char* p_msg, unsigned int len)
{
  unsigned short accum = 0;
  
  while(len--) 
    {
      accum += *p_msg++;
    }
  return accum;
}

unsigned short checksum16(unsigned char* p_msg, unsigned int len)
{
  unsigned short accum = 0;

  while(len--)
    {
      accum += *p_msg++;
    }
  return accum;
}

void processChar(unsigned char c)
{
  static unsigned char synced = 0;
  static unsigned char headerReceived = 0;

  static unsigned short msgLen = 0;
  static unsigned short msgType = 0;
  static unsigned int count = 0;
  static unsigned short rxdChecksum;
  static unsigned short calcdChecksum;


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
	      rxdChecksum = (G_msg[msgLen-2] << 8) | G_msg[msgLen-1];
	      calcdChecksum = checksum16(&(G_msg[1]), msgLen-CHECKLEN-1);

	      printf("Received checksum: 0x%x \t Calc'd Checksum: 0x%x\n",
	      	     rxdChecksum, calcdChecksum);
	      if( calcdChecksum == rxdChecksum)
	      	printf("Message validated.\n");

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

void processMessage(char* p_msg, unsigned int dataLen)
{
  int i;

  printf("Message received: \"");
  for (i = 0; i < dataLen; ++i)
    printf("%c", p_msg[i]);
  printf("\"\n");
}


int makePacket(unsigned char msgType, unsigned char* p_data, 
		unsigned int dataLen, unsigned char* p_outBuf)
{
  int i;
  unsigned int msgLen;
  unsigned short calcdChecksum;

  msgLen = dataLen + HEADERLEN + CHECKLEN;

  p_outBuf[0] = SYNCBYTE;
  p_outBuf[1] = msgType;
  p_outBuf[2] = (msgLen >> 8) & 0x00ff;
  p_outBuf[3] = msgLen & 0x00ff;

  for(i = 0; i < dataLen; ++i)
    {
      p_outBuf[HEADERLEN + i] = p_data[i];
    }

  calcdChecksum = checksum16(&(p_outBuf[1]), msgLen - CHECKLEN - 1);
  p_outBuf[HEADERLEN + dataLen]     = (calcdChecksum >> 8) & 0x00ff;
  p_outBuf[HEADERLEN + dataLen + 1] = calcdChecksum & 0x00ff;

  return msgLen;
}


#ifdef UNIT_TEST

void main()
{
  unsigned char c;
  unsigned char ohai[] = "oh hello";
  unsigned char msg[] = { '$',           // Sync
			  0x01,          // MsgType
			  0x00, 0x08,    // MsgLen
			  'o', 'k',      // Data
			  0x00, 0xe3 };  // Checksum
  int i, msgLen;

  //Read in test message
  printf("Parsing test packet...\n");
  for(i = 0; i < 8; ++i)
    processChar(msg[i]);


  //Create test packet & parse
  printf("\n\nMaking new packet & parsing...\n");
  msgLen = makePacket(1, ohai, 8, G_outMsg);

  for(i = 0; i < msgLen; ++i)
    processChar(G_outMsg[i]);


  // Begin reading in characters from the keyboard
  //----------------------------------------------
  c = getchar();

  while (c != 'q')
    {
      processChar(c);
      c = getchar();
    }


}

#endif
