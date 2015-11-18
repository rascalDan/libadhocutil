#define BOOST_TEST_MODULE Semaphore
#include <boost/test/unit_test.hpp>

#include <thread>
#include <semaphore.h>

BOOST_AUTO_TEST_CASE( addRemoveOne )
{
	AdHoc::Semaphore s;
	s.notify();
	s.wait();
}

BOOST_AUTO_TEST_CASE( initial )
{
	AdHoc::Semaphore s(2);
	s.wait();
	s.wait();
	BOOST_REQUIRE_EQUAL(false, s.wait(0));
}

BOOST_AUTO_TEST_CASE( addRemoveSome )
{
	AdHoc::Semaphore s;
	s.notify();
	s.notify();
	s.notify();
	s.wait();
	s.wait();
	s.wait();
}

BOOST_AUTO_TEST_CASE( addRemoveTimeOut )
{
	AdHoc::Semaphore s;
	s.notify();
	s.wait();
	BOOST_REQUIRE_EQUAL(false, s.wait(100));
	BOOST_REQUIRE_EQUAL(false, s.wait(0));
}

BOOST_AUTO_TEST_CASE( addRemoveWait )
{
	AdHoc::Semaphore s;
	s.notify();
	s.wait();
	std::thread th([&s]() { usleep(100000); s.notify(); });
	BOOST_REQUIRE_EQUAL(false, s.wait(1));
	s.wait();
	th.join();
}

