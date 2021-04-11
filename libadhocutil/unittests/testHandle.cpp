#define BOOST_TEST_MODULE Handle
#include <boost/test/unit_test.hpp>

#include "handle.h"
#include <cerrno>
#include <fcntl.h>
#include <type_traits>
#include <unistd.h>
#include <utility>

// Test case base on a file handle
using T = decltype(STDIN_FILENO);
using D = decltype(&close);
static_assert(std::is_trivially_move_constructible_v<T>);
static_assert(std::is_trivially_move_constructible_v<D>);

using TestHandle = AdHoc::Handle<T, D>;

static_assert(std::is_nothrow_move_constructible_v<TestHandle>);
static_assert(std::is_nothrow_move_assignable_v<TestHandle>);
static_assert(!std::is_copy_constructible_v<TestHandle>);
static_assert(!std::is_copy_assignable_v<TestHandle>);

BOOST_AUTO_TEST_CASE(values)
{
	TestHandle th {open("/dev/null", O_RDWR), &close};
	static_assert(std::is_same_v<T, std::remove_reference_t<decltype(th.get())>>);
	static_assert(std::is_same_v<T, std::remove_reference_t<decltype(*th)>>);
	BOOST_REQUIRE_EQUAL(1, write(th, "1", 1));
}

BOOST_AUTO_TEST_CASE(values_const)
{
	const TestHandle cth {open("/dev/null", O_RDWR), &close};
	static_assert(std::is_same_v<const T, std::remove_reference_t<decltype(cth.get())>>);
	static_assert(std::is_same_v<const T, std::remove_reference_t<decltype(*cth)>>);
	BOOST_REQUIRE_EQUAL(1, write(cth, "1", 1));
}

BOOST_AUTO_TEST_CASE(make)
{
	bool closed = false;
	int fd = -2;
	const auto doClose = [&](T lfd) {
		BOOST_CHECK(!closed);
		BOOST_CHECK_EQUAL(lfd, fd);
		closed = true;
		return close(lfd);
	};
	{
		auto th = AdHoc::make_handle<T>(doClose, open("/dev/null", O_RDWR));
		BOOST_REQUIRE(!closed);
		BOOST_REQUIRE_EQUAL(1, write(th, "1", 1));
		auto th2 = std::move(th);
		BOOST_REQUIRE(!closed);
		BOOST_REQUIRE_EQUAL(1, write(th2, "1", 1));
		fd = th2;
	}
	BOOST_REQUIRE(closed);
	BOOST_REQUIRE_EQUAL(-1, write(fd, "1", 1));
	BOOST_REQUIRE_EQUAL(errno, EBADF);
}
