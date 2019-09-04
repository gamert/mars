using System;
using System.Collections.Generic;
using System.Text;

namespace Network.Protocol
{

    public class MsgHead : AbstractSmartObj
    {
        public const int kFixedHeadSize = 20;
        private byte[] _data = new byte[kFixedHeadSize];    //报文固定为20B 头？

        private static int kSizeUShort = sizeof(ushort);
        private static int kSizeULong = sizeof(ulong);
        private static int kSizeInt = sizeof(int);
        private static int kSizeUInt = sizeof(uint);

        public ushort MsgId
        {
            get
            {
                return BitConverter.ToUInt16(_data, 0);
            }
            set
            {
                if (BitConverter.IsLittleEndian)
                {
                    for (int i = 0; i < kSizeUShort; i++)
                    {
                        _data[i] = (byte)(value >> i * 8);
                    }
                }
                else
                {
                    for (int i = 0; i < kSizeUShort; i++)
                    {
                        _data[kSizeUShort - 1 - i] = (byte)(value >> i * 8);
                    }
                }
            }
        }

        public ushort MsgLen
        {
            get
            {
                return BitConverter.ToUInt16(_data, 2);
            }
            set
            {
                if (BitConverter.IsLittleEndian)
                {
                    for (int i = 0; i < kSizeUShort; i++)
                    {
                        _data[2 + i] = (byte)(value >> i * 8);
                    }
                }
                else
                {
                    for (int i = 0; i < kSizeUShort; i++)
                    {
                        _data[2 + kSizeUShort - 1 - i] = (byte)(value >> i * 8);
                    }
                }
            }
        }

        public ulong Timestamp
        {
            get
            {
                return BitConverter.ToUInt64(_data, 4);
            }
            set
            {
                if (BitConverter.IsLittleEndian)
                {
                    for (int i = 0; i < kSizeULong; i++)
                    {
                        _data[4 + i] = (byte)(value >> i * 8);
                    }
                }
                else
                {
                    for (int i = 0; i < kSizeULong; i++)
                    {
                        _data[4 + kSizeULong - 1 - i] = (byte)(value >> i * 8);
                    }
                }
            }
        }

        public int Version
        {
            get
            {
                return BitConverter.ToInt32(_data, 12);
            }
            set
            {
                if (BitConverter.IsLittleEndian)
                {
                    for (int i = 0; i < kSizeInt; i++)
                    {
                        _data[12 + i] = (byte)(value >> i * 8);
                    }
                }
                else
                {
                    for (int i = 0; i < kSizeInt; i++)
                    {
                        _data[12 + kSizeInt - 1 - i] = (byte)(value >> i * 8);
                    }
                }
            }
        }

        public uint SvrPkgSeq
        {
            get
            {
                return BitConverter.ToUInt32(_data, 16);
            }
            set
            {
                if (BitConverter.IsLittleEndian)
                {
                    for (int i = 0; i < kSizeUInt; i++)
                    {
                        _data[16 + i] = (byte)(value >> i * 8);
                    }
                }
                else
                {
                    for (int i = 0; i < kSizeUInt; i++)
                    {
                        _data[16 + kSizeUInt - 1 - i] = (byte)(value >> i * 8);
                    }
                }
            }
        }

        public byte[] ToByteArray()
        {
            return _data;
        }

        public static MsgHead New(ushort msgId, ushort msgLen, ulong time, int ver = 0, uint seq = 0u)
        {
            MsgHead msgHead = TPoolClass<MsgHead>.AutoCreate();//new MsgHead();
            msgHead.MsgId = msgId;
            msgHead.MsgLen = msgLen;
            msgHead.Timestamp = time;
            msgHead.Version = ver;
            msgHead.SvrPkgSeq = seq;

            return msgHead;
        }

        public static MsgHead New(byte[] raw, int len = -1)
        {
            int size = len < 0 ? (raw != null ? raw.Length : 0) : len;
            MsgHead msgHead = TPoolClass<MsgHead>.AutoCreate();//new MsgHead();
            Buffer.BlockCopy(raw, 0, msgHead._data, 0, size);
            return msgHead;
        }

        public MsgHead() { }
        public override void OnRelease() { }
    }


    //PPacket:
    public class PPacket : AbstractSmartObj
    {
        public MsgHead oHead = null;
        public byte[] oBodyRaw = null;      //zz:使用原始数据来构造报文..

        public ushort MsgId
        {
            get
            {
                return oHead != null ? oHead.MsgId : (ushort)0;
            }
        }

        public ulong Timestamp
        {
            get
            {
                return oHead != null ? oHead.Timestamp : 0L;
            }
        }

        public int Version
        {
            get
            {
                return oHead != null ? oHead.Version : 0;
            }
        }

        public uint SvrPkgSeq
        {
            get
            {
                return oHead != null ? oHead.SvrPkgSeq : 0u;
            }
        }

        public const int kPacketCacheSize = 4096;
        public byte[] _buffer = new byte[kPacketCacheSize];    //临时使用。。。

        public const int kGlobalCacheSize = 20480;
        public static byte[] g_GloballySharedBuffer = new byte[kGlobalCacheSize];

        protected List<SharedBuffer> _sharedbufferList = new List<SharedBuffer>();

        public override void OnRelease()
        {
            oBodyRaw = null;
            if (oHead != null)
            {
                oHead.Release();
                oHead = null;
            }

            if (_sharedbufferList.Count > 0)
            {
                for (int ii = 0; ii < _sharedbufferList.Count; ++ii)
                    _sharedbufferList[ii].Release();
                _sharedbufferList.Clear();
            }
        }

        //

    }//end of PPackage
}
