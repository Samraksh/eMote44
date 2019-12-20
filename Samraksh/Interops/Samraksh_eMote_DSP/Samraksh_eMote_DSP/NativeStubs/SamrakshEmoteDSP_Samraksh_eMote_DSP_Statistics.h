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


#ifndef _SAMRAKSHEMOTEDSP_SAMRAKSH_EMOTE_DSP_STATISTICS_H_
#define _SAMRAKSHEMOTEDSP_SAMRAKSH_EMOTE_DSP_STATISTICS_H_

namespace Samraksh
{
    namespace eMote
    {
        namespace DSP
        {
            struct Statistics
            {
                // Helper Functions to access fields of managed object
                // Declaration of stubs. These functions are implemented by Interop code developers
                static UINT64 Sum( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, HRESULT &hr );
                static UINT64 Sum( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, HRESULT &hr );
                static UINT64 Sum( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT32 param0, HRESULT &hr );
                static UINT64 Sum( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT64 param0, HRESULT &hr );
                static UINT8 Median( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, HRESULT &hr );
                static UINT16 Median( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, HRESULT &hr );
                static UINT32 Median( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT32 param0, HRESULT &hr );
                static UINT64 Median( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT64 param0, HRESULT &hr );
                static UINT8 Mode( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, HRESULT &hr );
                static UINT16 Mode( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, HRESULT &hr );
                static UINT32 Mode( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT32 param0, HRESULT &hr );
                static UINT64 Mode( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT64 param0, HRESULT &hr );
                static double Variance( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, HRESULT &hr );
                static double Variance( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, HRESULT &hr );
                static double Variance( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT32 param0, HRESULT &hr );
                static double Variance( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT64 param0, HRESULT &hr );
            };
        }
    }
}
#endif  //_SAMRAKSHEMOTEDSP_SAMRAKSH_EMOTE_DSP_STATISTICS_H_
