#define BOOST_TEST_MODULE Curl
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include "curlHandle.h"
#include "curlMultiHandle.h"
#include "curlStream.h"
#include "definedDirs.h"
#include "net.h"
#include <boost/algorithm/string/predicate.hpp>

using namespace AdHoc::Net;

static
size_t
discard(void *, size_t sz, size_t nm, void *)
{
	return sz * nm;
}

BOOST_AUTO_TEST_CASE( fetch_file )
{
	auto url = "file://" + rootDir.string() + "/testCurl.cpp";
	CurlHandle ch(url);
	ch.setopt(CURLOPT_WRITEFUNCTION, discard);
	ch.perform();
}

BOOST_AUTO_TEST_CASE( setAndGetOptions )
{
	auto url = "file://" + rootDir.string() + "/testCurl.cpp";
	CurlHandle ch(url);
	// function
	ch.setopt(CURLOPT_WRITEFUNCTION, discard);
	// object
	ch.setopt(CURLOPT_WRITEDATA, this);
	// int
	ch.setopt(CURLOPT_LOCALPORT, 8000);
	// long
	ch.setopt(CURLOPT_LOCALPORT, 8000L);
	ch.perform();
	// char *
	char * eurl;
	ch.getinfo(CURLINFO_EFFECTIVE_URL, eurl);
	BOOST_REQUIRE_EQUAL(url, eurl);
	// double
	double totalTime;
	ch.getinfo(CURLINFO_TOTAL_TIME, totalTime);
	BOOST_REQUIRE_GT(totalTime, 0);
	BOOST_REQUIRE_LT(totalTime, 50);
}

BOOST_AUTO_TEST_CASE( fetch_missing )
{
	auto url = "file://" + rootDir.string() + "/nothere";
	CurlHandle ch(url);
	BOOST_REQUIRE_THROW(ch.perform(), AdHoc::Net::CurlException);
}

BOOST_AUTO_TEST_CASE( fetch_http_stream )
{
	CurlStreamSource css("http://sys.randomdan.homeip.net/env.cgi");
	css.appendHeader("X-POWERED-BY: mature-cheddar");
	CurlStream curlstrm(css);
	std::string tok;
	int expected = 0;
	while (!curlstrm.eof()) {
		curlstrm >> tok;
		if (boost::algorithm::starts_with(tok, "HTTP_X_POWERED_BY=")) {
			expected += 1;
			BOOST_REQUIRE_EQUAL("HTTP_X_POWERED_BY=mature-cheddar", tok);
		}
	}
	BOOST_REQUIRE_EQUAL(1, expected);
}

BOOST_AUTO_TEST_CASE( fetch_file_stream )
{
	auto url = "file://" + rootDir.string() + "/testCurl.cpp";
	CurlStreamSource css(url);
	CurlStream curlstrm(css);
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
	auto url = "file://" + rootDir.string() + "/nothere";
	CurlStreamSource css(url);
	css.setopt(CURLOPT_FAILONERROR, 1L);
	CurlStream curlstrm(css);
	std::string tok;
	curlstrm >> tok;
	BOOST_REQUIRE(!curlstrm.good());
}

static
void
mapFileToName(std::map<std::string, std::string> & map, const std::string & file, std::istream & curlstrm)
{
	std::string tok;
	curlstrm >> tok; // #define
	curlstrm >> tok; // BOOST_TEST_MODULE
	curlstrm >> tok; // name :)
	map[file] = tok;
}

BOOST_AUTO_TEST_CASE( fetch_multi )
{
	CurlMultiHandle cmh;
	std::map<std::string, std::string> files;
	cmh.addCurl("file://" + rootDir.string() + "/testBuffer.cpp",
			boost::bind(&mapFileToName, boost::ref(files), "testBuffer.cpp", _1));
	cmh.addCurl("file://" + rootDir.string() + "/testCurl.cpp",
			boost::bind(&mapFileToName, boost::ref(files), "testCurl.cpp", _1));
	cmh.addCurl("file://" + rootDir.string() + "/testLocks.cpp",
			boost::bind(&mapFileToName, boost::ref(files), "testLocks.cpp", _1));
	cmh.performAll();
	BOOST_REQUIRE_EQUAL(3, files.size());
	BOOST_REQUIRE_EQUAL("Locks", files["testLocks.cpp"]);
	BOOST_REQUIRE_EQUAL("Buffer", files["testBuffer.cpp"]);
	BOOST_REQUIRE_EQUAL("Curl", files["testCurl.cpp"]);
}

BOOST_AUTO_TEST_CASE( fetch_multi_fail )
{
	CurlMultiHandle cmh;
	bool errored = false;
	bool finished = false;
	cmh.addCurl("http://sys.randomdan.homeip.net/missing", [&finished, &errored](std::istream & s) {
		std::string tok;
		while (!s.eof()) {
			if (!s.good()) {
				errored = true;
				return;
			}
			s >> tok;
		}
		finished = true;
	});
	cmh.performAll();
	BOOST_REQUIRE(!finished);
	BOOST_REQUIRE(errored);
}

