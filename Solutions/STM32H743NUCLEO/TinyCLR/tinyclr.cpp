////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tinyclr_application.h>
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>

extern void HAL_CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents);

void CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents)
{
    HAL_CPU_Sleep(level, wakeEvents);
}

void Timer_Green_Handler(void *arg)
{
	static bool state = TRUE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_EnableOutputPin(LED1, state);
	//CPU_GPIO_SetPinState(GPIO_0, TRUE);
	//CPU_GPIO_SetPinState(GPIO_0, FALSE);
}

void Timer_Red_Handler(void *arg)
{
	static bool state = TRUE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_EnableOutputPin(LED3, state);
	/*CPU_GPIO_SetPinState(GPIO_4, TRUE);
	CPU_GPIO_SetPinState(GPIO_4, FALSE);
	CPU_GPIO_SetPinState(GPIO_3, TRUE);
	CPU_GPIO_SetPinState(GPIO_3, FALSE);
	CPU_GPIO_SetPinState(GPIO_5, TRUE);
	CPU_GPIO_SetPinState(GPIO_5, FALSE);*/
}

////////////////////////////////////////////////////////////////////////////////
void ApplicationEntryPoint()
{
    CLR_SETTINGS clrSettings;

	//hal_printf(" CLR 20 ");
    
	memset(&clrSettings, 0, sizeof(CLR_SETTINGS));

    clrSettings.MaxContextSwitches         = 50;
    clrSettings.WaitForDebugger            = false;
    clrSettings.EnterDebuggerLoopAfterExit = true;

	VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 500000, FALSE, FALSE, Timer_Green_Handler);
	VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 1000000, FALSE, FALSE, Timer_Red_Handler);
	VirtTimer_Start(VIRT_TIMER_LED_RED);
	
//	I2S_Internal_Initialize();
//	I2S_Test();
  //  hal_printf(" CLR 28 ");
    ClrStartup( clrSettings );

#if !defined(BUILD_RTM)
    debug_printf( "Exiting.\r\n" );
#else
    ::CPU_Reset();
#endif
}

BOOL Solution_GetReleaseInfo(MfReleaseInfo& releaseInfo)
{
    MfReleaseInfo::Init(releaseInfo,
                        VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD, VERSION_REVISION,
                        OEMSYSTEMINFOSTRING, hal_strlen_s(OEMSYSTEMINFOSTRING)
                        );
    return TRUE; // alternatively, return false if you didn't initialize the releaseInfo structure.
}

