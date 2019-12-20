using Samraksh.eMote.Net.Radio;


// ReSharper disable once CheckNamespace
namespace Samraksh.eMote.Net.MAC
{
    /// <summary>CSMA class</summary>
    public class CSMA : MACBase
    {
        /// <summary>Constructor</summary>
        /// <param name="radioConfiguration">Radio configuratin to use</param>
        /// <param name="cca">Use Clear Channel Assessment (CCA)</param>
        /// <param name="numberOfRetries">Number of times to try resending packet</param>
        /// <param name="ccaSenseTime">Time (in milliseconds) to check for clear channel</param>
        /// <param name="bufferSize">Send buffer size</param>
        /// <param name="neighborLivenessDelay">Timeout (in milliseconds) to declare neighbor dead</param>
        public CSMA(IRadioConfiguration radioConfiguration, uint neighborLivenessDelay = 650, byte numberOfRetries = 0, byte bufferSize = 8, bool cca = true, byte ccaSenseTime = 120)
            : base(MACType.CSMA, radioConfiguration, neighborLivenessDelay, numberOfRetries, bufferSize, cca, ccaSenseTime)
        {
        }

    }
}
