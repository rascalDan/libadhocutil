#define BOOST_TEST_MODULE LazyPointer
#include <boost/test/unit_test.hpp>

#include "lazyPointer.h"
#include <memory>
#include <string>

using namespace AdHoc;

class Test {
public:
	explicit Test(int v) : val(v) { }
	const int val;
};

using TestLazyPointer = LazyPointer<Test>;
using RawLazyPointer = LazyPointer<int, int *>;

/// LCOV_EXCL_START (diagnostics)
BOOST_TEST_DONT_PRINT_LOG_VALUE(TestLazyPointer);
BOOST_TEST_DONT_PRINT_LOG_VALUE(RawLazyPointer);
/// LCOV_EXCL_STOP

static TestLazyPointer::pointer_type
factory()
{
	return std::make_shared<Test>(3);
}

static TestLazyPointer::pointer_type
paramFactory(const std::string & str)
{
	return std::make_shared<Test>(str.length());
}

BOOST_AUTO_TEST_CASE(islazy)
{
	TestLazyPointer p([] {
		return factory();
	});
	BOOST_REQUIRE_EQUAL(false, p.hasValue());
	Test * t = p.get();
	BOOST_REQUIRE(t);
	BOOST_REQUIRE(p);
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(p, t);
	BOOST_REQUIRE_EQUAL(3, t->val);
	BOOST_REQUIRE_EQUAL(3, p->val);
}

BOOST_AUTO_TEST_CASE(preinit)
{
	Test * t = new Test(4);
	TestLazyPointer p(TestLazyPointer::pointer_type {t});
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(p, t);
	BOOST_REQUIRE_EQUAL(4, p->val);
}

BOOST_AUTO_TEST_CASE(reset)
{
	Test * t = new Test(4);
	TestLazyPointer p(TestLazyPointer::pointer_type {t});
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(4, p->val);
	p = nullptr;
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(true, !p);
	p = [] {
		return factory();
	};
	BOOST_REQUIRE_EQUAL(false, p.hasValue());
	BOOST_REQUIRE(p.get());
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(3, p->val);
}

BOOST_AUTO_TEST_CASE(nondefault)
{
	TestLazyPointer p([] {
		return paramFactory("some string");
	});
	BOOST_REQUIRE_EQUAL(false, p.hasValue());
	BOOST_REQUIRE_EQUAL(11, (*p).val);
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
}

BOOST_AUTO_TEST_CASE(rawPointerNull)
{
	RawLazyPointer null;
	BOOST_REQUIRE(null.hasValue());
	BOOST_REQUIRE(!null);
	BOOST_REQUIRE(!null.get());
}

BOOST_AUTO_TEST_CASE(rawPointerNonNull)
{
	RawLazyPointer value(new int(3));
	BOOST_REQUIRE(value.hasValue());
	BOOST_REQUIRE(value);
	BOOST_REQUIRE(value.get());
	BOOST_REQUIRE_EQUAL(*value, 3);
	int * x = value.operator int *();
	BOOST_REQUIRE_EQUAL(*x, 3);
	delete x;
}

int *
rawFactory(const std::string & s)
{
	return new int(static_cast<int>(s.length()));
}

BOOST_AUTO_TEST_CASE(rawPointerFactory)
{
	RawLazyPointer value([] {
		return rawFactory(std::string("four"));
	});
	BOOST_REQUIRE(!value.hasValue());
	BOOST_REQUIRE_EQUAL(*value, 4);
	BOOST_REQUIRE(value.hasValue());
	delete value.operator int *();
}
