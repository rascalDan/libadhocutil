#define BOOST_TEST_MODULE Buffer
#include <boost/test/unit_test.hpp>

#include "buffer.h"
#include <boost/format.hpp>
#include <cstring>
#include <iosfwd>
#include <string>

using namespace AdHoc;

BOOST_AUTO_TEST_CASE(create)
{
	Buffer empty;
	Buffer copy("const", Buffer::Copy);
	auto nonconst = strdup("nonconst");
	Buffer use(nonconst, Buffer::Use);
	Buffer fre(nonconst, Buffer::Free);

	BOOST_REQUIRE(!empty);
	BOOST_REQUIRE_EQUAL(0, empty.length());
	BOOST_REQUIRE_EQUAL("", empty.str());

	BOOST_REQUIRE(copy);
	BOOST_REQUIRE_EQUAL(5, copy.length());
	BOOST_REQUIRE_EQUAL("const", copy.str());

	BOOST_REQUIRE_EQUAL(8, use.length());
	BOOST_REQUIRE_EQUAL("nonconst", use.str());

	BOOST_REQUIRE_EQUAL(8, fre.length());
	BOOST_REQUIRE_EQUAL("nonconst", fre.str());

	nonconst[0] = 'N';
	BOOST_REQUIRE_EQUAL("Nonconst", use.str());
	BOOST_REQUIRE_EQUAL("Nonconst", fre.str());
}

BOOST_AUTO_TEST_CASE(writestream)
{
	std::stringstream buf;
	Buffer b;
	b.append("Some text.").append("And then ").append("some more.");
	buf << b;
	BOOST_REQUIRE_EQUAL("Some text.And then some more.", buf.str());
}

BOOST_AUTO_TEST_CASE(appendempty)
{
	Buffer b;
	// These should not add content
	b.append("");
	BOOST_REQUIRE(!b);
	b.append(std::string());
	BOOST_REQUIRE(!b);
	b.appendf("%s", "");
	BOOST_REQUIRE(!b);
	b.appendbf("%s", "");
	BOOST_REQUIRE(!b);
}

BOOST_AUTO_TEST_CASE(appendthings)
{
	Buffer b;
	b.append("string a").append(std::string(" b")).appendf(" num %d", 1).appendbf(" num %d", 2);
	BOOST_REQUIRE_EQUAL(22, b.length());
	BOOST_REQUIRE_EQUAL("string a b num 1 num 2", b.str());
	const char * cstring = b;
	std::string_view sv {cstring};
	BOOST_REQUIRE_EQUAL(22, sv.length());
	BOOST_REQUIRE_EQUAL("string a b num 1 num 2", sv);
}

BOOST_AUTO_TEST_CASE(writeto)
{
	Buffer b;
	b.append("string a").append(std::string(" b")).appendf(" num %d", 1).appendbf(" num %d", 2);
	std::string buf(22, '\0');
	b.writeto(buf.data(), 23, 0);
	BOOST_REQUIRE_EQUAL(buf, "string a b num 1 num 2");
}

BOOST_AUTO_TEST_CASE(operators)
{
	auto expected = "cstringstd::string";
	Buffer a;
	Buffer b;
	a += "cstring";
	a += std::string("std::string");
	BOOST_REQUIRE_EQUAL(expected, a.str());
	b = a;
	BOOST_REQUIRE_EQUAL(expected, a.str());
	BOOST_REQUIRE_EQUAL(expected, b.str());

	Buffer c;
	c = expected;
	BOOST_REQUIRE_EQUAL(expected, c.str());

	Buffer d;
	d = std::string(expected);
	BOOST_REQUIRE_EQUAL(expected, d.str());
}

BOOST_AUTO_TEST_CASE(clear)
{
	Buffer b("some");
	BOOST_REQUIRE(b);
	b.clear();
	BOOST_REQUIRE(!b);
}

BOOST_AUTO_TEST_CASE(replacesstringbf)
{
	auto str = Buffer().appendbf("something %d", 1234).str();
	BOOST_REQUIRE_EQUAL("something 1234", str);

	std::string macrostringf = stringf("something %d", 1234);
	BOOST_REQUIRE_EQUAL("something 1234", macrostringf);
	std::string macrostringbf = stringbf("something %d", 1234);
	BOOST_REQUIRE_EQUAL("something 1234", macrostringbf);
}
