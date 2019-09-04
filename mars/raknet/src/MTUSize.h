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

//EthernetII帧的结构DMAC+SMAC+Type+Data+CRC由于以太网传输电气方面的限制，每个以太网帧都有最小的大小64字节，最大不能超过1518字节
//由于以太网EthernetII最大的数据帧是1518字节，除去以太网帧的帧头（DMAC目的MAC地址48bit=6Bytes+SMAC源MAC地址48bit=6Bytes+Type域2bytes）14Bytes
//和帧尾CRC校验部分4Bytes那么剩下承载上层协议的地方也就是Data域最大就只能有1500字节这个值我们就把它称之为MTU。
//鉴于Internet上的标准MTU值为576字节，所以建议在进行Internet的UDP编程时，最好将UDP的数据长度控制在(576 - 8 - 20)548字节以内。

#define MAXIMUM_MTU_SIZE 576//

//ethernet head 12+2
//ip head : 20
//udp: 8
//all: 42 = 1494-1452


#define MINIMUM_MTU_SIZE 400

#endif
