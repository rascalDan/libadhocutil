#define BOOST_TEST_MODULE Cache
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include "cache.h"
#include "cache.impl.h"

// LCOV_EXCL_START
BOOST_TEST_DONT_PRINT_LOG_VALUE(std::nullptr_t);
// LCOV_EXCL_STOP

class Obj {
	public:
		Obj(int i) : v(i) { }
		bool operator==(const int & i) const {
			return v == i;
		}
		int v;
};

bool
operator==(const int & i, const Obj & o)
{
	return i == o.v;
}

namespace std {
	ostream & operator<<(ostream & s, const Obj & o)
	{
		return s << o.v;
	}
}

namespace AdHoc {
	typedef Cache<Obj, std::string> TestCache;
	template class Cache<Obj, std::string>;
	template class Cacheable<Obj, std::string>;
	template class ObjectCacheable<Obj, std::string>;
	template class CallCacheable<Obj, std::string>;
}

using namespace AdHoc;

BOOST_AUTO_TEST_CASE( miss )
{
	TestCache tc;
	BOOST_REQUIRE_EQUAL(0, tc.size());
	tc.add("key", 3, time(NULL) + 5);
	BOOST_REQUIRE_EQUAL(1, tc.size());
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("anything"));
	BOOST_REQUIRE_EQUAL(nullptr, tc.getItem("anything"));
	BOOST_REQUIRE_EQUAL(1, tc.size());
}

BOOST_AUTO_TEST_CASE( hit )
{
	TestCache tc;
	auto vu = time(NULL) + 5;
	tc.add("key", 3, vu);
	BOOST_REQUIRE_EQUAL(1, tc.size());
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(3, tc.getItem("key")->item());
	BOOST_REQUIRE_EQUAL(vu, tc.getItem("key")->validUntil);
	BOOST_REQUIRE_EQUAL("key", tc.getItem("key")->key);
	BOOST_REQUIRE_EQUAL(1, tc.size());
}

BOOST_AUTO_TEST_CASE( expired )
{
	TestCache tc;
	tc.add("miss", 3, time(NULL) - 5);
	tc.add("hit", 3, time(NULL) + 5);
	// We only prune once a second... so size() should stay at 1
	BOOST_REQUIRE_EQUAL(2, tc.size());
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("miss"));
	BOOST_REQUIRE_EQUAL(nullptr, tc.getItem("miss"));
	BOOST_REQUIRE(tc.get("hit"));
	BOOST_REQUIRE(tc.getItem("hit"));
	BOOST_REQUIRE_EQUAL(2, tc.size());
	sleep(1);
	// Should prune now...
	BOOST_REQUIRE_EQUAL(2, tc.size());
	BOOST_REQUIRE(tc.get("hit"));
	BOOST_REQUIRE_EQUAL(2, tc.size());
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("miss"));
	BOOST_REQUIRE_EQUAL(1, tc.size());
}

BOOST_AUTO_TEST_CASE( callcache )
{
	TestCache tc;
	int callCount = 0;
	auto vu = time(NULL) + 5;
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("key"));
	tc.add("key", [&callCount]{ callCount++; return 3; }, vu);
	BOOST_REQUIRE_EQUAL(0, callCount);
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(1, callCount);
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(1, callCount);
}

