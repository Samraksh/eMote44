#define DBG_RADARDETECTOR_VERBOSE

using System;
using Microsoft.SPOT;
using System.Collections;
using System.Threading;
using Samraksh.eMote.RadarInterfaceNS;

namespace Samraksh.eMote.RadarDetectorNS
{
    /// <summary> The detector class </summary>
    public class RadarDetector
    {
        /// <summary> A pair of buffers storing in fase and quadrature components of radar samples </summary>
        /// <remarks>Lets a pair of buffers be handled together as a single object</remarks>
        public class IQ
        {
            // ReSharper disable once InconsistentNaming
            public readonly ushort[] IBuff;
            public readonly ushort[] QBuff;
            public IQ(ushort[] iBuff, ushort[] qBuff)
            {
                IBuff = iBuff;
                QBuff = qBuff;
            }
        }
        /// <summary> The status of the a given buffer </summary>
        public enum DetectionStatus
        {
            /// <summary> Speculative </summary>
            Speculative,
            /// <summary> Confirmed </summary>
            Confirmed
        }

        /// <summary>Callback method </summary>
        public delegate void RadarDetectorCallBack(IQ iq, DetectionStatus WindowDetectionStatus, int TotalRotationsOverWindow,bool is_last_window);

        /// <summary> Public event for passing the decision results.</summary>
        public event RadarDetectorCallBack m_detectorCallback;

        /// <summary> Public event for passing the decision results.</summary>
        /// <param name="_callbackhandler"> the calback handler. Can be null.  </param>
        /// <param name="_radar"> radar interface to be used by the detector. If null, a new radar interface is created. </param>
        public RadarDetector( RadarDetectorCallBack _callbackhandler = null, RadarInterface _radar = null)
        {
            if (_radar == null) radar = new RadarInterface();
            else radar = _radar;
            if (radar == null) throw new Exception("HumanDetector cannot claim radar interface");

            if (_callbackhandler != null) m_detectorCallback += _callbackhandler;

            var bufferThread = new Thread(this.SampleBufferQueue);
            bufferThread.Start();


            radar.ConfigureFPGADetection(ADCBufferI, ADCBufferQ, ADCBufferqdiff, ADCBufferqdiffMovSum, ADCBufferMofNCOunt, ADCBufferDetect, ADCBufferSize, ADCCallback);
        }

        #region Private Methods
        private const int ADCBufferSize = 128; // Number of ushorts per ADC buffer
        private static RadarInterface radar;

        // A semaphore used by the ADC callback to signal WriteSampleBufferQueue that data is ready for processing
        private static readonly AutoResetEvent SampleBufferSemaphore = new AutoResetEvent(false);
        private bool _collectIsDone = false;

        // The ADC buffers that are populated by the ADC driver
        private static readonly ushort[] ADCBufferI = new ushort[ADCBufferSize];
        private static readonly ushort[] ADCBufferQ = new ushort[ADCBufferSize];
        private static readonly Int16[] ADCBufferqdiff = new Int16[ADCBufferSize];
        private static readonly Int16[] ADCBufferqdiffMovSum = new Int16[ADCBufferSize];
        private static readonly Int16[] ADCBufferMofNCOunt = new Int16[ADCBufferSize];
        private static bool[] ADCBufferDetect = new bool[ADCBufferSize];


        /// <summary>
        /// ADC callback
        /// </summary>
        /// <remarks>Called when the ADC driver has collected a buffer's worth of data</remarks>
        /// <param name="threshhold"></param>
        private void ADCCallback(long threshhold)
        {
            radar.SetProcessingInProgress(true);
            Debug.Print("ADCCallback start ");
            // Signal the processing thread
            SampleBufferSemaphore.Set();
        }

        /// <summary>
        /// Process the sample buffer in a separate thread
        /// </summary>
        /// <remarks>
        /// We do this so that the ADC callback will return quickly.
        /// The main program blocks until this thread ends
        /// </remarks>
        private void SampleBufferQueue()
        {
            // Run until user indicates end or there is a queue-full error
            while (true)
            {
                // Wait for signal that a buffer is ready for processing
                SampleBufferSemaphore.WaitOne();

       
                // Get a buffer to process

                DetectionStatus detection_radar;
                if (ADCBufferDetect[ADCBufferSize-1]) detection_radar = DetectionStatus.Confirmed;
                else detection_radar = DetectionStatus.Speculative;

                int cur_distance = ADCBufferqdiffMovSum[ADCBufferSize-1];
                bool is_last_window = radar.CurrentDetectionFinished();
                IQ iq = new RadarDetector.IQ(ADCBufferI, ADCBufferQ);

                bool assumedDetect = false;
                int total_rots = 0;

                for (int i = 0;  i < ADCBufferDetect.Length; ++i)
                {
                    if (ADCBufferDetect[i])
                    {
                        assumedDetect = true;
                    }
                    total_rots = ADCBufferqdiff[i];


                }

                Debug.Print("SampleBufferQueue assumedDetect=" + assumedDetect + ", total_rots=" + total_rots ); 
                               
                m_detectorCallback(iq, detection_radar, total_rots, is_last_window);
            
                radar.SetProcessingInProgress(false);
                // Check if the end-sampling flag is set. If so, we're done
                if (!_collectIsDone)
                {
                    continue;
                }
                //Debug.Print("Finished sampling");
                //AnalogInput.StopSampling();
                // Terminate the thread and join with the main program
                return;
            }
        }

            #endregion


        }




}
