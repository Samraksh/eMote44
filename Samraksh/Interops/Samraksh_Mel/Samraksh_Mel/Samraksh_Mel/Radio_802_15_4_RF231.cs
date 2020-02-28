using System;
using Microsoft.SPOT;

namespace Samraksh_Mel.Radio
{
    /// <summary>RF231 radio object</summary>
    public class Radio_802_15_4_RF231 : Radio_802_15_4_Base
    {
        private static bool _genericRadioInstanceSet;

        /// <summary>Constructor</summary>
        public Radio_802_15_4_RF231()
        {
        }

        /// <summary>Constructor</summary>
        /// <param name="user"></param>
        public Radio_802_15_4_RF231(RadioUser user)
        {
            if (!_genericRadioInstanceSet)
            {
                _genericRadioInstanceSet = true;
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
                Debug.Print("Generic radio (RF231) already initialized");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <returns></returns>
        public DeviceStatus Radio_802_15_4_RF231_UnInitialize()
        {
            _genericRadioInstanceSet = false;
            CurrUser = RadioUser.Idle;
            return DeviceStatus.Success;
            //Radio is uninitialized when MAC is uninitialized
            //return UnInitialize((byte)RadioName.RF231);
        }

    }
}
