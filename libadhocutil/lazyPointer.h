#ifndef ADHOCUTIL_LAZYPOINTER_H
#define ADHOCUTIL_LAZYPOINTER_H

#include <functional>
#include <variant>
#include <memory>
#include <ostream>

namespace AdHoc {

/// Smart pointer that initializes itself only if it has to.
/**
 * LazyPointer behaves like a normal smarter for the most part. It's benefit
 * is that it can be assigned a factory function which is only called on
 * an attempt to dereference the pointer. All such operations will call
 * this factory function as required prior to evaluating the pointer's value.
 */
template <typename T, typename P = std::shared_ptr<T>>
class LazyPointer {
	public:
		/// @cond
		typedef T element_type;
		typedef P pointer_type;
		typedef std::function<P()> Factory;
		typedef std::variant<P, Factory> Source;
		/// @endcond

		/** Construct pointer with a factory function. */
		LazyPointer(const Factory & f) :
			source(f)
		{
		}

		/** Construct pointer with an instance value. */
		LazyPointer(const P & p) :
			source(p)
		{
		}

		/** Construct pointer with an instance value. */
		template <typename TT = T>
		LazyPointer(T * p, typename std::enable_if<!std::is_same<TT *, P>::value>::type * = NULL) :
			source(P(p))
		{
		}

		/** Construct pointer with no factory or value. */
		LazyPointer() :
			source(P(NULL))
		{
		}

		// Getters
		/// @cond
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
			if constexpr (std::is_pointer<P>::value) {
				return deref();
			}
			else {
				return deref().get();
			}
		}

		P deref() const
		{
			if (Factory * f = std::get_if<Factory>(&source)) {
				P p = (*f)();
				source = p;
				return p;
			}
			else {
				return std::get<P>(source);
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
		/// @endcond

		/** Does the lazy pointer have a value? (as opposed to a factory). */
		bool hasValue() const
		{
			return std::get_if<P>(&source);
		}

	private:
		mutable Source source;
};

}
namespace boost {
	template <typename R, typename T, typename P>
		R * dynamic_pointer_cast(const AdHoc::LazyPointer<T, P> & p) {
			return dynamic_cast<R *>(p.get());
		}
}

#endif

