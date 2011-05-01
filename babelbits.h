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
#define MAX_VARNAMELEN 30
#define MAX_NUMTMVARS 64

enum Datatype
  {
    SHORT,
    INT,
    FLOAT,
    DOUBLE,
    CHAR
  };

enum MsgType
  {
    HANDSHAKE_INIT,
    ACK,
    TMPACKET
  };

typedef struct
{
  void* p_var;
  enum Datatype type;
  char varname[MAX_VARNAMELEN];
} TMVar;


unsigned char G_msg[MAX_MSGLEN];
unsigned char G_outMsg[MAX_MSGLEN];
TMVar tmvars[MAX_NUMTMVARS];

void processChar(unsigned char c);
void processMessage(char* p_msg, unsigned int msgLen);
