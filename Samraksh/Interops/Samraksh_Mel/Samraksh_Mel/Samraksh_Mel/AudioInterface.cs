using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;

namespace Samraksh_Mel
{
    /// <summary>
    /// Audio Interface 
    /// </summary>
    public class AudioInterface : NativeEventDispatcher
    {
        private int callbackDataSize;
        /// <summary>
        /// Audio Interface constructor
        /// <param name="callbackDataSizeInit">Size of float array sent to callback function</param>
        /// </summary>
        public AudioInterface(int callbackDataSizeInit) : base("AICallback", 2345)
        {
            if ((callbackDataSize > 0) && (callbackDataSizeInit < 1024))
                callbackDataSize = callbackDataSizeInit;
            else
                callbackDataSize = 6;
            Initialize(callbackDataSize);
            OnInterrupt += aiCallbackFunction;
        }

        /// <summary>
        /// Audio Interface destructor
        /// </summary>
        ~AudioInterface()
        {
            Uninitialize();
        }

        private void aiCallbackFunction(uint data1, uint data2, DateTime time)
        {
            float[] callbackData = new float[callbackDataSize];
            GetResultData(callbackData);
            audio_inference_callback(callbackData);
        }

        // Changed first argument to IMAC instead of MACBase -- Bill
        /// <summary>Event handler for classes implementing IMAC's on receive event</summary>
        /// <param name="results"></param>
        public delegate void AudioInterfaceCallback(float[] results);

        /// <summary>Raised when a packet has been received</summary>
        public event AudioInterfaceCallback audio_inference_callback;

        ///////////////////////////////////Internal methods/////////////////////////

        /// <summary>
        /// Initializes AudioInterface
        /// </summary>
        /// <returns>The result of AudioInterface initialization: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool Initialize(int callbackDataSize);

        /// <summary>
        /// Uninitializes AudioInterface
        /// </summary>
        /// <returns>The result of AudioInterface uninitialization: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool Uninitialize();

        /// <summary>
        /// Uninitializes AudioInterface
        /// </summary>
        /// <returns>The result of AudioInterface uninitialization: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool GetResultData(float[] resultRead);

        /// <summary>
        /// Starts the audio interface at <param name="rate"></param>
        /// </summary>
        /// <returns>The result of AudioInterface starting: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool start_audio_inference(int rate);

        /// <summary>
        /// Stops the audio interface
        /// </summary>
        /// <returns>The result of AudioInterface stops: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool stop_audio_inference();
    }
}
