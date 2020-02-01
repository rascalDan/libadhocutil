#define BOOST_TEST_MODULE Nagios

#include <boost/test/unit_test.hpp>
#include <nagios.h>

BOOST_AUTO_TEST_CASE(test_write_cmd, * boost::unit_test::disabled())
{
	BOOST_REQUIRE(AdHoc::submitNagiosPassiveServiceCheck("some service", AdHoc::NagiosStatusCode::OK, "bar"));
}

BOOST_AUTO_TEST_CASE(test_fmtrite_cmd)
{
	std::stringstream strm;
	BOOST_REQUIRE(AdHoc::submitNagiosPassiveServiceCheck(strm, "some service", AdHoc::NagiosStatusCode::OK, "bar"));
}

