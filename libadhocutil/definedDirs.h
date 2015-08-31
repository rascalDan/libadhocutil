#ifndef ADHOCUTIL_DEFINEDDIRS_H
#define ADHOCUTIL_DEFINEDDIRS_H

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef ROOT
#error "ROOT needs to be defined at compilation time"
#endif

#define XSTR(s) STR(s)
#define STR(s) #s
const auto selfExe = boost::filesystem::canonical("/proc/self/exe");
const auto binDir = selfExe.parent_path();
const boost::filesystem::path rootDir(XSTR(ROOT));

#endif

