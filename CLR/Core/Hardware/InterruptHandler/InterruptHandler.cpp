////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) Microsoft Corporation.  All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "..\..\core.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

 //this method is called by Hardware_Initialize
 //If you want to initialize your own interrupt handling related objects, put them here
 
HRESULT CLR_HW_Hardware::ManagedHardware_Initialize()
{
    NATIVE_PROFILE_CLR_HARDWARE();
    TINYCLR_HEADER();
    
    m_interruptData.m_HalQueue.Initialize( (CLR_HW_Hardware::HalInterruptRecord*)&g_scratchInterruptDispatchingStorage, InterruptRecords() );

    m_interruptData.m_applicationQueue.DblLinkedList_Initialize ();

    m_interruptData.m_queuedInterrupts = 0;

    TINYCLR_NOCLEANUP_NOLABEL();

}

HRESULT CLR_HW_Hardware::SpawnDispatcher()
{
    NATIVE_PROFILE_CLR_HARDWARE();
    TINYCLR_HEADER();

    CLR_RT_ApplicationInterrupt* interrupt;
    CLR_RT_HeapBlock_NativeEventDispatcher* ioPort;
    CLR_RT_HeapBlock_NativeEventDispatcher ::InterruptPortInterrupt *interruptData;

    // if reboot is in progress, just bail out
    if(CLR_EE_DBG_IS( RebootPending )) 
    {
        return S_OK;
    }

    interrupt = (CLR_RT_ApplicationInterrupt*)m_interruptData.m_applicationQueue.FirstValidNode();

    if((interrupt == NULL) || !g_CLR_RT_ExecutionEngine.EnsureSystemThread( g_CLR_RT_ExecutionEngine.m_interruptThread, ThreadPriority::System_Highest ))
    {
        return S_OK;
    }

    interrupt->Unlink();

    interruptData = &interrupt->m_interruptPortInterrupt;
    ioPort = interruptData->m_context;

    CLR_RT_ProtectFromGC gc1 ( *ioPort );
                    
    TINYCLR_SET_AND_LEAVE(ioPort->StartDispatch( interrupt, g_CLR_RT_ExecutionEngine.m_interruptThread ));
            
    TINYCLR_CLEANUP();

    if(FAILED(hr))
    {
        ioPort->ThreadTerminationCallback( interrupt );
    }

    --m_interruptData.m_queuedInterrupts;

    TINYCLR_CLEANUP_END();    
}

HRESULT CLR_HW_Hardware::TransferAllInterruptsToApplicationQueue()
{
    NATIVE_PROFILE_CLR_HARDWARE();
    TINYCLR_HEADER();

    while(true)
    {
        HalInterruptRecord* rec;

        {
            GLOBAL_LOCK(irq1);

            rec = m_interruptData.m_HalQueue.Peek();
        }

        if(rec == NULL) break;

        CLR_RT_ApplicationInterrupt* queueRec = (CLR_RT_ApplicationInterrupt*)CLR_RT_Memory::Allocate_And_Erase( sizeof(CLR_RT_ApplicationInterrupt), CLR_RT_HeapBlock::HB_CompactOnFailure );  CHECK_ALLOCATION(queueRec);

        queueRec->m_interruptPortInterrupt.m_data1   =                                          rec->m_data1;
        queueRec->m_interruptPortInterrupt.m_data2   =                                          rec->m_data2;
        queueRec->m_interruptPortInterrupt.m_data3   =                                          rec->m_data3;
        queueRec->m_interruptPortInterrupt.m_time    =                                          rec->m_time;
        queueRec->m_interruptPortInterrupt.m_context = (CLR_RT_HeapBlock_NativeEventDispatcher*)rec->m_context;

        m_interruptData.m_applicationQueue.LinkAtBack( queueRec ); ++m_interruptData.m_queuedInterrupts;

        {
            GLOBAL_LOCK(irq2);
            
            m_interruptData.m_HalQueue.Pop();
        }
    }

    if(m_interruptData.m_queuedInterrupts == 0)
    {
        TINYCLR_SET_AND_LEAVE(CLR_E_NO_INTERRUPT);
    }

    TINYCLR_CLEANUP();

    if(CLR_E_OUT_OF_MEMORY == hr)
    {
        // if there is no memory left discard all interrupts to avoid getting into a death spiral of OOM exceptions
        {
            GLOBAL_LOCK(irq3);

            while(!m_interruptData.m_HalQueue.IsEmpty())
            {
                m_interruptData.m_HalQueue.Pop();
            }
			//remove any queued interrupts for this port
    		TINYCLR_FOREACH_NODE(CLR_RT_ApplicationInterrupt,interrupt,g_CLR_HW_Hardware.m_interruptData.m_applicationQueue)
    		{
            		interrupt->Unlink();

            		--g_CLR_HW_Hardware.m_interruptData.m_queuedInterrupts;

					CLR_RT_ApplicationInterrupt* appInterrupt = interrupt;
   					CLR_RT_HeapBlock_NativeEventDispatcher::InterruptPortInterrupt& interrupt = appInterrupt->m_interruptPortInterrupt;

    				FreeManagedEvent((interrupt.m_data1 >>  8) & 0xff, //category
                     (interrupt.m_data1      ) & 0xff, //subCategory
                      interrupt.m_data1 >> 16        , //data1
                      interrupt.m_data2              );

    				interrupt.m_data1 = 0;
    				interrupt.m_data2 = 0;

   					CLR_RT_Memory::Release( appInterrupt );

    				g_CLR_HW_Hardware.SpawnDispatcher();
    		}
    		TINYCLR_FOREACH_NODE_END();
        }
		// Ideally we would generate an Out of Memory exception here because at this point we are no longer out of memory due to our above recovery and so we will not throw an exception later
		// TinyCLR_SET_AND_LEAVE does not work because it just sets hr to be CLR_E_OUT_OF_MEMORY (which it already is)
		// TINYCLR_SET_AND_LEAVE(CLR_E_OUT_OF_MEMORY);
		// The following seems to lock us up (maybe it keeps jumping us to TINYCLR_CLEANUP()?)
		//(void)Library_corlib_native_System_Exception::CreateInstance( g_CLR_RT_ExecutionEngine.m_currentThread->m_currentException, g_CLR_RT_WellKnownTypes.m_OutOfMemoryException, CLR_E_OUT_OF_MEMORY, g_CLR_RT_ExecutionEngine.m_currentThread->CurrentFrame() );
		// For now we will print a warning message. The fault recovery and warning message is better than before but an exception thrown is needed
		hal_printf("### Warning: System out of memory. Interrupt queue has been purged to recover memory.\r\n");
    }    
    
    TINYCLR_CLEANUP_END();
}

HRESULT CLR_HW_Hardware::ProcessInterrupts()
{
    NATIVE_PROFILE_CLR_HARDWARE();
    TINYCLR_HEADER();

    TINYCLR_CHECK_HRESULT(TransferAllInterruptsToApplicationQueue());

    SpawnDispatcher();

    TINYCLR_NOCLEANUP();
}
