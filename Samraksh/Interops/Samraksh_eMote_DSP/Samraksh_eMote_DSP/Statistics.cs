using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;

namespace Samraksh.eMote.DSP
{
    /// <summary>
    /// Simple statistics on data
    /// </summary>
    public class Statistics
    {
        /**********************************************SUM******************************/
        /// <summary>
        /// Sums an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Sum of array</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt64 Sum(byte[] array);

        /// <summary>
        /// Sums an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Sum of array</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt64 Sum(UInt16[] array);

        /// <summary>
        /// Sums an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Sum of array</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt64 Sum(UInt32[] array);

        /// <summary>
        /// Sums an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Sum of array</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt64 Sum(UInt64[] array);

        /**********************************************Median******************************/
        /// <summary>
        /// Calculate the median of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Median value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern byte Median(byte[] array);

        /// <summary>
        /// Calculate the median of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Median value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt16 Median(UInt16[] array);

        /// <summary>
        /// Calculate the median of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Median value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt32 Median(UInt32[] array);

        /// <summary>
        /// Calculate the median of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Median value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt64 Median(UInt64[] array);



        /**********************************************Mode******************************/
        /// <summary>
        /// Calculate the mode of an rray
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Mode value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern byte Mode(byte[] array);

        /// <summary>
        /// Calculate the mode of an rray
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Mode value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt16 Mode(UInt16[] array);

        /// <summary>
        /// Calculate the mode of an rray
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Mode value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt32 Mode(UInt32[] array);

        /// <summary>
        /// Calculate the mode of an rray
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Mode value</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern UInt64 Mode(UInt64[] array);

        /**********************************************Variance******************************/
        /// <summary>
        /// Calculate the variance of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Variance</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern double Variance(byte[] array);

        /// <summary>
        /// Calculate the variance of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Variance</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern double Variance(UInt16[] array);

        /// <summary>
        /// Calculate the variance of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Variance</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern double Variance(UInt32[] array);

        /// <summary>
        /// Calculate the variance of an array
        /// </summary>
        /// <param name="array">Input array</param>
        /// <returns>Variance</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern double Variance(UInt64[] array);


    }
}
