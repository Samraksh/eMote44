#ifndef _OMAC_TYPES_H_
#define _OMAC_TYPES_H_

#define MAX_DATA_PCKT_SIZE 32

#define MAX_UINT16 	(0xFFFF)
#define MAX_UINT32 	(0xFFFFFFFF)
#define MAX_UINT64 	(0xFFFFFFFFFFFFFFFF)
#define MID_UINT64  (0x7FFFFFFFFFFFFFFF)


enum OMACSchedulerState_t{
  I_IDLE,
  I_DATA_RCV_PENDING, //waiting to receive
  I_DATA_SEND_PENDING, //pending to send
  I_TIMESYNC_PENDING,
  I_DISCO_PENDING,
  I_DWELL_SEND, //BK: What is this?
  I_RADIO_STOP_RETRY,
  I_FAILSAFE_STOP,
  I_POST_EXECUTE
} ;

/*
 *
 */
typedef enum {
  NULL_HANDLER,
  CONTROL_BEACON_HANDLER,
  DATA_RX_HANDLER,
  DATA_TX_HANDLER,
  TIMESYNC_HANDLER
} HandlerType_t;

/*
 *
 */
typedef enum {
  OMAC_NORMAL_SEND,
  OMAC_SEND_PRELOAD,
  OMAC_FIRST_SEND
} OMacAction_t;

/*
 *
 */
typedef struct PACK DiscoveryMsg
{
	//UINT32 msg_identifier;
	//seed to generate the pseduo-random wakeup schedule
	UINT16 nextSeed;
	UINT16 mask;
	//use to compute the offset of neighbor's current slot w.r.t. the start of the next frame
	//UINT64 nextwakeupSlot;
	UINT32 nextwakeupSlot0;
	UINT32 nextwakeupSlot1;
	//the wakeup interval of the neighbor
	UINT32 seedUpdateIntervalinSlots;
	//fields below are just for convenience. not transmitted over the air
	//UINT16 nodeID;

	//UINT32 localTime0;
	//UINT32 localTime1;

	//UINT32 lastwakeupSlotUpdateTimeinTicks0;
	//UINT32 lastwakeupSlotUpdateTimeinTicks1;
} DiscoveryMsg_t;

typedef struct{
	UINT8 RSSI;
//	UINT8 LinkQuality;
	UINT8 Delay;
}MsgLinkQualityMetrics_t;

#define DataMsgOverhead sizeof(UINT8)
typedef struct DataMsg_t
{
	//UINT32 msg_identifier;
	UINT8 size;
	UINT8 payload[MAX_DATA_PCKT_SIZE];
} DataMsg_t;

/*
 * After TEP 133, the message timestamp contains the difference between
 * event time and the time the message was actually sent out. TimeSyncP
 * sends the local time associated with this globalTime to the
 * TimeStamping mechanism, which then calculates the difference.
 *
 * On the receiving side, the difference is applied to the local
 * timestamp. The receiving timestamp thus represents the time on the
 * receiving clock when the remote globalTime was taken.
 */
struct PACK TimeSyncMsg
{

//  UINT32 globalTime0;
//  UINT32 globalTime1;

  //the time to startup the radio could be different for different nodes.
  //use this neighbor info along with local info to compute this difference
  //UINT16 radioStartDelay;
//  float skew;
  //UINT32 timesyncIdentifier;
  UINT32 localTime0;
  UINT32 localTime1;
  //bool request_TimeSync;
 // UINT16 nodeID;
  //UINT32 seqNo;

};

struct PACK TimeSyncRequestMsg
{
  //UINT32 timesyncIdentifier;
  bool request_TimeSync;
};




/*
 *
 */
typedef struct DataBeacon {
  UINT16 nodeID;
} DataBeacon_t;

/*
 *
 */
typedef struct OMacHeader {
  UINT8 flag;
} OMacHeader;

//Overflow provisioned class
template<class Base_T>
class OFProv:Base_T{
public:
	bool isThereOverflow(const Base_T& rhs){
		if(rhs>*this){
			if(rhs - *this <= MID_UINT64) return false;
			else return true;
		}
		else{
			if(*this - rhs <= MID_UINT64) return false;
			else return true;
		}
	}
	bool operator<(const Base_T& rhs){
		if (rhs == *this) return false;
		else if(isThereOverflow(rhs)){
			if (rhs<*this) return true;
			else return false;
		}
		else{
			if (rhs<*this) return true;
			else return false;
		}
	}
	bool operator>(const Base_T& rhs){
		if (rhs == *this) return false;
		else if(isThereOverflow(rhs)){
			if (rhs>*this) return true;
			else return false;
		}
		else{
			if (rhs>*this) return true;
			else return false;
		}
	};
	bool operator<=(const Base_T& rhs){
		if (*this == rhs) return true;
		return (*this < *rhs);
	}
	bool operator>=(const Base_T& rhs){
		if (*this == rhs) return true;
		return (*this > *rhs);
	}
};


enum {
  ///TICKS_PER_SEC 	  = 32768,
  ///TICKS_PER_MILLI     = TICKS_PER_SEC / 1000,
  ///TICKS_PER_MICRO     = TICKS_PER_MILLI / 1000,

  TICKS_PER_MILLI     = 8000,
  TICKS_PER_MICRO     = 8,
  BITS_PER_BYTE = 8,

#ifdef SHORT_SLOT
#warning *** USING 8ms SLOT ***
  SLOT_PERIOD_MILLI     = 8,    /*modify this along with SLOT_PERIOD_BITS*/
  SLOT_PERIOD_BITS    = 3 + 13,  /*13 = # of bits of TICKS_PER_MILLI, 4 = # of bits in SLOT_PERIOD_MILLI*/
#else
  //SLOT_PERIOD_MILLI     = 16,     /*modify this along with SLOT_PERIOD_BITS*/
  SLOT_PERIOD_MILLI   = 80,
  SLOT_PERIOD_BITS    = 4 + 13,  /*13 = # of bits of TICKS_PER_MILLI, assuming its a 10Mhz clock, 4 = # of bits in SLOT_PERIOD_MILLI*/


#endif
  SLOT_PERIOD_TICKS   = SLOT_PERIOD_MILLI * TICKS_PER_MILLI,
  SLOT_PERIOD   = SLOT_PERIOD_MILLI,
  DWELL_TIME        = 10,
  /* sender margin compensates for time sync jitter, packet preload delay
   * and radio startup jitter*/
  SENDER_MARGIN_IN_MILLI = 3,
  SENDER_MARGIN     = SENDER_MARGIN_IN_MILLI * TICKS_PER_MILLI,
  TRANSITION_MARGIN   = SENDER_MARGIN_IN_MILLI * TICKS_PER_MILLI,
  MINIMUM_BACKOFF_WINDOW  = 64, /*in the unit of 1/32 ms*/
  RADIO_BACKOFF_WINDOW  = 64, /*in the unit of 1/32 ms*/
  RANDOM_SCHEDULE_WINDOW  = 0x0,
  /* receiver's wait time before the first packet arrives. It will not affect efficiency much
   * if the percentage of failed rendezvous is small because receivers go back to sleep upon
   * receiving the packet tagged as the last packet in the queue*/
  WAIT_TIME_AFTER_DATA_BEACON = (RADIO_BACKOFF_WINDOW + MINIMUM_BACKOFF_WINDOW) / TICKS_PER_MILLI + 12,
  WAIT_TIME_AFTER_PRELOAD = SLOT_PERIOD_MILLI,
  //8 minutes is the optimum time window for linear regression
  //the maximum number of times we tx a packet when previous
  //attempts fail due to congestion
  MAX_RETRY_CNT     = 10,
  // number of entries per neighbor
  MAX_ENTRIES             = 8,
  MAX_POOL_SIZE     = 32,
  MAX_CTRL_MSG_POOL_SIZE  = 8,
  //the number of consecutive messages sent during dwell time
  //DWELL_COUNT       = DEFAULT_DWELL_COUNT,
  DWELL_COUNT       = 3,
  DATA_ALARM_MAX_DURATION = 5,
  DELAY_AVG_FACTOR    = 9,
  //copy  from RIMAC implementation, should acknolwedge them
  //the number of cca detections needed to declare collision
  OMAC_COLLISION_CCA_THRESHOLD = 0,
  MAX_NON_SLEEP_STATE   = 10,
  MAX_NBR_SIZE      = 8,
  AM_DATA_BEACON      = 0x2E,
  AM_TIMESYNCMSG      = 0x3E,
  TIMESYNCMSG_LEN     = sizeof(TimeSyncMsg) - sizeof(UINT32) - sizeof(UINT16),
  INVALID_TIMESTAMP   = 0xFFFFFFFF,
  INVALID_ADDRESS     = 0xFFFF,
  INVALID_INDEX       = 0xFF,
  // time to declare itself the root if no msg was received (in sync periods)
  ROOT_TIMEOUT            = 5,
  // after becoming the root ignore other roots messages (in send period)
  IGNORE_ROOT_MSG         = 4,
  // of entries to become synchronized
  ENTRY_VALID_LIMIT       = 8,
  // if time sync error is bigger than this clear the table
  ENTRY_THROWOUT_LIMIT    = 200,
  // to detect whether my clock or my neighbor's clock has wrapped
  FLAG_TIMESTAMP_VALID  = (UINT8)1 << 3,
  FLAG_REQUEST_BEACON   = (UINT8)1 << 4,
  FLAG_DWELL_TIME     = (UINT8)1 << 6,
  OMAC_HEADER_LEN     = sizeof(OMacHeader),
  SEND_PIGGYBACK_BEACON = 1
    //TODO: needs random mechanism for DATA_INTERVAL
};


typedef OFProv<UINT64> OMACMicroSeconds;
typedef OFProv<UINT64> OMACTicks;


#endif //_OMAC_TYPES_H_