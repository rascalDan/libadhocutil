#define BOOST_TEST_MODULE CompileTimeFormatter
#include <boost/test/unit_test.hpp>

#include <compileTimeFormatter.h>

#include <boost/filesystem/path.hpp>

using namespace AdHoc;

extern constexpr const char * formatEdgeCaseEmpty = "";
extern constexpr const char * formatEdgeCaseSingle = "1";
extern constexpr const char * formatEdgeCaseFormatStart = "%? after";
extern constexpr const char * formatEdgeCaseFormatEnd = "before %?";
extern constexpr const char * formatEdgeCaseFormatLonely = "%?";

extern constexpr const char * formatStringLiteral = "literal";
extern constexpr const char * formatStringSingle = "single %?.";
extern constexpr const char * formatStringMulti = "First %?, then %?.";

BOOST_AUTO_TEST_CASE ( empty )
{
	std::stringstream buf;
  Formatter<formatEdgeCaseEmpty>::write(buf);
	BOOST_REQUIRE(buf.str().empty());
}

BOOST_AUTO_TEST_CASE ( single )
{
	std::stringstream buf;
  Formatter<formatEdgeCaseSingle>::write(buf);
	BOOST_REQUIRE_EQUAL(buf.str(), "1");
}

BOOST_AUTO_TEST_CASE ( start )
{
	std::stringstream buf;
  Formatter<formatEdgeCaseFormatStart>::write(buf, 10);
	BOOST_REQUIRE_EQUAL(buf.str(), "10 after");
}

BOOST_AUTO_TEST_CASE ( end )
{
	std::stringstream buf;
  Formatter<formatEdgeCaseFormatEnd>::write(buf, 10);
	BOOST_REQUIRE_EQUAL(buf.str(), "before 10");
}

BOOST_AUTO_TEST_CASE ( lonely )
{
	std::stringstream buf;
  Formatter<formatEdgeCaseFormatLonely>::write(buf, 10);
	BOOST_REQUIRE_EQUAL(buf.str(), "10");
}

BOOST_AUTO_TEST_CASE ( literal )
{
	std::stringstream buf;
  Formatter<formatStringLiteral>::write(buf);
	BOOST_REQUIRE_EQUAL(buf.str(), "literal");
}

BOOST_AUTO_TEST_CASE ( singleInt )
{
	std::stringstream buf;
  Formatter<formatStringSingle>::write(buf, 32);
	BOOST_REQUIRE_EQUAL(buf.str(), "single 32.");
}

BOOST_AUTO_TEST_CASE ( singleDouble )
{
	std::stringstream buf;
  Formatter<formatStringSingle>::write(buf, 3.14);
	BOOST_REQUIRE_EQUAL(buf.str(), "single 3.14.");
}

BOOST_AUTO_TEST_CASE ( singlePath )
{
	std::stringstream buf;
	boost::filesystem::path p("/tmp/test/path");
  Formatter<formatStringSingle>::write(buf, p);
	BOOST_REQUIRE_EQUAL(buf.str(), R"(single "/tmp/test/path".)");
}

BOOST_AUTO_TEST_CASE ( multi )
{
	std::stringstream buf;
  Formatter<formatStringMulti>::write(buf, "one", "two");
	BOOST_REQUIRE_EQUAL(buf.str(), "First one, then two.");
}

namespace AdHoc {
	// Custom stream writer formatter, formats as (bracketed expression)
	template<const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter<S, start, stream, '%', '(', ')', sn...> {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << "-( " << p << " )-";
			StreamWriter<S, start + 3, stream, sn...>::write(s, pn...);
		}
	};
	// Custom stream writer formatter, formats
	//            right-aligned by given width
	template<const char * const & S, int start, typename stream, char ... sn>
	struct StreamWriter<S, start, stream, '%', 'r', 'a', sn...> {
		template<typename P, typename ... Pn>
		static void write(stream & s, int width, const P & p, const Pn & ... pn)
		{
			std::stringstream buf;
			buf << p;
			std::string spaces(width - buf.str().length(), ' ');
			s << spaces << buf.str();
			StreamWriter<S, start + 3, stream, sn...>::write(s, pn...);
		}
	};
}

extern constexpr const char * formatStringCustom = "custom %()";
BOOST_AUTO_TEST_CASE ( customBracketted )
{
	std::stringstream buf;
  Formatter<formatStringCustom>::write(buf, "expr");
	BOOST_REQUIRE_EQUAL(buf.str(), "custom -( expr )-");
}

extern constexpr const char * formatStringMultiArg = "value%ra";
BOOST_AUTO_TEST_CASE ( customMultiArgRightAlign )
{
	std::stringstream buf1, buf2, buf3;
	const int width = 20;
  Formatter<formatStringMultiArg>::write(buf1, width, "something");
  Formatter<formatStringMultiArg>::write(buf2, width, "something else");
  Formatter<formatStringMultiArg>::write(buf3, width, 123.45);
	BOOST_REQUIRE_EQUAL(buf1.str(), "value           something");
	BOOST_REQUIRE_EQUAL(buf2.str(), "value      something else");
	BOOST_REQUIRE_EQUAL(buf3.str(), "value              123.45");
}

extern constexpr const char * formatStringLong = "                                                                                                                                                                                                                                                      ";
BOOST_AUTO_TEST_CASE ( longFormatString )
{
	std::stringstream buf;
  Formatter<formatStringLong>::write(buf);
	BOOST_REQUIRE_EQUAL(buf.str().length(), 246);
}

