using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;

namespace Samraksh.eMote
{
    /// <summary>
    /// Options for setting the IQ rejection parameter for the Radar Detection class  
    /// </summary>
    public enum RADAR_NOISE_CONTROL
    {
        /// <summary>The noise rejection parameter is fixed at noiseRejection.</summary>
        FIXED_NOISE_REJECTION,
        /// <summary>The noise rejection parameter will be the sum of the background noise plus the parameter noiseRejection.</summary>
        SCALING_NOISE_REJECTION_ADD,
        /// <summary>The noise rejection parameter will be the product of the background noise times the parameter noiseRejection.</summary>
        SCALING_NOISE_REJECTION_MULTIPLY,
        /// <summary>The noise rejection parameter will adjusted until we reach the specified background noise of the noiseRejection parameter.</summary>
        SCALING_NOISE_REJECTION_TARGET,
        /// <summary>The noise rejection parameter will adjusted based on the median of the raw radar values.</summary>
        SCALING_NOISE_REJECTION_RAW_RADAR,
        /// <summary>The noise rejection parameter will scale depending upon how high the median of the raw radar values are. Higher medians are suppressed more.</summary>
        SCALING_NOISE_REJECTION_RAW_RADAR_SCALING
    };

    /// <summary>
    /// Options for requesting the IQ rejection parameter for the Radar Detection class  
    /// </summary>
    public enum RADAR_NOISE_REQUEST
    {
        /// <summary>The noise rejection value set to zero</summary>
        IQ_REJECTION_ZERO,
        /// <summary>The noise rejection value set to max</summary>
        IQ_REJECTION_MAX,
        /// <summary>The noise rejection value currently used by the system</summary>
        IQ_REJECTION_CURRENTLY_USED,
        /// <summary>The current radar Q noise.</summary>
        RADAR_Q
    };

    /// <summary>
    /// Options for how much of the sample window is used in determining a request for information.  
    /// </summary>
    public enum SAMPLE_WINDOW_PORTION
    {
        /// <summary>The entire ADC sample window is used.</summary>
        SAMPLE_WINDOW_FULL,
        /// <summary>The first half of the ADC sample window is used.</summary>
        SAMPLE_WINDOW_FIRST_HALF,
        /// <summary>The second half of the ADC sample window is used.</summary>
        SAMPLE_WINDOW_SECOND_HALF
    };

    public class Algorithm
    {
        /// <summary>
        /// Radar Detection class  
        /// </summary>
        public class RadarDetection
        {            
            /// <summary>
            /// 
            /// </summary>
            public RadarDetection() {
                Initialize();
                SetDetectionParameters(RADAR_NOISE_CONTROL.SCALING_NOISE_REJECTION_RAW_RADAR_SCALING, 2.1, 1, 2, 3, 0, 0, 310);
            }
            
            //////////////////////////public properties and methods/////////////////////

            /// <summary>
            /// 
            /// </summary>
            ~RadarDetection()
            {
                Uninitialize();
            }

            ///////////////////////////////////Internal methods/////////////////////////

            /// <summary>
            /// Initializes the RadarDetection software
            /// </summary>
            /// <returns>The result of radar detection initialization: Success, Fail</returns>
            [MethodImpl(MethodImplOptions.InternalCall)]
            private extern bool Initialize();

            /// <summary>
            /// Uninitializes the RadarDetection software
            /// </summary>
            /// <returns>The result of radar detection initialization: Success, Fail</returns>
            [MethodImpl(MethodImplOptions.InternalCall)]
            private extern bool Uninitialize();

            /// <summary>
            /// Radar data is given to the unwrap algorithm which looks for displacements above the threshold.
            /// </summary>
            /// <returns>Returns true if a detection was found, false if no detection was found.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool DetectionCalculation(ushort[] bufferI, ushort[] bufferQ, ushort[] bufferUnwrap, Int32 numBytes);

            /// <summary>
            /// Radar data is given to the unwrap algorithm which looks for displacements above the threshold.
            /// </summary>
            /// <returns>Returns true if a detection was found, false if no detection was found.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool DetectionCalculation(ushort[] bufferI, ushort[] bufferQ, Int32 numBytes);

            /// <summary>
            /// Sets detection parameters.
            /// </summary>
            /// <param name="radarNoiseCtrl">The type of algorithm used to control background noise.</param>
            /// <param name="thresholdSet">The threshold used to determine if there is a detection. Lower thresholds increase sensitivity but might cause false alarms.</param>
            /// <param name="noiseRejection">The parameter used in the algorithm controling background noise. 1 is default. Higher suppresses noise more but hurts sensitivity.</param>
            /// <param name="M">The number of M windows used in the M out of N detection.</param>
            /// <param name="N">The number of windows used in the M out of N detection.</param>
            /// <param name="debugVal">A value that outputs raw radar data in different formats to the serial port.</param>
            /// <param name="targetSizeFilter">A small target filter that suppresses windows where no radar data exceeds this value. The higher the value the greater the suppression. Generally only this or classifierTargetFilter is used.</param>
            /// <param name="classifierTargetFilter">A small target filter that partially suppresses windows where no radar data exceeds this value. The higher the value the greater the suppression. Generally only this or targetSizeFilter is used.</param>
            /// <returns>Returns true if parameters are set, false if there was a problem.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool SetDetectionParameters(RADAR_NOISE_CONTROL radarNoiseCtrl, double thresholdSet, double noiseRejection, ushort M, ushort N, UInt16 debugVal, UInt16 targetSizeFilter, UInt16 classifierTargetFilter);

            /// <summary>
            /// Request the background noise level
            /// </summary>
            /// <returns>Return the current background noise level.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetBackgroundNoiseLevel(RADAR_NOISE_REQUEST iqRequestType);

            /// <summary>
            /// Reset background noise tracking
            /// </summary>
            /// <returns>Returns true if reset occured, false if there was no reset.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool ResetBackgroundNoiseTracking();

            /// <summary>
            /// Request the IQ rejection currently used
            /// </summary>
            /// <returns>Return the current IQ rejection level.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetIQRejectionLevel();

            /// <summary>
            /// Request the last unwrap value
            /// </summary>
            /// <returns>Returns the last unwrap value.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetLastUnwrap(RADAR_NOISE_REQUEST iqRequestType);

            /// <summary>
            /// Request whether the last window's displacement was over the threshold
            /// </summary>
            /// <returns>Returns true if last window's displacement was over the threshold and false if not.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool GetWindowOverThreshold();

            /// <summary>
            /// Request whether the current detection has finished
            /// </summary>
            /// <returns>Returns true if the current detection is finished and false if not.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool CurrentDetectionFinished();

            /// <summary>
            /// Request the window's net displacement
            /// </summary>
            /// <returns>Returns the window's net displacement.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetNetDisplacement(SAMPLE_WINDOW_PORTION portion);

            /// <summary>
            /// Request the window's absolute displacement.
            /// </summary>
            /// <returns>Returns the window's absolute displacement.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetAbsoluteDisplacement(SAMPLE_WINDOW_PORTION portion);
            /// <summary>
            /// Request the window's displacement range.
            /// </summary>
            /// <returns>Returns the window's displacement range.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetDisplacementRange(SAMPLE_WINDOW_PORTION portion);

            /// <summary>
            /// Request the window's count of samples over classifierTargetFilter parameter.
            /// </summary>
            /// <returns>Returns the window's count of samples over target.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public int GetCountOverTarget();
        }

        /// <summary>
        /// Classifier class  
        /// </summary>
        public class DecisionFunction
        {
            /// <summary>
            /// Classifier class
            /// </summary>
            public DecisionFunction()
            {
                
            }


            /// <summary>
            /// Initialization for the decision function
            /// </summary>
            /// <returns>A pointer to the nomralized vectors is returned.</returns>
            /// 
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public void Initialize(UInt16 p_nSV, UInt16 p_nFeature, float p_rho, float p_gamma, float[] p_weight, float[] p_feature_min, float[] p_scalingFactors);

            //////////////////////////public properties and methods/////////////////////

            /// <summary>
            /// 
            /// </summary>
            ~DecisionFunction()
            {
            }

            ///////////////////////////////////Internal methods/////////////////////////

            /// <summary>
            /// The feature vector is normailized
            /// </summary>
            /// <returns>A pointer to the nomralized vectors is returned.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public void NormalizeFeatureVector(float[] featureVector, int[] normalizedVector);

            /// <summary>
            /// A decision is made based on the support vecorts
            /// </summary>
            /// <returns>A decision is returned.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public float Decide(int[] normalizedVector);
        }

        /// <summary>
        /// Acoustic Detection class  
        /// </summary>
        public class AcousticDetection
        {
            /// <summary>
            /// 
            /// </summary>
            public AcousticDetection()
            {
                SetDetectionParameters(1, 1);
            }

            //////////////////////////public properties and methods/////////////////////

            /// <summary>
            /// 
            /// </summary>
            ~AcousticDetection()
            {
            }

            ///////////////////////////////////Internal methods/////////////////////////

            /// <summary>
            /// Acoustic data is given to the detection algorithm which looks for detections.
            /// </summary>
            /// <returns>Returns true if a detection was found, false if no detection was found.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool DetectionCalculation(ushort[] bufferAcoustic, Int32 numBytes, double[] processingOutput, bool historyUpdateControl);

            /// <summary>
            /// Sets detection parameters.
            /// </summary>
            /// <returns>Returns true if sucessful, false if failed.</returns>
            [MethodImplAttribute(MethodImplOptions.InternalCall)]
            extern public bool SetDetectionParameters(Int32 Pa, Int32 Pm);
        }
    }
}
