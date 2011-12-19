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

#include <stdint.h>  //Needed for explicit-size datatypes (uint8_t, etc)
#include <string.h>  //Needed for calculating string sizes

#include "babelbits.h"

void bb_init(bbInstance* p_bb, uint8_t* p_buf, uint16_t buflen)
{
  uint8_t i = 0;
  bbField* p_field;

  p_bb->nfields = 0; //Reset number of registered fields to zero

  p_bb->p_out_msgbuf = p_buf;
  p_bb->out_msgbuf_len = buflen;
  p_bb->out_bytes_waiting = 0;

  //Clear all fields & subscriptions
  for(i = 0; i < BB_MAX_NFIELDS; i++)
    {
      p_field = &(p_bb->fields[i]);  //Clear field
      p_field->p_var = 0;
      p_field->type = UNKNOWN;
      p_field->name[0] = 0;  //Null the string

      p_bb->subscriptions[i] = 0;    //Clear subscription
    }
}


void bb_processMessage(bbInstance* p_bb, uint8_t* p_msg, uint16_t msgLen)
{
  uint8_t i;
  uint8_t msgType;
  uint8_t* p_innerMsg;
  uint16_t innerMsgLen;

  printf("Message received: \"");
  for (i = 0; i < msgLen; ++i)
    printf("%c", p_msg[i]);
  printf("\"\n");
 
  //Arrange inner message and inner message length
  msgType = p_msg[0];
  p_innerMsg = &(p_msg[1]);
  innerMsgLen = msgLen - 1;

  switch(msgType)
    {
    case BB_HANDSHAKE_INIT:
      p_bb->connection_state = BB_CONNECTED;
      break;

    case BB_ACK:
      if(p_bb->connection_state == BB_HANDSHAKE_SENT)
	p_bb->connection_state = BB_CONNECTED;
      break;

    case BB_REQUEST_AVAILABLE_FIELDS:
      break;

    case BB_PUBLISH:
      printf("Publish message received.\n");
      break;

    case BB_SUBSCRIBE:
      bb_process_subscribe(p_bb, (bbMsg_Subscribe*)p_innerMsg);
      break;

    case BB_UNSUBSCRIBE:
      bb_process_unsubscribe(p_bb, (bbMsg_Unsubscribe*)p_innerMsg);
      break;

    case BB_TMPACKET:
      bb_process_tmpacket(p_bb, (bbMsg_TM*)p_innerMsg);
      break;

    case BB_TXTPACKET:
      p_bb->txt_msg_handler(p_innerMsg);
      break;

    default:
      printf("Error: unknown packet type\n");
    }

}

void bb_process_tmpacket(bbInstance* p_bb, bbMsg_TM* p_msg)
{
}

void bb_process_subscribe(bbInstance* p_bb, bbMsg_Subscribe* p_msg)
{
  p_bb->subscriptions[p_msg->field_id] = 1;
}

void bb_process_unsubscribe(bbInstance* p_bb, bbMsg_Unsubscribe* p_msg)
{
  p_bb->subscriptions[p_msg->field_id] = 0;
}


void bb_register_field(bbInstance* p_bb, void* p_var, enum Datatype type, char* p_name)
{
  uint8_t field_id;
  bbField* p_field;

  p_field = &(p_bb->fields[p_bb->nfields]);

  p_field->p_var = p_var;
  p_field->type = type;
  strcpy(p_field->name, p_name);

  p_bb->nfields += 1;
}

void bb_send_handshake(bbInstance* p_bb)
{
  uint8_t* p_buf;

  p_buf = p_bb->p_out_msgbuf;

  *p_buf = BB_HANDSHAKE_INIT;

  p_bb->out_bytes_waiting = 1;
  p_bb->connection_state = BB_HANDSHAKE_SENT;
}

int8_t bb_publish_field(bbInstance* p_bb, bbField* p_field)
{
  uint8_t i;
  uint8_t* p_buf;
  uint8_t* p_buf_end;
  uint8_t* p_cur;

  uint16_t nbytes;

  //If no outgoing buffer has been set, raise error
  if(p_bb->p_out_msgbuf == 0) return -1;

  if(p_bb->out_bytes_waiting > 0)
    printf("bb_publish_field(): Error, out_bytes_waiting = %d", 
	   p_bb->out_bytes_waiting);

  p_buf = p_bb->p_out_msgbuf;
  p_cur = p_buf;

  *p_cur = BB_PUBLISH;             //Add BB message type
  p_cur += 1;
  
  //Add field struct to outgoing buff
  memcpy(p_cur, p_field, sizeof(bbField));

  nbytes = 1 + sizeof(*p_field);
  printf("\nnbytes: %d\n", nbytes);

  p_bb->out_bytes_waiting = nbytes;
  return nbytes;
}



  /* for(i = 0; i < p_bb->nfields; i++) */
  /*   { */
  /*     p_field = &(p_bb->fields[i]); */

  /*     //Calculate number of bytes we'll be adding so we know if we're about to overrun */
  /*     nbytes = sizeof(bbField) + strlen(p_field->p_name); */
  /*     if(p_cur + nbytes >= p_buf_end) return -2;  //About to overrun--raise error */

  /*     //Good to go, send field to buffer */
  /*   } */





#ifdef UNIT_TEST

void print_fields(bbInstance* p_bb)
{
  int i;
  bbField* p_field;

  printf("\nRegistered fields:\n");
  for(i = 0; i < p_bb->nfields; i++)
    {
      p_field = &(p_bb->fields[i]);
	printf("p_var: %p \t type: %d \t name: %s\n", 
	      p_field->p_var, p_field->type, p_field->name);
    }
}


bbInstance bbits;
uint8_t bbuf[64];

void main()
{
  uint32_t counter = 0;
  float pi = 3.141592;
  

  printf("Initializing babelbits instance...\n");
  bb_init(&bbits, bbuf, 64);
  printf("Babelbits instance initialized.\n");

  printf("Registering fields...\n");
  bb_register_field(&bbits, &counter, INT32, "counter");
  bb_register_field(&bbits, &pi, FLOAT32, "pi");
  printf("Fields registered.\n");

  print_fields(&bbits);

  bb_publish_field(&bbits, &(bbits.fields[0]));
  printf("Done.\n");

  if(bbits.out_bytes_waiting > 0)
    printf("Bytes waiting to be sent: %d\n", bbits.out_bytes_waiting);

  bb_processMessage(&bbits, bbits.p_out_msgbuf, bbits.out_bytes_waiting);
}

#endif
