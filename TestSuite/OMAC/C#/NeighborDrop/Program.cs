#define RF231
//#define SI4468


using System;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using System.Threading;
using System.Collections;

using Samraksh.eMote.Net;
using Samraksh.eMote.Net.MAC;
using Samraksh.eMote.Net.Radio;
using Samraksh.eMote.DotNow;

//1. This program initializes OMAC as the MAC protocol.
//  1a. Registers a function that tracks change in neighbor (NeighborChange) and a function to handle messages that are received.
//2. Pings are sent at pre-determined intervals.
//3. Pongs are sent back for received messages.
namespace Samraksh.eMote.Net.Mac.Receive
{
    public class NeighborTableInfo
    {
        public UInt32 recvCount;
        public UInt32 prevId;
        public ArrayList AL;
    }

    public class PingPayload
    {
        public UInt32 pingMsgId;
        public string pingMsgContent = "PING";

        public PingPayload()
        {

        }

        public byte[] ToBytes()
        {
            byte[] msg = new byte[4];
            msg[0] = (byte)((pingMsgId >> 24) & 0xFF);
            msg[1] = (byte)((pingMsgId >> 16) & 0xFF);
            msg[2] = (byte)((pingMsgId >> 8) & 0xFF);
            msg[3] = (byte)((pingMsgId) & 0xFF);

            //Convert string to byte array
            byte[] msgContent = System.Text.Encoding.UTF8.GetBytes(pingMsgContent);

            //Merge array containing msgID and array containing string into a single byte array for transmission
            byte[] merged = new byte[msg.Length + msgContent.Length];
            msg.CopyTo(merged, 0);
            msgContent.CopyTo(merged, msg.Length);

            return merged;
        }

        public PingPayload FromBytesToPingPayload(byte[] msg)
        {
            try
            {
                PingPayload pingPayload = new PingPayload();

                //Convert byte array to an integer
                pingPayload.pingMsgId = (UInt32)(msg[0] << 24);
                pingPayload.pingMsgId += (UInt32)(msg[1] << 16);
                pingPayload.pingMsgId += (UInt32)(msg[2] << 8);
                pingPayload.pingMsgId += (UInt32)(msg[3]);

                //Create a byte array to store the string
                byte[] msgContent = new byte[4];
                msgContent[0] = msg[4];
                msgContent[1] = msg[5];
                msgContent[2] = msg[6];
                msgContent[3] = msg[7];

                //Convert byte to char array
                char[] msgContentChar = System.Text.Encoding.UTF8.GetChars(msgContent);

                //Convert char array to string
                pingPayload.pingMsgContent = new string(msgContentChar);

                return pingPayload;
            }
            catch (Exception ex)
            {
                Debug.Print(ex.ToString());
                Debug.Print(((UInt32)(msg[0] << 24)).ToString());
                Debug.Print(((UInt32)(msg[1] << 16)).ToString());
                Debug.Print(((UInt32)(msg[2] << 8)).ToString());
                Debug.Print(((UInt32)(msg[3])).ToString());
                Debug.Print(msg[0].ToString());
                Debug.Print(msg[1].ToString());
                Debug.Print(msg[2].ToString());
                Debug.Print(msg[3].ToString());
                Debug.Print(msg[4].ToString());
                Debug.Print(msg[5].ToString());
                Debug.Print(msg[6].ToString());
                Debug.Print(msg[7].ToString());
                return null;
            }
        }
    }

    public class Program
    {
        //const UInt16 MAX_NEIGHBORS = 12;
        const UInt32 endOfTest = 50;
        Hashtable neighborHashtable = new Hashtable();
        EmoteLCD lcd;

        static bool hitTwoNeighbors = false;
        static bool hitZeroNeighbors = false;

        UInt16 myAddress;
        static UInt32 totalRecvCounter = 0;

        PingPayload pingMsg = new PingPayload();
        OMAC myOMACObj;

        int errors = 0;


        public void Initialize()
        {
            //Init LCD
            lcd = new EmoteLCD();
            lcd.Initialize();
            lcd.Write(LCD.CHAR_I, LCD.CHAR_n, LCD.CHAR_i, LCD.CHAR_t);

            try
            {
                Debug.Print("Initializing radio");
#if RF231
                var radioConfig = new RF231RadioConfiguration(RF231TxPower.Power_0Point0dBm, RF231Channel.Channel_13);
#elif SI4468
                var radioConfig = new SI4468RadioConfiguration(SI4468TxPower.Power_15Point5dBm, SI4468Channel.Channel_01);
#endif

                //configure OMAC
                myOMACObj = new OMAC(radioConfig);
                myOMACObj.OnReceive += Receive;
                myOMACObj.OnNeighborChange += NeighborChange;
				myOMACObj.OnSendStatus += SendStatus;
				myOMACObj.NeighborLivenessDelay = 60;

                myAddress = myOMACObj.MACRadioObj.RadioAddress;

                var chan1 = new MACPipe(myOMACObj, PayloadType.Type01);
                chan1.OnReceive += Receive;

                var chan2 = new MACPipe(myOMACObj, PayloadType.Type02);
                chan2.OnReceive += Receive;
            }
            catch (Exception e)
            {
                Debug.Print("exception!: " + e.ToString());
            }

            Debug.Print("OMAC init done");
            myAddress = myOMACObj.MACRadioObj.RadioAddress;
            Debug.Print("My address is: " + myAddress.ToString() + ". I am in Receive mode");
        }

		public void SendStatus(IMAC macBase, DateTime time, SendPacketStatus ACKStatus, uint transmitDestination, ushort index)
		{
			if (ACKStatus == SendPacketStatus.SendInitiated)
				Debug.Print("Packet to " + transmitDestination.ToString() + " started\r\n" );
			else if (ACKStatus == SendPacketStatus.SendACKed)
				Debug.Print("Packet to " + transmitDestination.ToString() + "  ACK\r\n");
			else if (ACKStatus == SendPacketStatus.SendNACKed)
				Debug.Print("Packet to " + transmitDestination.ToString() + "  NACK\r\n");
			else if (ACKStatus == SendPacketStatus.SendFailed)
				Debug.Print("Packet to " + transmitDestination.ToString() + "  failed\r\n");
		}
        //Keeps track of change in neighborhood
        public void NeighborChange(IMAC macBase, DateTime time)
        {
            ushort[] _neighborList;
            int neighborCnt = 0;
            _neighborList = MACBase.NeighborListArray();
            var status = macBase.NeighborList(_neighborList);

            foreach (var neighbor in _neighborList)
            {
                if (neighbor == 0) { continue; }
				Debug.Print(neighbor.ToString());
                neighborCnt++;
            }
            Debug.Print("Current neighbor count: " + neighborCnt.ToString());
            if (neighborCnt == 2)
            {
				Debug.Print("first milestone");
                hitTwoNeighbors = true;
            }
            if ((neighborCnt == 0) && (hitTwoNeighbors == true))
            {
				Debug.Print("second milestone");
                hitZeroNeighbors = true;
            }
            if ((neighborCnt == 2) && (hitTwoNeighbors == true) && (hitZeroNeighbors == true))
            {
                Debug.Print("result = PASS");
                Debug.Print("accuracy = " + errors.ToString());
                Debug.Print("resultParameter1 = ");
                Debug.Print("resultParameter2 = ");
                Debug.Print("resultParameter3 = " + totalRecvCounter.ToString());
                Debug.Print("resultParameter4 = null");
                Debug.Print("resultParameter5 = null");
            }
        }

        //Handles received messages 
        public void Receive(IMAC macBase, DateTime time, Packet receivedPacket)
        {
            totalRecvCounter++;


            byte[] rcvPayload = receivedPacket.Payload;
            if (rcvPayload != null)
            {
                PingPayload pingPayload = pingMsg.FromBytesToPingPayload(rcvPayload);
                if (pingPayload != null)
                {
                    //Debug.Print("Received msgID " + pingPayload.pingMsgId + " from SRC " + receivedPacket.Src);
                    NeighborTableInfo nbrTableInfo;
                    //If hashtable already contains an entry for the source, extract it, increment recvCount and store it back
                    if (neighborHashtable.Contains(receivedPacket.Src))
                    {
                        NeighborTableInfo nbrTableInfoAnalyze = (NeighborTableInfo)neighborHashtable[receivedPacket.Src];
                        //Debug.Print(receivedPacket.Src.ToString() + " " + pingPayload.pingMsgId.ToString() + " " + nbrTableInfoAnalyze.prevId.ToString());
                        if (pingPayload.pingMsgId != nbrTableInfoAnalyze.prevId + 1)
                        {
                            //Debug.Print("error");
                            errors++;
                        }

                        nbrTableInfo = (NeighborTableInfo)neighborHashtable[receivedPacket.Src];
                        nbrTableInfo.recvCount++;
                        nbrTableInfo.prevId = pingPayload.pingMsgId;
                        nbrTableInfo.AL.Add(pingPayload.pingMsgId);
                        neighborHashtable[receivedPacket.Src] = nbrTableInfo;

                    }
                    //If hashtable does not have an entry, create a new instance and store it
                    else
                    {
                        nbrTableInfo = new NeighborTableInfo();
                        nbrTableInfo.recvCount = 1;
                        nbrTableInfo.prevId = pingPayload.pingMsgId;
                        ArrayList AL = new ArrayList();
                        AL.Add(pingPayload.pingMsgId);
                        nbrTableInfo.AL = AL;
                        neighborHashtable[receivedPacket.Src] = nbrTableInfo;
                        //neighborHashtable.Add(receivedPacket.Src, nbrTableInfo);
                    }

                }

            }
        }


        public static void Main()
        {
            Program p = new Program();
            p.Initialize();
            Thread.Sleep(Timeout.Infinite);
        }
    }
}



