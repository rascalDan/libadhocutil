#ifndef P2_UT_DEFINEDDIRS
#define P2_UT_DEFINEDDIRS

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#ifndef ROOT
#error "ROOT needs to be defined at compilation time"
#endif

#define XSTR(s) STR(s)
#define STR(s) #s
const auto BinDir = boost::filesystem::canonical("/proc/self/exe").parent_path();
const boost::filesystem::path RootDir(XSTR(ROOT));

#endif

