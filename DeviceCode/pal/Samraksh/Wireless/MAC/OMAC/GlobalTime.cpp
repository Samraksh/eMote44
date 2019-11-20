/*
 * GlobalTime.cpp
 *
 *  Created on: Sep 5, 2019
 *      Author: mukun
 */
#include <Samraksh/GlobalTime.h>


void GlobalTime::Init(){
	regressgt2.Init();
	leader = 0xFFFF;
	offset=0;
}


UINT64 GlobalTime::Neighbor2LocalTime(UINT16 nbr, const UINT64& nbrTime){
	return regressgt2.Neighbor2LocalTime(nbr, nbrTime);
}

UINT64 GlobalTime::Local2NeighborTime(UINT16 nbr, const UINT64& curtime){
	return regressgt2.Local2NeighborTime(nbr, curtime);
}

UINT64 Regression::Neighbor2LocalTime(UINT16 nbr, const UINT64& nbrTime){
	if (NumberOfRecordedElements(nbr) < 2) return(HAL_Time_CurrentTicks());
	NeighborIndex_t nbrIndex = FindNeighbor(nbr);
	if(nbrIndex == c_bad_nbrIndex) {
		ASSERT_SP(0);
		return 0;
	}
	/*UINT64 lastrecordedTime = regressgt2.samples[nbrIndex].recordedTime[regressgt2.samples[nbrIndex].lastTimeIndex];
	UINT64 periodlength;
	bool negativeperiod = FALSE;
	//Check roll over
	if(nbrTime < lastrecordedTime) {
		if( lastrecordedTime - nbrTime > ((MAXRangeUINT64)/2)) { //Roll Over
			periodlength = (MAXRangeUINT64 - lastrecordedTime) + nbrTime;
		}
		else{ // Negative Interval
			periodlength = lastrecordedTime - nbrTime;
			negativeperiod = TRUE;
		}
	}
	else { // No rollover no negative
		periodlength = nbrTime - lastrecordedTime;
	}
	// Calculate the time
	lastrecordedTime = lastrecordedTime - regressgt2.samples[nbrIndex].offsetBtwNodes[regressgt2.samples[nbrIndex].lastTimeIndex];
	if (negativeperiod) lastrecordedTime = lastrecordedTime - (((float) periodlength)  * regressgt2.samples[nbrIndex].relativeFreq);
	else lastrecordedTime = lastrecordedTime + (((float) periodlength)  * regressgt2.samples[nbrIndex].relativeFreq);
	return (lastrecordedTime);*/
	return ( ( (double)nbrTime - samples[nbrIndex].y_intercept - samples[nbrIndex].additional_y_intercept_offset)/samples[nbrIndex].relativeFreq );
}

UINT64 Regression::Local2NeighborTime(UINT16 nbr, const UINT64& curtime){
	if (NumberOfRecordedElements(nbr) < 2) return(0);
	NeighborIndex_t nbrIndex = FindNeighbor(nbr);
	if(nbrIndex == c_bad_nbrIndex) {
		ASSERT_SP(0);
		return 0;
	}
	/*//UINT64 curtime = HAL_Time_CurrentTime();
	UINT8 nbrIndex = regressgt2.FindNeighbor(nbr);
	UINT64 periodlength;
	bool negativeperiod = FALSE;
	UINT64 lastlocalTime;
	// Get last sample
	if (regressgt2.samples[nbrIndex].offsetBtwNodes[regressgt2.samples[nbrIndex].lastTimeIndex] > 0) {
		lastlocalTime = regressgt2.samples[nbrIndex].recordedTime[regressgt2.samples[nbrIndex].lastTimeIndex] - (UINT64) regressgt2.samples[nbrIndex].offsetBtwNodes[regressgt2.samples[nbrIndex].lastTimeIndex];
	}
	else {
		lastlocalTime = regressgt2.samples[nbrIndex].recordedTime[regressgt2.samples[nbrIndex].lastTimeIndex] + (UINT64)((regressgt2.samples[nbrIndex].offsetBtwNodes[regressgt2.samples[nbrIndex].lastTimeIndex])*-1);
	}
	//Check roll over
	if(curtime < lastlocalTime) {
		if( lastlocalTime - curtime > ((MAXRangeUINT64)/2)) { //Roll Over
			periodlength = (MAXRangeUINT64 - lastlocalTime) + curtime;
		}
		else{ // Negative Interval
			periodlength = lastlocalTime - curtime;
			negativeperiod = TRUE;
		}
	}
	else { // No rollover no negative
		periodlength = curtime - lastlocalTime;
	}
	// Calculate the time
	lastlocalTime = regressgt2.samples[nbrIndex].recordedTime[regressgt2.samples[nbrIndex].lastTimeIndex];
	if (negativeperiod) lastlocalTime = lastlocalTime - (((float) periodlength) / regressgt2.samples[nbrIndex].relativeFreq);
	else lastlocalTime = lastlocalTime + (((float) periodlength) / regressgt2.samples[nbrIndex].relativeFreq);
	return (lastlocalTime);*/
	return ( samples[nbrIndex].relativeFreq * (double)curtime + samples[nbrIndex].y_intercept + samples[nbrIndex].additional_y_intercept_offset);
}
