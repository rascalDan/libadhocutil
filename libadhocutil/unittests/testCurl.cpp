#define BOOST_TEST_MODULE Curl
#include <boost/test/unit_test.hpp>

#include "curlHandle.h"
#include "curlStream.h"
#include "definedDirs.h"
#include "net.h"

size_t discard(void *, size_t sz, size_t nm, void *)
{
	return sz * nm;
}

BOOST_AUTO_TEST_CASE( fetch_file )
{
	auto url = "file://" + RootDir.string() + "/testCurl.cpp";
	CurlHandle ch(url);
	ch.setopt(CURLOPT_WRITEFUNCTION, (void*)&discard);
	ch.perform();
}

BOOST_AUTO_TEST_CASE( fetch_missing )
{
	auto url = "file://" + RootDir.string() + "/nothere";
	CurlHandle ch(url);
	BOOST_REQUIRE_THROW(ch.perform(), AdHoc::Net::CurlException);
}

BOOST_AUTO_TEST_CASE( fetch_file_stream )
{
	auto url = "file://" + RootDir.string() + "/testCurl.cpp";
	CurlStreamSource css(url);
	boost::iostreams::stream<css_ref> curlstrm(boost::ref(css));
	std::string tok;
	curlstrm >> tok;
	BOOST_REQUIRE_EQUAL("#define", tok);
	curlstrm >> tok;
	BOOST_REQUIRE_EQUAL("BOOST_TEST_MODULE", tok);
	curlstrm >> tok;
	BOOST_REQUIRE_EQUAL("Curl", tok);
	while (!curlstrm.eof()) {
		curlstrm >> tok;
	}
}

BOOST_AUTO_TEST_CASE( fetch_missing_stream )
{
	auto url = "file://" + RootDir.string() + "/nothere";
	BOOST_REQUIRE_THROW({
		CurlStreamSource css(url);
		boost::iostreams::stream<css_ref> curlstrm(boost::ref(css));
		std::string tok;
		curlstrm >> tok;
	}, AdHoc::Net::CurlException);
}

