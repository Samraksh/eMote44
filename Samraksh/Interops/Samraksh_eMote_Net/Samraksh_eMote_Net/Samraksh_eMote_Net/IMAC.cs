using System;
using Samraksh.eMote.Net.Radio;

namespace Samraksh.eMote.Net
{
    namespace MAC
    {
        /// <summary>
        /// MAC address type
        /// </summary>
        /// <remarks>
        /// Only used with CSMA
        /// </remarks>
        public enum AddressType
        {
            /// <summary>
            /// Indicates destination is all nodes in range. 
            /// <para>Other values indicate a particular node.</para>
            /// </summary>
            Broadcast = 65535,
        }

        /// <summary>
        /// Link quality
        /// </summary>
        public class Link
        {

            /// <summary> Average Received Signal Strength Indication, RSSI.   </summary>
            /// <remarks> 
            /// Exponentially smoothed (with lambda of 0.2) average of   SINR code as reported by the radio.
            ///	        For RF231 has conversion (-91 + AvgRSSI) provides the true value.
            /// 		For SI radio the conversion is already done at the driver and is cast into uint.
            ///	  	  Hence the conversion to get signed value is -1*( (0xFF - AvgRSSI)+1 ).
            /// </remarks>
            public byte AverageRSSI;


            /// <summary>Link quality</summary>             
            /// <remarks> 
            /// As reported by the radio.
            /// For RF231, represent a measure of the BER for the corresponding SINR.
            /// For SI radio, not available. Hence a value of 0 is reported.
            /// Exponentially smoothed with lambda of 0.2.
            /// </remarks>
            public byte LinkQuality;

            /// <summary>Average delay</summary>
            /// <remarks> 
            /// Has units of 65.535 (=10^3/(2^19/2^8)) ms. Exponentially smoothed
            /// </remarks>
            public byte AverageDelay;

            /// <summary> Charecteristics of a link. </summary>
            public Link()
            {
                AverageRSSI = 0;
                LinkQuality = 0;
                AverageDelay = 0;
            }
        }

        /// <summary>
        /// Neighbor status
        /// </summary>
        public enum NeighborStatus
        {
            /// <summary>Neighbor is alive</summary>
            Alive,
            /// <summary>Neighbor is dead</summary>
            Dead,
            /// <summary>Neighbor is suspect</summary>
            Suspect
        }

        /// <summary>
        /// Neighbor details
        /// </summary>
        public class Neighbor
        {
            /// <summary>MAC address of neighbor</summary>
            public ushort MACAddress;
            /// <summary>Send (formerly forward) link details between current and neighbor node. Provides link details 
            /// of transmissions to a certain neighbor.  
            /// </summary>
            #region commented code Removed by Bora.This is not implemented for the current MAC algorithms. 
            /// Since a sender will not have the link details when sending a packet, the receiver
            /// has to send back the details (such as RSSI, LQI) to the sender (by piggybacking on a data packet for instance).
            #endregion
            public Link SendLink;
            /// <summary>Receive (formerly reverse) link details between current and neighbor node. Provides link details 
            /// of receptions from a certain neighbor. A received packet's metadata carries details such as RSSI and LQI.</summary>
			public Link ReceiveLink;
            /// <summary>Status of neighbor</summary>
            public NeighborStatus NeighborStatus;
            /// <summary>IsAvailableForUpperLayers</summary>
            public bool IsAvailableForUpperLayers;
            /// <summary>NumTimeSyncMessagesSent</summary>
            public ushort NumTimeSyncMessagesSent;
            /// <summary>NumTimeSyncMessagesRecv</summary>
            public ushort NumOfTimeSamplesRecorded;
            //		/// <summary>Packet receive count</summary>
            //		public ushort CountOfPacketsReceived;

            /// <summary>Last time heard from neighbor</summary>
            public ulong LastHeardTime;
            /// <summary>Receive duty cycle of neighbor</summary>
            public byte ReceiveDutyCycle; //percentage
                                          /// <summary>Frame length of neighbor</summary>
            public ushort FrameLength;

            /// <summary> Constructs a dummy neighbor object </summary>
            public Neighbor()
            {
                MACAddress = 0;
                SendLink = new Link();
                ReceiveLink = new Link();
                NeighborStatus = NeighborStatus.Dead;
                IsAvailableForUpperLayers = false;
                NumTimeSyncMessagesSent = 0;
                NumOfTimeSamplesRecorded = 0;
                LastHeardTime = 0;
                ReceiveDutyCycle = 0;
                FrameLength = 0;
            }
        }


        /// <summary>
        /// MAC interface
        /// </summary>
        public interface IMAC
        {
            // several below added by Bill to force MACPipe to conform to signature of MACBase (so that there can be a one-line change to use MACPipe instead of MACBase)

            /// <summary>The type of MAC (OMAC, CSMA)</summary>
            MACType MACType { get; }

            /// <summary>The radio object the MAC is using</summary>
            Radio_802_15_4_Base MACRadioObj { get; }

            /// <summary>Raised when a packet has been received</summary>
            event MACBase.IMACReceiveEventHandler OnReceive;

            /// <summary>Raised when neighborhood changes</summary>
            event MACBase.IMACNeighborChangeEventHandler OnNeighborChange;

            /// <summary>Raised when we get an ACK for a transmitted packet</summary>
            event MACBase.IMACTransmitACKEventHandler OnSendStatus;

            /// <summary>True iff MAC CCA (clear channel assessment) is enabled</summary>
            bool CCA { get; set; }

            /// <summary>Number of times to try sending before MAC gives up</summary>
            byte NumberOfRetries { get; set; }

            /// <summary>Amount of time (in milliseconds) to assess whether channel is clear (CCA)</summary>
            byte CCASenseTime { get; set; }

            /// <summary>Size of send buffer</summary>
            byte BufferSize { get; set; }

            /// <summary>Delay (in milliseconds) before a Neighbor is deemed dead</summary>
            uint NeighborLivenessDelay { get; set; }

            /*/// <summary>
			/// Get the next packet from the MAC buffer
			/// </summary>
			/// <returns>Next packet if any, else null</returns>
			bool NextPacket(Packet receivedPacket);*/

            /// <summary>
            ///		Get the list of neighbors from the MAC
            /// </summary>
            /// <param name="neighborListArray">
            ///		Array is filled with the addresses of active neighbors, padded with zeroes at the end.
            /// </param>
            /// <returns>
            ///		Result status
            /// </returns>
            DeviceStatus NeighborList(ushort[] neighborListArray);

            /// <summary>Send packet</summary>
            /// <param name="address">Address of recipient</param>
            /// <param name="payload">Payload (in byte array) to send</param>
            /// <param name="offset">Offset into array</param>
            /// <param name="size">Size of payload</param>
            /// <returns>Result status</returns>
            NetOpStatus Send(ushort address, byte[] payload, ushort offset, ushort size);

            /// <summary>Send packet with time value</summary>
            /// <param name="address">Address of recipient</param>
            /// <param name="payload">Payload (in byte array) to send</param>
            /// <param name="offset">Offset into array</param>
            /// <param name="size">Size of message</param>
            /// <param name="eventTime">Time value to add to packet</param>
            /// <returns>Result status</returns>
            NetOpStatus Send(ushort address, byte[] payload, ushort offset, ushort size, DateTime eventTime);

            /*/// <summary>Send packet with payload type</summary>
			/// <param name="address">Address of recipient</param>
			/// <param name="payloadType">Payload type of packet</param>
			/// <param name="payload">Payload (in byte array) to send</param>
			/// <param name="offset">Offset into array</param>
			/// <param name="size">Size of payload</param>
			/// <returns>Result status</returns>
			NetOpStatus Send(ushort address, PayloadType payloadType, byte[] payload, ushort offset, ushort size);

			/// <summary>Send packet with payload type and time value</summary>
			/// <param name="address">Address of recipient</param>
			/// <param name="payloadType">Payload type of packet</param>
			/// <param name="payload">Payload (in byte array) to send</param>
			/// <param name="offset">Offset into array</param>
			/// <param name="size">Size of payload</param>
			/// <param name="eventTime">Time value to add to packet</param>
			/// <returns>Result status</returns>
			NetOpStatus Send(ushort address, PayloadType payloadType, byte[] payload, ushort offset, ushort size, DateTime eventTime);*/

            //Neighbor methods

            /// <summary>
            /// Get neighbor status
            /// </summary>
            /// <param name="macAddress">MAC address of neighbor</param>
            /// <returns>Neighbor status</returns>
            Neighbor NeighborStatus(ushort macAddress);

            /// <summary>
            /// Get pending packet count of MAC instance
            /// </summary>
            /// <returns></returns>
            byte PendingSendPacketCount();

            /// <summary>
            /// Get pending packet count of MAC instance
            /// </summary>
            /// <returns></returns>
            byte PendingReceivePacketCount();

            /// <summary>
            /// Remove packet from pending
            /// </summary>
            /// <returns>Status of result</returns>
            DeviceStatus RemovePacket();
        }
    }
}