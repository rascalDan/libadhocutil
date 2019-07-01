#define BOOST_TEST_MODULE NvpParse
#include <boost/test/unit_test.hpp>
#include <boost/mpl/list.hpp>

#include "memstream.h"

using namespace AdHoc;

BOOST_FIXTURE_TEST_SUITE(s, MemStream);

BOOST_AUTO_TEST_CASE(empty)
{
	BOOST_CHECK_EQUAL("", this->sv());
	BOOST_CHECK_EQUAL("", this->buffer());
	BOOST_CHECK_EQUAL(0, this->sv().length());
	BOOST_CHECK_EQUAL(0, this->length());
}

using cast_types = boost::mpl::list<const char *, std::string_view, FILE *>;
BOOST_AUTO_TEST_CASE_TEMPLATE(casts, T, cast_types )
{
	auto dummy = [](const T &) { };
	dummy(*this);
}

BOOST_AUTO_TEST_CASE(simple)
{
	auto len = fprintf(*this, "Some %s write.", "simple");
	BOOST_CHECK_EQUAL("Some simple write.", this->buffer());
	BOOST_CHECK_EQUAL(len, this->length());
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE(move_construct)
{
	MemStream ms1;
	MemStream ms2(std::move(ms1));
}

BOOST_AUTO_TEST_CASE(move_assign)
{
	MemStream ms1;
	MemStream ms2;
	ms2 = std::move(ms1);
}

