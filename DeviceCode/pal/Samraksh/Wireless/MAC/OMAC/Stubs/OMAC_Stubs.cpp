/*
 * OMAC_Stubs.cpp
 *
 *  Created on: Mar 22, 2017
 *      Author: Bora
 */

#include <Samraksh/MAC/OMAC/OMAC.h>

OMACType g_OMAC;


/*Already set in MAC class from which OMAC is derived
	UINT16 GetAddress(){return MyID;}
	BOOL SetAddress(UINT16 address){
		MyAddress = address;
		return TRUE;
	}*/

BOOL SetRadioAddress(UINT16 address){
	return false;
}

UINT16 GetRadioAddress(){
	return 0;
}

BOOL SetRadioName(UINT8 radioName){
	return false;
}

BOOL SetRadioTxPower(int power){
	return false;
}

BOOL SetRadioChannel(int channel){
	return false;
}

UINT16 GetMaxPayload(){
	return 0;
}
void SetMaxPayload(UINT16 payload){
}

void SetCurrentActiveApp(UINT8 CurrentActiveApp){

}

UINT8 GetCurrentActiveApp(){
	return 0;
}

void SetMyID(UINT16 MyID){

}

UINT16 GetMyID(){
	return 0;
}

//Override base class methods here, implement them later in cpp file
DeviceStatus Initialize(MACEventHandler* eventHandler, UINT8 macName, UINT8 routingAppID, UINT8 radioID, MACConfig *config){
	return DS_Fail;
}
DeviceStatus SetConfig(MACConfig *config){return DS_Fail;}
DeviceStatus SetOMACParametersBasedOnRadioName(UINT8 radioID){return DS_Fail;}
BOOL Send(UINT16 dest, UINT8 dataType, void* msg, int size){return false;}
BOOL SendTimeStamped(UINT16 dest, UINT8 dataType, void* msg, int Size, UINT32 eventTime) {return false;}

Message_15_4_t* ReceiveHandler(Message_15_4_t* msg, int size){ return NULL;}


BOOL RadioInterruptHandler(RadioInterrupt Interrupt, void* Param){return false;}
void SendAckHandler(void* msg, int Size, NetOpStatus status){ }
BOOL UnInitialize(void) {return false;}
BOOL HandleBroadcastMessage(Message_15_4_t * msg){ return false;}
BOOL HandleUnicastMessage(Message_15_4_t * msg){return false;}
Message_15_4_t* FindFirstSyncedNbrMessage(){return NULL;}
Message_15_4_t* FindFirstMessageForNbr(UINT16 nbr){ return NULL;}
UINT8 UpdateNeighborTable(){return 0;}
UINT8 GetSendBufferSize() {return 0;}
UINT8 GetReceiveBufferSize(){ return 0;}
UINT16 GetSendPending() { return 0;}
UINT16 GetReceivePending() {return 0;}

void PushPacketsToUpperLayers(void* arg){}


