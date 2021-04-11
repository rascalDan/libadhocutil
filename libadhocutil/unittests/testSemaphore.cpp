#define BOOST_TEST_MODULE Semaphore
#include <boost/test/unit_test.hpp>

#include <semaphore.h>
#include <thread>
#include <unistd.h>

BOOST_AUTO_TEST_CASE(addRemoveOne)
{
	AdHoc::Semaphore s;
	s.notify();
	s.wait();
}

BOOST_AUTO_TEST_CASE(initial)
{
	AdHoc::Semaphore s(2);
	s.wait();
	s.wait();
	BOOST_REQUIRE_EQUAL(false, s.wait(0));
}

BOOST_AUTO_TEST_CASE(addRemoveSome)
{
	AdHoc::Semaphore s;
	BOOST_REQUIRE_EQUAL(0, s.freeCount());
	s.notify();
	BOOST_REQUIRE_EQUAL(1, s.freeCount());
	s.notify();
	BOOST_REQUIRE_EQUAL(2, s.freeCount());
	s.notify();
	BOOST_REQUIRE_EQUAL(3, s.freeCount());
	s.wait();
	BOOST_REQUIRE_EQUAL(2, s.freeCount());
	s.wait();
	BOOST_REQUIRE_EQUAL(1, s.freeCount());
	s.wait();
	BOOST_REQUIRE_EQUAL(0, s.freeCount());
}

BOOST_AUTO_TEST_CASE(addRemoveTimeOut)
{
	AdHoc::Semaphore s;
	s.notify();
	s.wait();
	BOOST_REQUIRE_EQUAL(false, s.wait(100));
	BOOST_REQUIRE_EQUAL(false, s.wait(0));
}

BOOST_AUTO_TEST_CASE(addRemoveWait)
{
	AdHoc::Semaphore s;
	s.notify();
	s.wait();
	std::thread th([&s]() {
		usleep(100000);
		s.notify();
	});
	BOOST_REQUIRE_EQUAL(false, s.wait(1));
	s.wait();
	th.join();
}
