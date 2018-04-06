#define BOOST_TEST_MODULE Locks
#include <boost/test/unit_test.hpp>

#include "lockHelpers.h"
#include <shared_mutex>

BOOST_AUTO_TEST_CASE ( lock )
{
	std::shared_mutex _lock;
	{
		Lock(_lock);
		BOOST_CHECK(_lck11.owns_lock());
		BOOST_CHECK(!_lock.try_lock());
		BOOST_CHECK(!_lock.try_lock_shared());
	}
	BOOST_CHECK(_lock.try_lock());
	_lock.unlock();
	BOOST_CHECK(_lock.try_lock_shared());
	_lock.unlock_shared();
}

BOOST_AUTO_TEST_CASE ( sharedlock )
{
	std::shared_mutex _lock;
	{
		SharedLock(_lock);
		BOOST_CHECK(_lck26.owns_lock());
		BOOST_CHECK(!_lock.try_lock());
		BOOST_CHECK(_lock.try_lock_shared());
		_lock.unlock_shared();
	}
	BOOST_CHECK(_lock.try_lock());
	_lock.unlock();
	BOOST_CHECK(_lock.try_lock_shared());
	_lock.unlock_shared();
}

BOOST_AUTO_TEST_CASE ( scopelock )
{
	std::shared_mutex _lock;
	ScopeLock(_lock) {
		BOOST_CHECK(_lck41.owns_lock());
		BOOST_CHECK(!_lock.try_lock());
		BOOST_CHECK(!_lock.try_lock_shared());
	}
	BOOST_CHECK(_lock.try_lock());
	_lock.unlock();
	BOOST_CHECK(_lock.try_lock_shared());
	_lock.unlock_shared();
}

BOOST_AUTO_TEST_CASE ( sharedscopelock )
{
	std::shared_mutex _lock;
	SharedScopeLock(_lock) {
		BOOST_CHECK(_lck55.owns_lock());
		BOOST_CHECK(!_lock.try_lock());
		BOOST_CHECK(_lock.try_lock_shared());
		_lock.unlock_shared();
	}
	BOOST_CHECK(_lock.try_lock());
	_lock.unlock();
	BOOST_CHECK(_lock.try_lock_shared());
	_lock.unlock_shared();
}

