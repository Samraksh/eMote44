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


#ifndef _SAMRAKSHEMOTEDSP_SAMRAKSH_EMOTE_DSP_TRANSFORMS_H_
#define _SAMRAKSHEMOTEDSP_SAMRAKSH_EMOTE_DSP_TRANSFORMS_H_

namespace Samraksh
{
    namespace eMote
    {
        namespace DSP
        {
            struct Transforms
            {
                // Helper Functions to access fields of managed object
                // Declaration of stubs. These functions are implemented by Interop code developers
                static INT8 FFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, CLR_RT_TypedArray_UINT8 param1, UINT16 param2, HRESULT &hr );
                static INT8 FFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, CLR_RT_TypedArray_UINT16 param1, UINT16 param2, HRESULT &hr );
                static INT8 FFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT32 param0, CLR_RT_TypedArray_UINT32 param1, UINT16 param2, HRESULT &hr );
                static INT8 FFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_INT16 param0, CLR_RT_TypedArray_INT16 param1, UINT16 param2, HRESULT &hr );
                static INT8 IFFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT8 param0, CLR_RT_TypedArray_UINT8 param1, UINT16 param2, HRESULT &hr );
                static INT8 IFFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT16 param0, CLR_RT_TypedArray_UINT16 param1, UINT16 param2, HRESULT &hr );
                static INT8 IFFT( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_UINT32 param0, CLR_RT_TypedArray_UINT32 param1, UINT16 param2, HRESULT &hr );
            };
        }
    }
}
#endif  //_SAMRAKSHEMOTEDSP_SAMRAKSH_EMOTE_DSP_TRANSFORMS_H_
