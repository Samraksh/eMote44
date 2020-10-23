//-----------------------------------------------------------------------------
//
//                   ** WARNING! ** 
//    This file was generated automatically by a tool.
//    Re-running the tool will overwrite this file.
//    You should copy this file to a custom location
//    before adding any customization in the copy to
//    prevent loss of your changes when the tool is
//    re-run.
//
//-----------------------------------------------------------------------------


#include "Samraksh_Mel.h"
#include "Samraksh_Mel_Samraksh_Mel_MelUtility.h"

#include "Samraksh/serial_frame_pal.h"

using namespace Samraksh_Mel;

static int32_t avg24(uint32_t *x) {
	int32_t ret=0;
	for(int i=0; i<24; i++) {
		ret += x[i];
	}
	return ret/24;
}

INT32 MelUtility::GetMelStatus( CLR_RT_TypedArray_INT32 param0, HRESULT &hr )
{
	bms_rx_v6_t *bms = get_bms_data_v6();
	uint32_t i = bms->hour_idx;
	param0[0] = bms->tot;
	param0[1] = bms->cells[0];
	param0[2] = bms->cells[1];
	param0[3] = bms->cells[2];
	param0[4] = bms->cells[3];
	param0[5] = (int32_t)bms->temperature_24[i];
	param0[6] = bms->power_in_24[i];		// power in last hour
	param0[7] = avg24(bms->power_in_24);	// power in last day
	param0[8] = bms->power_out_24[i];		// power out last hour
	param0[9] = avg24(bms->power_out_24);	// power out last day
	return 0;
}

