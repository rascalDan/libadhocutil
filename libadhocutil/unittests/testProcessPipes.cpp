#define BOOST_TEST_MODULE ProcessPipes
#include <boost/test/unit_test.hpp>

#include "processPipes.h"
#include "definedDirs.h"
#include <sys/wait.h>

using namespace AdHoc::System;

BOOST_AUTO_TEST_CASE ( readfind )
{
	ProcessPipes pp({"/usr/bin/find", rootDir.string(), "-maxdepth", "1"}, false, true, true);
	BOOST_REQUIRE_EQUAL(pp.fdIn(), -1);
	BOOST_REQUIRE_NE(pp.fdOut(), -1);
	BOOST_REQUIRE_NE(pp.fdError(), -1);
	char buf[BUFSIZ];
	ssize_t bytes = read(pp.fdOut(), buf, BUFSIZ);
	BOOST_REQUIRE_MESSAGE(bytes > 0, "bytes = " << bytes);
	buf[bytes] = '\0';
	char * lnf = strstr(buf, "testProcessPipes.cpp");
	BOOST_REQUIRE_MESSAGE(lnf, buf);
	int status;
	waitpid(pp.pid(), &status, 0);
}

