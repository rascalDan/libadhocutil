#ifndef ADHOCUTIL_CACHE_IMPL_H
#define ADHOCUTIL_CACHE_IMPL_H

#include "cache.h"
#include <boost/lambda/lambda.hpp>
#include "lockHelpers.h"

template<typename T, typename K>
Cacheable<T, K>::Cacheable(const K & k, time_t vu) :
	key(k),
	validUntil(vu)
{
}

template<typename T, typename K>
ObjectCacheable<T, K>::ObjectCacheable(const T & t, const K & k, time_t vu) :
	Cacheable<T, K>(k, vu),
	value(t)
{
}

template<typename T, typename K>
const T &
ObjectCacheable<T, K>::item() const
{
	return value;
}

template<typename T, typename K>
CallCacheable<T, K>::CallCacheable(const T & t, const K & k, time_t vu) :
	Cacheable<T, K>(k, vu),
	value(t)
{
}

template<typename T, typename K>
CallCacheable<T, K>::CallCacheable(const boost::function<T()> & t, const K & k, time_t vu) :
	Cacheable<T, K>(k, vu),
	value(t)
{
}

template<typename T, typename K>
const T &
CallCacheable<T, K>::item() const
{
	Lock(lock);
	const T * t = boost::get<T>(&value);
	if (t) {
		return *t;
	}
	const boost::function<T()> & f = boost::get<boost::function<T()>>(value);
	value = f();
	return boost::get<T>(value);
}


template<typename T, typename K>
Cache<T, K>::Cache() :
	lastPruneTime(time(NULL))
{
}

template<typename T, typename K>
void
Cache<T, K>::add(const K & k, const T & t, time_t validUntil)
{
	Lock(lock);
	cached.insert(Element(new ObjectCacheable<T, K>(t, k, validUntil)));
}

template<typename T, typename K>
void
Cache<T, K>::add(const K & k, const boost::function<T()> & tf, time_t validUntil)
{
	Lock(lock);
	cached.insert(Element(new CallCacheable<T, K>(tf, k, validUntil)));
}

template<typename T, typename K>
typename Cache<T, K>::Element
Cache<T, K>::getItem(const K & k) const
{
	{
		SharedLock(lock);
		auto & collection = cached.template get<byKey>();
		auto i = collection.find(k);
		if (i == collection.end()) {
			return Element();
		}
		if ((*i)->validUntil > time(NULL)) {
			return (*i);
		}
	}
	prune();
	return Element();
}

template<typename T, typename K>
const T *
Cache<T, K>::get(const K & k) const
{
	auto i = getItem(k);
	if (i) {
		return &i->item();
	}
	return nullptr;
}

template<typename T, typename K>
size_t
Cache<T, K>::size() const
{
	return cached.size();
}

template<typename T, typename K>
void
Cache<T, K>::prune() const
{
	auto now = time(NULL);
	if (lastPruneTime < now) {
		Lock(lock);
		auto & collection = cached.template get<byValidity>();
		auto range = collection.range(boost::multi_index::unbounded, boost::lambda::_1 < now);
		collection.erase(range.first, range.second);
		lastPruneTime = now;
	}
}

#endif

