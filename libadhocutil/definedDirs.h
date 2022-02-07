#pragma once

#include <filesystem>

inline const std::filesystem::path selfExe = std::filesystem::canonical("/proc/self/exe");
inline const std::filesystem::path binDir = selfExe.parent_path();
#ifdef ROOT
#	define XSTR(s) STR(s)
#	define STR(s) #    s
inline const std::filesystem::path rootDir(XSTR(ROOT));
#	undef XSTR
#	undef STR
#endif
