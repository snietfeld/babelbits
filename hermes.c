/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    hermes.c
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
 *
 * Test configuration: uncomment "#define UNIT_TEST"
 *                     gcc -o hermes.exe hermes.c
 *                     run
 *--------------------------------------------------------------------*/

#define UNIT_TEST

#ifdef UNIT_TEST
#include <stdio.h>
#endif

#include "hermes.h"


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

  static unsigned short packetLen = 0;
  static unsigned short formatID = 0;
  static unsigned short checkType;
  static unsigned short checkLen;
  static unsigned int   count = 0;
  static unsigned short rxdChecksum;
  static unsigned short calcdChecksum;


  if (synced)
    {
      G_msg[count] = c;
      ++count;

      if (headerReceived)
	{
	  if (count >= packetLen)
	    {
	      //printf("Finished reading message.\n");

	      if( checkType == NOCHECK ) //If no error checking, just forward message
		{
		  p_msgHandler(&G_msg[HEADERLEN], packetLen-HEADERLEN-checkLen);
		}

	      else  //Need to do error checking
		{
		  //Validate message
		  //printf("\nCheck type: 0x%x\n", formatID >> 4 & 0x00ff);
		  switch( checkType )  //First nibble is check type
		    {
		    case CHECKSUM8:
		      {
			rxdChecksum = G_msg[packetLen-1];
			calcdChecksum = checksum8(&(G_msg[1]), packetLen-1-1);
			break;
		      }
		    case CHECKSUM16:
		      {
			//printf("\nRunning CHECKSUM16...\n");
			rxdChecksum = (G_msg[packetLen-2] << 8) | G_msg[packetLen-1];
			calcdChecksum = checksum16(&(G_msg[1]), packetLen-2-1);
			break;
		      }
		      
		    default:     //Invalid check type, scrap packet & start over
		      synced = 0;
		      headerReceived = 0;
		      count = 0;
		      formatID = 0;
		      packetLen = 0;
		    }
		  
		  //printf("Received checksum: 0x%x \t Calc'd Checksum: 0x%x\n",
		  //rxdChecksum, calcdChecksum);
		  
		  if( calcdChecksum == rxdChecksum)
		    {
		      //printf("Message validated.\n");
		      
		      //Pass message to user
		      p_msgHandler(&G_msg[HEADERLEN], packetLen-HEADERLEN-checkLen);
		    }
		}
	      
	      //Done with this message, reset variables for next one
	      synced = 0;
	      headerReceived = 0;
	      count = 0;
	      formatID = 0;
	      packetLen = 0;
	    }
	}
    

      else if (count == HEADERLEN)  //Time to read the header
	{
	  //printf("Processing header...\n");
	  formatID = G_msg[1];   //Parse message type
	  packetLen  = (G_msg[2] << 8) + G_msg[3];   //Parse message length

	  //Calculate checksum length based on checkType in formatId
	  checkType = formatID >> 4 & 0x00ff;
	  //printf("\ncheckType: 0x%x\n", checkType);

	  switch(checkType)  //Extract check type
	    {
	    case CHECKSUM16:   
	    case CRC16:
	      checkLen = 2;
	      break;

	    case CHECKSUM8:
	    case CRC8:
	      checkLen = 1;
	      break;

	    case NOCHECK:
	      checkLen = 0;
	      break;

	    //Invalid check type--scrap packet and start over
	    default: 
	      //printf("\nInvalid check type: 0x%x\n", checkType);
	      synced = 0;
	      headerReceived = 0;
	      count = 0;
	      formatID = 0;
	      packetLen = 0;
	    }
	  //printf("formatID: %d \t packetLen: %d \t checkLen: %d\n", 
	  //	 formatID, packetLen, checkLen);

	  headerReceived = 1;
	}
    }

  else if (c == SYNCBYTE)   //Check for syncbyte to start message
    {
      //printf("Synced.\n");
      synced = 1;
      G_msg[count] = c;
      ++count;
    }
}



int makePacket(unsigned char checkType, unsigned char* p_data, 
		unsigned int msgLen, unsigned char* p_outBuf)
{
  int i;
  unsigned int packetLen;
  unsigned int formatID;
  unsigned short calcdChecksum;

  //Calculate packet length, switch on check type since it affects this
  formatID = checkType << 4;

  switch( checkType )
    {      
    case CHECKSUM16:
    case CRC16:
      packetLen = msgLen + HEADERLEN + 2;
      break;

    case CHECKSUM8:
    case CRC8:
      packetLen = msgLen + HEADERLEN + 1;
      break;

    case NOCHECK:
      packetLen = msgLen + HEADERLEN;
      break;

    default:     //Unrecognized checkType, abort
      return 0;
    }


  if( packetLen > MAX_MSGLEN ) return 0;

  p_outBuf[0] = SYNCBYTE;
  p_outBuf[1] = formatID;
  p_outBuf[2] = (packetLen >> 8) & 0x00ff;
  p_outBuf[3] = packetLen & 0x00ff;

  //Add message to packet
  for(i = 0; i < msgLen; ++i)
    {
      p_outBuf[HEADERLEN + i] = p_data[i];
    }

  //Add checksum (if needed)
  switch( checkType )  //First nibble is check type
    {
    case CHECKSUM16:
      {
	calcdChecksum = checksum16(&(p_outBuf[1]), packetLen - 2 - 1);
	p_outBuf[HEADERLEN + msgLen]     = (calcdChecksum >> 8) & 0x00ff;
	p_outBuf[HEADERLEN + msgLen + 1] = calcdChecksum & 0x00ff;
	break;
      }

    case CHECKSUM8:
      {
	calcdChecksum = checksum8(&(p_outBuf[1]), packetLen - 1 - 1);
	p_outBuf[HEADERLEN + msgLen] = calcdChecksum;
	break;
      }

    case NOCHECK:
      break;

    default:     //Unrecognized check type, abort
      return 0;
    }

  return packetLen;
}


#ifdef UNIT_TEST
int msgReadSuccess  = 0;
int msgWriteSuccess = 0;

void processMessage(char* p_msg, unsigned int msgLen)
{
  int i;

  printf("Message received: \"");
  for (i = 0; i < msgLen; ++i)
    printf("%c", p_msg[i]);
  printf("\"\n");

  if( msgLen == 2 && p_msg[0] == 'o' && p_msg[1] == 'k') msgReadSuccess = 1;
  else if( msgLen == 2 && p_msg[0] == 'k' && p_msg[1] == 'o' ) msgWriteSuccess = 1;
}

int hermes_unit(void)
{
  int i, packetLen;

  //Test message
  unsigned char msg[] = { '$',              // Sync
			  CHECKSUM16 << 4,  // CheckType
			  0x00, 0x08,       // MsgLen
			  'o', 'k',         // Data
			  0x01, 0x02 };     // Checksum


  //Assign message handler fcn to pointer
  p_msgHandler = &processMessage;


  //Read in test message
  for(i = 0; i < 8; ++i)
    processChar(msg[i]);
  if( msgReadSuccess == 0 ) return -1;


  //Create test packet & parse
  packetLen = makePacket(CHECKSUM8, "ko", 2, G_outMsg);

  for(i = 0; i < packetLen; ++i)
    processChar(G_outMsg[i]);
  if( msgWriteSuccess == 0 ) return -2;

  return 1; //Successful test
}

void main(void)
{
  int result;
  unsigned char c;

  printf("\nBeginning unit test for hermes.c message passing...\n");

  result = hermes_unit();

  if(result > 0)
    printf("\nUnit test for hermes.c: --PASSED--\n");
  else
    printf("\nUnit test for hermes.c: --FAILED-- with error code %d\n", result);
  

  // Begin reading in characters from the keyboard
  //----------------------------------------------
  /* c = getchar(); */

  /* while (c != 'q') */
  /*   { */
  /*     processChar(c); */
  /*     c = getchar(); */
  /*   } */


}

#endif
