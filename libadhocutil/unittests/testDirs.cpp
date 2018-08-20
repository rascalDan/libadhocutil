#define BOOST_TEST_MODULE Dirs
#include <boost/test/unit_test.hpp>

#include <definedDirs.h>
#include <boost/filesystem/convenience.hpp>

BOOST_AUTO_TEST_CASE( iexist )
{
	BOOST_REQUIRE(boost::filesystem::exists(selfExe));
	BOOST_REQUIRE(selfExe.is_absolute());
	BOOST_REQUIRE_EQUAL("testDirs", selfExe.leaf().string());
	BOOST_REQUIRE_EQUAL("unittests", rootDir.leaf().string());
	BOOST_REQUIRE(boost::filesystem::is_directory(binDir));
	BOOST_REQUIRE_EQUAL("libadhocutil", rootDir.parent_path().leaf().string());
}

