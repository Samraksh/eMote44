// ReSharper disable once CheckNamespace
namespace Samraksh.eMote.Net.Radio
{
	/// <summary>
	/// Kind of user
	/// </summary>
	public enum RadioUser
	{
		/// <summary>
		/// Radio Object is used by a C# application
		/// </summary>
		CSharp,
		/// <summary>
		/// Radio is being used by the CSMA MAC object
		/// </summary>
		CSMA,
		/// <summary>
		/// Radio is being used by the OMAC MAC object
		/// </summary>
		OMAC,
		/// <summary>
		/// Radio is idle
		/// </summary>
		Idle,
	}

	/// <summary>Name of radio</summary>
	public enum RadioName
	{
		/// <summary>On-board radio</summary>
		RF231,
		/// <summary>Long-range radio</summary>
		RF231LR,
		/// <summary>Long range (433 MHz) radio</summary>
		SI4468
	}

	#region Commented code
	/*/// <summary></summary>
	//public enum TxPowerValueSI4468
	//{
	//	/// <summary></summary>
	//	Power_SI4468_3dBm = 16,
	//	/// <summary></summary>
	//	Power_SI4468_2Point8dBm = 17
	//};

	///// <summary>
	///// Power levels supported by the RF231 radio
	///// </summary>
	//public enum TxPowerValueRF231
	//{
	//	/// <summary>+3.0 dB</summary>
	//	Power_3dBm = 0,
	//	/// <summary>+2.8 dB</summary>
	//	Power_2Point8dBm = 1,
	//	/// <summary>+2.3 dB</summary>
	//	Power_2Point3dBm = 2,
	//	/// <summary>+1.8 dB</summary>
	//	Power_1Point8dBm = 3,
	//	/// <summary>+1.3 dB</summary>
	//	Power_1Point3dBm = 4,
	//	/// <summary>+0.7 dB</summary>
	//	Power_0Point7dBm = 5,
	//	/// <summary>+0.0 dB</summary>
	//	Power_0Point0dBm = 6,
	//	/// <summary>-1.0 dB</summary>
	//	Power_Minus1dBm = 7,
	//	/// <summary>-2.0 dB</summary>
	//	Power_Minus2dBm = 8,
	//	/// <summary>-3.0 dB</summary>
	//	Power_Minus3dBm = 9,
	//	/// <summary>-4.0 dB</summary>
	//	Power_Minus4dBm = 10,
	//	/// <summary>-5.0 dB</summary>
	//	Power_Minus5dBm = 11,
	//	/// <summary>-7.0 dB</summary>
	//	Power_Minus7dBm = 12,
	//	/// <summary>-9.0 dB</summary>
	//	Power_Minus9dBm = 13,
	//	/// <summary>-12.0 dB</summary>
	//	Power_Minus12dBm = 14,
	//	/// <summary>-17.0 dB</summary>
	//	Power_Minus17dBm = 15,
	//};

	///// <summary></summary>
	//public class TxPowerValue
	//{
	//	/// <summary></summary>
	//	public UInt16 txPowerValue;
	//}

	///// <summary></summary>
	//public enum ChannelRF231
	//{
	//	/// <summary></summary>
	//	Channel_RF231_11 = 0,
	//	/// <summary></summary>
	//	Channel_RF231_12 = 1,
	//	/// <summary></summary>
	//	Channel_RF231_26 = 15
	//};

	///// <summary></summary>
	//public enum ChannelSI4468 
	//{
	//	/// <summary></summary>
	//	Channel_SI4468_01 = 16,
	//	/// <summary></summary>
	//	Channel_SI4468_02 = 17
	//};

	///// <summary>Channels the RF231 radio can use</summary>
	//public class Channel
	//{
	//	/// <summary></summary>
	//	public UInt16 channelType;

	//	/// <summary></summary>
	//	public Channel()
	//	{
	//		//channelType = ChannelRF231.Channel_RF231_26;
	//	}
	//};*/
	#endregion 

	// Separated power values and channels into enums according to radio type - Bill

	/// <summary>
	/// Power levels supported by the RF231 radio (onboard and long range)
	/// </summary>
	public enum RF231TxPower
	{
		/// <summary>+3.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_3dBm = 0,
        /// <summary>+2.8 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_2Point8dBm = 1,
        /// <summary>+2.3 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_2Point3dBm = 2,
        /// <summary>+1.8 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_1Point8dBm = 3,
        /// <summary>+1.3 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_1Point3dBm = 4,
        /// <summary>+0.7 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_0Point7dBm = 5,
        /// <summary>+0.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_0Point0dBm = 6,
        /// <summary>-1.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus1dBm = 7,
        /// <summary>-2.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus2dBm = 8,
        /// <summary>-3.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus3dBm = 9,
        /// <summary>-4.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus4dBm = 10,
        /// <summary>-5.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus5dBm = 11,
        /// <summary>-7.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus7dBm = 12,
        /// <summary>-9.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus9dBm = 13,
        /// <summary>-12.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus12dBm = 14,
        /// <summary>-17.0 dB</summary>
		// ReSharper disable once InconsistentNaming
		Power_Minus17dBm = 15,
	}

	/// <summary>
	/// Power levels supported by the SI4468 radio
	/// </summary>
	public enum SI4468TxPower
	{
		/// <summary>20 dB</summary>
		Power_20dBm = 127,
		/// <summary>19.7 dB</summary>
		Power_19Point7dBm = 126,
		/// <summary>19.4 dB</summary>
		Power_19Point4dBm = 125,
        /// <summary>19.1 dB</summary>
        Power_19Point1dBm = 124,
        /// <summary>18.8 dB</summary>
        Power_18Point8dBm = 123,
        /// <summary>18.5 dB</summary>
        Power_18Point5dBm = 122,
        /// <summary>18.2 dB</summary>
        Power_18Point2dBm = 121,
        /// <summary>17.9 dB</summary>
        Power_17Point9dBm = 120,
        /// <summary>17.6 dB</summary>
        Power_17Point6dBm = 119,
        /// <summary>17.3 dB</summary>
        Power_17Point3dBm = 118,
        /// <summary>17.0 dB</summary>
        Power_17Point0dBm = 117,
        /// <summary>16.7 dB</summary>
        Power_16Point7dBm = 116,
        /// <summary>16.4 dB</summary>
        Power_16Point4dBm = 115,
        /// <summary>16.1 dB</summary>
        Power_16Point1dBm = 114,
        /// <summary>15.8 dB</summary>
        Power_15Point8dBm = 113,
        /// <summary>15.5 dB</summary>
        Power_15Point5dBm = 112,
        /// <summary>15.2 dB</summary>
        Power_15Point2dBm = 111,
        /// <summary>14.9 dB</summary>
        Power_14Point9dBm = 110,
        /// <summary>14.6 dB</summary>
        Power_14Point6dBm = 109,
        /// <summary>14.3 dB</summary>
        Power_14Point3dBm = 108,
        /// <summary>14.0 dB</summary>
        Power_14Point0dBm = 107,
        /// <summary>13.7 dB</summary>
        Power_13Point7dBm = 106,
        /// <summary>13.4 dB</summary>
        Power_13Point4dBm = 105,
        /// <summary>13.1 dB</summary>
        Power_13Point1dBm = 104,
        /// <summary>12.8 dB</summary>
        Power_12Point8dBm = 103,
        /// <summary>12.5 dB</summary>
        Power_12Point5dBm = 102,
        /// <summary>12.2 dB</summary>
        Power_12Point2dBm = 101,
        /// <summary>11.9 dB</summary>
        Power_11Point9dBm = 100,
        /// <summary>11.6 dB</summary>
        Power_11Point6dBm = 99,
        /// <summary>11.3 dB</summary>
        Power_11Point3dBm = 98,
        /// <summary>11.0 dB</summary>
        Power_11Point0dBm = 97,
        /// <summary>10.7 dB</summary>
        Power_10Point7dBm = 96,
        /// <summary>10.4 dB</summary>
        Power_10Point4dBm = 95,
        /// <summary>10.1 dB</summary>
        Power_10Point1dBm = 94,
        /// <summary>9.8 dB</summary>
        Power_9Point8dBm = 93,
        /// <summary>9.5 dB</summary>
        Power_9Point5dBm = 92,
        /// <summary>9.2 dB</summary>
        Power_9Point2dBm = 91,
        /// <summary>8.9 dB</summary>
        Power_8Point9dBm = 90,
        /// <summary>8.6 dB</summary>
        Power_8Point6dBm = 89,
        /// <summary>8.3 dB</summary>
        Power_8Point3dBm = 88,
        /// <summary>8.0 dB</summary>
        Power_8Point0dBm = 87,
        /// <summary>7.7 dB</summary>
        Power_7Point7dBm = 86,
        /// <summary>7.4 dB</summary>
        Power_7Point4dBm = 85,
        /// <summary>7.1 dB</summary>
        Power_7Point1dBm = 84,
        /// <summary>6.8 dB</summary>
        Power_6Point8dBm = 83,
        /// <summary>6.5 dB</summary>
        Power_6Point5dBm = 82,
        /// <summary>6.2 dB</summary>
        Power_6Point2dBm = 81,
        /// <summary>5.9 dB</summary>
        Power_5Point9dBm = 80,
        /// <summary>5.6 dB</summary>
        Power_5Point6dBm = 79,
        /// <summary>5.3 dB</summary>
        Power_5Point3dBm = 78,
        /// <summary>5.0 dB</summary>
        Power_5Point0dBm = 77,
        /// <summary>4.7 dB</summary>
        Power_4Point7dBm = 76,
        /// <summary>4.4 dB</summary>
        Power_4Point4dBm = 75,
        /// <summary>4.1 dB</summary>
        Power_4Point1dBm = 74,
        /// <summary>3.8 dB</summary>
        Power_3Point8dBm = 73,
        /// <summary>3.5 dB</summary>
        Power_3Point5dBm = 72,
        /// <summary>3.2 dB</summary>
        Power_3Point2dBm = 71,
        /// <summary>2.9 dB</summary>
        Power_2Point9dBm = 70,
        /// <summary>2.6 dB</summary>
        Power_2Point6dBm = 69,
        /// <summary>2.3 dB</summary>
        Power_2Point3dBm = 68,
        /// <summary>2.0 dB</summary>
        Power_2Point0dBm = 67,
        /// <summary>1.7 dB</summary>
        Power_1Point7dBm = 66,
        /// <summary>1.4 dB</summary>
        Power_1Point4dBm = 65,
        /// <summary>1.1 dB</summary>
        Power_1Point1dBm = 64,
        /// <summary>0.8 dB</summary>
        Power_0Point8dBm = 63,
        /// <summary>0.5 dB</summary>
        Power_0Point5dBm = 62,
        /// <summary>0.2 dB</summary>
        Power_0Point2dBm = 61,
        /// <summary>-0.1 dB</summary>
        Power_Minus0Point1dBm = 60,
        /// <summary>-0.4 dB</summary>
        Power_Minus0Point4dBm = 59,
        /// <summary>-0.7 dB</summary>
        Power_Minus0Point7dBm = 58,
        /// <summary>-1.0 dB</summary>
        Power_Minus1Point0dBm = 57,
        /// <summary>-1.3 dB</summary>
        Power_Minus1Point3dBm = 56,
        /// <summary>-1.6 dB</summary>
        Power_Minus1Point6dBm = 55,
        /// <summary>-1.9 dB</summary>
        Power_Minus1Point9dBm = 54,
        /// <summary>-2.2 dB</summary>
        Power_Minus2Point2dBm = 53,
        /// <summary>-2.5 dB</summary>
        Power_Minus2Point5dBm = 52,
        /// <summary>-2.8 dB</summary>
        Power_Minus2Point8dBm = 51,
        /// <summary>-3.1 dB</summary>
        Power_Minus3Point1dBm = 50,
        /// <summary>-3.4 dB</summary>
        Power_Minus3Point4dBm = 49,
        /// <summary>-3.7 dB</summary>
        Power_Minus3Point7dBm = 48,
        /// <summary>-4.0 dB</summary>
        Power_Minus4Point0dBm = 47,
        /// <summary>-4.3 dB</summary>
        Power_Minus4Point3dBm = 46,
        /// <summary>-4.6 dB</summary>
        Power_Minus4Point6dBm = 45,
        /// <summary>-4.9 dB</summary>
        Power_Minus4Point9dBm = 44,
        /// <summary>-5.2 dB</summary>
        Power_Minus5Point2dBm = 43,
        /// <summary>-5.5 dB</summary>
        Power_Minus5Point5dBm = 42,
        /// <summary>-5.8 dB</summary>
        Power_Minus5Point8dBm = 41,
        /// <summary>-6.1 dB</summary>
        Power_Minus6Point1dBm = 40,
        /// <summary>-6.4 dB</summary>
        Power_Minus6Point4dBm = 39,
        /// <summary>-6.7 dB</summary>
        Power_Minus6Point7dBm = 38,
        /// <summary>-7.0 dB</summary>
        Power_Minus7Point0dBm = 37,
        /// <summary>-7.3 dB</summary>
        Power_Minus7Point3dBm = 36,
        /// <summary>-7.6 dB</summary>
        Power_Minus7Point6dBm = 35,
        /// <summary>-7.9 dB</summary>
        Power_Minus7Point9dBm = 34,
        /// <summary>-8.2 dB</summary>
        Power_Minus8Point2dBm = 33,
        /// <summary>-8.5 dB</summary>
        Power_Minus8Point5dBm = 32,
        /// <summary>-8.8 dB</summary>
        Power_Minus8Point8dBm = 31,
        /// <summary>-9.1 dB</summary>
        Power_Minus9Point1dBm = 30,
        /// <summary>-9.4 dB</summary>
        Power_Minus9Point4dBm = 29,
        /// <summary>-9.7 dB</summary>
        Power_Minus9Point7dBm = 28,
        /// <summary>-10.0 dB</summary>
        Power_Minus10Point0dBm = 27,
        /// <summary>-10.3 dB</summary>
        Power_Minus10Point3dBm = 26,
        /// <summary>-10.6 dB</summary>
        Power_Minus10Point6dBm = 25,
        /// <summary>-10.9 dB</summary>
        Power_Minus10Point9dBm = 24,
        /// <summary>-11.2 dB</summary>
        Power_Minus11Point2dBm = 23,
        /// <summary>-11.5 dB</summary>
        Power_Minus11Point5dBm = 22,
        /// <summary>-11.8 dB</summary>
        Power_Minus11Point8dBm = 21,
        /// <summary>-12.1 dB</summary>
        Power_Minus12Point1dBm = 20,
        /// <summary>-12.4 dB</summary>
        Power_Minus12Point4dBm = 19,
        /// <summary>-12.7 dB</summary>
        Power_Minus12Point7dBm = 18,
        /// <summary>-13.0 dB</summary>
        Power_Minus13Point0dBm = 17,
        /// <summary>-13.4 dB</summary>
        Power_Minus13Point4dBm = 16,
        /// <summary>-13.8 dB</summary>
        Power_Minus13Point8dBm = 15,
        /// <summary>-14.2 dB</summary>
        Power_Minus14Point2dBm = 14,
        /// <summary>-14.6 dB</summary>
        Power_Minus14Point6dBm = 13,
        /// <summary>-15.0 dB</summary>
        Power_Minus15Point0dBm = 12,
        /// <summary>-15.4 dB</summary>
        Power_Minus15Point4dBm = 11,
        /// <summary>-15.8 dB</summary>
        Power_Minus15Point8dBm = 10,
        /// <summary>-16.2 dB</summary>
        Power_Minus16Point2dBm = 9,
        /// <summary>-16.6 dB</summary>
        Power_Minus16Point6dBm = 8,
        /// <summary>-17.0 dB</summary>
        Power_Minus17Point0dBm = 7,
        /// <summary>-17.4 dB</summary>
        Power_Minus17Point4dBm = 6,
        /// <summary>-17.8 dB</summary>
        Power_Minus17Point8dBm = 5,
        /// <summary>-18.2 dB</summary>
        Power_Minus18Point2dBm = 4,
        /// <summary>-18.6 dB</summary>
        Power_Minus18Point6dBm = 3,
        /// <summary>-19.0 dB</summary>
        Power_Minus19Point0dBm = 2,
        /// <summary>-19.4 dB</summary>
        Power_Minus19Point4dBm = 1,
        /// <summary>-19.7 dB</summary>
        Power_Minus19Point7dBm = 0,
	}

	/// <summary>
	/// Channels supported by RF231 radio
	/// </summary>
	public enum RF231Channel
	{
		/// <summary>Channel 11 Frequency 2405 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_11 = 0,

		/// <summary>Channel 12 Frequency 2410 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_12 = 1,

		/// <summary>Channel 13 Frequency 2415 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_13 = 2,

		/// <summary>Channel 14 Frequency 2420 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_14 = 3,

		/// <summary>Channel 15 Frequency 2425 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_15 = 4,

		/// <summary>Channel 16 Frequency 2430 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_16 = 5,

		/// <summary>Channel 17 Frequency 2435 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_17 = 6,

		/// <summary>Channel 18 Frequency 2440 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_18 = 7,

		/// <summary>Channel 19 Frequency 2445 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_19 = 8,

		/// <summary>Channel 20 Frequency 2450 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_20 = 9,

		/// <summary>Channel 21 Frequency 2455 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_21 = 10,

		/// <summary>Channel 22 Frequency 2460 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_22 = 11,

		/// <summary>Channel 23 Frequency 2465 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_23 = 12,

		/// <summary>Channel 24 Frequency 2470 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_24 = 13,

		/// <summary>Channel 25 Frequency 2475 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_25 = 14,

		/// <summary>Channel 26 Frequency 2480 MHz</summary>
		// ReSharper disable once InconsistentNaming
		Channel_26 = 15,
	}

	/// <summary>
	/// Channels supported by SI4468 radio
	/// </summary>
	public enum SI4468Channel
	{
        /// <summary>Channel 0</summary>
        Channel_00 = 0,
        /// <summary>Channel 1</summary>
        Channel_01 = 1,
        /// <summary>Channel 2</summary>
        Channel_02 = 2,
        /// <summary>Channel 3</summary>
        Channel_03 = 3,
        /// <summary>Channel 4</summary>
        Channel_04 = 4,
        /// <summary>Channel 5</summary>
        Channel_05 = 5,
        /// <summary>Channel 6</summary>
        Channel_06 = 6,
        /// <summary>Channel 7</summary>
        Channel_07 = 7,
        /// <summary>Channel 8</summary>
        Channel_08 = 8,
        /// <summary>Channel 9</summary>
        Channel_09 = 9,
        /// <summary>Channel 10</summary>
        Channel_10 = 10,
        /// <summary>Channel 11</summary>
        Channel_11 = 11,
        /// <summary>Channel 12</summary>
        Channel_12 = 12,
        /// <summary>Channel 13</summary>
        Channel_13 = 13,
        /// <summary>Channel 14</summary>
        Channel_14 = 14,
        /// <summary>Channel 15</summary>
        Channel_15 = 15,
		/// <summary>Channel 16</summary>
		// ReSharper disable once InconsistentNaming
		Channel_16 = 16,
		/// <summary>Channel 17</summary>
		// ReSharper disable once InconsistentNaming
		Channel_17 = 17
	}

	/// <summary>
	/// Radio configuration interface
	/// </summary>
	public interface IRadioConfiguration
	{
		/// <summary>Transmission power</summary>
		int TxPower { get; set; }

		/// <summary>Channel</summary>
		int Channel { get; set; }

		/// <summary>Radio name</summary>
		RadioName RadioName { get; set; }
	}

	// Changed power value and channel to int (since enums cannot be inherited) - Bill

	/// <summary>
	/// RF231 Radio configuration
	/// </summary>
	public class RF231RadioConfiguration : IRadioConfiguration
	{
		/// <summary>Transmission power of the radio</summary>
		public int TxPower { get; set; }

		/// <summary>Channel the radio will transmit on</summary>
		public int Channel { get; set; }

		/// <summary>Type of radio</summary>
		public RadioName RadioName { get; set; }

		/// <summary>Radio configuration constructor</summary>
		/// <param name="power">Transmission power</param>
		/// <param name="channel">Channel</param>
		public RF231RadioConfiguration(RF231TxPower power = RF231TxPower.Power_3dBm,
			RF231Channel channel = RF231Channel.Channel_26)
		{
			RadioName = RadioName.RF231;
			TxPower = (int)power;
			Channel = (int)channel;
		}
	}

	/// <summary>
	/// RF231 Radio configuration
	/// </summary>
	public class RF231LRRadioConfiguration : IRadioConfiguration
	{
		/// <summary>Transmission power of the radio</summary>
		public int TxPower { get; set; }

		/// <summary>Channel the radio will transmit on</summary>
		public int Channel { get; set; }

		/// <summary>Type of radio</summary>
		public RadioName RadioName { get; set; }

		/// <summary>Radio configuration constructor</summary>
		/// <param name="power">Transmission power</param>
		/// <param name="channel">Channel</param>
		public RF231LRRadioConfiguration(RF231TxPower power = RF231TxPower.Power_3dBm, RF231Channel channel= RF231Channel.Channel_26)
		{
			RadioName = RadioName.RF231LR;
			TxPower = (int)power;
			Channel = (int)channel;
		}
	}

	/// <summary>SI4468 Radio configuration</summary>
	public class SI4468RadioConfiguration : IRadioConfiguration
	{
		/// <summary>Transmission power of the radio</summary>
		public int TxPower { get; set; }

		/// <summary>Channel the radio will transmit on</summary>
		public int Channel { get; set; }

		/// <summary>Type of radio</summary>
		public RadioName RadioName { get; set; }

		/// <summary>Radio configuration constructor</summary>
		/// <param name="power">Transmission power</param>
		/// <param name="channel">Channel</param>
		public SI4468RadioConfiguration(SI4468TxPower power = SI4468TxPower.Power_20dBm, SI4468Channel channel = SI4468Channel.Channel_02)
		{
			RadioName = RadioName.SI4468;
			TxPower = (int)power;
			Channel = (int)channel;
		}
	}

	/// <summary>
	/// Radio interface
	/// </summary>
	public interface IRadio
	{
		//DeviceStatus Initialize(RadioConfiguration config, ReceiveCallBack callback); //Initializes Return the ID of the Radio layer that was initialized
		//DeviceStatus Configure(RadioConfiguration config, ReceiveCallBack callback);  //Change configuration after initialization

		/*/// <summary>
		/// Unitialize radio
		/// </summary>
        /// <param name="radioName">Radio name</param>
		/// <returns>Success of operation</returns>
		DeviceStatus UnInitialize(byte radioName);*/

		/// <summary>
		/// Turn radio on
		/// </summary>
		/// <param name="radioName">Radio name</param>
		/// <returns>Success of operation</returns>
		DeviceStatus TurnOnRx(byte radioName);

		/// <summary>
		/// Put radio to sleep
		/// </summary>
		/// <param name="radioName">Radio name</param>
		/// <param name="level">Sleep level</param>
		/// <returns>Success of operation</returns>
		DeviceStatus Sleep(byte radioName, byte level);

		/// <summary>
		/// Preload radio
		/// </summary>
		/// <param name="packet">Packet to preload</param>
		/// <param name="size">Size of packet</param>
		/// <returns>Success of operation</returns>
		NetOpStatus PreLoad(byte[] packet, ushort size);

		/// <summary>
		/// Send preloaded packet
		/// </summary>
		/// <returns>Success of operation</returns>
		NetOpStatus SendStrobe(byte radioName, ushort size);

		/// <summary>
		/// Send packet on radio
		/// </summary>
		/// <param name="radioName">Radio name</param>
		/// <param name="packet">Packet to send</param>
		/// <param name="size">Size of packet</param>
		/// <returns>Success of operation</returns>
		NetOpStatus Send(byte radioName, byte[] packet, ushort size);

		/// <summary>
		/// Sent time-stamped packet
		/// </summary>
		/// <param name="radioName">Radio name</param>
		/// <param name="packet">Packet buffer to send</param>
		/// <param name="size">Size of packet</param>
		/// <param name="eventTime">Time stamp of packet</param>
		/// <returns>Send status</returns>
		NetOpStatus SendTimeStamped(byte radioName, byte[] packet, ushort size, uint eventTime);

		/// <summary>
		/// Check if channel is clear
		/// </summary>
		/// <param name="radioName">Radio name</param>
		/// <returns>True iff clear</returns>
		bool ClearChannelAssesment(byte radioName);

		/// <summary>
		/// Check if channel has been clear for the specified interval of time
		/// </summary>
		/// <param name="radioName">Radio name</param>
		/// <param name="numberOfMicroSecond">Interval (microseconds)</param>
		/// <returns>True iff clear</returns>
		bool ClearChannelAssesment(byte radioName, ushort numberOfMicroSecond);
	}
}
