using System;
using Microsoft.SPOT;

namespace Samraksh.eMote.ProgramManager
{
    //using UpdateID = UInt32;

    /// <summary>
    /// correspond to protocol messages.
    /// </summary>
    public enum UpdateMessage
    {
        NOMSG = 0,
        START = 1, START_ACK = 2,
        AUTHCMD = 3, AUTHCMD_ACK = 4,
        AUTHENTICATE = 5, AUTHETICATE_ACK = 6,
        GETMISSINGPACKETS = 7, GETMISSINGPACKETS_ACK = 8,
        ADDPACKET = 9, ADDPACKET_ACK = 10,
        INSTALL = 11, INSTALL_ACK = 12,
    };

    /// <summary>
    /// TODO: add first known missing packet.
    /// TODO: add total packets.
    /// </summary>
    public class UpdateState
    {
        /// <summary>
        /// unique ID of the update. Currently a CRC of the update binary. should not be 0.
        /// </summary>
        public UInt32 updateID;

        /// <summary>
        /// MAC ID of where the update should be installed.
        /// </summary>
        public UInt16 destAddr;

        public UpdateMessage lastSentMsg;

        public UpdateMessage lastReceivedMsg;

        /// <summary>
        /// amount of binary data transmitted with each packet.
        /// this is set ahead-of-time so the local and remote nodes may communicate missing data chunks.
        /// </summary>
        public UInt32 packetDataSize;

        public UInt32 missingPacketCount;

        public DateTime latestActionDate;

        public UpdateState() {
            updateID = 0;
            destAddr = 0;
            lastSentMsg = UpdateMessage.NOMSG;
            lastReceivedMsg = UpdateMessage.NOMSG;
            packetDataSize = 50;
            missingPacketCount = 0;
            latestActionDate = System.DateTime.MinValue;
        }

        public UpdateState(UInt32 updateID, UInt16 destAddr, UpdateMessage lastSent, UpdateMessage lastReceived, UInt32 packetDataSize, UInt32 missingPacketCount, DateTime latestActionDate)
        {
            this.updateID = updateID;
            this.destAddr = destAddr;
            this.lastSentMsg = lastSent;
            this.lastReceivedMsg = lastReceived;
            this.packetDataSize = packetDataSize;
            this.missingPacketCount = missingPacketCount;
            this.latestActionDate = latestActionDate;
        }
    }
}
