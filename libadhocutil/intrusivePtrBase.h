#ifndef ADHOCUTIL_INTRUSIVEPTRBASE_H
#define ADHOCUTIL_INTRUSIVEPTRBASE_H

#include <boost/intrusive_ptr.hpp>

/// Base class suitable for use with boost::intrusive_ptr<T>.
class IntrusivePtrBase {
	protected:
		/// @cond
		inline IntrusivePtrBase() : _refCount(0) { }
		inline virtual ~IntrusivePtrBase() = 0;

		mutable unsigned int _refCount;
		friend void intrusive_ptr_release(const IntrusivePtrBase * p);
		friend void intrusive_ptr_add_ref(const IntrusivePtrBase * p);
	private:
		IntrusivePtrBase(const IntrusivePtrBase &) = delete;
		void operator=(const IntrusivePtrBase &) = delete;
};

inline
IntrusivePtrBase::~IntrusivePtrBase() = default;

inline
void
intrusive_ptr_release(const IntrusivePtrBase * p)
{
	if (p->_refCount == 1) {
		delete p;
	}
	else {
		p->_refCount -= 1;
	}
}

inline
void
intrusive_ptr_add_ref(const IntrusivePtrBase * p)
{
	p->_refCount += 1;
}
/// @endcond

#endif

