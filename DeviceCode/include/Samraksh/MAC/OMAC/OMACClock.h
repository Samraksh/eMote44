#ifndef OMACCLOK_H_
#define OMACCLOK_H_

#include <Samraksh/Hal_util.h>
#include <Samraksh/VirtualTimer.h>

class OMACClock{
	//UINT64 m_inter_clock_offset;
	UINT64 m_first_clock_reading;
public:
	OMACClock();
	UINT64 GetCurrentTimeinTicks(); //This function gets the time ticks required for OMAC

	UINT64 ConvertTickstoMicroSecs(const UINT64& ticks); //This function gets the time ticks required for OMAC

	UINT64 ConvertMicroSecstoTicks(const UINT64& microsecs); //This function gets the time ticks required for OMAC

	UINT64 ConvertSlotstoTicks(const UINT64& slots); //This function gets the time ticks required for OMAC

	void CreateSyncPointBetweenClocks();

	UINT64 AddTicks(const UINT64& ticks1, const UINT64& ticks2);

	UINT64 SubstractTicks(const UINT64& ticks1, const UINT64& ticks2);

	UINT64 SubstractMicroSeconds(const UINT64& ticks1, const UINT64& ticks2);

	UINT64 AddMicroSeconds(const UINT64& ticks1, const UINT64& ticks2);
};

#endif /* OMACCLOK_H_ */