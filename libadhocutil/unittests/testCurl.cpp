#define BOOST_TEST_MODULE Curl
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include "curlHandle.h"
#include "curlMultiHandle.h"
#include "curlStream.h"
#include "definedDirs.h"
#include "net.h"

size_t discard(void *, size_t sz, size_t nm, void *)
{
	return sz * nm;
}

BOOST_AUTO_TEST_CASE( fetch_file )
{
	auto url = "file://" + rootDir.string() + "/testCurl.cpp";
	CurlHandle ch(url);
	ch.setopt(CURLOPT_WRITEFUNCTION, (void*)&discard);
	ch.perform();
}

BOOST_AUTO_TEST_CASE( fetch_missing )
{
	auto url = "file://" + rootDir.string() + "/nothere";
	CurlHandle ch(url);
	BOOST_REQUIRE_THROW(ch.perform(), AdHoc::Net::CurlException);
}

BOOST_AUTO_TEST_CASE( fetch_file_stream )
{
	auto url = "file://" + rootDir.string() + "/testCurl.cpp";
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
	auto url = "file://" + rootDir.string() + "/nothere";
	BOOST_REQUIRE_THROW({
		CurlStreamSource css(url);
		boost::iostreams::stream<css_ref> curlstrm(boost::ref(css));
		std::string tok;
		curlstrm >> tok;
	}, AdHoc::Net::CurlException);
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
	cmh.addCurl("http://sys.randomdan.homeip.net/missing", [&finished, &errored](auto & s) {
			try {
				std::string tok;
				while (!s.eof()) {
					s >> tok;
				}
				finished = true;
			} catch (...) {
				errored = true;
			}
		});
	cmh.performAll();
	BOOST_REQUIRE(!finished);
	BOOST_REQUIRE(errored);
}

