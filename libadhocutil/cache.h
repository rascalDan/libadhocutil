#ifndef ADHOCUTIL_CACHE_H
#define ADHOCUTIL_CACHE_H

#include <time.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/variant.hpp>
#include "visibility.h"

namespace AdHoc {

/// @cond
template <typename T, typename K>
class DLL_PUBLIC Cacheable {
	public:
		Cacheable(const K & k, time_t validUntil);

		const K key;
		const time_t validUntil;

		virtual const T & item() const = 0;
};

template <typename T, typename K>
class DLL_PUBLIC ObjectCacheable : public Cacheable<T, K> {
	public:
		ObjectCacheable(const T & t, const K & k, time_t validUtil);

		virtual const T & item() const override;

	private:
		const T value;
};

template <typename T, typename K>
class DLL_PUBLIC CallCacheable : public Cacheable<T, K> {
	public:
		CallCacheable(const T & t, const K & k, time_t validUtil);
		CallCacheable(const boost::function<T()> & t, const K & k, time_t validUtil);

		virtual const T & item() const override;

	private:
		mutable boost::variant<T, boost::function<T()>> value;
		mutable boost::shared_mutex lock;
};

struct byValidity {};
struct byKey {};
/// @endcond

/// In-memory cache of T, keyed by K.
template <typename T, typename K>
class DLL_PUBLIC Cache {
	public:
		/// @cond
		typedef K Key;
		typedef T Value;
		typedef Cacheable<T, K> Item;
		typedef boost::shared_ptr<Item> Element;
		/// @endcond

		/** Construct a default empty cache. */
		Cache();

		/** Add a known item to the cache.
		 * @param k The key of the cache item.
		 * @param t The item to cache.
		 * @param validUntil The absolute time the cache item should expire.
		 */
		void add(const K & k, const T & t, time_t validUntil);
		/** Add a callback item to the cache.
		 * The callback will be called on first hit of the cache item, at which
		 * point the return value of the function will be cached.
		 * @param k The key of the cache item.
		 * @param tf The callback function to cache.
		 * @param validUntil The absolute time the cache item should expire.
		 */
		void add(const K & k, const boost::function<T()> & tf, time_t validUntil);
		/** Get an Element from the cache. The element represents the key, item and expiry time.
		 * Returns null on cache-miss.
		 * @param k Cache key to get. */
		Element getItem(const K & k) const;
		/** Get an Item from the cache. Returns null on cache-miss.
		 * @param k Cache key to get. */
		const T * get(const K & k) const;
		/** Get the size of the cache (number of items). @warning This cannot be reliably used to
		 * determine or estimate the amount of memory used by items in the cache without further
		 * knowledge of the items themselves. */
		size_t size() const;

	private:
		void DLL_PRIVATE prune() const;
		mutable time_t lastPruneTime;

		mutable boost::shared_mutex lock;

		typedef boost::multi_index::multi_index_container<Element,
						boost::multi_index::indexed_by<
						boost::multi_index::ordered_unique<
							boost::multi_index::tag<byKey>, BOOST_MULTI_INDEX_MEMBER(Item, const K, key)>,
						boost::multi_index::ordered_non_unique<
							boost::multi_index::tag<byValidity>,  BOOST_MULTI_INDEX_MEMBER(Item, const time_t, validUntil)>
							> > Cached;
		mutable Cached cached;
};

}

#endif

