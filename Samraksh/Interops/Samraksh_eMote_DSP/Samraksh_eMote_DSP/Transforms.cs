using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;

// ReSharper disable InconsistentNaming

namespace Samraksh.eMote.DSP
{
    /// <summary>
    /// Transforms on data
    /// </summary>
    public class Transforms
    {
        /**********************************************FFT******************************/
        /// <summary>
        /// Calculate the Fast Fourier Transform 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool FFT(byte[] input, byte[] output, UInt16 transformSize);


        /// <summary>
        /// Calculate the Fast Fourier Transform 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool FFT(UInt16[] input, UInt16[] output,  UInt16 transformSize);

        /// <summary>
        /// Calculate the Fast Fourier Transform 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool FFT(UInt32[] input, UInt32[] output, UInt16 transformSize);

        /// <summary>
        /// Calculate the Fast Fourier Transform 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool FFT(Int16[] input, Int16[] output, UInt16 transformSize);


        /**********************************************Inverse FFT******************************/
        /// <summary>
        /// Calculate the inverse Fast Fourier Transfrom 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool IFFT(byte[] input, byte[] output, UInt16 transformSize);


        /// <summary>
        /// Calculate the inverse Fast Fourier Transfrom 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool IFFT(UInt16[] input, UInt16[] output, UInt16 transformSize);

        /// <summary>
        /// Calculate the inverse Fast Fourier Transfrom 
        /// </summary>
        /// <param name="input">Input array</param>
        /// <param name="output">Output array</param>
        /// <param name="transformSize">Size of the transform</param>
        /// <returns>Transform array of size transformSize</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool IFFT(UInt32[] input, UInt32[] output, UInt16 transformSize);








    }
}
// ReSharper restore InconsistentNaming
