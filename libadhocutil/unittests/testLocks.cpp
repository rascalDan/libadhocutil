#define BOOST_TEST_MODULE Locks
#include <boost/test/unit_test.hpp>

#include "lockHelpers.h"
#include <boost/thread/shared_mutex.hpp>

BOOST_AUTO_TEST_CASE ( lock )
{
	boost::shared_mutex _lock;
	Lock(_lock);
}

BOOST_AUTO_TEST_CASE ( sharedlock )
{
	boost::shared_mutex _lock;
	SharedLock(_lock);
}

BOOST_AUTO_TEST_CASE ( upgradelock )
{
	boost::upgrade_mutex _lock;
	UpgradableLock(_lock, ln);
	UpgradeScopeLock(ln) { }
	UpgradeLock(ln);
}

BOOST_AUTO_TEST_CASE ( scopelock )
{
	boost::shared_mutex _lock;
	ScopeLock(_lock) { }
}

BOOST_AUTO_TEST_CASE ( sharedscopelock )
{
	boost::shared_mutex _lock;
	SharedScopeLock(_lock) { }
}

BOOST_AUTO_TEST_CASE ( upgradescopelock )
{
	boost::upgrade_mutex _lock;
	UpgradableScopeLock(_lock, ln) {
		UpgradeScopeLock(ln) { }
		UpgradeLock(ln);
	}
}

