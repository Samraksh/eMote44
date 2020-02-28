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


#include "Samraksh_Mel.h"
#include "Samraksh_Mel_Samraksh_Mel_Radio_Radio_802_15_4_Base.h"
#include <Samraksh/Radio_decl.h>

using namespace Samraksh_Mel::Radio;

INT32 Radio_802_15_4_Base::UnInitialize( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, HRESULT &hr )
{
	DeviceStatus status = DS_Fail;
	BOOL ret = CPU_Radio_UnInitialize(radioName);
	if(ret)
		status = DS_Success;
	else
		status = DS_Fail;

	return status;
}

UINT16 Radio_802_15_4_Base::GetRadioAddress( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, HRESULT &hr )
{
	UINT16 address;
	address = CPU_Radio_GetAddress(radioName);
	return address;
}

INT8 Radio_802_15_4_Base::SetRadioAddress( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, UINT16 address, HRESULT &hr )
{
	BOOL status;
	status = CPU_Radio_SetAddress(radioName, address);
	return status;
}

INT32 Radio_802_15_4_Base::SetRadioName( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, HRESULT &hr )
{
	DeviceStatus status;
	//BOOL ret = MAC_SetRadioType(radioName);
	BOOL ret = CPU_Radio_SetRadioName(radioName);
	if(ret)
		status = DS_Success;
	else
		status = DS_Fail;

	return status;
}

INT32 Radio_802_15_4_Base::SetTxPower( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, INT32 power, HRESULT &hr )
{
	DeviceStatus status;
	//BOOL ret = MAC_SetRadioTxPower(power);
	BOOL ret = CPU_Radio_ChangeTxPower(radioName, power);
	if(ret)
		status = DS_Success;
	else
		status = DS_Fail;

	return status;
}

INT32 Radio_802_15_4_Base::SetChannel( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, INT32 channel, HRESULT &hr )
{
	DeviceStatus status;
	//BOOL ret = MAC_SetRadioChannel(channel);
	BOOL ret = CPU_Radio_ChangeChannel(radioName, channel);
	if(ret)
		status = DS_Success;
	else
		status = DS_Fail;

	return status;
}

INT32 Radio_802_15_4_Base::TurnOnRx( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, HRESULT &hr )
{
	DeviceStatus status;
	status = CPU_Radio_TurnOnRx(radioName);
	return status;
}

INT32 Radio_802_15_4_Base::TurnOffRx( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, HRESULT &hr )
{
	DeviceStatus status;
	status = CPU_Radio_TurnOffRx(radioName);
	return status;
}

INT32 Radio_802_15_4_Base::Sleep( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, UINT8 level, HRESULT &hr )
{
	DeviceStatus status;
	status = CPU_Radio_Sleep(radioName, level);
	return status;
}

INT32 Radio_802_15_4_Base::PreLoad( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 messageBuffer, UINT16 size, HRESULT &hr )
{
	UINT8* managedBuffer = messageBuffer.GetBuffer();
	NetOpStatus status = CPU_Radio_PreloadMessage(managedBuffer, size);
    return status;
}

INT32 Radio_802_15_4_Base::SendStrobe( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, UINT16 size, HRESULT &hr )
{
	NetOpStatus status = NetworkOperations_Success;
	void* returnMsg = CPU_Radio_SendStrobe(radioName, size);
	if(returnMsg == NULL){
		status = NetworkOperations_Fail;
	}

    return status;
}

INT32 Radio_802_15_4_Base::Send( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, CLR_RT_TypedArray_UINT8 message, UINT16 size, HRESULT &hr )
{
	void* msg;
	msg = CPU_Radio_Send(radioName, message.GetBuffer(), size);
	if(msg == NULL)
		return -1;
	else
		return 0;
}

INT32 Radio_802_15_4_Base::SendTimeStamped( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, CLR_RT_TypedArray_UINT8 message, UINT16 size, UINT32 eventTime, HRESULT &hr )
{
	void* msg;
	msg = CPU_Radio_Send_TimeStamped(radioName, message.GetBuffer(), size, eventTime);
	if(msg == NULL)
		return -1;
	else
		return 0;
}

INT8 Radio_802_15_4_Base::ClearChannelAssesment( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, HRESULT &hr )
{
	DeviceStatus status;

	status = CPU_Radio_ClearChannelAssesment(0);
	if(status == DS_Success){
		return true;
	}
	else
		return false;

	//return status;
}

INT8 Radio_802_15_4_Base::ClearChannelAssesment( CLR_RT_HeapBlock* pMngObj, UINT8 radioName, UINT16 numberOfMicroSecond, HRESULT &hr )
{
	DeviceStatus status;
	status = CPU_Radio_ClearChannelAssesment(radioName, numberOfMicroSecond);
	return status;
}
