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


#ifndef _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_ALGORITHM__DECISIONFUNCTION_H_
#define _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_ALGORITHM__DECISIONFUNCTION_H_

namespace Samraksh
{
    namespace eMote
    {
        struct Algorithm_DecisionFunction
        {
            // Helper Functions to access fields of managed object
            // Declaration of stubs. These functions are implemented by Interop code developers
            static void Initialize( CLR_RT_HeapBlock* pMngObj, UINT16 param0, UINT16 param1, float param2, float param3, CLR_RT_TypedArray_float param4, CLR_RT_TypedArray_float param5, CLR_RT_TypedArray_float param6, HRESULT &hr );
            static void NormalizeFeatureVector( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_float param0, CLR_RT_TypedArray_INT32 param1, HRESULT &hr );
            static float Decide( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_INT32 param0, HRESULT &hr );
        };
    }
}
#endif  //_SAMRAKSHEMOTE_SAMRAKSH_EMOTE_ALGORITHM__DECISIONFUNCTION_H_
