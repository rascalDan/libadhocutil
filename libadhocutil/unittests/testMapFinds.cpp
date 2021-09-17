#define BOOST_TEST_MODULE MapFinds
#include <boost/test/unit_test.hpp>

#include "safeMapFind.h"
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

using namespace AdHoc;

class NotFound : std::runtime_error {
public:
	explicit NotFound(int key) : std::runtime_error("Key not found: %d" + std::to_string(key)) { }
};

const std::map<int, std::string> sample {{1, "one"}, {2, "two"}, {4, "four"}, {8, "eight"}, {16, "sixteen"}};

BOOST_AUTO_TEST_CASE(testSafeMapFind)
{
	BOOST_REQUIRE_EQUAL(safeMapFind<NotFound>(sample, 1)->first, 1);
	BOOST_REQUIRE_EQUAL(safeMapFind<NotFound>(sample, 1)->second, "one");
	BOOST_REQUIRE_EQUAL(safeMapFind<NotFound>(sample, 4)->first, 4);
	BOOST_REQUIRE_EQUAL(safeMapFind<NotFound>(sample, 4)->second, "four");
	BOOST_REQUIRE_THROW(safeMapFind<NotFound>(sample, 5), NotFound);
}

BOOST_AUTO_TEST_CASE(testDefaultMapLookup)
{
	BOOST_REQUIRE_EQUAL(defaultMapLookup(sample, 1), "one");
	BOOST_REQUIRE_EQUAL(defaultMapLookup(sample, 4), "four");
	BOOST_REQUIRE_EQUAL(defaultMapLookup(sample, 5), std::string());
	BOOST_REQUIRE_EQUAL(defaultMapLookup(sample, 5, "default"), "default");
}

BOOST_AUTO_TEST_CASE(testSafeMapLookup)
{
	BOOST_REQUIRE_EQUAL(safeMapLookup<NotFound>(sample, 1), "one");
	BOOST_REQUIRE_EQUAL(safeMapLookup<NotFound>(sample, 4), "four");
	BOOST_REQUIRE_THROW(safeMapLookup<NotFound>(sample, 5), NotFound);
}

BOOST_AUTO_TEST_CASE(testContainerContains)
{
	BOOST_REQUIRE_EQUAL(true, containerContains(sample, {1, "one"}));
	BOOST_REQUIRE_EQUAL(false, containerContains(sample, {2, "one"}));
	BOOST_REQUIRE_EQUAL(false, containerContains(sample, {1, "two"}));
	BOOST_REQUIRE_EQUAL(true, containerContains(sample, {2, "two"}));
}
