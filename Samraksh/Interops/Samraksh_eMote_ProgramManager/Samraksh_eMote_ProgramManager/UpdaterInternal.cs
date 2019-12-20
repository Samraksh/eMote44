using System;
using System.Runtime.CompilerServices;
using Microsoft.SPOT;
using Microsoft.SPOT.Hardware;

namespace Samraksh.eMote.ProgramManager
{
    class UpdaterInternal : NativeEventDispatcher
    {
        #region MULTI-THREAD_SINGLETON
        private static /*volatile*/ UpdaterInternal instance;
        private static object syncRoot = new Object();

        private static string strDrvName = "UpdaterProgressCallback";
        private static ulong drvData = 0;

        private UpdaterInternal(string strDrvName, ulong drvData)
            : base(strDrvName, drvData) {
        }

        public static bool Configure(string strDrvName, ulong drvData) {
            if (instance == null) {
                lock (syncRoot) {
                    if (instance == null) {
                        UpdaterInternal.strDrvName = strDrvName;
                        UpdaterInternal.drvData = drvData;
                        return true;
                    }
                }
            }
            return false;
        }

        public static UpdaterInternal Instance
        {
            get {
                if (instance == null) {
                    lock (syncRoot) {
                        if (instance == null) {
                            instance = new UpdaterInternal(strDrvName, drvData);
                        }
                    }
                }
                return instance;
            }
        }

        #endregion
        

    }
}
