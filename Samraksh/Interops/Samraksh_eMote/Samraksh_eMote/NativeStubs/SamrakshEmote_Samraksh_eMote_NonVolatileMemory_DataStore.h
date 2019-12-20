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


#ifndef _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_NONVOLATILEMEMORY_DATASTORE_H_
#define _SAMRAKSHEMOTE_SAMRAKSH_EMOTE_NONVOLATILEMEMORY_DATASTORE_H_

namespace Samraksh
{
    namespace eMote
    {
        namespace NonVolatileMemory
        {
            struct DataStore
            {
                // Helper Functions to access fields of managed object
                static INT32& Get_storageType( CLR_RT_HeapBlock* pMngObj )    { return Interop_Marshal_GetField_INT32( pMngObj, Library_SamrakshEmote_Samraksh_eMote_NonVolatileMemory_DataStore::FIELD__storageType ); }

                // Declaration of stubs. These functions are implemented by Interop code developers
                static INT32 CreateDataStore( CLR_RT_HeapBlock* pMngObj, INT8 param0, HRESULT &hr );
                static INT32 GetMaxAllocationSize( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 GetTotalSpace( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 GetUsedBytes( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 GetFreeBytes( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 GetReadAllDataIds( CLR_RT_HeapBlock* pMngObj, CLR_RT_TypedArray_INT32 param0, INT32 param1, INT32 param2, HRESULT &hr );
                static INT32 GetCountOfDataIds( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 DeleteAll( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 EraseAllBlocks( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT8 DataStoreGC( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT8 GetReadWriteStatus( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 GetLastDatastoreStatus( CLR_RT_HeapBlock* pMngObj, HRESULT &hr );
                static INT32 WriteRaw( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_UINT8 param1, UINT32 param2, UINT32 param3, UINT8 param4, INT32 param5, HRESULT &hr );
                static INT32 WriteRaw( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_UINT8 param1, UINT32 param2, UINT32 param3, UINT16 param4, INT32 param5, HRESULT &hr );
                static INT32 WriteRaw( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_UINT8 param1, UINT32 param2, UINT32 param3, UINT32 param4, INT32 param5, HRESULT &hr );
                static INT32 ReadRaw( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_UINT8 param1, UINT32 param2, UINT32 param3, UINT8 param4, INT32 param5, HRESULT &hr );
                static INT32 ReadRaw( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_UINT8 param1, UINT32 param2, UINT32 param3, UINT16 param4, INT32 param5, HRESULT &hr );
                static INT32 ReadRaw( CLR_RT_HeapBlock* pMngObj, UINT32 param0, CLR_RT_TypedArray_UINT8 param1, UINT32 param2, UINT32 param3, UINT32 param4, INT32 param5, HRESULT &hr );
            };
        }
    }
}
#endif  //_SAMRAKSHEMOTE_SAMRAKSH_EMOTE_NONVOLATILEMEMORY_DATASTORE_H_
