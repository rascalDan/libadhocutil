#ifndef ADHOCUTIL_DEFINEDDIRS_H
#define ADHOCUTIL_DEFINEDDIRS_H

#include <filesystem>

#ifndef ROOT
#	error "ROOT needs to be defined at compilation time"
#endif

#define XSTR(s) STR(s)
#define STR(s) #s
const std::filesystem::path selfExe = std::filesystem::canonical("/proc/self/exe");
const std::filesystem::path binDir = selfExe.parent_path();
const std::filesystem::path rootDir(XSTR(ROOT));

#endif
