using CommLib;
using System;
using System.Diagnostics;
using System.Text;
using xGameUtility;

//zzAdd: for temp test...
public interface IClear
{
    void Clear();
}
//public class TDebug
//{
//    public TDebug()
//    {
//    }

//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void Log(object message)
//    {
//#if WIN_CLIENT
//        	System.Console.WriteLine(message);
//#else
//        DevDebug.Log(message);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void Log(object message, Object context)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(message);
//#else
//        DevDebug.Log(message);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void LogError(object message)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(message);
//#else
//        DevDebug.LogError(message);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void LogError(object message, Object context)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(message);
//#else
//        DevDebug.LogError(message);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void LogException(System.Exception exception)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(exception);
//#else
//        DevDebug.LogException(exception);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void LogException(System.Exception exception, Object context)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(exception);
//#else
//        DevDebug.LogException(exception);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void LogWarning(object message)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(message);
//#else
//        DevDebug.LogWarning(message);
//#endif
//    }
//    [Conditional("UNITY_EDITOR"), Conditional("FORCE_LOG"), Conditional("UNITY_STANDALONE_WIN")]
//    public static void LogWarning(object message, Object context)
//    {
//#if WIN_CLIENT
//        System.Console.WriteLine(message);
//#else
//        DevDebug.LogWarning(message);
//#endif
//    }
//}


//
public class BaseStructRev : EventArgs, IClear
{
    /**主命令**/
    public ushort mCommand;
    /**副命令*/
    public ushort mSubCommand;

    /**结构体**/
    private byte[] mStruct;

    private int mPosition = 0;

    //owner...
    public object mSocket
    {
        get;
        set;
    }

    public void Clear()
    {

    }

    public BaseStructRev()
    {
    }
    public BaseStructRev(ushort mainCommand, ushort subCommand, object _socket)
    {
        Init(mainCommand, subCommand, _socket);
    }

    public void Init(ushort mainCommand, ushort subCommand, object _socket)
    {
        mCommand = mainCommand;
        mSubCommand = subCommand;
        mSocket = _socket;
        mPosition = 0;
    }

    //for debug:
    public BaseStructRev(byte[] buffer, int offset, int size)
    {
        byte[] head = new byte[4];
        Array.Copy(buffer, offset, head, 0, 4);

        int headInt = BitConverter.ToInt32(head, 0);

        short command = (short)((headInt >> 16) & 0xffff);
        short mcommand = (short)((command >> 8) & 0xff);
        short mSubcommand = (short)(command & 0xff);

        data = buffer;
        mPosition = offset;

        int len = ReadShort();
        mSubCommand = ReadByte();
        mCommand = ReadByte();

        UnityEngine.Debug.LogError(string.Format("BaseStructRev:{0}-{1}-{2}", len, mCommand, mSubCommand));
    }

    /// <summary>
    /// 从服务器接收到的字节数组
    /// </summary>
    public byte[] data
    {
        get
        {
            return mStruct;
        }
        set
        {
            mStruct = value;
        }
    }

    //返回剩余字节数
    public int LeftBytes()
    {
        return (mStruct!=null)?(mStruct.Length - mPosition):0;
    }

    public byte ReadByte()
    {
        if (mPosition + 1 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadByte遇到文件尾: " + mStruct.Length);
        }
        byte result = mStruct[mPosition];
        mPosition++;
        return result;
    }

    public int ReadShort()
    {
        if (mPosition + 2 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadShort遇到文件尾: " + mStruct.Length);
        }

        int value = BitConverter.ToInt16(mStruct, mPosition);
        mPosition += 2;
        return value;
    }

    public int ReadUnShort()
    {
        if (mPosition + 2 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadUnShort遇到文件尾: " + mStruct.Length);
        }

        int value = BitConverter.ToUInt16(mStruct, mPosition);
        mPosition += 2;
        return value;
    }

    public int ReadInt()
    {
        if (mPosition + 4 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadInt遇到文件尾: " + mStruct.Length);
        }

        int value = BitConverter.ToInt32(mStruct, mPosition);
        mPosition += 4;
        return value;
    }


    public uint ReadUnInt()
    {
        if (mPosition + 4 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadUnInt遇到文件尾: " + mStruct.Length);
        }

        uint value = BitConverter.ToUInt32(mStruct, mPosition);
        mPosition += 4;
        return value;
    }

    public float ReadFloat()
    {
        if (mPosition + 4 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadFloat遇到文件尾: " + mStruct.Length);
        }

        float value = BitConverter.ToSingle(mStruct, mPosition);
        mPosition += 4;
        return value;
    }

    public long ReadInt64()
    {
        if (mPosition + 8 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadInt64遇到文件尾: " + mStruct.Length);
        }

        long value = BitConverter.ToInt64(mStruct, mPosition);
        mPosition += 8;
        return value;
    }

    public double ReadDouble()
    {
        if (mPosition + 8 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadDouble遇到文件尾: " + mStruct.Length);
        }

        double value = BitConverter.ToDouble(mStruct, mPosition);
        mPosition += 8;
        return value;
    }

    public UInt64 ReadUInt64()
    {
        if (mPosition + 8 > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadUInt64遇到文件尾: " + mStruct.Length);
        }

        UInt64 value = BitConverter.ToUInt64(mStruct, mPosition);
        mPosition += 8;
        return value;
    }

    public string ReadBString()
    {
        int iStrLength = ReadUnShort();
        return ReadFixedString(iStrLength);
    }

    public string ReadFixedString(int length)
    {
        if (mPosition + length > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadFixedString遇到文件尾: " + mStruct.Length);
        }

        string value = Encoding.UTF8.GetString(mStruct, mPosition, length);
        mPosition += length;
        if (value.IndexOf('\0') > 0)
        {
            string newString = value.Substring(0, value.IndexOf('\0'));
            return newString;
        }
        return value;
    }

    public byte[] ReadBytes(int length)
    {
        if (mPosition + length > mStruct.Length)
        {
            throw new Exception("socket data " + mPosition + "处ReadBytes遇到文件尾: " + mStruct.Length);
        }
        byte[] result = new byte[length];
        Buffer.BlockCopy(mStruct, mPosition, result, 0, length);
        mPosition += length;
        return result;
    }

    public bool CheckEnded(int lastSize)
    {
        if (mPosition + lastSize > mStruct.Length)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    public int Position()
    {
        return mPosition;
    }
}//end of 

public class BaseStructRevPool : TSimplePoolClass<BaseStructRev>
{
    public static BaseStructRev SafeCreate(ushort mcommand, ushort mSubcommand, object mSocket)
    {
        BaseStructRev p = AutoCreate();
        p.Init(mcommand,  mSubcommand,  mSocket);
        return p;
    }
}

//TODO: 修改为FlatBuffers.ByteBuffer
namespace Comm.Network
{
    public class ZBitConverter
    {
        static byte[] _bbb = new byte[8];
        public static byte[] GetBytes(byte u)
        {
            _bbb[0] = (byte)(u);
            return _bbb;
        }
        public static byte[] GetBytes(short u)
        {
            _bbb[0] = (byte)(u);
            _bbb[1] = (byte)(u >> 8);
            return _bbb;
        }
        public static byte[] GetBytes(ushort u)
        {
            _bbb[0] = (byte)(u);
            _bbb[1] = (byte)(u >> 8);
            return _bbb;
        }
        public static byte[] GetBytes(int u)
        {
            _bbb[0] = (byte)(u);
            _bbb[1] = (byte)(u >> 8);
            _bbb[2] = (byte)(u >> 16);
            _bbb[3] = (byte)(u >> 24);
            return _bbb;
        }
        public static byte[] GetBytes(uint u)
        {
            _bbb[0] = (byte)(u);
            _bbb[1] = (byte)(u >> 8);
            _bbb[2] = (byte)(u >> 16);
            _bbb[3] = (byte)(u >> 24);
            return _bbb;
        }
        //public static byte[] GetBytes(float f)
        //{
        //    //BitConverter.
        //    unsafe
        //    {
        //        var pf = (byte*)&f;
        //        _bbb[0] = pf[0];
        //        _bbb[1] = pf[1];
        //        _bbb[2] = pf[2];
        //        _bbb[3] = pf[3];
        //    }
        //    //float.
        //    return _bbb;
        //}
        //public static byte[] GetBytes(long value)
        //{
        //    return _bbb;
        //}
        //public static byte[] GetBytes(double value)
        //{
        //    return _bbb;
        //}
    }


    public class BaseCons : IClear
    {
        /*主命令*/
        public short mCommand;
        //
        public short mSubCommand;

        public short mReliability;

        public byte[] mByte;

        //public byte[]   mBuffer;    //暂存数据（注意： 本结构可能为临时结构，请不要持久存储...）
        public int mPakLen;    //用于统计

        protected int mPosition;

        public int Length
        {
            get { return mPosition; }
        }

        /// <summary>
        /// 支持两种发包方式 1+==直接将data设置为一个结构体 2--一个字节一个字节的写 只能选其一 不能混用
        /// </summary>
        /// <param name="mainCommand"></param>
        /// <param name="subCommand"></param>
        public BaseCons(short mainCommand, short subCommand, int pre_len)
            : this(pre_len)
        {
            mReliability = 3;
            setCommand(mainCommand, subCommand);
        }
        public BaseCons(int pre_len)
        {
            if (pre_len > 0)
            {
                mByte = new byte[pre_len];//6200
            }
            mPosition = 0;
        }

        public void Clear()
        {
            mPosition = 0;
        }

        public void initParam(short mainCommand, short subCommand, int pre_len)
        {
            setCommand(mainCommand, subCommand);
            initPreLen(pre_len);
        }
        public void initPreLen(int pre_len)
        {
            if (mByte == null || mByte.Length < pre_len)
                mByte = new byte[pre_len];
            mPosition = 0;
        }

        public void setCommand(short mainCommand, short subCommand)
        {
            mCommand = mainCommand;
            mSubCommand = subCommand;
            mPosition = 0;
        }

        protected void EnsureSpace(int ll)
        {
            if (mPosition + ll > mByte.Length)
            {
                byte[] pp = new byte[mPosition + ll];
                Buffer.BlockCopy(mByte, 0, pp, 0, mPosition);
                mByte = pp;
            }
        }

        public void WriteByte(byte value)
        {
            EnsureSpace(16);

            Buffer.BlockCopy(ZBitConverter.GetBytes(value), 0, mByte, mPosition, 1);
            mPosition += 1;
        }

        public void WriteShort(short value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(ZBitConverter.GetBytes(value), 0, mByte, mPosition, 2);
            mPosition += 2;
        }

        public void WriteUnshort(ushort value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(ZBitConverter.GetBytes(value), 0, mByte, mPosition, 2);
            mPosition += 2;
        }

        public void WriteInt(int value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(ZBitConverter.GetBytes(value), 0, mByte, mPosition, 4);
            mPosition += 4;
        }

        public void WriteUint(uint value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(ZBitConverter.GetBytes(value), 0, mByte, mPosition, 4);
            mPosition += 4;
        }

        public void WriteFloat(float value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, mByte, mPosition, 4);
            mPosition += 4;
        }

        public void WriteLong(long value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, mByte, mPosition, 8);
            mPosition += 8;
        }

        public void WriteDouble(double value)
        {
            EnsureSpace(16);
            Buffer.BlockCopy(BitConverter.GetBytes(value), 0, mByte, mPosition, 8);
            mPosition += 8;
        }

        public void WriteFixedString(string value, int length)
        {
            byte[] pp = Encoding.UTF8.GetBytes(value);
            EnsureSpace(pp.Length + 16);
            Buffer.BlockCopy(pp, 0, mByte, mPosition, pp.Length);
            //         DevDebug.LogWarning("WriteFixedString:Length=" + length + ";str=" + value + ";pp.Length=" + pp.Length);
            mPosition += length;
        }

        public void WriteBString(string str)
        {
            byte[] pp = Encoding.UTF8.GetBytes(str);
            int length = pp.Length;

            EnsureSpace(pp.Length + 32);

            WriteShort((short)length);
            WriteFixedData(pp, length);
            //		DevDebug.LogWarning("WriteBString:Length=" + length + ";str=" + str);

        }

        public void WriteFixedData(byte[] value, int length)
        {
            EnsureSpace(length + 16);
            Buffer.BlockCopy(value, 0, mByte, mPosition, length);
            mPosition += length;
        }
    }

    /// <summary>
    /// zzFix: pre_len(128B)+52B=180B ;pre_len(64B)+52B=116B
    /// </summary>
    //    public class BaseStructSend : BaseCons
    //    {
    //        public BaseStructSend()
    //            : base(0, 0, 0)
    //        {
    //        }
    //        public BaseStructSend(short mainCommand, short subCommand) : base(mainCommand, subCommand, 64)
    //        {
    //            //base.(mainCommand, subCommand);
    //        }

    //        static byte[] s_Buffer = new byte[512]; //
    //        static byte[] head = new byte[4];//
    //        /// <summary>
    //        /// 打包 buffer,外部传入临时缓存;offset 为临时数据偏移;
    //        /// </summary>
    //        public byte[] packge(byte[] buffer, int offset)
    //        {
    //            //获得命令号的组合
    //            short command = (short)(((ushort)mCommand << 8) | (ushort)mSubCommand);
    //            //把数据包的长度和命令号组合成一个int 数据包的长度包括了头文件的长度 这里头文件是4个byte
    //            int headInt = (int)((command << 16) | (Length + 4));

    //            //把头文件写入字节数组
    //            //byte[] head = BitConverter.GetBytes(headInt);   //zzMem: 20B
    //            //byte[] head = new byte[RemoteManager.HEAD_LENGTH];
    //            head[0] = (byte)(headInt);
    //            head[1] = (byte)(headInt >> 8);
    //            head[2] = (byte)(headInt >> 16);
    //            head[3] = (byte)(headInt >> 24);

    //            byte[] mBuffer;
    //            if (Length == 0)
    //            {
    //                mBuffer = buffer == null ? new byte[head.Length] : buffer;
    //                Buffer.BlockCopy(head, 0, mBuffer, 0 + offset, head.Length);
    //                this.mPakLen = head.Length + offset;
    //                return mBuffer;
    //            }

    //            byte[] newdata = Length < 512 ? s_Buffer : new byte[Length];
    //            Buffer.BlockCopy(mByte, 0, newdata, 0, Length);

    //            //加密+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    ////            if (mCommand != (int)EMainProtcol.PUBLIC_LOGIN_MAIN)
    ////            {
    ////#if ENABLE_PROFILER
    ////                ////Profiler.BeginSample("EncryptEx");
    ////#endif
    ////                newdata = RC4Crypt.RC4.FastEncrypt(newdata, Length);
    ////#if ENABLE_PROFILER
    ////                ////Profiler.EndSample();
    ////#endif
    ////            }
    //            //加密+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //            //此时就有了两个字节数组，一个是标记数据包的长度字节数组， 一个是数据包字节数组，
    //            //同时把这两个字节数组合并成一个字节数组
    //            mPakLen = head.Length + Length + offset;
    //            if (buffer == null || (mPakLen) > buffer.Length)
    //            {
    //                mBuffer = new byte[mPakLen];
    //            }
    //            else
    //            {
    //                mBuffer = buffer;
    //            }

    //            //把两个字节的数据copy到一个字节数组里面去
    //            Buffer.BlockCopy(head, 0, mBuffer, 0 + offset, head.Length);
    //            Buffer.BlockCopy(newdata, 0, mBuffer, head.Length + offset, Length);
    //            return mBuffer;
    //        }
    //    }


    //
    //public class BaseStructRevPool : XPoolClass<BaseStructRev>
    //{
    //    static object _criticObj = new object();
    //    public static BaseStructRev SafeCreate(ushort mainCommand, ushort subCommand, object _socket)
    //    {
    //        BaseStructRev r;
    //        lock (_criticObj)
    //        {
    //            r = AutoCreate();
    //            r.Init(mainCommand, subCommand, _socket);
    //        }
    //        return r;
    //    }
    //    public static void SafeRecycle(BaseStructRev pNode)
    //    {
    //        lock (_criticObj)
    //        {
    //            Recycle(pNode);
    //        }
    //    }
    //}



    /// <summary>
    /// zzAdd: for data send...
    /// </summary>
    class StructSend
    {
        static object[] _ObjectParam_II = new object[] { 0, 0 };
        static public object[] GetObjectParam_Int2(int param1, int param2)
        {
            _ObjectParam_II[0] = param1;
            _ObjectParam_II[1] = param2;
            return _ObjectParam_II;
        }

        //位置移动参数:
        static object[] s_PosPro = new object[] { (short)0, (float)0, (float)0, (float)0, (byte)0 };
        static public object[] GetPosPro(short InstanceID, float x, float z, float CurAngle, byte state)
        {
            s_PosPro[0] = (short)InstanceID;
            s_PosPro[1] = x;
            s_PosPro[2] = z;
            s_PosPro[3] = CurAngle;
            s_PosPro[4] = state;
            return s_PosPro;
        }
        static object[] _ObjectParam_IB = new object[] { 0, (byte)0 };
        static public object[] GetObjectParam_IB(int param1, byte param2)
        {
            _ObjectParam_IB[0] = param1;
            _ObjectParam_IB[1] = param2;
            return _ObjectParam_IB;
        }
    }


    public class DataEventArgs : EventArgs
    {
        public byte[] Data { get; set; }
        public int Offset { get; set; }
        public int Length { get; set; }
    }

}
