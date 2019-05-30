#include <Samraksh/VirtualTimer.h>
#include <Samraksh/Hal_util.h>

VirtualTimerMapper gVirtualTimerMapperObject;

BOOL VirtualTimerMapper::Initialize(UINT16, UINT16)
{
	return TRUE;
}

BOOL VirtualTimerMapper::StopTimer(UINT8 timer_id)
{
	return TRUE;
}


BOOL VirtualTimerMapper::ChangeTimer(UINT8 timer_id, UINT32 dtime)
{
	return TRUE;
}

BOOL VirtualTimerMapper::StartTimer(UINT8 timer_id)
{
	return TRUE;
}

BOOL VirtualTimerMapper::SetTimer(UINT8 timer_id, UINT32 start_delay, UINT32 period, BOOL is_one_shot, BOOL _isreserved, TIMER_CALLBACK_FPN callback)
{
	return TRUE;
}
