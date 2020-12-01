////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tinyclr_application.h>
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>
#include <Samraksh/serial_frame_pal.h>
#include "Samraksh/SONYC_ML/sonyc_util.h" // for sonyc filter init

#include "network_id.h"

#define GPIO_0 _P(B,12)
#define GPIO_1 _P(B,13)
#define GPIO_2 _P(C,12)

// Debug functions for timing
static void reset_cnt()
{
    CoreDebug->DEMCR |= 0x01000000;
    DWT->CYCCNT = 0; // reset the counter
    DWT->CTRL = 0;
}

static void start_cnt()
{
    DWT->CTRL |= 0x00000001 ; // enable the counter
}

static void stop_cnt()
{
    DWT->CTRL &= 0xFFFFFFFE ; // disable the counter
}

static unsigned getCycles()
{
    return DWT->CYCCNT ;
}
// End Debug functions for timing

extern void HAL_CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents);

void CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents)
{
    HAL_CPU_Sleep(level, wakeEvents);
}

void Timer_Green_Handler(void *arg)
{
	static bool state = FALSE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED1, state);
}

void Timer_Red_Handler(void *arg)
{
	static bool state = TRUE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED3, state);
}

void Timer_RTC_Handler(void *arg)
{

	CPU_GPIO_SetPinState(GPIO_0, TRUE);
	CPU_GPIO_SetPinState(GPIO_0, FALSE);
}

void Timer_1_Handler(void *arg)
{
	CPU_GPIO_SetPinState(GPIO_1, TRUE);
	CPU_GPIO_SetPinState(GPIO_1, FALSE);
	//uint64_t time = CPU_Timer_CurrentTicks(LPTIM);
	//hal_printf("%llu\r\n", time);
}

void nathan_rtc_handler(void *arg) {
	static unsigned i=0;
	if (i++ & 1)
		CPU_GPIO_SetPinState(GPIO_2, TRUE);
	else
		CPU_GPIO_SetPinState(GPIO_2, FALSE);
}

extern "C" void MX_FMC_Init(void); // TODO MAKE NOT STUPID

////////////////////////////////////////////////////////////////////////////////
void ApplicationEntryPoint()
{
    CLR_SETTINGS clrSettings;
	MX_FMC_Init();
	framed_serial_init();
	sonyc_init_filters();

	// print the CPU UID and the hash16 used for network ID
	const uint8_t *uid = get_cpu_uid();
	uint16_t uid_hash = get_cpu_uid_hash16();
	if (get_cpu_uid_len() > 0) {
		hal_printf("CPU UID: 0x");
		for(int i=0; i < get_cpu_uid_len()-1; i++) {
			hal_printf("%.2X", uid[i]);
		}
		hal_printf("%.2X\r\n", uid[get_cpu_uid_len()-1]);
	}
	hal_printf("Device Network ID %u (0x%.4X)\r\n", uid_hash, uid_hash);

	// Initial delay to allow UART terminals to start and catch startup messages
    //HAL_Delay(5000);

	memset(&clrSettings, 0, sizeof(CLR_SETTINGS));

    clrSettings.MaxContextSwitches         = 50;
    clrSettings.WaitForDebugger            = false;
    clrSettings.EnterDebuggerLoopAfterExit = true;

	//CPU_GPIO_EnableOutputPin(LED1, TRUE);
    //CPU_GPIO_EnableOutputPin(LED3, TRUE);
	//CPU_GPIO_EnableOutputPin(GPIO_2, FALSE);

	// VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 500000, FALSE, FALSE, Timer_Green_Handler);
	// VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	// VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 500000, FALSE, FALSE, Timer_Red_Handler, LOW_DRIFT_TIMER);
	// VirtTimer_Start(VIRT_TIMER_LED_RED);

	//VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 100000, FALSE, FALSE, nathan_rtc_handler, LOW_DRIFT_TIMER);
	//VirtTimer_Start(VIRT_TIMER_LED_RED);

	//CPU_GPIO_EnableOutputPin(GPIO_0, FALSE);
	//CPU_GPIO_EnableOutputPin(GPIO_1, FALSE);
/*	CPU_GPIO_EnableOutputPin(GPIO_2, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_3, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_4, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_5, FALSE);
*/

	/*VirtTimer_SetTimer(VIRT_TIMER_TIME_TEST, 0, 1000000, FALSE, FALSE, Timer_1_Handler);
	VirtTimer_Start(VIRT_TIMER_TIME_TEST);
	VirtTimer_SetTimer(VIRT_TIMER_RTC_TEST, 0, 450000, FALSE, FALSE, Timer_RTC_Handler, LOW_DRIFT_TIMER);
	VirtTimer_Start(VIRT_TIMER_RTC_TEST);*/
	//VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 800000, FALSE, FALSE, Timer_Green_Handler, LOW_DRIFT_TIMER);
	//VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	I2S_Internal_Initialize();
    ClrStartup( clrSettings );

	// while(1) {
		// while( HAL_CONTINUATION::Dequeue_And_Execute() == TRUE ) ;
		// __WFI();
	// }

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
