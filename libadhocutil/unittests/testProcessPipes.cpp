#define BOOST_TEST_MODULE ProcessPipes
#include <boost/test/unit_test.hpp>

#include "definedDirs.h"
#include "processPipes.h"
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

using namespace AdHoc::System;

BOOST_AUTO_TEST_CASE(readfind)
{
	ProcessPipes pp({"/usr/bin/find", rootDir, "-maxdepth", "1"}, false, true, true);
	BOOST_REQUIRE_EQUAL(pp.fdIn(), -1);
	BOOST_REQUIRE_NE(pp.fdOut(), -1);
	BOOST_REQUIRE_NE(pp.fdError(), -1);
	std::array<char, BUFSIZ> buf {};
	const auto bytes = read(pp.fdOut(), buf.data(), BUFSIZ);
	BOOST_REQUIRE_MESSAGE(bytes > 0, "bytes = " << bytes);
	std::string_view str {buf.data(), buf.data() + bytes};
	BOOST_TEST_CONTEXT(str) {
		const auto lnf = str.find("testProcessPipes.cpp");
		BOOST_REQUIRE_NE(lnf, std::string_view::npos);
	}
	int status;
	waitpid(pp.pid(), &status, 0);
}

BOOST_AUTO_TEST_CASE(readwrite)
{
	ProcessPipes pp({"/usr/bin/md5sum"}, true, true, false);
	BOOST_REQUIRE_NE(pp.fdIn(), -1);
	BOOST_REQUIRE_NE(pp.fdOut(), -1);
	BOOST_REQUIRE_EQUAL(pp.fdError(), -1);
	BOOST_REQUIRE_EQUAL(11, write(pp.fdIn(), "some string", 11));
	BOOST_REQUIRE_EQUAL(0, pp.closeIn());
	std::string out(32, ' ');
	BOOST_REQUIRE_EQUAL(32, read(pp.fdOut(), out.data(), 32));
	BOOST_CHECK_EQUAL(out, "5ac749fbeec93607fc28d666be85e73a");
	int status;
	waitpid(pp.pid(), &status, 0);
}

BOOST_AUTO_TEST_CASE(noargs)
{
	BOOST_CHECK_THROW({ ProcessPipes({}, false, false, false); }, std::invalid_argument);
}
