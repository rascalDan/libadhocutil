#define BOOST_TEST_MODULE Dirs
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <filesystem>

BOOST_AUTO_TEST_CASE( iexist )
{
	BOOST_REQUIRE(std::filesystem::exists(selfExe));
	BOOST_REQUIRE(selfExe.is_absolute());
	BOOST_REQUIRE_EQUAL("testDirs", selfExe.filename());
	BOOST_REQUIRE_EQUAL("unittests", rootDir.filename());
	BOOST_REQUIRE(std::filesystem::is_directory(binDir));
	BOOST_REQUIRE_EQUAL("libadhocutil", rootDir.parent_path().filename());
}

