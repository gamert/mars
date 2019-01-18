/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "EmptyHeader.h"
#if (defined(__GNUC__)  || defined(__GCCXML__)) && !defined(__WIN32__)
#include <netdb.h>
#else
#include "comm/jni/ifaddrs.h"
#endif

#ifdef RAKNET_SOCKET_2_INLINE_FUNCTIONS

#ifndef RAKNETSOCKET2_WINDOWS_LINUX_CPP
#define RAKNETSOCKET2_WINDOWS_LINUX_CPP

#if !defined(WINDOWS_STORE_RT) && !defined(__native_client__)

#if RAKNET_SUPPORT_IPV6==1

void PrepareAddrInfoHints2(addrinfo *hints)
{
	memset(hints, 0, sizeof (addrinfo)); // make sure the struct is empty
	hints->ai_socktype = SOCK_DGRAM; // UDP sockets
	hints->ai_flags = AI_PASSIVE;     // fill in my IP for me
}



//在ios下,
void GetMyIP_Windows_Linux_IPV4And6( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] )
{
   	int idx=0;
    
	char ac[ 80 ];
	int err = gethostname( ac, sizeof( ac ) );
	RakAssert(err != -1);
	
	struct addrinfo hints;
	struct addrinfo *servinfo=0, *aip;  // will point to the results
	PrepareAddrInfoHints2(&hints);
    
    
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = getifaddrs(&interfaces);
    
    if (success == 0) {  // 0 表示获取成功
        
        idx=0;
        temp_addr = interfaces;
        while (temp_addr != NULL && idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS)
        {
           // NSLog(@"ifa_name===%@",[NSString stringWithUTF8String:temp_addr->ifa_name]);
            // Check if interface is en0 which is the wifi connection on the iPhone
            if (strcmp(temp_addr->ifa_name,"en0") == 0 || strcmp(temp_addr->ifa_name,"pdp_ip0")== 0)
            {
                struct sockaddr	*ai_addr = temp_addr->ifa_addr;
                //如果是IPV4地址，直接转化
                if (ai_addr->sa_family == AF_INET){
                    struct sockaddr_in *ipv4 = (struct sockaddr_in *)ai_addr;
                    memcpy(&addresses[idx].address.addr4,ipv4,sizeof(sockaddr_in));
                    // Get NSString from C String
                    //address = [self formatIPV4Address:((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr];
                    idx++;
                }
                
                //如果是IPV6地址
                else if (ai_addr->sa_family == AF_INET6){
                    struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)ai_addr;
                    memcpy(&addresses[idx].address.addr4,ipv6,sizeof(sockaddr_in6));
                    idx++;
                    
//                    address = [self formatIPV6Address:((struct sockaddr_in6 *)ai_addr)->sin6_addr];
//                    if (address && ![address isEqualToString:@""] && ![address.uppercaseString hasPrefix:@"FE80"]) break;
                }
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    
    freeifaddrs(interfaces);
    
    
    
//	getaddrinfo(ac, "",&hints , &servinfo);//&hints
//
//	for (idx=0, aip = servinfo; aip != NULL && idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS; aip = aip->ai_next, idx++)
//	{
//		if (aip->ai_family == AF_INET)
//		{
//			struct sockaddr_in *ipv4 = (struct sockaddr_in *)aip->ai_addr;
//			memcpy(&addresses[idx].address.addr4,ipv4,sizeof(sockaddr_in));
//		}
//		else
//		{
//			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)aip->ai_addr;
//			memcpy(&addresses[idx].address.addr4,ipv6,sizeof(sockaddr_in6));
//		}
//
//	}
//
//	freeaddrinfo(servinfo); // free the linked-list

	while (idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS)
	{
		addresses[idx]=UNASSIGNED_SYSTEM_ADDRESS;
		idx++;
	}
}

#else


void GetMyIP_Windows_Linux_IPV4( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] )
{



	int idx=0;
	char ac[ 80 ];
	int err = gethostname( ac, sizeof( ac ) );
    (void) err;
	RakAssert(err != -1);
	
    struct hostent *phe = NULL;//gethostbyname( ac );//IOS: 向DNS查询一个域名的IP地址 //getifaddrs + getnameinfo 可以枚举出所有接口的ip
#if  defined(__LINUX__) || defined(_WIN32)
    phe = gethostbyname( ac );
#else
    phe = gethostent();
#endif
	if ( phe == 0 )
	{
		RakAssert(phe!=0);
		return ;
	}
	for ( idx = 0; idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS; ++idx )
	{
		if (phe->h_addr_list[ idx ] == 0)
			break;

		memcpy(&addresses[idx].address.addr4.sin_addr,phe->h_addr_list[ idx ],sizeof(struct in_addr));
	}
	
	while (idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS)
	{
		addresses[idx]=UNASSIGNED_SYSTEM_ADDRESS;
		idx++;
	}

}

#endif // RAKNET_SUPPORT_IPV6==1


void GetMyIP_Windows_Linux( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] )
{
	#if RAKNET_SUPPORT_IPV6==1
		GetMyIP_Windows_Linux_IPV4And6(addresses);
	#else
		GetMyIP_Windows_Linux_IPV4(addresses);
	#endif
}


#endif // Windows and Linux

#endif // file header

#endif // #ifdef RAKNET_SOCKET_2_INLINE_FUNCTIONS
