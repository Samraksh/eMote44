/*
 *  OMAC.cpp
 *
 *  Created on: Aug 30, 2012
 *  	Authors: Mukundan.Sridharan, Nived.Sivadas
 *  	Description :  OMAC Implementation, v 1.0
 *
 *  Modified on: Oct 30, 2015
 *  	Authors: Bora Karaoglu; Ananth Muralidharan
 *
 *  Copyright The Samraksh Company
 */
#define UINT_TO_BINARY(byte)  \
		(byte & 0x80 ? '1' : '0'), \
		(byte & 0x40 ? '1' : '0'), \
		(byte & 0x20 ? '1' : '0'), \
		(byte & 0x10 ? '1' : '0'), \
		(byte & 0x08 ? '1' : '0'), \
		(byte & 0x04 ? '1' : '0'), \
		(byte & 0x02 ? '1' : '0'), \
		(byte & 0x01 ? '1' : '0')

#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/Radio_decl.h>

UINT16 CONTROL_BEACON_INTERVAL_SLOT;
UINT32 ArbiterP_Timing;

#include <Samraksh/MAC/OMAC/OMACConstants.h>

#define DEBUG_OMAC 0
const bool NEED_OMAC_CALLBACK_CONTINUATION = false;

#define SOFT_BREAKPOINT() ASSERT(0)

#if defined(OMAC_DEBUG_PRINTF) || defined(DEBUG_OMAC_PREPAREMESSAGEBUFFER)
#define DEBUG_OMAC_PMB_PRINTF(...) hal_printf( __VA_ARGS__ )
#else
#define DEBUG_OMAC_PMB_PRINTF(...) (void)0
#endif

OMACType g_OMAC;

UINT8 OMACType::payloadTypeArrayIndex = 0;
HAL_CONTINUATION OMACType::OMAC_callback_continuation;

UINT32 OMACType::GUARDTIME_MICRO_OMAC = GUARDTIME_MICRO;
UINT16 OMACType::OMAC_TIME_ERROR = 3*MILLISECINMICSEC;
UINT16 OMACType::DELAY_DUE_TO_CCA_MICRO = CCA_PERIOD_MICRO;
UINT16 OMACType::RETRY_FUDGE_FACTOR = 0.3*MILLISECINMICSEC;


// long long unsigned integer
static void x64toa(unsigned long long val, char *buf, unsigned radix, int is_neg)
{
  char *p;
  char *firstdig;
  char temp;
  unsigned digval;
  p = buf; *p=0,p[1]='\0',p;
  if (val==0||radix<2||radix>32||radix&1) return; 
  if ( is_neg )  *p++ = '-', val = (unsigned long long)(-(long long)val);
  firstdig = p;
  if(radix--==10)
    do { // optimized for fixed division
    digval = (unsigned) (val % 10);
    val /= 10;
    *p++ = (char) (digval + '0');
    } while (val > 0);
  else do { temp=radix;
    digval = (unsigned) (val & radix );
    while(temp) val>>=1,temp>>=1;
    *p++ = digval>9?(char)(digval + 'W'):(char) (digval + '0');
  } while (val>0);
  *p-- = '\0';

  do { // reverse string
    temp = *p;
    *p = *firstdig;
    *firstdig = temp;
    --p;
    ++firstdig;
  } while (firstdig < p);
}

//----------------------------------------------------------------------------
char* _i64toa(long long val, char *buf, int radix)
{
  x64toa((unsigned long long)val, buf, radix, (radix == 10 && val < 0));
  return buf;
}

//----------------------------------------------------------------------------
char* _ui64toa(unsigned long long val, char *buf, int radix)
{
  x64toa(val, buf, radix, 0);
  return buf;
}

char* l2s(long long v,int sign) { 
	char r,s;
	static char buff[33];  
	r=sign>>8; 
	s=sign;
	if(!r) r=10; 
	if(r!=10||s&&v>=0) s=0; 
	if(r<8) r=0;
	x64toa(v,buff,r,s); 
	return buff;
}
//--//


/*
 *
 */
void* OMACReceiveHandler(void* msg, UINT16 size){
	return (void*) g_OMAC.ReceiveHandler((Message_15_4_t*) msg, size);
}

/*
 *
 */
BOOL OMACRadioInterruptHandler(RadioInterrupt Interrupt, void* Param){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(DATA_RX_INTERRUPT_PIN, TRUE);
#endif
	if( g_OMAC.m_omac_scheduler.m_execution_started && g_OMAC.m_omac_scheduler.m_state == I_DATA_RCV_PENDING && Interrupt==StartOfReception){
		g_OMAC.m_omac_scheduler.m_DataReceptionHandler.HandleRadioInterrupt();
	}
	else if( g_OMAC.m_omac_scheduler.m_execution_started && g_OMAC.m_omac_scheduler.m_state == I_DISCO_PENDING && Interrupt==StartOfReception){
		g_OMAC.m_omac_scheduler.m_DiscoveryHandler.HandleRadioInterrupt();
	}
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(DATA_RX_INTERRUPT_PIN, FALSE);
#endif
	return TRUE;
}

/*
 *
 */
void OMACSendAckHandler(void* msg, UINT16 Size, NetOpStatus status, UINT8 radioAckStatus){
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(DATA_TX_ACK_PIN, TRUE);
#endif

	Message_15_4_t* rcv_msg = (Message_15_4_t*)msg;

	//Set flag when send is complete
	g_OMAC.isSendDone = true;

	if(status != NetworkOperations_Success){
		return;
	}
	if(Size == sizeof(softwareACKHeader)){ //For not this type is not expected.
		if(g_OMAC.m_omac_scheduler.m_execution_started && g_OMAC.m_omac_scheduler.m_state == I_DATA_RCV_PENDING){
			g_OMAC.m_omac_scheduler.m_DataReceptionHandler.SendACKHandler();
		}
	}
	else{
		//Demutiplex packets received based on type
		switch(rcv_msg->GetHeader()->payloadType){
		case MFM_OMAC_DISCOVERY:
			g_OMAC.m_omac_scheduler.m_DiscoveryHandler.BeaconAckHandler(rcv_msg,rcv_msg->GetPayloadSize(),status);
			break;
		case MFM_OMAC_ROUTING:
			break;
		case MFM_OMAC_NEIGHBORHOOD:
			break;
		case MFM_TIMESYNC:
#ifdef OMAC_DEBUG_GPIO		
			CPU_GPIO_SetPinState(SEND_ACK_PIN, TRUE);
			CPU_GPIO_SetPinState(SEND_ACK_PIN, FALSE);
#endif
			break;
		case MFM_OMAC_DATA_ACK:
			if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
				if(g_OMAC.m_omac_scheduler.m_execution_started && g_OMAC.m_omac_scheduler.m_state == I_DATA_RCV_PENDING ){
					g_OMAC.m_omac_scheduler.m_DataReceptionHandler.SendACKHandler();
				}
			}
			break;
		case MFM_DATA:
		default:
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(SEND_ACK_PIN, TRUE);
#endif			
			if(g_OMAC.m_omac_scheduler.m_execution_started && g_OMAC.m_omac_scheduler.m_state == I_DATA_SEND_PENDING && radioAckStatus == NetworkOperations_Success){
				g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.SendACKHandler(rcv_msg, radioAckStatus);
			}
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(SEND_ACK_PIN, FALSE);
#endif
			break;
		}
	}

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(DATA_TX_ACK_PIN, FALSE);
#endif
}


//DeviceStatus SetConfig(MACConfig *config);
//BOOL Send(UINT16 dest, UINT8 dataType, void* msg, int Size);

/*
 *
 */
DeviceStatus OMACType::SetConfig(MACConfig *config){

#if OMAC_DEBUG_PRINTF_SETCONFIG
	hal_printf("MyConfig.CCA = %d, config->CCA = %d ",MyConfig.CCA, config->CCA);
	hal_printf("MyConfig.NumberOfRetries = %u, config->NumberOfRetries = %u ",MyConfig.NumberOfRetries, config->NumberOfRetries);
	hal_printf("MyConfig.CCASenseTime = %u, config->CCASenseTime = %u ",MyConfig.CCASenseTime, config->CCASenseTime);
	hal_printf("MyConfig.BufferSize = %u, config->BufferSize = %u ",MyConfig.BufferSize, config->BufferSize);
	hal_printf("MyConfig.NeighborLivenessDelay = %lu, config->NeighborLivenessDelay = %lu ",MyConfig.NeighborLivenessDelay, config->NeighborLivenessDelay);
#endif


	/*MyConfig.FCF = config->FCF;
	MyConfig.DestPAN = config->DestPAN;
	MyConfig.Network = config->Network;*/
	MyConfig.CCA = config->CCA;
	MyConfig.NumberOfRetries = config->NumberOfRetries;
	MyConfig.CCASenseTime = config->CCASenseTime;
	MyConfig.BufferSize = config->BufferSize;
	//MyConfig.RadioType = config->RadioType;
	MyConfig.NeighborLivenessDelay = config->NeighborLivenessDelay;

	return DS_Success;
}

DeviceStatus OMACType::SetOMACParametersBasedOnRadioName(UINT8 radioName){
	/*OMAC_TIME_ERROR = 3*MILLISECINMICSEC;
	DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX = 300;
	DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX = 284;
	DELAY_DUE_TO_CCA_MICRO = 260;
	RETRY_FUDGE_FACTOR = 0.3*MILLISECINMICSEC;
	RANDOM_BACKOFF_COUNT_MAX = 4;
	RANDOM_BACKOFF_COUNT_MIN = 1;*/

	CCA_PERIOD_DESIRED = CCA_PERIOD_MICRO;

	DeviceStatus rv = DS_Fail;
	switch(radioName){
	case RF231RADIOLR:
	case RF231RADIO:
	{
		UINT16 TX_BUFFER = 1 * MILLISECINMICSEC;
		UINT16 RX_BUFFER = 1 * MILLISECINMICSEC;
		UINT16 DISCO_BUFFER = 0.5 * MILLISECINMICSEC;
		TX_TIME_PER_BIT_IN_MICROSEC = 3.9;	/*** (1/256kbps) ***/
		RETRY_RANDOM_BACKOFF_DELAY_MICRO = (RANDOM_BACKOFF_COUNT_MAX*DELAY_DUE_TO_CCA_MICRO);
		MAX_PACKET_TX_DURATION_MICRO = (IEEE802_15_4_FRAME_LENGTH*BITS_PER_BYTE*TX_TIME_PER_BIT_IN_MICROSEC) + TX_BUFFER;	//5*MILLISECINMICSEC;
		MAX_PACKET_RX_DURATION_MICRO = MAX_PACKET_TX_DURATION_MICRO + RX_BUFFER;
		ACK_RX_MAX_DURATION_MICRO = (sizeof(softwareACKHeader)*BITS_PER_BYTE*TX_TIME_PER_BIT_IN_MICROSEC) + RX_BUFFER;	//20*MILLISECINMICSEC;
		DISCO_PACKET_TX_TIME_MICRO = (sizeof(DiscoveryMsg_t)*BITS_PER_BYTE*TX_TIME_PER_BIT_IN_MICROSEC) + DISCO_BUFFER;	//1*MILLISECINMICSEC;
		DISCO_BEACON_TX_MAX_DURATION_MICRO = 1.2*MILLISECINMICSEC;
		DISCO_SLOT_PERIOD_MICRO = DISCOPERIODINMILLI *MILLISECINMICSEC;
		HIGH_DISCO_PERIOD_IN_SLOTS = HIGH_DISCO_PERIOD_IN_SLOTS_CONSTANT;
		if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
			DELAY_IN_RECEIVING_HW_ACK = 0*MILLISECINMICSEC;
			DELAY_IN_RECEIVING_SW_ACK = 0.6*MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_HW_ACK = 0*MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_SW_ACK = 0.2*MILLISECINMICSEC;
			ACK_TX_MAX_DURATION_MICRO = 4*MILLISECINMICSEC;
		}
		else if(CPU_Radio_GetRadioAckType() == HARDWARE_ACK){
			DELAY_IN_RECEIVING_HW_ACK = 0.4*MILLISECINMICSEC;
			DELAY_IN_RECEIVING_SW_ACK = 0*MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_HW_ACK = 0*MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_SW_ACK = 0*MILLISECINMICSEC;
			ACK_TX_MAX_DURATION_MICRO = 0*MILLISECINMICSEC;
		}
		else{
			ASSERT_SP(0);
		}
		DELAY_IN_RECEIVING_ACK = DELAY_IN_RECEIVING_HW_ACK+DELAY_IN_RECEIVING_SW_ACK;
		RETRANS_DELAY_DUE_TO_MISSING_ACK = RETRANS_DELAY_DUE_TO_MISSING_HW_ACK+RETRANS_DELAY_DUE_TO_MISSING_SW_ACK;
		DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX = DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX_RF231;
		DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX = DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX_RF231;
		CCA_PERIOD_ACTUAL = CCA_PERIOD_DESIRED + CCA_PERIOD_ERROR_RF231;
		RADIO_TURN_ON_DELAY_TX = RADIO_TURN_ON_DELAY_TX_MICRO_RF231;
		RADIO_TURN_ON_DELAY_RX = RADIO_TURN_ON_DELAY_RX_MICRO_RF231;
		TIME_RX_TIMESTAMP_OFFSET_MICRO = TIME_RX_TIMESTAMP_OFFSET_MICRO_RF231;

		if(FAST_RECOVERY)
			LISTEN_PERIOD_FOR_RECEPTION_HANDLER = GUARDTIME_MICRO+GUARDTIME_MICRO+OMAC_TIME_ERROR\
			+DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX+DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX+DELAY_IN_RECEIVING_ACK\
			+RETRY_RANDOM_BACKOFF_DELAY_MICRO+RETRY_FUDGE_FACTOR\
			+RETRANS_DELAY_DUE_TO_MISSING_ACK;
		else if(FAST_RECOVERY2)
			LISTEN_PERIOD_FOR_RECEPTION_HANDLER  =   GUARDTIME_MICRO + GUARDTIME_MICRO + DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //This is the duration used in FastRecovery 2.0
			else
				LISTEN_PERIOD_FOR_RECEPTION_HANDLER  =   GUARDTIME_MICRO + GUARDTIME_MICRO + DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //This is the duration used in FastRecovery 2.0
		rv = DS_Success;
		break;
	}
	case SI4468_SPI2:
	{
		UINT16 TX_BUFFER = 2 * MILLISECINMICSEC;
		UINT16 RX_BUFFER = 7 * MILLISECINMICSEC;
		UINT16 DISCO_BUFFER = 1 * MILLISECINMICSEC;
		TX_TIME_PER_BIT_IN_MICROSEC = 25;	/*** (1/40kbps) ***/
		RETRY_RANDOM_BACKOFF_DELAY_MICRO = (RANDOM_BACKOFF_COUNT_MAX*DELAY_DUE_TO_CCA_MICRO);
		MAX_PACKET_TX_DURATION_MICRO = (IEEE802_15_4_FRAME_LENGTH*BITS_PER_BYTE*TX_TIME_PER_BIT_IN_MICROSEC) + TX_BUFFER;	//27.6*MILLISECINMICSEC;
		MAX_PACKET_RX_DURATION_MICRO = MAX_PACKET_TX_DURATION_MICRO;
		ACK_RX_MAX_DURATION_MICRO = (sizeof(softwareACKHeader)*BITS_PER_BYTE*TX_TIME_PER_BIT_IN_MICROSEC) + RX_BUFFER;	//8*MILLISECINMICSEC;
		DISCO_PACKET_TX_TIME_MICRO = MAX_PACKET_TX_DURATION_MICRO; //(sizeof(DiscoveryMsg_t)*BITS_PER_BYTE*TX_TIME_PER_BIT_IN_MICROSEC) + DISCO_BUFFER;	//10*MILLISECINMICSEC;
		DISCO_BEACON_TX_MAX_DURATION_MICRO = 10*MILLISECINMICSEC;
		DISCO_SLOT_PERIOD_MICRO = DISCOPERIODINMILLI *MILLISECINMICSEC;
		HIGH_DISCO_PERIOD_IN_SLOTS = HIGH_DISCO_PERIOD_IN_SLOTS_CONSTANT;
		if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
			DELAY_IN_RECEIVING_HW_ACK = 0*MILLISECINMICSEC;
			DELAY_IN_RECEIVING_SW_ACK = 0*MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_HW_ACK = 0*MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_SW_ACK = 0*MILLISECINMICSEC;
			ACK_TX_MAX_DURATION_MICRO = 8*MILLISECINMICSEC;;
		}
		else if(CPU_Radio_GetRadioAckType() == HARDWARE_ACK){
			ASSERT_SP(0);
		}
		else{
			ASSERT_SP(0);
		}
		DELAY_IN_RECEIVING_ACK = 3.6*MILLISECINMICSEC;
		RETRANS_DELAY_DUE_TO_MISSING_ACK = 12.8*MILLISECINMICSEC;
		DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX = DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX_SI;
		DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX = DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX_SI;
		CCA_PERIOD_ACTUAL = CCA_PERIOD_DESIRED + CCA_PERIOD_ERROR_SI;
		RADIO_TURN_ON_DELAY_TX = RADIO_TURN_ON_DELAY_TX_MICRO_SI;
		RADIO_TURN_ON_DELAY_RX = RADIO_TURN_ON_DELAY_RX_MICRO_SI;
		TIME_RX_TIMESTAMP_OFFSET_MICRO = TIME_RX_TIMESTAMP_OFFSET_MICRO_SI;

		if(FAST_RECOVERY)
			LISTEN_PERIOD_FOR_RECEPTION_HANDLER = GUARDTIME_MICRO+GUARDTIME_MICRO+OMAC_TIME_ERROR\
			+DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX+DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX+DELAY_IN_RECEIVING_ACK\
			+RETRY_RANDOM_BACKOFF_DELAY_MICRO+RETRY_FUDGE_FACTOR\
			+RETRANS_DELAY_DUE_TO_MISSING_ACK;
		else if(FAST_RECOVERY2)
			LISTEN_PERIOD_FOR_RECEPTION_HANDLER  =   GUARDTIME_MICRO + GUARDTIME_MICRO + DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //This is the duration used in FastRecovery 2.0
			else
				LISTEN_PERIOD_FOR_RECEPTION_HANDLER  =   GUARDTIME_MICRO + GUARDTIME_MICRO + DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //This is the duration used in FastRecovery 2.0

		rv = DS_Success;
		break;
	}
	case SX1276RADIO:
		{
			UINT16 TX_BUFFER = 10351;//20 * MILLISECINMICSEC;
			UINT16 RX_BUFFER = 70 * MILLISECINMICSEC;
			UINT16 DISCO_BUFFER = 7031;//10 * MILLISECINMICSEC;
			TX_TIME_PER_BIT_IN_MICROSEC = (UINT32)1000;	/*** (1/1kbps) ***/
			RETRY_RANDOM_BACKOFF_DELAY_MICRO = ((UINT32)RANDOM_BACKOFF_COUNT_MAX*(UINT32)DELAY_DUE_TO_CCA_MICRO);
			MAX_PACKET_TX_DURATION_MICRO = 56559;//((UINT32)IEEE802_15_4_FRAME_LENGTH*(UINT32)BITS_PER_BYTE*(UINT32)TX_TIME_PER_BIT_IN_MICROSEC) + TX_BUFFER;
			MAX_PACKET_RX_DURATION_MICRO = 46208;//MAX_PACKET_TX_DURATION_MICRO;
			ACK_RX_MAX_DURATION_MICRO = 15488;//MAX_PACKET_TX_DURATION_MICRO; //(sizeof(softwareACKHeader)*BITS_PER_BYTE*(UINT32)TX_TIME_PER_BIT_IN_MICROSEC) + RX_BUFFER;	//8*MILLISECINMICSEC;
			DISCO_PACKET_TX_TIME_MICRO = 43000;//(sizeof(DiscoveryMsg_t)*BITS_PER_BYTE*(UINT32)TX_TIME_PER_BIT_IN_MICROSEC) + DISCO_BUFFER;	//10*MILLISECINMICSEC;
			DISCO_BEACON_TX_MAX_DURATION_MICRO = 10*MILLISECINMICSEC;
			DISCO_SLOT_PERIOD_MICRO = DISCOPERIODINMILLI *MILLISECINMICSEC;
			HIGH_DISCO_PERIOD_IN_SLOTS = HIGH_DISCO_PERIOD_IN_SLOTS_CONSTANT;
			if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
				DELAY_IN_RECEIVING_HW_ACK = 0*MILLISECINMICSEC;
				DELAY_IN_RECEIVING_SW_ACK = 0*MILLISECINMICSEC;
				RETRANS_DELAY_DUE_TO_MISSING_HW_ACK = 0*MILLISECINMICSEC;
				RETRANS_DELAY_DUE_TO_MISSING_SW_ACK = 0*MILLISECINMICSEC;
				ACK_TX_MAX_DURATION_MICRO = 25839;//MAX_PACKET_TX_DURATION_MICRO; //100*(UINT32)MILLISECINMICSEC;;
			}
			else if(CPU_Radio_GetRadioAckType() == HARDWARE_ACK){
				ASSERT_SP(0);
			}
			else{
				ASSERT_SP(0);
			}
			DELAY_IN_RECEIVING_ACK = (UINT32)5*(UINT32)MILLISECINMICSEC;
			RETRANS_DELAY_DUE_TO_MISSING_ACK = (UINT32)12.8*(UINT32)MILLISECINMICSEC;
			DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX = (UINT32)DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX_SX1276;
			DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX = (UINT32)DELAY_FROM_DTH_TX_TO_RADIO_DRIVER_TX_SX1276;
			CCA_PERIOD_ACTUAL = CCA_PERIOD_DESIRED + (UINT32)CCA_PERIOD_ERROR_SX1276;
			RADIO_TURN_ON_DELAY_TX = (UINT32)RADIO_TURN_ON_DELAY_TX_MICRO_SX1276;
			RADIO_TURN_ON_DELAY_RX = (UINT32)RADIO_TURN_ON_DELAY_RX_MICRO_SX1276;
			TIME_RX_TIMESTAMP_OFFSET_MICRO = (UINT32)TIME_RX_TIMESTAMP_OFFSET_MICRO_SX1276;

			if(FAST_RECOVERY)
				LISTEN_PERIOD_FOR_RECEPTION_HANDLER = GUARDTIME_MICRO+GUARDTIME_MICRO+OMAC_TIME_ERROR\
				+DELAY_FROM_OMAC_TX_TO_RADIO_DRIVER_TX+DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX+DELAY_IN_RECEIVING_ACK\
				+RETRY_RANDOM_BACKOFF_DELAY_MICRO+RETRY_FUDGE_FACTOR\
				+RETRANS_DELAY_DUE_TO_MISSING_ACK;
			else if(FAST_RECOVERY2)
				LISTEN_PERIOD_FOR_RECEPTION_HANDLER  =   GUARDTIME_MICRO + GUARDTIME_MICRO + DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //This is the duration used in FastRecovery 2.0
				else
					LISTEN_PERIOD_FOR_RECEPTION_HANDLER  =   GUARDTIME_MICRO + GUARDTIME_MICRO + DELAY_FROM_RADIO_DRIVER_TX_TO_RADIO_DRIVER_RX; //This is the duration used in FastRecovery 2.0

			rv = DS_Success;
			break;
		}



	default:
	{
		ASSERT_SP(0);
		break;
	}
	}
	return rv;
}

void PushToUpperLayerHelper(void* arg)
{
	g_OMAC.PushPacketsToUpperLayers(arg);
}

void OMACType::PushPacketsToUpperLayers(void* arg){
	/*Message_15_4_t* next_free_buffer;
	while(!g_receive_buffer.IsEmpty()){
		next_free_buffer = g_receive_buffer.GetOldest();
		(*m_rxAckHandler)(next_free_buffer, next_free_buffer->GetHeader()->length - sizeof(IEEE802_15_4_Header_t));
	}*/

	//No need for a while loop, as every time a data msg is received, this callback is enqueued
	//Message_15_4_t* lastFullBuffer = g_receive_buffer.GetLastFullBuffer();
	//(*m_rxAckHandler)(lastFullBuffer, lastFullBuffer->GetHeader()->payloadType);
	Message_15_4_t* lastFullBuffer = NULL;
	payloadTypeArrayIndex--;
	ASSERT_SP(payloadTypeArrayIndex >= 0);
	UINT8 payloadTypeUpperLayer = payloadTypeArray[payloadTypeArrayIndex];
	(*m_rxAckHandler)(lastFullBuffer, payloadTypeUpperLayer);
}

void OMACType::SendRXPacketToUpperLayers(Message_15_4_t *msg, UINT8 payloadType){
	MACReceiveFuncPtrType multi_m_rxAckHandler = NULL;
	if( IsValidNativeAppIdOffset(payloadType) )
	{
		MACEventHandler* mac_event_handler_ptr = g_OMAC.GetNativeAppHandler(payloadType);
		if(mac_event_handler_ptr){
			multi_m_rxAckHandler = mac_event_handler_ptr->ReceiveHandler;
		}
	}
	//
	if(multi_m_rxAckHandler == NULL) {
		multi_m_rxAckHandler = m_rxAckHandler;
	}
	if(multi_m_rxAckHandler != NULL) {
		(*multi_m_rxAckHandler)(msg, payloadType);
#if OMAC_RECEIVE_DEBUGGING_FOR_MF
		hal_printf("R\r\n");
#endif
	}
}

/*
 *
 */
DeviceStatus OMACType::Initialize(MACEventHandler* eventHandler, UINT8 macName, UINT8 routingAppID, UINT8 radioID, MACConfig* config) {
	//DeviceStatus OMACType::Initialize(MACEventHandler* eventHandler, UINT8* macID, UINT8 routingAppID, MACConfig *config) {


#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_EnableOutputPin(OMAC_DATARXPIN, TRUE);
	CPU_GPIO_SetPinState( OMAC_DATARXPIN, FALSE );
	CPU_GPIO_EnableOutputPin(DATARX_TIMESTAMP_PIN, TRUE);
	CPU_GPIO_EnableOutputPin(OMAC_TIMESYNCREQRXPIN, TRUE);
	CPU_GPIO_EnableOutputPin(SEND_ACK_PIN, TRUE);
	CPU_GPIO_EnableOutputPin(OMAC_RXPIN, FALSE);
	CPU_GPIO_EnableOutputPin(DATA_TX_ACK_PIN, FALSE);
	CPU_GPIO_SetPinState(DATA_TX_ACK_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(DATA_RX_INTERRUPT_PIN, FALSE);
	CPU_GPIO_EnableOutputPin(DATATX_POSTEXEC, TRUE);
	CPU_GPIO_SetPinState( DATATX_POSTEXEC, FALSE );
	CPU_GPIO_EnableOutputPin(OMAC_CONTINUATION, TRUE);
	CPU_GPIO_SetPinState( OMAC_CONTINUATION, FALSE );

#endif

#if OMAC_DEBUG_PRINTF_NEIGHCHANGE || OMAC_DEBUG_PRINTF_DISCO_RX || OMAC_DEBUG_PRINTF_TS_RX
	is_print_neigh_table = false;
#endif


	DeviceStatus status;
	//Initialize yourself first (you being the MAC)
	if(this->Initialized){
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("OMACType Error: Already Initialized!! My address: %d\r\n", g_OMAC.GetMyAddress());
#endif
	}
	else {
		////MAC<Message_15_4_t, MACConfig>::Initialize(eventHandler, macName, routingAppID, radioID, config);
		if(routingAppID >= MAX_APPS) {
			return DS_Fail;
		}
		this->macName = macName;
		this->radioName = radioID;
		this->isSendDone = true;
		SetConfig(config);

		//Initialize upper layer call backs
		MAC<Message_15_4_t, MACConfig>::SetAppIdIndex(routingAppID);
		MAC<Message_15_4_t, MACConfig>::SetAppHandlers(eventHandler);
		MACEventHandler_t* appHandler = MAC<Message_15_4_t, MACConfig>::GetAppHandler(MAC<Message_15_4_t, MACConfig>::GetAppIdIndex());

		AppCount = 0; //number of upperlayers connected to you
		//MaxPayload is already defined in Message.h
		//		OMACType::SetMaxPayload( (UINT16)( IEEE802_15_4_FRAME_LENGTH-sizeof(IEEE802_15_4_Header_t) ) );
		//OMACType::SetMaxPayload( (UINT16)( IEEE802_15_4_FRAME_LENGTH-sizeof(IEEE802_15_4_Header_t)-sizeof(IEEE802_15_4_Metadata) ) );
		OMACType::SetMaxPayload((UINT16)IEEE802_15_4_MAX_PAYLOAD);

		Radio_Event_Handler.RadioInterruptMask = (StartOfTransmission|EndOfTransmission|StartOfReception);
		Radio_Event_Handler.SetRadioInterruptHandler(OMACRadioInterruptHandler);
		Radio_Event_Handler.SetReceiveHandler(OMACReceiveHandler);
		Radio_Event_Handler.SetSendAckHandler(OMACSendAckHandler);

		OMAC_callback_continuation.InitializeCallback(PushToUpperLayerHelper, NULL);

		//g_send_buffer.Initialize();
		g_receive_buffer.Initialize();
		g_NeighborTable.ClearTable();

		g_NeighborTable.SetPreviousNumberOfNeighbors(0);
		if((status = CPU_Radio_Initialize(&Radio_Event_Handler, this->radioName, NumberRadios, macName)) != DS_Success) {
			SOFT_BREAKPOINT();
			return status;
		}

		if((status = CPU_Radio_TurnOnRx(this->radioName)) != DS_Success){
			return status;
		}

		SetOMACParametersBasedOnRadioName(this->radioName);
		SetMyAddress(CPU_Radio_GetAddress(radioName));
		SetMyID(CPU_Radio_GetAddress(radioName));

#if OMAC_DEBUG_PRINTF_RADIOADDRESS
		hal_printf("Initializing OMACType: My address: %d \r\n", g_OMAC.GetMyAddress());
#endif

		m_omac_RadioControl.Initialize();
		m_omac_scheduler.Initialize(radioName, macName);
		Initialized = TRUE;
	}

	CurrentActiveApp = routingAppID;

	//hal_printf("OMACType::init CPU_GetCurrentPowerLevel = %u \r\n",CPU_GetCurrentPowerLevel());

	m_rxAckHandler = NULL;
	m_txAckHandler = NULL;
	if(g_OMAC.GetAppHandler(g_OMAC.GetAppIdIndex())){
		m_rxAckHandler = g_OMAC.GetAppHandler(g_OMAC.GetAppIdIndex())->GetReceiveHandler();
		m_txAckHandler = g_OMAC.GetAppHandler(g_OMAC.GetAppIdIndex())->GetSendAckHandler();
	}

#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif


#ifdef def_Neighbor2beFollowed
#if defined(TWO_NODES_TX_RX)
	if(g_OMAC.GetMyAddress() == RXNODEID) {
		Neighbor2beFollowed = TXNODEID;
	}
	else {
		Neighbor2beFollowed = RXNODEID;
	}
#endif
#endif

#if defined(FAN_OUT)
	if(g_OMAC.GetMyAddress() == RXNODEID1 || g_OMAC.GetMyAddress() == RXNODEID2) {
#ifdef def_Neighbor2beFollowed
		Neighbor2beFollowed = TXNODEID;
#endif
	}
	else {
#ifdef def_Neighbor2beFollowed2
		Neighbor2beFollowed1 = RXNODEID1;
		Neighbor2beFollowed2 = RXNODEID2;
#endif
	}
#elif defined(FAN_IN)
	if(g_OMAC.GetMyAddress() == TXNODEID1 || g_OMAC.GetMyAddress() == TXNODEID2) {
#ifdef def_Neighbor2beFollowed
		Neighbor2beFollowed = RXNODEID;
#endif
	}
	else {
#ifdef def_Neighbor2beFollowed2
		Neighbor2beFollowed1 = TXNODEID1;
		Neighbor2beFollowed2 = TXNODEID2;
#endif
	}
#endif

	memset(&tx_msg,0,sizeof(Message_15_4_t));
	//Initialize radio layer
	return DS_Success;
}

/*
 *
 */
BOOL OMACType::UnInitialize(){
	BOOL ret = TRUE;
	m_omac_scheduler.UnInitialize();
	ret &= (m_omac_RadioControl.Uninitialize() == DS_Success);
	ret &= (CPU_Radio_TurnOffRx(this->radioName) == DS_Success);
	ret &= CPU_Radio_UnInitialize(this->radioName);
	Initialized = FALSE;
	return ret;
}

BOOL OMACType::IsPcktValid(Message_15_4_t* msg, int Size){
/*	UINT16 maxPayload = OMACType::GetMaxPayload();
	if( Size > sizeof(IEEE802_15_4_Header_t) && (Size - sizeof(IEEE802_15_4_Header_t) > maxPayload) ){
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("OMAC Receive Error: Packet is too big: %d \r\n", Size+sizeof(IEEE802_15_4_Header_t));
#endif
		return FALSE;
	}
	if(Size < sizeof(IEEE802_15_4_Header_t)){
		return FALSE;
	}
	Size -= sizeof(IEEE802_15_4_Header_t);
	RadioAddress_t sourceID = msg->GetHeader()->src;
	RadioAddress_t destID = msg->GetHeader()->dest;
	if( sourceID == 0 || sourceID == MAX_UINT16
			||	destID == 0
			||  msg->GetHeader()->length > MAX_PCKT_SIZE
	){ //Check for incompliant packets
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
		return FALSE;
	}*/
	switch(msg->GetHeader()->payloadType){
		case MFM_CSMA_TIMESYNCREQ:
		case MFM_CSMA_NEIGHBORHOOD:
		case MFM_CSMA_ROUTING:
		case MFM_CSMA_DISCOVERY:
		case MFM_TIMESYNC:
		case MFM_CSMA_DATA_ACK:
		case MFM_OMAC_TIMESYNCREQ:
		case MFM_OMAC_NEIGHBORHOOD:
		case MFM_OMAC_ROUTING:
		case MFM_OMAC_DISCOVERY:
		case MFM_OMAC_DATA_ACK:
		case MFM_OMAC_DATA_BEACON_TYPE:
		case TYPE31:
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

/*
 *
 */
Message_15_4_t* ProcessSoftwareAck(Message_15_4_t* msg, int Size){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, TRUE);
#endif
		softwareACKHeader *swAckHeader = (softwareACKHeader*)msg;
		RadioAddress_t sourceID = swAckHeader->src;
		RadioAddress_t destID = swAckHeader->dest;
		UINT8 payloadType = swAckHeader->payloadType;
		RadioAddress_t myID = g_OMAC.GetMyAddress();
#if OMAC_DEBUG_PRINTF_ACKREC
		hal_printf("ACK Received sourceID = %u, destID = %u   \r\n", sourceID, destID);
		g_OMAC.is_print_neigh_table = true;
#endif
		if(destID == myID){
			if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK && payloadType == MFM_OMAC_DATA_ACK){
				g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.ReceiveDATAACK(sourceID);
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
				return msg;
			}
		}
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
		return msg;
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif

}

Message_15_4_t* OMACType::ReceiveHandler(Message_15_4_t* msg, int Size){
	if(!Initialized){
		return msg;
	}
#ifdef OMAC_DEBUG_GPIO
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
	CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif
	DeviceStatus ds;
	EntendedMACInfoMsgSummary* macinfosum_msg = NULL;
	DiscoveryMsg_t* disco_msg = NULL;
	TimeSyncMsg* tsmg = NULL;
	DataMsg_t* data_msg = NULL;
	TimeSyncRequestMsg* tsreqmg = NULL;

	INT64 evTime;
	UINT64 rx_start_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	UINT64 senderDelay = MAX_UINT64;
	UINT64 rx_time_stamp;
	UINT16 location_in_packet_payload = 0;

	MsgLinkQualityMetrics_t msgLinkQualityMetrics;
	RadioAddress_t myID = g_OMAC.GetMyAddress();

	//MS: Hack to consider anything less than header size as ack message
	if(Size == sizeof(softwareACKHeader) || (Size > sizeof(softwareACKHeader) && Size < sizeof(IEEE802_15_4_Header_t))){
		return ProcessSoftwareAck(msg,Size);
	}
	else {
		if(msg->GetHeader()->fcf.fcfWordValue != FCF_WORD_VALUE){
			return msg;
		}
	}

	UINT16 maxPayload = OMACType::GetMaxPayload();
	//if( Size > sizeof(IEEE802_15_4_Header_t) && (Size - sizeof(IEEE802_15_4_Header_t)-sizeof(IEEE802_15_4_Footer_t)-sizeof(IEEE802_15_4_Metadata) > maxPayload) ){
	if( Size > sizeof(IEEE802_15_4_Header_t) && (Size - sizeof(IEEE802_15_4_Header_t) > maxPayload) ){
		//OMAC_HAL_PRINTF("CSMA Receive Error: Packet is too big: %d \r\n", Size+sizeof(IEEE802_15_4_Header_t)+sizeof(IEEE802_15_4_Footer_t)+sizeof(IEEE802_15_4_Metadata));
#ifdef OMAC_DEBUG_PRINTF
		OMAC_HAL_PRINTF("OMAC Receive Error: Packet is too big: %d \r\n", Size+sizeof(IEEE802_15_4_Header_t));
#endif
		hal_printf("OMAC Receive Error: Packet is too big: %d \r\n", Size);
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif
		return msg;
	}
	if(Size < sizeof(IEEE802_15_4_Header_t)){
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
		CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif
		hal_printf("OMAC Receive Error: Packet is too small: %d \r\n", Size);
		return msg;
	}

	Size -= sizeof(IEEE802_15_4_Header_t);

	RadioAddress_t sourceID = msg->GetHeader()->src;
	RadioAddress_t destID = msg->GetHeader()->dest;
#ifdef	def_Neighbor2beFollowed
	if(sourceID == Neighbor2beFollowed) {
#endif
#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, TRUE);
#endif

		if( sourceID == 0 || sourceID == MAX_UINT16
				||	destID == 0
				||  msg->GetHeader()->length > MAX_PCKT_SIZE
		){ //Check for incompliant packets
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif
			hal_printf("Packet invalid1 \r\n");
			return msg;
		}

		if(!IsPcktValid(msg, Size)){
			hal_printf("Packet invalid2: %d \r\n", msg->GetHeader()->payloadType);
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
			CPU_GPIO_SetPinState( DATARECEPTION_SLOTPIN, !CPU_GPIO_GetPinState(DATARECEPTION_SLOTPIN) );
#endif
			return msg;
		}
		//g_OMAC.m_omac_scheduler.m_DataReceptionHandler.m_isreceiving = false;

		//if( destID == myID || destID == RADIO_BROADCAST_ADDRESS){

		
		//Any message might have timestamping attached to it. Check for it and process
		if(msg->GetHeader()->flags & TIMESTAMPED_FLAG){
			UINT64 temp = g_OMAC.TIME_RX_TIMESTAMP_OFFSET_MICRO;
			senderDelay = PacketTimeSync_15_4::SenderDelay(msg,Size) + g_OMAC.m_Clock.ConvertMicroSecstoTicks(temp);
			UINT64 test_tick_value;
			test_tick_value = (HAL_Time_CurrentTicks() - msg->GetMetaData()->GetReceiveTimeStamp());
			rx_time_stamp = g_OMAC.m_Clock.GetCurrentTimeinTicks() - g_OMAC.m_Clock.ConvertMicroSecstoTicks(CPU_TicksToMicroseconds(test_tick_value, SYSTEM_TIME));
			
			///rx_time_stamp = g_OMAC.m_Clock.GetCurrentTimeinTicks() - (HAL_Time_CurrentTicks() - msg->GetMetaData()->GetReceiveTimeStamp());
			Size = Size + TIMESTAMP_OFFSET;
		}

#if OMAC_DEBUG_PRINTF_PACKETREC
		hal_printf("OMAC RX sourceID = %u, destID = %u payloadType = %u flags = %u RSSI = %u/ \r\n", sourceID, destID, msg->GetHeader()->payloadType, msg->GetHeader()->flags, msg->GetMetaData()->GetRssi());
		g_OMAC.is_print_neigh_table = true;
#endif



		//Get the primary packet
		switch(msg->GetHeader()->payloadType){
		case MFM_OMAC_DISCOVERY:
		{
			disco_msg = (DiscoveryMsg_t*) (msg->GetPayload());
			if(Size < location_in_packet_payload + sizeof(DiscoveryMsg_t)) {//Check for incompliant packets
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
				return msg;
			}
			msgLinkQualityMetrics.RSSI = msg->GetMetaData()->GetRssi();
			//msgLinkQualityMetrics.LinkQuality = msg->GetMetaData()->GetLqi();
			g_OMAC.m_omac_scheduler.m_DiscoveryHandler.Receive(sourceID, disco_msg, &msgLinkQualityMetrics);
			location_in_packet_payload += sizeof(DiscoveryMsg_t);
			break;
		}
		case MFM_OMAC_ROUTING: //Not processed
		{
			data_msg = (DataMsg_t*) msg->GetPayload();
			//					if(data_msg->msg_identifier != 16843009){
			//						//ASSERT_SP(0);
			//					}
			location_in_packet_payload += data_msg->size + DataMsgOverhead;
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("OMACType::ReceiveHandler MFM_ROUTING\r\n");
#endif
			break;
		}
		case MFM_OMAC_NEIGHBORHOOD: //Not processed
		{
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("OMACType::ReceiveHandler MFM_NEIGHBORHOOD\r\n");
#endif
			break;
		}
		case MFM_OMAC_TIMESYNCREQ:
		{
			if(Size < location_in_packet_payload + DataMsgOverhead + sizeof(TimeSyncRequestMsg) ) {
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
				return msg;
			}
			//TODO: Commenting out below code for SI4468 radio. Needs to be re-visited.
			if(g_OMAC.m_omac_scheduler.m_execution_started && g_OMAC.m_omac_scheduler.m_state == (I_DATA_RCV_PENDING)){
				g_OMAC.m_omac_scheduler.m_DataReceptionHandler.HandleEndofReception(sourceID);
			}
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(OMAC_TIMESYNCREQRXPIN, TRUE);
#endif
			//ASSERT_SP(msg->GetHeader()->flags & TIMESTAMPED_FLAG);
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("OMACType::ReceiveHandler MFM_TIMESYNC \r\n");
#endif
			data_msg = (DataMsg_t*) msg->GetPayload();
			location_in_packet_payload += data_msg->size + DataMsgOverhead;
			//					if(data_msg->msg_identifier != 16843009){
			//						//ASSERT_SP(0);
			//					}
			if(myID == destID) {
				tsreqmg =  (TimeSyncRequestMsg*) (data_msg->payload);
				ds = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.ReceiveTSReq(sourceID, tsreqmg);
			}
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(OMAC_TIMESYNCREQRXPIN, FALSE);
#endif
			break;
		}
		case MFM_OMAC_DATA_BEACON_TYPE: //Not processed
		{
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("OMACType::ReceiveHandler MFM_OMAC_DATA_BEACON_TYPE \r\n");
			OMAC_HAL_PRINTF("Got a data beacon packet \r\n");
#endif
			break;
		}
		case MFM_OMAC_DATA_ACK:
		{
			if(CPU_Radio_GetRadioAckType() == SOFTWARE_ACK){
				g_OMAC.m_omac_scheduler.m_DataTransmissionHandler.ReceiveDATAACK(sourceID);
				location_in_packet_payload += 1;
			}
			break;
		}
		case MFM_DATA:
		default:
		{
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(OMAC_DATARXPIN, TRUE);
#endif


			if(Size < location_in_packet_payload + DataMsgOverhead ) {//Check for incompliant packets
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
				return msg;
			}


			if(myID == destID && g_OMAC.m_omac_scheduler.m_state == (I_DATA_RCV_PENDING) && g_OMAC.m_omac_scheduler.m_execution_started){
				g_OMAC.m_omac_scheduler.m_DataReceptionHandler.HandleEndofReception(sourceID);
			}

			data_msg = (DataMsg_t*) msg->GetPayload();
			location_in_packet_payload += data_msg->size + DataMsgOverhead;


			if( data_msg->size > msg->GetHeader()->length){ //Check for incompliant packets
#ifdef OMAC_DEBUG_GPIO
				CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
				return msg;
			}

			//if(data_msg->msg_identifier != 16843009){
			//ASSERT_SP(0);
			//}

			ASSERT_SP(data_msg->size <= MAX_DATA_PCKT_SIZE);



			static volatile INT32 pt = msg->GetHeader()->payloadType;

			if(myID == destID && g_OMAC.m_omac_scheduler.m_state == (I_DATA_RCV_PENDING) && g_OMAC.m_omac_scheduler.m_execution_started) {//Process the data packet only if the destination matches

				Message_15_4_t* next_free_buffer = g_receive_buffer.GetNextFreeBuffer();
				if(! (next_free_buffer))
				{
					g_receive_buffer.DropOldest(1);
					next_free_buffer = g_receive_buffer.GetNextFreeBuffer();
				}
				ASSERT_SP(next_free_buffer);

				memcpy(next_free_buffer->GetPayload(),data_msg->payload,data_msg->size);
				memcpy(next_free_buffer->GetHeader(),msg->GetHeader(), sizeof(IEEE802_15_4_Header_t));
				memcpy(next_free_buffer->GetFooter(),msg->GetFooter(), sizeof(IEEE802_15_4_Footer_t));
				memcpy(next_free_buffer->GetMetaData(),msg->GetMetaData(), sizeof(IEEE802_15_4_Metadata_t));
				next_free_buffer->GetHeader()->length = data_msg->size + sizeof(IEEE802_15_4_Header_t);

				if(NEED_OMAC_CALLBACK_CONTINUATION){
					payloadTypeArray[payloadTypeArrayIndex % payloadTypeArrayMaxValue] = msg->GetHeader()->payloadType;
					payloadTypeArrayIndex++;
					OMAC_callback_continuation.Enqueue();
				}
				else{
					SendRXPacketToUpperLayers(next_free_buffer, msg->GetHeader()->payloadType);
				}

			}
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(OMAC_DATARXPIN, FALSE);
#endif
			break;
		}
		};

		if(msg->GetHeader()->flags & MFM_TIMESYNC_FLAG) {
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(DATARX_TIMESTAMP_PIN, TRUE);
#endif
			//ASSERT_SP(msg->GetHeader()->GetFlags() & TIMESTAMPED_FLAG);
			tsmg = (TimeSyncMsg*) (msg->GetPayload() + location_in_packet_payload);
			ASSERT_SP(senderDelay != MAX_UINT64);
			//hal_printf("\r senderDelay=%s\r\n", l2s(senderDelay,0));
			ds = g_OMAC.m_omac_scheduler.m_TimeSyncHandler.Receive(sourceID, tsmg, senderDelay, rx_time_stamp );
			location_in_packet_payload += sizeof(TimeSyncMsg);
#ifdef OMAC_DEBUG_GPIO
			CPU_GPIO_SetPinState(DATARX_TIMESTAMP_PIN, FALSE);
#endif
		}
		if(msg->GetHeader()->flags &  MFM_DISCOVERY_FLAG) {
			disco_msg = (DiscoveryMsg_t*) (msg->GetPayload() + location_in_packet_payload);
			msgLinkQualityMetrics.RSSI = msg->GetMetaData()->GetRssi();
		//	msgLinkQualityMetrics.LinkQuality = msg->GetMetaData()->GetLqi();
			g_OMAC.m_omac_scheduler.m_DiscoveryHandler.Receive(sourceID, disco_msg, &msgLinkQualityMetrics);
			location_in_packet_payload += sizeof(DiscoveryMsg_t);
		}

#if OMAC_DEBUG_PRINTF_EXTENDEDMACINfo
		if(msg->GetHeader()->flags &  MFM_EXTENDED_MAC_INFO_FLAG) {
			macinfosum_msg = (EntendedMACInfoMsgSummary*) (msg->GetPayload() + location_in_packet_payload);
			UINT8 numNfits = macinfosum_msg->NumEntriesInMsg;

			hal_printf("   EntendedMACInfoMsgSummary NTE=%u, NAFUL=%u, NEntInMsg=%u \r\n"
					, macinfosum_msg->NumTotalEntries, macinfosum_msg->NNeigh_AFUL, numNfits);


			location_in_packet_payload += sizeof(EntendedMACInfoMsgSummary);


			if(numNfits > 0 && location_in_packet_payload + (numNfits) * sizeof(MACNeighborInfo) + numNfits * sizeof(MACNeighborLinkInfo) <= Size ){
				MACNeighborInfo * macinfo_msg;
				MACNeighborLinkInfo* macinfo_msg2;
				for(UINT8 i=0; i < numNfits ; ++i){
					macinfo_msg = (MACNeighborInfo*) (msg->GetPayload() + location_in_packet_payload);
					macinfo_msg2 = (MACNeighborLinkInfo*) (msg->GetPayload() + location_in_packet_payload + (numNfits-i) * sizeof(MACNeighborInfo) + i * sizeof(MACNeighborLinkInfo));
					hal_printf("   EMI MAC=%u, S=%u, A=%u, SN=%u, NTSS=%u, NTSR=%u, SLR=%c%c%c%c%c%c%c%c, RLRSSI = %u\r\n"
							, macinfo_msg->MACAddress
							, macinfo_msg->neighborStatus
							, macinfo_msg->IsAvailableForUpperLayers
							, macinfo_msg->IsMyScheduleKnown
							, macinfo_msg->NumInitializationMessagesSent
							, macinfo_msg->NumTimeSyncMessagesRecv
							, UINT_TO_BINARY(macinfo_msg2->SendLink.Link_reliability_bitmap)
							, macinfo_msg2->ReceiveLink.AvgRSSI
					);
					location_in_packet_payload += sizeof(MACNeighborInfo);
				}
			}
			else if(numNfits > 0 && location_in_packet_payload + (numNfits) * sizeof(MACNeighborInfo) <= Size ){
				MACNeighborInfo * macinfo_msg;
				MACNeighborLinkInfo* macinfo_msg2;
				for(UINT8 i=0; i < numNfits ; ++i){
					macinfo_msg = (MACNeighborInfo*) (msg->GetPayload() + location_in_packet_payload);
					macinfo_msg2 = (MACNeighborLinkInfo*) (msg->GetPayload() + location_in_packet_payload + (numNfits-i) * sizeof(MACNeighborInfo) + i * sizeof(MACNeighborLinkInfo));
					hal_printf("   EMI MAC=%u, S=%u, A=%u, SN=%u, NTSS=%u, NTSR=%u\r\n"
							, macinfo_msg->MACAddress
							, macinfo_msg->neighborStatus
							, macinfo_msg->IsAvailableForUpperLayers
							, macinfo_msg->IsMyScheduleKnown
							, macinfo_msg->NumInitializationMessagesSent
							, macinfo_msg->NumTimeSyncMessagesRecv
					);
					location_in_packet_payload += sizeof(MACNeighborInfo);
				}
			}


		}
#endif
		//			if( tsmg != NULL && disco_msg == NULL){
		//				rx_start_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
		//			}

		ds = g_NeighborTable.RecordLastHeardTime(sourceID,g_OMAC.m_Clock.GetCurrentTimeinTicks());
		if(ds != DS_Success){
#ifdef OMAC_DEBUG_PRINTF
			OMAC_HAL_PRINTF("OMACType::ReceiveHandler RecordLastHeardTime failure; address: %d; line: %d \r\n", sourceID, __LINE__);
#endif
		}

		if(msg->GetHeader()->payloadType != MFM_OMAC_DATA_ACK && msg->GetHeader()->payloadType != MFM_OMAC_DISCOVERY){
			if(msg->GetHeader()->flags & TIMESTAMPED_FLAG){
				//					double SenderDelayTemp = (((double) g_OMAC.m_Clock.ConvertTickstoMicroSecs(senderDelay) / 100000) ); // Recorded delay has units of second
				UINT64 SenderDelayTemp = senderDelay>>19; // Lets use units of 65536 microsecs (instead of 100000 microsecs above) for the delay measurement. This reduces the taxation on the CPU
				if(SenderDelayTemp > 0x000000FF){
					g_NeighborTable.RecordSenderDelayIncoming( sourceID, (UINT8)(0xFF));
				}
				else{
					g_NeighborTable.RecordSenderDelayIncoming( sourceID, (UINT8)(SenderDelayTemp));
				}
			}
		}


		//}

#ifdef OMAC_DEBUG_GPIO
		CPU_GPIO_SetPinState(OMAC_RXPIN, FALSE);
#endif
#ifdef	def_Neighbor2beFollowed
	}
#endif
	//	UINT64 rx_end_ticks = g_OMAC.m_Clock.GetCurrentTimeinTicks();
	//	if(rx_end_ticks - rx_start_ticks > 8*2000000){ //Dummy if conditions to catch interrupted reception
	//		return msg;
	//	}



	return msg;
}

/*
 *
 */
void RadioInterruptHandler(RadioInterrupt Interrupt, void* Param){

}


PacketID_T OMACType::EnqueueToSend(UINT16 address, UINT8 dataType, void* msg, int size){
	if(size + timestamp_size < OMACType::GetMaxPayload() ){
		return EnqueueToSendTimeStamped(address, dataType,  msg, size, g_OMAC.m_Clock.GetCurrentTimeinTicks());
	}
	else{
		PacketID_T rv = INVALID_PACKET_ID;
		if(!Initialized){
	#if OMAC_DEBUG_PACKET_REJECTION
			hal_printf("OMACType::SendTimeStamped Pckt Reject Initialized destID = %u dataType = %u  \r\n", address, dataType);
	#endif
			return rv;
		}
		Message_15_4_t* msg_carrier = PrepareMessageBuffer(address, dataType, msg, size);
		if(msg_carrier == (Message_15_4_t*)(NULL)){
	#if OMAC_DEBUG_PACKET_REJECTION
			hal_printf("OMACType::SendTimeStamped Pckt Reject destID = %u dataType = %u  \r\n", address, dataType);
	#endif
			return rv;
		}
		IEEE802_15_4_Header_t* header = msg_carrier->GetHeader();
		IEEE802_15_4_Metadata* metadata = msg_carrier->GetMetaData();
		metadata->SetReceiveTimeStamp(g_OMAC.m_Clock.GetCurrentTimeinTicks());

		rv = g_NeighborTable.InsertMessageGetIndex(msg_carrier);
		if(!ISPACKET_ID_VALID(rv)) {
	#if OMAC_DEBUG_PACKET_REJECTION
			hal_printf("OMACType::EnqueueToSend Pckt Reject buffer full destID = %u dataType = %u buffersize = %u \r\n", address, dataType, g_NeighborTable.send_buffer.GetNumberofElements());
	#endif
		}
		return rv;
	}
}


PacketID_T OMACType::EnqueueToSendTimeStamped(UINT16 address, UINT8 dataType, void* msg, int size, UINT32 eventTime){
	PacketID_T rv = INVALID_PACKET_ID;
	if(!Initialized){
#if OMAC_DEBUG_PACKET_REJECTION
		hal_printf("OMACType::SendTimeStamped Pckt Reject Initialized destID = %u dataType = %u  \r\n", address, dataType);
#endif
		return rv;
	}
	Message_15_4_t* msg_carrier = PrepareMessageBuffer(address, dataType, msg, size);
	if(msg_carrier == (Message_15_4_t*)(NULL)){
#if OMAC_DEBUG_PACKET_REJECTION
		hal_printf("OMACType::SendTimeStamped Pckt Reject destID = %u dataType = %u  \r\n", address, dataType);
#endif
		return rv;
	}
	IEEE802_15_4_Header_t* header = msg_carrier->GetHeader();
	IEEE802_15_4_Metadata* metadata = msg_carrier->GetMetaData();
	metadata->SetReceiveTimeStamp(eventTime);
	header->flags = (TIMESTAMPED_FLAG);
	//return true;
	rv = g_NeighborTable.InsertMessageGetIndex(msg_carrier);
	if(!ISPACKET_ID_VALID(rv)) {
#if OMAC_DEBUG_PACKET_REJECTION
		hal_printf("OMACType::EnqueueToSendTimeStamped3 Pckt Reject buffer full destID = %u dataType = %u buffersize = %u \r\n", address, dataType, g_NeighborTable.send_buffer.GetNumberofElements());
#endif
	}
	return rv;
}

/*
 * Store packet in the send buffer and return; Scheduler will pick it up later and send it
 */
BOOL OMACType::Send(UINT16 address, UINT8 dataType, void* msg, int size){
	if(size + timestamp_size < OMACType::GetMaxPayload() ){
		return SendTimeStamped(address, dataType,  msg, size, g_OMAC.m_Clock.GetCurrentTimeinTicks());
	}
	else{
//		return false;
			if(!Initialized){
		#if OMAC_DEBUG_PACKET_REJECTION
				hal_printf("OMACType::Send Pckt Reject Initialized destID = %u dataType = %u  \r\n", address, dataType);
		#endif
				return false;
			}
			Message_15_4_t* msg_carrier = PrepareMessageBuffer(address, dataType, msg, size);
			if(msg_carrier == (Message_15_4_t*)(NULL)){
		#if OMAC_DEBUG_PACKET_REJECTION
				hal_printf("OMACType::Send Pckt Reject destID = %u dataType = %u  \r\n", address, dataType);
		#endif
				return false;
			}
			IEEE802_15_4_Header_t* header = msg_carrier->GetHeader();
			IEEE802_15_4_Metadata* metadata = msg_carrier->GetMetaData();
			header->flags = (0);
			metadata->SetReceiveTimeStamp(g_OMAC.m_Clock.GetCurrentTimeinTicks());
			bool rv = g_NeighborTable.InsertMessage(msg_carrier);
			if(rv == false) {
		#if OMAC_DEBUG_PACKET_REJECTION
				hal_printf("OMACType::SendTimeStamped Pckt Reject buffer full destID = %u dataType = %u  \r\n", address, dataType);
		#endif
			}
			return rv;
	}
}


DeviceStatus OMACType::GetPacketWithIndex(UINT8 **managedBuffer, UINT8 buffersize, PacketID_T index){
	Message_15_4_t* msg_carrier = g_NeighborTable.GetPacketPtrWithIndex(index);
	if(msg_carrier){
		DataMsg_t* data_msg = (DataMsg_t*)msg_carrier->GetPayload();
		UINT8* payload = data_msg->payload;
		UINT8 size = data_msg->size;
		if(size != buffersize) {
			return DS_Fail;
		}
		else{
			for(UINT8 i = 0 ; i < size; i++){
				(*managedBuffer)[i] = payload[i];
			}
			return DS_Success;
		}
	}
	else{
		return DS_Fail;
	}
}

DeviceStatus OMACType::GetPacketSizeWithIndex(UINT8* buffersizeptr, PacketID_T index){
	Message_15_4_t* msg_carrier = g_NeighborTable.GetPacketPtrWithIndex(index);
	if(msg_carrier){
		DataMsg_t* data_msg = (DataMsg_t*)msg_carrier->GetPayload();
		UINT8* payload = data_msg->payload;
		UINT8 size = data_msg->size;
		*buffersizeptr = size;
		return DS_Success;
	}
	else{
		return DS_Fail;
	}
}

DeviceStatus OMACType::DeletePacketWithIndexInternal(PacketID_T index){
	Message_15_4_t* msg_carrier = g_NeighborTable.GetPacketPtrWithIndex(index);
	if(msg_carrier){
		if(g_NeighborTable.DeletePacket(msg_carrier)){
			return DS_Success;
		}
	}
	return DS_Fail;
}


/*
 * Store packet in the send buffer and return; Scheduler will pick it up later and send it
 */
////BOOL OMACType::SendTimeStamped(RadioAddress_t address, UINT8 dataType, Message_15_4_t* msg, int size, UINT32 eventTime)
BOOL OMACType::SendTimeStamped(UINT16 address, UINT8 dataType, void* msg, int size, UINT32 eventTime){
	if(!Initialized){
#if OMAC_DEBUG_PACKET_REJECTION
		hal_printf("OMACType::SendTimeStamped Pckt Reject Initialized destID = %u dataType = %u  \r\n", address, dataType);
#endif
		return false;
	}
	Message_15_4_t* msg_carrier = PrepareMessageBuffer(address, dataType, msg, size);
	if(msg_carrier == (Message_15_4_t*)(NULL)){
#if OMAC_DEBUG_PACKET_REJECTION
		hal_printf("OMACType::SendTimeStamped Pckt Reject destID = %u dataType = %u  \r\n", address, dataType);
#endif
		return false;
	}
	IEEE802_15_4_Header_t* header = msg_carrier->GetHeader();
	IEEE802_15_4_Metadata* metadata = msg_carrier->GetMetaData();
	metadata->SetReceiveTimeStamp(eventTime);
	header->flags = (TIMESTAMPED_FLAG);
	//return true;
	bool rv = g_NeighborTable.InsertMessage(msg_carrier);
	if(rv == false) {
#if OMAC_DEBUG_PACKET_REJECTION
		hal_printf("OMACType::SendTimeStamped Pckt Reject buffer full destID = %u dataType = %u  \r\n", address, dataType);
#endif
	}
	return rv;
}

/**
 * @return NULL pointer when no buffer available.
 */
Message_15_4_t* OMACType::PrepareMessageBuffer(UINT16 address, UINT8 dataType, void* msg, int size){
	static UINT8 seqNumber = 0;
	UINT8 finalSeqNumber = 0;
	Message_15_4_t* msg_carrier = (Message_15_4_t*)(NULL);

	if(size > OMACType::GetMaxPayload()){
		DEBUG_OMAC_PMB_PRINTF("OMACType Send Error: Packet is too big: %d ", size);
		return (Message_15_4_t*)(NULL);
	}

	Neighbor_t* neighborEntry = g_NeighborTable.GetNeighborPtr(address);
	if(neighborEntry == NULL) {
		DEBUG_OMAC_PMB_PRINTF("OMACType Send Error: Destination does not exist in neighbor table %d  \r\n", address);
		return (Message_15_4_t*)(NULL);
	}
	else if(neighborEntry->neighborStatus == Dead) {
		DEBUG_OMAC_PMB_PRINTF("OMACType Send Error: Destination exists in neighbor table but its status is dead: %d  \r\n", address);
		return (Message_15_4_t*)(NULL);
	}

	if(dataType == MFM_OMAC_TIMESYNCREQ){

		msg_carrier = &tx_msg;
		ClearMsgContents(msg_carrier);

		if(msg_carrier == (Message_15_4_t*)(NULL)){
			DEBUG_OMAC_PMB_PRINTF("ERROR: OMACType::PrepareMessageBuffer no free tsr_send_buffer available. Addr: %d. \r\n", neighborEntry->MACAddress);
			return (Message_15_4_t*)(NULL);
		}
	}
	else{
		msg_carrier = &tx_msg;
		ClearMsgContents(msg_carrier);

		if(msg_carrier == (Message_15_4_t*)(NULL)){
			DEBUG_OMAC_PMB_PRINTF("ERROR: OMACType::PrepareMessageBuffer no free send_buffer available. Addr: %d. \r\n", neighborEntry->MACAddress);
			return (Message_15_4_t*)(NULL);
		}
	}

	IEEE802_15_4_Header_t* header = msg_carrier->GetHeader();
	/****** Taking the word value of below bits gives FCF_WORD_VALUE *******/
	/*header->fcf->IEEE802_15_4_Header_FCF_BitValue.frameType = FRAME_TYPE_MAC;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.securityEnabled = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.framePending = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.ackRequired = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.intraPAN = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.reserved = 0;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.destAddrMode = 2;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.frameVersion = 1;
	header->fcf->IEEE802_15_4_Header_FCF_BitValue.srcAddrMode = 2;*/
	/**************************************************************/
	header->fcf.fcfWordValue = FCF_WORD_VALUE;
	finalSeqNumber = GetMyAddress() ^ 0xAA;
	finalSeqNumber += ((GetMyAddress() >> 8) ^ 0x55);
	finalSeqNumber += seqNumber;
	//Make sure that we differentiate between a DISCO and DATA sequence number
	if(finalSeqNumber == OMAC_DISCO_SEQ_NUMBER){
		finalSeqNumber += 1;
	}
	//header->dsn = finalSeqNumber;
	//header->dsn = 97;
	//header->srcpan = SRC_PAN_ID;
	//header->destpan = DEST_PAN_ID;
	/*if(GetRadioAddress() == 6846){
		header->dest = 0x0DB1;
	}
	else{
		header->dest = 0x1ABE;
	}*/
	/*//6846(0x1ABE) is the receiver
	if(g_OMAC.GetMyAddress() != 0x1ABE){
		header->dest = 0x1ABE;
	}
	else{
		header->dest = 0x1111;
	}*/
	header->dest = address;
	header->src = GetMyAddress();
	//header->retryAttempt = 0;
	seqNumber++;

	IEEE802_15_4_Metadata* metadata = msg_carrier->GetMetaData();

	metadata->ClearData();
	//header->SetLength(size + sizeof(IEEE802_15_4_Header_t) + sizeof(IEEE802_15_4_Footer_t)+sizeof(IEEE802_15_4_Metadata));
	//metadata->SetNetwork(MyConfig.Network);
	//metadata->SetNetwork(0);
	header->macName = (macName);
	header->payloadType = (dataType);
	metadata->SetReceiveTimeStamp((UINT32)0);

	DataMsg_t* data_msg = (DataMsg_t*)msg_carrier->GetPayload();
	data_msg->size = size;
	//data_msg->msg_identifier = 16843009; // 0x01010101
	UINT8* payload = data_msg->payload;
	UINT8* lmsg = (UINT8*) msg;
	for(UINT8 i = 0 ; i < size; i++){
		payload[i] = lmsg[i];
	}
	//msg_carrier->GetMetaData()->SetReceiveTimeStamp(0);

	//header->SetLength(size + sizeof(UINT32) + sizeof(UINT8) + sizeof(IEEE802_15_4_Header_t) + sizeof(IEEE802_15_4_Footer_t)+sizeof(IEEE802_15_4_Metadata));
	//header->length = (size + sizeof(UINT32) + sizeof(UINT8) + sizeof(IEEE802_15_4_Header_t));
	header->length = (size + DataMsgOverhead + sizeof(IEEE802_15_4_Header_t));


	return msg_carrier;
}


/*
 *
 */
Message_15_4_t* OMACType::FindFirstSyncedNbrMessage(){
	return NULL;
}

void OMACType::MarkNeighborAsDead(UINT8 tableIndex){
	Message_15_4_t* msg_carrier = NULL;
	g_NeighborTable.Neighbor[tableIndex].MarkAsDead();

#if OMAC_DEBUG_PRINTF_NEIGHCHANGE
						is_print_neigh_table = true;
#endif

	//Return Packets for neighbors
	msg_carrier = g_NeighborTable.FindDataPacketForNeighbor(g_NeighborTable.Neighbor[tableIndex].MACAddress);
	while(msg_carrier){
		m_omac_scheduler.m_DataTransmissionHandler.SendACKToUpperLayers(msg_carrier, sizeof(Message_15_4_t), NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
		msg_carrier = g_NeighborTable.FindDataPacketForNeighbor(g_NeighborTable.Neighbor[tableIndex].MACAddress);
	}
	msg_carrier = g_NeighborTable.FindTSRPacketForNeighbor(g_NeighborTable.Neighbor[tableIndex].MACAddress);
	while(msg_carrier){
		m_omac_scheduler.m_DataTransmissionHandler.SendACKToUpperLayers(msg_carrier, sizeof(Message_15_4_t), NetworkOperations_Fail, TRAC_STATUS_FAIL_TO_SEND);
		msg_carrier = g_NeighborTable.FindTSRPacketForNeighbor(g_NeighborTable.Neighbor[tableIndex].MACAddress);
	}
	//						g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.Clean(g_NeighborTable.Neighbor[tableIndex].MACAddress); //  BK: New logic enables deleting by the time sync table. We don't need to delete here.
}

/*
 *
 */
UINT8 OMACType::UpdateNeighborTable(){
	Neighbor_t* neighbor_ptr = NULL;
	Message_15_4_t* msg_carrier = NULL;
	UINT8 numberOfDeadNeighbors = 0, numberofNeighbors = 0;
	UINT64 currentTime =  m_Clock.GetCurrentTimeinTicks();
	UINT64 livelinessDelayInTicks = m_Clock.ConvertMicroSecstoTicks((UINT64)(MyConfig.NeighborLivenessDelay * 1000000));


	//Determine is available for upper layers
	//Det

	for (UINT8 tableIndex=0; tableIndex<MAX_NEIGHBORS; ++tableIndex){
		if( ISMAC_VALID(   g_NeighborTable.Neighbor[tableIndex].MACAddress) ){
			if(g_NeighborTable.Neighbor[tableIndex].neighborStatus == Alive) {
				if(g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.IsNeighborTimeAvailable(g_NeighborTable.Neighbor[tableIndex].MACAddress)){
					if( !(g_NeighborTable.Neighbor[tableIndex].IsInitializationTimeSamplesNeeded()) ){
						g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers = true;
					}
					else{
						g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers = false;
					}
					if(g_NeighborTable.Neighbor[tableIndex].LastHeardTime != 0 && currentTime - g_NeighborTable.Neighbor[tableIndex].LastHeardTime > livelinessDelayInTicks ){
						++numberOfDeadNeighbors;
						MarkNeighborAsDead(tableIndex);
					}
//					else if(!g_NeighborTable.Neighbor[tableIndex].SendLink.IsReliable()){
//						++numberOfDeadNeighbors;
//						g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers = false;
//					}
				}
				else{ //If IsNeighborTime NOT Available
					g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers = false;
					if(g_NeighborTable.Neighbor[tableIndex].LastHeardTime == 0){ //Should not happen since in order to exist in the table we should heard about it
						hal_printf("OMAC: Update Neighbor WARNIING Code: 001");
					}
					else if(currentTime - g_NeighborTable.Neighbor[tableIndex].LastHeardTime > g_OMAC.m_Clock.ConvertMicroSecstoTicks(OMAC_MAX_WAITING_TIME_FOR_TIMESAMPLES_INMICS) ){ //we have waited long enough to get time and we should delete it to clear space for other neighbors
						++numberOfDeadNeighbors;
						MarkNeighborAsDead(tableIndex);
					}
					//					else{ //We are still waiting for time samples to accumulate to give reliable time estimate keep it in the table
					//
					//					}
				}
			}
			else{ //Neighbor is DEAD don't do anything
				g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers = false; // BK: This should not be exercised but adding for completeness. A neighbor that is marked as Dead should also clear the flag for to be exposed to upper layers. However interrupt check both flags and even if we don't declare this as false it should still not be exposed.
			}
			if( g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers){
				++numberofNeighbors;
			}
		}
		else{ //Not valid MAC don't do anything
			g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers = false; //BK: CLear the is available for upper layers just in case.
		}
	}



	if (g_NeighborTable.PreviousNumberOfNeighbors() != numberofNeighbors)
	{

#if OMAC_DEBUG_PRINTF_NEIGHCHANGE
		is_print_neigh_table = true;
#endif
		NeighborChangeFuncPtrType appHandler = g_OMAC.GetAppHandler(CurrentActiveApp)->neighborHandler;

#if OMAC_DEBUG_PRINTF_NEIGHCHANGE
		if(is_print_neigh_table){
			hal_printf("New NeighborTable nn=%u Pnn=%u NumMsg = %u \r\n", numberofNeighbors, g_NeighborTable.PreviousNumberOfNeighbors(), g_NeighborTable.send_buffer.GetNumberofElements() );
			PrintNeighborTable();
		}
#endif

		// Check if neighbor change has been registered and the user is interested in this information
		if(appHandler != NULL)
		{
			// Make the neighbor changed callback signaling dead neighbors
			(*appHandler)((INT16) (numberofNeighbors));
		}
		g_NeighborTable.SetPreviousNumberOfNeighbors(numberofNeighbors);
	}
#if OMAC_DEBUG_PRINTF_NEIGHCHANGE || OMAC_DEBUG_PRINTF_DISCO_RX || OMAC_DEBUG_PRINTF_TS_RX ||OMAC_DEBUG_PRINTF_TSREQ_TX
	else{
		if(is_print_neigh_table){
			///// hal_printf("Re-Printing NeighborTable nn=%u Pnn=%u NumMsg = %u \r\n", numberofNeighbors, g_NeighborTable.PreviousNumberOfNeighbors(), g_NeighborTable.send_buffer.GetNumberofElements() );
			///// PrintNeighborTable();
		}
	}
#endif

	return numberOfDeadNeighbors;
	//g_NeighborTable.DegradeLinks();
}

void OMACType::PrintNeighborTable(){
	//	hal_printf("--NeighborTable-- \r\n", numberofNeighbors, g_NeighborTable.PreviousNumberOfNeighbors() );
	for (UINT8 tableIndex=0; tableIndex<MAX_NEIGHBORS; ++tableIndex){
		if(    g_NeighborTable.Neighbor[tableIndex].MACAddress != 0 && g_NeighborTable.Neighbor[tableIndex].MACAddress != 65535 ){

			/*hal_printf("  MAC=%u, S=%u, A=%u, SN=%u, NTSS=%u, NTSR=%u, DataPcktInQ = %u, TSRPcktInQ = %u, SLR=%c%c%c%c%c%c%c%c, RLRSSI = %u, LHT = %llu, CT = %llu \r\n "
					, g_NeighborTable.Neighbor[tableIndex].MACAddress
					, g_NeighborTable.Neighbor[tableIndex].neighborStatus
					, g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers
					, g_NeighborTable.Neighbor[tableIndex].IsMyScheduleKnown
					, g_NeighborTable.Neighbor[tableIndex].NumInitializationMessagesSent
					, g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[tableIndex].MACAddress)
					, g_NeighborTable.IsThereADataPacketWithDest(g_NeighborTable.Neighbor[tableIndex].MACAddress)
					, g_NeighborTable.IsThereATSRPacketWithDest(g_NeighborTable.Neighbor[tableIndex].MACAddress)
					, UINT_TO_BINARY(g_NeighborTable.Neighbor[tableIndex].SendLink.Link_reliability_bitmap)
					, g_NeighborTable.Neighbor[tableIndex].ReceiveLink.AvgRSSI
					, g_NeighborTable.Neighbor[tableIndex].LastHeardTime
					, g_OMAC.m_Clock.GetCurrentTimeinTicks()
			);*/
			hal_printf("  MAC=%u, S=%u, A=%u, SN=%u, NTSS=%u, NTSR=%u, DataPcktInQ = %u, TSRPcktInQ = %u, SLR=%c%c%c%c%c%c%c%c, RLRSSI = %u, LHT = %s, CT = %s \r\n "
					, g_NeighborTable.Neighbor[tableIndex].MACAddress
					, g_NeighborTable.Neighbor[tableIndex].neighborStatus
					, g_NeighborTable.Neighbor[tableIndex].IsAvailableForUpperLayers
					, g_NeighborTable.Neighbor[tableIndex].IsMyScheduleKnown
					, g_NeighborTable.Neighbor[tableIndex].NumInitializationMessagesSent
					, g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[tableIndex].MACAddress)
					, g_NeighborTable.IsThereADataPacketWithDest(g_NeighborTable.Neighbor[tableIndex].MACAddress)
					, g_NeighborTable.IsThereATSRPacketWithDest(g_NeighborTable.Neighbor[tableIndex].MACAddress)
					, UINT_TO_BINARY(g_NeighborTable.Neighbor[tableIndex].SendLink.Link_reliability_bitmap)
					, g_NeighborTable.Neighbor[tableIndex].ReceiveLink.AvgRSSI
					, l2s(g_NeighborTable.Neighbor[tableIndex].LastHeardTime,0)
					, l2s(g_OMAC.m_Clock.GetCurrentTimeinTicks(),0)
			);
		}
	}


#if OMAC_DEBUG_PRINTF_NEIGHCHANGE || OMAC_DEBUG_PRINTF_DISCO_RX || OMAC_DEBUG_PRINTF_TS_RX ||OMAC_DEBUG_PRINTF_TSREQ_TX
	is_print_neigh_table = false;
#endif
	//	hal_printf("--End NeighborTable--\r\n", numberofNeighbors, g_NeighborTable.PreviousNumberOfNeighbors() );
}


/*BOOL OMACType::SetAddress(UINT16 address){
	MyAddress = address;
	return TRUE;
}
 */

UINT8 OMACType::GetSendBufferSize(){
	return(0);
}

UINT8 OMACType::GetReceiveBufferSize(){
	return g_receive_buffer.Size();
}

UINT16 OMACType::GetSendPending(){
	UINT16 n_msg = 0;
	return n_msg;
}

UINT16 OMACType::GetReceivePending(){
	return g_receive_buffer.GetNumberMessagesInBuffer();
}
