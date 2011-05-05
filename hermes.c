/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    hermes.c
 *
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
 *
 *--------------------------------------------------------------------
 *                  Copyright 2011, Scott Nietfeld
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *--------------------------------------------------------------------*/

#define UNIT_TEST

#ifdef UNIT_TEST
#include <stdio.h>
#include <stdint.h>
#endif

#include "hermes.h"


//----------------Private Functions & Variables-----------------//

uint8_t* p_inBuf;
uint8_t* p_outBuf;

int inBufLen, outBufLen;

// Function pointer, user points this ptr to the function to be
// called whenever a message is finished being read in.
void (*p_msgHandler)(uint8_t* p_msg, uint16_t msgLen);


uint8_t checksum8(uint8_t* p_msg, uint16_t len);
uint16_t checksum16(uint8_t* p_msg, uint16_t len);

//--------------------------------------------------------------//



//----------------------Public Functions------------------------//
void hermes_init(uint8_t* p_newInBuf,  uint16_t newInBufLen,
		 uint8_t* p_newOutBuf, uint16_t newOutBufLen,
		 void (*p_newMsgHandler)(uint8_t* p_msg, uint16_t msgLen))
{
  p_inBuf = p_newInBuf;
  inBufLen = newInBufLen;

  p_outBuf = p_newOutBuf;
  outBufLen = newOutBufLen;

  p_msgHandler = p_newMsgHandler;
}

void hermes_processChar(uint8_t c)
{
  static uint8_t synced = 0;
  static uint8_t headerReceived = 0;

  static uint16_t packetLen = 0;
  static uint16_t formatID = 0;
  static uint16_t checkType;
  static uint16_t checkLen;
  static uint16_t count = 0;
  static uint16_t rxdChecksum;
  static uint16_t calcdChecksum;


  if( count >= inBufLen )
    {
      //Message is too long, just scrap it & start over
      synced         = 0;
      headerReceived = 0;
      count          = 0;
      formatID       = 0;
      packetLen      = 0;
      return;
    }


  if (synced)
    {
      p_inBuf[count] = c;
      ++count;

      if (headerReceived)
	{
	  if (count >= packetLen)
	    {
	      //printf("Finished reading message.\n");

	      if( checkType == NOCHECK ) //If no error checking, just forward message
		{
		  p_msgHandler(&p_inBuf[HEADERLEN], packetLen-HEADERLEN-checkLen);
		}

	      else  //Need to do error checking
		{
		  //Validate message
		  //printf("\nCheck type: 0x%x\n", formatID >> 4 & 0x00ff);
		  switch( checkType )  //First nibble is check type
		    {
		    case CHECKSUM8:
		      {
			rxdChecksum = p_inBuf[packetLen-1];
			calcdChecksum = checksum8(&(p_inBuf[1]), packetLen-1-1);
			break;
		      }
		    case CHECKSUM16:
		      {
			//printf("\nRunning CHECKSUM16...\n");
			rxdChecksum = (p_inBuf[packetLen-2] << 8) | p_inBuf[packetLen-1];
			calcdChecksum = checksum16(&(p_inBuf[1]), packetLen-2-1);
			break;
		      }
		      
		    default:     //Invalid check type, scrap packet & start over
		      synced         = 0;
		      headerReceived = 0;
		      count          = 0;
		      formatID       = 0;
		      packetLen      = 0;
		    }
		  
		  //printf("Received checksum: 0x%x \t Calc'd Checksum: 0x%x\n",
		  //rxdChecksum, calcdChecksum);
		  
		  if( calcdChecksum == rxdChecksum)
		    {
		      //printf("Message validated.\n");
		      
		      //Pass message to user
		      p_msgHandler(&p_inBuf[HEADERLEN], packetLen-HEADERLEN-checkLen);
		    }
		}
	      
	      //Done with this message, reset variables for next one
	      synced         = 0;
	      headerReceived = 0;
	      count          = 0;
	      formatID       = 0;
	      packetLen      = 0;
	    }
	}
    

      else if (count == HEADERLEN)  //Time to read the header
	{
	  //printf("Processing header...\n");
	  formatID = p_inBuf[1];   //Parse message type
	  packetLen  = (p_inBuf[2] << 8) + p_inBuf[3];   //Parse message length

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
      p_inBuf[count] = c;
      ++count;
    }
}



int hermes_makePacket(uint8_t checkType, uint8_t* p_data, 
		      uint16_t msgLen,   uint8_t* p_outBuf)
{
  uint16_t i;
  uint16_t packetLen;
  uint16_t formatID;
  uint16_t calcdChecksum;

  //Calculate packet length, switch on check type since it affects this
  formatID = checkType << 4;

  switch( checkType )
    {      
    case CHECKSUM16:
    case CRC16:
      packetLen = msgLen + HEADERLEN + 2;
      break;

    case CHECKSUM8:
      packetLen = msgLen + HEADERLEN + 1;
      break;

    case NOCHECK:
      packetLen = msgLen + HEADERLEN;
      break;

    default:     //Unrecognized checkType, abort
      return 0;
    }


  if( packetLen > outBufLen || packetLen > MAX_MSGLEN ) return 0;

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
//---------------------------------------------------------------//



//---------------------Error Checking Fcns-----------------------//

uint8_t checksum8(uint8_t* p_msg, uint16_t len)
{
  uint16_t accum = 0;
  
  while(len--) 
    {
      accum += *p_msg++;
    }
  return accum;
}

uint16_t checksum16(uint8_t* p_msg, uint16_t len)
{
  uint16_t accum = 0;

  while(len--)
    {
      accum += *p_msg++;
    }
  return accum;
}
//---------------------------------------------------------------//



//-----------------------UNIT TEST CODE--------------------------//
#ifdef UNIT_TEST

int msgReadSuccess  = 0;
int msgWriteSuccess = 0;

void processMessage(uint8_t* p_msg, uint16_t msgLen)
{
  uint16_t i;

  printf("Message received: \"");
  for (i = 0; i < msgLen; ++i)
    printf("%c", p_msg[i]);
  printf("\"\n");

  if     ( msgLen == 2 && p_msg[0] == 'o' && p_msg[1] == 'k' ) msgReadSuccess = 1;
  else if( msgLen == 2 && p_msg[0] == 'k' && p_msg[1] == 'o' ) msgWriteSuccess = 1;
}


int hermes_unit(void)
{
  uint8_t inPacketBuf[256];    // Buffers for hermes to store incoming
  uint8_t outPacketBuf[256];   // and outgoing packets

  uint16_t i, packetLen;


  //Test message
  uint8_t msg[] = { '$',              // Sync
			  CHECKSUM16 << 4,  // CheckType
			  0x00, 0x08,       // MsgLen
			  'o', 'k',         // Data
			  0x01, 0x02 };     // Checksum


  //Initialize hermes buffers and message handler
  hermes_init( inPacketBuf, 256, outPacketBuf, 256, &processMessage);


  //Read in test message
  for(i = 0; i < 8; ++i)
    hermes_processChar(msg[i]);
  if( msgReadSuccess == 0 ) return -1;


  //Create test packet & parse
  packetLen = hermes_makePacket(CHECKSUM8, "ko", 2, outPacketBuf);

  for(i = 0; i < packetLen; ++i)
    hermes_processChar(outPacketBuf[i]);
  if( msgWriteSuccess == 0 ) return -2;


  return 1; //Successful test
}

void main(void)
{
  uint16_t result;
  uint8_t c;

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
//------------------------End Unit Test Code--------------------//
