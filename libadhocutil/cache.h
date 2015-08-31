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

template <typename T, typename K>
class Cacheable {
	public:
		Cacheable(const K & k, time_t validUntil);

		const K key;
		const time_t validUntil;

		virtual const T & item() const = 0;
};

template <typename T, typename K>
class ObjectCacheable : public Cacheable<T, K> {
	public:
		ObjectCacheable(const T & t, const K & k, time_t validUtil);

		virtual const T & item() const override;

	private:
		const T value;
};

template <typename T, typename K>
class CallCacheable : public Cacheable<T, K> {
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
template <typename T, typename K>
class Cache {
	public:
		typedef K Key;
		typedef T Value;
		typedef Cacheable<T, K> Item;
		typedef boost::shared_ptr<Item> Element;

		Cache();

		void add(const K & k, const T & t, time_t validUntil);
		void add(const K & k, const boost::function<T()> & tf, time_t validUntil);
		Element getItem(const K & k) const;
		const T * get(const K & k) const;
		size_t size() const;

	private:
		void prune() const;
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

#endif

