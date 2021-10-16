#define BOOST_TEST_MODULE ResourcePool
#include <boost/test/unit_test.hpp>

#include "c++11Helpers.h"
#include "lockHelpers.h"
#include "resourcePool.impl.h"
#ifdef __cpp_lib_semaphore
#	include <semaphore>
#else
#	include "polyfill-semaphore.h" // IWYU pragma: keep
#endif
#include <atomic>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unistd.h>
#include <utility>

class MockResource {
public:
	MockResource() : id(ids++)
	{
		count += 1;
	}
	~MockResource()
	{
		count -= 1;
	}

	SPECIAL_MEMBERS_DELETE(MockResource);

	[[nodiscard]] bool
	valid() const
	{
		return true;
	}

	const unsigned int id;
	static std::atomic<unsigned int> ids;
	static std::atomic<unsigned int> count;
};

std::atomic<unsigned int> MockResource::ids;
std::atomic<unsigned int> MockResource::count;

class TRP : public AdHoc::ResourcePool<MockResource> {
public:
	TRP() : AdHoc::ResourcePool<MockResource>(10, 10) { }

protected:
	std::shared_ptr<MockResource>
	createResource() const override
	{
		return std::make_shared<MockResource>();
	}
};

class TRPSmall : public AdHoc::ResourcePool<MockResource> {
public:
	TRPSmall() : AdHoc::ResourcePool<MockResource>(3, 1) { }

protected:
	std::shared_ptr<MockResource>
	createResource() const override
	{
		return std::make_shared<MockResource>();
	}
};

class TRPCreateFail : public TRPSmall {
protected:
	std::shared_ptr<MockResource>
	createResource() const override
	{
		throw std::exception();
	}
};

class TRPReturnFail : public TRPSmall {
protected:
	void
	returnTestResource(MockResource const *) const override
	{
		throw std::exception();
	}
};

BOOST_AUTO_TEST_CASE(get)
{
	{
		TRP pool;
		BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());

		{
			auto r1 = pool.get();
			BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
			BOOST_REQUIRE_EQUAL(0, pool.availableCount());
			BOOST_REQUIRE_EQUAL(1, r1.handleCount());
			BOOST_REQUIRE_EQUAL(1, MockResource::count);
			BOOST_REQUIRE(r1.get());

			auto r2(pool.get());
			BOOST_REQUIRE_EQUAL(2, pool.inUseCount());
			BOOST_REQUIRE_EQUAL(0, pool.availableCount());
			BOOST_REQUIRE_EQUAL(1, r2.handleCount());
			BOOST_REQUIRE_EQUAL(2, MockResource::count);
			BOOST_REQUIRE(r2.get());

			auto r1a = r2;
			BOOST_CHECK_EQUAL(2, r1a.handleCount());
			r1a = r1;
			BOOST_REQUIRE_EQUAL(2, pool.inUseCount());
			BOOST_REQUIRE_EQUAL(0, pool.availableCount());
			BOOST_REQUIRE_EQUAL(2, r1.handleCount());
			BOOST_REQUIRE_EQUAL(2, r1a.handleCount());
			BOOST_REQUIRE_EQUAL(2, MockResource::count);
			BOOST_REQUIRE(r1.get());
			BOOST_REQUIRE(r1a.get());
			BOOST_REQUIRE_EQUAL(r1.get(), r1a.get());

			BOOST_REQUIRE(r1a);
			r1a.release();
			if (!r1a) {
				BOOST_REQUIRE(true);
			}
			BOOST_REQUIRE(!r1a);
			r1a = r2;
			BOOST_REQUIRE(r1a);
			BOOST_REQUIRE_EQUAL(r2.get(), r1a.get());
			if (r1a) {
				BOOST_REQUIRE(true);
			}
		}

		BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(2, pool.availableCount());
	}
	BOOST_REQUIRE_EQUAL(0, MockResource::count);
}

BOOST_AUTO_TEST_CASE(destroyPoolWhenInUse)
{
	auto rp = new TRP();
	auto rh1 = rp->get();
	auto rh2 = rp->get();
	// NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
	auto rh3 = rh1;
	delete rp;
	BOOST_REQUIRE(rh1->valid());
	BOOST_REQUIRE(rh2->valid());
	BOOST_REQUIRE(rh3->valid());
}

BOOST_AUTO_TEST_CASE(getMine)
{
	TRP pool;
	auto r1 = pool.get();
	BOOST_REQUIRE(r1.get());
	auto r2 = pool.getMine();
	BOOST_REQUIRE_EQUAL(r1.get()->id, r2.get()->id);
	BOOST_REQUIRE_EQUAL(2, r1.handleCount());
	BOOST_REQUIRE_EQUAL(2, r2.handleCount());
}

BOOST_AUTO_TEST_CASE(getMineNoCurrent)
{
	TRP pool;
	BOOST_REQUIRE_THROW(pool.getMine(), AdHoc::NoCurrentResourceT<MockResource>);
	{
		auto r1 = pool.get();
		auto r2 = pool.getMine();
		BOOST_REQUIRE_EQUAL(r1.get()->id, r2.get()->id);
	}
	BOOST_REQUIRE_THROW(pool.getMine(), AdHoc::NoCurrentResource);
}

BOOST_AUTO_TEST_CASE(move)
{
	TRP pool;
	{
		auto r1 = pool.get();
		BOOST_CHECK_EQUAL(pool.inUseCount(), 1);
		BOOST_CHECK(r1);
		{
			auto r2(std::move(r1));
			BOOST_CHECK_EQUAL(pool.inUseCount(), 1);
			BOOST_CHECK(!r1);
			BOOST_CHECK(r2);

			r1 = std::move(r2);
			BOOST_CHECK_EQUAL(pool.inUseCount(), 1);
			BOOST_CHECK(r1);
			BOOST_CHECK(!r2);

			r2 = pool.get();
			BOOST_CHECK_EQUAL(pool.inUseCount(), 2);
			BOOST_CHECK(r2);

			r1 = std::move(r2);
			BOOST_CHECK_EQUAL(pool.inUseCount(), 1);
			BOOST_CHECK(r1);
			BOOST_CHECK(!r2);
		}
		BOOST_CHECK_EQUAL(pool.inUseCount(), 1);
	}
	BOOST_CHECK_EQUAL(pool.inUseCount(), 0);
}

BOOST_AUTO_TEST_CASE(discard)
{
	TRP pool;
	try {
		auto r1 = pool.get();
		BOOST_REQUIRE(r1);
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		throw std::exception();
	}
	catch (...) {
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	}
}

BOOST_AUTO_TEST_CASE(keepSome1)
{
	TRPSmall pool;
	{
		auto r1 = pool.get();
		BOOST_CHECK(r1);
		{
			auto r2 = pool.get();
			BOOST_CHECK(r2);
			{
				auto r3 = pool.get();
				BOOST_CHECK(r3);
				BOOST_REQUIRE_EQUAL(3, pool.inUseCount());
				BOOST_REQUIRE_EQUAL(0, pool.availableCount());
				BOOST_REQUIRE_EQUAL(3, MockResource::count);
			}
			BOOST_REQUIRE_EQUAL(2, pool.inUseCount());
			BOOST_REQUIRE_EQUAL(1, pool.availableCount());
			BOOST_REQUIRE_EQUAL(3, MockResource::count);
		}
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(1, pool.availableCount());
		BOOST_REQUIRE_EQUAL(2, MockResource::count);
		{
			auto r2 = pool.get();
			BOOST_CHECK(r2);
			BOOST_REQUIRE_EQUAL(2, pool.inUseCount());
			BOOST_REQUIRE_EQUAL(0, pool.availableCount());
			BOOST_REQUIRE_EQUAL(2, MockResource::count);
		}
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(1, pool.availableCount());
		BOOST_REQUIRE_EQUAL(2, MockResource::count);
	}
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
	BOOST_REQUIRE_EQUAL(1, MockResource::count);
}

BOOST_AUTO_TEST_CASE(keepSome2)
{
	TRPSmall pool;
	{
		auto r1 = pool.get();
		BOOST_CHECK(r1);
		auto r2 = pool.get();
		BOOST_CHECK(r2);
		auto r3 = pool.get();
		BOOST_CHECK(r3);
		BOOST_REQUIRE_EQUAL(3, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(3, MockResource::count);
	}
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
	BOOST_REQUIRE_EQUAL(1, MockResource::count);
}

BOOST_AUTO_TEST_CASE(idle)
{
	TRP pool;
	{
		{
			auto r1 = pool.get();
			BOOST_CHECK(r1);
			auto r2 = pool.get();
			BOOST_CHECK(r2);
		}
		auto r3 = pool.get();
		BOOST_CHECK(r3);
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(1, pool.availableCount());
		BOOST_REQUIRE_EQUAL(2, MockResource::count);
		pool.idle();
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(1, MockResource::count);
	}
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
	BOOST_REQUIRE_EQUAL(1, MockResource::count);
	pool.idle();
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, MockResource::count);
}

BOOST_AUTO_TEST_CASE(threading1, *boost::unit_test::timeout(10))
{
	TRPSmall pool;
	std::list<std::thread> threads;
	for (int x = 0; x < 100; x += 1) {
		threads.emplace_back([&pool]() {
			auto r = pool.get();
			usleep(50000);
		});
		usleep(5000);
		// pool size never exceeds 3
		BOOST_REQUIRE_LE(pool.inUseCount(), 3);
	}
	for (auto & thread : threads) {
		thread.join();
	}
	// pool keep returns to 1
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
}

static void
acquireAndKeepFor1Second(TRPSmall * pool, AdHoc::ResourcePoolBase::SemaphoreType & s)
{
	auto r = pool->get();
	static std::mutex m;
	ScopeLock(m) {
		BOOST_CHECK(r);
	}
	s.release();
	sleep(1);
}

BOOST_AUTO_TEST_CASE(threading2)
{
	TRPSmall pool;
	AdHoc::ResourcePoolBase::SemaphoreType s {0};
	std::thread t1([&pool, &s]() {
		acquireAndKeepFor1Second(&pool, s);
	});
	std::thread t2([&pool, &s]() {
		acquireAndKeepFor1Second(&pool, s);
	});
	std::thread t3([&pool, &s]() {
		acquireAndKeepFor1Second(&pool, s);
	});

	s.acquire();
	s.acquire();
	s.acquire();
	BOOST_REQUIRE_THROW(pool.get(100), AdHoc::TimeOutOnResourcePoolT<MockResource>);
	BOOST_REQUIRE_EQUAL(3, pool.inUseCount());

	t1.join();
	{
		auto r = pool.get(0);
		t2.join();
		t3.join();
	}

	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
}

class TTRP : public TRP {
public:
	void
	testResource(MockResource const *) const override
	{
		n += 1;
		if (n % 2) {
			throw std::exception();
		}
	}

private:
	mutable int n {0};
};

BOOST_AUTO_TEST_CASE(test)
{
	TTRP pool;
	unsigned int rpId;
	{
		auto r = pool.get();
		rpId = r.get()->id;
	}
	{
		auto r = pool.get();
		BOOST_REQUIRE(r.get());
		BOOST_REQUIRE_NE(rpId, r.get()->id);
		BOOST_REQUIRE_EQUAL(1, MockResource::count);
		rpId = r.get()->id;
	}
	{
		auto r = pool.get();
		BOOST_REQUIRE(r.get());
		BOOST_REQUIRE_EQUAL(rpId, r.get()->id);
		BOOST_REQUIRE_EQUAL(1, MockResource::count);
	}
}

BOOST_AUTO_TEST_CASE(createFail)
{
	TRPCreateFail pool;
	BOOST_REQUIRE_EQUAL(0, MockResource::count);
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_THROW(pool.get(), std::exception);
	BOOST_REQUIRE_THROW(pool.get(0), std::exception);
	BOOST_REQUIRE_THROW(pool.get(100), std::exception);
	BOOST_REQUIRE_EQUAL(0, MockResource::count);
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_THROW(pool.get(), std::exception);
	BOOST_REQUIRE_THROW(pool.get(), std::exception);
}

BOOST_AUTO_TEST_CASE(returnFail)
{
	TRPReturnFail pool;
	{
		auto rh = pool.get();
		BOOST_CHECK(rh);
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
	}
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
}

BOOST_AUTO_TEST_CASE(exception_msgs)
{
	BOOST_CHECK_NO_THROW(AdHoc::TimeOutOnResourcePool("foo").message());
	BOOST_CHECK_NO_THROW(AdHoc::NoCurrentResource(std::this_thread::get_id(), "foo").message());
}
