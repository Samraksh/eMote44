/*
 * OMACConstants.h
 *
 *  Created on: Sep 4, 2012
 *      Author: Mukundan
 */

#ifndef OMACCONSTANTS_H_
#define OMACCONSTANTS_H_

#include <tinyhal.h>

#define PACK __attribute__ ((packed))

//types 1-5 are taken up by global types
//#define OMAC_DATA_BEACON_TYPE 7

// Define default primes
#ifndef P1
#define P1 97
#endif

#ifndef P2
#define P2 103
#endif

//How often should a node wakeup (in ms). This is roughly the receive duty cycle.
#ifndef WAKEUP_INTERVAL
#define WAKEUP_INTERVAL 2000
#endif
#ifndef DATA_INTERVAL
#define DATA_INTERVAL 43
#endif

#ifndef OPTIMUM_BEACON_PERIOD
#define OPTIMUM_BEACON_PERIOD 1024 * 32
#endif

#define TIMESTAMP_FOOTER_OFFSET -4
#define TIMESTAMP_SIZE 4

#define MAX_PCKT_SIZE 128

#define MAXUPDATESEEDITERS 2000

#define MILLISECINMICSEC 1000

#define MAX_SLEEP_RETRY_ATTEMPTS 10

#define FRAMERETRYMAXATTEMPT 2
#define FRAMERETRYMAXATTEMPTWARNINGLEVEL 10
#define SLOTRETRYMAXATTEMPT 2
#define CCA_PERIOD_FRAME_RETRY_MICRO 0 //BK: We need to double check this. Since 2 nodes will be off by this much. A node should CCA at least this much to make sure there was no other transmitter trying to reach the same destination.

#define HIGH_DISCO_PERIOD_IN_SLOTS_CONSTANT 13334 //This is setting the length of high disco period after start up. Note that the length of disco slot is different than the length of data slots. The ratio is determined by DISCOPERIODINSLOTS. Note that this is a convention and there is no direct relationship limiting the ratio between the data slots and the discovery slots.
//#define HIGH_DISCO_PERIOD_IN_SLOTS_CONSTANT 0
#ifdef PLATFORM_ARM_EmoteDotNow
#define HIGH_DISCO_PERIOD_ALWAYS_ON 1
#else
#define HIGH_DISCO_PERIOD_ALWAYS_ON 0
#endif

//#define RANDOM_BACKOFF_COUNT_MAX	4
//#define RANDOM_BACKOFF_COUNT_MIN	1
//#define DELAY_DUE_TO_CCA_MICRO	260
//#define RANDOM_BACKOFF_TOTAL_DELAY_MICRO	(RANDOM_BACKOFF_COUNT_MIN*DELAY_DUE_TO_CCA_MICRO)		//Random_backoff can happen atleast once. So, tx should wake up atleast this amount early.
																								// If it wakes up early by RANDOM_BACKOFF_COUNT_MAX amount, scheduler will not have a packet ready for tx.
//#define RETRY_RANDOM_BACKOFF_DELAY_MICRO	(RANDOM_BACKOFF_COUNT_MAX*DELAY_DUE_TO_CCA_MICRO)
//#define OMAC_TIME_ERROR	3*MILLISECINMICSEC	//pessimistic time error
// BK: Not used anymore #define EXTENDED_MODE_TX_DELAY_MICRO	0.8*MILLISECINMICSEC	//delay from start of tx to start of rx
//#define DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX	300	//(A)Delay from start of tx in OMAC to start of writing to SPI bus
//#define DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX	284	//(B)Delay between Node N1 starting TX to node N2 receiving
//#define RETRY_FUDGE_FACTOR	0.3*MILLISECINMICSEC			//(D)From observation, get avg,min,max for (A),(B). Min will go into (A),(B).
															//   Sum of (max-min) of (A),(B) will go into (D)


#define DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX_RF231	284
#define DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX_SI 1718
#define DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX_SX1276 46208 //86208

//#define PROCESSING_DELAY_BEFORE_TX_MICRO (581) //DELAY_FROM_OMAC_TX_TO_RF231_TX //581
#define DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX_RF231 581
#define DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX_SI 1440
#define DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX_SX1276 677
#define RADIO_TURN_ON_DELAY_RX_MICRO_RF231 693
#define RADIO_TURN_ON_DELAY_TX_MICRO_RF231 693
#define RADIO_TURN_ON_DELAY_RX_MICRO_SI 163
#define RADIO_TURN_ON_DELAY_TX_MICRO_SI 143
#define RADIO_TURN_ON_DELAY_RX_MICRO_SX1276 200
#define RADIO_TURN_ON_DELAY_TX_MICRO_SX1276 200


#define RADIO_TURN_OFF_DELAY_MICRO_RF231 184 //453 //BK: This is not used but it is measured 184 micro secs (may not be very accurate)
#define TIMER_MODIFICATION_AND_START_DELAY_MICRO 269 // BK: This is a very rough number

//#define CCA_REACTION_TIME_MICRO 165 //BK: We need to double check this. This is the reaction time of the CCA module from the beginning of channel activity.
//#define CCA_PERIOD_MICRO GUARDTIME_MICRO //BK: We need to double check this. Since 2 nodes will be off by this much. A node should CCA at least this much to make sure there was no other transmitter trying to reach the same destination.
#define CCA_PERIOD_ERROR_SI 1565 //BK: It is observed that CCA is being done more than set by the protocol. This is the observed error on it. It is used in scheduling the tx side this much early
#define CCA_PERIOD_ERROR_RF231 165 //BK: It is observed that CCA is being done more than set by the protocol. This is the observed error on it. It is used in scheduling the tx side this much early
#define CCA_PERIOD_ERROR_SX1276 165 //BK: It is observed that CCA is being done more than set by the protocol. This is the observed error on it. It is used in scheduling the tx side this much early
#define CCA_PERIOD_MICRO 200


#define ADDITIONAL_TIMEADVANCE_FOR_RECEPTION 0
#define TIME_RX_TIMESTAMP_OFFSET_MICRO_SI 1415 //TODO: BK: We need to revisit at this. This is a workaround for some unaccountant time stamping error. In rf231 this is due to the duration from the beginning of the first bit to the end of preamble (note that this is earlier than AMI) when the TS is taken.
#define TIME_RX_TIMESTAMP_OFFSET_MICRO_RF231 141 //Actual number is unknown. Have to go back into previous commits to find out for rf231 //TODO: BK: We need to revisit at this. This is a workaround for some unaccountant time stamping error. In rf231 this is due to the duration from the beginning of the first bit to the end of preamble (note that this is earlier than AMI) when the TS is taken.
#define TIME_RX_TIMESTAMP_OFFSET_MICRO_SX1276 1415 //Actual number is unknown. Have to go back into previous commits to find out for rf231 //TODO: BK: We need to revisit at this. This is a workaround for some unaccountant time stamping error. In rf231 this is due to the duration from the beginning of the first bit to the end of preamble (note that this is earlier than AMI) when the TS is taken.



#define END_OF_TX_TO_RECEPTION_OF_HW_ACK_MICRO	(1.2*MILLISECINMICSEC)
#define HW_ACK_TO_START_OF_TX_MICRO	(2*MILLISECINMICSEC)
#define EXTRA_DELAY_IN_WAITING_FOR_ACK (1.6*MILLISECINMICSEC)	//Difference between FAST_RECOVERY_WAIT_PERIOD_MICRO (or) MAX_PACKET_TX_DURATION_MICRO and 3.4ms. 3.4ms is the ideal round trip time.
#define TIME_BETWEEN_TX_RX_TS_TICKS (266*TICKS_PER_MICRO)

#define RADIO_STOP_RETRY_PERIOD_IN_MICS OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO
//Random_backoff is done before re-transmission
//GUARDTIME_MICRO+OMAC_TIME_ERROR - Pessimistic time error
//GUARDTIME_MICRO - optimistic time error (if there is a re-transmission, tx takes GUARDTIME_MICRO to do CCA

#define EXECUTE_WITH_CCA  0
#define  FAST_RECOVERY 0
#define  FAST_RECOVERY2 0

#define INITIAL_RETRY_BACKOFF_WINDOW_SIZE 3
#define MAX_RETRY_BACKOFF_WINDOW_SIZE 10


//How long should receiver be awake after sending a HW ack. BK: No it is not! see the following
// This is the maximum period to wait for the reception of a packet after receiving StartOfReception interrupt. Due to the change in RF231.cpp the interrupt is received after AMI. Hence it is the packet
#define PACKET_PERIOD_FOR_RECEPTION_HANDLER 16000
//#define PACKET_PERIOD_FOR_RECEPTION_HANDLER EXTENDED_MODE_TX_DELAY+END_OF_TX_TO_RECEPTION_OF_HW_ACK_MICRO+HW_ACK_TO_START_OF_TX_MICRO+CURRENTFRAMERETRYMAXATTEMPT*RANDOM_BACKOFF_TOTAL_DELAY_MICRO

#define RECEIVER_RADIO_STOP_RECHECK_INTERVAL_MICRO 1000
#define TIMER_EVENT_DELAY_OFFSET 0
#define MINEVENTTIME 50000				//minimum time (in micro seconds) required by scheduler to switch between modules
#define SEED_UPDATE_INTERVAL_IN_SLOTS 100 //The FRAME SIZE in slots


//Below 2 values are based on empirical observations made on a debug build
#define FAST_RECOVERY_WAIT_PERIOD_MICRO 5*MILLISECINMICSEC
#define RECV_HW_ACK_WAIT_PERIOD_MICRO	1.7*MILLISECINMICSEC
#define DATATX_POST_EXEC_DELAY	  10*MILLISECINMICSEC


//40000000 - 5 secs
//48000000 - 6 secs
//60000000 - 7.5 secs
//80000000 - 10 secs
//100000000 - 12.5 secs
//#define FORCE_REQUESTTIMESYNC_INTICKS 80000000					//Translates to 120 secs @8Mhz. Receiver centric time threshold to request for a TImeSync msg.
//#define FORCE_REQUESTTIMESYNC_INMICS 10000000000					//Translates to 120 secs @8Mhz. Receiver centric time threshold to request for a TImeSync msg.
////#define SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST  48000000		//Translates to 10 secs @8Mhz. Sender centric time threshold to send a TImeSync msg.
//#define SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST_INMICS  600000000		//Translates to 10 secs @8Mhz. Sender centric time threshold to send a TImeSync msg.


#define HFCLOCKID 1
//#define LFCLOCKID 4 // This is the RTC clock
#define LFCLOCKID LOW_DRIFT_TIMER
#define OMACClockSpecifier LOW_DRIFT_TIMER
//#define OMACClockFreq 32
//#define OMACClocktoSystemClockFreqRatio 244.140625
//#define OMACClockFreq 16
//#define OMACClocktoSystemClockFreqRatio 488.281250
//#define OMACClockSpecifier HFCLOCKID
//#define OMACClockFreq 8000
//#define OMACClocktoSystemClockFreqRatio 1
#define OMACClocktoSystemClockFreqRatio 30//1525.87890625

#define INITIALIZATION_TIMESYNC_INTERVAL_INMICS 10000000   // Controls the interval during the neighbor intiialization period after a discovery. This interval is faster than the regular intervals in order to fill the neighbor's timesync table with the samples from the current node.

#define OMAC_MAX_WAITING_TIME_FOR_TIMESAMPLES_INMICS  ((UINT64)EXPECTED_DISCOVERY_TIME_FOR_A_SINGLE_SAMPLE_IN_HOURS*(UINT64)3600*(UINT64)1000000) //After discovering a neighbor (ie receiving its schedule) OMAC_waits this much amount of time  to collect enough time samples to predict time before allowing deletion of this neighbor's entry in the neighbor table when a new neighbor is discovered. This is related to the max disco interval since it is the default way of discovering the neighbors.

#if (OMACClockSpecifier==LFCLOCKID)
//#define FORCE_REQUESTTIMESYNC_INTICKS 80000000					//Translates to 120 secs @8Mhz. Receiver centric time threshold to request for a TImeSync msg.
#define FORCE_REQUESTTIMESYNC_INMICS 6100000000					//Translates to 120 secs @8Mhz. Receiver centric time threshold to request for a TImeSync msg.
//#define SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST        10000000		//Translates to 10 secs @8Mhz. Sender centric time threshold to send a TImeSync msg.
#define SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST_INMICS  300000000		//z. Sender centric time threshold to send a TImeSync msg.
////GUARDTIME_MICRO should be calculated in conjuction with SLOT_PERIOD_MILLI
//// GUARDTIME_MICRO = (SLOT_PERIOD_MILLI - PacketTime)/2 - SWITCHING_DELAY_MICRO
////PacketTime = 125byte * 8 bits/byte / (250*10^3 bits/sec) = 4sec

///#define GUARDTIME_MICRO 2000//JH:why guardtime is 200ms?
#define GUARDTIME_MICRO 1000

#else
//#define FORCE_REQUESTTIMESYNC_INTICKS 80000000					//Translates to 120 secs @8Mhz. Receiver centric time threshold to request for a TImeSync msg.
#define FORCE_REQUESTTIMESYNC_INMICS 100000000					//Translates to 120 secs @8Mhz. Receiver centric time threshold to request for a TImeSync msg.
//#define SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST  48000000		//Translates to 10 secs @8Mhz. Sender centric time threshold to send a TImeSync msg.
#define SENDER_CENTRIC_PROACTIVE_TIMESYNC_REQUEST_INMICS  6000000		//Translates to 10 secs @8Mhz. Sender centric time threshold to send a TImeSync msg.
////GUARDTIME_MICRO should be calculated in conjuction with SLOT_PERIOD_MILLI
//// GUARDTIME_MICRO = (SLOT_PERIOD_MILLI - PacketTime)/2 - SWITCHING_DELAY_MICRO
////PacketTime = 125byte * 8 bits/byte / (250*10^3 bits/sec) = 4sec
#define GUARDTIME_MICRO 20000
#endif

#define OMAC_SCHEDULER_MIN_REACTION_TIME_IN_TICKS 4000
#define OMAC_SCHEDULER_MIN_REACTION_TIME_IN_MICRO 500

#define FAILSAFETIME_MICRO 6000000 //60000000//60000000000

#define WAKEUPPERIODINTICKS 8000000

#define TIMEITTAKES2TXDISCOPACKETINMICSEC 4096
#define DISCOPERIODINMILLI 75

//FCF table:
//15 14 13  12  11  10  9  8  7  6  5  4  3  2  1  0 (bits)
//1   0  0   0   1   0  0  0  0  1  1  0  0  0  0  1 (values)
//Frame type (bits 0,1,2)  			- Data
//Security enabled (bit 3) 			- No
//Frame pending (bit 4)				- No
//Ack request (bit 5)				- Yes
//Intra-pan (bit 6)					- Yes, source PAN-ID is omitted
//Bits 7,8,9						- Reserved
//Dest addressing mode (bits 10,11)	- Address field contains a 16-bit short address
//Frame version (bits 12,13)		- Frames are compatible with IEEE 802.15.4 2003
//Src addressing mode (bits 14,15)	- Address field contains a 16-bit short address
#define FCF_WORD_VALUE 0x8861 //34913
#define FCF_WORD_VALUE_DISCO FCF_WORD_VALUE//34913
#define SRC_PAN_ID	0xAAAA
#define DEST_PAN_ID	0x5555

//extern UINT16  CONTROL_BEACON_INTERVAL_SLOT = 7500;

extern UINT32 ArbiterP_Timing;

/*
 * Prime numbers used in determining DISCO period of a node
 */
#define EXPECTED_DISCOVERY_TIME_FOR_A_SINGLE_SAMPLE_IN_HOURS 152 //Max(CONTROL_P1*ControlP1) * 15ms

#define EXPECTED_DISCOVERY_TIME_FOR_A_SINGLE_SAMPLE_IN_TICKS EXPECTED_DISCOVERY_TIME_FOR_A_SINGLE_SAMPLE_IN_HOURS*3600*1000*1000*8

#define DISCOVERY_SIZE_OF_PRIME_NUMBER_POOL 7


//Define total size of a Disco packet with piggybacking
#if OMAC_DEBUG_SEND_EXTENDEDMACINfo

#else

#endif



#endif /* OMACCONSTANTS_H_ */
