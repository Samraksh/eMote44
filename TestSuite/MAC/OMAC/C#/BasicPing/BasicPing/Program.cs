#define RF231
//#define SI4468

using System;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using System.Threading;

using Samraksh.eMote.Net;
using Samraksh.eMote.Net.MAC;
using Samraksh.eMote.Net.Radio;
using Samraksh.eMote.DotNow;

//1. This program initializes OMAC as the MAC protocol.
//  1a. Registers a function that tracks change in neighbor (NeighborChange) and a function to handle messages that are received.
//2. Pings are sent at pre-determined intervals.
//3. Pongs are sent back for received messages.
namespace Samraksh.eMote.Net.Mac.Ping
{
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
                return null;
            }
        }
    }

    public class Program
    {
        //public variables
        UInt32 totalPingCount = 1000;
        //const UInt16 MAX_NEIGHBORS = 12;
        UInt16 dutyCyclePeriod = 20000;
        bool startSend = false;
        UInt16 myAddress;
        Timer sendTimer;
        NetOpStatus status;
        static UInt32 sendMsgCounter = 0;
        static UInt32 recvMsgCounter = 1;
        static UInt32 totalRecvCounter = 0;
        EmoteLCD lcd;
        
        PingPayload pingMsg = new PingPayload();
        
        OMAC myOMACObj;
        //ReceiveCallBack myReceiveCB;
        //NeighborhoodChangeCallBack myNeibhborhoodCB;

        //MACConfiguration myMacConfig = new MACConfiguration();
        //Radio.RadioConfiguration myRadioConfig = new Radio.RadioConfiguration();
                
        public void Initialize()
        {
            //Init LCD
            lcd = new EmoteLCD();
            lcd.Initialize();
            lcd.Write(LCD.CHAR_I, LCD.CHAR_n, LCD.CHAR_i, LCD.CHAR_t);

            Debug.Print("Initializing radio");
#if RF231
            var radioConfiguration = new RF231RadioConfiguration(RF231TxPower.Power_3dBm, RF231Channel.Channel_13);
#elif SI4468
            var radioConfiguration = new SI4468RadioConfiguration(SI4468TxPower.Power_15Point5dBm, SI4468Channel.Channel_01);
#endif

            Debug.Print("Configuring OMAC...");

            try
            {
                //configure OMAC
                myOMACObj = new OMAC(radioConfiguration);
                myOMACObj.OnReceive += Receive;
                myOMACObj.OnNeighborChange += NeighborChange;
            }
            catch(Exception e)
            {
                Debug.Print(e.ToString());
            }

            Debug.Print("OMAC init done");
            myAddress = myOMACObj.MACRadioObj.RadioAddress;
            Debug.Print("My address is: " + myAddress.ToString());
        }

        //Keeps track of change in neighborhood
        public void NeighborChange(IMAC macBase, DateTime time)
        {
            //Debug.Print("Count of neighbors " + countOfNeighbors.ToString());
        }

        //Starts a timer 
        public void Start()
        {
            Debug.Print("Starting timer...");
            TimerCallback timerCB = new TimerCallback(sendTimerCallback);
            sendTimer = new Timer(timerCB, null, 0, dutyCyclePeriod);
            Debug.Print("Timer initialization done");
        }

        //Calls ping at regular intervals
        void sendTimerCallback(Object obj)
        {
            SendPing();
        }

        public void SendPing()
        {
            try
            {
                bool sendFlag = false;
                UInt16[] neighborList = OMAC.NeighborListArray();
                DeviceStatus dsStatus = myOMACObj.NeighborList(neighborList);

                if (dsStatus == DeviceStatus.Success)
                {
                    foreach (var neighbor in neighborList)
                    {
                        if (neighbor != 0)
                        {
                            //Debug.Print("count of neighbors " + neighborList.Length);
                            startSend = true; sendFlag = true;
                            sendMsgCounter++;
                            pingMsg.pingMsgId = sendMsgCounter;
                            byte[] payload = pingMsg.ToBytes();
                            Debug.Print("Sending to neighbor " + neighbor + " ping msgID " + sendMsgCounter);

                            status = myOMACObj.Send(neighbor, PayloadType.MFM_Data, payload, 0, (ushort)payload.Length, DateTime.Now);
                            if (status != NetOpStatus.S_Success)
                            {
                                Debug.Print("Send failed. Ping msgID " + sendMsgCounter.ToString());
                            }
                        }
                    }
                }
                else
                {
                    Debug.Print("Could not get neighbor list");
                }
                if (sendFlag == false && startSend == true)
                {
                    Debug.Print("Ping failed. All neighbors dropped out");
                }
                
                if (sendMsgCounter < 10) 
                {
                    lcd.Write(LCD.CHAR_S, LCD.CHAR_S, LCD.CHAR_S, (LCD)sendMsgCounter);
                }
                else if (sendMsgCounter < 100)
                {
                    UInt16 tenthPlace = (UInt16)(sendMsgCounter / 10);
                    UInt16 unitPlace = (UInt16)(sendMsgCounter % 10);
                    lcd.Write(LCD.CHAR_S, LCD.CHAR_S, (LCD)tenthPlace, (LCD)unitPlace);
                }
                else if (sendMsgCounter < 1000)
                {
                    UInt16 hundredthPlace = (UInt16)(sendMsgCounter / 100);
                    UInt16 remainder = (UInt16)(sendMsgCounter % 100);
                    UInt16 tenthPlace = (UInt16)(remainder / 10);
                    UInt16 unitPlace = (UInt16)(remainder % 10);
                    lcd.Write(LCD.CHAR_S, (LCD)hundredthPlace, (LCD)tenthPlace, (LCD)unitPlace);
                }
                else if (sendMsgCounter < 10000)
                {
                    UInt16 thousandthPlace = (UInt16)(sendMsgCounter / 1000);
                    UInt16 remainder = (UInt16)(sendMsgCounter % 1000);
                    UInt16 hundredthPlace = (UInt16)(remainder / 100);
                    remainder = (UInt16)(remainder % 100);
                    UInt16 tenthPlace = (UInt16)(remainder / 10);
                    UInt16 unitPlace = (UInt16)(remainder % 10);
                    lcd.Write((LCD)thousandthPlace, (LCD)hundredthPlace, (LCD)tenthPlace, (LCD)unitPlace);
                }

                if (sendMsgCounter == totalPingCount)
                {
                    ShowStatistics();
                }
            }
            catch (Exception ex)
            {
                Debug.Print("SendPing: " + ex.ToString());
            }
        }

        //Handles received messages 
        public void Receive(IMAC macBase, DateTime time, Packet receivedPacket)
        {
            totalRecvCounter++;
            Debug.Print("---------------------------");
            /*if (myOMACObj.GetPendingPacketCount_Receive() == 0)
            {
                Debug.Print("no packets");
                return;
            }

            Packet receivedPacket = myOMACObj.GetNextPacket();*/
            if (receivedPacket == null)
            {
                Debug.Print("null");
                return;
            }

            Debug.Print("totalRecvCounter is " + totalRecvCounter);

            byte[] rcvPayload = receivedPacket.Payload;
            PingPayload pingPayload = pingMsg.FromBytesToPingPayload(rcvPayload);
            if (pingPayload != null)
            {
                Debug.Print("Received msgID " + pingPayload.pingMsgId);
                while (recvMsgCounter < pingPayload.pingMsgId)
                {
                    Debug.Print("Missed msgID: " + recvMsgCounter);
                    recvMsgCounter++;
                }
                recvMsgCounter = pingPayload.pingMsgId + 1;
                Debug.Print("Received msgContent " + pingPayload.pingMsgContent.ToString());
            }
            else
            {
                Debug.Print("Received a null msg");
            }

            Debug.Print("---------------------------");
        }

        //Show statistics
        void ShowStatistics()
        {
            Debug.Print("==============STATS================");
            Debug.Print("total msgs sent " + sendMsgCounter);
            Debug.Print("total msgs received " + totalRecvCounter);
            //Debug.Print("percentage received " + (totalRecvCounter / sendMsgCounter) * 100);
            Debug.Print("==================================");
            Thread.Sleep(Timeout.Infinite);
        }

        public static void Main()
        {
            Program p = new Program();
            p.Initialize();
            p.Start();
            Thread.Sleep(Timeout.Infinite);
        }
    }
}
