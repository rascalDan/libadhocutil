#ifndef LIBADHOC_LOCKHELPERS_H
#define LIBADHOC_LOCKHELPERS_H

template <typename locktype>
class _lockLoop {
	public:
		template<typename l>
		_lockLoop(l & _l) :
			lock(_l),
			flag(true)
		{
		}

		operator bool() const { return flag; }
		void operator!() { flag = false; }

	private:
		locktype lock;
		bool flag;
};

#define LIBADHOC_LOCK_CONCAT2(a, b) a ## b
#define LIBADHOC_LOCK_CONCAT(a, b) LIBADHOC_LOCK_CONCAT2(a, b)
#define LIBADHOC_LOCK_WITHLINE(a) LIBADHOC_LOCK_CONCAT(a,  __LINE__)

#define BaseScopeLock(l,lt,mt) for (_lockLoop<lt<mt>> LIBADHOC_LOCK_WITHLINE(_lck)(l); LIBADHOC_LOCK_WITHLINE(_lck); !LIBADHOC_LOCK_WITHLINE(_lck))
#define BaseLock(l,lt,mt) lt<mt> LIBADHOC_LOCK_WITHLINE(_lck)(l)

#define Lock(l) BaseLock(l, boost::unique_lock, boost::shared_mutex)
#define SharedLock(l) BaseLock(l, boost::shared_lock, boost::shared_mutex)

#define ScopeLock(l) BaseScopeLock(l, boost::unique_lock, boost::shared_mutex)
#define SharedScopeLock(l) BaseScopeLock(l, boost::shared_lock, boost::shared_mutex)

#endif

