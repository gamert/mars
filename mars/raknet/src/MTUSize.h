/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file
/// \brief \b [Internal] Defines the default maximum transfer unit.
///


#ifndef MAXIMUM_MTU_SIZE

/// \li \em 17914 16 Mbit/Sec Token Ring
/// \li \em 4464 4 Mbits/Sec Token Ring
/// \li \em 4352 FDDI
/// \li \em 1500. The largest Ethernet packet size \b recommended. This is the typical setting for non-PPPoE, non-VPN connections. The default value for NETGEAR routers, adapters and switches.
/// \li \em 1492. The size PPPoE prefers.
/// \li \em 1472. Maximum size to use for pinging. (Bigger packets are fragmented.)
/// \li \em 1468. The size DHCP prefers.
/// \li \em 1460. Usable by AOL if you don't have large email attachments, etc.
/// \li \em 1430. The size VPN and PPTP prefer.
/// \li \em 1400. Maximum size for AOL DSL.
/// \li \em 576. Typical value to connect to dial-up ISPs.
/// The largest value for an UDP datagram

//EthernetII֡�ĽṹDMAC+SMAC+Type+Data+CRC������̫�����������������ƣ�ÿ����̫��֡������С�Ĵ�С64�ֽڣ�����ܳ���1518�ֽ�
//������̫��EthernetII��������֡��1518�ֽڣ���ȥ��̫��֡��֡ͷ��DMACĿ��MAC��ַ48bit=6Bytes+SMACԴMAC��ַ48bit=6Bytes+Type��2bytes��14Bytes
//��֡βCRCУ�鲿��4Bytes��ôʣ�³����ϲ�Э��ĵط�Ҳ����Data������ֻ����1500�ֽ����ֵ���ǾͰ�����֮ΪMTU��
//����Internet�ϵı�׼MTUֵΪ576�ֽڣ����Խ����ڽ���Internet��UDP���ʱ����ý�UDP�����ݳ��ȿ�����(576 - 8 - 20)548�ֽ����ڡ�

#define MAXIMUM_MTU_SIZE 576//

//ethernet head 12+2
//ip head : 20
//udp: 8
//all: 42 = 1494-1452


#define MINIMUM_MTU_SIZE 400

#endif
