#ifndef ADHOCUTIL_LOCKHELPERS_H
#define ADHOCUTIL_LOCKHELPERS_H

#include <type_traits>

/// @cond
template <typename locktype>
class _LockLoop : public locktype {
	public:
		template<typename MutexType>
		_LockLoop(MutexType & m) :
			locktype(m),
			flag(true)
		{
		}

		operator bool() const { return flag; }
		void operator!() { flag = false; }

	private:
		bool flag;
};
/// @endcond

#define LIBADHOC_LOCK_CONCAT2(a, b) a ## b
#define LIBADHOC_LOCK_CONCAT(a, b) LIBADHOC_LOCK_CONCAT2(a, b)
#define LIBADHOC_LOCK_WITHLINE(a) LIBADHOC_LOCK_CONCAT(a,  __LINE__)
#define LIBADHOC_RRT(l) typename std::remove_reference<decltype(l)>::type

#define BaseScopeLock(l,lt) for (_LockLoop<lt<LIBADHOC_RRT(l)>> LIBADHOC_LOCK_WITHLINE(_lck)(l); LIBADHOC_LOCK_WITHLINE(_lck); !LIBADHOC_LOCK_WITHLINE(_lck))
#define TypeBaseLock(l,lt,mt) lt<mt> LIBADHOC_LOCK_WITHLINE(_lck)(l)
#define BaseLock(l,lt) TypeBaseLock(l, lt, LIBADHOC_RRT(l))

#define Lock(l) BaseLock(l, boost::unique_lock)
#define SharedLock(l) BaseLock(l, boost::shared_lock)
#define UpgradableLock(l, ln) boost::upgrade_lock<LIBADHOC_RRT(l)> ln(l)
#define UpgradeLock(l) TypeBaseLock(l, boost::upgrade_to_unique_lock, LIBADHOC_RRT(l)::mutex_type)

#define ScopeLock(l) BaseScopeLock(l, boost::unique_lock)
#define SharedScopeLock(l) BaseScopeLock(l, boost::shared_lock)
#define UpgradableScopeLock(l, ln) for (_LockLoop<boost::upgrade_lock<LIBADHOC_RRT(l)>> ln(l); ln; !ln)
#define UpgradeScopeLock(l) for (_LockLoop<boost::upgrade_to_unique_lock<LIBADHOC_RRT(l)::mutex_type>> LIBADHOC_LOCK_WITHLINE(_lck)(l); LIBADHOC_LOCK_WITHLINE(_lck); !LIBADHOC_LOCK_WITHLINE(_lck))

#endif

