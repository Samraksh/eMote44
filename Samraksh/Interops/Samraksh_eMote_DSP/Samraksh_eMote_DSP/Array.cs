using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;

namespace Samraksh.SPOT.DSP
{
    public class Array
    {


        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool DataEqualance(byte[] array1, byte[] array2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool DataEqualance(UInt16[] array1, UInt16[] array2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool DataEqualance(UInt32[] array1, UInt32[] array2);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool DataEqualance(UInt64[] array1, UInt64[] array2);
    }
}
