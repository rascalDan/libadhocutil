#define BOOST_TEST_MODULE CaseLess
#include <boost/test/unit_test.hpp>

#include <case_less.h>
#include <string>
#include <string_view>

BOOST_FIXTURE_TEST_SUITE(l, AdHoc::case_less);

BOOST_AUTO_TEST_CASE(case_less_test)
{
	using namespace std::literals;
	const auto & lc {*this};
	BOOST_CHECK(!lc(""sv, ""sv));
	BOOST_CHECK(lc("a"sv, "b"sv));
	BOOST_CHECK(lc("A"sv, "b"sv));
	BOOST_CHECK(lc("Aa"sv, "b"sv));
	BOOST_CHECK(lc("AA"sv, "b"sv));
	BOOST_CHECK(lc("aA"sv, "b"s));
	BOOST_CHECK(lc("A"s, "B"sv));
	BOOST_CHECK(lc("Aa"sv, "Bb"s));
	BOOST_CHECK(lc("AA"s, "bB"s));
	BOOST_CHECK(lc("aA"s, "BB"s));
}

BOOST_AUTO_TEST_SUITE_END()
