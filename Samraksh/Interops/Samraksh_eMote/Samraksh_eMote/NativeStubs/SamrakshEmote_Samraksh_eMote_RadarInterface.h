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


#ifndef _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_RADARINTERFACE_H_
#define _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_RADARINTERFACE_H_

namespace Samraksh
{
    namespace eMote
    {
        struct RadarInterface
        {
            // Helper Functions to access fields of managed object
            // Declaration of stubs. These functions are implemented by Interop code developers
            static INT8 TurnOn( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT8 TurnOff( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
            static INT8 Blind( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
        };
    }
}
#endif  //_SAMRAKSHEMOTE_SAMRAKSH_EMOTE_RADARINTERFACE_H_
