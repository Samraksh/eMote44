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


#ifndef _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_ALGORITHM__RADARDETECTION_H_
#define _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_ALGORITHM__RADARDETECTION_H_

namespace Samraksh
{
    namespace eMote
    {
        struct Algorithm_RadarDetection
        {
            // Helper Functions to access fields of managed object
            // Declaration of stubs. These functions are implemented by Interop code developers
            static INT8 Initialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT8 Uninitialize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT8 DetectionCalculation( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, CLR_RT_TypedArray_UINT16 param1, CLR_RT_TypedArray_UINT16 param2, INT32 param3, HRESULT &hr );
            static INT8 DetectionCalculation( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, CLR_RT_TypedArray_UINT16 param1, INT32 param2, HRESULT &hr );
            static INT8 SetDetectionParameters( CLR_RT_HeapBlock* pMngObj, INT32 param0, double param1, double param2, UINT16 param3, UINT16 param4, UINT16 param5, UINT16 param6, UINT16 param7, HRESULT &hr );
            static INT32 GetBackgroundNoiseLevel( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
            static INT8 ResetBackgroundNoiseTracking( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT32 GetIQRejectionLevel( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT32 GetLastUnwrap( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
            static INT8 GetWindowOverThreshold( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT8 CurrentDetectionFinished( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT32 GetNetDisplacement( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
            static INT32 GetAbsoluteDisplacement( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
            static INT32 GetDisplacementRange( CLR_RT_HeapBlock* pMngObj, INT32 param0, HRESULT &hr );
            static INT32 GetCountOverTarget( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        };
    }
}
#endif  //_SAMRAKSHEMOTE_SAMRAKSH_EMOTE_ALGORITHM__RADARDETECTION_H_
