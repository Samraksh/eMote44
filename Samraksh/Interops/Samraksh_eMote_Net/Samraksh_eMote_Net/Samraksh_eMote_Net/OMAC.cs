using Samraksh.eMote.Net.Radio;


namespace Samraksh.eMote.Net.MAC
{
    /// <summary>OMAC class</summary>
    public class OMAC : MACBase
    {
        /// <summary>OMAC constructor</summary>
        /// <param name="radioConfiguration">Radio configuration to use</param>
        /// <param name="cca">Use clear channel assessment</param>
        /// <param name="numberOfRetries">Number of times to retry sending a packet</param>
        /// <param name="ccaSenseTime">cca sense time</param>
        /// <param name="bufferSize">Size of send buffer</param>
        /// <param name="neighborLivenessDelay">Time to wait (in milliseconds) before declaring a neighbor dead</param>
        public OMAC(IRadioConfiguration radioConfiguration, uint neighborLivenessDelay = 750, byte numberOfRetries = 0, byte bufferSize = 8, bool cca = true, byte ccaSenseTime = 120)
            : base(MACType.OMAC, radioConfiguration, neighborLivenessDelay, numberOfRetries, bufferSize, cca, ccaSenseTime)
        {
        }

    }
}
