#include "LinkTest.h"
//#include "platform_selector.h"

#define PKT_PERIOD_MICRO 10000000 // 10 secs
#define TESTRADIONAME SX1276
//#define TESTRADIONAME RF231RADIO

#define LinkTest_PRINT_RX_PACKET_INFO 1


UINT16 testDest=0;
LinkTest gLinkTest(0,0);
extern UINT8 currentMacName;

void PrintHex(char *x, int size){
	for (int j=0;j<size; j++){
		hal_printf("0x%.2X , ",x[j]);
	}
	hal_printf("\r\n");
}


LinkTest::LinkTest( int seedValue, int numberOfEvents ){

}



BOOL LinkTest::Execute( int testLevel )
{
	Initialize();
	//VirtTimer_Start(LocalClockMonitor_TIMER1);
} //Execute


void LinkTest_ReceiveHandler (void* msg, UINT16 PacketType)
{
	UINT8 index;

	Message_15_4_t* packet_ptr = static_cast<Message_15_4_t*>(msg);
	UINT16 srcID= packet_ptr->GetHeader()->src;

	UINT64 packetID;
	memcpy(&packetID,packet_ptr->GetPayload(),sizeof(UINT64));
	if(g_NeighborTable.FindIndex(srcID, &index) == DS_Success)
	{
		++gLinkTest.rx_packet_count[index];
		#if LinkTest_PRINT_RX_PACKET_INFO
			hal_printf("\r\nLinkTest_RX: rcd pkt size %u, rx_packet_count = %u ", packet_ptr->GetHeader()->length, gLinkTest.rx_packet_count[index]);
			hal_printf("src = %u, PacketID = %llu \r\n", srcID, packetID );
		#endif
	}else {
		#if LinkTest_PRINT_RX_PACKET_INFO
			hal_printf("\r\nLinkTest_RX: rcd pkt size %u, from unkown neighbor ", packet_ptr->GetHeader()->length);
			hal_printf("ID = %u, PacketID = %llu \r\n", srcID, packetID );
		#endif
	}
}



void LinkTest_NeighborChangeHandler (INT16 args){
	hal_printf("\r\nNeighbor Change Notification for %u neighbors!\r\n", args);

	VirtTimer_Start(LocalClockMonitor_TIMER1);

	/*for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		if(g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers){
//			hal_printf("MACAddress = %u IsAvailableForUpperLayers = %u NumTimeSyncMessagesSent = %u NumberOfRecordedElements = %u \r\n"
//					, g_NeighborTable.Neighbor[i].MACAddress
//					, g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers
//					, g_NeighborTable.Neighbor[i].NumTimeSyncMessagesSent
//					, g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[i].MACAddress)
//					);
			testDest= g_NeighborTable.Neighbor[i].MACAddress;
			VirtTimer_Start(LocalClockMonitor_TIMER1);
			hal_printf("Neighbor available MAC=%u \r\n", testDest);
			break;
		}
	}*/
}

void LinkTest_SendAckHandler (void* msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
	//hal_printf("\r\n LinkTest_SendAckHandler: status = %u radioAckStatus = %u ", status, radioAckStatus);

	Message_15_4_t* pktPtr = static_cast<Message_15_4_t*>(msg);

	UINT64 packetID;
	if(size <= sizeof(UINT64)) {
		memcpy(&packetID,pktPtr->GetPayload(),size);
	}
	else {
		memcpy(&packetID,pktPtr->GetPayload(),sizeof(UINT64));
	}

	g_NeighborTable.DeletePacket(pktPtr);

	//hal_printf(" dest = %u  PacketID = %llu rx_packet_count = %llu \r\n",pktPtr->GetHeader()->dest, packetID,  gLinkTest.rx_packet_count );
}

void SendTimerHandler(void * arg){
	gLinkTest.SendMsg();
}

void LinkTest::Initialize()
{
	for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		rx_packet_count[i]=0;
		sent_packet_count[i]=0;
	}

	MyAppID = 3; //pick a number less than MAX_APPS currently 4.
	//Config.Network = 138;
	//Config.NeighborLivenessDelay = 620;
	MACReceiveFuncPtrType rx_fptr = &LinkTest_ReceiveHandler;
	myEventHandler.SetReceiveHandler(rx_fptr);
	SendAckFuncPtrType sa_fptr = &LinkTest_SendAckHandler;
	myEventHandler.SetSendAckHandler(sa_fptr);
	NeighborChangeFuncPtrType nc_fptr =  &LinkTest_NeighborChangeHandler;
	myEventHandler.SetNeighborChangeHandler(nc_fptr);
	MacId = CSMAMAC;

	VirtTimer_Initialize();

	MAC_Initialize(&myEventHandler,MacId, MyAppID, TESTRADIONAME, (void*) &Config);

	//This is important.
	//currentMacName == CSMAMAC;

	hal_printf("Initialize LinkTest\r\n");

	VirtualTimerReturnMessage rm;
	rm = VirtTimer_SetTimer(LocalClockMonitor_TIMER1, 0, PKT_PERIOD_MICRO, FALSE, FALSE, SendTimerHandler, DEFAULT_TIMER);
	ASSERT_SP(rm == TimerSupported);
};


void LinkTest::SendMsg(){

	for(UINT8 i = 0; i < MAX_NEIGHBORS ; ++i){
		if(g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers){
			testDest= g_NeighborTable.Neighbor[i].MACAddress;
			sent_packet_count[i]++;
			UINT16 dest=testDest;
			if(dest==0) dest=0xFFFF;
			DeviceStatus ret= MAC_Send(dest, 128,  &sent_packet_count[i], sizeof(UINT64));
			if(ret!=DS_Success){
				hal_printf("LinkTest::SendMsg: Sending failed\r\n");
			}else {
				hal_printf("LinkTest::SendMsg: Sending to %u Success\r\n", dest);
			}
			::Events_WaitForEvents( 0, 500 );
		}
	}

}

void ApplicationEntryPoint()
{
    BOOL result;
    LinkTest test(0,0);

    do
    {
    	if(!test.Execute(0))
    		hal_printf("Error in Link Test.");
    	else
			hal_printf("Link Test initialization  Success!\r\n");

    } while(FALSE); // run only once!

    while(TRUE){
		::Events_WaitForEvents( 0, 100 );
    }

}
