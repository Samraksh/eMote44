using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;

namespace Samraksh.eMote
{
    /// <summary>
    /// Delegate for Radar callback method
    /// </summary>
    public delegate void RadarCallBack(long time);


    /// <summary>
    /// The public interface for the radar
    /// </summary>
    public class RadarInterface : IRadarInterface
    {
        /// <summary>
        /// ADC internal class 
        /// </summary>
        static RadarInternal _radarInternal;

        /// <summary>
        /// Callback method
        /// </summary>
        static RadarCallBack _myCallback;
        /// <summary>
        /// 
        /// </summary>
        public RadarInterface()
        {
            _radarInternal = new RadarInternal("RadarCallback", 1234, 0);
        }

        //////////////////////////public properties and methods/////////////////////

        /// <summary>
        /// 
        /// </summary>
        ~RadarInterface()
        {
            _radarInternal = null;
            TurnOff();
            RadarInternal.Uninit();
        }

        /// <summary> Turns on the Radar </summary>
        /// <param name="sampleBuffI">Buffer for samples</param>
        /// <param name="sampleBuffQ">Buffer for samples</param>
        /// <param name="sampleqdiff">Buffer for samples</param>
        /// <param name="sampleqdiffMovSum">Buffer for samples</param>
        /// <param name="sampleMofNCOunt">Buffer for samples</param>
        /// <param name="sampleDetect">Buffer for samples</param>
        /// <param name="numSamples">Number of samples</param>
        /// <param name="callback">Method to call when numSamples collected</param>
        /// <returns>True if operation success</returns>
        /// <returns>The result of turning on the radar: Success, Fail</returns>
        public bool ConfigureFPGADetection(ushort[] sampleBuffI, ushort[] sampleBuffQ, Int16[] sampleqdiff, Int16[] sampleqdiffMovSum, Int16[] sampleMofNCOunt, bool[] sampleDetect,  uint numSamples, RadarCallBack callback)
        {
            _myCallback = callback;
            var eventHandler = new NativeEventHandler(InternalCallback);
            _radarInternal.OnInterrupt += eventHandler;

            if (_radarInternal.ConfigureFPGADetectionPrivate(sampleBuffI, sampleBuffQ, sampleqdiff, sampleqdiffMovSum, sampleMofNCOunt, sampleDetect,numSamples))
                return true;
            else
                return false;
        }

        /// <summary>
        /// Request whether the current detection has finished
        /// </summary>
        /// <returns>Returns true if the current detection is finished and false if not.</returns>
        public bool CurrentDetectionFinished()
        {
            return _radarInternal.CurrentDetectionFinished();
        }

        /// <summary>
        /// Inform firmware of status of data processing
        /// </summary>
        /// <returns>Nothing returned</returns>
        public void SetProcessingInProgress(bool status)
        {
            _radarInternal.SetProcessingInProgress(status);
        }

        /// <summary>
        /// Ask FPGA to continue sending a number of buffers after last detect. The default value is 6. 
        /// </summary>
        /// <returns>Nothing returned</returns>
        public void setContinueToSendCount(UInt16 s)
        {
            _radarInternal.setContinueToSendCount(s);
        }

        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        public UInt16 getContinueToSendCount()
        {
            return _radarInternal.getContinueToSendCount();
        }

        /// <summary>
        /// Ask FPGA to continue sending a number of buffers after last detect. The default value is 6. 
        /// </summary>
        /// <returns>Nothing returned</returns>
        public void setNumLookAheadWindows(UInt16 s)
        {
            _radarInternal.setNumLookAheadWindows(s);
        }

        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        public UInt16 getNumLookAheadWindows()
        {
            return _radarInternal.getNumLookAheadWindows();
        }


        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        public UInt16 getNumDetectionsInWindow()
        {
            return _radarInternal.getNumDetectionsInWindow();
        }

        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        public Int16 getTotalRotationsofWindow()
        {
            return _radarInternal.getTotalRotationsofWindow();
        }

        /// <summary>
        /// Native Radar driver callback
        /// </summary>
        /// <param name="data1">Parameter passed from native</param>
        /// <param name="data2">Parameter passed from native</param>
        /// <param name="time">Time of callback</param>
        static public void InternalCallback(uint data1, uint data2, DateTime time)
        {
            _myCallback((long)(((long)data1 << 32) | (long)data2));
        }
        ///////////////////////////////////Internal methods/////////////////////////



        /// <summary>
        /// Turns on the Radar
        /// </summary>
        /// <returns>The result of turning on the radar: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool TurnOn();

        /// <summary>
        /// Turns off the Radar
        /// </summary>
        /// <returns>The result of turning off the radar: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool TurnOff();

        /// <summary>
        /// Blinds the Radar
        /// </summary>
        /// <returns>The result of blinding the radar: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool Blind();
    }

    /// <summary>
    /// ADCInternal interface to the native driver
    /// </summary>
    class RadarInternal : NativeEventDispatcher
    {
        /// <summary>
        /// Specify the driver name for matching with the native eventdispatcher
        /// </summary>
        /// <param name="strDrvName"></param>
        /// <param name="drvData"></param>
        /// <param name="callbackCount"></param>
        public RadarInternal(string strDrvName, ulong drvData, int callbackCount)
            : base(strDrvName, drvData)
        {

        }

        /// <summary>
        /// Initialize the Radar native driver
        /// </summary>
        /// <param name="channel">Specify the channel to be sampled</param>
        /// <returns>Returns the result of the init function</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public int Init(int channel);

        /// <summary>
        /// Uninitialize the Radar native driver
        /// </summary>
        /// <returns>Returns the result of the uninit function</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern static public bool Uninit();

        /// <summary>
        /// Turns on the Radar
        /// </summary>
        /// <param name="sampleBuffI">Buffer for samples</param>
        /// <param name="sampleBuffQ">Buffer for samples</param>
        /// <param name="sampleqdiff">Buffer for samples</param>
        /// <param name="sampleqdiffMovSum">Buffer for samples</param>
        /// <param name="sampleMofNCOunt">Buffer for samples</param>
        /// <param name="sampleDetect">Buffer for samples</param>
        /// <param name="numSamples">Number of samples</param>
        /// <returns>True if operation success</returns>
        /// <returns>The result of turning on the radar: Success, Fail</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        public extern bool ConfigureFPGADetectionPrivate(ushort[] sampleBuffI, ushort[] sampleBuffQ, Int16[] sampleqdiff, Int16[] sampleqdiffMovSum, Int16[] sampleMofNCOunt, bool[] sampleDetect, uint numSamples);


        /// <summary>
        /// Request whether the current detection has finished
        /// </summary>
        /// <returns>Returns true if the current detection is finished and false if not.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern bool CurrentDetectionFinished();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void setContinueToSendCount(UInt16 s);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern UInt16 getContinueToSendCount();

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void setNumLookAheadWindows(UInt16 s);

        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern UInt16 getNumLookAheadWindows();



        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern UInt16 getNumDetectionsInWindow();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern Int16 getTotalRotationsofWindow();
        

        /*
        /// <summary>
        /// Request the total displacement over the window
        /// </summary>
        /// <returns>Returns the window's displacement.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetDisplacement();
        /// <summary>
        /// Request the window's displacement range.
        /// </summary>
        /// <returns>Returns the window's displacement range.</returns>
        /// 
       /// <summary>
        /// Request the window's net displacement
        /// </summary>
        /// <returns>Returns the window's net displacement.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetNetDisplacement(SAMPLE_WINDOW_PORTION portion);


        /// <summary>
        /// Request the window's absolute displacement.
        /// </summary>
        /// <returns>Returns the window's absolute displacement.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetAbsoluteDisplacement(SAMPLE_WINDOW_PORTION portion);
        /// <summary>
        /// Request the window's displacement range.
        /// </summary>
        /// <returns>Returns the window's displacement range.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetDisplacementRange(SAMPLE_WINDOW_PORTION portion);

        /// <summary>
        /// Request the window's count of samples over classifierTargetFilter parameter.
        /// </summary>
        /// <returns>Returns the window's count of samples over target.</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern int GetCountOverTarget();
        */
        /// <summary>
        /// Inform firmware of status of data processing
        /// </summary>
        /// <returns>Nothing returned</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        public extern void SetProcessingInProgress(bool status);
    }


}
