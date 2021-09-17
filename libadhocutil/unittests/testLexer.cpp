#define BOOST_TEST_MODULE Lexer
#include <boost/test/unit_test.hpp>

#include <functional>
#include <lexer-regex.h>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"
#ifndef __clang__
#	pragma GCC diagnostic ignored "-Wuseless-cast"
#endif
#include <glib.h>
#include <glibmm/ustring.h>
#pragma GCC diagnostic pop

using namespace AdHoc;
using namespace AdHoc::LexerMatchers;

BOOST_AUTO_TEST_CASE(defaultConstructor)
{
	AdHoc::Lexer l;
	l.rules.push_back({{AdHoc::Lexer::InitialState}, regex("a"), [](auto) {}});
}

BOOST_AUTO_TEST_CASE(simple)
{
	int m = 0;
	AdHoc::Lexer l({{{AdHoc::Lexer::InitialState}, regex("a"), [&](auto) {
						 m += 1;
					 }}});
	BOOST_REQUIRE_EQUAL(0, m);
	l.extract("aaaa", 4);
	BOOST_REQUIRE_EQUAL(4, m);
	BOOST_REQUIRE_THROW({ l.extract("abcd", 4); }, std::runtime_error);
}

BOOST_AUTO_TEST_CASE(state)
{
	int m = 0;
	std::string s;
	AdHoc::Lexer l({{{AdHoc::Lexer::InitialState}, regex("a"),
							[&](auto es) {
								m += 1;
								BOOST_REQUIRE_EQUAL(1, es->depth());
								es->pushState("2");
								BOOST_REQUIRE_EQUAL(2, es->depth());
							}},
			{{"2"}, regex("a"),
					[&](auto es) {
						m += 2;
						BOOST_REQUIRE_EQUAL("2", es->getState());
						BOOST_REQUIRE_EQUAL(2, es->depth());
						es->pushState("3");
						BOOST_REQUIRE_EQUAL("3", es->getState());
						BOOST_REQUIRE_EQUAL(3, es->depth());
					}},
			{{"3"}, regex("a"), [&](auto es) {
				 m += 3;
				 s += *es->pattern()->match(0);
				 BOOST_REQUIRE_EQUAL(3, es->depth());
				 es->setState("4");
				 BOOST_REQUIRE_EQUAL(3, es->depth());
				 BOOST_REQUIRE_EQUAL("4", es->getState());
				 BOOST_REQUIRE_EQUAL(3, es->depth());
				 BOOST_REQUIRE(!es->pattern()->match(1));
				 BOOST_REQUIRE(!es->pattern()->match(2));
				 es->popState();
				 BOOST_REQUIRE_EQUAL(2, es->depth());
				 BOOST_REQUIRE_EQUAL("2", es->getState());
				 es->pushState("3");
				 BOOST_REQUIRE_EQUAL(3, es->depth());
				 BOOST_REQUIRE_EQUAL("3", es->getState());
			 }}});
	BOOST_REQUIRE_EQUAL(0, m);
	l.extract("aaaa", 4);
	BOOST_REQUIRE_EQUAL(9, m);
	BOOST_REQUIRE_EQUAL("aa", s);
}

BOOST_AUTO_TEST_CASE(multibyte)
{
	AdHoc::Lexer::PatternPtr maskHead
			= AdHoc::LexerMatchers::regex("^# ([^<\n]+)? ?(<(.+?@[^\n>]+)>?)? \\((\\d+ "
										  "*(?:jan|feb|mar|apr|may|jun|jul|aug|sep|oct|nov|dec)\\w* \\d+)\\)$",
					GRegexCompileFlags(G_REGEX_OPTIMIZE | G_REGEX_CASELESS | G_REGEX_UNGREEDY));
	Glib::ustring input("# Michał Górny <mgorny@gentoo.org> (28 Mar 2015)");
	BOOST_REQUIRE_GT(input.bytes(), input.length());
	BOOST_REQUIRE(maskHead->matches(input.c_str(), input.bytes(), 0));
	BOOST_REQUIRE_EQUAL(maskHead->matchedLength(), input.bytes());
	BOOST_REQUIRE(maskHead->match(1));
	BOOST_REQUIRE_EQUAL("Michał Górny", *maskHead->match(1));
	BOOST_REQUIRE(maskHead->match(2));
	BOOST_REQUIRE_EQUAL("<mgorny@gentoo.org>", *maskHead->match(2));
	BOOST_REQUIRE(maskHead->match(3));
	BOOST_REQUIRE_EQUAL("mgorny@gentoo.org", *maskHead->match(3));
	BOOST_REQUIRE(maskHead->match(4));
	BOOST_REQUIRE_EQUAL("28 Mar 2015", *maskHead->match(4));
}

BOOST_AUTO_TEST_CASE(badre)
{
	BOOST_REQUIRE_THROW(regex("["), std::runtime_error);
}
