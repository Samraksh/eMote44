using System;
using System.Runtime.CompilerServices;


namespace Samraksh.eMote.NonVolatileMemory
{

    /// <summary>
    /// A data reference is is a pointer to a record in DataStore.
    /// Class methods to read and write from/to DataStore, to delete a data reference, and to get meta data about the reference.
    /// </summary>
    public class DataReference
    {
        private DataStore dStore;
        // Data Id unique to a pointer
        private UInt32 dataId = 0;
        private UInt32 referenceSize = 0;
        //Reference to data created on the storage device (not actual location. Similar to malloc returning a user space address)
        //private UInt32 dataReference = 0;
        //Actual location of data on storage device
        private UInt32 dataLocationOnStorage = 0;
        private ReferenceDataType referenceDataType;
        /// <summary>
        /// DataType of reference
        /// </summary>
        private Type dataType;

        private Byte dataTypeByte = new byte();
        private UInt16 dataTypeUInt16 = new UInt16();
        private UInt32 dataTypeUInt32 = new UInt32();


        /*/// <summary>
        /// Data constructor that takes DataStore and the data type as parameters. 
        /// Possible to create max of 256 active data objects.
        /// </summary>
        /// <param name="dStore">DataStore object</param>
        /// <param name="dataType">Type of data to be stored in DataStore</param>
        public DataReference(DataStore dStore, Type dataType)
        {
            this.dStore = dStore;
            this.dataType = dataType;
        }*/

        /// <summary>
        /// 
        /// </summary>
        /// <param name="offset"></param>
        /// <param name="numData"></param>
        private void VerifyDataReferenceParams(int offset, int numData)
        {
            if (offset < 0)
                throw new DataStoreException("Offset should not be negative");

            if (numData < 0)
                throw new DataStoreException("numData should not be negative");

            //Amount of data to be written should not be greater than allocation size
            if (numData > this.referenceSize)
                throw new DataStoreException("Amount of data to be written is greater than size of data allocated");

            //Amount of data to be written from offset should not be greater than allocation size
            if (offset + numData > this.referenceSize)
                throw new DataStoreException("Amount of data to be written exceeds size of data allocated");
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numData"></param>
        private void VerifyDataReferenceParams(byte[] data, int offset, int numData)
        {
            VerifyDataReferenceParams(offset, numData);

            if (data.Length == 0)
                throw new DataStoreException("data array cannot be of zero length");

            /*if (this.dataReference == 0x0)
                throw new DataStoreException("Reference to data is null");*/

            if (this.dataLocationOnStorage == 0x0)
                throw new DataStoreException("Data location is null");

            //DataType to be written should be the same as that created during allocation
            if (this.referenceDataType != ReferenceDataType.BYTE)
                throw new DataStoreException("DataType is not the same as the one during creation");

            //Amount of data to be written should not be greater than size of input array
            if (numData > 0 && numData > data.Length)
            {
                throw new DataStoreException("Amount of data to be written is greater than array size");
            }

            if(numData == 0)
            {
                //Amount of data to be written should not be greater than allocation size
                if (data.Length > this.referenceSize)
                    throw new DataStoreException("Amount of data to be written is greater than size of data allocated");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numData"></param>
        private void VerifyDataReferenceParams(UInt16[] data, int offset, int numData)
        {
            VerifyDataReferenceParams(offset, numData);

            if (data.Length == 0)
                throw new DataStoreException("data array cannot be of zero length");

            /*if (this.dataReference == 0x0)
                throw new DataStoreException("Reference to data is null");*/

            if (this.dataLocationOnStorage == 0x0)
                throw new DataStoreException("Data location is null");

            //DataType to be written should be the same as that created during allocation
            if (this.referenceDataType != ReferenceDataType.UINT16)
                throw new DataStoreException("DataType is not the same as the one during creation");

            //Amount of data to be written should not be greater than size of input array
            if (numData > 0 && numData > data.Length)
            {
                throw new DataStoreException("Amount of data to be written is greater than array size");
            }

            if (numData == 0)
            {
                //Amount of data to be written should not be greater than allocation size
                if (data.Length > this.referenceSize)
                    throw new DataStoreException("Amount of data to be written is greater than size of data allocated");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="data"></param>
        /// <param name="offset"></param>
        /// <param name="numData"></param>
        private void VerifyDataReferenceParams(UInt32[] data, int offset, int numData)
        {
            VerifyDataReferenceParams(offset, numData);

            if (data.Length == 0)
                throw new DataStoreException("data array cannot be of zero length");

            /*if (this.dataReference == 0x0)
                throw new DataStoreException("Reference to data is null");*/

            if (this.dataLocationOnStorage == 0x0)
                throw new DataStoreException("Data location is null");

            //DataType to be written should be the same as that created during allocation
            if (this.referenceDataType != ReferenceDataType.UINT32)
                throw new DataStoreException("DataType is not the same as the one during creation");

            //Amount of data to be written should not be greater than size of input array
            if (numData > 0 && numData > data.Length)
            {
                throw new DataStoreException("Amount of data to be written is greater than array size");
            }

            if (numData == 0)
            {
                //Amount of data to be written should not be greater than allocation size
                if (data.Length > this.referenceSize)
                    throw new DataStoreException("Amount of data to be written is greater than size of data allocated");
            }
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="retVal"></param>
        /// <returns></returns>
        private static DataStoreReturnStatus CheckWriteStatus(int retVal)
        {
            if (retVal == 0)
                return DataStoreReturnStatus.Success;
            else if (retVal == -3)
                return DataStoreReturnStatus.InvalidReference;
            else
                return DataStoreReturnStatus.Failure;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <param name="retVal"></param>
        /// <returns></returns>
        private static DataStoreReturnStatus CheckReadStatus(int retVal)
        {
            if (retVal == -3)
                return DataStoreReturnStatus.InvalidReference;
            else
                return DataStoreReturnStatus.Failure;
        }

        /// <summary>
        /// Default data class constructor. Possible to create max of 256 active data objects.
        /// </summary>
        /// <param name="dStore">DataStore object</param>
        /// <param name="refSize">Size of the data object to be stored in DataStore. 
        /// Max size is (2^32 - 1) if type is bytes; (2^31 - 1) if type is uint16; (2^30 - 1) if type is uint32</param>
        /// <param name="referenceDataType">Type of data to be stored in DataStore</param>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        /// <exception caption="Argument Exception" cref="System.ArgumentException">Method invocation has an invalid argument</exception>
        public DataReference(DataStore dStore, int refSize, ReferenceDataType referenceDataType)
        {
            if (dStore == null)
                throw new DataStoreException("DataStore object is NULL. DataStore could be corrupt. Please try erasing the flash while initializing (eraseDataStore = true) and try again.");

            this.dStore = dStore;
            this.referenceDataType = referenceDataType;

            //Size allocation should not be zero or negative
            if (refSize <= 0)
            {
                throw new ArgumentException("refSize cannot be less than or equal to zero", "refSize");
            }

            if (referenceDataType == ReferenceDataType.BYTE)
            {
                refSize = sizeof(byte) * refSize;
                //this.dataReference = (UInt32)CreateData((UInt32)refSize, 0);
                this.dataLocationOnStorage = (UInt32)CreateData((UInt32)refSize, 0);
                this.dataType = typeof(byte);
            }
            else if (referenceDataType == ReferenceDataType.UINT16)
            {
                refSize = sizeof(UInt16) * refSize;
                //this.dataReference = (UInt32)CreateData((UInt32)refSize, 1);
                this.dataLocationOnStorage = (UInt32)CreateData((UInt32)refSize, 1);
                this.dataType = typeof(UInt16);
            }
            else if (referenceDataType == ReferenceDataType.UINT32)
            {
                refSize = sizeof(UInt32) * refSize;
                //this.dataReference = (UInt32)CreateData((UInt32)refSize, 2);
                this.dataLocationOnStorage = (UInt32)CreateData((UInt32)refSize, 2);
                this.dataType = typeof(UInt32);
            }
            else
            {
                throw new ArgumentException("Invalid dataType", "dataType");
            }

            int datastoreStatus = GetLastDatastoreStatus();
            if (datastoreStatus == (int)DataStoreReturnStatus.Failure) {
                throw new DataStoreException("Failure while creating data reference");
            }
            else if (datastoreStatus == (int)DataStoreReturnStatus.InvalidArgument) {
                throw new DataStoreInvalidArgumentException("Failure - Invalid argument");
            }
            else if (datastoreStatus == (int)DataStoreReturnStatus.InvalidReference) {
                throw new DataStoreInvalidReferenceException("Failure - Invalid reference");
            }
            else if (datastoreStatus == (int)DataStoreReturnStatus.DataStoreNotInitialized) {
                throw new DataStoreNotInitializedException("Failure - datastore not initialized");
            }
            else if (datastoreStatus == (int)DataStoreReturnStatus.DataAllocationOutOfMemory) {
                throw new DataStoreOutOfMemoryException("Failure - out of memory during data allocation");
            }

            /*if (this.dataReference == 0)
                throw new DataStoreException("DataPointer is NULL. Data could not be created.");*/

            if (this.dataLocationOnStorage == 0) {
                throw new DataStoreException("Data location is NULL and I don't know what the datastoreStatus is. Data could not be created.");
            }

            this.referenceSize = (UInt32)refSize;
            this.dataId = GetDataID();

            if (this.dataId == 0)
                throw new DataStoreException("DataId is zero. Data could not be created.");
        }

        /// <summary>
        /// Returns a data object corresponding to the input parameter data ID.
        /// Possible to create max of 256 active data objects.
        /// </summary>
        /// <param name="dStore">DataStore object</param>
        /// <param name="dataId">ID of data for which user wants a reference/handle</param>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        /// <exception caption="Argument Exception" cref="System.ArgumentException">Method invocation has an invalid argument</exception>
        public DataReference(DataStore dStore, int dataId)
        {
            if (dStore == null)
                throw new DataStoreException("DataStore object is NULL. DataStore could be corrupt. Please try erasing the flash while initializing (eraseDataStore = true) and try again.");

            //dataId should not be negative
            if (dataId < 0)
                throw new ArgumentException("dataId cannot be less than zero", "dataId");

            this.dStore = dStore;
            this.dataId = (UInt32)dataId;
            /*this.dataReference = (UInt32)GetDataReference((UInt32)dataId);
            if (this.dataReference == 0)
                throw new DataStoreException("Reference to data is NULL. Data was not created.");*/

            this.dataLocationOnStorage = (UInt32)GetDataLocation((UInt32)dataId);
            if (this.dataLocationOnStorage == 0)
                throw new DataStoreException("No location for given dataID. Data was not created.");

            this.referenceSize = (UInt32)LookupDataSize((UInt32)dataId);
            if (this.referenceSize == 0)
                throw new DataStoreException("Data size is zero. Data was not created.");

            int retType = LookupDataType((UInt32)dataId);
            if (retType == 0)
            {
                this.referenceDataType = ReferenceDataType.BYTE;
                this.dataType = typeof(byte);
            }
            else if (retType == 1)
            {
                this.referenceDataType = ReferenceDataType.UINT16;
                this.dataType = typeof(UInt16);
            }
            else if (retType == 2)
            {
                this.referenceDataType = ReferenceDataType.UINT32;
                this.dataType = typeof(UInt32);
            }
            //m_Size = ConstructNativeMemoryPointer(dataId, MaxDataSize);
        }

        /// <summary>Write a byte array into DataStore, specifying the amount to write and the offset from the start of the record.</summary>
        /// <param name="data">Array to write</param>
        /// <param name="offset">Offset from start of DataStore allocation to write to, and not an offset from start of data array. Even value recommended for byte data type.</param>
        /// <param name="numData">Count of data to be written to DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(byte[] data, int offset, int numData)
        {
            int retVal = 0;

            try
            {
                VerifyDataReferenceParams(data, offset, numData);
                //VerifyDataReferenceParams(offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            //Since the NOR flash cannot write to odd addresses.
            if (offset % 2 == 1)
            {
                //offset = offset + sizeof(byte);
                throw new DataStoreException("Offset should not be odd for byte dataType");
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, data, (UInt32)offset, (UInt32)numData, dataTypeByte);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, data, (UInt32)offset, (UInt32)numData, dataTypeByte);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a ushsort (UInt16) array into DataStore, specifying the amount to write and the offset from the start of the record.</summary>
        /// <param name="data">Array to write</param>
        /// <param name="offset">Offset from start of DataStore allocation to write to, and not an offset from start of data array.</param>
        /// <param name="numData">Count of data to be written to DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(UInt16[] data, int offset, int numData)
        {
            int retVal = 0;

            try
            {
                VerifyDataReferenceParams(data, offset, numData);
                //VerifyDataReferenceParams(offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt16) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            for (uint objIndex = 0; objIndex < numData; ++objIndex)
            {
                byteArray[objIndex * sizeof(UInt16) + 0] = (byte)(data[objIndex] >> 8);
                byteArray[objIndex * sizeof(UInt16) + 1] = (byte)(data[objIndex]);
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a uint (UInt32) array into DataStore, specifying the amount to write and the offset from the start of the record.</summary>
        /// <param name="data">Array to write</param>
        /// <param name="offset">Offset from start of DataStore allocation to write to, and not an offset from start of data array.</param>
        /// <param name="numData">Count of data to be written to DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(UInt32[] data, int offset, int numData)
        {
            int retVal = 0;

            try
            {
                VerifyDataReferenceParams(data, offset, numData);
                //VerifyDataReferenceParams(offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt32) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            for (uint objIndex = 0; objIndex < numData; ++objIndex)
            {
                byteArray[objIndex * sizeof(UInt32) + 0] = (byte)(data[objIndex] >> 24);
                byteArray[objIndex * sizeof(UInt32) + 1] = (byte)(data[objIndex] >> 16);
                byteArray[objIndex * sizeof(UInt32) + 2] = (byte)(data[objIndex] >> 8);
                byteArray[objIndex * sizeof(UInt32) + 3] = (byte)(data[objIndex]);
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a byte array into DataStore, specifying the amount to write.</summary>
        /// <param name="data">Array to write</param>
        /// <param name="numData">Count of data to be written to DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(byte[] data, int numData)
        {
            int retVal = 0;
            UInt32 offset = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, data, offset, (UInt32)numData, dataTypeByte);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, data, offset, (UInt32)numData, dataTypeByte);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a ushsort (UInt16) array into DataStore, specifying the amount to write.</summary>
        /// <param name="data">Array to write</param>
        /// <param name="numData">Count of data to be written to DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(UInt16[] data, int numData)
        {
            int retVal = 0;
            UInt32 offset = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt16) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            for (uint objIndex = 0; objIndex < numData; ++objIndex)
            {
                byteArray[objIndex * sizeof(UInt16) + 0] = (byte)(data[objIndex] >> 8);
                byteArray[objIndex * sizeof(UInt16) + 1] = (byte)(data[objIndex]);
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, byteArray, offset, numBytes, dataTypeUInt16);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, byteArray, offset, numBytes, dataTypeUInt16);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a uint (UInt32) array into DataStore, specifying the amount to write.</summary>
        /// <param name="data">Array to write</param>
        /// <param name="numData">Count of data to be written to DataStore</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(UInt32[] data, int numData)
        {
            int retVal = 0;
            UInt32 offset = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt32) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            for (uint objIndex = 0; objIndex < numData; ++objIndex)
            {
                byteArray[objIndex * sizeof(UInt32) + 0] = (byte)(data[objIndex] >> 24);
                byteArray[objIndex * sizeof(UInt32) + 1] = (byte)(data[objIndex] >> 16);
                byteArray[objIndex * sizeof(UInt32) + 2] = (byte)(data[objIndex] >> 8);
                byteArray[objIndex * sizeof(UInt32) + 3] = (byte)(data[objIndex]);
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, byteArray, offset, numBytes, dataTypeUInt32);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, byteArray, offset, numBytes, dataTypeUInt32);
            return CheckWriteStatus(retVal);
        }


        /// <summary>Write a byte array into DataStore.</summary>
        /// <param name="data">Array to write</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(byte[] data)
        {
            int retVal = 0;
            UInt32 offset = 0;
            int numData = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, data, offset, (UInt32)data.Length, dataTypeByte);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, data, offset, (UInt32)data.Length, dataTypeByte);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a ushsort (UInt16) array into DataStore.</summary>
        /// <param name="data">Array to write</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(UInt16[] data)
        {
            int retVal = 0;
            UInt32 offset = 0;
            int numData = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt16) * (UInt32)data.Length;
            byte[] byteArray = new byte[numBytes];

            for (uint objIndex = 0; objIndex < data.Length; ++objIndex)
            {
                byteArray[objIndex * sizeof(UInt16) + 0] = (byte)(data[objIndex] >> 8);
                byteArray[objIndex * sizeof(UInt16) + 1] = (byte)(data[objIndex]);
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, byteArray, offset, numBytes, dataTypeUInt16);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, byteArray, offset, numBytes, dataTypeUInt16);
            return CheckWriteStatus(retVal);
        }

        /// <summary>Write a uint (UInt32) array into DataStore.</summary>
        /// <param name="data">Array to write</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Write(UInt32[] data)
        {
            int retVal = 0;
            UInt32 offset = 0;
            int numData = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt32) * (UInt32)data.Length;
            byte[] byteArray = new byte[numBytes];

            for (uint objIndex = 0; objIndex < data.Length; ++objIndex)
            {
                byteArray[objIndex * sizeof(UInt32) + 0] = (byte)(data[objIndex] >> 24);
                byteArray[objIndex * sizeof(UInt32) + 1] = (byte)(data[objIndex] >> 16);
                byteArray[objIndex * sizeof(UInt32) + 2] = (byte)(data[objIndex] >> 8);
                byteArray[objIndex * sizeof(UInt32) + 3] = (byte)(data[objIndex]);
            }

            //retVal = dStore.WriteRaw((uint)this.dataReference, byteArray, offset, numBytes, dataTypeUInt32);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.WriteRaw((uint)this.dataLocationOnStorage, byteArray, offset, numBytes, dataTypeUInt32);
            return CheckWriteStatus(retVal);
        }


        /// <summary>Read a byte array from DataStore, specifying the amount to read and the offset from start of allocation.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <param name="offset">Offset from start of DataStore allocation to read from, and not an offset from start of data array.</param>
        /// <param name="numData">Count of data to be read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(byte[] data, int offset, int numData)
        {
            int retVal = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams(offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(byte) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeByte);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeByte);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < numData; ++objIndex)
                {
                    data[objIndex] = (byte)(byteArray[objIndex * sizeof(byte)]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }

        /// <summary>Read a ushort (UInt16) array from DataStore, specifying the amount to read and the offset from start of allocation.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <param name="offset">Offset from start of DataStore allocation to read from, and not an offset from start of data array.</param>
        /// <param name="numData">Count of data to be read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(UInt16[] data, int offset, int numData)
        {
            int retVal = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams(offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt16) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < numData; ++objIndex)
                {
                    data[objIndex] = (UInt16)((byteArray[objIndex * sizeof(UInt16) + 0] << 8) +
                                                        byteArray[objIndex * sizeof(UInt16) + 1]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }

        /// <summary>Read a uint (UInt16) array from DataStore, specifying the amount to read and the offset from start of allocation.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <param name="offset">Offset from start of DataStore allocation to read from, and not an offset from start of data array.</param>
        /// <param name="numData">Count of data to be read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(UInt32[] data, int offset, int numData)
        {
            int retVal = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams(offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt32) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < numData; ++objIndex)
                {
                    data[objIndex] = (UInt32)((byteArray[objIndex * sizeof(UInt32) + 0] << 24) +
                                                    (byteArray[objIndex * sizeof(UInt32) + 1] << 16) +
                                                    (byteArray[objIndex * sizeof(UInt32) + 2] << 8) +
                                                    byteArray[objIndex * sizeof(UInt32) + 3]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }


        /// <summary>Read a byte array from DataStore, specifying the amount to read.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <param name="numData">Count of data to be read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(byte[] data, int numData)
        {
            int retVal = 0;
            UInt32 offset = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(byte) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, offset, numBytes, dataTypeByte);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, offset, numBytes, dataTypeByte);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < numData; ++objIndex)
                {
                    data[objIndex] = (byte)(byteArray[objIndex * sizeof(byte)]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }

        /// <summary>Read a ushort (UInt16) array from DataStore, specifying the amount to read.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <param name="numData">Count of data to be read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(UInt16[] data, int numData)
        {
            int retVal = 0;
            UInt32 offset = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt16) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < numData; ++objIndex)
                {
                    data[objIndex] = (UInt16)((byteArray[objIndex * sizeof(UInt16) + 0] << 8) +
                                                        byteArray[objIndex * sizeof(UInt16) + 1]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }

        /// <summary>Read a uint (UInt16) from DataStore, specifying the amount to read.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <param name="numData">Count of data to be read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(UInt32[] data, int numData)
        {
            int retVal = 0;
            UInt32 offset = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }
            
            UInt32 numBytes = sizeof(UInt32) * (UInt32)numData;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < numData; ++objIndex)
                {
                    data[objIndex] = (UInt32)((byteArray[objIndex * sizeof(UInt32) + 0] << 24) +
                                                    (byteArray[objIndex * sizeof(UInt32) + 1] << 16) +
                                                    (byteArray[objIndex * sizeof(UInt32) + 2] << 8) +
                                                    byteArray[objIndex * sizeof(UInt32) + 3]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }


        /// <summary>Read a byte array from DataStore.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(byte[] data)
        {
            int retVal = 0;
            UInt32 offset = 0;
            int numData = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }
            
            UInt32 numBytes = sizeof(byte) * (UInt32)data.Length;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, offset, numBytes, dataTypeByte);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, offset, numBytes, dataTypeByte);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < data.Length; ++objIndex)
                {
                    data[objIndex] = (byte)(byteArray[objIndex * sizeof(byte)]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }

        /// <summary>Read a ushort (UInt16) array from DataStore.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(UInt16[] data)
        {
            int retVal = 0;
            UInt32 offset = 0;
            int numData = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }
            
            UInt32 numBytes = sizeof(UInt16) * (UInt32)data.Length;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt16);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < data.Length; ++objIndex)
                {
                    data[objIndex] = (UInt16)((byteArray[objIndex * sizeof(UInt16) + 0] << 8) +
                                                        byteArray[objIndex * sizeof(UInt16) + 1]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }

        /// <summary>Read a uint (UInt16) from DataStore, specifying the amount to read and the offset from start of allocation.</summary>
        /// <param name="data">Array to receive data read</param>
        /// <returns>Success / failure status</returns>
        /// <exception caption="DataStore Exception" cref="DataStoreException">Method invocation has an invalid argument</exception>
        public DataStoreReturnStatus Read(UInt32[] data)
        {
            int retVal = 0;
            UInt32 offset = 0;
            int numData = 0;

            try
            {
                VerifyDataReferenceParams(data, (int)offset, numData);
                //VerifyDataReferenceParams((int)offset, numData);
            }
            catch (Exception ex)
            {
                throw ex;
            }

            UInt32 numBytes = sizeof(UInt32) * (UInt32)data.Length;
            byte[] byteArray = new byte[numBytes];

            //retVal = dStore.ReadRaw((uint)this.dataReference, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);
            this.dataLocationOnStorage = (uint)this.GetDataLocation(this.dataId);
            retVal = dStore.ReadRaw((uint)this.dataLocationOnStorage, byteArray, (UInt32)offset, numBytes, dataTypeUInt32);

            if (retVal == 0)
            {
                for (uint objIndex = 0; objIndex < data.Length; ++objIndex)
                {
                    data[objIndex] = (UInt32)((byteArray[objIndex * sizeof(UInt32) + 0] << 24) +
                                                    (byteArray[objIndex * sizeof(UInt32) + 1] << 16) +
                                                    (byteArray[objIndex * sizeof(UInt32) + 2] << 8) +
                                                    byteArray[objIndex * sizeof(UInt32) + 3]);
                }
                return DataStoreReturnStatus.Success;
            }
            else
            {
                return CheckReadStatus(retVal);
            }
        }


        /// <summary>
        /// Delete data from DataStore
        /// </summary>
        /// <returns>Success / failure status</returns>
        public DataStoreReturnStatus Delete()
        {
            if (DeleteData(dataId) == true)
                return DataStoreReturnStatus.Success;
            else
                return DataStoreReturnStatus.Failure;
        }


        /// <summary>
        /// Return data reference type 
        /// </summary>
        [Obsolete("Deprecated. Use Type instead.")]
        public Type getDataReferenceType
        {
            get
            {
                return this.dataType;
            }
        }

        /// <summary>
        /// Return data reference type 
        /// </summary>
        public Type Type
        {
            get
            {
                return dataType;
            }
        }

        /// <summary>
        /// Return data reference size 
        /// </summary>
        [Obsolete("Deprecated. Use Size instead.")]
        public int getDataReferenceSize
        {
            get
            {
                return (int)this.referenceSize;
            }
        }

        /// <summary>
        /// Return data reference size 
        /// </summary>
        public int Size
        {
            get
            {
                return (int)referenceSize;
            }
        }


        /*~Data()
        {
            DisposeNativeMemoryPointer(dataId);
            Debug.Print("Destructor");
        }*/


        ///////////////////////////////////Internal methods/////////////////////////

        /// <summary>
        /// Get data reference for specified data ID from DataStore
        /// </summary>
        /// <param name="dataId">ID of the data to be looked up</param>
        /// <returns>Address reference to the dataID</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetDataReference(UInt32 dataId);

        /// <summary>
        /// Get actual data location for specified data ID from DataStore
        /// </summary>
        /// <param name="dataId">ID of the data to be looked up</param>
        /// <returns>Address of data</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetDataLocation(UInt32 dataId);

        /// <summary>
        /// Get the data type of dataID
        /// </summary>
        /// <param name="dataId">ID of the data to be looked up</param>
        /// <returns>Data type</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int LookupDataType(UInt32 dataId);

        /// <summary>
        /// Get the allocation size for dataID
        /// </summary>
        /// <param name="dataId">ID of the data to be looked up</param>
        /// <returns>Allocation size</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int LookupDataSize(UInt32 dataId);

        /// <summary>
        /// Delete the data represented by the data id from DataStore.
        /// </summary>
        /// <param name="dataId">ID of the data to be deleted</param>
        /// <returns>True if success</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private bool DeleteData(UInt32 dataId);

        /// <summary>
        /// Creates data for the specified size in the block storage device. Allocates space, assigns a reference which is returned back to the user
        /// </summary>
        /// <param name="Size">Size of the data</param>
        /// <param name="dataTypeByte">Data type - byte</param>
        /// <returns>Reference to the created data</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int CreateData(UInt32 Size, byte dataTypeByte);

        /// <summary>
        /// Creates data for the specified size in the block storage device. Allocates space, assigns a reference which is returned back to the user
        /// </summary>
        /// <param name="Size">Size of the data</param>
        /// <param name="dataTypeUInt16">Data type - UInt16</param>
        /// <returns>Reference to the created data</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int CreateData(UInt32 Size, UInt16 dataTypeUInt16);

        /// <summary>
        /// Creates data for the specified size in the block storage device. Allocates space, assigns a reference which is returned back to the user
        /// </summary>
        /// <param name="Size">Size of the data</param>
        /// <param name="dataTypeUInt32">Data type - UInt32</param>
        /// <returns>Reference to the created data</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int CreateData(UInt32 Size, UInt32 dataTypeUInt32);

        /// <summary>
        /// Returns the current data ID
        /// </summary>
        /// <returns>Current DataID</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private UInt32 GetDataID();

        /// <summary>
        /// Gets error status of DataStore
        /// </summary>
        /// <returns>DataStore error status</returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int GetLastDatastoreStatus();


        /*/// <summary>
        /// 
        /// </summary>
        /// <param name="dataId"></param>
        /// <param name="bytes"></param>
        /// <returns></returns>
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private int ConstructNativeMemoryPointer(UInt32 dataId, UInt32 bytes);*/

        /// <summary>
        /// Native call that destroys data created on the flash
        /// </summary>
        /// <param name="dataId"></param>
        /// Write data array to DataStore starting from specified offset
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        extern private bool DisposeNativeMemoryPointer(UInt32 dataId);

    }
}
