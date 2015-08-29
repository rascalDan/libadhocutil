#ifndef LIBADHOC_LAZYPOINTER_H
#define LIBADHOC_LAZYPOINTER_H

#include <boost/function.hpp>
#include <boost/variant.hpp>
#include <boost/intrusive_ptr.hpp>

template <typename T, typename P = boost::intrusive_ptr<T>>
class LazyPointer {
	public:
		typedef T element_type;
		typedef P pointer_type;
		typedef boost::function0<P> Factory;
		typedef boost::variant<P, Factory> Source;

		// Constructors
		LazyPointer(const Factory & f) :
			source(f)
		{
		}

		LazyPointer(const P & p) :
			source(p)
		{
		}

		LazyPointer(T * p) :
			source(P(p))
		{
		}

		LazyPointer() :
			source(P(NULL))
		{
		}

		// Getters
		operator P() const
		{
			return deref();
		}

		T * operator->() const
		{
			return get();
		}

		T & operator*() const
		{
			return *get();
		}

		T * get() const
		{
			return deref().get();
		}

		P deref() const
		{
			if (Factory * f = boost::get<Factory>(&source)) {
				P p = (*f)();
				source = p;
				return p;
			}
			else {
				return boost::get<P>(source);
			}
		}

		bool operator!() const
		{
			return get() == nullptr;
		}

		operator bool() const
		{
			return get() != nullptr;
		}

		bool operator==(const P & o) const
		{
			return (deref() == o);
		}

		bool operator==(const T * o) const
		{
			return (deref().get() == o);
		}

		// Setters
		LazyPointer<T, P> & operator=(const P & p)
		{
			source = p;
			return *this;
		}

		LazyPointer<T, P> & operator=(T * t)
		{
			source = P(t);
			return *this;
		}

		LazyPointer<T, P> & operator=(const Factory & f)
		{
			source = f;
			return *this;
		}

		bool hasValue() const
		{
			return boost::get<P>(&source);
		}

	private:
		mutable Source source;
};

namespace boost {
	template <typename R, typename T, typename P>
		R * dynamic_pointer_cast(const LazyPointer<T, P> & p) {
			return dynamic_cast<R *>(p.get());
		}
}

#endif

