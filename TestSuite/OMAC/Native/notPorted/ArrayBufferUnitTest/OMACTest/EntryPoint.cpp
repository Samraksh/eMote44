////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) The Samraksh Company.  All rights reserved.
// This is an auto generated file by the test create tool
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define __regtest

#include <tinyhal.h>
#include "../../ArrayBufferUnitTest/OMACTest/OMACTest.h"



void Keep_Linker_Happy() {
	BootstrapCode();
	BootEntry();
}


//HAL_DECLARE_CUSTOM_HEAP( CLR_RT_Memory::Allocate, CLR_RT_Memory::Release, CLR_RT_Memory::ReAllocate );

void PostManagedEvent( UINT8 category, UINT8 subCategory, UINT16 data1, UINT32 data2 )
{
}

void ApplicationEntryPoint()
{
    BOOL result;
    

    do
    {

    	OMACTest_Initialize();

    } while(false); // run only once!

    while(true){
    	::Events_WaitForEvents(0, 100);
    }
}
