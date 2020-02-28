using System;
using Microsoft.SPOT;

namespace Samraksh_Mel
{
    /// <summary>
	/// To be kept in sync with that in DeviceCode/Include/Samraksh/Message.h
	/// Values go from 1 to 31 (lower 5 bits - 0x00 to 0x1F) and possibly 0xFF (255) are for user applications.
	/// </summary>
	public enum PayloadType
    {
        /// <summary>Payload type 0</summary>
        Type00 = 0x00,
        /// <summary>Payload type 1</summary>
        Type01 = 0x01,
        /// <summary>Payload type 2</summary>
        Type02 = 0x02,
        /// <summary>Payload type 3</summary>
        Type03 = 0x03,
        /// <summary>Payload type 4</summary>
        Type04 = 0x04,
        /// <summary>Payload type 5</summary>
        Type05 = 0x05,
        /// <summary>Payload type 6</summary>
        Type06 = 0x06,
        /// <summary>Payload type 7</summary>
        Type07 = 0x07,
        /// <summary>Payload type 8</summary>
        Type08 = 0x08,
        /// <summary>Payload type 9</summary>
        Type09 = 0x09,
        /// <summary>Payload type 10</summary>
        Type10 = 0x0A,
        /// <summary>Payload type 11</summary>
        Type11 = 0x0B,
        /// <summary>Payload type 12</summary>
        Type12 = 0x0C,
        /// <summary>Payload type 13</summary>
        Type13 = 0x0D,
        /// <summary>Payload type 14</summary>
        Type14 = 0x0E,
        /// <summary>Payload type 15</summary>
        Type15 = 0x0F,
        /// <summary>Payload type 16</summary>
        Type16 = 0x10,
        /// <summary>Payload type 17</summary>
        Type17 = 0x11,
        /// <summary>Payload type 18</summary>
        Type18 = 0x12,
        /// <summary>Payload type 19</summary>
        Type19 = 0x13,
        /// <summary>Payload type 20</summary>
        Type20 = 0x14,
        /// <summary>Payload type 21</summary>
        Type21 = 0x15,
        /// <summary>Payload type 22</summary>
        Type22 = 0x16,
        /// <summary>Payload type 23</summary>
        Type23 = 0x17,
        /// <summary>Payload type 24</summary>
        Type24 = 0x18,
        /// <summary>Payload type 25</summary>
        Type25 = 0x19,
        /// <summary>Payload type 26</summary>
        Type26 = 0x1A,
        /// <summary>Payload type 27</summary>
        Type27 = 0x1B,
        /// <summary>Payload type 28</summary>
        Type28 = 0x1C,
        /// <summary>Payload type 29</summary>
        Type29 = 0x1D,
        /// <summary>Payload type 30</summary>
        Type30 = 0x1E,
        /// <summary>Payload type 31</summary>
        Type31 = 0x1F,
        /// <summary>Micro Framework Message_Data</summary>
        // ReSharper disable once InconsistentNaming
        MFM_Data = 0x20
    };

    /// <summary>
    /// 
    /// </summary>
    internal enum PayloadTypeMAC
    {
        /// <summary>CSMA time sync request</summary>
        // ReSharper disable once InconsistentNaming
        MFM_CSMA_TimeSyncReq = 0x21,

        /// <summary>CSMA Neighborhood</summary>
        // ReSharper disable once InconsistentNaming
        MFM_CSMA_Neighborhood = 0x22,

        /// <summary>CSMA Routing</summary>
        // ReSharper disable once InconsistentNaming
        MFM_CSMA_Routing = 0x23,

        /// <summary>CSMA Discovery</summary>
        // ReSharper disable once InconsistentNaming
        MFM_CSMA_Discovery = 0x24,

        /// <summary>Time sync</summary>
        // ReSharper disable once InconsistentNaming
        MFM_TimeSync = 0x25,

        /// <summary>CSMA data acknowledgement</summary>
        // ReSharper disable once InconsistentNaming
        MFM_CSMA_Data_Ack = 0x26,

        /// <summary>OMAC time sync request</summary>
        // ReSharper disable once InconsistentNaming
        MFM_OMAC_TimeSyncReq = 0x31,

        /// <summary>OMAC neighborhood</summary>
        // ReSharper disable once InconsistentNaming
        MFM_OMAC_Neighborhood = 0x32,

        /// <summary>OMAC routing</summary>
        // ReSharper disable once InconsistentNaming
        MFM_OMAC_Routing = 0x33,

        /// <summary>OMAC discovery</summary>
        // ReSharper disable once InconsistentNaming
        MFM_OMAC_Discovery = 0x34,

        /// <summary>OMAC data ack</summary>
        // ReSharper disable once InconsistentNaming
        MFM_OMAC_Data_Ack = 0x36,

        /// <summary>OMAC data beacon type</summary>
        // ReSharper disable once InconsistentNaming
        MFM_OMAC_Data_Beacon_Type = 0x37
    };

    /// <summary>Packet object. Passed to native.</summary>
    public class Packet
    {
        /// <summary>
        /// The default size of the mac packet
        /// </summary>
        const byte MACPacketSize = 128;

        /// <summary>Received Signal Strength (rssi) of packet</summary>
        public byte RSSI;

        /// <summary>Link Quality Indication measured during the packet reception</summary>
        // ReSharper disable once InconsistentNaming
        public byte LQI;

        /// <summary>Source of the packet transmitted</summary>
        public ushort Src;

        /// <summary>True iff packet was unicast (else false)</summary>
        public bool IsUnicast;

        /// <summary>Received packet</summary>
        public byte[] Payload;

        /// <summary>Type of payload</summary>
        public PayloadType PayloadType;

        /// <summary>size of the packet payload</summary>
        public ushort Size;

        /// <summary>The time at which the packet was sent out (microseconds)</summary>
        public DateTime SenderEventTimeStamp;

        /// <summary>True iff packet is timestamped</summary>
        public bool IsPacketTimeStamped;

        /// <summary>Create a packet with the default size</summary>
        public Packet()
        {
            Payload = new byte[MACPacketSize];
        }

        /// <summary>Configure size of payload</summary>
        /// <param name="size">Size of payload</param>
        public Packet(int size)
        {
            Payload = new byte[size];
        }

        /// <summary>Create a packet with size, Payload, rssi, lqi, src and Unicast information specified in packet array</summary>
        /// <param name="msg">Packet. size, Payload, PayloadType, rssi, lqi, src and Unicast information specified in the first 6 bytes. Rest is payload</param>
        internal Packet(byte[] msg)
        {
            ushort i;
            ushort length = msg[0];
            length |= (ushort)(msg[1] << 8);

            Size = length;

            Payload = new byte[Size];

            Array.Copy(msg, 2, Payload, 0, length);

            PayloadType = (PayloadType)msg[length + 2];    //2 is for the first 2 bytes msg[0] and msg[1]
            RSSI = msg[length + 3];
            LQI = msg[length + 4];

            Src = msg[length + 5];
            Src |= (ushort)(msg[length + 6] << 8);

            // Determines whether the packet is unicast or not 
            if (msg[length + 7] == 1)
                IsUnicast = true;
            else
                IsUnicast = false;

            // Check if the packet is timestamped from the sender 
            if (msg[length + 8] == 1)
                IsPacketTimeStamped = true;
            else
                IsPacketTimeStamped = false;

            // Elaborate conversion plan because nothing else works 
            uint lsbItem = msg[length + 9];
            lsbItem |= ((uint)msg[length + 10] << 8);
            lsbItem |= ((uint)msg[length + 11] << 16);
            lsbItem |= ((uint)msg[length + 12] << 24);

            uint msbItem = msg[length + 13];
            msbItem |= ((uint)msg[length + 14] << 8);
            msbItem |= ((uint)msg[length + 15] << 16);
            msbItem |= ((uint)msg[length + 16] << 24);


            var tempTimeStamp = ((long)msbItem << 32) | lsbItem;

            SenderEventTimeStamp = new DateTime(tempTimeStamp);
        }

        /// <summary>Create a packet with specified parameters</summary>
        /// <param name="payload">Packet payload</param>
        /// <param name="payloadType">Packet payload type</param>
        /// <param name="src">Source of the packet</param>
        /// <param name="isUnicast">Was transmission unicast</param>
        /// <param name="rssi">rssi</param>
        /// <param name="lqi">lqi</param>
        internal Packet(byte[] payload, ushort src, PayloadType payloadType, bool isUnicast, byte rssi, byte lqi)
        {
            //Create a payload object of default size
            Payload = new byte[MACPacketSize];

            // Copy the payload to the receive packet buffer
            Array.Copy(payload, 0, Payload, 0, payload.Length);

            // Copy other parameters to this object 
            Src = src;
            PayloadType = payloadType;
            IsUnicast = isUnicast;
            RSSI = rssi;
            LQI = lqi;
        }

        /// <summary>Create a packet with specified parameters</summary>
        /// <param name="payload">Packet payload</param>
        /// <param name="payloadType">Packet payload type</param>
        /// <param name="src">Source of the packet</param>
        /// <param name="isUnicast">Was transmission unicast?</param>
        /// <param name="rssi">RSSI value</param>
        /// <param name="lqi">LQI value</param>
        /// <param name="size">size of the payload buffer</param>
        internal Packet(byte[] payload, ushort src, PayloadType payloadType, bool isUnicast, byte rssi, byte lqi, ushort size)
        {
            //Create a packet object of default size
            Payload = new byte[size];

            // Copy the packet to the receive message buffer 
            Array.Copy(payload, 0, Payload, 0, payload.Length);

            // Copy other parameters to this object 
            Src = src;
            PayloadType = payloadType;
            IsUnicast = isUnicast;
            RSSI = rssi;
            LQI = lqi;
        }

        /// <summary>Create a packet with specified parameters</summary>
        /// <param name="payload">Packet payload</param>
        /// <param name="payloadType">Packet payload type</param>
        /// <param name="src">Source of the packet</param>
        /// <param name="isUnicast">Was transmission unicast?</param>
        /// <param name="rssi">RSSI value</param>
        /// <param name="lqi">LQI value</param>
        /// <param name="size">size of the payload buffer</param>
        /// <param name="isPacketTimeStamped">Is packet timestamped?</param>
        internal Packet(byte[] payload, ushort src, PayloadType payloadType, bool isUnicast, byte rssi, byte lqi, ushort size, bool isPacketTimeStamped)
        {
            //Create a payload object of default size
            Payload = new byte[size];

            // Copy the payload to the receive message buffer 
            Array.Copy(payload, 0, Payload, 0, payload.Length);

            // Copy other parameters to this object 
            Src = src;
            PayloadType = payloadType;
            IsUnicast = isUnicast;
            RSSI = rssi;
            LQI = lqi;
            IsPacketTimeStamped = isPacketTimeStamped;
        }

    }
}
