#define BOOST_TEST_MODULE Semaphore
#include <boost/test/unit_test.hpp>

#include <polyfill-semaphore.h>
#include <thread>
#include <unistd.h>

BOOST_AUTO_TEST_CASE(addRemoveOne)
{
	AdHoc::Semaphore s {0};
	s.release();
	s.acquire();
}

BOOST_AUTO_TEST_CASE(initial)
{
	AdHoc::Semaphore s(2);
	s.acquire();
	s.acquire();
	// cppcheck-suppress assertWithSideEffect
	BOOST_REQUIRE_EQUAL(false, s.try_acquire_for(0));
}

BOOST_AUTO_TEST_CASE(addRemoveSome)
{
	AdHoc::Semaphore s {0};
	BOOST_REQUIRE_EQUAL(0, s.freeCount());
	s.release();
	BOOST_REQUIRE_EQUAL(1, s.freeCount());
	s.release();
	BOOST_REQUIRE_EQUAL(2, s.freeCount());
	s.release();
	BOOST_REQUIRE_EQUAL(3, s.freeCount());
	s.acquire();
	BOOST_REQUIRE_EQUAL(2, s.freeCount());
	s.acquire();
	BOOST_REQUIRE_EQUAL(1, s.freeCount());
	s.acquire();
	BOOST_REQUIRE_EQUAL(0, s.freeCount());
}

BOOST_AUTO_TEST_CASE(addRemoveTimeOut)
{
	AdHoc::Semaphore s {0};
	s.release();
	s.acquire();
	// cppcheck-suppress assertWithSideEffect
	BOOST_REQUIRE_EQUAL(false, s.try_acquire_for(100));
	// cppcheck-suppress assertWithSideEffect
	BOOST_REQUIRE_EQUAL(false, s.try_acquire_for(0));
}

BOOST_AUTO_TEST_CASE(addRemoveWait)
{
	AdHoc::Semaphore s {0};
	s.release();
	s.acquire();
	std::thread th([&s]() {
		usleep(100000);
		s.release();
	});
	// cppcheck-suppress assertWithSideEffect
	BOOST_REQUIRE_EQUAL(false, s.try_acquire_for(1));
	s.acquire();
	th.join();
}
