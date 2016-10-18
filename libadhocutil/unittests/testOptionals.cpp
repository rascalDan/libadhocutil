#define BOOST_TEST_MODULE Buffer
#include <boost/test/unit_test.hpp>

#include <optionals.h>
#include <boost/optional.hpp>
#include <IceUtil/Exception.h>
#include <IceUtil/Optional.h>

using namespace AdHoc;

BOOST_AUTO_TEST_CASE ( general )
{
	boost::optional<int> x;
	boost::optional<double> y = 2.3;
	IceUtil::Optional<float> ix;
	IceUtil::Optional<short> iy = 4;
	std::string * p = nullptr;
	std::string * q = new std::string("str");
	boost::optional<std::string> r;
	IceUtil::Optional<std::string> s;

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

