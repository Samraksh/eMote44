#define RF231
//#define SI4468

using System;
using System.IO;
using System.IO.Ports;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;
using System.Threading;

using Samraksh.eMote.Net;
using Samraksh.eMote.Net.MAC;
using Samraksh.eMote.Net.Radio;
//using Samraksh.eMote.DotNow;

//1. This program initializes OMAC as the MAC protocol.
//  1a. Registers a function that tracks change in neighbor (NeighborChange) and a function to handle messages that are received.
//2. Pings are sent at pre-determined intervals.
//3. Pongs are sent back for received messages.
namespace Samraksh.eMote.Net.Mac.TestUnInit.Send
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
        const UInt16 SecToMsec = 1000;
        const UInt32 totalPingCount = 10001;
        const int initialDelayInMsecs = 75 * SecToMsec;
        const UInt16 uninitAtThisValue = 10;
        const int dutyCyclePeriodInMsecs = 20 * SecToMsec;
        const UInt16 neighborLivenessDelayInSecs = 360;
        const int howLongToRemainUnInitializedInMsecs = (neighborLivenessDelayInSecs + 20) * SecToMsec;   //Wait until node is dropped from neighborTable

        bool startSend = false;
        bool initialize = false;
        UInt16 myAddress;
        Timer OMACSendTimer = null;
        Timer uninitTimer = null;
        NetOpStatus status;
        //EmoteLCD lcd;
        static UInt32 sendMsgCounter = 1;

        PingPayload pingMsg = new PingPayload();
        OMAC myOMACObj;

        public void Initialize()
        {
            //Init LCD
            //lcd = new EmoteLCD();
            //lcd.Initialize();
            //lcd.Write(LCD.CHAR_I, LCD.CHAR_n, LCD.CHAR_i, LCD.CHAR_t);

            Debug.Print("2.Initializing radio");
#if (RF231)
            var radioConfiguration = new RF231RadioConfiguration(RF231TxPower.Power_3dBm, RF231Channel.Channel_13);
#elif (SI4468)
            var radioConfiguration = new SI4468RadioConfiguration(SI4468TxPower.Power_20dBm, SI4468Channel.Channel_01);
#endif

            Debug.Print("Configuring OMAC...");

            try
            {
                //configure OMAC
                myOMACObj = new OMAC(radioConfiguration, neighborLivenessDelayInSecs);
                myOMACObj.OnReceive += Receive;
                myOMACObj.OnNeighborChange += NeighborChange;
            }
            catch (Exception e)
            {
                Debug.Print(e.ToString());
            }

            Debug.Print("OMAC init done");
            myAddress = myOMACObj.MACRadioObj.RadioAddress;
            Debug.Print("My address is: " + myAddress.ToString() + ". I am in Send mode");

            initialize = true;
        }

        public void UnInitialize()
        {
            initialize = false;
            StopOMACSendTimer();
            Debug.Print("Uninitializing OMAC");
            myOMACObj.Dispose();
        }

        //Starts a timer 
        public void StartOMACSendTimer()
        {
            Debug.Print("Waiting to start test");
            Thread.Sleep(initialDelayInMsecs);
            Debug.Print("Starting timer...");
            TimerCallback timerCB = new TimerCallback(sendTimerEvent);
            if (OMACSendTimer == null)
            {
                OMACSendTimer = new Timer(timerCB, null, 0, dutyCyclePeriodInMsecs);
            }
            else
            {
                OMACSendTimer.Change(0, dutyCyclePeriodInMsecs);
            }
            Debug.Print("Timer initialization done");
        }

        public void StopOMACSendTimer()
        {
            bool result = OMACSendTimer.Change(Timeout.Infinite, Timeout.Infinite);
            if (!result)
            {
                throw new Exception("Timer could not be stopped");
            }
        }

        void StartUnInitTimer(Object obj)
        {
            if (!initialize)
            {
                StopUnInitTimer();
                Initialize();
                StartOMACSendTimer();
            }
        }

        public void StopUnInitTimer()
        {
            uninitTimer.Change(Timeout.Infinite, Timeout.Infinite);
        }

        //Calls ping at regular intervals
        void sendTimerEvent(Object obj)
        {
            SendPing();
        }

        public void SendPing()
        {
            try
            {
                if (sendMsgCounter % uninitAtThisValue == 0 && initialize == true)
                {
                    sendMsgCounter++;
                    UnInitialize();

                    TimerCallback timerCB = new TimerCallback(StartUnInitTimer);
                    uninitTimer = new Timer(timerCB, null, howLongToRemainUnInitializedInMsecs, 0);
                    return;
                }

                bool sendFlag = false;
                UInt16[] neighborList = OMAC.NeighborListArray();
                DeviceStatus dsStatus = myOMACObj.NeighborList(neighborList);

                if (dsStatus == DeviceStatus.Success)
                {
                    //for (int j = 0; j < neighborList.Length; j++)
                    //{
                        //if (neighborList[j] != 0)
                        //{
                            //Debug.Print("count of neighbors " + neighborList.Length);
                            startSend = true; sendFlag = true;
                            pingMsg.pingMsgId = sendMsgCounter;
                            byte[] payload = pingMsg.ToBytes();

                            /*Debug.Print("Sending to neighbor " + neighborList[j] + " ping msgID " + sendMsgCounter + " payload length " + payload.Length);
                            status = myOMACObj.Send(neighborList[j], PayloadType.Type00, payload, 0, (ushort)payload.Length, DateTime.Now);
                            if (status != NetOpStatus.S_Success)
                            {
                                Debug.Print("Send to " + neighborList[j] + " failed. Ping msgID " + sendMsgCounter.ToString());
                            }

                            if (myOMACObj.NeighborStatus(neighborList[j]) == null)
                            {
                                Debug.Print("neighborstatus is null");
                            }*/

#if (RF231)
                            Debug.Print("Sending to neighbor " + 3505 + " ping msgID " + sendMsgCounter + " payload length " + payload.Length + " on channel " + PayloadType.MFM_Data);
                            status = myOMACObj.Send(3505, PayloadType.MFM_Data, payload, 0, (ushort)payload.Length);
                            if (status != NetOpStatus.S_Success)
                            {
                                Debug.Print("Send to " + 3505 + " failed. Ping msgID " + sendMsgCounter.ToString());
                            }
#endif

#if (SI4468)
                            Debug.Print("Sending to neighbor " + 2488 + " ping msgID " + sendMsgCounter + " payload length " + payload.Length + " on channel " + PayloadType.MFM_Data);
                            status = myOMACObj.Send(2488, PayloadType.MFM_Data, payload, 0, (ushort)payload.Length);
                            if (status != NetOpStatus.S_Success)
                            {
                                Debug.Print("Send to " + 2488 + " failed. Ping msgID " + sendMsgCounter.ToString());
                            }
#endif
                        //}
                    //}
                }
                if (sendFlag == false && startSend == true)
                {
                    Debug.Print("Ping failed. All neighbors dropped out");
                }
                //Increment msgCounter only if there is a send
                if (sendFlag == true)
                {
                    sendMsgCounter++;
                }

                if (sendMsgCounter < 10)
                {
                    //lcd.Write(LCD.CHAR_S, LCD.CHAR_S, LCD.CHAR_S, (LCD)sendMsgCounter);
                }
                else if (sendMsgCounter < 100)
                {
                    UInt16 tenthPlace = (UInt16)(sendMsgCounter / 10);
                    UInt16 unitPlace = (UInt16)(sendMsgCounter % 10);
                    //lcd.Write(LCD.CHAR_S, LCD.CHAR_S, (LCD)tenthPlace, (LCD)unitPlace);
                }
                else if (sendMsgCounter < 1000)
                {
                    UInt16 hundredthPlace = (UInt16)(sendMsgCounter / 100);
                    UInt16 remainder = (UInt16)(sendMsgCounter % 100);
                    UInt16 tenthPlace = (UInt16)(remainder / 10);
                    UInt16 unitPlace = (UInt16)(remainder % 10);
                    //lcd.Write(LCD.CHAR_S, (LCD)hundredthPlace, (LCD)tenthPlace, (LCD)unitPlace);
                }
                else if (sendMsgCounter < 10000)
                {
                    UInt16 thousandthPlace = (UInt16)(sendMsgCounter / 1000);
                    UInt16 remainder = (UInt16)(sendMsgCounter % 1000);
                    UInt16 hundredthPlace = (UInt16)(remainder / 100);
                    remainder = (UInt16)(remainder % 100);
                    UInt16 tenthPlace = (UInt16)(remainder / 10);
                    UInt16 unitPlace = (UInt16)(remainder % 10);
                    //lcd.Write((LCD)thousandthPlace, (LCD)hundredthPlace, (LCD)tenthPlace, (LCD)unitPlace);
                }

                if (sendMsgCounter == totalPingCount)
                {
                    sendMsgCounter--;
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

        }

        //Keeps track of change in neighborhood
        public void NeighborChange(IMAC macBase, DateTime time)
        {
            
        }

        //Show statistics
        void ShowStatistics()
        {
            Debug.Print("==============STATS================");
            Debug.Print("total msgs sent " + sendMsgCounter);
            Debug.Print("==================================");
            Thread.Sleep(Timeout.Infinite);
        }

        public static void Main()
        {
            Program p = new Program();
            p.Initialize();
            p.StartOMACSendTimer();
            Thread.Sleep(Timeout.Infinite);
        }
    }
}
