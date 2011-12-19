/*---------------------------------------------------------------------*
 * Project Name: babelbits
 * File name:    babelbits.h
 *
 * Description: Header file for babelbits.c
 *    
 *
 * Test configuration: See babelbits.c
 *--------------------------------------------------------------------*/

#include <stdarg.h>   //Needed for variadic function def in bbCommand

#define BB_MAX_VARNAMELEN 32
#define BB_MAX_NFIELDS 64
#define BB_MAX_NARGS 16
#define BB_MIN_BUFLEN 64   //Minimum BB out buffer length needed


enum Datatype
  {
    UNKNOWN,
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT32,
    FLOAT64,
  };

enum bbMsgType
  {
    BB_HANDSHAKE_INIT,                //Initiates communication
    BB_ACK,                           //Confirms communication
    BB_REQUEST_AVAILABLE_FIELDS,      //Client asks what fields we have
    BB_PUBLISH,                       //Host sends list of fields
    BB_SUBSCRIBE,                     //Client subscribes to a field
    BB_UNSUBSCRIBE,                   //Client unsubscribes to a field
    BB_TMPACKET,                      //Host sends telemetry update
    BB_TXTPACKET,
  };

//Message structs
typedef struct
{
  uint8_t field_id;
} bbMsg_Subscribe;

typedef struct
{
  uint8_t field_id;
} bbMsg_Unsubscribe;

//---------------

enum bbConnectionState
  {
    BB_NO_CONNECTION,
    BB_HANDSHAKE_SENT,
    BB_CONNECTED
  };

enum Unit
  {
    NONE,
    SECONDS, MINUTES, HOURS, DAYS, YEARS,
    MILLIMETERS, CENTIMETERS, METERS, KILOMETERS, 
    METERS_PER_SEC, KILOMETERS_PER_HOUR,
    METERS_PER_SEC_SQUARED,
    DEGREES_C,
    GRAMS, KILOGRAMS,
    NEWTONS,
    MILLIVOLTS, VOLTS,
    AMPS
  };

//Basic TM field - generic housekeeping data
#pragma pack(push, 1)
typedef struct
{
  void* p_var;
  enum Datatype type;
  char name[BB_MAX_VARNAMELEN];
} bbField;
#pragma pack(pop)

/*
//Sensor field - has units, can be calibrated
typedef struct
{
  bbField* p_field;
  enum Unit units;
} bbSensor;

//Actuator field - has associated sensor, current command
typedef struct
{
  bbSensor* p_sensor;   //Actuator status
  void* p_cmd;          //Current command
} bbActuator;

//http://publications.gbdirect.co.uk/c_book/chapter9/stdarg.html
//http://en.wikipedia.org/wiki/Variadic_function
typedef struct
{
  char name[MAX_VARNAMELEN];   //Could just be a pointer to a user-deffed string?
  uint8_t nargs;
  enum Datatype[MAX_NARGS];
  char argNames[MAX_NARGS][MAX_VARNAMELEN]; //Could just be a desc. string?
  void (*p_cmdfcn)(...);   //FIXMEOHGOD
} bbCommand;

*/

//Struct to keep all babelbits info
typedef struct
{
  uint8_t nfields;       //Number of fields registered
  bbField fields[BB_MAX_NFIELDS]; //Declared w/ max to avoid dynamic mem alloc

  //Array contains bools indicating which fields client is subscribed to
//Note: could be string of bits instead of bytes & just use bit shifts+mask
  uint8_t subscriptions[BB_MAX_NFIELDS];

  uint8_t* p_out_msgbuf;       //Buffer for outgoing BB messages
  uint16_t out_msgbuf_len;     
  uint16_t out_bytes_waiting;  //Lets user know BB wants to send a msg

  uint8_t connection_state;

  void (*txt_msg_handler)(uint8_t* p_txt);
} bbInstance;


//Public functions
//------------------------------------------------------------------
void bb_init(bbInstance* p_bb, uint8_t* p_buf, uint16_t buflen);
void bb_send_handshake(bbInstance* p_bb);
void bb_processMessage(bbInstance* p_bb, uint8_t* p_msg, uint16_t msgLen);
void bb_register_field(bbInstance* p_bb, void* p_var, enum Datatype type, char* p_name);
int8_t bb_publish_field(bbInstance* p_bb, bbField* p_field);

//Incoming message handlers
void bb_process_subscribe(bbInstance* p_bb, bbMsg_Subscribe* p_msg);
void bb_process_unsubscribe(bbInstance* p_bb, bbMsg_Unsubscribe* p_msg);
