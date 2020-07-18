using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;

using System.Text;

namespace Samraksh_Mel
{
    /// <summary>
    /// USB Serial Interface 
    /// </summary>
    public class UsbSerialInterface
    {
        /// <summary>
        /// USB Serial Interface constructor
        /// </summary>
        public UsbSerialInterface()
        {
            //OnInterrupt += UsbSerialCallback;
        }

        private readonly char[] _oneCharArray = new char[1];
        /// <summary>
        /// Write a single char
        /// </summary>
        /// <param name="theChar"></param>
        /// <returns></returns>
        public bool Write(char theChar)
        {
            _oneCharArray[0] = theChar;
            return Write(_oneCharArray);
        }

        /// <summary>
        /// Write a char array to the port
        /// </summary>
        /// <param name="theChars"></param>
        /// <returns></returns>
        public bool Write(char[] theChars)
        {
            uint chan = 0;
            int ret;
            var bytes = new byte[theChars.Length];
            for (var i = 0; i < theChars.Length; i++)
            {
                bytes[i] = (byte)theChars[i];
            }
            ret = mel_serial_tx(bytes, chan, bytes.Length);
            if (ret != 0) return false;
            else return true;
        }

        /// <summary>
        /// Write a string to the port
        /// </summary>
        /// <param name="str">The string to write</param>
        /// <remarks>Flushes the port after writing the bytes to ensure it all gets sent.</remarks>
        /// <returns></returns>
        public bool Write(string str)
        {
            uint chan = 0;
            int ret;
            var bytes = Encoding.UTF8.GetBytes(str);
            ret = mel_serial_tx(bytes, chan, bytes.Length);
            if (ret != 0) return false;
            else return true;
        }

        /// <summary>
        /// Sends a block of data out the USB serial port
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int mel_serial_tx(byte[] data, uint chan, int len);

        /// <summary>
        /// Read a block of data from the buffer
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int mel_serial_rx(byte[] data, uint max);
    }
    /// <summary>
    /// Audio Interface 
    /// </summary>
    public class AudioInterface : NativeEventDispatcher
    {
        // Constants
        private static readonly float[] EMPTY_FLOAT = new float[0];
        private const int DOWNSTREAM_LEN = 8; // floats
        private const int UPSTREAM_LEN = 256; // floats

        // State and defaults
        private bool collectUpStream;
        private const bool collectUpStream_reset = false;

        private bool collectDownStream;
        private const bool collectDownStream_reset = true;

        /// <summary>
        /// Audio Interface constructor
        /// </summary>
        public AudioInterface() : base("AICallback", 0)
        {
            Initialize();
            set_model_recording(collectUpStream_reset, collectDownStream_reset);
            OnInterrupt += aiCallbackFunction;
        }

        /// <summary>
        /// Audio Interface destructor
        /// </summary>
        ~AudioInterface()
        {
            Uninitialize();
        }

        /// <summary>
        /// Set Model data to collect.
        /// Setting both to false will disable the model and return only SPL data.
        /// </summary>
        /// <param name="upstream">Enables or Disables upstream data collection</param>
        /// <param name="downstream">Enables or Disables downstream data collection</param>
        public void set_model_recording(bool upstream, bool downstream)
        {
            collectUpStream = upstream;
            collectDownStream = downstream;
            set_model_recording_internal(upstream, downstream);
        }

        /// <summary>
        /// Change the FIR filter. Does not persist across reboots.
        /// Valid parameters TBD.
        /// </summary>
        /// <param name="num_taps">Number of taps. 0 to disable.</param>
        /// <param name="taps"></param>
        /// <returns>True on success. False on invalid parameters or not supported.</returns>
        public bool set_fir_taps(uint num_taps, float[] taps)
        {
            float[] my_taps;

            if (taps == null) my_taps = EMPTY_FLOAT;
            else my_taps = taps;

            if (num_taps > 0 && num_taps > my_taps.Length) return false;

            return set_fir_taps_internal(num_taps, my_taps);
        }

        private void aiCallbackFunction(uint data1, uint data2, DateTime time)
        {
            float dbSPL = -1;
            float[] downstream;
            float[] upstream;

            // TinyCLR won't allow null pointer parameters hence new T[0]

            if (collectDownStream)
                downstream = new float[DOWNSTREAM_LEN];
            else
                downstream = EMPTY_FLOAT;

            if (collectUpStream)
                upstream = new float[UPSTREAM_LEN];
            else
                upstream = EMPTY_FLOAT;

            GetResultData(ref dbSPL, upstream, downstream);
            audio_inference_callback(dbSPL, upstream, downstream);
        }

        /// <summary>
        /// Called after data collection event, typically 1-second intervals.
        /// Must null check both arrays.
        /// </summary>
        /// <param name="dbSPL"></param>
        /// <param name="upstream">256-float array. Size 0 if collection disabled.</param>
        /// <param name="downstream">8-float array. Size 0 if collection disabled.</param>
        public delegate void AudioInterfaceCallback(float dbSPL, float[] upstream, float[] downstream);

        /// <summary>
        /// ML Data Handler
        /// </summary>
        public event AudioInterfaceCallback audio_inference_callback;

        ///////////////////////////////////Internal methods/////////////////////////

        /// <summary>
        /// Change the FIR filter. Does not persist across reboots.
        /// Valid parameters TBD.
        /// </summary>
        /// <param name="num_taps">Number of taps. 0 to disable.</param>
        /// <param name="taps"></param>
        /// <returns>True on success. False on invalid parameters.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool set_fir_taps_internal(uint num_taps, float[] taps);

        /// <summary>
        /// Set Model data to collect.
        /// </summary>
        /// <param name="upstream">Enables or Disables upstream data collection</param>
        /// <param name="downstream">Enables or Disables downstream data collection</param>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void set_model_recording_internal(bool upstream, bool downstream);

        /// <summary>
        /// Initializes AudioInterface
        /// </summary>
        /// <returns>The result of AudioInterface initialization: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool Initialize();

        /// <summary>
        /// Uninitializes AudioInterface
        /// </summary>
        /// <returns>The result of AudioInterface uninitialization: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool Uninitialize();

        /// <summary>
        /// Called to return most recent audio results
        /// </summary>
        /// <param name="dbSPL"></param>
        /// <param name="upstream"></param>
        /// <param name="downstream"></param>
        /// <returns>False on data collection error.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern bool GetResultData(ref float dbSPL, float[] upstream, float[] downstream);

        /// <summary>
        /// Starts Audio sampling and (if enabled) the inference engine
        /// </summary>
        /// <returns>True on success</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool start_audio_inference();

        /// <summary>
        /// Stops audio data collection and associated processes
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern void stop_audio_inference();

        /// <summary>
        /// Process only M out of N 1-second clips
        /// Example: M=1 N=3 will run the model for 1 second every 3 seconds and produce 1 callback
        /// Call will be ignored (return False) if any parameter is 0 or >= 3600
        /// Default M=1 N=1 (100% Duty Cycle)
        /// </summary>
        /// <param name="M"></param>
        /// <param name="N"></param>
        /// <returns>True on success. False if operation not supported.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool set_ml_duty_cycle(uint M, uint N);

        /// <summary>
        /// Turns on or off output of raw data stream over serial
        /// Does not propagate to C# program level
        /// </summary>
        /// <param name="on">Enables or Disables raw data output</param>
        /// <returns>True on success. False if operation not supported.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool set_raw_data_output(bool on);

        /// <summary>
        /// Set a threshold measurement required to run the model
        /// </summary>
        /// <param name="thresh">dB SPL threshold. Negative to disable.</param>
        /// <returns>True on Success. False if not supported.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool set_dB_thresh(float thresh);

        /// <summary>
        /// Sets time period window for dB SPL measurment.
        /// </summary>
        /// <param name="time_ms">Window size in milliseconds. Default 1000ms.</param>
        /// <returns>True on success. False if not supported.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool set_time_interval(uint time_ms);
    }
}
