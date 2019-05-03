////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TinyBooter.h"
#include "TinyCLR_Version.h"
#include <tinybooterentry.h>
#include "ConfigurationManager.h"

extern bool WaitForTinyBooterUpload( INT32 &timeout_ms );


////////////////////////////////////////////////////////////////////////////////

//--//


Loader_Engine g_eng;

//--//

HAL_DECLARE_CUSTOM_HEAP( SimpleHeap_Allocate, SimpleHeap_Release, SimpleHeap_ReAllocate );

//--//

void ApplicationEntryPoint()
{
    INT32 timeout       = 20000; // 20 second timeout
    bool  enterBootMode = false;

    // crypto API needs to allocate memory. Initialize simple heap for it. 
    UINT8* BaseAddress;                                 
    UINT32 SizeInBytes;                                 
                                                        
    HeapLocation         ( BaseAddress, SizeInBytes );  
    SimpleHeap_Initialize( BaseAddress, SizeInBytes );  

    g_eng.Initialize( HalSystemConfig.DebuggerPorts[ 0 ] );

    // internal reset and stop check
    enterBootMode = g_PrimaryConfigManager.IsBootLoaderRequired( timeout );

    // ODM defined method to enter bootloader mode
    if(!enterBootMode)
    {
        enterBootMode = WaitForTinyBooterUpload( timeout );
    }
    if(!enterBootMode)   
    {
        if(!g_eng.EnumerateAndLaunch())
        {
            timeout       = -1;
            enterBootMode = true;
        }
    }

    if(enterBootMode)
    {
       // LCD_Clear();
        
      //  hal_fprintf( STREAM_LCD, "TinyBooter v%d.%d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_REVISION);
       // hal_fprintf( STREAM_LCD, "%s Build Date:\r\n\t%s %s\r\n", HalName, __DATE__, __TIME__ );

        DebuggerPort_Initialize( HalSystemConfig.DebuggerPorts[ 0 ] );
		//hal_printf(" 64 TinyBooter.cpp \n");
		//USART_Initialize( ConvertCOM_ComPort(USART_DEFAULT_PORT), HalSystemConfig.USART_DefaultBaudRate, USART_PARITY_NONE, 8, USART_STOP_BITS_ONE, USART_FLOW_NONE );
	    //USART_Write(ConvertCOM_ComPort(USART_DEFAULT_PORT), "test\n", 10);
		
		//CPU_USART_WriteCharToTxBuffer(ConvertCOM_ComPort(USART_DEFAULT_PORT), 'a' );
		//CPU_SPI_PortsCount();
		//hal_printf(" 66 TinyBooter.cpp \n");
        
		SPI_CONFIGURATION testabc;
		testabc.Clock_RateKHz      = 100;
        testabc.CS_Active          = 0;
        testabc.CS_Hold_uSecs      = 0;
        testabc.CS_Setup_uSecs     = 0;
        testabc.DeviceCS           = 0;
        testabc.MD_16bits          = 0; 
        testabc.MSK_SampleEdge     = 0;
        testabc.SPI_mod            = 0;

		hal_printf(" 82 TinyBooter.cpp \n");		
		
	    UINT8 toBuffer[4];
		UINT8 fromBuffer[8];
		toBuffer[0] = 1;
		toBuffer[1] = 0;
		toBuffer[2] = 0;
		toBuffer[3] = 1;
		
		CPU_SPI_Xaction_Start(testabc);
		hal_printf(" 93 TinyBooter.cpp %d\n",HAL_Time_CurrentTicks());
		//CPU_SPI_PortsCount();
		CPU_SPI_nWrite8_nRead8(testabc, toBuffer, 4, fromBuffer, 0, 0);
		hal_printf(" 88 TinyBooter.cpp \n");
		CPU_SPI_Xaction_Stop(testabc);
		//HAL_Time_Sleep_MicroSeconds(10000000);
		hal_printf(" 99 TinyBooter.cpp %d\n",HAL_Time_CurrentTicks());
		//CPU_USART_WriteCharToTxBuffer(ConvertCOM_ComPort(USART_DEFAULT_PORT), 'a' );
			
		TinyBooter_OnStateChange( State_EnterBooterMode, NULL );

	
        DebuggerPort_Flush( HalSystemConfig.DebugTextPort  );
        hal_printf( "TinyBooter v%d.%d.%d.%d\r\n", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_REVISION);
        hal_printf( "%s Build Date: %s %s\r\n", HalName, __DATE__, __TIME__ );
#if defined(__GNUC__)
        hal_printf("GNU Compiler version %d\r\n", __GNUC__);
#elif defined(_ARC)
        hal_printf("ARC Compiler version %d\r\n", _ARCVER);
#elif defined(__ADSPBLACKFIN__)
        hal_printf( "Blackfin Compiler version %d\r\n", __VERSIONNUM__ );
#elif defined(__RENESAS__)
        hal_printf( "Renesas Compiler version %d\r\n", __RENESAS_VERSION__ );
#else
        hal_printf( "ARM Compiler version %d\r\n", __ARMCC_VERSION );
#endif
        DebuggerPort_Flush( HalSystemConfig.DebugTextPort );
		//hal_printf(" 87 TinyBooter.cpp \n");
        //
        // Send "presence" ping.
        //
        {
            CLR_DBG_Commands::Monitor_Ping cmd;

            cmd.m_source = CLR_DBG_Commands::Monitor_Ping::c_Ping_Source_TinyBooter;

            g_eng.m_controller.SendProtocolMessage( CLR_DBG_Commands::c_Monitor_Ping, WP_Flags::c_NonCritical, sizeof(cmd), (UINT8*)&cmd );
        }
    
        UINT64 ticksStart = HAL_Time_CurrentTicks();

        //
        // Wait for somebody to press a button; if no button press comes in, lauch the image
        //
        do
        {
            const UINT32 c_EventsMask = SYSTEM_EVENT_FLAG_COM_IN |
                                        SYSTEM_EVENT_FLAG_USB_IN |
                                        SYSTEM_EVENT_FLAG_BUTTON;

            UINT32 events = ::Events_WaitForEvents( c_EventsMask, timeout );

            if(events != 0)
            {
                Events_Clear( events );
            }

            if(events & SYSTEM_EVENT_FLAG_BUTTON)
            {
                TinyBooter_OnStateChange( State_ButtonPress, (void*)&timeout );
            }
            
            if(events & (SYSTEM_EVENT_FLAG_COM_IN | SYSTEM_EVENT_FLAG_USB_IN))
            {
                g_eng.ProcessCommands();
            }

            if(LOADER_ENGINE_ISFLAGSET(&g_eng, Loader_Engine::c_LoaderEngineFlag_ValidConnection))
            {
                LOADER_ENGINE_CLEARFLAG(&g_eng, Loader_Engine::c_LoaderEngineFlag_ValidConnection);
                
                TinyBooter_OnStateChange( State_ValidCommunication, (void*)&timeout );

                ticksStart = HAL_Time_CurrentTicks();
            }
            else if((timeout != -1) && (HAL_Time_CurrentTicks()-ticksStart) > CPU_MillisecondsToTicks((UINT32)timeout))
            {
                TinyBooter_OnStateChange( State_Timeout, NULL );
                g_eng.EnumerateAndLaunch();
            }
        } while(true);
    }

    ::CPU_Reset();
}
