using System;

namespace Samraksh.eMote.Net
{
    /// <summary>
	/// Radio type is unknown
	/// </summary>
	public class UnknownRadioTypeException : Exception
    {
        /// <summary>
        /// Radio type is unknown
        /// </summary>
        public UnknownRadioTypeException() { }

        /// <summary>
        /// Radio type is unknown
        /// </summary>
        /// <param name="message"></param>
        public UnknownRadioTypeException(string message) : base(message) { }
    }

    /// <summary>
    /// Radio is not configured
    /// </summary>
    public class RadioNotConfiguredException : Exception
    {
        /// <summary>
        /// Radio is not configured
        /// </summary>
        public RadioNotConfiguredException() { }

        /// <summary>
        /// Radio is not configured
        /// </summary>
        /// <param name="message"></param>
        public RadioNotConfiguredException(string message) : base(message) { }
    }

    /// <summary>
    /// Radio is busy
    /// </summary>
    public class RadioBusyException : Exception
    {
        /// <summary>
        /// Radio is busy
        /// </summary>
        public RadioBusyException() { }

        /// <summary>
        /// Radio is busy
        /// </summary>
        /// <param name="message"></param>
        public RadioBusyException(string message) : base(message) { }
    }

    /// <summary>
    /// MAC is not configured
    /// </summary>
    public class MACNotConfiguredException : Exception
    {
        /// <summary>
        /// MAC is not configured
        /// </summary>
        public MACNotConfiguredException() { }

        /// <summary>
        /// MAC is not configured
        /// </summary>
        /// <param name="message"></param>
        public MACNotConfiguredException(string message) : base(message) { }

        /// <summary>
        /// MAC is not configured
        /// </summary>
        /// <param name="message"></param>
        /// <param name="innerException"></param>
        public MACNotConfiguredException(string message, Exception innerException) : base(message, innerException) { }
    }

    /// <summary>
    /// MAC type is unknown
    /// </summary>
    public class MACTypeMismatchException : Exception
    {
        /// <summary>
        /// MAC type is unknown
        /// </summary>
        public MACTypeMismatchException() { }

        /// <summary>
        /// MAC type is unknown
        /// </summary>
        /// <param name="message"></param>
        public MACTypeMismatchException(string message) : base(message) { }
    }
    #region commented code. Exception is not used.
    ///// <summary>
    ///// 
    ///// </summary>
    //public class CallbackNotConfiguredException : Exception
    //{
    //	/// <summary>
    //	/// 
    //	/// </summary>
    //	public CallbackNotConfiguredException() { }

    //	/// <summary>
    //	/// 
    //	/// </summary>
    //	/// <param name="message"></param>
    //	public CallbackNotConfiguredException(string message) : base(message) { }
    #endregion
}

