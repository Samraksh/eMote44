//-----------------------------------------------------------------------------
//
//    ** DO NOT EDIT THIS FILE! **
//    This file was generated by a tool
//    re-running the tool will overwrite this file.
//
//-----------------------------------------------------------------------------


#ifndef _SAMRAKSH_EMOTE_PROGRAMMANAGER_H_
#define _SAMRAKSH_EMOTE_PROGRAMMANAGER_H_

#include <TinyCLR_Interop.h>
struct Library_Samraksh_eMote_ProgramManager_Samraksh_eMote_ProgramManager_ProgramManager
{
    static const int FIELD_STATIC__instance = 0;
    static const int FIELD_STATIC__syncRoot = 1;
    static const int FIELD_STATIC___updaterInternal = 2;
    static const int FIELD_STATIC___updaterProgressCallback = 3;
    static const int FIELD_STATIC___updaterProgressHandler = 4;

    TINYCLR_NATIVE_DECLARE(UpdaterStart___BOOLEAN);
    TINYCLR_NATIVE_DECLARE(UpdaterStart___BOOLEAN__U4);
    TINYCLR_NATIVE_DECLARE(UpdaterSetDestination___VOID__U2);

    //--//

};

struct Library_Samraksh_eMote_ProgramManager_Samraksh_eMote_ProgramManager_UpdateState
{
    static const int FIELD__updateID = 1;
    static const int FIELD__destAddr = 2;
    static const int FIELD__lastSentMsg = 3;
    static const int FIELD__lastReceivedMsg = 4;
    static const int FIELD__packetDataSize = 5;
    static const int FIELD__missingPacketCount = 6;
    static const int FIELD__latestActionDate = 7;


    //--//

};

struct Library_Samraksh_eMote_ProgramManager_Samraksh_eMote_ProgramManager_UpdaterInternal
{
    static const int FIELD_STATIC__instance = 5;
    static const int FIELD_STATIC__syncRoot = 6;
    static const int FIELD_STATIC__strDrvName = 7;
    static const int FIELD_STATIC__drvData = 8;


    //--//

};



extern const CLR_RT_NativeAssemblyData g_CLR_AssemblyNative_Samraksh_eMote_ProgramManager;

#endif  //_SAMRAKSH_EMOTE_PROGRAMMANAGER_H_
