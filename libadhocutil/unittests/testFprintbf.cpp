#define BOOST_TEST_MODULE fprintbf
#include <boost/test/unit_test.hpp>

#include "fprintbf.h"
#include "definedDirs.h"
#include <system_error>

BOOST_AUTO_TEST_CASE( writestring )
{
	FILE * f = fopen(binDir / "writestring", "w");
	BOOST_REQUIRE(f);
	BOOST_REQUIRE_EQUAL(fprintss(f, std::string("some string")), 11);
	fclose(f);
}

BOOST_AUTO_TEST_CASE( writestringerror )
{
	FILE * f = fopen(binDir / "writestring", "r");
	BOOST_REQUIRE(f);
	BOOST_REQUIRE_THROW(fprintss(f, "some string"), std::system_error);
	fclose(f);
}

BOOST_AUTO_TEST_CASE( writeformatNoArgs )
{
	FILE * f = fopen(binDir / "writeformatNoArgs", "w");
	BOOST_REQUIRE(f);
	BOOST_REQUIRE_EQUAL(fprintbf(f, "some string"), 11);
	fclose(f);
}

BOOST_AUTO_TEST_CASE( writeformatOneArg )
{
	FILE * f = fopen(binDir / "writeformatOneArg", "w");
	BOOST_REQUIRE(f);
	BOOST_REQUIRE_EQUAL(fprintbf(f, "some %s", "string"), 11);
	fclose(f);
}

BOOST_AUTO_TEST_CASE( writeformatTwoArgs )
{
	FILE * f = fopen(binDir / "writeformatTwoArgs", "w");
	BOOST_REQUIRE(f);
	BOOST_REQUIRE_EQUAL(fprintbf(f, "s%dme %s", 0, "string"), 11);
	fclose(f);
}

BOOST_AUTO_TEST_CASE( writeformatError )
{
	FILE * f = fopen(binDir / "writeformatTwoArgs", "r");
	BOOST_REQUIRE(f);
	BOOST_REQUIRE_THROW(fprintbf(f, "s%dme %s", 0, "string"), std::system_error);
	fclose(f);
}

BOOST_AUTO_TEST_CASE( fopenerror )
{
	BOOST_REQUIRE_THROW(fopen(binDir / "missing" / "folder", "r"), std::system_error);
}

