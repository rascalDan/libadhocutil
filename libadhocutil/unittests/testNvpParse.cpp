#define BOOST_TEST_MODULE NvpParse
#include <boost/test/unit_test.hpp>

#include "nvpParse.h"
#include <iosfwd>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

using namespace AdHoc;

class TestTarget {
public:
	std::string a;
	std::string b;
	int c {0};
	double d {0};
};

NvpTarget(TestTarget) TestTargetMap {
		NvpValue(TestTarget, a),
		NvpValue(TestTarget, b),
		NvpValue(TestTarget, c),
		NvpValue(TestTarget, d),
};

/// LCOV_EXCL_START (diagnostics)
BOOST_TEST_DONT_PRINT_LOG_VALUE(decltype(TestTargetMap.find("")))
/// LCOV_EXCL_STOP

BOOST_AUTO_TEST_CASE(targetmap)
{
	BOOST_REQUIRE_NE(TestTargetMap.find("a"), TestTargetMap.end());
	BOOST_REQUIRE_NE(TestTargetMap.find("b"), TestTargetMap.end());
	BOOST_REQUIRE_NE(TestTargetMap.find("c"), TestTargetMap.end());
	BOOST_REQUIRE_NE(TestTargetMap.find("d"), TestTargetMap.end());
	BOOST_REQUIRE_EQUAL(TestTargetMap.find("e"), TestTargetMap.end());
}

BOOST_AUTO_TEST_CASE(parse)
{
	TestTarget tt;
	std::stringstream i("a = foo;b=bar; c=3;d=3.14");
	NvpParse::parse(i, TestTargetMap, tt);
	BOOST_REQUIRE_EQUAL("foo", tt.a);
	BOOST_REQUIRE_EQUAL("bar", tt.b);
	BOOST_REQUIRE_EQUAL(3, tt.c);
	BOOST_REQUIRE_CLOSE(3.14, tt.d, 0.01);
}

BOOST_AUTO_TEST_CASE(missing)
{
	TestTarget tt;
	std::stringstream i("missing=nothing;");
	BOOST_REQUIRE_THROW(NvpParse::parse(i, TestTargetMap, tt), NvpParse::ValueNotFound);
}

BOOST_AUTO_TEST_CASE(bad)
{
	TestTarget tt;
	std::stringstream i("{bad=");
	BOOST_REQUIRE_THROW(NvpParse::parse(i, TestTargetMap, tt), std::runtime_error);
}
