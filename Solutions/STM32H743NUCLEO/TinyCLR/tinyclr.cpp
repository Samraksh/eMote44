////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tinyclr_application.h>
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>

#define GPIO_0 _P(B,12)
#define GPIO_1 _P(B,13)
#define GPIO_2 _P(C,12)

#include "Samraksh/SONYC_ML/sonyc_util.h"
static float my_data[8000];
volatile static float my_output[8000];
static unsigned my_count;
static UINT64 now1, now2;

//#include "core_cm7.h"  // Applies to all Cortex-M7

void reset_cnt()
{
    CoreDebug->DEMCR |= 0x01000000;
    DWT->CYCCNT = 0; // reset the counter
    DWT->CTRL = 0; 
}

void start_cnt()
{
    DWT->CTRL |= 0x00000001 ; // enable the counter
}

void stop_cnt()
{
    DWT->CTRL &= 0xFFFFFFFE ; // disable the counter    
}

unsigned getCycles()
{
    return DWT->CYCCNT ;
}

#define DWT_LSR_Present_Msk ITM_LSR_Present_Msk
#define DWT_LSR_Access_Msk ITM_LSR_Access_Msk
#define DWT_LAR_KEY 0xC5ACCE55

void dwt_access_enable( unsigned ena )
{
    uint32_t lsr = DWT->LSR;;

    if( (lsr & DWT_LSR_Present_Msk) != 0 ) 
    {
        if( ena ) 
        {
            if ((lsr & DWT_LSR_Access_Msk) != 0) //locked: access need unlock
            {    
                DWT->LAR = DWT_LAR_KEY;
            }
        } 
        else 
        {
            if ((lsr & DWT_LSR_Access_Msk) == 0) //unlocked
            {   
                DWT->LAR = 0;
            }
        }
    }
}

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

// extern "C" {
// void MX_X_CUBE_AI_Init(void);
// int aiRun(const void *in_data, void *out_data);
// }

//static float in_data[64][51];
//static float out_data[8];

////////////////////////////////////////////////////////////////////////////////
void ApplicationEntryPoint()
{
    CLR_SETTINGS clrSettings;

	//hal_printf(" CLR 20 ");
	// Initial delay to allow UART terminals to start and catch startup messages
    HAL_Delay(6000);

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

	//MX_X_CUBE_AI_Init();
	//aiRun(in_data, out_data);

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
	//I2S_Internal_Initialize();
	//I2S_Test();
    //hal_printf(" CLR 30 ");
    //ClrStartup( clrSettings );
	
	for(int i=0; i<8000; i++)
		my_data[i] = i/(float)8000.0;
	volatile int x = sonyc_init_comp_filter();
	
	reset_cnt();
	now1 = HAL_Time_CurrentTicks();
	start_cnt();
	sonyc_comp_filter_go(my_data, (float *)my_output);
	stop_cnt();
	now2 = HAL_Time_CurrentTicks();
	my_count = getCycles();
	
	hal_printf("Took %u cycles time diff %lu\r\n", my_count, (uint32_t)(now2-now1));

	while(1) {
		while( HAL_CONTINUATION::Dequeue_And_Execute() == TRUE ) ;
		__WFI();
	}

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
