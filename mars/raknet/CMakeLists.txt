cmake_minimum_required (VERSION 3.6)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE PATH "Installation directory" FORCE)
message(STATUS "CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}")


project (raknet)

include(../comm/CMakeUtils.txt)
include(../comm/CMakeExtraFlags.txt)

include_directories(.)
include_directories(src)
include_directories(..)
include_directories(../..)
include_directories(../comm)
include_directories(../../..)


list(APPEND SELF_TEMP_SRC_FILES
	src/Base64Encoder.cpp
    src/BitStream.cpp
    src/CCRakNetSlidingWindow.cpp
    src/CCRakNetUDT.cpp
    src/CheckSum.cpp
    src/CloudClient.cpp
    src/CloudCommon.cpp
    src/CloudServer.cpp
    src/CommandParserInterface.cpp
    src/ConnectionGraph2.cpp
    src/ConsoleServer.cpp
    src/DataCompressor.cpp
    src/DirectoryDeltaTransfer.cpp
    src/DR_SHA1.cpp
    src/DS_BytePool.cpp
    src/DS_ByteQueue.cpp
    src/DS_HuffmanEncodingTree.cpp
    src/DS_Table.cpp
    src/DynDNS.cpp
    src/EmailSender.cpp
    src/EpochTimeToString.cpp
    src/FileList.cpp
    src/FileListTransfer.cpp
    src/FileOperations.cpp
    src/FormatString.cpp
    src/FullyConnectedMesh2.cpp
    src/Getche.cpp
    src/Gets.cpp
    src/GetTime.cpp
    src/gettimeofday.cpp
    src/GridSectorizer.cpp
    src/HTTPConnection.cpp
    src/HTTPConnection2.cpp
    src/IncrementalReadInterface.cpp
    src/Itoa.cpp
    src/LinuxStrings.cpp
    src/LocklessTypes.cpp
    src/LogCommandParser.cpp
    src/MessageFilter.cpp
    src/NatPunchthroughClient.cpp
    src/NatPunchthroughServer.cpp
    src/NatTypeDetectionClient.cpp
    src/NatTypeDetectionCommon.cpp
    src/NatTypeDetectionServer.cpp
    src/NetConnector.cpp
    src/NetworkIDManager.cpp
    src/NetworkIDObject.cpp
    src/PacketConsoleLogger.cpp
    src/PacketFileLogger.cpp
    src/PacketizedTCP.cpp
    src/PacketLogger.cpp
    src/PacketOutputWindowLogger.cpp
    src/PluginInterface2.cpp
    src/PS4Includes.cpp
    src/Rackspace.cpp
    src/RakMemoryOverride.cpp
    src/RakNetCommandParser.cpp
    src/RakNetConnector.cpp
    src/RakNetSocket.cpp
    src/RakNetSocket2.cpp
    src/RakNetSocket2_360_720.cpp
    src/RakNetSocket2_Berkley.cpp
    src/RakNetSocket2_Berkley_NativeClient.cpp
    src/RakNetSocket2_NativeClient.cpp
    src/RakNetSocket2_PS3_PS4.cpp
    src/RakNetSocket2_PS4.cpp
    src/RakNetSocket2_Vita.cpp
    src/RakNetSocket2_WindowsStore8.cpp
    src/RakNetSocket2_Windows_Linux.cpp
    src/RakNetSocket2_Windows_Linux_360.cpp
    src/RakNetStatistics.cpp
    src/RakNetTransport2.cpp
    src/RakNetTypes.cpp
    src/RakPeer.cpp
    src/RakSleep.cpp
    src/RakString.cpp
    src/RakThread.cpp
    src/RakWString.cpp
    src/Rand.cpp
    src/RandSync.cpp
    src/ReadyEvent.cpp
    src/RelayPlugin.cpp
    src/ReliabilityLayer.cpp
    src/ReplicaManager3.cpp
    src/Router2.cpp
    src/RPC4Plugin.cpp
    src/SecureHandshake.cpp
    src/SendToThread.cpp
    src/SignaledEvent.cpp
    src/SimpleMutex.cpp
    src/SocketLayer.cpp
    src/StatisticsHistory.cpp
    src/StringCompressor.cpp
    src/StringTable.cpp
    src/SuperFastHash.cpp
    src/TableSerializer.cpp
    src/TCPInterface.cpp
    src/TeamBalancer.cpp
    src/TeamManager.cpp
    src/TelnetTransport.cpp
    src/ThreadsafePacketLogger.cpp
    src/TwoWayAuthentication.cpp
    src/UDPForwarder.cpp
    src/UDPProxyClient.cpp
    src/UDPProxyCoordinator.cpp
    src/UDPProxyServer.cpp
    src/VariableDeltaSerializer.cpp
    src/VariableListDeltaTracker.cpp
    src/VariadicSQLParser.cpp
    src/VitaIncludes.cpp
    src/WSAStartupSingleton.cpp
    src/_FindFirst.cpp 
     )
	 
###file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} src/*.cpp src/*.h)
source_group(src FILES ${SELF_TEMP_SRC_FILES})
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

file(GLOB SELF_TEMP_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} *.cpp *.h)
list(APPEND SELF_SRC_FILES ${SELF_TEMP_SRC_FILES})

        

if(MSVC)
    add_definitions(/FI"../../comm/projdef.h")
    include_directories(../comm/windows)

elseif(ANDROID)
    file(GLOB SELF_ANDROID_SRC_FILES RELATIVE ${PROJECT_SOURCE_DIR} jni/*.cc)
        
    list(APPEND SELF_SRC_FILES ${SELF_ANDROID_SRC_FILES})
endif()

add_library(${PROJECT_NAME} STATIC ${SELF_SRC_FILES})

install(TARGETS ${PROJECT_NAME} ARCHIVE DESTINATION ${CMAKE_SYSTEM_NAME}.out)
