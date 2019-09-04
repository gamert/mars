namespace Comm.Network
{
    public class NetworkStatistic
    {
        public int totalReceivedPacketNum = 0;
        public int totalSentPacketNum = 0;
        public ulong totalReceivedBytes = 0uL;
        public ulong totalSentBytes = 0uL;
        public float receivePPS = 0f;
        public float sentPPS = 0f;
    }

    public interface IPacketStatistic
    {
        void AddStatisticOfSentPacket(object msg, int len);
        void AddStatisticOfReceivedPacket(object msg, int len);
        void UpdateStat();
        void ResetStat();

        NetworkStatistic stat { get; }        
    }
}
