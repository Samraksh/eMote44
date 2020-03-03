using System;
using Microsoft.SPOT;

namespace Samraksh.eMote.Net.Radio
{
    /// <summary>SI4468 radio object</summary>
    public class Radio_802_15_4_SI4468 : Radio_802_15_4_Base
    {
        private static bool _si4468RadioInstanceSet;

        /// <summary>Constructor</summary>
        public Radio_802_15_4_SI4468()
        {
        }

        /// <summary>Constructor</summary>
        /// <param name="user">Radio user</param>
        public Radio_802_15_4_SI4468(RadioUser user)
        {
            if (!_si4468RadioInstanceSet)
            {
                _si4468RadioInstanceSet = true;
                if (user == RadioUser.CSMA)
                {
                    CurrUser = RadioUser.CSMA;
                }
                else if (user == RadioUser.OMAC)
                {
                    CurrUser = RadioUser.OMAC;
                }
                else if (user == RadioUser.CSharp)
                {
                    CurrUser = RadioUser.CSharp;
                }
            }
            else
                Debug.Print("Long range radio (SI4468) already initialized");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public DeviceStatus Radio_802_15_4_SI4468_UnInitialize()
        {
            _si4468RadioInstanceSet = false;
            CurrUser = RadioUser.Idle;
            return DeviceStatus.Success;
            //Radio is uninitialized when MAC is uninitialized
            //return UnInitialize((byte)RadioName.SI4468);
        }

    }
}
