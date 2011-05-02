/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    babelbits.h
 *
 * Description: Header file for babelbits.c
 *    
 *
 * Test configuration: See babelbits.c
 *--------------------------------------------------------------------*/

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


TMVar tmvars[MAX_NUMTMVARS];

void processMessage(char* p_msg, unsigned int msgLen);
