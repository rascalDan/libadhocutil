#define BOOST_TEST_MODULE Nagios

#include <boost/test/unit_test.hpp>
#include <nagios.h>

/// LCOV_EXCL_START (calls real Nagios)
BOOST_AUTO_TEST_CASE(test_write_cmd, * boost::unit_test::disabled())
{
	BOOST_REQUIRE(AdHoc::submitNagiosPassiveServiceCheck("some service", AdHoc::NagiosStatusCode::OK, "bar"));
}
/// LCOV_EXCL_STOP

BOOST_AUTO_TEST_CASE(test_fmtrite_cmd)
{
	std::stringstream strm;
	BOOST_REQUIRE(AdHoc::submitNagiosPassiveServiceCheck(strm, "some service", AdHoc::NagiosStatusCode::OK, "bar"));
}

