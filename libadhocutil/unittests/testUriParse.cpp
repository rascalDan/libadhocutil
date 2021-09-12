#define BOOST_TEST_MODULE UriParse
#include <boost/test/unit_test.hpp>

#include "boost-test-extra.h"
#include "uriParse.h"
#include <iterator>
#include <map>
#include <optional>
#include <string>
#include <typeinfo>
#include <utility>

BOOST_AUTO_TEST_CASE(simple)
{
	AdHoc::Uri u("http://localhost");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK(!u.port);
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK(!u.path);
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simple_ipv6)
{
	AdHoc::Uri u("http://[fe80::20e2:d5ff:fed7:c631]");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("[fe80::20e2:d5ff:fed7:c631]", u.host);
	BOOST_CHECK(!u.port);
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK(!u.path);
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(lowerScheme)
{
	AdHoc::Uri u("HtTP://localhost");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK(!u.port);
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK(!u.path);
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simpleTrailingSlash)
{
	AdHoc::Uri u("ssh+git://localhost/");
	BOOST_CHECK_EQUAL("ssh+git", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK(!u.port);
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK_IF(p, u.path) {
		BOOST_CHECK_EQUAL("", *u.path);
	}
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simpleWithPort)
{
	AdHoc::Uri u("http://localhost:80");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_IF(p, u.port) {
		BOOST_CHECK_EQUAL(80, *u.port);
	}
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK(!u.path);
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simpleTrailingSlashWithPort)
{
	AdHoc::Uri u("http://localhost:80/");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_IF(p, u.port) {
		BOOST_CHECK_EQUAL(80, *u.port);
	}
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK_IF(p, u.path) {
		BOOST_CHECK_EQUAL("", *u.path);
	}
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(username)
{
	AdHoc::Uri u("http://user@localhost");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_IF(p, u.username) {
		BOOST_CHECK_EQUAL("user", *u.username);
	}
	BOOST_CHECK(!u.password);
	BOOST_CHECK(!u.path);
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(usernameAndPassword)
{
	AdHoc::Uri u("http://user:pass@localhost");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_IF(p, u.username) {
		BOOST_CHECK_EQUAL("user", *u.username);
	}
	BOOST_CHECK_IF(p, u.password) {
		BOOST_CHECK_EQUAL("pass", *u.password);
	}
	BOOST_CHECK(!u.path);
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(path)
{
	AdHoc::Uri u("http://localhost/path/to/resource");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK_IF(p, u.path) {
		BOOST_CHECK_EQUAL("path/to/resource", *u.path);
	}
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK(!u.fragment);
}

BOOST_AUTO_TEST_CASE(query0)
{
	AdHoc::Uri u("http://localhost/?");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK(u.query.empty());
}

BOOST_AUTO_TEST_CASE(query1)
{
	AdHoc::Uri u("http://localhost/?var=val");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_EQUAL_IF(1, u.query.size()) {
		BOOST_CHECK_EQUAL("var", u.query.begin()->first);
		BOOST_CHECK_EQUAL("val", u.query.begin()->second);
	}
}

BOOST_AUTO_TEST_CASE(query2)
{
	AdHoc::Uri u("http://localhost/?var=val&name=value");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_EQUAL_IF(2, u.query.size()) {
		BOOST_CHECK_EQUAL("name", u.query.begin()->first);
		BOOST_CHECK_EQUAL("value", u.query.begin()->second);
		BOOST_CHECK_EQUAL("var", u.query.rbegin()->first);
		BOOST_CHECK_EQUAL("val", u.query.rbegin()->second);
	}
}

BOOST_AUTO_TEST_CASE(queryMany)
{
	AdHoc::Uri u("http://localhost/?name=val&name=value");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_EQUAL_IF(2, u.query.size()) {
		BOOST_CHECK_EQUAL("name", u.query.begin()->first);
		BOOST_CHECK_EQUAL("val", u.query.begin()->second);
		BOOST_CHECK_EQUAL("name", u.query.rbegin()->first);
		BOOST_CHECK_EQUAL("value", u.query.rbegin()->second);
	}
}

BOOST_AUTO_TEST_CASE(queryNoValue1)
{
	AdHoc::Uri u("http://localhost/?n1");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_EQUAL_IF(1, u.query.size()) {
		BOOST_CHECK_EQUAL("n1", u.query.begin()->first);
		BOOST_CHECK_EQUAL("", u.query.begin()->second);
	}
}

BOOST_AUTO_TEST_CASE(queryNoValue1eq)
{
	AdHoc::Uri u("http://localhost/?n1=");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_EQUAL_IF(1, u.query.size()) {
		BOOST_CHECK_EQUAL("n1", u.query.begin()->first);
		BOOST_CHECK_EQUAL("", u.query.begin()->second);
	}
}

BOOST_AUTO_TEST_CASE(queryNoValue2)
{
	AdHoc::Uri u("http://localhost/?n1=&n2");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK_EQUAL_IF(2, u.query.size()) {
		BOOST_CHECK_EQUAL("n1", u.query.begin()->first);
		BOOST_CHECK_EQUAL("", u.query.begin()->second);
		BOOST_CHECK_EQUAL("n2", u.query.rbegin()->first);
		BOOST_CHECK_EQUAL("", u.query.rbegin()->second);
	}
}

BOOST_AUTO_TEST_CASE(fragment)
{
	AdHoc::Uri u("http://localhost/path/to/resource#someFrag");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("localhost", u.host);
	BOOST_CHECK(!u.username);
	BOOST_CHECK(!u.password);
	BOOST_CHECK_IF(p, u.path) {
		BOOST_CHECK_EQUAL("path/to/resource", *u.path);
	}
	BOOST_CHECK(u.query.empty());
	BOOST_CHECK_IF(p, u.fragment) {
		BOOST_CHECK_EQUAL("someFrag", *u.fragment);
	}
}

BOOST_AUTO_TEST_CASE(ipv6)
{
	AdHoc::Uri u("http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html");
	BOOST_CHECK_EQUAL("http", u.scheme);
	BOOST_CHECK_EQUAL("[fedc:ba98:7654:3210:fedc:ba98:7654:3210]", u.host);
	BOOST_CHECK_IF(p, u.port) {
		BOOST_CHECK_EQUAL(80, *u.port);
	}
	BOOST_CHECK_IF(p, u.path) {
		BOOST_CHECK_EQUAL("index.html", *u.path);
	}
}

BOOST_AUTO_TEST_CASE(bad)
{
	BOOST_CHECK_THROW(AdHoc::Uri(""), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("localhost"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("t00+p://foo"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp:"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("http:/"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("ftp/local"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:foo"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://:"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp:///"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://[abcd"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:foo/"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:80a/"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:80a"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:-80"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:-1"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:65536"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://local:/"), AdHoc::InvalidUri);
	BOOST_CHECK_THROW(AdHoc::Uri("tcp://user:pass@"), AdHoc::InvalidUri);

	AdHoc::InvalidUri ui("message", "http://localhost");
	BOOST_CHECK_EQUAL("InvalidUri (message) parsing [http://localhost]", ui.what());
}
