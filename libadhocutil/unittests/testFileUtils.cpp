#define BOOST_TEST_MODULE FileUtils
#include <boost/test/unit_test.hpp>

#include <fileUtils.h>
#include <definedDirs.h>
#include <sys.h>

#define REQUIRE_INVALID_FH(fh) \
	BOOST_REQUIRE_EQUAL(fcntl(fh, F_GETFD), -1)

#define REQUIRE_VALID_FH(fh) \
	BOOST_REQUIRE_NE(fcntl(fh, F_GETFD), -1)

template <typename T>
void testRaw()
{
	int f = open("/proc/self/exe", O_RDONLY);
	BOOST_REQUIRE_NE(f, -1);
	T fh(f);
	BOOST_REQUIRE_EQUAL(f, fh);
}

BOOST_AUTO_TEST_CASE( raw )
{
	testRaw<AdHoc::FileUtils::FileHandle>();
	testRaw<AdHoc::FileUtils::FileHandleStat>();
	testRaw<AdHoc::FileUtils::MemMap>();
}

template <typename T, typename ... P>
T openfh(P ... p)
{
	T fh(rootDir / "testFileUtils.cpp", p...);
	return fh;
}

template <typename T, typename ... P>
T moveTest(P ... p)
{
	T fh = openfh<T>(p...);
	REQUIRE_VALID_FH(fh);
	char out;
	BOOST_REQUIRE_EQUAL(1, read(fh, &out, 1));
	return fh;
}

class X {
	public:
		X(AdHoc::FileUtils::FileHandle h) : fh(std::move(h)) { }
		AdHoc::FileUtils::FileHandle fh;
};

class Y : public X {
	public:
		Y(AdHoc::FileUtils::FileHandle h) : X(std::move(h)) { }
};

BOOST_AUTO_TEST_CASE( movePassThrough )
{
	auto f = openfh<AdHoc::FileUtils::FileHandle>();
	int ffd = f.fh;
	REQUIRE_VALID_FH(f.fh);
	auto y = std::make_unique<Y>(std::move(f));
	BOOST_REQUIRE_EQUAL(y->fh, ffd);
	REQUIRE_VALID_FH(y->fh);
	BOOST_REQUIRE_EQUAL(f.fh, -1);
	REQUIRE_INVALID_FH(f.fh);
	y.reset();
	REQUIRE_INVALID_FH(ffd);
}


BOOST_AUTO_TEST_CASE( moveFileHandle )
{
	auto f = moveTest<AdHoc::FileUtils::FileHandle>();
	REQUIRE_VALID_FH(f.fh);
	moveTest<AdHoc::FileUtils::FileHandle>(O_RDONLY);
	moveTest<AdHoc::FileUtils::FileHandle>(O_RDONLY, O_NONBLOCK);
}

BOOST_AUTO_TEST_CASE( moveFileHandleStat )
{
	auto f = moveTest<AdHoc::FileUtils::FileHandleStat>();
	REQUIRE_VALID_FH(f.fh);
	BOOST_REQUIRE_EQUAL(0100644, f.getStat().st_mode);
	moveTest<AdHoc::FileUtils::FileHandleStat>(O_RDONLY);
	moveTest<AdHoc::FileUtils::FileHandleStat>(O_RDONLY, O_NONBLOCK);
}

BOOST_AUTO_TEST_CASE( moveMemMap )
{
	auto f = moveTest<AdHoc::FileUtils::MemMap>();
	REQUIRE_VALID_FH(f.fh);
	BOOST_REQUIRE_EQUAL(0100644, f.getStat().st_mode);
	BOOST_REQUIRE_EQUAL(0, memcmp(f.data, "#define BOOST_TEST_MODULE FileUtils", 35));
	moveTest<AdHoc::FileUtils::MemMap>(O_RDONLY);
	moveTest<AdHoc::FileUtils::MemMap>(O_RDONLY, O_NONBLOCK);
}

BOOST_AUTO_TEST_CASE( memmap )
{
	AdHoc::FileUtils::MemMap f(rootDir / "testFileUtils.cpp");
	BOOST_REQUIRE(f.fh);
	BOOST_REQUIRE_EQUAL(f.getStat().st_mode, 0100644);
	BOOST_REQUIRE_EQUAL(0, memcmp(f.data, "#define BOOST_TEST_MODULE FileUtils", 35));
}

BOOST_AUTO_TEST_CASE( openmode )
{
	boost::filesystem::remove(binDir / "test.file");
	BOOST_REQUIRE_THROW({
		AdHoc::FileUtils::FileHandle fh(binDir / "test.file", O_RDONLY, S_IRWXU);
	}, AdHoc::SystemExceptionOn);
	AdHoc::FileUtils::FileHandle fh(binDir / "test.file", O_CREAT, S_IRWXU);
	boost::filesystem::remove(binDir / "test.file");
}

BOOST_AUTO_TEST_CASE( openfail )
{
	BOOST_REQUIRE_THROW({
		AdHoc::FileUtils::MemMap f("/tmp/nothere");
	}, AdHoc::SystemException);
}

BOOST_AUTO_TEST_CASE( mapfail )
{
	BOOST_REQUIRE_THROW({
		AdHoc::FileUtils::MemMap f("/dev/null");
	}, AdHoc::SystemException);
	auto fd = open("/dev/null", O_RDWR);
	REQUIRE_VALID_FH(fd);
	BOOST_REQUIRE_THROW({
		AdHoc::FileUtils::MemMap f(fd, O_RDWR);
	}, AdHoc::SystemException);
	REQUIRE_INVALID_FH(fd);
}

BOOST_AUTO_TEST_CASE( msg )
{
	AdHoc::SystemException x("verb", "No such file or directory", ENOENT);
	BOOST_REQUIRE_EQUAL(x.what(), "verb (2:No such file or directory)");
}

BOOST_AUTO_TEST_CASE( msgOn )
{
	AdHoc::SystemExceptionOn x("verb", "No such file or directory", ENOENT, "noun");
	BOOST_REQUIRE_EQUAL(x.what(), "verb on 'noun' (2:No such file or directory)");
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

