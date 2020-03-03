//-----------------------------------------------------------------------------
//
//                   ** WARNING! ** 
//    This file was generated automatically by a tool.
//    Re-running the tool will overwrite this file.
//    You should copy this file to a custom location
//    before adding any customization in the copy to
//    prevent loss of your changes when the tool is
//    re-run.
//
//-----------------------------------------------------------------------------


#include "Samraksh_eMote_Net.h"
#include "Samraksh_eMote_Net_Samraksh_eMote_Net_MACBase.h"
#include <tinyhal.h>
#include <Samraksh/MAC_decl.h>
#include <Samraksh/MAC.h>
#include <Samraksh\Buffer.h>

using namespace Samraksh::eMote::Net;
extern CLR_RT_HeapBlock_NativeEventDispatcher *Net_ne_Context;

MACEventHandler_t MACBase::Event_Handler;
UINT8 MacID = 0;
UINT8 MACBase::MyAppID;

enum CallBackTypes
{
	ReceivedCallback,
	NeighborChangedCallback,
	SendInitiated,
	SendACKed,
	SendNACKed,
	SendFailed
};

void ManagedCallback(UINT32 arg1, UINT32 arg2);
void ManagedSendAckCallbackFn(void *msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus);

void NeighborChangedCallbackFn(INT16 numberOfNeighbors);
void ReceiveDoneCallbackFn(void* msg, UINT16 numberOfPackets);

extern Buffer_15_4_t g_send_buffer;
extern Buffer_15_4_t g_receive_buffer;

UINT8 MACInteropBuffer[IEEE802_15_4_FRAME_LENGTH];

INT32 MACBase::RemovePacket( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    Message_15_4_t** temp = g_receive_buffer.GetOldestPtr();
    if((*temp) == NULL){
		return DS_Fail;
	}
	return DS_Success;
}

UINT8 MACBase::PendingReceivePacketCount( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    return g_receive_buffer.GetNumberMessagesInBuffer();
}

UINT8 MACBase::PendingSendPacketCount( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    return g_send_buffer.GetNumberMessagesInBuffer();
}

INT32 MACBase::Send( CLR_RT_HeapBlock* pMngObj, UINT16 address, CLR_RT_TypedArray_UINT8 payloadTemp, UINT16 offset, UINT16 size, HRESULT &hr )
{
    InteropNetOpStatus retVal;
    //UINT16 address, offset, length;
    UINT8* payload = payloadTemp.GetBuffer();
    //address=param0;
    //offset=param2;
    //length=param3;
    memcpy (MACInteropBuffer, payload, size);

    if (MAC_Send(address, MFM_DATA, (void*) MACInteropBuffer, size) == DS_Success)
		retVal = S_Success;
	else
		retVal = E_MacSendError;

	return retVal;
}

INT32 MACBase::UnInitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr )
{
    DeviceStatus status;
	status = MAC_UnInitialize();
    return status;
}

INT32 MACBase::InternalInitialize( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 marshalBuffer, UINT8 macname, HRESULT &hr )
{
	hal_printf("MAC base init\r\n");
    INT32 result = DS_Success;
	UINT8* configParams = marshalBuffer.GetBuffer();
	MACConfig config;

	config.CCA = (configParams[0] == 1) ? TRUE :  FALSE;
	config.NumberOfRetries = configParams[1];
	config.CCASenseTime = configParams[2];
	config.BufferSize = configParams[3];
	//config.RadioType = configParams[4];
	config.NeighborLivenessDelay = configParams[4];
	config.NeighborLivenessDelay |= configParams[5] << 8;
	config.NeighborLivenessDelay |= configParams[6] << 16;
	config.NeighborLivenessDelay |= configParams[7] << 24;

	Event_Handler.SetReceiveHandler(&ReceiveDoneCallbackFn);
	Event_Handler.SetNeighborChangeHandler(&NeighborChangedCallbackFn);
	Event_Handler.SetSendAckHandler(&ManagedSendAckCallbackFn);

	MyAppID=3; //pick a number less than MAX_APPS currently 4.

	if(MAC_Initialize(&Event_Handler, macname, MyAppID, configParams[8], (void*) &config) != DS_Success) {
	    hr = -1;
	    result = DS_Fail;
	}
	
	return result;
}

INT32 MACBase::InternalReConfigure( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 marshalBuffer, HRESULT &hr )
{
    INT32 result = DS_Success;
	UINT8* configParams = marshalBuffer.GetBuffer();
	MACConfig config;

	config.CCA = (configParams[0] == 1) ? TRUE :  FALSE;
	config.NumberOfRetries = configParams[1];
	config.CCASenseTime = configParams[2];
	config.BufferSize = configParams[3];
	//config.RadioType = configParams[4];
	config.NeighborLivenessDelay = configParams[4];
	config.NeighborLivenessDelay |= configParams[5] << 8;
	config.NeighborLivenessDelay |= configParams[6] << 16;
	config.NeighborLivenessDelay |= configParams[7] << 24;

	if( MAC_Reconfigure((void*) &config) != DS_Success ){
		hr = -1;
		result = DS_Fail;
	}
	return result;
}

INT32 MACBase::GetNextPacket( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 nativeBuffer, HRESULT &hr )
{
    UINT8* managedBuffer = nativeBuffer.GetBuffer();
	return MAC_GetNextPacket(&managedBuffer);
}

INT32 MACBase::GetNeighborInternal( CLR_RT_HeapBlock* pMngObj, UINT16 macAddress, CLR_RT_TypedArray_UINT8 neighborlist, HRESULT &hr )
{
    return MAC_GetNeighborStatus(macAddress, neighborlist.GetBuffer());
}

INT32 MACBase::GetNeighborListInternal( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 neighborlist, HRESULT &hr )
{
    return MAC_GetNeighborList(neighborlist.GetBuffer());
}

INT32 MACBase::DeletePacketWithIndexInternal( CLR_RT_HeapBlock* pMngObj, UINT16 param0, HRESULT &hr )
{
	return MAC_DeletePacketWithIndexInternal(param0);
}

INT32 MACBase::GetMACNeighborListInternal( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 neighborlist, HRESULT &hr )
{
    return MAC_GetMACNeighborList(neighborlist.GetBuffer());
}

INT32 MACBase::GetPacketWithIndex( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 nativeBuffer, UINT8 buffersize, UINT16 index, HRESULT &hr )
{
    UINT8* managedBuffer = nativeBuffer.GetBuffer();
	return MAC_GetPacketWithIndex(&managedBuffer, buffersize, index);
}

INT32 MACBase::GetPacketSizeWithIndex( CLR_RT_HeapBlock* pMngObj, UINT8 * buffersize, UINT16 index, HRESULT &hr )
{
    return MAC_GetPacketSizeWithIndex(buffersize, index);
}

UINT16 MACBase::EnqueueToSend(  CLR_RT_HeapBlock* pMngObj, UINT16 address, UINT8 payloadType, CLR_RT_TypedArray_UINT8 payloadTemp, UINT16 offset, UINT16 size, HRESULT &hr )
{
    UINT32 retVal = MAC_EnqueueToSend(address, payloadType, (void*) payloadTemp.GetBuffer(), size);
    return retVal;
}

UINT16 MACBase::EnqueueToSend( CLR_RT_HeapBlock* pMngObj, UINT16 address, UINT8 payloadType, CLR_RT_TypedArray_UINT8 payloadTemp, UINT16 offset, UINT16 size, UINT32 eventtime, HRESULT &hr )
{
    UINT32 retVal = MAC_EnqueueToSendTimeStamped(address, payloadType, (void*) payloadTemp.GetBuffer(), size, eventtime);
    return retVal;
}

INT32 MACBase::Send( CLR_RT_HeapBlock* pMngObj, UINT16 address, UINT8 payloadType, CLR_RT_TypedArray_UINT8 payloadTemp, UINT16 offset, UINT16 size, HRESULT &hr )
{
	InteropNetOpStatus retVal;
    //UINT16 address, offset, length;
    UINT8* payload = payloadTemp.GetBuffer();
    //address=param0;
    //offset=param2;
    //length=param3;
    memcpy (MACInteropBuffer, payload, size);

    if (MAC_Send(address, payloadType, (void*) MACInteropBuffer, size) == DS_Success)
		retVal = S_Success;
	else
		retVal = E_MacSendError;

    return retVal;
}

INT32 MACBase::Send( CLR_RT_HeapBlock* pMngObj, UINT16 address, CLR_RT_TypedArray_UINT8 payloadTemp, UINT16 offset, UINT16 size, UINT32 eventTime, HRESULT &hr )
{
	InteropNetOpStatus retVal;
    //UINT16 address, offset, length;
    UINT8* payload = payloadTemp.GetBuffer();
    //address=param0;
    //offset=param2;
    //length=param3;
    memcpy (MACInteropBuffer, payload, size);

    //if (MAC_Send(address, MFM_DATA, (void*) MACInteropBuffer, size) == DS_Success)
    if (MAC_SendTimeStamped(address, MFM_DATA, (void*) MACInteropBuffer, size, eventTime) == DS_Success)
		retVal = S_Success;
	else
		retVal = E_MacSendError;

    return retVal;
}

INT32 MACBase::Send( CLR_RT_HeapBlock* pMngObj, UINT16 address, UINT8 payloadType, CLR_RT_TypedArray_UINT8 payloadTemp, UINT16 offset, UINT16 size, UINT32 eventTime, HRESULT &hr )
{
	InteropNetOpStatus retVal;
	//UINT16 address, offset, length;
	UINT8* payload = payloadTemp.GetBuffer();

	//address=param0;
	//offset=param2;
	//length=param3;
	memcpy (MACInteropBuffer, payload, size);

	if (MAC_SendTimeStamped(address, payloadType, (void*) MACInteropBuffer, size, eventTime) == DS_Success)
		retVal = S_Success;
	else
		retVal = E_MacSendError;

    return retVal;
}

void ReceiveDoneCallbackFn(void* msg, UINT16 payloadType)
{
	ManagedCallback(ReceivedCallback, payloadType);
}

void NeighborChangedCallbackFn(INT16 countOfNeighbors)
{
	ManagedCallback(NeighborChangedCallback,(UINT16) countOfNeighbors);
}

void ManagedSendAckCallbackFn(void *msg, UINT16 size, NetOpStatus status, UINT8 radioAckStatus){
	Message_15_4_t* tx_msg = (Message_15_4_t*)msg;
	UINT16 index = MAC_GetMsgIdWithPtr(tx_msg);

	// NetOpStatus needs to be revisted...here we translate from NetOpStatus to the opcodes used in the callback (CallbackTyep in Samraksh_eMote_Net)
	if (status == NetworkOperations_SendInitiated){
		//hal_printf("ManagedSendAckCallbackFn::NetworkOperations_SendInitiated %u %u \r\n", tx_msg->GetHeader()->payloadType, tx_msg->GetHeader()->dest);
		ManagedCallback(((UINT32)(tx_msg->GetHeader()->dest) << 16) + SendInitiated, (((UINT32)index)<<16) + tx_msg->GetHeader()->payloadType);
	} else if (status == NetworkOperations_Success){
		//hal_printf("ManagedSendAckCallbackFn::NetworkOperations_SendACKed %u %u \r\n", tx_msg->GetHeader()->payloadType, tx_msg->GetHeader()->dest);
		MAC_ChangeOwnerShipOfElementwIndex(index, BO_MFUser);
		ManagedCallback(((UINT32)(tx_msg->GetHeader()->dest) << 16) + SendACKed, (((UINT32)index)<<16) + tx_msg->GetHeader()->payloadType);
	} else if (status == NetworkOperations_SendNACKed){
		//hal_printf("ManagedSendAckCallbackFn::NetworkOperations_SendNACKed %u %u  \r\n", tx_msg->GetHeader()->payloadType, tx_msg->GetHeader()->dest);
		ManagedCallback(((UINT32)(tx_msg->GetHeader()->dest) << 16) + SendNACKed, (((UINT32)index)<<16) + tx_msg->GetHeader()->payloadType);
	} else if (status == NetworkOperations_Fail){
		//hal_printf("ManagedSendAckCallbackFn::NetworkOperations_SendFailed %u %u  \r\n", tx_msg->GetHeader()->payloadType, tx_msg->GetHeader()->dest);
		MAC_ChangeOwnerShipOfElementwIndex(index, BO_MFUser);
		ManagedCallback(((UINT32)(tx_msg->GetHeader()->dest) << 16) + SendFailed,  (((UINT32)index)<<16) + tx_msg->GetHeader()->payloadType);
	}
	//else{
	//	hal_printf("ManagedSendAckCallbackFn Unknown Status!");
	//}
}

void ManagedCallback(UINT32 arg1, UINT32 arg2)
{
	UINT32 data1, data2;
	data1 = arg1;
	data2 = arg2;
	//data2 = arg2;

	GLOBAL_LOCK(irq);
	SaveNativeEventToHALQueue( Net_ne_Context, data1, data2 );
}
