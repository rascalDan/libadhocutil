#define BOOST_TEST_MODULE CompileTimeFormatter
#include <boost/test/unit_test.hpp>

#include <compileTimeFormatter.h>
#include <fileUtils.h>
#include <definedDirs.h>

#include <boost/filesystem/path.hpp>

using namespace AdHoc;

constexpr const char * formatEdgeCaseEmpty = "";
constexpr const char * formatEdgeCaseSingle = "1";
constexpr const char * formatEdgeCaseFormatStart = "%? after";
constexpr const char * formatEdgeCaseFormatEnd = "before %?";
constexpr const char * formatEdgeCaseFormatLonely = "%?";
constexpr const char * formatStringLiteral = "literal";
constexpr const char * formatStringSingle = "single %?.";
constexpr const char * formatStringMulti = "First %?, then %?.";
constexpr const char * formatStringCustom = "custom %()";
constexpr const char * formatStringCustomParam1 = "custom %(\x3)";
constexpr const char * formatStringCustomParam2 = "custom %(\x9)";
constexpr const char * formatStringCustomLong = "custom %(longname)";
constexpr const char * formatStringLong = "                                                                                                                                                                                                                                                      ";
constexpr const char * formatStringMultiArg = "value%ra";
constexpr const char * formatStringEscape1 = "literal %% percentage.";
constexpr const char * formatStringEscape2 = "literal %%? percentage.";
constexpr const char * formatStringEscape3 = "literal %%%? percentage.";
constexpr const char * formatStringEscape4 = "literal %%%?%% percentage.";

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

// Compile string util assertions
static_assert(strlen<formatEdgeCaseEmpty>() == 0);
static_assert(strlen<formatEdgeCaseSingle>() == 1);
static_assert(strlen<formatEdgeCaseFormatLonely>() == 2);
static_assert(strlen<formatStringLiteral>() == 7);
static_assert(strlen<formatStringLong>() == 246);

static_assert(strchr<formatEdgeCaseEmpty, 't'>() == -1);
static_assert(strchr<formatEdgeCaseSingle, 't'>() == -1);
static_assert(strchr<formatEdgeCaseSingle, '1'>() == 0);
static_assert(strchr<formatEdgeCaseFormatLonely, '%'>() == 0);
static_assert(strchr<formatEdgeCaseFormatLonely, '?'>() == 1);
static_assert(strchr<formatStringLiteral, 'e'>() == 3);
static_assert(strchr<formatStringLiteral, 'f'>() == -1);
static_assert(strchr<formatStringLiteral, 'e', 3>() == 3);
static_assert(strchr<formatStringLiteral, 'e', 4>() == -1);
static_assert(strchr<formatStringLiteral, 'f', 3>() == -1);

static_assert(strchrnul<formatEdgeCaseEmpty, 't'>() == 0);
static_assert(strchrnul<formatEdgeCaseSingle, 't'>() == 1);
static_assert(strchrnul<formatEdgeCaseSingle, '1'>() == 0);
static_assert(strchrnul<formatEdgeCaseFormatLonely, '%'>() == 0);
static_assert(strchrnul<formatEdgeCaseFormatLonely, '?'>() == 1);
static_assert(strchrnul<formatStringLiteral, 'e'>() == 3);
static_assert(strchrnul<formatStringLiteral, 'f'>() == 7);
static_assert(strchrnul<formatStringLiteral, 'e', 3>() == 3);
static_assert(strchrnul<formatStringLiteral, 'e', 4>() == 7);
static_assert(strchrnul<formatStringLiteral, 'f', 3>() == 7);

static_assert(strchrnul<formatEdgeCaseEmpty, 't'>() == 0);
static_assert(strchrnul<formatEdgeCaseSingle, 't'>() == 1);
static_assert(strchrnul<formatEdgeCaseSingle, '1'>() == 0);
static_assert(strchrnul<formatEdgeCaseFormatLonely, '%'>() == 0);
static_assert(strchrnul<formatEdgeCaseFormatLonely, '?'>() == 1);
static_assert(strchrnul<formatStringLiteral, 'e'>() == 3);
static_assert(strchrnul<formatStringLiteral, 'f'>() == 7);

BOOST_FIXTURE_TEST_SUITE( TestStreamWrite, std::stringstream )

BOOST_AUTO_TEST_CASE ( empty )
{
	Formatter<formatEdgeCaseEmpty>::write(*this);
	BOOST_CHECK(this->str().empty());
}

BOOST_AUTO_TEST_CASE ( single )
{
	Formatter<formatEdgeCaseSingle>::write(*this);
	BOOST_CHECK_EQUAL(this->str(), "1");
}

BOOST_AUTO_TEST_CASE ( start )
{
	Formatter<formatEdgeCaseFormatStart>::write(*this, 10);
	BOOST_CHECK_EQUAL(this->str(), "10 after");
}

BOOST_AUTO_TEST_CASE ( end )
{
	Formatter<formatEdgeCaseFormatEnd>::write(*this, 10);
	BOOST_CHECK_EQUAL(this->str(), "before 10");
}

BOOST_AUTO_TEST_CASE ( lonely )
{
	Formatter<formatEdgeCaseFormatLonely>::write(*this, 10);
	BOOST_CHECK_EQUAL(this->str(), "10");
}

BOOST_AUTO_TEST_CASE ( literal )
{
	Formatter<formatStringLiteral>::write(*this);
	BOOST_CHECK_EQUAL(this->str(), "literal");
}

BOOST_AUTO_TEST_CASE ( singleInt )
{
	Formatter<formatStringSingle>::write(*this, 32);
	BOOST_CHECK_EQUAL(this->str(), "single 32.");
}

BOOST_AUTO_TEST_CASE ( singleIntReturn )
{
	BOOST_CHECK_EQUAL(Formatter<formatStringSingle>::write(*this, 32).str(), "single 32.");
}

BOOST_AUTO_TEST_CASE ( singleDouble )
{
	Formatter<formatStringSingle>::write(*this, 3.14);
	BOOST_CHECK_EQUAL(this->str(), "single 3.14.");
}

BOOST_AUTO_TEST_CASE ( singlePath )
{
	boost::filesystem::path p("/tmp/test/path");
	Formatter<formatStringSingle>::write(*this, p);
	BOOST_CHECK_EQUAL(this->str(), R"(single "/tmp/test/path".)");
}

BOOST_AUTO_TEST_CASE ( multi )
{
	Formatter<formatStringMulti>::write(*this, "one", "two");
	BOOST_CHECK_EQUAL(this->str(), "First one, then two.");
}

BOOST_AUTO_TEST_CASE ( escape1 )
{
	Formatter<formatStringEscape1>::write(*this);
	BOOST_CHECK_EQUAL(this->str(), "literal % percentage.");
}

BOOST_AUTO_TEST_CASE ( escape2 )
{
	Formatter<formatStringEscape2>::write(*this);
	BOOST_CHECK_EQUAL(this->str(), "literal %? percentage.");
}

BOOST_AUTO_TEST_CASE ( escape3 )
{
	Formatter<formatStringEscape3>::write(*this, 3);
	BOOST_CHECK_EQUAL(this->str(), "literal %3 percentage.");
}

BOOST_AUTO_TEST_CASE ( escape4 )
{
	Formatter<formatStringEscape4>::write(*this, 3);
	BOOST_CHECK_EQUAL(this->str(), "literal %3% percentage.");
}

BOOST_AUTO_TEST_CASE ( customBracketted )
{
	Formatter<formatStringCustom>::write(*this, "expr");
	BOOST_CHECK_EQUAL(this->str(), "custom -( expr )-");
}

BOOST_AUTO_TEST_CASE ( customLongName )
{
	Formatter<formatStringCustomLong>::write(*this, "some text here");
	BOOST_CHECK_EQUAL(this->str(), "custom ---( some text here )---");
}

BOOST_AUTO_TEST_CASE ( customParam1 )
{
	Formatter<formatStringCustomParam1>::write(*this, "some text here");
	BOOST_CHECK_EQUAL(this->str(), "custom ---( some text here )---");
}

BOOST_AUTO_TEST_CASE ( customParam2 )
{
	Formatter<formatStringCustomParam2>::write(*this, "some text here");
	BOOST_CHECK_EQUAL(this->str(), "custom ---------( some text here )---------");
}

typedef Formatter<formatStringCustom> TestFormat;
BOOST_AUTO_TEST_CASE ( typedefFormat )
{
	TestFormat::write(*this, "expr");
	BOOST_CHECK_EQUAL(this->str(), "custom -( expr )-");
}

AdHocFormatter(TypedefWrapper, "Typedef wrapper %?.");
BOOST_AUTO_TEST_CASE ( typedefWrapper )
{
	TypedefWrapper::write(*this, "expr");
	BOOST_CHECK_EQUAL(this->str(), "Typedef wrapper expr.");
}

BOOST_AUTO_TEST_CASE ( longFormatString )
{
	Formatter<formatStringLong>::write(*this);
	BOOST_CHECK_EQUAL(this->str().length(), 246);
}

BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_CASE ( customMultiArgRightAlign )
{
	std::stringstream buf1, buf2, buf3;
	const int width = 20;
	Formatter<formatStringMultiArg>::write(buf1, width, "something");
	Formatter<formatStringMultiArg>::write(buf2, width, "something else");
	Formatter<formatStringMultiArg>::write(buf3, width, 123.45);
	BOOST_CHECK_EQUAL(buf1.str(), "value           something");
	BOOST_CHECK_EQUAL(buf2.str(), "value      something else");
	BOOST_CHECK_EQUAL(buf3.str(), "value              123.45");
}

BOOST_AUTO_TEST_CASE ( get )
{
	auto s = Formatter<formatStringMultiArg>::get(20, "something else");
	BOOST_CHECK_EQUAL(s, "value      something else");
}

constexpr
#include <lorem-ipsum.h>
BOOST_AUTO_TEST_CASE( lorem_ipsum )
{
	typedef Formatter<lorem_ipsum_txt, sizeof(lorem_ipsum_txt)> LIF;
	auto s = LIF::get();
	BOOST_CHECK_EQUAL(s.length(), lorem_ipsum_txt_len);
	AdHoc::FileUtils::MemMap li(rootDir / "lorem-ipsum.txt");
	auto sv = li.sv<std::decay<decltype(*lorem_ipsum_txt)>::type>();
	BOOST_CHECK_EQUAL_COLLECTIONS(s.begin(), s.end(), sv.begin(), sv.end());
}

namespace AdHoc {
	template<>
	inline void appendStream(FILE & strm, const char * const p, size_t n)
	{
		BOOST_VERIFY(fwrite(p, n, 1, &strm) == 1);
	}
}

FILE &
operator<<(FILE & strm, const char * const p)
{
	BOOST_VERIFY(fputs(p, &strm) != EOF);
	return strm;
}

BOOST_AUTO_TEST_CASE( filestar )
{
	char * buf = NULL;
	size_t len = 0;
	FILE * strm = open_memstream(&buf, &len);
	BOOST_REQUIRE(strm);
	Formatter<formatStringMulti>::write(*strm, "file", "star");
	fclose(strm);
	BOOST_CHECK_EQUAL(len, 22);
	BOOST_CHECK_EQUAL(buf, "First file, then star.");
	free(buf);
}

// The following tests represent CTF's [partial] emulation of many
// POSIX formatting features
#define GLIBC_FMT_TEST(NAME, FMT, ...) \
	AdHocFormatter(NAME ## fmtr, FMT); \
	BOOST_AUTO_TEST_CASE(NAME ## t) { \
		BOOST_TEST_CONTEXT(FMT) { \
			auto str = NAME ## fmtr::get(__VA_ARGS__); \
			char * buf = NULL; \
			int len = asprintf(&buf, FMT, __VA_ARGS__); \
			BOOST_REQUIRE(buf); \
			BOOST_CHECK_EQUAL(str.length(), len); \
			BOOST_CHECK_EQUAL(str, buf); \
			free(buf); \
		} \
	}

GLIBC_FMT_TEST(s1, "in %s.", "string");
GLIBC_FMT_TEST(s2, "in %s %s.", "string", "other");
GLIBC_FMT_TEST(s3, "in %.*s.", 3, "other");
GLIBC_FMT_TEST(s4, "in %.*s.", 5, "other");
GLIBC_FMT_TEST(s5, "in %.*s.", 7, "other");

GLIBC_FMT_TEST(c1, "in %c.", 'b');
GLIBC_FMT_TEST(c2, "in %c.", 'B');

GLIBC_FMT_TEST(d1, "in %d.", 123);
GLIBC_FMT_TEST(d2, "in %d.", 123456);
GLIBC_FMT_TEST(d3, "in %hd.", (int16_t)-12345);
GLIBC_FMT_TEST(d4, "in %hhd.", (int8_t)-123);
GLIBC_FMT_TEST(d5, "in %ld.", -123456L);
GLIBC_FMT_TEST(d6, "in %lld.", -123456LL);
GLIBC_FMT_TEST(i1, "in %i.", 123);
GLIBC_FMT_TEST(i2, "in %i.", -123);

GLIBC_FMT_TEST(x1, "in %x.", 123);
GLIBC_FMT_TEST(x2, "in %x %d.", 123, 256);
GLIBC_FMT_TEST(x3, "in %d %x.", 123, 1024);
GLIBC_FMT_TEST(x4, "in %X %x.", 123, 13);
GLIBC_FMT_TEST(x5, "in %X %s.", 123, "miXED case after UPPER X");
GLIBC_FMT_TEST(x6, "in %#x.", 123);
GLIBC_FMT_TEST(x7, "in %#X.", 123);
GLIBC_FMT_TEST(x8, "in %#X %x.", 123, 150);

GLIBC_FMT_TEST(o1, "in %o.", 123);
GLIBC_FMT_TEST(o2, "in %o %d.", 123, 256);
GLIBC_FMT_TEST(o3, "in %d %o.", 123, 1024);

GLIBC_FMT_TEST(a1, "in %a.", 123.456789);
GLIBC_FMT_TEST(a2, "in %a.", -123.456789);
GLIBC_FMT_TEST(a3, "in %a.", .123456789);
GLIBC_FMT_TEST(a4, "in %a.", -.123456789);
GLIBC_FMT_TEST(a5, "in %A.", -.123456789);
GLIBC_FMT_TEST(a6, "in %A.", -123.456789);
GLIBC_FMT_TEST(a7, "in %a.", 123456789.123);
GLIBC_FMT_TEST(a8, "in %a.", -123456789.123);

GLIBC_FMT_TEST(e1, "in %e.", 123.456789);
GLIBC_FMT_TEST(e2, "in %e.", -123.456789);
GLIBC_FMT_TEST(e3, "in %e.", .123456789);
GLIBC_FMT_TEST(e4, "in %e.", -.123456789);
GLIBC_FMT_TEST(e5, "in %E.", -.123456789);
GLIBC_FMT_TEST(e6, "in %E.", -123.456789);
GLIBC_FMT_TEST(e7, "in %e.", 123456789.123);
GLIBC_FMT_TEST(e8, "in %e.", -123456789.123);

GLIBC_FMT_TEST(f1, "in %f.", 123.456789);
GLIBC_FMT_TEST(f2, "in %f.", -123.456789);
GLIBC_FMT_TEST(f3, "in %f.", .123456789);
GLIBC_FMT_TEST(f4, "in %f.", -.123456789);
GLIBC_FMT_TEST(f5, "in %F.", -.123456789);
GLIBC_FMT_TEST(f6, "in %F.", -123.456789);
GLIBC_FMT_TEST(f7, "in %f.", 123456789.123);
GLIBC_FMT_TEST(f8, "in %f.", -123456789.123);

GLIBC_FMT_TEST(g1, "in %g.", 123.456789);
GLIBC_FMT_TEST(g2, "in %g.", -123.456789);
GLIBC_FMT_TEST(g3, "in %g.", .123456789);
GLIBC_FMT_TEST(g4, "in %g.", -.123456789);
GLIBC_FMT_TEST(g5, "in %G.", -.123456789);
GLIBC_FMT_TEST(g6, "in %G.", -123.456789);
GLIBC_FMT_TEST(g7, "in %g.", 123456789.123);
GLIBC_FMT_TEST(g8, "in %g.", -123456789.123);

AdHocFormatter(chars_written_fmt, "%n %s %n %d %n");
BOOST_AUTO_TEST_CASE(chars_written)
{
	int a = -1, b = -1, c = -1;
	auto s = chars_written_fmt::get(&a, "some string", &b, 10, &c);
	BOOST_CHECK_EQUAL(s, " some string  10 ");
	BOOST_CHECK_EQUAL(a, 0);
	BOOST_CHECK_EQUAL(b, s.length() - 4);
	BOOST_CHECK_EQUAL(c, s.length());
}

GLIBC_FMT_TEST(p2, "in %p.", this);

AdHocFormatter(smartptr_fmt, "Address is %p.");
BOOST_AUTO_TEST_CASE(smartptr)
{
	auto uni = std::make_unique<int>(42);
	smartptr_fmt::get(uni);
	auto shrd = std::make_shared<int>(42);
	smartptr_fmt::get(shrd);
}

