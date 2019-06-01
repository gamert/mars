
#if UNITY_EDITOR || ((UNITY_STANDALONE_WIN || UNITY_STANDALONE_OSX) && DEVELOPMENT_BUILD)
#define KNET_LOGGING
#endif

using System;
using System.Text;
using System.Runtime.InteropServices;



namespace KNET
{
    /*
        KNET version number.  Check this against KNET::System::getVersion / System_GetVersion
        0xaaaabbcc -> aaaa = major version number.  bb = minor version number.  cc = development version number.
    */
    public class VERSION
    {
        public const int    number = 0x00010814;
#if (UNITY_IPHONE || UNITY_TVOS) && !UNITY_EDITOR
        public const string dll    = "__Internal";
#elif (UNITY_PS4) && !UNITY_EDITOR
        public const string dll    = "libknet";
#elif (UNITY_PSP2) && !UNITY_EDITOR
        public const string dll    = "libknetstudio";
#elif (UNITY_WIIU) && !UNITY_EDITOR
        public const string dll    = "libknetstudio";
#elif (UNITY_EDITOR_WIN) || (UNITY_STANDALONE_WIN && DEVELOPMENT_BUILD)
        public const string dll    = "knetstudiol";
#elif (UNITY_STANDALONE_WIN)
        public const string dll    = "knetstudio";
#elif UNITY_EDITOR_OSX || (UNITY_STANDALONE_OSX && DEVELOPMENT_BUILD)
        public const string dll    = "knetl";
#else
        public const string dll    = "knet";
#endif
    }

    public class CONSTANTS
    {
        public const int MAX_CHANNEL_WIDTH = 32;
        public const int MAX_LISTENERS = 8;
    }

    /*
        KNET types
    */

    /*
    [ENUM]
    [
        [DESCRIPTION]
        error codes.  Returned from every function.

        [REMARKS]

        [SEE_ALSO]
    ]
    */
    public enum RESULT : int
    {
        OK,                        /* No errors. */
        ERR_BADCOMMAND,            /* Tried to call a function on a data type that does not allow this type of functionality (ie calling Sound::lock on a streaming sound). */

    }

    public class HandleBase
    {
        public HandleBase(IntPtr newPtr)
        {
            rawPtr = newPtr;
        }

        public bool isValid()
        {
            return rawPtr != IntPtr.Zero;
        }

        public IntPtr getRaw()
        {
            return rawPtr;
        }

        protected IntPtr rawPtr;

        #region equality

        public override bool Equals(Object obj)
        {
            return Equals(obj as HandleBase);
        }
        public bool Equals(HandleBase p)
        {
            // Equals if p not null and handle is the same
            return ((object)p != null && rawPtr == p.rawPtr);
        }
        public override int GetHashCode()
        {
            return rawPtr.ToInt32();
        }
        public static bool operator ==(HandleBase a, HandleBase b)
        {
            // If both are null, or both are same instance, return true.
            if (Object.ReferenceEquals(a, b))
            {
                return true;
            }
            // If one is null, but not both, return false.
            if (((object)a == null) || ((object)b == null))
            {
                return false;
            }
            // Return true if the handle matches
            return (a.rawPtr == b.rawPtr);
        }
        public static bool operator !=(HandleBase a, HandleBase b)
        {
            return !(a == b);
        }
        #endregion

    }



    class StringMarshalHelper
    {
        static internal void NativeToBuilder(StringBuilder builder, IntPtr nativeMem)
        {
            byte[] bytes = new byte[builder.Capacity];
            Marshal.Copy(nativeMem, bytes, 0, builder.Capacity);
			int strlen = Array.IndexOf(bytes, (byte)0);
			if (strlen > 0)
			{
				String str = Encoding.UTF8.GetString(bytes, 0, strlen);
				builder.Append(str);
			}
        }
    }

    public static class KNet
    {

        [DllImport(VERSION.dll)]
        public static extern void Net_Initialize(string dataPath, string persistentDataPath);
        [DllImport(VERSION.dll)]
        public static extern void Net_Uninitialize();

        [DllImport(VERSION.dll)]
        public static extern IntPtr Net_CreateSession(string session_type);
        [DllImport(VERSION.dll)]
        public static extern void Net_ReleaseSession(IntPtr handle);

        [DllImport(VERSION.dll)]
        public static extern int Net_Connect(IntPtr handle, string ip, ushort port, uint uuid);
        [DllImport(VERSION.dll)]
        public static extern void Net_CloseConnect(IntPtr handle);

        [DllImport(VERSION.dll)]
        public unsafe static extern void* Net_ReceiveCycle2(IntPtr handle, out int result);

        [DllImport(VERSION.dll)]
        public static extern int Net_Send(IntPtr handle, byte[] data, int length, int sendType);

        [DllImport(VERSION.dll)]
        public static extern int Net_GetConnectionState(int handle, ulong guid);

        [DllImport(VERSION.dll)]
        public static extern int Net_GetAveragePing(int handle, ulong guid);

        [DllImport(VERSION.dll)]
        public static extern UInt64 Net_GetTimeUs();

        //for fast MemCopy
        [DllImport(VERSION.dll)]
        public static extern void MemCopy(IntPtr dest, IntPtr src, int length);

        //for zip Compress
        [DllImport(VERSION.dll)]
        public unsafe static extern int Compress(byte* dest, ulong* destLen, byte* source, ulong sourceLen);

        [DllImport(VERSION.dll)]
        public unsafe static extern int UnCompress(byte* dest, ulong* destLen, byte* source, ulong sourceLen);

        //for xLog
        [DllImport(VERSION.dll)]
        public static extern void Net_xLog(int level, string content);

        [DllImport(VERSION.dll)]
        public static extern void Net_xLogFlush(int param);

        //2019/1/18: 增加网络事件通知，用于更新IPStack
        //        [DllImport(VERSION.dll)]
        //        public static extern int Net_NetEvent(int param);

    }
}
