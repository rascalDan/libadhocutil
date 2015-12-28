#define BOOST_TEST_MODULE ResourcePool
#include <boost/test/unit_test.hpp>

#include <resourcePool.impl.h>

class MockResource {
	public:
		MockResource() { count += 1; }
		~MockResource() { count -= 1; }

		MockResource(const MockResource &) = delete;
		void operator=(const MockResource &) = delete;

		bool valid() const { return true; }

		static std::atomic<unsigned int> count;
};

std::atomic<unsigned int> MockResource::count;

class TRP : public AdHoc::ResourcePool<MockResource> {
	public:
		TRP() : AdHoc::ResourcePool<MockResource>(10, 10) { }
	protected:
		MockResource * createResource() const override
		{
			return new MockResource();
		}
};

class TRPSmall : public AdHoc::ResourcePool<MockResource> {
	public:
		TRPSmall() : AdHoc::ResourcePool<MockResource>(3, 1) { }
	protected:
		MockResource * createResource() const override
		{
			return new MockResource();
		}
};

class TRPCreateFail : public TRPSmall {
	protected:
		MockResource * createResource() const override
		{
			throw std::exception();
		}
};

class TRPReturnFail : public TRPSmall {
	protected:
		void returnTestResource(const MockResource *) const override
		{
			throw std::exception();
		}
};

BOOST_AUTO_TEST_CASE ( get )
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
			BOOST_REQUIRE_THROW(r1a.get(), std::runtime_error);
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

BOOST_AUTO_TEST_CASE( destroyPoolWhenInUse )
{
	auto rp = new TRP();
	auto rh1 = rp->get();
	auto rh2 = rp->get();
	auto rh3 = rh1;
	delete rp;
	BOOST_REQUIRE(rh1->valid());
	BOOST_REQUIRE(rh2->valid());
	BOOST_REQUIRE(rh3->valid());
}

BOOST_AUTO_TEST_CASE ( getMine )
{
	TRP pool;
	auto r1 = pool.get();
	BOOST_REQUIRE(r1.get());
	auto r2 = pool.getMine();
	BOOST_REQUIRE_EQUAL(r1.get(), r2.get());
	BOOST_REQUIRE_EQUAL(2, r1.handleCount());
	BOOST_REQUIRE_EQUAL(2, r2.handleCount());
}

BOOST_AUTO_TEST_CASE( getMineNoCurrent )
{
	TRP pool;
	BOOST_REQUIRE_THROW(pool.getMine(), AdHoc::NoCurrentResourceT<MockResource>);
	{
		auto r1 = pool.get();
		auto r2 = pool.getMine();
		BOOST_REQUIRE_EQUAL(r1.get(), r2.get());
	}
	BOOST_REQUIRE_THROW(pool.getMine(), AdHoc::NoCurrentResource);
}

BOOST_AUTO_TEST_CASE( discard )
{
	TRP pool;
	try {
		auto r1 = pool.get();
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		throw std::exception();
	}
	catch (...) {
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	}
}

BOOST_AUTO_TEST_CASE( keepSome1 )
{
	TRPSmall pool;
	{
		auto r1 = pool.get();
		{
			auto r2 = pool.get();
			{
				auto r3 = pool.get();
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

BOOST_AUTO_TEST_CASE( keepSome2 )
{
	TRPSmall pool;
	{
		auto r1 = pool.get();
		auto r2 = pool.get();
		auto r3 = pool.get();
		BOOST_REQUIRE_EQUAL(3, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(3, MockResource::count);
	}
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
	BOOST_REQUIRE_EQUAL(1, MockResource::count);
}

BOOST_AUTO_TEST_CASE( idle )
{
	TRP pool;
	{
		{
			auto r1 = pool.get();
			auto r2 = pool.get();
		}
		auto r3 = pool.get();
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

BOOST_AUTO_TEST_CASE( threading1 )
{
	TRPSmall pool;
	std::list<std::thread *> threads;
	for (int x = 0; x < 100; x += 1) {
		threads.push_back(new std::thread([&pool](){
			auto r = pool.get();
			usleep(50000);
		}));
		usleep(5000);
		// pool size never exceeds 3
		BOOST_REQUIRE(pool.inUseCount() <= 3);
	}
	for(std::thread * thread : threads) {
		thread->join();
		delete thread;
	}
	// pool keep returns to 1
	BOOST_REQUIRE_EQUAL(1, pool.availableCount());
}

static
void
acquireAndKeepFor1Second(TRPSmall * pool, AdHoc::Semaphore & s)
{
	auto r = pool->get();
	s.notify();
	sleep(1);
}

BOOST_AUTO_TEST_CASE( threading2 )
{
	TRPSmall pool;
	AdHoc::Semaphore s;
	std::thread t1([&pool, &s]() { acquireAndKeepFor1Second(&pool, s); });
	std::thread t2([&pool, &s]() { acquireAndKeepFor1Second(&pool, s); });
	std::thread t3([&pool, &s]() { acquireAndKeepFor1Second(&pool, s); });

	s.wait();
	s.wait();
	s.wait();
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
		TTRP() : n(0) { }
		void testResource(const MockResource *) const override
		{
			n += 1;
			if (n % 2) {
				throw std::exception();
			}
		}
	private:
		mutable int n;
};

BOOST_AUTO_TEST_CASE( test )
{
	TTRP pool;
	MockResource * rp;
	{
		auto r = pool.get();
		rp = r.get();
	}
	{
		auto r = pool.get();
		BOOST_REQUIRE(r.get());
		BOOST_REQUIRE(rp != r.get());
		BOOST_REQUIRE_EQUAL(1, MockResource::count);
		rp = r.get();
	}
	{
		auto r = pool.get();
		BOOST_REQUIRE(r.get());
		BOOST_REQUIRE(rp == r.get());
		BOOST_REQUIRE_EQUAL(1, MockResource::count);
	}
}

BOOST_AUTO_TEST_CASE( createFail )
{
	TRPCreateFail pool;
	BOOST_REQUIRE_EQUAL(0, MockResource::count);
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
	BOOST_REQUIRE_THROW(pool.get(), std::exception);
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
	BOOST_REQUIRE_THROW(pool.get(0), std::exception);
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
	BOOST_REQUIRE_THROW(pool.get(100), std::exception);
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
	BOOST_REQUIRE_EQUAL(0, MockResource::count);
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_THROW(pool.get(), std::exception);
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
	BOOST_REQUIRE_THROW(pool.get(), std::exception);
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
}

BOOST_AUTO_TEST_CASE( returnFail )
{
	TRPReturnFail pool;
	{
		auto rh = pool.get();
		BOOST_REQUIRE_EQUAL(0, pool.availableCount());
		BOOST_REQUIRE_EQUAL(1, pool.inUseCount());
		BOOST_REQUIRE_EQUAL(2, pool.freeCount());
	}
	BOOST_REQUIRE_EQUAL(0, pool.availableCount());
	BOOST_REQUIRE_EQUAL(0, pool.inUseCount());
	BOOST_REQUIRE_EQUAL(3, pool.freeCount());
}

