
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using KNET;

namespace libknet_test
{
    class Program
    {
        const int TF_TYPE_PING = 127;
        const int TF_TYPE_PONG = 126;
        static private void printf(string v)
        {
        //    throw new NotImplementedException();
        }

        public static byte[] intToBytes(int value, byte[] src,int offset)
        {
            src[3+offset] = (byte)((value >> 24) & 0xFF);
            src[2 + offset] = (byte)((value >> 16) & 0xFF);
            src[1 + offset] = (byte)((value >> 8) & 0xFF);
            src[0 + offset] = (byte)(value & 0xFF);
            return src;
        }

        public static byte[] longToBytes(UInt64 value, byte[] src, int offset)
        {
            src[7 + offset] = (byte)((value >> 56) & 0xFF);
            src[6 + offset] = (byte)((value >> 48) & 0xFF);
            src[5 + offset] = (byte)((value >> 40) & 0xFF);
            src[4 + offset] = (byte)((value >> 32) & 0xFF);
            src[3 + offset] = (byte)((value >> 24) & 0xFF);
            src[2 + offset] = (byte)((value >> 16) & 0xFF);
            src[1 + offset] = (byte)((value >> 8) & 0xFF);
            src[0 + offset] = (byte)(value & 0xFF);
            return src;
        }

        static int ping_index = 1;
        static void send_ping(IntPtr handle, int index, bool bTcp = true)
        {
            //printf("send_ping \n");
            //在 windows 下 和 linux 下 取到的 时间精度 很不一样啊, windows下 居然 位数都不对, 比linux 下 少两位数
            UInt64 t2 = KNet._std_get_timeUs();// (DateTime.Now.Ticks - 621355968000000000)/100;//
            byte[] buf = new byte[128];
            buf[0] = TF_TYPE_PING;

            intToBytes(index, buf, 1);
            longToBytes(t2, buf, 5);

            KNet._std_send(handle, buf, 1+4+8, bTcp?0:1);
        }


        static void Main(string[] args)
        {
            //
            KNet._std_initialize("","");

            IntPtr handle = KNet._std_create_session("kcp");

            int res = KNet._std_connect(handle, "127.0.0.1", 9001, 11);
            byte[] data = new byte[128];

            //Thread.Sleep(1000);

          //  send_ping(handle, ping_index++);

            int index = 0;
            Random rr = new Random();
            while (true)
            {
                int length = (rr.Next() % 128) ;
                if(length > 0)
                {
                    //(ping_index % 2) == 0
                    //int bytes = KNet._std_send(handle, data, length, (index++)%2);
                    KNet._std_send_ping(handle, ping_index++, false);
                }
                Thread.Sleep(2000);
            }


            KNet._std_close_connect(handle);

            KNet._std_release_session(handle);

            KNet._std_uninitialize();

        }
    }
}
