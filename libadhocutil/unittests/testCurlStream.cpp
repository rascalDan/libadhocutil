#define BOOST_TEST_MODULE CurlStream
#include <boost/test/unit_test.hpp>

#include "curlStream.h"
#include "definedDirs.h"

BOOST_AUTO_TEST_CASE( fetch_file )
{
	auto url = "file://" + RootDir.string() + "/testCurlStream.cpp";
	CurlStreamSource css(url);
	boost::iostreams::stream<css_ref> curlstrm(boost::ref(css));
	std::string tok;
	curlstrm >> tok;
	BOOST_REQUIRE_EQUAL("#define", tok);
	curlstrm >> tok;
	BOOST_REQUIRE_EQUAL("BOOST_TEST_MODULE", tok);
	curlstrm >> tok;
	BOOST_REQUIRE_EQUAL("CurlStream", tok);
}

