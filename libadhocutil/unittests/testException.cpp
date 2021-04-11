#define BOOST_TEST_MODULE Excetion
#include <boost/test/unit_test.hpp>

#include <buffer.h>
#include <cstdlib>
#include <exception.h>
#include <stdexcept>
#include <string>

using namespace AdHoc;

class Ex1 : public AdHoc::StdException {
public:
	Ex1(const char * f, int l) : file(f), line(l) { }

private:
	std::string
	message() const noexcept override
	{
		return stringf("Something something at %s:%d", file, line);
	}

	const char * file;
	const int line;
};

class OtherBaseException : public std::exception {
};

class Ex2 : public AdHoc::Exception<OtherBaseException> {
public:
	Ex2(const char * f, int l) : file(f), line(l) { }

private:
	std::string
	message() const noexcept override
	{
		return stringf("Something other something at %s:%d", file, line);
	}

	const char * file;
	const int line;
};

class Ex3 : public AdHoc::StdException {
private:
	// LCOV_EXCL_START
	std::string
	message() const noexcept override
	{
		// Never called
		std::abort();
	}
	// LCOV_EXCL_STOP
};

void
failing1()
{
	throw Ex1(__PRETTY_FUNCTION__, 1);
}

void
failing2()
{
	throw Ex2(__PRETTY_FUNCTION__, 2);
}

void
failing3()
{
	throw Ex3();
}

BOOST_AUTO_TEST_CASE(throwCatch)
{
	BOOST_REQUIRE_THROW(failing1(), Ex1);
	BOOST_REQUIRE_THROW(failing1(), std::exception);

	BOOST_REQUIRE_THROW(failing2(), Ex2);
	BOOST_REQUIRE_THROW(failing2(), OtherBaseException);
	BOOST_REQUIRE_THROW(failing2(), std::exception);

	BOOST_REQUIRE_THROW(failing3(), Ex3);
	BOOST_REQUIRE_THROW(failing3(), std::exception);
}

BOOST_AUTO_TEST_CASE(message1)
{
	try {
		failing1();
	}
	catch (const std::exception & ex) {
		BOOST_REQUIRE_EQUAL("Something something at void failing1():1", ex.what());
	}
}

BOOST_AUTO_TEST_CASE(message2)
{
	try {
		failing2();
	}
	catch (const std::exception & ex) {
		BOOST_REQUIRE_EQUAL("Something other something at void failing2():2", ex.what());
	}
}
