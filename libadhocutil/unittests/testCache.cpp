#define BOOST_TEST_MODULE Cache
#include <boost/test/unit_test.hpp>

#include <functional>
#include "cache.h"
#include "cache.impl.h"

// NOLINTNEXTLINE(hicpp-special-member-functions)
class Obj {
	public:
		// NOLINTNEXTLINE(hicpp-explicit-conversions)
		Obj(int i) : v(i) { }
		void operator=(const Obj &) = delete;
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
	/// LCOV_EXCL_START (diagnostics)
	ostream & operator<<(ostream & s, const Obj & o)
	{
		return s << o.v;
	}
	/// LCOV_EXCL_STOP
}

namespace AdHoc {
	using TestCache = Cache<Obj, std::string>;
	template class Cache<Obj, std::string>;
	template class Cacheable<Obj, std::string>;
	template class ObjectCacheable<Obj, std::string>;
	template class CallCacheable<Obj, std::string>;
	template class PointerCallCacheable<Obj, std::string>;
}

using namespace AdHoc;

BOOST_AUTO_TEST_CASE( miss )
{
	TestCache tc;
	BOOST_REQUIRE_EQUAL(0, tc.size());
	tc.add("key", 3, time(nullptr) + 5);
	BOOST_REQUIRE_EQUAL(1, tc.size());
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("anything"));
	BOOST_REQUIRE_EQUAL(nullptr, tc.getItem("anything"));
	BOOST_REQUIRE_EQUAL(1, tc.size());
}

BOOST_AUTO_TEST_CASE( hit )
{
	TestCache tc;
	auto vu = time(nullptr) + 5;
	tc.add("key", 3, vu);
	BOOST_REQUIRE_EQUAL(1, tc.size());
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(3, *tc.getItem("key")->item());
	BOOST_REQUIRE_EQUAL(vu, tc.getItem("key")->validUntil);
	BOOST_REQUIRE_EQUAL("key", tc.getItem("key")->key);
	BOOST_REQUIRE_EQUAL(1, tc.size());
	tc.remove("key");
	BOOST_REQUIRE_EQUAL(0, tc.size());
}

BOOST_AUTO_TEST_CASE( multivalues )
{
	TestCache tc;
	auto vu = time(nullptr) + 5;
	tc.add("key1", 1, vu);
	tc.add("key2", 2, vu);
	tc.add("key3", 3, vu);
	BOOST_REQUIRE_EQUAL(3, tc.size());
	BOOST_REQUIRE_EQUAL(1, *tc.get("key1"));
	BOOST_REQUIRE_EQUAL(2, *tc.get("key2"));
	BOOST_REQUIRE_EQUAL(3, *tc.get("key3"));
	tc.remove("key1");
	BOOST_REQUIRE_EQUAL(2, tc.size());
	BOOST_REQUIRE(!tc.get("key1"));
	BOOST_REQUIRE_EQUAL(2, *tc.get("key2"));
	BOOST_REQUIRE_EQUAL(3, *tc.get("key3"));
	tc.clear();
	BOOST_REQUIRE_EQUAL(0, tc.size());
}

BOOST_AUTO_TEST_CASE( expired )
{
	TestCache tc;
	tc.add("miss", 3, time(nullptr) - 5);
	tc.add("hit", 3, time(nullptr) + 5);
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
	auto vu = time(nullptr) + 5;
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("key"));
	tc.addFactory("key", [&callCount]{ callCount++; return 3; }, vu);
	BOOST_REQUIRE_EQUAL(0, callCount);
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(1, callCount);
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(1, callCount);
}

BOOST_AUTO_TEST_CASE( pointercallcache )
{
	TestCache tc;
	int callCount = 0;
	auto vu = time(nullptr) + 5;
	BOOST_REQUIRE_EQUAL(nullptr, tc.get("key"));
	tc.addPointerFactory("key", [&callCount]{ callCount++; return TestCache::Value(new Obj(3)); }, vu);
	BOOST_REQUIRE_EQUAL(0, callCount);
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(1, callCount);
	BOOST_REQUIRE_EQUAL(3, *tc.get("key"));
	BOOST_REQUIRE_EQUAL(1, callCount);
}

BOOST_AUTO_TEST_CASE( hitThenRenove )
{
	TestCache tc;
	tc.add("key", 3, time(nullptr) + 5);
	auto h = tc.get("key");
	BOOST_REQUIRE(h);
	BOOST_REQUIRE_EQUAL(3, *h);
	tc.remove("key");
	BOOST_REQUIRE(!tc.get("key"));
	BOOST_REQUIRE_EQUAL(3, *h);
}

BOOST_AUTO_TEST_CASE( addPointer )
{
	TestCache tc;
	auto v = TestCache::Value(new Obj(3));
	tc.addPointer("key", v, time(nullptr) + 1);
	auto h = tc.get("key");
	BOOST_REQUIRE(h);
	BOOST_REQUIRE_EQUAL(3, *h);
}

