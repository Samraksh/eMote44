#include <Samraksh/Neighbors.h>

DeviceStatus NeighborTable::RecordSenderDelayIncoming(UINT16 MACAddress, const UINT8& delay){
	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(MACAddress, &index);

	if ( (retValue==DS_Success) && ISMAC_VALID(MACAddress)){
		if(Neighbor[index].ReceiveLink.AveDelay == 0) {
			Neighbor[index].ReceiveLink.AveDelay = (UINT8)delay;
		}
		else{
			Neighbor[index].ReceiveLink.AveDelay = (UINT8)((float)Neighbor[index].ReceiveLink.AveDelay*0.8 + (float)delay*0.2);
		}

		return DS_Success;
	}
	else {
		return DS_Fail;
	}
}


DeviceStatus NeighborTable::RecordLastHeardTime(UINT16 MACAddress, UINT64 currTime){
	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(MACAddress, &index);

	if ( (retValue==DS_Success) && ISMAC_VALID(MACAddress)){
		Neighbor[index].LastHeardTime = currTime;
		return DS_Success;
	}
	else {
		return DS_Fail;
	}
}

UINT16 NeighborTable::GetMaxNeighbors(void){
	return MAX_NEIGHBORS;
}

UINT8 NeighborTable::UpdateNeighborTable(UINT64 livelinessDelayInTicks, UINT64 currentTime)
{
	//	UINT8 deadNeighbors = 0;
	//
	////	UINT64 livelinessDelayInTicks = CPU_MillisecondsToTicks(NeighborLivenessDelay * 1000);
	////
	//
	//	//if (Neighbor[0].Status == Alive)
	//	//	hal_printf("neighbor 0 last time: %lld\tcurrent time: %lld\tlivelinessDelayinticks: %lld\r\n", Neighbor[0].LastHeardTime,  currentTime, livelinessDelayInTicks);
	//
	//	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	//	{
	//		if((Neighbor[i].neighborStatus == Alive) && ((currentTime - Neighbor[i].LastHeardTime) > livelinessDelayInTicks) && (Neighbor[i].LastHeardTime != 0))
	//		{
	//
	//			DEBUG_PRINTF_NB("[NATIVE] Neighbors.h : Removing inactive neighbor %hu \n", Neighbor[i].MACAddress);
	//			Neighbor[i].neighborStatus = Dead;
	//			++deadNeighbors;
	//			--NumberValidNeighbor;
	//		}
	//	}
	//
	//	return deadNeighbors;

	return 0;
}

Neighbor_t* NeighborTable::GetMostObsoleteTimeSyncNeighborPtr(NeighborStatus ns){
	Neighbor_t* rn = NULL;
	int tableIndex;
	for (tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if (Neighbor[tableIndex].neighborStatus == ns){
			if(rn == NULL) {
				rn = &Neighbor[tableIndex];
			}
			else if( Neighbor[tableIndex].LastHeardTime != 0 && rn->neighborStatus <= Neighbor[tableIndex].LastHeardTime){
				rn = &Neighbor[tableIndex];
			}
		}
	}
	return rn;
}


UINT8 NeighborTable::UpdateNeighborTable(UINT32 NeighborLivenessDelay)
{
	return BringOutYourDead(NeighborLivenessDelay);
}

DeviceStatus NeighborTable::FindIndexEvenDead(const UINT16 MACAddress, UINT8* index){
	int tableIndex;

	for (tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if ( (Neighbor[tableIndex].MACAddress == MACAddress)) {
			*index = tableIndex;
			return DS_Success;
		}
	}
	return DS_Fail;
}

DeviceStatus NeighborTable::FindIndex(const UINT16 MACAddress, UINT8* index){
	int tableIndex;

	for (tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if ( (Neighbor[tableIndex].MACAddress == MACAddress) && (Neighbor[tableIndex].neighborStatus == Alive)) {
			*index = tableIndex;
			return DS_Success;
		}
	}
	return DS_Fail;
}

UINT8 NeighborTable::BringOutYourDead(UINT32 delay){

	////	GLOBAL_LOCK(irq);
	//
	//	UINT8 deadNeighbors = 0;
	//
	//	UINT64 livelinessDelayInTicks = CPU_MillisecondsToTicks(delay * 1000);
	//
	//	UINT64 currentTime = HAL_Time_CurrentTicks();
	//
	//
	//	//if (Neighbor[0].Status == Alive)
	//	//	hal_printf("neighbor 0 last time: %lld\tcurrent time: %lld\tlivelinessDelayinticks: %lld\r\n", Neighbor[0].LastHeardTime,  currentTime, livelinessDelayInTicks);
	//
	//	for(UINT16 i = 0; i < MAX_NEIGHBORS; i++)
	//	{
	//		if((Neighbor[i].neighborStatus == Alive) && ((currentTime - Neighbor[i].LastHeardTime) > livelinessDelayInTicks) && (Neighbor[i].LastHeardTime != 0))
	//		{
	//			DEBUG_PRINTF_NB("[NATIVE] Neighbors.h : Removing Neighbor %hu due to inactivity\n", Neighbor[i].MACAddress);
	//			Neighbor[i].neighborStatus = Dead;
	//			deadNeighbors++;
	//			NumberValidNeighbor--;
	//		}
	//	}
	//
	//	return deadNeighbors;

	return 0;
}

DeviceStatus NeighborTable::ClearNeighbor(UINT16 nodeId){
	UINT8 tableIndex;
	if (FindIndex(nodeId, &tableIndex) == DS_Success){
		return ClearNeighborwIndex(tableIndex);
	} else {
		return DS_Fail;
	}
}

DeviceStatus NeighborTable::ClearNeighborwIndex(UINT8 index){
	Message_15_4_t* msg_carrier;
	msg_carrier = FindDataPacketForNeighbor(Neighbor[index].MACAddress);
	while(msg_carrier != NULL){
		DeletePacket(msg_carrier);
		msg_carrier = FindDataPacketForNeighbor(Neighbor[index].MACAddress);
	}
	msg_carrier = FindTSRPacketForNeighbor(Neighbor[index].MACAddress);
	while(msg_carrier != NULL){
		DeletePacket(msg_carrier);
		msg_carrier = FindDataPacketForNeighbor(Neighbor[index].MACAddress);
	}

	Neighbor[index].Clear();

	NumberValidNeighbor--;
	return DS_Success;
}
void NeighborTable::ClearTable(){
	int tableIndex;
	for (tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		ClearNeighborwIndex(tableIndex);
	}
	NumberValidNeighbor = 0;
	send_buffer.Initialize();
}

// neighbor table util functions
DeviceStatus NeighborTable::GetFreeIdx(UINT8* index){
	DeviceStatus rv = DS_Fail;
	int tableIndex;
	*index = INVALID_NEIGHBOR_INDEX;

	for (tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if( !ISMAC_VALID(Neighbor[tableIndex].MACAddress) ){
			*index = tableIndex;
			rv = DS_Success;
			break;
		}
		else if (Neighbor[tableIndex].neighborStatus != Alive){
			*index = tableIndex;
			rv = DS_Success;
		}
	}
	if(rv == DS_Success){
		if(ISMAC_VALID(Neighbor[*index].MACAddress)){
			ClearNeighborwIndex(*index);
			rv = DS_Success;
		}
	}
	return rv;
}

Neighbor_t* NeighborTable::GetNeighborPtr(UINT16 address){
	UINT8 index;

	if (FindIndex(address, &index) != DS_Success){
		return NULL;
	}else {
		return &Neighbor[index];
	}
}

UINT8 NeighborTable::NumberOfNeighbors(){
	//return NumberValidNeighbor;
	UINT8 numneigh = 0;
	for (UINT8 tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if (Neighbor[tableIndex].neighborStatus == Alive){
			++numneigh;
		}
	}
	return numneigh;
}

UINT8 NeighborTable::NumberOfNeighborsTotal(){
	//return NumberValidNeighbor;
	UINT8 numneigh = 0;
	for (UINT8 tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if (ISMAC_VALID(Neighbor[tableIndex].MACAddress)){
			++numneigh;
		}
	}
	return numneigh;
}

UINT8 NeighborTable::PreviousNumberOfNeighbors(){
	return PreviousNumberValidNeighbor;
}

void NeighborTable::SetPreviousNumberOfNeighbors(UINT8 previousNeighborCnt){
	PreviousNumberValidNeighbor = previousNeighborCnt;
}

DeviceStatus NeighborTable::FindOrInsertNeighbor(const UINT16 address, UINT8* index){
	DeviceStatus retValue = DS_Fail;
	if(ISMAC_VALID(address)){
		retValue = FindIndexEvenDead(address, index);
		if(retValue == DS_Fail) {
			retValue = GetFreeIdx(index);
			if(retValue == DS_Success) {
				Neighbor[*index].MACAddress = address;
				Neighbor[*index].NumInitializationMessagesSent = 0;
				Neighbor[*index].IsMyScheduleKnown = false;
				DEBUG_PRINTF_NB("[NATIVE] Neighbors.h : Inserting Neighbor %hu.\n", address);
			}
		}
		else{
			if(Neighbor[*index].neighborStatus != Alive){
				Neighbor[*index].NumInitializationMessagesSent = 0;
				Neighbor[*index].IsMyScheduleKnown = false;
			}
		}
	}
	return retValue;
}

DeviceStatus NeighborTable::InsertNeighbor(const NeighborTableCommonParameters_One_t *neighborTableCommonParameters_One_t, const NeighborTableCommonParameters_Two_t *neighborTableCommonParameters_Two_t){
	UINT16 address = neighborTableCommonParameters_One_t->MACAddress;
	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(address, &index);

	if (retValue == DS_Success && ISMAC_VALID(address)){
		NumberValidNeighbor++;
		Neighbor[index].ReceiveLink.AvgRSSI =  0;
	//	Neighbor[index].ReceiveLink.LinkQuality =  0;
		Neighbor[index].ReceiveLink.AveDelay =  0;
		Neighbor[index].SendLink.AvgRSSI =  0;
	//	Neighbor[index].SendLink.LinkQuality =  0;
		Neighbor[index].SendLink.AveDelay =  0;
		retValue = UpdateNeighbor(neighborTableCommonParameters_One_t, neighborTableCommonParameters_Two_t);
	}
	return retValue;
}

DeviceStatus NeighborTable::UpdateLink(UINT16 address, Link_t *forwardLink, Link_t *reverseLink, UINT8* index){
	DeviceStatus retValue = FindIndex(address, index);

	if ((retValue!=DS_Success) && ISMAC_VALID(address)){
		if(forwardLink != NULL){
			Neighbor[*index].SendLink.AveDelay = forwardLink->AveDelay;
			Neighbor[*index].SendLink.AvgRSSI = forwardLink->AvgRSSI;
		//	Neighbor[*index].SendLink.LinkQuality = forwardLink->LinkQuality;
		}
		if(reverseLink != NULL){
			Neighbor[*index].ReceiveLink.AveDelay = reverseLink->AveDelay;
			Neighbor[*index].ReceiveLink.AvgRSSI = reverseLink->AvgRSSI;
		//	Neighbor[*index].ReceiveLink.LinkQuality = reverseLink->LinkQuality;
		}
	}
	return retValue;
}

DeviceStatus NeighborTable::UpdateFrameLength(UINT16 address, NeighborStatus status, UINT16 frameLength, UINT8* index){
	DeviceStatus retValue = FindIndex(address, index);

	if ((retValue!=DS_Success) && ISMAC_VALID(address)){
		Neighbor[*index].FrameLength = frameLength;
	}
	return retValue;
}

DeviceStatus NeighborTable::UpdateDutyCycle(UINT16 address, UINT8 dutyCycle, UINT8* index){
	DeviceStatus retValue = FindIndex(address, index);

	if ((retValue!=DS_Success) && ISMAC_VALID(address)){
		Neighbor[*index].ReceiveDutyCycle = dutyCycle;
	}
	return retValue;
}

DeviceStatus NeighborTable::UpdateNeighbor(const NeighborTableCommonParameters_One_t *neighborTableCommonParameters_One_t, const NeighborTableCommonParameters_Two_t *neighborTableCommonParameters_Two_t){
	UINT16 address = neighborTableCommonParameters_One_t->MACAddress;
	NeighborStatus status = neighborTableCommonParameters_One_t->status;
	UINT64 LastHeardTime = neighborTableCommonParameters_One_t->lastHeardTime;
//	UINT8 lqi = neighborTableCommonParameters_One_t->linkQualityMetrics.LinkQuality;
	UINT8 rssi = neighborTableCommonParameters_One_t->linkQualityMetrics.AvgRSSI;
	UINT8 delay = neighborTableCommonParameters_One_t->linkQualityMetrics.AveDelay;

	if(status == NbrStatusError){
		return DS_Fail;
	}
	if(LastHeardTime == 0){
		return DS_Fail;
	}

	UINT16 nextSeed = neighborTableCommonParameters_Two_t->nextSeed;
	UINT16 mask = neighborTableCommonParameters_Two_t->mask;
	UINT64 nextwakeupSlot = neighborTableCommonParameters_Two_t->nextwakeupSlot;
	UINT32 seedUpdateIntervalinSlots = neighborTableCommonParameters_Two_t->seedUpdateIntervalinSlots;

	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(address, &index);
	if (retValue == DS_Success && ISMAC_VALID(address)){
		Neighbor[index].MACAddress = address;
		Neighbor[index].neighborStatus = status;
		Neighbor[index].LastHeardTime = LastHeardTime;
		Neighbor[index].IsAvailableForUpperLayers= neighborTableCommonParameters_One_t->availableForUpperLayers;
		if (Neighbor[index].ReceiveLink.AvgRSSI == 0) {
			Neighbor[index].ReceiveLink.AvgRSSI = (UINT8)(rssi);
		//	Neighbor[index].ReceiveLink.LinkQuality = (UINT8)lqi;
		} else {
			Neighbor[index].ReceiveLink.AvgRSSI =  (UINT8)((float)Neighbor[index].ReceiveLink.AvgRSSI*0.8 + (float)rssi*0.2);
	//		Neighbor[index].ReceiveLink.LinkQuality =  (UINT8)((float)Neighbor[index].ReceiveLink.LinkQuality*0.8 + (float)lqi*0.2);
		}

		Neighbor[index].nextSeed = nextSeed;
		Neighbor[index].mask = mask;
		Neighbor[index].nextwakeupSlot = nextwakeupSlot;
		Neighbor[index].seedUpdateIntervalinSlots = seedUpdateIntervalinSlots;
	}
	return retValue;
}

DeviceStatus NeighborTable::UpdateNeighbor(const NeighborTableCommonParameters_One_t *neighborTableCommonParameters_One_t){
	UINT16 address = neighborTableCommonParameters_One_t->MACAddress;
	NeighborStatus status = neighborTableCommonParameters_One_t->status;
	UINT64 LastHeardTime = neighborTableCommonParameters_One_t->lastHeardTime;
	//UINT8 lqi = neighborTableCommonParameters_One_t->linkQualityMetrics.LinkQuality;
	UINT8 rssi = neighborTableCommonParameters_One_t->linkQualityMetrics.AvgRSSI;
	UINT8 delay = neighborTableCommonParameters_One_t->linkQualityMetrics.AveDelay;

	if(status == NbrStatusError){
		return DS_Fail;
	}
	if(LastHeardTime == 0){
		return DS_Fail;
	}

	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(address, &index);
	if (retValue == DS_Success && ISMAC_VALID(address)){
		Neighbor[index].ReceiveLink.AvgRSSI =  (UINT8)((float)Neighbor[index].ReceiveLink.AvgRSSI*0.8 + (float)rssi*0.2);
	//	Neighbor[index].ReceiveLink.LinkQuality =  (UINT8)((float)Neighbor[index].ReceiveLink.LinkQuality*0.8 + (float)lqi*0.2);
		Neighbor[index].CountOfPacketsReceived++;
		Neighbor[index].LastHeardTime = LastHeardTime;
		Neighbor[index].neighborStatus = status;
		/*
		Neighbor[index].dataInterval = dataInterval;
		Neighbor[index].radioStartDelay = radioStartDelay;
		Neighbor[index].counterOffset = counterOffset;
		Neighbor[index].lastSeed = seed;
		 */
	}

	return retValue;
}

DeviceStatus NeighborTable::RecordTimeSyncRequestSent(UINT16 address, UINT64 _LastTimeSyncTime){
	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(address, &index);

	if ( (retValue==DS_Success) && ISMAC_VALID(address)){
		Neighbor[index].LastTimeSyncRequestTime = _LastTimeSyncTime;
		return DS_Success;
	}
	else {
		return DS_Fail;
	}
}

DeviceStatus NeighborTable::RecordTimeSyncSent(UINT16 address, UINT64 _LastTimeSyncTime){
	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(address, &index);


	if ( (retValue==DS_Success) && ISMAC_VALID(address)){
		Neighbor[index].LastTimeSyncSendTime = _LastTimeSyncTime;
		return DS_Success;
	}
	else {
		return DS_Fail;
	}
}

/*UINT64 NeighborTable::GetLastTimeSyncRequestTime(UINT16 address){ //TODO BK: We should eventually use a class for Neighbor rather than a struct, this would enable protecting variables.
	 UINT8 index;
	 DeviceStatus retValue = FindIndex(address, &index);

	if ( (retValue==DS_Success) && ISMAC_VALID(address)){
		return(Neighbor[index].LastTimeSyncRequestTime);
	}
	else {
		//Bk: Maybe we should abort or define an uninitialized TimeValue here.
		return 0;
	}
}*/

DeviceStatus NeighborTable::RecordTimeSyncRecv(UINT16 address, UINT64 _LastTimeSyncTime){
	UINT8 index;
	DeviceStatus retValue = FindOrInsertNeighbor(address, &index);


	if ( (retValue==DS_Success) && ISMAC_VALID(address)){
		Neighbor[index].LastTimeSyncRecvTime = _LastTimeSyncTime;
		return DS_Success;
	}
	else {
		return DS_Fail;
	}
}

UINT64 NeighborTable::GetLastTimeSyncRecv(UINT16 address){
	UINT8 index;
	DeviceStatus retValue = FindIndex(address, &index);

	if ( (retValue==DS_Success) && ISMAC_VALID(address)){
		return(Neighbor[index].LastTimeSyncRecvTime);
	}
	else {
		return (0);
	}
}





Neighbor_t* NeighborTable::GetCritalSyncNeighborWOldestSyncPtr(const UINT64& curticks, const UINT64& request_limit, const UINT64& forcererequest_limit, const UINT64& fast_disco_request_interval ){
	Neighbor_t* rn = NULL;
	int tableIndex;
	for (tableIndex=0; tableIndex<MAX_NEIGHBORS; tableIndex++){
		if (Neighbor[tableIndex].neighborStatus != Dead){
			//Get neighbor which has to be sent a timesync packet asap
			if(rn == NULL || Neighbor[tableIndex].LastTimeSyncSendTime < rn->LastTimeSyncSendTime || Neighbor[tableIndex].IsInitializationTimeSamplesNeeded() ){ //Consider this neighbor

				if((curticks - Neighbor[tableIndex].LastTimeSyncSendTime > request_limit || curticks - Neighbor[tableIndex].LastTimeSyncRecvTime > forcererequest_limit || Neighbor[tableIndex].IsInitializationTimeSamplesNeeded() )){

					if(Neighbor[tableIndex].LastTimeSyncRequestTime == 0  || curticks - Neighbor[tableIndex].LastTimeSyncRequestTime  > request_limit || curticks - Neighbor[tableIndex].LastTimeSyncRequestTime  > forcererequest_limit ){
						///rn = &Neighbor[tableIndex];
						///return rn;
						return &Neighbor[tableIndex];
					}
					else if(Neighbor[tableIndex].IsInitializationTimeSamplesNeeded() && (Neighbor[tableIndex].LastTimeSyncRequestTime == 0  || curticks - Neighbor[tableIndex].LastTimeSyncRequestTime  > fast_disco_request_interval)  ){
						///rn = &Neighbor[tableIndex];
						///return rn;
						return &Neighbor[tableIndex];
					}
				}
			}
		}
	}

	return rn;
}

void NeighborTable::DegradeLinks(){
	UINT8 i=0;
	for (i=0; i < NumberValidNeighbor; i++){
		//Neighbor[index].Status = status;
		/*if(Neighbor[i].ReceiveLink.Quality >2){
				Neighbor[i].ReceiveLink.Quality--;
		}*/
	}
}

extern "C"
{
	void ClearMsgContents(Message_15_4_t* msg){
		memset(msg,0,sizeof(Message_15_4_t));
	}
}

