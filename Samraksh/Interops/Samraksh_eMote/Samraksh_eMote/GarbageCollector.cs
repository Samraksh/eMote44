/* Copyright: The Samaraksh Company (All rights reserved)
 * Author: Mukundan Sridharan
 * Date: Feb 2013
 * Function: Provides access to Samraksh Garbage Collector Extensions functions
 */
/*
using System;
using System.Runtime.CompilerServices;

namespace Samraksh.eMote
{
    /// <summary>Garbage Collector class.</summary>
    public class GC
    {
        /// <summary>Get amount of used heap space in kilo bytes.</summary>
        /// <returns>Amount of used heap in kilo bytes.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public float GetUsedKBytes();

        /// <summary>Get amount of free heap space in kilo bytes.</summary>
        /// <returns>Amount of free heap space in kilo bytes.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public float GetFreeKBytes();

        /// <summary>Check if Samraksh's Garbage Collection extensions are enabled or not.</summary>
        /// <returns>True iff Samraksh GC extensions enabled.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool IsGCSamExtensionsEnabled();
        
        /// <summary>Enable incremental Mark & Sweep heap cleaning in GC.</summary>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool EnableIncrementalCleaning();

        /// <summary>Enable incremental compaction of heap in GC.</summary>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool EnableIncrementalCompaction();

        /// <summary>Disable incremental Mark & Sweep heap cleaning in GC.</summary>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool DisableIncrementalCleaning();

        /// <summary>Disable incremental compaction of heap in GC.</summary>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool DisableIncrementalCompaction();
        
        /// <summary>Instruct GC to clean the heap and create the amount of space specified, then puts CPU to sleep.</summary>
        /// <param name="KBytesToFree">Amount of free space to be created.</param>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
// ReSharper disable once InconsistentNaming
        extern public bool CleanBeforeSleep(UInt32 KBytesToFree);

        /// <summary>Instruct GC to clean the heap now (suspending managed code execution) and create the amount of space specified.</summary>
        /// <param name="KBytesToFree">Amount of free space to be created.</param>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
// ReSharper disable once InconsistentNaming
        extern public bool CleanNow(UInt32 KBytesToFree);

        /// <summary>Instruct GC to clean the heap now and create as much space as possible, without compaction.</summary>
        /// <returns>True iff operation succeeds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool CleanNow();

        /// <summary>Average time for the Mark and Sweep operation. Implements weighted (0.5 weight) moving average.</summary>
        /// <returns>Average Mark and Sweep time in microseconds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public UInt32 AvgTimeForCleaning();

        /// <summary>Average time for the compaction operation. Implements weighted (0.5 weight) moving average.</summary>
        /// <returns>Average Mark and Sweep time as microseconds.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public UInt32 AvgTimeForCompaction();
    }
}
*/