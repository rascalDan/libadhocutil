#define BOOST_TEST_MODULE FileUtils
#include <boost/test/unit_test.hpp>

#include <fileUtils.h>
#include <definedDirs.h>

BOOST_AUTO_TEST_CASE( memmap )
{
	AdHoc::FileUtils::MemMap f(rootDir / "testFileUtils.cpp");
	BOOST_REQUIRE(f.fh);
	BOOST_REQUIRE_EQUAL(f.getStat().st_mode, 0100644);
	BOOST_REQUIRE_EQUAL(0, memcmp(f.data, "#define BOOST_TEST_MODULE FileUtils", 35));
}

BOOST_AUTO_TEST_CASE( openfail )
{
	BOOST_REQUIRE_THROW({
		AdHoc::FileUtils::MemMap f("/tmp/nothere");
	}, std::runtime_error);
}

BOOST_AUTO_TEST_CASE( mapfail )
{
	BOOST_REQUIRE_THROW({
		AdHoc::FileUtils::MemMap f("/dev/null");
	}, std::runtime_error);
}

BOOST_AUTO_TEST_CASE( pathPart )
{
	using namespace AdHoc::FileUtils;
	boost::filesystem::path p("/this/is/some/path");
	BOOST_REQUIRE_EQUAL(p / 0, "/");
	BOOST_REQUIRE_EQUAL(p / 1, "this");
	BOOST_REQUIRE_EQUAL(p / 2, "is");
	BOOST_REQUIRE_EQUAL(p / 3, "some");
	BOOST_REQUIRE_EQUAL(p / 4, "path");
	BOOST_REQUIRE_EQUAL(p / 5, "");
}
