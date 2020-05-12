#ifndef DBG_H
#define DBG_H

#ifdef _WIN32
#pragma once
#endif


inline void DevMsg(...) {}
inline void DevWarning(...) {}
inline void DevLog(...) {}
inline void ConMsg(...) {}
inline void ConLog(...) {}
inline void NetMsg(...) {}
inline void NetWarning(...) {}
inline void NetLog(...) {}

#define PlatLog ConLog

#endif