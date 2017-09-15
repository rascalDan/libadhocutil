#define BOOST_TEST_MODULE LazyPointer
#include <boost/test/unit_test.hpp>

#include <boost/bind.hpp>
#include "intrusivePtrBase.h"
#include "lazyPointer.h"

using namespace AdHoc;

class Test : public IntrusivePtrBase {
	public:
		Test(int v) :
			val(v)
		{
		}
		const int val;
};

typedef LazyPointer<Test> TestLazyPointer;
typedef LazyPointer<int, int *> RawLazyPointer;

static
TestLazyPointer::pointer_type
factory()
{
	return new Test(3);
}

static
TestLazyPointer::pointer_type
paramFactory(const std::string & str)
{
	return new Test(str.length());
}

BOOST_AUTO_TEST_CASE ( islazy )
{
	TestLazyPointer p(boost::bind(&factory));
	BOOST_REQUIRE_EQUAL(false, p.hasValue());
	Test * t = p.get();
	BOOST_REQUIRE(t);
	bool pbool = p;
	BOOST_REQUIRE(pbool);
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(p, t);
	BOOST_REQUIRE_EQUAL(3, t->val);
	BOOST_REQUIRE_EQUAL(3, p->val);
}

BOOST_AUTO_TEST_CASE ( preinit )
{
	Test * t = new Test(4);
	TestLazyPointer p(t);
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(p, t);
	BOOST_REQUIRE_EQUAL(4, p->val);
}

BOOST_AUTO_TEST_CASE ( reset )
{
	Test * t = new Test(4);
	TestLazyPointer p(t);
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(4, p->val);
	p = nullptr;
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(true, !p);
	p = boost::bind(&factory);
	BOOST_REQUIRE_EQUAL(false, p.hasValue());
	p.get();
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
	BOOST_REQUIRE_EQUAL(3, p->val);
}

BOOST_AUTO_TEST_CASE ( nondefault )
{
	TestLazyPointer p(boost::bind(&paramFactory, "some string"));
	BOOST_REQUIRE_EQUAL(false, p.hasValue());
	BOOST_REQUIRE_EQUAL(11, (*p).val);
	BOOST_REQUIRE_EQUAL(true, p.hasValue());
}

BOOST_AUTO_TEST_CASE( rawPointerNull )
{
	RawLazyPointer null;
	BOOST_REQUIRE(null.hasValue());
	BOOST_REQUIRE(!null);
	BOOST_REQUIRE(!null.get());
}

BOOST_AUTO_TEST_CASE( rawPointerNonNull )
{
	RawLazyPointer value(new int(3));
	BOOST_REQUIRE(value.hasValue());
	BOOST_REQUIRE(value);
	BOOST_REQUIRE(value.get());
	BOOST_REQUIRE_EQUAL(*value, 3);
	int * x = value;
	BOOST_REQUIRE_EQUAL(*x, 3);
	delete value;
}

int *
rawFactory(const std::string & s)
{
	return new int(s.length());
}

BOOST_AUTO_TEST_CASE( rawPointerFactory )
{
	RawLazyPointer value(boost::bind(&rawFactory, std::string("four")));
	BOOST_REQUIRE(!value.hasValue());
	BOOST_REQUIRE_EQUAL(*value, 4);
	BOOST_REQUIRE(value.hasValue());
	delete value;
}

