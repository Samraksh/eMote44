#ifndef _LinkTest_H_
#define _LinkTest_H_


#include <tinyhal.h>
#include <Samraksh/MAC_decl.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Message.h>
#include <Samraksh/Radio_decl.h>



struct MyMessage{
	UINT64 packetID;
	UINT8 payload[97];
};

class LinkTest
{
	UINT8 MyAppID;
	MACEventHandler myEventHandler;
	MACConfig Config;
	UINT8 MacId;


	UINT64 sent_packet_count[MAX_NEIGHBORS];
public:
	UINT32 rx_packet_count[MAX_NEIGHBORS];

	LinkTest ( int seedValue, int numberOfEvents );
    BOOL     Execute( int testLevel );
    void Initialize();
    void SendMsg();

};




#endif //_LinkTest_H_
