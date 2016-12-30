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
extern constexpr const char * formatStringCustom = "custom %()";
extern constexpr const char * formatStringCustomParam1 = "custom %(\x3)";
extern constexpr const char * formatStringCustomParam2 = "custom %(\x9)";
extern constexpr const char * formatStringCustomLong = "custom %(longname)";
extern constexpr const char * formatStringLong = "                                                                                                                                                                                                                                                      ";
extern constexpr const char * formatStringMultiArg = "value%ra";
extern constexpr const char * formatStringEscape1 = "literal %% percentage.";
extern constexpr const char * formatStringEscape2 = "literal %%? percentage.";
extern constexpr const char * formatStringEscape3 = "literal %%%? percentage.";
extern constexpr const char * formatStringEscape4 = "literal %%%?%% percentage.";

namespace AdHoc {
	// Custom stream writer formatter, formats as
	// -( bracketed expression )-
	StreamWriterT('(', ')') {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << "-( " << p << " )-";
			StreamWriter::next(s, pn...);
		}
	};
	// Custom stream writer formatter with a long identifier, formats as
	// ---( bracketed expression )---
	StreamWriterT('(', 'l', 'o', 'n', 'g', 'n', 'a', 'm', 'e', ')') {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			s << "---( " << p << " )---";
			StreamWriter::next(s, pn...);
		}
	};
	// Custom stream writer formatter that has parameter in the format string, formats as
	// dashes*-( bracketed expression )dashes*-
	StreamWriterTP(dashes, '(', dashes, ')') {
		template<typename P, typename ... Pn>
		static void write(stream & s, const P & p, const Pn & ... pn)
		{
			std::string d(dashes, '-');
			s << d << "( " << p << " )" << d;
			StreamWriter::next(s, pn...);
		}
	};
	// Custom stream writer formatter, formats
	//            right-aligned by given width
	StreamWriterT('r', 'a') {
		template<typename P, typename ... Pn>
		static void write(stream & s, int width, const P & p, const Pn & ... pn)
		{
			std::stringstream buf;
			buf << p;
			std::string spaces(width - buf.str().length(), ' ');
			s << spaces << buf.str();
			StreamWriter::next(s, pn...);
		}
	};
}

BOOST_FIXTURE_TEST_SUITE( TestStreamWrite, std::stringstream )

BOOST_AUTO_TEST_CASE ( empty )
{
	Formatter<formatEdgeCaseEmpty>::write(*this);
	BOOST_REQUIRE(this->str().empty());
}

BOOST_AUTO_TEST_CASE ( single )
{
	Formatter<formatEdgeCaseSingle>::write(*this);
	BOOST_REQUIRE_EQUAL(this->str(), "1");
}

BOOST_AUTO_TEST_CASE ( start )
{
	Formatter<formatEdgeCaseFormatStart>::write(*this, 10);
	BOOST_REQUIRE_EQUAL(this->str(), "10 after");
}

BOOST_AUTO_TEST_CASE ( end )
{
	Formatter<formatEdgeCaseFormatEnd>::write(*this, 10);
	BOOST_REQUIRE_EQUAL(this->str(), "before 10");
}

BOOST_AUTO_TEST_CASE ( lonely )
{
	Formatter<formatEdgeCaseFormatLonely>::write(*this, 10);
	BOOST_REQUIRE_EQUAL(this->str(), "10");
}

BOOST_AUTO_TEST_CASE ( literal )
{
	Formatter<formatStringLiteral>::write(*this);
	BOOST_REQUIRE_EQUAL(this->str(), "literal");
}

BOOST_AUTO_TEST_CASE ( singleInt )
{
	Formatter<formatStringSingle>::write(*this, 32);
	BOOST_REQUIRE_EQUAL(this->str(), "single 32.");
}

BOOST_AUTO_TEST_CASE ( singleDouble )
{
	Formatter<formatStringSingle>::write(*this, 3.14);
	BOOST_REQUIRE_EQUAL(this->str(), "single 3.14.");
}

BOOST_AUTO_TEST_CASE ( singlePath )
{
	boost::filesystem::path p("/tmp/test/path");
	Formatter<formatStringSingle>::write(*this, p);
	BOOST_REQUIRE_EQUAL(this->str(), R"(single "/tmp/test/path".)");
}

BOOST_AUTO_TEST_CASE ( multi )
{
	Formatter<formatStringMulti>::write(*this, "one", "two");
	BOOST_REQUIRE_EQUAL(this->str(), "First one, then two.");
}

BOOST_AUTO_TEST_CASE ( escape1 )
{
	Formatter<formatStringEscape1>::write(*this);
	BOOST_REQUIRE_EQUAL(this->str(), "literal % percentage.");
}

BOOST_AUTO_TEST_CASE ( escape2 )
{
	Formatter<formatStringEscape2>::write(*this);
	BOOST_REQUIRE_EQUAL(this->str(), "literal %? percentage.");
}

BOOST_AUTO_TEST_CASE ( escape3 )
{
	Formatter<formatStringEscape3>::write(*this, 3);
	BOOST_REQUIRE_EQUAL(this->str(), "literal %3 percentage.");
}

BOOST_AUTO_TEST_CASE ( escape4 )
{
	Formatter<formatStringEscape4>::write(*this, 3);
	BOOST_REQUIRE_EQUAL(this->str(), "literal %3% percentage.");
}

BOOST_AUTO_TEST_CASE ( customBracketted )
{
	Formatter<formatStringCustom>::write(*this, "expr");
	BOOST_REQUIRE_EQUAL(this->str(), "custom -( expr )-");
}

BOOST_AUTO_TEST_CASE ( customLongName )
{
	Formatter<formatStringCustomLong>::write(*this, "some text here");
	BOOST_REQUIRE_EQUAL(this->str(), "custom ---( some text here )---");
}

BOOST_AUTO_TEST_CASE ( customParam1 )
{
	Formatter<formatStringCustomParam1>::write(*this, "some text here");
	BOOST_REQUIRE_EQUAL(this->str(), "custom ---( some text here )---");
}

BOOST_AUTO_TEST_CASE ( customParam2 )
{
	Formatter<formatStringCustomParam2>::write(*this, "some text here");
	BOOST_REQUIRE_EQUAL(this->str(), "custom ---------( some text here )---------");
}

typedef Formatter<formatStringCustom> TestFormat;
BOOST_AUTO_TEST_CASE ( typedefFormat )
{
	TestFormat::write(*this, "expr");
	BOOST_REQUIRE_EQUAL(this->str(), "custom -( expr )-");
}

BOOST_AUTO_TEST_CASE ( longFormatString )
{
	Formatter<formatStringLong>::write(*this);
	BOOST_REQUIRE_EQUAL(this->str().length(), 246);
}

BOOST_AUTO_TEST_SUITE_END();

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

BOOST_AUTO_TEST_CASE ( get )
{
	auto s = Formatter<formatStringMultiArg>::get(20, "something else");
	BOOST_REQUIRE_EQUAL(s, "value      something else");
}
