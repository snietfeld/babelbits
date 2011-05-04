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


void hermes_init(unsigned char* p_newInBuf,  int newInBufLen,
		 unsigned char* p_newOutBuf, int newOutBufLen,
		 void (*p_msgHandler)(char* p_msg, unsigned int msgLen));
		 
void hermes_processChar(unsigned char c);

int hermes_makePacket(unsigned char  checkType, 
		      unsigned char* p_data, 
		      unsigned int   msgLen, 
		      unsigned char* p_outBuf);
