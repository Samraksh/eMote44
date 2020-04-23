#include <Samraksh/VirtualTimer.h>
#include <Samraksh/MAC/CSMAMAC/csmaMAC.h>
//#include <Timer/netmf_timers.cpp>

#define BEACON_PERIOD_MICRO 10000000
#define SOFT_BREAKPOINT() ASSERT(0)
#define DEBUG_PRINTF_CSMA hal_printf
#define DEBUG_CSMAMAC
csmaMAC g_csmaMacObject;

void PrintHex(UINT8* x, UINT16 size){
	for(int i=0; i< size;i++){
		hal_printf("%02X ",x[i]);
	}
	hal_printf("\n");
}

volatile UINT32 csmaSendToRadioFailCount = 0;  //!< count DS_Fail from radio during sendToRadio.

UINT8 RadioLockUp;
UINT16 discoveryCounter = 0;

void* csmaReceiveHandler(void *msg, UINT16 Size){
	hal_printf("receive handler\r\n");
	return (void*) g_csmaMacObject.ReceiveHandler((Message_15_4_t *) msg, Size);
}

void csmaSendAckHandler(void* msg, UINT16 Size, NetOpStatus status, UINT8 radioAckStatus){
	g_csmaMacObject.SendAckHandler(msg, Size, status, radioAckStatus);
}

BOOL csmaRadioInterruptHandler(RadioInterrupt Interrupt, void *param){
	return g_csmaMacObject.RadioInterruptHandler(Interrupt, param);
}

void SendFirstPacketToRadio(void * arg){
	if (g_send_buffer.GetNumberMessagesInBuffer() >= 1){
	g_csmaMacObject.SendToRadio();
	} else {
		//hal_printf("no packets to send...stopping flushbuffer\r\n");
		if (g_csmaMacObject.flushTimerRunning == true) {
			VirtTimer_Stop(VIRT_TIMER_MAC_FLUSHBUFFER);
		}
	}
}

// Send a beacon everytime this fires
void beaconScheduler(void *arg){
	DEBUG_PRINTF_CSMA("bS fire\r\n");
	//hal_printf("bS fire\r\n");
	g_csmaMacObject.UpdateNeighborTable();
	g_csmaMacObject.SendHello();
}

DeviceStatus csmaMAC::SendHello(){
	UINT8 helloPayload[5];

	helloPayload[0] = (UINT8) 'H';
	helloPayload[1] = (UINT8) 'E';
	helloPayload[2] = (UINT8) 'L';
	helloPayload[3] = (UINT8) 'L';
	helloPayload[4] = (UINT8) 'O';

	hal_printf("MFM_CSMA_DISCOVERY: %d\n\r", MFM_CSMA_DISCOVERY);
	if(g_csmaMacObject.Send(0xffff, MFM_CSMA_DISCOVERY, (void *) helloPayload, 5) == TRUE) {
		//hal_printf("bS success\r\n");	
		return DS_Success;
	}
	return DS_Fail;
}

DeviceStatus csmaMAC::SetConfig(MACConfig *config){
	MyConfig.BufferSize = config->BufferSize;
	MyConfig.CCA = config->CCA;
	MyConfig.NumberOfRetries = config->NumberOfRetries;
	MyConfig.CCASenseTime = config->CCASenseTime;
	//MyConfig.RadioType  = config->RadioType;
	MyConfig.NeighborLivenessDelay = config->NeighborLivenessDelay;

	//DEBUG_PRINTF_CSMA("SetConfig: %d %d %d %d %d %d %d %d\r\n",MyConfig.BufferSize,MyConfig.CCA,MyConfig.CCASenseTime,MyConfig.RadioID,MyConfig.FCF,MyConfig.DestPAN,MyConfig.Network,MyConfig.NeighborLivelinessDelay);
	DEBUG_PRINTF_CSMA("SetConfig: %d %d %d %d\r\n",MyConfig.BufferSize,MyConfig.CCA,MyConfig.CCASenseTime,MyConfig.NeighborLivenessDelay);

	return DS_Success;
}

DeviceStatus csmaMAC::Initialize(MACEventHandler* eventHandler, UINT8 macName, UINT8 routingAppID, UINT8 radioID, MACConfig* config){
	DeviceStatus status;

	//Initialize yourself first (you being the MAC)
	if(!this->Initialized){
		////MAC<Message_15_4_t, MACConfig>::Initialize(eventHandler, macName, routingAppID, radioID, config);
		if(routingAppID >= MAX_APPS) {
			SOFT_BREAKPOINT();
			return DS_Fail;
		}
		this->macName = macName;
		this->radioName = radioID;
		SetConfig(config);
		//MAC<Message_15_4_t, MACConfig>::AppIDIndex = routingAppID;
		g_csmaMacObject.SetAppIdIndex(routingAppID);
		//Initialize upperlayer callbacks
		g_csmaMacObject.SetAppHandlers(eventHandler);

		AppCount = 0; //number of upperlayers connected to you
		csmaMAC::SetMaxPayload((UINT16)(IEEE802_15_4_FRAME_LENGTH-sizeof(IEEE802_15_4_Header_t)));

		Radio_Event_Handler.SetRadioInterruptHandler(csmaRadioInterruptHandler);
		Radio_Event_Handler.SetReceiveHandler(csmaReceiveHandler);
		Radio_Event_Handler.SetSendAckHandler(csmaSendAckHandler);

		g_send_buffer.Initialize();
		g_receive_buffer.Initialize();
		g_NeighborTable.ClearTable();

		UINT8 numberOfRadios = 1;
		RadioAckPending = FALSE;
		m_recovery = 1;

		if((status = CPU_Radio_Initialize(&Radio_Event_Handler, this->radioName, numberOfRadios, macName)) != DS_Success) {
			SOFT_BREAKPOINT();
			return status;
		}

		if((status = CPU_Radio_TurnOnRx(this->radioName)) != DS_Success) {
			SOFT_BREAKPOINT();
			return status;
		}

		SetMyAddress(CPU_Radio_GetAddress(this->radioName));

		// telling the radio to keep the RX on instead of sleeping
		CPU_Radio_SetDefaultRxState(0);

		// VIRT_TIMER_MAC_SENDPKT is the one-shot resend timer that will be activated if we need to resend a packet
		if(VirtTimer_SetOrChangeTimer(VIRT_TIMER_MAC_SENDPKT, 0, 30000, TRUE, TRUE, SendFirstPacketToRadio, ADVTIMER_32BIT) != TimerSupported){ //50 milli sec Timer in micro seconds
			ASSERT(FALSE);
			return DS_Fail;
		}

		if(VirtTimer_SetOrChangeTimer(VIRT_TIMER_MAC_BEACON, 0, BEACON_PERIOD_MICRO, FALSE, TRUE, beaconScheduler, ADVTIMER_32BIT) != TimerSupported){
			ASSERT(FALSE);
			return DS_Fail;
		}
		VirtTimer_Start(VIRT_TIMER_MAC_BEACON);

		// This is the buffer flush timer that flushes the send buffer if it contains more than just one packet
		flushTimerRunning = false;
		if(VirtTimer_SetOrChangeTimer(VIRT_TIMER_MAC_FLUSHBUFFER, 0, 50000, FALSE, TRUE, SendFirstPacketToRadio, ADVTIMER_32BIT) != TimerSupported){
			ASSERT(FALSE);
			return DS_Fail;
		}
		Initialized = TRUE;
	}

	// Stop the timer
	//VirtTimer_Stop(VIRT_TIMER_MAC_SENDPKT);  // Why?

	//Initialize upperlayer callbacks
	////AppHandlers[routingAppID] = eventHandler;
	CurrentActiveApp = routingAppID;

	return DS_Success;
}

BOOL csmaMAC::SetRadioAddress(UINT16 address){
	BOOL ret = CPU_Radio_SetAddress(this->radioName, address);
	return ret;
}

UINT16 csmaMAC::GetRadioAddress(){
	UINT16 temp = CPU_Radio_GetAddress(this->radioName);
	return temp;
}

BOOL csmaMAC::SetRadioName(UINT8 radioName){
	this->radioName = radioName;
	BOOL ret = CPU_Radio_SetRadioName(radioName);
	return ret;
}

BOOL csmaMAC::SetRadioTxPower(int power){
	BOOL ret = CPU_Radio_ChangeTxPower(this->radioName, power);
	return ret;
}

BOOL csmaMAC::SetRadioChannel(int channel){
	BOOL ret = CPU_Radio_ChangeChannel(this->radioName, channel);
	return ret;
}

BOOL csmaMAC::UnInitialize(){
	BOOL retVal = TRUE;
	if(this->Initialized) {
		retVal = retVal && (VirtTimer_Stop(VIRT_TIMER_MAC_BEACON) == TimerSupported );
		ASSERT(retVal);
		retVal = retVal && (VirtTimer_Stop(VIRT_TIMER_MAC_SENDPKT) == TimerSupported );
		ASSERT(retVal);
		retVal = retVal && (VirtTimer_Stop(VIRT_TIMER_MAC_FLUSHBUFFER) == TimerSupported );
		ASSERT(retVal);
		retVal = retVal && CPU_Radio_UnInitialize(this->radioName);
		ASSERT(retVal);
	}
	ASSERT(retVal);
	this->Initialized = FALSE;
	return retVal;
	//TODO: CPU_Radio_Sleep(this->radioName,/*RadioStateEnum::STATE_SLEEP defined in RF231RegDef.h but not mapped AFAIK.*/1);//TODO: re-visit using SLEEP_STATE or something else.
	//TODO: clear out other values, clear out queues.
}

BOOL csmaMAC::SendTimeStamped(UINT16 dest, UINT8 dataType, void* msg, int Size, UINT32 eventTime){
	static UINT8 seqNumber = 0;
	UINT8 finalSeqNumber = 0;

	Message_15_4_t msg_carrier;
	if(Size > csmaMAC::GetMaxPayload()){
		hal_printf("CSMA Send Error: Packet is too big: %d \r\n", Size);
		return FALSE;
	}
	IEEE802_15_4_Header_t* header = msg_carrier.GetHeader();
	/****** Taking the word value of below bits gives 26150 *******/
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
	//header->dsn = finalSeqNumber;
	//header->srcpan = 0x0001;
	//header->destpan = DEST_PAN_ID;
	/*if(GetRadioAddress() == 6846){
		header->dest = 0x0DB1;
	}
	else{
		header->dest = 0x1ABE;
	}*/
	header->dest = dest;
	header->src = GetMyAddress();
	seqNumber++;

	/*IEEE802_15_4_Footer* footer = msg_carrier.GetFooter();
	footer->FCS = 0xAAAA;*/

	IEEE802_15_4_Metadata_t* metadata = msg_carrier.GetMetaData();
	//UINT8 length = Size + sizeof(IEEE802_15_4_Header_t) + sizeof(IEEE802_15_4_Metadata_t);
	UINT8 length = Size + sizeof(IEEE802_15_4_Header_t);
	header->length = length;
	//metadata->SetNetwork(MyConfig.Network);
	//metadata->SetNetwork(0);
	header->macName = this->macName;
	header->payloadType = dataType;
	header->flags = (TIMESTAMPED_FLAG);
	metadata->SetReceiveTimeStamp(eventTime);

	UINT8* lmsg = (UINT8 *) msg;
	UINT8* payload =  msg_carrier.GetPayload();
	for(UINT8 i = 0 ; i < Size; i++){
		payload[i] = lmsg[i];
	}

	DEBUG_PRINTF_CSMA("CSMA Sending: My address is : %d\r\n",CPU_Radio_GetAddress(this->radioName));

	// Check if the circular buffer is full
	//if(!g_send_buffer.Store((void *) &msg_carrier, header->GetLength())){
	if(!g_send_buffer.Store((void *) &msg_carrier, header->length)){
		return FALSE;
	}

	// Try to  send the packet out immediately if possible
	SendFirstPacketToRadio(NULL);

	return TRUE;
}

BOOL csmaMAC::Send(UINT16 dest, UINT8 dataType, void* msg, int Size){
	static UINT8 seqNumber = 0;
	UINT8 finalSeqNumber = 0;

	Message_15_4_t msg_carrier;
	if(Size >  csmaMAC::GetMaxPayload()){
		hal_printf("CSMA Send Error: Packet is too big: %d \r\n", Size);
		return FALSE;
	}
	IEEE802_15_4_Header_t* header = msg_carrier.GetHeader();
	/****** Taking the word value of below bits gives 26150 *******/
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
	//header->dsn = finalSeqNumber;
	//header->srcpan = 0x0001;
	//header->destpan = DEST_PAN_ID;
	/*if(GetRadioAddress() == 6846){
		header->dest = 0x0DB1;
	}
	else{
		header->dest = 0x1ABE;
	}*/
	header->dest = dest;
	header->src = GetMyAddress();
	seqNumber++;

	/*IEEE802_15_4_Footer* footer = msg_carrier.GetFooter();
	footer->FCS = 0xAAAA;*/

	IEEE802_15_4_Metadata_t* metadata = msg_carrier.GetMetaData();
	//UINT8 length = Size + sizeof(IEEE802_15_4_Header_t) + sizeof(IEEE802_15_4_Metadata_t);
	UINT8 length = Size + sizeof(IEEE802_15_4_Header_t);
	header->length = length;
	//metadata->SetNetwork(MyConfig.Network);
	//metadata->SetNetwork(0);
	header->macName = this->macName;
	header->payloadType = dataType;
	header->flags = (0);

	UINT8* lmsg = (UINT8 *) msg;
	UINT8* payload =  msg_carrier.GetPayload();
	for(UINT8 i = 0 ; i < Size; i++){
		payload[i] = lmsg[i];
	}

	//DEBUG_PRINTF_CSMA("CSMA Sending: dest: %d, src: %d, network: %d, mac_id: %d, type: %d\r\n",dest, GetMyAddress(),MyConfig.Network,this->macName,dataType);
	DEBUG_PRINTF_CSMA("CSMA Sending: dest: %d, src: %d, network: %d, type: %d\r\n",dest, GetMyAddress(),this->macName,dataType);
	// Check if the circular buffer is full
	//if(!g_send_buffer.Store((void *) &msg_carrier, metadata->GetLength())){
	if(!g_send_buffer.Store((void *) &msg_carrier, length)){
		return FALSE;
	}

	// Try to  send the packet out immediately if possible
	SendFirstPacketToRadio(NULL);

	return TRUE;
}

// This function calls the updateneighbrtable of the neighbor object and calls the neighbor change
// callback if the neighbors died
void csmaMAC::UpdateNeighborTable(){
	UINT8 numberOfDeadNeighbors = g_NeighborTable.UpdateNeighborTable(MyConfig.NeighborLivenessDelay);

	if(numberOfDeadNeighbors > 0)
	{
		DEBUG_PRINTF_CSMA("number of dead neighbors: %d\r\n",numberOfDeadNeighbors);
		NeighborChangeFuncPtrType appHandler = g_csmaMacObject.GetAppHandler(CurrentActiveApp)->neighborHandler;

		// Check if neighbor change has been registered and the user is interested in this information
		if(appHandler != NULL)
		{
			// Make the neighbor changed callback signalling dead neighbors
			//(*appHandler)((INT16) ((-1) *numberOfDeadNeighbors));
			(*appHandler)((INT16) (g_NeighborTable.NumberOfNeighbors()));
		}
	}
	//g_NeighborTable.DegradeLinks();
}

// Called by the mac for retrying in case of failed packets 
BOOL csmaMAC::Resend(void* msg, int Size){
	// Try and push the packet back into the buffer
	if(!g_send_buffer.Store(msg, Size))
		return FALSE;
		
	return TRUE;
}

void csmaMAC::SendToRadio(){
	// if we have more than one packet in the send buffer we will switch on the timer that will be used to flush the packets out
	////DEBUG_PRINTF_CSMA("SndRad<%d> %d\r\n",g_send_buffer.GetNumberMessagesInBuffer(), RadioAckPending);
	if ( (g_send_buffer.GetNumberMessagesInBuffer() > 1) && (flushTimerRunning == false) ){
		DEBUG_PRINTF_CSMA("start FLUSHBUFFER3\r\n");
		VirtTimer_Start(VIRT_TIMER_MAC_FLUSHBUFFER);
		flushTimerRunning = true;
	}
	else if ( (g_send_buffer.GetNumberMessagesInBuffer() == 0) && (flushTimerRunning == true) ){
		DEBUG_PRINTF_CSMA("stop FLUSHBUFFER3\r\n");
		VirtTimer_Stop(VIRT_TIMER_MAC_FLUSHBUFFER);
		flushTimerRunning = false;
	}


	if(!g_send_buffer.IsEmpty() && !RadioAckPending){

		m_recovery = 1;

		//Try twice with random wait between, if carrier sensing fails return; MAC will try again later
		CPU_Radio_TurnOnRx(this->radioName);
		//DeviceStatus ds = (this->radioName, 200);
		DeviceStatus ds = CPU_Radio_ClearChannelAssesment(this->radioName);
		//DeviceStatus ds = DS_Success;
		if(ds == DS_Busy) {
			HAL_Time_Sleep_MicroSeconds((CPU_Radio_GetAddress(this->radioName) % 200)); // 500?
			//if(CPU_Radio_ClearChannelAssesment(this->radioName, 200)!=DS_Success){
			if(CPU_Radio_ClearChannelAssesment(this->radioName)!=DS_Success){
				VirtTimer_Start(VIRT_TIMER_MAC_SENDPKT);
				return;
			}
		} else if (ds == DS_Fail) {
			//SOFT_BREAKPOINT();
#ifdef DEBUG_CSMAMAC
			ASSERT(0);
			DEBUG_PRINTF_CSMA("Radio might have locked up\r\n");
			//CPU_Radio_Reset(this->radioName);
#endif
			++csmaSendToRadioFailCount;
			VirtTimer_Start(VIRT_TIMER_MAC_SENDPKT);
			return;
		}

		//Message_15_4_t** temp = g_send_buffer.GetOldestPtr();
		//Message_15_4_t* msg = *temp;
		
		Message_15_4_t txMsg;
		Message_15_4_t* txMsgPtr = &txMsg;
		Message_15_4_t** tempPtr = g_send_buffer.GetOldestPtr();
		Message_15_4_t* msgPtr = *tempPtr;
		memset(txMsgPtr, 0, msgPtr->GetMessageSize());
		memcpy(txMsgPtr, msgPtr, msgPtr->GetMessageSize());
		

		UINT8* snd_payload = txMsgPtr->GetPayload();

		//if(temp == NULL || *temp == NULL){test = 0;}

		// Check to see if there are any messages in the buffer
		if(txMsgPtr != NULL){
			DEBUG_PRINTF_CSMA("-------><%d> %d\r\n", (int)snd_payload[0], ((int)(snd_payload[1] << 8) + (int)snd_payload[2]) );
			RadioAckPending = TRUE;
			if(txMsgPtr->GetHeader()->flags & MFM_TIMESYNC)
			{
				UINT32 snapShot = (UINT32) txMsgPtr->GetMetaData()->GetReceiveTimeStamp();
				txMsgPtr = (Message_15_4_t *) CPU_Radio_Send_TimeStamped(this->radioName, (txMsgPtr), (txMsgPtr->GetHeader())->length, snapShot);
			}
			else
			{
				//txMsgPtr = (Message_15_4_t *) CPU_Radio_Send(this->radioName, (txMsgPtr), (txMsgPtr->GetHeader())->GetLength());
				txMsgPtr = (Message_15_4_t *) CPU_Radio_Send(this->radioName, (txMsgPtr), (txMsgPtr->GetHeader())->length);
				//txMsgPtr = (Message_15_4_t *) CPU_Radio_Send(this->radioName, (txMsgPtr), 70);
			}
		}
	}
}

Message_15_4_t* csmaMAC::ReceiveHandler(Message_15_4_t* msg, int Size){
	NeighborTableCommonParameters_One_t neighborTableCommonParameters_One_t;
	NeighborTableCommonParameters_Two_t neighborTableCommonParameters_two_t;
	UINT8 index;


	if(Size == sizeof(softwareACKHeader)){
		hal_printf("software ACK\r\n");
		return msg;
	}
	else if(Size < sizeof(IEEE802_15_4_Header_t)){
		hal_printf("CSMA Receive Error: Packet is too small. Size: %d, Expected atleast HeaderSize: %d \n Printing raw bytes: ", Size, sizeof(IEEE802_15_4_Header_t));
		PrintHex((UINT8*)msg, Size);
		return msg;
	}
	else if(Size > ( csmaMAC::GetMaxPayload() + sizeof(IEEE802_15_4_Header_t))){
		hal_printf("CSMA Receive Error: Packet is too big. Size: %d, MaxPayload: %d, ExpectedHeaderSize: %d \r\n", Size, csmaMAC::GetMaxPayload(), sizeof(IEEE802_15_4_Header_t));
		return msg;
	}

	// Get the header packet
	IEEE802_15_4_Header_t* rcv_msg_hdr = msg->GetHeader();
	//If send happened with a timestamp, then subtract TIMESTAMP_SIZE

	//MS:The incoming message might have a different size, based on sending side buffer, reset it just to be sure.
	//rcv_msg_hdr->length=Size;

	if(rcv_msg_hdr->flags == TIMESTAMPED_FLAG){
		rcv_msg_hdr->length -= TIMESTAMP_SIZE;
	}

	IEEE802_15_4_Metadata_t* rcv_meta = msg->GetMetaData();
	UINT8* rcv_payload = msg->GetPayload();

	hal_printf("(%d) <%d> %d\r\n",Size, (int)rcv_payload[0],((int)(rcv_payload[1] << 8) + (int)rcv_payload[2]) );

	// If the message type is a discovery then return the same bag you got from the radio layer
	// Don't make a callback here because the neighbor table takes care of informing the application of a changed situation of
	// it neighbors
	//hal_printf("MFM_CSMA_DISCOVERY receive value = %d (%d)", rcv_msg_hdr->payloadType, rcv_msg_hdr->src);
	if(rcv_msg_hdr->payloadType == MFM_CSMA_DISCOVERY)
	{
		hal_printf("csmaMAC.cpp:502\r\n");
		//Add the sender to NeighborTable
			if(g_NeighborTable.FindIndex(rcv_msg_hdr->src, &index) != DS_Success)
			{
				hal_printf("csmaMAC.cpp:506\r\n");
				// Insert into the table if a new node was discovered
				neighborTableCommonParameters_One_t.MACAddress = rcv_msg_hdr->src;
				neighborTableCommonParameters_One_t.status = Alive;
				neighborTableCommonParameters_One_t.lastHeardTime = HAL_Time_CurrentTicks();
				neighborTableCommonParameters_One_t.linkQualityMetrics.AvgRSSI = rcv_meta->GetRssi();
				//neighborTableCommonParameters_One_t.linkQualityMetrics.LinkQuality = rcv_meta->GetLqi();
				neighborTableCommonParameters_One_t.availableForUpperLayers=TRUE;

				neighborTableCommonParameters_two_t.nextSeed = 0;
				neighborTableCommonParameters_two_t.mask = 0;
				neighborTableCommonParameters_two_t.nextwakeupSlot = 0;
				neighborTableCommonParameters_two_t.seedUpdateIntervalinSlots = 0;

				if(g_NeighborTable.InsertNeighbor(&neighborTableCommonParameters_One_t, &neighborTableCommonParameters_two_t) == DS_Success)
				{
					hal_printf("csmaMAC.cpp:522\r\n");
					NeighborChangeFuncPtrType appHandler = g_csmaMacObject.GetAppHandler(CurrentActiveApp)->neighborHandler;

					// Check if  a neighbor change has been registered
					if(appHandler != NULL)
					{
						//GLOBAL_LOCK(irq);//MS: why?
						// CLR_RT_HeapBlock_NativeEventDispatcher::SaveToHALQueue calls ASSERT_IRQ_MUST_BE_OFF()
						// Insert neighbor always inserts one neighbor so the call back argument will alsways be 1
						hal_printf("Neighbor Changed Handler in Link Test\n\r");
						(*appHandler)(1);
					}
				}
			}
			else
			{
				hal_printf("csmaMAC.cpp:538\r\n");
				//g_NeighborTable.UpdateNeighbor(rcv_msg_hdr->src, Alive, HAL_Time_CurrentTicks(), rcv_meta->GetRssi(), rcv_meta->GetLqi());
				neighborTableCommonParameters_One_t.MACAddress = rcv_msg_hdr->src;
				neighborTableCommonParameters_One_t.status = Alive;
				neighborTableCommonParameters_One_t.lastHeardTime = HAL_Time_CurrentTicks();
				//neighborTableCommonParameters_One_t.linkQualityMetrics.LinkQuality = rcv_meta->GetLqi();
				neighborTableCommonParameters_One_t.linkQualityMetrics.AvgRSSI = rcv_meta->GetRssi();
				g_NeighborTable.UpdateNeighbor(&neighborTableCommonParameters_One_t);
			}
			return msg;
	}

	// Dont add the packet to the handler if the message happens to be a unicast not intended for me, unless you want to enable promiscous
	if((rcv_msg_hdr->dest == 0)){
		//dont do anything
	}
	else if( ( rcv_msg_hdr->dest != MAC_BROADCAST_ADDRESS && rcv_msg_hdr->dest != CPU_Radio_GetAddress(this->radioName) ) )
	{
		//HandlePromiscousMessage(msg);
		hal_printf("csmaMAC.cpp:556\r\n");
		return msg;
	}
	// Implement bag exchange if the packet type is data
	Message_15_4_t** next_free_buffer = g_receive_buffer.GetNextFreeBufferPtr();

	if(! (next_free_buffer))
	{
		g_receive_buffer.DropOldest(1);
		next_free_buffer = g_receive_buffer.GetNextFreeBufferPtr();
	}

	//Implement bag exchange, by actually switching the contents.
	Message_15_4_t* temp = *next_free_buffer;	//get the ptr to a msg inside the first free buffer.
	(*next_free_buffer) = msg;	//put the currently received message into the buffer (thereby its not free anymore)
								//finally the temp, which is a ptr to free message will be returned.


	//Call routing/app receive callback
	MACReceiveFuncPtrType appHandler = g_csmaMacObject.GetAppHandler(CurrentActiveApp)->ReceiveHandler;

	// Protect against catastrophic errors like dereferencing a null pointer
	if(appHandler == NULL)
	{
		SOFT_BREAKPOINT();
		hal_printf("[NATIVE] Error from csma mac recieve handler :  Handler not registered\n");
		goto ReceiveHandler_out;
	}

	//TODO: GLOBAL_LOCK(irq); // CLR_RT_HeapBlock_NativeEventDispatcher::SaveToHALQueue requires IRQs off.  Updater needs IRQs on; TODO: make Update use a queue and disable IRQs again?
	//(*appHandler)(msg, g_receive_buffer.GetNumberMessagesInBuffer());
	hal_printf("Receive Handler in Link Test\n\r");
	(*appHandler)(*next_free_buffer, rcv_msg_hdr->payloadType);

#if 0
	//hal_printf("CSMA Receive: SRC address is : %d\n", rcv_msg_hdr->src);
	if(rcv_msg_hdr->dest == MAC_BROADCAST_ADDRESS){

		// Nived.Sivadas - changing interfaces with new dll design
		(*appHandler)(g_receive_buffer.GetNumberMessagesInBuffer());
		//(*appHandler)(msg->GetPayload(), Size- sizeof(IEEE802_15_4_Header_t), rcv_msg_hdr->src,FALSE,rcv_meta->GetRssi(), rcv_meta->GetLqi());
		//HandleBroadcastMessage(msg);
	}else if(rcv_msg_hdr->dest == CPU_Radio_GetAddress(this->radioName)){
		//HandleUnicastMessage(msg);
		(*appHandler)(g_receive_buffer.GetNumberMessagesInBuffer());
		//(*appHandler)(msg->GetPayload(), Size- sizeof(IEEE802_15_4_Header_t), rcv_msg_hdr->src,TRUE,rcv_meta->GetRssi(), rcv_meta->GetLqi());
	}
	else {
		//HandlePromiscousMessage(msg);
	}
#endif
ReceiveHandler_out:
	return temp;
}


BOOL csmaMAC::RadioInterruptHandler(RadioInterrupt Interrupt, void* Param){
	return FALSE;
}


void csmaMAC::SendAckHandler(void* msg, int Size, NetOpStatus status, UINT8 radioAckStatus){
#ifdef DEBUG_CSMAMAC
	Message_15_4_t* temp = (Message_15_4_t *)msg;
	UINT8* rcv_payload =  temp->GetPayload();
#endif
	Message_15_4_t* sentPtr = (Message_15_4_t *)msg;
	switch(status)
	{
		case NetworkOperations_Success:
			{
				DEBUG_PRINTF_CSMA("Success <%d> #%d\r\n", (int)rcv_payload[0],((int)(rcv_payload[1] << 8) + (int)rcv_payload[2]));
				//VirtTimer_Stop(VIRT_TIMER_MAC_FLUSHBUFFER);
				if(sentPtr->GetHeader()->payloadType == MFM_CSMA_DISCOVERY){
					//This is our discovery message, dont call app/routing, just remove pkt from buffer
					g_NeighborTable.DeletePacket(sentPtr);
				}else {
					//This is application packet, call its senkack handler
					if(SendAckFuncPtrType appHandler = g_csmaMacObject.GetAppHandler(CurrentActiveApp)->SendAckHandler)
					(*appHandler)(msg, Size, status, radioAckStatus);

				}
				// Attempt to send the next packet out since we have no scheduler
				if(!g_send_buffer.IsBufferEmpty())
				{
					VirtTimer_Start(VIRT_TIMER_MAC_FLUSHBUFFER);
					flushTimerRunning = true;
				}
			}
			break;
		
		case NetworkOperations_Busy:
			//TODO: when resend is called, packet should be placed at front of buffer. Right now it is at end of buffer.
			DEBUG_PRINTF_CSMA("Resending <%d> #%d\r\n", (int)rcv_payload[0],((int)(rcv_payload[1] << 8) + (int)rcv_payload[2]));
			Resend(msg, Size);
			VirtTimer_Start(VIRT_TIMER_MAC_FLUSHBUFFER);
			flushTimerRunning = true;
			break;
			
		default:
			DEBUG_PRINTF_CSMA("Error #%d\r\n",((int)(rcv_payload[1] << 8) + (int)rcv_payload[2]));
			break;
	}
	
	RadioAckPending=FALSE;
	RadioLockUp=0;
}

UINT8 csmaMAC::GetSendBufferSize(){
	return g_send_buffer.Size();
}

UINT8 csmaMAC::GetReceiveBufferSize(){
	return g_receive_buffer.Size();
}

