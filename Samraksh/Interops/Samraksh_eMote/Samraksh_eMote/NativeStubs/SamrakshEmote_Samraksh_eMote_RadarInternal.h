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


#ifndef _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_RADARINTERNAL_H_
#define _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_RADARINTERNAL_H_

namespace Samraksh
{
    namespace eMote
    {
        struct RadarInternal
        {
            // Helper Functions to access fields of managed object
            // Declaration of stubs. These functions are implemented by Interop code developers
            static INT8 ConfigureFPGADetectionPrivate( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, CLR_RT_TypedArray_UINT16 param1, CLR_RT_TypedArray_INT16 param2, CLR_RT_TypedArray_INT16 param3, CLR_RT_TypedArray_INT16 param4, CLR_RT_TypedArray_INT8 param5, UINT32 param6, HRESULT &hr );
            static INT8 CurrentDetectionFinished( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static void setContinueToSendCount( CLR_RT_HeapBlock* pMngObj, UINT16 param0, HRESULT &hr );
            static UINT16 getContinueToSendCount( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static void setNumLookAheadWindows( CLR_RT_HeapBlock* pMngObj, UINT16 param0, HRESULT &hr );
            static UINT16 getNumLookAheadWindows( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static UINT16 getNumDetectionsInWindow( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT16 getTotalRotationsofWindow( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static void SetProcessingInProgress( CLR_RT_HeapBlock* pMngObj, INT8 param0, HRESULT &hr );
            static INT32 Init( INT32 param0, HRESULT &hr );
            static INT8 Uninit( HRESULT &hr );
        };
    }
}
#endif  //_SAMRAKSHEMOTE_SAMRAKSH_EMOTE_RADARINTERNAL_H_
