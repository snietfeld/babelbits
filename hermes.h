/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    hermes.h
 *
 * Description: Header file for hermes.c
 *    
 *
 * Test configuration: See hermes.c
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
#include <stdint.h>

#define SYNCBYTE '$'
#define HEADERLEN 4
#define CHECKLEN  2
#define MAX_MSGLEN 0xffff


enum
  {
    NOCHECK,
    CHECKSUM8,
    CHECKSUM16,
    CRC16,
    CRC32
  };


void hermes_init(uint8_t* p_newInBuf,  uint16_t newInBufLen,
		 uint8_t* p_newOutBuf, uint16_t newOutBufLen,
		 void (*p_msgHandler)(uint8_t* p_msg, uint16_t msgLen));
		 
void hermes_processChar(uint8_t c);

int hermes_makePacket(uint8_t  checkType, 
		      uint8_t* p_data, 
		      uint16_t msgLen, 
		      uint8_t* p_outBuf);
