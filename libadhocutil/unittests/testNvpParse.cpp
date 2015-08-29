#define BOOST_TEST_MODULE NvpParse
#include <boost/test/unit_test.hpp>

#include "nvpParse.h"

class TestTarget {
	public:
		std::string a;
		std::string b;
		int c;
		double d;
};

NvpTarget(TestTarget) TestTargetMap {
	NvpValue(TestTarget, a),
	NvpValue(TestTarget, b),
	NvpValue(TestTarget, c),
	NvpValue(TestTarget, d),
};

BOOST_AUTO_TEST_CASE ( targetmap )
{
	BOOST_REQUIRE(TestTargetMap.find("a") != TestTargetMap.end());
	BOOST_REQUIRE(TestTargetMap.find("b") != TestTargetMap.end());
	BOOST_REQUIRE(TestTargetMap.find("c") != TestTargetMap.end());
	BOOST_REQUIRE(TestTargetMap.find("d") != TestTargetMap.end());
	BOOST_REQUIRE(TestTargetMap.find("e") == TestTargetMap.end());
}

BOOST_AUTO_TEST_CASE ( parse )
{
	TestTarget tt;
	std::stringstream i("a = foo;b=bar; c=3;d=3.14");
	NvpParse::Parse(i, TestTargetMap, tt);
	BOOST_REQUIRE_EQUAL("foo", tt.a);
	BOOST_REQUIRE_EQUAL("bar", tt.b);
	BOOST_REQUIRE_EQUAL(3, tt.c);
	BOOST_REQUIRE_CLOSE(3.14, tt.d, 0.01);
}

BOOST_AUTO_TEST_CASE ( missing )
{
	TestTarget tt;
	std::stringstream i("missing=nothing;");
	BOOST_REQUIRE_THROW(NvpParse::Parse(i, TestTargetMap, tt), NvpParse::ValueNotFound);
}

BOOST_AUTO_TEST_CASE ( bad )
{
	TestTarget tt;
	std::stringstream i("{bad=");
	BOOST_REQUIRE_THROW(NvpParse::Parse(i, TestTargetMap, tt), std::runtime_error);
}

