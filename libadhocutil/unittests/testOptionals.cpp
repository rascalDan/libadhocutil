#define BOOST_TEST_MODULE Buffer
#include <boost/test/unit_test.hpp>

#include <optionalUtils.h>
#include <optional>
#include <Ice/Exception.h>
#include <Ice/Optional.h>

using namespace AdHoc;

BOOST_AUTO_TEST_CASE ( general )
{
	std::optional<int> x;
	std::optional<double> y = 2.3;
	Ice::optional<float> ix;
	Ice::optional<short> iy = 4;
	std::string * p = nullptr;
	auto * q = new std::string("str");
	std::optional<std::string> r;
	Ice::optional<std::string> s;

	bool b(p);
	BOOST_REQUIRE_EQUAL(false, b);
	auto a1 = x / y;
	BOOST_REQUIRE(a1);
	BOOST_REQUIRE_EQUAL(2.3, *a1);
	auto a2 = x / 10;
	BOOST_REQUIRE_EQUAL(10, a2);
	auto a3 = ix /  11;
	BOOST_REQUIRE_EQUAL(11, a3);
	auto a4 = iy /  11;
	BOOST_REQUIRE_EQUAL(4, a4);

	auto s1 = p / s / r;
	BOOST_REQUIRE(!s1);
	auto s2 = q / s / r;
	BOOST_REQUIRE(s2);
	BOOST_REQUIRE_EQUAL("str", *s2);

	auto p1 = r / q;
	BOOST_REQUIRE(p1);
	BOOST_REQUIRE_EQUAL("str", *p1);

	delete q;
}

