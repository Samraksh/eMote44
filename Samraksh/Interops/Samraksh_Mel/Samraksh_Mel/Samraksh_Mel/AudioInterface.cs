using System;
//using Microsoft.SPOT;
//using System.Collections;
//using System.Threading;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;

using System.Text;

#pragma warning disable 1591

namespace Samraksh_Mel
{
    public class MelUtility
    {
        public const int STATUS_SIZE = 10; // whole bank [0] + each cell [1-4] + temperature [5] + solar [6-7] + power [8-9]
        public const int BATTERY_SIZE = 5;

        // Returns battery status in mV
        public static int[] GetBatteryStatus()
        {
            int[] ret = new int[STATUS_SIZE];
            GetMelStatus(ret);
            int[] batt = new int[BATTERY_SIZE];

            Array.Copy(ret, batt, BATTERY_SIZE);

            return batt;
        }

        // Returns Solar status
        public static int[] GetSolarStatus()
        {
            int[] ret = new int[STATUS_SIZE];
            GetMelStatus(ret);
            int[] solar = new int[2];

            solar[0] = ret[6];
            solar[1] = ret[7];

            return solar;
        }

        public static int[] GetPowerStatus()
        {
            int[] ret = new int[STATUS_SIZE];
            GetMelStatus(ret);
            int[] power = new int[2];

            power[0] = ret[8];
            power[1] = ret[9];

            return power;
        }

        // Returns Temperature in degrees C
        public static int GetTemperature()
        {
            int[] ret = new int[STATUS_SIZE];
            GetMelStatus(ret);
            return ret[5];
        }

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private static int GetMelStatus(int[] data);
    }
    /// <summary>
    /// USB Serial Interface hacked by NPS at Samraksh 2020-07-22
    /// </summary>
    public class UsbSerialInterface
    {
        // RX Event stuff attempting to use Native_UART/SerialPort.cs as template
        // This all feels very Loony Tunes to me as an embedded C guy but whatever
        private bool m_fDisposed;
        public delegate void SerialDataReceivedEventHandler();
        private NativeEventDispatcher m_evtDataEvent = null;
        private SerialDataReceivedEventHandler m_callbacksDataEvent = null;

        private event SerialDataReceivedEventHandler InternalDataRX
        {
            [MethodImplAttribute(MethodImplOptions.Synchronized)]
            add
            {
                if (m_fDisposed)
                {
                    throw new ObjectDisposedException();
                }
                SerialDataReceivedEventHandler callbacksOld = m_callbacksDataEvent;
                SerialDataReceivedEventHandler callbacksNew = (SerialDataReceivedEventHandler)Delegate.Combine(callbacksOld, value);

                try
                {
                    m_callbacksDataEvent = callbacksNew;
                    if (callbacksOld == null && m_callbacksDataEvent != null)
                    {
                        m_evtDataEvent.OnInterrupt += new NativeEventHandler(DataEventHandler);
                    }
                }
                catch
                {
                    m_callbacksDataEvent = callbacksOld;
                    throw;
                }
            }
            [MethodImplAttribute(MethodImplOptions.Synchronized)]
            remove
            {
                if (m_fDisposed)
                {
                    throw new ObjectDisposedException();
                }
                SerialDataReceivedEventHandler callbacksOld = m_callbacksDataEvent;
                SerialDataReceivedEventHandler callbacksNew = (SerialDataReceivedEventHandler)Delegate.Remove(callbacksOld, value);
                try
                {
                    m_callbacksDataEvent = callbacksNew;
                    if (m_callbacksDataEvent == null)
                    {
                        m_evtDataEvent.OnInterrupt -= new NativeEventHandler(DataEventHandler);
                    }
                }
                catch
                {
                    m_callbacksDataEvent = callbacksOld;
                    throw;
                }
            }
        }

        private void DataEventHandler(uint evt, uint data2, DateTime timestamp)
        {
            if (m_callbacksDataEvent != null)
            {
                //m_callbacksDataEvent(this, new SerialDataReceivedEventArgs((SerialData)evt));
                m_callbacksDataEvent();
            }
        }

        protected void Dispose(bool disposing)
        {
            if (!m_fDisposed)
            {
                try
                {
                    if (disposing)
                    {

                        if (m_callbacksDataEvent != null)
                        {
                            m_evtDataEvent.OnInterrupt -= new NativeEventHandler(DataEventHandler);
                            m_callbacksDataEvent = null;
                            m_evtDataEvent.Dispose();
                        }
                    }
                }
                finally
                {
                    m_fDisposed = true;
                }
            }
        }

        public uint BytesToRead { get { return BytesInBuffer(); } }
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private uint BytesInBuffer();

        /// <summary>Delegate for read callback</summary>
        /// <param name="readBytes">Bytes read</param>
        /// <param name="mode">Indicates source, typ 0</param>
        public delegate void ReceiveCallback(byte[] readBytes, int mode);

        /// <summary>Client callback (can be null)</summary>
        public event ReceiveCallback DataReceived;

        /// <summary>
        /// USB Serial Interface constructor
        /// </summary>
        public UsbSerialInterface(ReceiveCallback receiveCallback = null)
        {
            m_fDisposed = false;
            m_evtDataEvent = new NativeEventDispatcher("USBPortDataEvent", 0);
            if (receiveCallback != null)
            {
                DataReceived += receiveCallback;
            }
            InternalDataRX += PortHandler;
        }

        private void PortHandler()
        {
            var numBytes = BytesToRead;
            var recvBuffer = new byte[numBytes];
            mel_serial_rx(recvBuffer, numBytes);
            if (DataReceived == null)
            {
                return;
            } else
            {
                DataReceived(recvBuffer, 0); // Default mode 0 for serial
            }
        }

        // END RX HANDLER STUFF

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
        public const int DOWNSTREAM_LEN = 8; // floats
        public const int UPSTREAM_LEN = 256; // floats

        // State and defaults
        private bool collectUpStream;
        private const bool collectUpStream_reset = false;

        private bool collectDownStream;
        private const bool collectDownStream_reset = true;

        /// <summary>
        /// Audio Interface constructor
        /// </summary>
        public AudioInterface(float[] thresh = null) : base("AICallback", 0)
        {
            Initialize();
            if (thresh != null) change_class_thresh(thresh);
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

        public int change_class_thresh(float[] thresh)
        {
            if (thresh.Length != DOWNSTREAM_LEN) return -1;
            int ret = mel_set_thresh(thresh);
            return 0;
        }

        public float[] get_class_thresh()
        {
            float[] ret = new float[DOWNSTREAM_LEN];
            mel_get_thresh(ret);
            return ret;
        }

        /// <summary>
        /// Get Mel ML downstream class thresholds
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern void mel_get_thresh(float[] data);

        /// <summary>
        /// Set Mel ML thresholds
        /// </summary>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private extern int mel_set_thresh(float[] data);

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
            return set_fir_taps_internal(num_taps, taps);
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
#pragma warning restore 1591
