using System;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;

namespace Samraksh.eMote.RealTime
{
    /// <summary>
    /// Realtime Timer
    /// </summary>
    public class Timer : NativeEventDispatcher
    {
        /// <summary>Realtime Timer</summary>
        /// <param name="strDrvName">Driver name; must be "RealTimeInteropTimer"</param>
        /// <param name="period">Interval between callbacks (microseconds)</param>
        /// <param name="callbackCount">How many callbacks to make (0 = infinite)</param>
        [Obsolete("Deprecated. Use other constructor.")]
        public Timer(string strDrvName, ulong period, int callbackCount) 
            : base(strDrvName, period) 
        {

        }

        /// <summary>Realtime Timer</summary>
        /// <param name="period">Interval between callbacks (microseconds)</param>
        /// <param name="callbackCount">How many callbacks to make (0 = infinite)</param>
        [Obsolete("Deprecated. callbackCount no longer used.")]
        public Timer(ulong period, int callbackCount)
            : base("RealTimeInteropTimer", period)
        {

        }

        /// <summary>Realtime Timer</summary>
        /// <param name="period">Interval between callbacks (microseconds)</param>
        public Timer(ulong period)
            : base("RealTimeInteropTimer", period) {

        }

        /// <summary>Dispose realtime timer</summary>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        new extern public void Dispose();

        /// <summary>Change the timer due time and period</summary>
        /// <param name="dueTime">Delay before first callback (microseconds)</param>
        /// <param name="period">Interval between callbacks (microseconds)</param>
        /// <returns>Success / failure</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public bool Change(uint dueTime, uint period);

        /// <summary>Generate an interrupt</summary>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern public void GenerateInterrupt(); 
    }
}
