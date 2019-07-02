/*
 * MAC_Functions.cpp
 *
 *  Created on: Oct 1, 2012
 *      Author: Mukundan
 */

#include <Samraksh/MAC_decl.h>
#include <Samraksh/PacketTimeSync_15_4.h>


#if defined(__MAC_CSMA__)
#include <Samraksh/MAC/CSMAMAC/csmaMAC.h>
#endif

#if defined(__MAC_OMAC__)
#include <Samraksh/MAC/OMAC/OMAC.h>
#endif

#if defined(__MAC_CSMA__)
extern csmaMAC g_csmaMacObject;
#endif

#if defined(__MAC_OMAC__)
extern OMACType g_OMAC;
#endif

//extern UINT8 MAC_ID::Unique_Mac_ID = 0;

Buffer_15_4_t g_send_buffer;
Buffer_15_4_t g_receive_buffer;
NeighborTable g_NeighborTable;

UINT8 currentMacName;

//#define DEBUG_MACFUNCTIONS 1

#if defined(DEBUG_MACFUNCTIONS)
#define ENABLE_PIN_MAC(x,y) CPU_GPIO_EnableOutputPin(x,y)
#define SET_PIN(x,y) CPU_GPIO_SetPinState(x,y)
#define DEBUG_PRINTF_MAC(x) CLR_Debug::Printf(x)
#else
#define ENABLE_PIN_MAC(x,y)
#define SET_PIN(x,y)
#define DEBUG_PRINTF_MAC(x)
#endif


//Basic functions
//UINT8 MacName = 0;

DeviceStatus MAC_Initialize(MACEventHandler* eventHandler, UINT8 macName, UINT8 routingAppID, UINT8 radioName, void* config){
	DeviceStatus status = DS_Success;
	if(macName == CSMAMAC){
		currentMacName = macName;
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.Initialize(eventHandler, macName, routingAppID, radioName, (MACConfig*)config) ;
#endif		
	}
	else if(macName == OMAC) {
		currentMacName = macName;
#if defined(__MAC_OMAC__)			
		status = g_OMAC.Initialize(eventHandler, macName, routingAppID, radioName, (MACConfig *) config);
#endif
	}
	else{
		status = DS_Fail;
	}

	return status;
}

DeviceStatus MAC_Reconfigure(void* config)
{
	DeviceStatus status = DS_Success;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.SetConfig((MACConfig*)config);
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)			
		status = g_OMAC.SetConfig((MACConfig*)config);
#endif
	}
	else{
		status = DS_Fail;
	}

	return status;
}

UINT16 MAC_GetRadioAddress(){
	UINT16 tempMacName = -1;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		tempMacName = g_csmaMacObject.GetRadioAddress();
		return tempMacName;
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)			
		tempMacName = g_OMAC.GetRadioAddress();
		return tempMacName;
#endif
	}
	return tempMacName;
}

BOOL MAC_SetRadioAddress(UINT16 address){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.SetRadioAddress(address);
		return status;
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)			
		status = g_OMAC.SetRadioAddress(address);
		return status;
#endif
	}
	return status;
}

BOOL MAC_SetRadioName(INT8 radioName){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.SetRadioName(radioName);
		return status;
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)			
		status = g_OMAC.SetRadioName(radioName);
		return status;
#endif
	}
	return status;
}

BOOL MAC_SetRadioTxPower(int power){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.SetRadioTxPower(power);
		return status;
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)			
		status = g_OMAC.SetRadioTxPower(power);
		return status;
#endif
	}
	return status;
}

BOOL MAC_SetRadioChannel(int channel){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.SetRadioChannel(channel);
		return status;
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)			
		status = g_OMAC.SetRadioChannel(channel);
		return status;
#endif
	}
	return status;
}

DeviceStatus MAC_DeletePacketWithIndexInternal(PacketID_T index){
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return DS_Fail;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return g_OMAC.DeletePacketWithIndexInternal(index);
#endif
	}
	return DS_Fail;
}

UINT16 MAC_GetMsgIdWithPtr(Message_15_4_t* msg_carrier)
{
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return INVALID_PACKET_ID;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return  g_NeighborTable.GetIndexWithPtr(msg_carrier);
#endif
	}
	return INVALID_PACKET_ID;
}

bool MAC_ChangeOwnerShipOfElementwIndex(PacketID_T index,  BufferOwner n_buf_ow)
{
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return INVALID_PACKET_ID;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return  g_NeighborTable.ChangeOwnerShipOfElementwIndex(index,n_buf_ow);
#endif
	}
	return INVALID_PACKET_ID;
}

DeviceStatus MAC_GetPacketWithIndex(UINT8 **managedBuffer, UINT8 buffersize, PacketID_T index)
{
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return DS_Fail;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return g_OMAC.GetPacketWithIndex(managedBuffer, buffersize, index);
#endif
	}
	return DS_Fail;
}

DeviceStatus MAC_GetPacketSizeWithIndex( UINT8* buffersizeptr, PacketID_T index)
{
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return DS_Fail;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return g_OMAC.GetPacketSizeWithIndex( buffersizeptr, index);
#endif
	}
	return DS_Fail;
}


DeviceStatus MAC_GetNextPacket(UINT8 **managedBuffer)
{
	GLOBAL_LOCK(irq);

	Message_15_4_t** temp = g_receive_buffer.GetOldestPtr();

	//Message_15_4_t* temp = (Message_15_4_t*)managedBuffer;

	if((*temp) == NULL){
		return DS_Fail;
	}

	UINT8 Size = ((*temp)->GetHeader())->length - sizeof(IEEE802_15_4_Header_t);

	if(Size > 127){
		return DS_Fail;
	}
		
	(*managedBuffer)[0] = Size & 0x00ff;
	(*managedBuffer)[1] = (Size & 0xff00) >> 8;

	memcpy(&((*managedBuffer)[2]), (*temp)->GetPayload(), Size);

	(*managedBuffer)[2 + Size] = (*temp)->GetHeader()->payloadType;
	(*managedBuffer)[3 + Size] = (*temp)->GetMetaData()->GetRssi();
	(*managedBuffer)[4 + Size] = (*temp)->GetMetaData()->GetLqi();
	(*managedBuffer)[5 + Size] = (*temp)->GetHeader()->src;
	(*managedBuffer)[6 + Size] = ((*temp)->GetHeader()->src & 0Xff00) >> 8;
	(*managedBuffer)[7 + Size] = ((*temp)->GetHeader()->dest == MAC_BROADCAST_ADDRESS) ? 0 : 1;
	//memcpy(*managedBuffer, *temp, ((*temp)->GetHeader())->length - sizeof(IEEE802_15_4_Header_t));

	if( ((*temp)->GetHeader())->flags & MFM_TIMESYNC )
	{
		// The packet is timestamped
		(*managedBuffer)[8 + Size] = (BYTE) TRUE;
		UINT64 EventTime = PacketTimeSync_15_4::EventTime((*temp), ((*temp)->GetHeader())->length);

		UINT32 eventTime = (EventTime & 0xffffffff);

#ifdef DEBUG_MACFUNCTIONS
		hal_printf("The Snap Shot in native is %u\n", eventTime);
#endif

		(*managedBuffer)[9 + Size] = (EventTime & 0xff);
		(*managedBuffer)[10 + Size] = (EventTime >> 8) & 0xff;
		(*managedBuffer)[11 + Size] = (EventTime >> 16) & 0xff;
		(*managedBuffer)[12 + Size] = (EventTime >> 24) & 0xff;
		(*managedBuffer)[13 + Size] = (EventTime >> 32) & 0xff;
		(*managedBuffer)[14 + Size] = (EventTime >> 40) & 0xff;
		(*managedBuffer)[15 + Size] = (EventTime >> 48) & 0xff;
		(*managedBuffer)[16 + Size] = (EventTime >> 56) & 0xff;
	}
	else
	{
		(*managedBuffer)[8 + Size] = (BYTE) FALSE;
		(*managedBuffer)[9 + Size] = ((*temp)->GetMetaData()->GetReceiveTimeStamp() & 0xff);
     	(*managedBuffer)[10 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 8) & 0xff);
		(*managedBuffer)[11 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 16) & 0xff);
		(*managedBuffer)[12 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 24) & 0xff);
		(*managedBuffer)[13 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 32) & 0xff);
		(*managedBuffer)[14 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 40) & 0xff);
		(*managedBuffer)[15 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 48) & 0xff);
		(*managedBuffer)[16 + Size] = (((*temp)->GetMetaData()->GetReceiveTimeStamp() >> 56) & 0xff);
	}

	//g_receive_buffer.DropOldest(1);

	return DS_Success;
}

DeviceStatus MAC_UnInitialize(){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.UnInitialize();
#endif
	}
	else if(currentMacName == OMAC) {
#if defined(__MAC_OMAC__)	
		status = g_OMAC.UnInitialize();
#endif
	}

	return ((status == TRUE) ? DS_Success : DS_Fail);
}

UINT8 MAC_GetID(){
	return currentMacName;
}

DeviceStatus MAC_SendTimeStamped(UINT16 destAddress, UINT8 dataType, void * msg, UINT16 size, UINT32 eventTime){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.SendTimeStamped(destAddress, dataType, msg, size, eventTime);
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)	
		status = g_OMAC.SendTimeStamped(destAddress, dataType, msg, size, eventTime);
#endif
	}

	if(status != TRUE)
	    return DS_Fail;

	return DS_Success;
}

DeviceStatus MAC_Send(UINT16 destAddress, UINT8 dataType, void * msg, UINT16 size){
	//msg is just the payload,

	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.Send(destAddress, dataType, msg, size);
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		status = g_OMAC.Send(destAddress, dataType, msg, size);
#endif
	}

	if(status != TRUE)
		return DS_Fail;

	return DS_Success;
}

PacketID_T MAC_EnqueueToSend(UINT16 destAddress, UINT8 dataType, void * msg, UINT16 size){
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return INVALID_PACKET_ID;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return g_OMAC.EnqueueToSend(destAddress, dataType, msg, size);
#endif
	}
	return INVALID_PACKET_ID;
}

PacketID_T MAC_EnqueueToSendTimeStamped(UINT16 destAddress, UINT8 dataType, void * msg, UINT16 size, UINT32 eventTime){
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		return INVALID_PACKET_ID;
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		return g_OMAC.EnqueueToSendTimeStamped(destAddress, dataType, msg, size, eventTime);
#endif
	}
	return INVALID_PACKET_ID;
}




DeviceStatus MAC_GetNeighborList(UINT16 *buffer)
{
	UINT8 neighborCount = 0;

	DEBUG_PRINTF_MAC("[NATIVE] : Calling GetNeighbor List\n");

	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	{
		// initializing buffer
		buffer[i] = 0;
	}
	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	{
		if(g_NeighborTable.Neighbor[i].neighborStatus == Alive && g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers)
		{
			buffer[neighborCount++] = g_NeighborTable.Neighbor[i].MACAddress;
		}
	}

	if(neighborCount == 0)
	{
		DEBUG_PRINTF_MAC("[NATIVE] : Neighbor Count is 0\n");
		buffer[0] = 0;
	}

	return DS_Success;
}
DeviceStatus MAC_GetMACNeighborList(UINT16 *buffer)
{
	UINT8 neighborCount = 0;

	DEBUG_PRINTF_MAC("[NATIVE] : Calling GetNeighbor List\n");

	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	{
		// initializing buffer
		buffer[i] = 0;
	}
	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	{
		if(ISMAC_VALID(g_NeighborTable.Neighbor[i].MACAddress) )
		{
			buffer[neighborCount++] = g_NeighborTable.Neighbor[i].MACAddress;
		}
	}

	if(neighborCount == 0)
	{
		DEBUG_PRINTF_MAC("[NATIVE] : Neighbor Count is 0\n");
		buffer[0] = 0;
	}

	return DS_Success;
}

DeviceStatus MAC_GetNeighborStatus(UINT16 macAddress, UINT8 *buffer)
{
	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	{
		if(g_NeighborTable.Neighbor[i].MACAddress == macAddress)
		{
			buffer[0] = g_NeighborTable.Neighbor[i].MACAddress & 0xff;
			buffer[1] = (g_NeighborTable.Neighbor[i].MACAddress & 0xff00) >> 8;
			buffer[2] = (g_NeighborTable.Neighbor[i].SendLink.AvgRSSI);
			buffer[3] = (g_NeighborTable.Neighbor[i].SendLink.LinkQuality);
			buffer[4] = (g_NeighborTable.Neighbor[i].SendLink.AveDelay);
			buffer[5] = (g_NeighborTable.Neighbor[i].ReceiveLink.AvgRSSI);
			buffer[6] = (g_NeighborTable.Neighbor[i].ReceiveLink.LinkQuality);
			buffer[7] = (g_NeighborTable.Neighbor[i].ReceiveLink.AveDelay);
			buffer[8] = (g_NeighborTable.Neighbor[i].neighborStatus & 0x0F) || ( ((g_NeighborTable.Neighbor[i].IsAvailableForUpperLayers) & 0x0F) << 4)  ;
			buffer[9] = (g_NeighborTable.Neighbor[i].NumInitializationMessagesSent);
#if defined(__MAC_OMAC__)
			buffer[10] =  g_OMAC.m_omac_scheduler.m_TimeSyncHandler.m_globalTime.regressgt2.NumberOfRecordedElements(g_NeighborTable.Neighbor[i].MACAddress);
#endif
//			buffer[9] = (g_NeighborTable.Neighbor[i].CountOfPacketsReceived & 0xff);
//			buffer[10] = (g_NeighborTable.Neighbor[i].CountOfPacketsReceived & 0xff00) >> 8;
			buffer[11] = (g_NeighborTable.Neighbor[i].LastHeardTime) & 0xff;
			buffer[12] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff00) >> 8;
			buffer[13] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff0000) >> 16;
			buffer[14] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff000000) >> 24;
			buffer[15] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff00000000) >> 32;
			buffer[16] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff0000000000) >> 40;
			buffer[17] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff000000000000) >> 48;
			buffer[18] = (g_NeighborTable.Neighbor[i].LastHeardTime & 0xff00000000000000) >> 56;
			buffer[19] = (g_NeighborTable.Neighbor[i].ReceiveDutyCycle);
			buffer[20] = (g_NeighborTable.Neighbor[i].FrameLength);
			buffer[21] = (g_NeighborTable.Neighbor[i].FrameLength & 0xff00) >> 8;

			return DS_Success;

		}
	}

	return DS_Fail;
}

#if  0
//Neighbor functions
NeighborTable* MAC_GetNeighborTable(UINT8 macID){
	if(MacName == CSMAMAC)
#if defined(__MAC_CSMA__)		
		return g_csmaMacObject.GetNeighborTable();
#endif
	else if(MacName == OMAC)
#if defined(__MAC_OMAC__)	
		return NULL;
#endif
	//return (NeighborTable *)(NULL);
	return NULL;
}

Neighbor_t* MAC_GetNeighbor(UINT8 macID, UINT16 macAddress){

}
#endif
//Channel/freq functions


//Buffer functions
UINT8 MAC_GetSendBufferSize(){
	UINT8 bufferSize = -1;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		bufferSize = g_csmaMacObject.GetSendBufferSize();
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)	
		bufferSize = g_OMAC.GetSendBufferSize();
#endif
	}

	return bufferSize;
}

UINT8 MAC_GetReceiveBufferSize(){
	UINT8 bufferSize = -1;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		bufferSize = g_csmaMacObject.GetReceiveBufferSize();
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		bufferSize = g_OMAC.GetReceiveBufferSize();
#endif
	}

	return bufferSize;
}

UINT8 MAC_GetPendingPacketsCount_Send(){
	UINT8 pendingPackets = -1;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		pendingPackets = g_csmaMacObject.GetSendPending();
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)	
		pendingPackets = g_OMAC.GetSendPending();
#endif
	}

	return pendingPackets;
}

UINT8 MAC_GetPendingPacketsCount_Receive(){
	UINT8 pendingPackets = -1;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		pendingPackets = g_csmaMacObject.GetReceivePending();
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)			
		pendingPackets = g_OMAC.GetReceivePending();
#endif
	}

	return pendingPackets;
}

DeviceStatus MAC_RemovePacket(UINT8* msg){
	return DS_Fail;
}

//Mac Aggregate APIs
BOOL MACLayer_Initialize(){
	return FALSE;
}

BOOL MACLayer_UnInitialize(){
	BOOL status = FALSE;
	if(currentMacName == CSMAMAC){
#if defined(__MAC_CSMA__)
		status = g_csmaMacObject.UnInitialize();
#endif
	}
	else if(currentMacName == OMAC){
#if defined(__MAC_OMAC__)		
		status = g_OMAC.UnInitialize();
#endif
	}

	return status;
}

UINT8 MACLayer_NumberMacsSupported(){
	return FALSE;
}


