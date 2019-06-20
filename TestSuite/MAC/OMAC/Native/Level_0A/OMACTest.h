/*
 * OMACTest.h
 */

#ifndef OMACTest_H_
#define OMACTest_H_



#include <tinyhal.h>
#include <Samraksh/MAC/OMAC/OMAC.h>
#include <Samraksh/PacketTimeSync_15_4.h>
#include <Samraksh/GlobalTime.h>

class OMACTest{
public:
	OMACTest();
	GlobalTime m_globalTime;
	UINT8 MyAppID;
	MacEventHandler myEventHandler;
	MacConfig Config;
	UINT8 MacId;
	BOOL LocalClkPINState;
	BOOL NeighborClkPINState;
	BOOL IsLocalCLKScheduled;
	BOOL IsNeighborCLKScheduled;


	BOOL Initialize();
	BOOL StartTest();

	BOOL ScheduleNextNeighborCLK();
	BOOL ScheduleNextLocalCLK();
};

OMACTest gOMACTest;

void OMACTest_Initialize();


#endif /* OMACTest_H_ */


