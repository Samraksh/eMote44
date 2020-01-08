////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <tinyclr_application.h>
#include <tinyhal.h>
#include <Samraksh/VirtualTimer.h>

//#define GPIO_0 _P(B,12)
//#define GPIO_1 _P(B,13)

// long long unsigned integer
static void x64toa2(unsigned long long val, char *buf, unsigned radix, int is_neg)
{
  char *p;
  char *firstdig;
  char temp;
  unsigned digval;
  p = buf; *p=0,p[1]='\0',p;
  if (val==0||radix<2||radix>32||radix&1) return; 
  if ( is_neg )  *p++ = '-', val = (unsigned long long)(-(long long)val);
  firstdig = p;
  if(radix--==10)
    do { // optimized for fixed division
    digval = (unsigned) (val % 10);
    val /= 10;
    *p++ = (char) (digval + '0');
    } while (val > 0);
  else do { temp=radix;
    digval = (unsigned) (val & radix );
    while(temp) val>>=1,temp>>=1;
    *p++ = digval>9?(char)(digval + 'W'):(char) (digval + '0');
  } while (val>0);
  *p-- = '\0';

  do { // reverse string
    temp = *p;
    *p = *firstdig;
    *firstdig = temp;
    --p;
    ++firstdig;
  } while (firstdig < p);
}

//----------------------------------------------------------------------------
char* _i64toa2(long long val, char *buf, int radix)
{
  x64toa2((unsigned long long)val, buf, radix, (radix == 10 && val < 0));
  return buf;
}

//----------------------------------------------------------------------------
char* _ui64toa2(unsigned long long val, char *buf, int radix)
{
  x64toa2(val, buf, radix, 0);
  return buf;
}

char* l2s2(long long v,int sign) { 
	char r,s;
	static char buff[33];  
	r=sign>>8; 
	s=sign;
	if(!r) r=10; 
	if(r!=10||s&&v>=0) s=0; 
	if(r<8) r=0;
	x64toa2(v,buff,r,s); 
	return buff;
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
	

	
	//CPU_GPIO_SetPinState(GPIO_0, TRUE);
	//CPU_GPIO_SetPinState(GPIO_0, FALSE);
}

void Timer_Red_Handler(void *arg)
{
	static bool state = FALSE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED3, state);

//	CPU_GPIO_SetPinState(GPIO_0, TRUE);
//	CPU_GPIO_SetPinState(GPIO_0, FALSE);
//	CPU_GPIO_SetPinState(GPIO_1, TRUE);
//	CPU_GPIO_SetPinState(GPIO_1, FALSE);
//	CPU_GPIO_SetPinState(GPIO_2, TRUE);
//	CPU_GPIO_SetPinState(GPIO_2, FALSE);
	
	/*CPU_GPIO_SetPinState(GPIO_4, TRUE);
	CPU_GPIO_SetPinState(GPIO_4, FALSE);
	CPU_GPIO_SetPinState(GPIO_3, TRUE);
	CPU_GPIO_SetPinState(GPIO_3, FALSE);
	CPU_GPIO_SetPinState(GPIO_5, TRUE);
	CPU_GPIO_SetPinState(GPIO_5, FALSE);*/
}

void Timer_RTC_Handler(void *arg)
{

	CPU_GPIO_SetPinState(GPIO_0, TRUE);
	CPU_GPIO_SetPinState(GPIO_0, FALSE);
	/*static bool state = FALSE;
	if (state)
		state = FALSE;
	else
		state = TRUE;
	CPU_GPIO_SetPinState(LED2, state);
	*/
	//hal_printf("test\n");

//	CPU_GPIO_SetPinState(GPIO_0, TRUE);
//	CPU_GPIO_SetPinState(GPIO_0, FALSE);
//	CPU_GPIO_SetPinState(GPIO_1, TRUE);
//	CPU_GPIO_SetPinState(GPIO_1, FALSE);
//	CPU_GPIO_SetPinState(GPIO_2, TRUE);
//	CPU_GPIO_SetPinState(GPIO_2, FALSE);
	
	/*CPU_GPIO_SetPinState(GPIO_3, TRUE);
	CPU_GPIO_SetPinState(GPIO_3, FALSE);
	CPU_GPIO_SetPinState(GPIO_4, TRUE);
	CPU_GPIO_SetPinState(GPIO_4, FALSE);
	CPU_GPIO_SetPinState(GPIO_5, TRUE);
	CPU_GPIO_SetPinState(GPIO_5, FALSE);*/
}

void Timer_1_Handler(void *arg)
{
	//CPU_GPIO_SetPinState(GPIO_1, TRUE);
	//CPU_GPIO_SetPinState(GPIO_1, FALSE);
	
	hal_printf("\r\n RTC = %s\r\n", l2s2(CPU_Timer_CurrentTicks(RTC_32BIT),0));
	hal_printf("\r\n HT = %s\r\n", l2s2(HAL_Time_CurrentTicks(),0));
	//	CPU_GPIO_SetPinState(GPIO_0, TRUE);
	//CPU_GPIO_SetPinState(GPIO_0, FALSE);
	//CPU_Timer_CurrentTicks(RTC_32BIT);
	//CPU_GPIO_SetPinState(GPIO_0, TRUE);
	//CPU_GPIO_SetPinState(GPIO_0, FALSE);
	//hal_printf("%llu\r\n", CPU_Timer_CurrentTicks(RTC_32BIT));
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

	CPU_GPIO_EnableOutputPin(LED1, TRUE);
    CPU_GPIO_EnableOutputPin(LED3, TRUE);

	VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 1000000, FALSE, FALSE, Timer_1_Handler);
	VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	VirtTimer_SetTimer(VIRT_TIMER_LED_RED, 0, 1000000, FALSE, FALSE, Timer_Red_Handler);
	VirtTimer_Start(VIRT_TIMER_LED_RED);

	CPU_GPIO_EnableOutputPin(GPIO_0, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_1, FALSE);
/*	CPU_GPIO_EnableOutputPin(GPIO_2, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_3, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_4, FALSE);
	CPU_GPIO_EnableOutputPin(GPIO_5, FALSE);
*/
/*	for (int j=0; j<5; j++){
		CPU_GPIO_SetPinState(GPIO_0, TRUE);
		CPU_GPIO_SetPinState(GPIO_0, FALSE);
		CPU_GPIO_SetPinState(GPIO_1, TRUE);
		CPU_GPIO_SetPinState(GPIO_1, FALSE);
		CPU_GPIO_SetPinState(GPIO_2, TRUE);
		CPU_GPIO_SetPinState(GPIO_2, FALSE);
	}*/
	/*	CPU_GPIO_SetPinState(GPIO_3, TRUE);
		CPU_GPIO_SetPinState(GPIO_3, FALSE);
		CPU_GPIO_SetPinState(GPIO_4, TRUE);
		CPU_GPIO_SetPinState(GPIO_4, FALSE);
		CPU_GPIO_SetPinState(GPIO_5, TRUE);
		CPU_GPIO_SetPinState(GPIO_5, FALSE);
	}
*/
	//VirtTimer_SetTimer(VIRT_TIMER_TIME_TEST, 0, 800000, FALSE, FALSE, Timer_1_Handler, RTC_32BIT); 
	//VirtTimer_Start(VIRT_TIMER_TIME_TEST);
	VirtTimer_SetTimer(VIRT_TIMER_RTC_TEST, 0, 2000000, FALSE, FALSE, Timer_RTC_Handler, RTC_32BIT);
	VirtTimer_Start(VIRT_TIMER_RTC_TEST);
	//VirtTimer_SetTimer(VIRT_TIMER_LED_GREEN, 0, 800000, FALSE, FALSE, Timer_Green_Handler, RTC_32BIT);
	//VirtTimer_Start(VIRT_TIMER_LED_GREEN);
	//I2S_Internal_Initialize();
	//I2S_Test();
    hal_printf(" CLR 30 ");
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
