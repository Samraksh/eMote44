namespace Samraksh.eMote
{
    /// <summary>
    /// The public interface for the radar
    /// </summary>
    public interface IRadarInterface
    {
        /// <summary>
        /// Blinds the Radar
        /// </summary>
        /// <returns>The result of blinding the radar: Success, Fail</returns>
        bool Blind();

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
        bool ConfigureFPGADetection(ushort[] sampleBuffI, ushort[] sampleBuffQ, short[] sampleqdiff, short[] sampleqdiffMovSum, short[] sampleMofNCOunt, bool[] sampleDetect, uint numSamples, RadarCallBack callback);
        /// <summary>
        /// Request whether the current detection has finished
        /// </summary>
        /// <returns>Returns true if the current detection is finished and false if not.</returns>
        bool CurrentDetectionFinished();
        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        ushort getContinueToSendCount();
        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        ushort getNumDetectionsInWindow();
        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        ushort getNumLookAheadWindows();
        /// <summary>
        /// Gets remainng count for the FPGA to send after the last detect. 
        /// </summary>
        short getTotalRotationsofWindow();
        /// <summary>
        /// Ask FPGA to continue sending a number of buffers after last detect. The default value is 6. 
        /// </summary>
        /// <returns>Nothing returned</returns>
        void setContinueToSendCount(ushort s);

        /// <summary>
        /// Ask FPGA to continue sending a number of buffers after last detect. The default value is 6. 
        /// </summary>
        /// <returns>Nothing returned</returns>
        void setNumLookAheadWindows(ushort s);

        /// <summary>
        /// Inform firmware of status of data processing
        /// </summary>
        /// <returns>Nothing returned</returns>
        void SetProcessingInProgress(bool status);
        /// <summary>
        /// Turns off the Radar
        /// </summary>
        /// <returns>The result of turning off the radar: Success, Fail</returns>
        bool TurnOff();
        /// <summary>
        /// Turns on the Radar
        /// </summary>
        /// <returns>The result of turning on the radar: Success, Fail</returns>
        bool TurnOn();
    }
}