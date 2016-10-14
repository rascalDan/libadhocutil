#define BOOST_TEST_MODULE Lexer
#include <boost/test/unit_test.hpp>

#include <lexer.h>
#include <lexer-regex.h>

using namespace AdHoc;
using namespace AdHoc::LexerMatchers;

BOOST_AUTO_TEST_CASE( defaultConstructor )
{
	AdHoc::Lexer l;
	l.rules.push_back({ { AdHoc::Lexer::InitialState }, regex("a"), [](auto) { } });
}

BOOST_AUTO_TEST_CASE( simple )
{
	int m = 0;
	AdHoc::Lexer l({
		{ { AdHoc::Lexer::InitialState }, regex("a"), [&](auto) { m += 1; } }
	});
	BOOST_REQUIRE_EQUAL(0, m);
	l.extract("aaaa", 4);
	BOOST_REQUIRE_EQUAL(4, m);
	BOOST_REQUIRE_THROW({
		l.extract("abcd", 4);
	}, std::runtime_error);
}

BOOST_AUTO_TEST_CASE( state )
{
	int m = 0;
	std::string s;
	AdHoc::Lexer l({
		{ { AdHoc::Lexer::InitialState }, regex("a"), [&](auto es)
			{
				m += 1;
				BOOST_REQUIRE_EQUAL(1, es->depth());
				es->pushState("2");
				BOOST_REQUIRE_EQUAL(2, es->depth());
			} },
		{ { "2" }, regex("a"), [&](auto es)
			{
				m += 2;
				BOOST_REQUIRE_EQUAL("2", es->getState());
				BOOST_REQUIRE_EQUAL(2, es->depth());
				es->pushState("3");
				BOOST_REQUIRE_EQUAL("3", es->getState());
				BOOST_REQUIRE_EQUAL(3, es->depth());
			} },
		{ { "3" }, regex("a"), [&](auto es)
			{
				m += 3;
				s += *es->pattern->match(0);
				BOOST_REQUIRE_EQUAL(3, es->depth());
				es->setState("4");
				BOOST_REQUIRE_EQUAL(3, es->depth());
				BOOST_REQUIRE_EQUAL("4", es->getState());
				BOOST_REQUIRE_EQUAL(3, es->depth());
				BOOST_REQUIRE(!es->pattern->match(1));
				BOOST_REQUIRE(!es->pattern->match(2));
				es->popState();
				BOOST_REQUIRE_EQUAL(2, es->depth());
				BOOST_REQUIRE_EQUAL("2", es->getState());
				es->pushState("3");
				BOOST_REQUIRE_EQUAL(3, es->depth());
				BOOST_REQUIRE_EQUAL("3", es->getState());
			} }
	});
	BOOST_REQUIRE_EQUAL(0, m);
	l.extract("aaaa", 4);
	BOOST_REQUIRE_EQUAL(9, m);
	BOOST_REQUIRE_EQUAL("aa", s);
}

BOOST_AUTO_TEST_CASE( badre )
{
	BOOST_REQUIRE_THROW(regex("["), std::runtime_error);
}

