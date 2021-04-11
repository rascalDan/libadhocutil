#define BOOST_TEST_MODULE UriParse
#include <boost/test/unit_test.hpp>

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
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(!u.port);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(!u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(lowerScheme)
{
	AdHoc::Uri u("HtTP://localhost");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(!u.port);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(!u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simpleTrailingSlash)
{
	AdHoc::Uri u("ssh+git://localhost/");
	BOOST_REQUIRE_EQUAL("ssh+git", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(!u.port);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(u.path);
	BOOST_REQUIRE_EQUAL("", *u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simpleWithPort)
{
	AdHoc::Uri u("http://localhost:80");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(u.port);
	BOOST_REQUIRE_EQUAL(80, *u.port);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(!u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(simpleTrailingSlashWithPort)
{
	AdHoc::Uri u("http://localhost:80/");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(u.port);
	BOOST_REQUIRE_EQUAL(80, *u.port);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(u.path);
	BOOST_REQUIRE_EQUAL("", *u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(username)
{
	AdHoc::Uri u("http://user@localhost");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(u.username);
	BOOST_REQUIRE_EQUAL("user", *u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(!u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(usernameAndPassword)
{
	AdHoc::Uri u("http://user:pass@localhost");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(u.username);
	BOOST_REQUIRE_EQUAL("user", *u.username);
	BOOST_REQUIRE(u.password);
	BOOST_REQUIRE_EQUAL("pass", *u.password);
	BOOST_REQUIRE(!u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(path)
{
	AdHoc::Uri u("http://localhost/path/to/resource");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(u.path);
	BOOST_REQUIRE_EQUAL("path/to/resource", *u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(!u.fragment);
}

BOOST_AUTO_TEST_CASE(query0)
{
	AdHoc::Uri u("http://localhost/?");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(0, u.query.size());
}

BOOST_AUTO_TEST_CASE(query1)
{
	AdHoc::Uri u("http://localhost/?var=val");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(1, u.query.size());
	BOOST_REQUIRE_EQUAL("var", u.query.begin()->first);
	BOOST_REQUIRE_EQUAL("val", u.query.begin()->second);
}

BOOST_AUTO_TEST_CASE(query2)
{
	AdHoc::Uri u("http://localhost/?var=val&name=value");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(2, u.query.size());
	BOOST_REQUIRE_EQUAL("name", u.query.begin()->first);
	BOOST_REQUIRE_EQUAL("value", u.query.begin()->second);
	BOOST_REQUIRE_EQUAL("var", u.query.rbegin()->first);
	BOOST_REQUIRE_EQUAL("val", u.query.rbegin()->second);
}

BOOST_AUTO_TEST_CASE(queryMany)
{
	AdHoc::Uri u("http://localhost/?name=val&name=value");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(2, u.query.size());
	BOOST_REQUIRE_EQUAL("name", u.query.begin()->first);
	BOOST_REQUIRE_EQUAL("val", u.query.begin()->second);
	BOOST_REQUIRE_EQUAL("name", u.query.rbegin()->first);
	BOOST_REQUIRE_EQUAL("value", u.query.rbegin()->second);
}

BOOST_AUTO_TEST_CASE(queryNoValue1)
{
	AdHoc::Uri u("http://localhost/?n1");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(1, u.query.size());
	BOOST_REQUIRE_EQUAL("n1", u.query.begin()->first);
	BOOST_REQUIRE_EQUAL("", u.query.begin()->second);
}

BOOST_AUTO_TEST_CASE(queryNoValue1eq)
{
	AdHoc::Uri u("http://localhost/?n1=");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(1, u.query.size());
	BOOST_REQUIRE_EQUAL("n1", u.query.begin()->first);
	BOOST_REQUIRE_EQUAL("", u.query.begin()->second);
}

BOOST_AUTO_TEST_CASE(queryNoValue2)
{
	AdHoc::Uri u("http://localhost/?n1=&n2");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE_EQUAL(2, u.query.size());
	BOOST_REQUIRE_EQUAL("n1", u.query.begin()->first);
	BOOST_REQUIRE_EQUAL("", u.query.begin()->second);
	BOOST_REQUIRE_EQUAL("n2", u.query.rbegin()->first);
	BOOST_REQUIRE_EQUAL("", u.query.rbegin()->second);
}

BOOST_AUTO_TEST_CASE(fragment)
{
	AdHoc::Uri u("http://localhost/path/to/resource#someFrag");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("localhost", u.host);
	BOOST_REQUIRE(!u.username);
	BOOST_REQUIRE(!u.password);
	BOOST_REQUIRE(u.path);
	BOOST_REQUIRE_EQUAL("path/to/resource", *u.path);
	BOOST_REQUIRE(u.query.empty());
	BOOST_REQUIRE(u.fragment);
	BOOST_REQUIRE_EQUAL("someFrag", *u.fragment);
}

BOOST_AUTO_TEST_CASE(ipv6)
{
	AdHoc::Uri u("http://[FEDC:BA98:7654:3210:FEDC:BA98:7654:3210]:80/index.html");
	BOOST_REQUIRE_EQUAL("http", u.scheme);
	BOOST_REQUIRE_EQUAL("[fedc:ba98:7654:3210:fedc:ba98:7654:3210]", u.host);
	BOOST_REQUIRE(u.port);
	BOOST_REQUIRE_EQUAL(80, *u.port);
	BOOST_REQUIRE(u.path);
	BOOST_REQUIRE_EQUAL("index.html", *u.path);
}

BOOST_AUTO_TEST_CASE(bad)
{
	BOOST_REQUIRE_THROW(AdHoc::Uri(""), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("localhost"), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("t00+p://foo"), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("tcp:"), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("http:/"), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("tcp://"), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("ftp/local"), AdHoc::InvalidUri);
	BOOST_REQUIRE_THROW(AdHoc::Uri("tcp://local:"), std::bad_cast);
	BOOST_REQUIRE_THROW(AdHoc::Uri("tcp://local:foo"), std::bad_cast);
	BOOST_REQUIRE_THROW(AdHoc::Uri("tcp://user:pass@"), AdHoc::InvalidUri);

	AdHoc::InvalidUri ui("message", "http://localhost");
	BOOST_REQUIRE_EQUAL("InvalidUri (message) parsing [http://localhost]", ui.what());
}
