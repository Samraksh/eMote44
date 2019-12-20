using System;
using Microsoft.SPOT;
using System.Runtime.CompilerServices;

namespace Samraksh.eMote
{
    /// <summary>
    /// Types of hardware platforms  
    /// </summary>
    public enum PLATFORM_TYPE
    {
        /// <summary>Unknown</summary>
        UNKNOWN,
        /// <summary>.NOW</summary>
        DOTNOW,
        /// <summary>WLN</summary>
        WLN            
    };

    public static class Platform
    {
        public static PLATFORM_TYPE Type
        {
            get
            {
                return GetPlatformTypeInternal();
            }
        }
        ///////////////////////////////////Internal methods/////////////////////////

        /// <summary>
        /// Initializes the platformType variable
        /// </summary>
        /// <returns>The platform type is returned.</returns>
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern PLATFORM_TYPE GetPlatformTypeInternal();
    }
}
