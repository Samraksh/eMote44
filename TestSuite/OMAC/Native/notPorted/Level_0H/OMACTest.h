/*
 * OMACTest.h
 */

#ifndef OMACTEST_H_
#define OMACTEST_H_

#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/MAC/OMAC/RadioControl.h>

//#include <Samraksh/Mac_decl.h>
//
//#include <Samraksh/MAC/OMAC/OMAC.h>
//
//#include <Samraksh/Message.h>

typedef struct  {
	UINT16 MSGID;
	UINT8 data[5];
}Payload_t;

class OMACTest{
public:

	RadioEventHandler_t Radio_Event_Handler;
	MacConfig Config;
	RadioControl_t radio_cont;
	bool my_radio_state;
	BOOL Initialize();
	BOOL StartTest();
	DeviceStatus StartRx();


};

OMACTest g_OMACTest;

void OMACTest_Initialize();

#endif /* OMACTEST_H_ */



