/*
 * Samraksh virtual timer driver
 * Initial Create - Mukundan Sridharan; Ananth Muralidharan
 * 07/24/2014
 *
 */

#include <Samraksh/VirtualTimer.h>

#if defined(DEBUG_VT)
#define DEBUG_VT_ASSERT_ANAL(x) ASSERT(x)
#else
#define DEBUG_VT_ASSERT_ANAL(x)
#endif

extern const UINT8 g_CountOfHardwareTimers;

VirtualTimer gVirtualTimerObject;
BOOL VirtualTimer::isInitialized = FALSE;

namespace VirtTimerHelperFunctions
{
	// used to take the timer (RTC_32BIT, ADVTIMER_32BIT) and get the correct VT mapper
	void AssignMapper(UINT32 hardwareTimerId, UINT8 &currentVTMapper)
	{
		currentVTMapper = -1;
		for(UINT16 i = 0; i < g_CountOfHardwareTimers; i++)
		{
			if(hardwareTimerId == g_HardwareTimerIDs[i])
			{
				currentVTMapper = i;
				return;
			}
		}
	}

	void HardwareVirtTimerMapper(UINT8 timer_id, UINT8 &mapperId)
	{
		mapperId = 0;

		for(UINT16 i = 0; i < g_CountOfHardwareTimers; i++)
		{
			for (UINT16 j = 0; j < g_VirtualTimerPerHardwareTimer; j++)
			{
				if(gVirtualTimerObject.virtualTimerMapper[i].g_VirtualTimerInfo[j].get_m_timer_id() == timer_id)
				{
					mapperId = i;
					return;
				}
			}
		}
	}
}

//BOOL VirtTimer_Initialize(UINT16 Timer, BOOL FreeRunning, UINT32 ClkSource, UINT32 Prescaler, HAL_CALLBACK_FPN ISR, void* ISR_PARAM)
BOOL VirtTimer_Initialize()
{
	if (gVirtualTimerObject.isInitialized == TRUE) {
		DEBUG_VT_ASSERT_ANAL(0);
		return TRUE;
	}

	for(UINT16 i = 0; i < g_CountOfHardwareTimers; i++)
	{
		if(!gVirtualTimerObject.virtualTimerMapper[i].Initialize(g_HardwareTimerIDs[i]))
		{
			ASSERT(0);
			return FALSE;
		}
	}

	gVirtualTimerObject.isInitialized = TRUE;
	return TRUE;
}

VirtualTimerReturnMessage VirtTimer_SetOrChangeTimer(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, BOOL _isreserved, TIMER_CALLBACK_FPN callback, UINT8 hardwareTimerId)
{
	VirtualTimerReturnMessage retVal = TimerSupported;;

	// try
	retVal = VirtTimer_SetTimer(timer_id, start_delay, period, is_one_shot, _isreserved, callback, hardwareTimerId);

	// catch
	if(retVal != TimerSupported) {
		retVal = VirtTimer_Change(timer_id, start_delay, period, is_one_shot, hardwareTimerId);
	}

	ASSERT(retVal == TimerSupported);

	return retVal;
}


VirtualTimerReturnMessage VirtTimer_SetTimer(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, BOOL _isreserved, TIMER_CALLBACK_FPN callback, UINT8 hardwareTimerId)
{
	VirtualTimerReturnMessage retVal = TimerSupported;
	UINT8 VTMapper = 0;
	VirtTimerHelperFunctions::AssignMapper(hardwareTimerId, VTMapper);

	if(!gVirtualTimerObject.virtualTimerMapper[VTMapper].SetTimer(timer_id, start_delay, period, is_one_shot, _isreserved, callback)) {
		retVal = TimerReserved;
	}
	else {
		retVal = TimerSupported;
	}
	return retVal;
}


VirtualTimerReturnMessage VirtTimer_Start(UINT8 timer_id)
{
	VirtualTimerReturnMessage retVal = TimerSupported;
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	if(!gVirtualTimerObject.virtualTimerMapper[mapperId].StartTimer(timer_id)) {
		retVal = TimerNotSupported;
	}
	else {
		retVal = TimerSupported;
	}
	return retVal;
}


//AnanthAtSamraksh - when the virtual timer is stopped, it is not released by default. User has to explicitly release it by setting _isreserved to FALSE
VirtualTimerReturnMessage VirtTimer_Stop(UINT8 timer_id)
{
	VirtualTimerReturnMessage retVal = TimerSupported;
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	if(gVirtualTimerObject.virtualTimerMapper[mapperId].StopTimer(timer_id) == TimerNotSupported) {
		retVal = TimerNotSupported;
	}
	else {
		retVal = TimerSupported;
	}
	return retVal;
}

BOOL VirtTimer_IsRunning(UINT8 timer_id)
{
	VirtualTimerReturnMessage retVal = TimerSupported;
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return gVirtualTimerObject.virtualTimerMapper[mapperId].IsRunning(timer_id);
}


VirtualTimerReturnMessage VirtTimer_Change(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, UINT8 hardwareTimerId)
{
	VirtualTimerReturnMessage retVal = TimerSupported;
	UINT8 VTMapper = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, VTMapper);

	if(!gVirtualTimerObject.virtualTimerMapper[VTMapper].ChangeTimer(timer_id, start_delay, period, is_one_shot))
		retVal = TimerNotSupported;
	else
		retVal = TimerSupported;
	return retVal;
}


UINT64 VirtTimer_TicksToTime(UINT8 timer_id, UINT64 Ticks)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_TicksToTime(Ticks, (UINT16)g_HardwareTimerIDs[mapperId]);
}

UINT64 VirtTimer_TicksToMicroseconds(UINT8 timer_id, UINT64 Ticks)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_TicksToMicroseconds(Ticks, (UINT16)g_HardwareTimerIDs[mapperId]);
}


UINT64 VirtTimer_GetTicks(UINT8 timer_id)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_Timer_CurrentTicks((UINT16)g_HardwareTimerIDs[mapperId]);
}

//TODO: not used?
UINT32 VirtTimer_GetCounter(UINT8 timer_id)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_Timer_GetCounter((UINT16)g_HardwareTimerIDs[mapperId]);
}

UINT32 VirtTimer_SetCounter(UINT8 timer_id, UINT32 Count)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_Timer_SetCounter((UINT16)g_HardwareTimerIDs[mapperId], Count);
}

BOOL VirtTimer_SetCompare(UINT8 timer_id, UINT64 CompareValue)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_Timer_SetCompare((UINT16)g_HardwareTimerIDs[mapperId], CompareValue );
}


void VirtTimer_SleepMicroseconds(UINT8 timer_id, UINT32 uSec)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	CPU_Timer_Sleep_MicroSeconds(uSec, (UINT16)g_HardwareTimerIDs[mapperId]);
}

UINT32 VirtTimer_GetMaxTicks(UINT8 timer_id)
{
	UINT8 mapperId = 0;
	VirtTimerHelperFunctions::HardwareVirtTimerMapper(timer_id, mapperId);

	return CPU_Timer_GetMaxTicks(g_HardwareTimerIDs[mapperId]);
}

// Debug helper function
static void print_all_vts() __attribute__ ((unused));
static void print_all_vts() {
	for (int i=0; i<g_CountOfHardwareTimers; i++) {
		hal_printf("%d NOW: %llu\r\n", i, CPU_Timer_CurrentTicks(gVirtualTimerObject.virtualTimerMapper[i].VTM_hardwareTimerId));
		for (int j=0; j<gVirtualTimerObject.virtualTimerMapper[i].m_current_timer_cnt_; j++) {
			VirtualTimerInfo *tim = &gVirtualTimerObject.virtualTimerMapper[i].g_VirtualTimerInfo[j];
			hal_printf("%d:%d\tRUN?:%s MTH:%llu RES?:%s\r\n", i, tim->get_m_timer_id(), tim->get_m_is_running() ? "yes" : "no", tim->get_m_ticks_when_match_(),
				tim->get_m_reserved() ? "yes" : "no");
		}
	}
}

// Input is system timer ticks (e.g., 8 MHz)
// Output is system timer ticks
UINT64 VirtTimer_GetNextAlarm(UINT64 now) {

	UINT64 extra=0xFFFFFFFFFFFFFFFF-now; // system timer ticks
	UINT16 system_hw_id = gVirtualTimerObject.virtualTimerMapper[0].VTM_hardwareTimerId;
	// The VT can schedule a callback but not ensure it gets executed before sleep
	// Manually check for this here.
	if ( VTCallbackQueueHasItem() )
		return 0;

	{ // System timer is first and special case
		UINT64 alr_next = gVirtualTimerObject.virtualTimerMapper[0].GetNextAlarm();
		UINT64 alr_now  = CPU_Timer_CurrentTicks(system_hw_id);
		if (alr_now >= alr_next) {
			return 0; // A timer is behind?
		}
		// Already in system timer ticks, no conversion
		UINT64 ticks = alr_next - alr_now;
		if (ticks < extra)
			extra = ticks;
	}

	for (int i=1; i<g_CountOfHardwareTimers; i++) {
		UINT16 hw_id = gVirtualTimerObject.virtualTimerMapper[i].VTM_hardwareTimerId;
		// Below two ops are in native ticks (unique to timer)
		UINT64 alr_next = gVirtualTimerObject.virtualTimerMapper[i].GetNextAlarm();
		UINT64 alr_now  = CPU_Timer_CurrentTicks(hw_id);

		if (alr_now >= alr_next) {
			return 0; // A timer is behind?
		}

		// Convert to microseconds
		UINT64 next_us = CPU_TicksToMicroseconds(alr_next - alr_now , hw_id);

		// Convert back to system timer ticks
		UINT64 ticks = CPU_MicrosecondsToTicks(next_us, system_hw_id);
		if (ticks < extra)
			extra = ticks;
	}
	return now+extra;
}

void VirtTimer_UpdateAlarms()
{
	// After waking up from sleep the alarm needs to be set again
	UINT16 i = 0;
	//for(UINT16 i = 0; i < g_CountOfHardwareTimers; i++)
	{
		gVirtualTimerObject.virtualTimerMapper[i].SetAlarmForTheNextTimer();
	}
}

BOOL VirtTimer_UnInitialize()
{
	BOOL ret = TRUE;
	for(UINT16 i = 0; i < g_CountOfHardwareTimers; i++)
	{
			ret &= gVirtualTimerObject.virtualTimerMapper[i].UnInitialize(g_HardwareTimerIDs[i]);
			gVirtualTimerObject.virtualTimerMapper[i].m_current_timer_cnt_ = 0;
	}
	gVirtualTimerObject.isInitialized = FALSE;
	return ret;
}

