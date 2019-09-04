#include "RakNetPeer.h"


//#ifdef _WIN32
//#pragma comment(lib,"libRakNet_x64.lib")
//#endif

bool RakNetPeerUDP::s_initialized = false;
byte RakNetPeerUDP::s_data[512];
