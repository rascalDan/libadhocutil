#pragma once

#define LIBADHOC_LOCK_CONCAT2(a, b) a##b
#define LIBADHOC_LOCK_CONCAT(a, b) LIBADHOC_LOCK_CONCAT2(a, b)
#define LIBADHOC_LOCK_WITHLINE(a) LIBADHOC_LOCK_CONCAT(a, __LINE__)

#define BaseLock(l, lt) lt LIBADHOC_LOCK_WITHLINE(_lck)(l)
#define BaseScopeLock(l, lt) if (lt LIBADHOC_LOCK_WITHLINE(_lck) {l})

#define Lock(l) BaseLock(l, std::unique_lock)
#define SharedLock(l) BaseLock(l, std::shared_lock)

#define ScopeLock(l) BaseScopeLock(l, std::unique_lock)
#define SharedScopeLock(l) BaseScopeLock(l, std::shared_lock)
