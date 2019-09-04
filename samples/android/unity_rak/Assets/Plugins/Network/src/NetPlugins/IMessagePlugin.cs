//using Comm.Network;
using System;

namespace Comm.Network
{
    //zzAdd: for 
    public class NByteBuffer : FlatBuffers.ByteBuffer
    {
        const int LENGHT = 64*1024;
        protected int _posWrite;  // 有效数据长度..
        //默认支持长度
        public NByteBuffer():base(new byte[LENGHT])
        {
            _posWrite = 0; //
        }
        public int dataLen() { return _posWrite - _pos; }

        //附加一个新报文...
        public void Append(byte[] data, int offset, int length)
        {
            EnsureSpace(length);
            Buffer.BlockCopy(data, offset, _buffer, _posWrite, length);
            _posWrite += length;
        }
        //fast set:
        public void HookSet(byte[] data, int offset, int length)
        {
            _buffer = data;
            _pos = offset;
            _posWrite = _pos+length;
        }
        //全部清空:
        public void Purge()
        {
            _pos = _posWrite = 0;
        }
        //扩展空间: 1. 首先要讲offset归0; 因之前的报文已经无效；2.再按需要扩展空间
        public void EnsureSpace(int size)
        {
            int nn = dataLen();
            int space = _buffer.Length - nn; //绝对可写入空间（需要考虑移位）
            if (space < size)
            {
                //必须扩展空间..
                byte[] data = new byte[_buffer.Length + (size - space) + 4096]; //
                if (nn > 0)
                {
                    Buffer.BlockCopy(_buffer, _pos, data, 0, nn);
                }
                _buffer = data; //转移...
            }
            else
            {
                if (nn > 0 && _pos > 0)
                {
                    Buffer.BlockCopy(_buffer, _pos, _buffer, 0, nn);
                }
            }
            if (_pos > 0)
            {
                _posWrite -= _pos;
                if(_posWrite < 0)
                {
                    BaseConnector.Log("ERROR: EnsureSpace: _posWrite = "+ _posWrite);
                }
                _pos = 0;
            }
        }
    }


    public interface IMessagePlugin
    {
        BaseConnector conn { set; }
//        bool Process(ref byte[] data, int length, int cmd = 0);
        bool Process(NByteBuffer data, int cmd = 0);
        bool ProcessBeforeSent(object msg, ref byte[] data, out int usedSz);
    }

    public class MessagePluginBase : IMessagePlugin
    {
        protected BaseConnector _conn = null;
        public BaseConnector conn { set { _conn = value; } }
        //
        virtual public bool Process(NByteBuffer data, int cmd = 0) { return false; }
        virtual public bool ProcessBeforeSent(object msg, ref byte[] data, out int usedSz)
        {
            usedSz = 0;
            return false;
        }
    }
}