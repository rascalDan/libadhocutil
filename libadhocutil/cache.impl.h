#pragma once

#include "cache.h" // IWYU pragma: export
#include "lockHelpers.h"
#include <boost/lambda/lambda.hpp>
#include <ctime>
#include <memory>
#include <mutex> // IWYU pragma: keep
#include <variant>
// IWYU pragma: no_include <boost/multi_index/detail/unbounded.hpp>

namespace AdHoc {

	/// @cond
	template<typename T, typename K> Cacheable<T, K>::Cacheable(K k, time_t vu) : key(std::move(k)), validUntil(vu) { }

	template<typename T, typename K>
	ObjectCacheable<T, K>::ObjectCacheable(const T & t, const K & k, time_t vu) :
		Cacheable<T, K>(k, vu), value(std::make_shared<T>(t))
	{
	}

	template<typename T, typename K>
	// cppcheck-suppress passedByValue
	ObjectCacheable<T, K>::ObjectCacheable(typename Cacheable<T, K>::Value t, const K & k, time_t vu) :
		Cacheable<T, K>(k, vu), value(std::move(t))
	{
	}

	template<typename T, typename K>
	typename Cacheable<T, K>::Value
	ObjectCacheable<T, K>::item() const
	{
		return value;
	}

	template<typename T, typename K>
	CallCacheable<T, K>::CallCacheable(const Factory & t, const K & k, time_t vu) : Cacheable<T, K>(k, vu), value(t)
	{
	}

	template<typename T, typename K>
	typename Cacheable<T, K>::Value
	CallCacheable<T, K>::item() const
	{
		Lock(lock);
		if (auto t = std::get_if<0>(&value)) {
			return *t;
		}
		const Factory & f = std::get<Factory>(value);
		value = std::make_shared<T>(f());
		return std::get<0>(value);
	}

	template<typename T, typename K>
	PointerCallCacheable<T, K>::PointerCallCacheable(const Factory & t, const K & k, time_t vu) :
		Cacheable<T, K>(k, vu), value(t)
	{
	}

	template<typename T, typename K>
	typename Cacheable<T, K>::Value
	PointerCallCacheable<T, K>::item() const
	{
		Lock(lock);
		if (auto t = std::get_if<0>(&value)) {
			return *t;
		}
		const Factory & f = std::get<Factory>(value);
		value = f();
		return std::get<0>(value);
	}

	template<typename T, typename K> Cache<T, K>::Cache() : lastPruneTime(time(nullptr)) { }

	template<typename T, typename K>
	void
	Cache<T, K>::add(const K & k, const T & t, time_t validUntil)
	{
		Lock(lock);
		cached.insert(std::make_shared<ObjectCacheable<T, K>>(t, k, validUntil));
	}

	template<typename T, typename K>
	void
	Cache<T, K>::addPointer(const K & k, Value & t, time_t validUntil)
	{
		Lock(lock);
		cached.insert(std::make_shared<ObjectCacheable<T, K>>(t, k, validUntil));
	}

	template<typename T, typename K>
	void
	Cache<T, K>::addFactory(const K & k, const Factory & tf, time_t validUntil)
	{
		Lock(lock);
		cached.insert(std::make_shared<CallCacheable<T, K>>(tf, k, validUntil));
	}

	template<typename T, typename K>
	void
	Cache<T, K>::addPointerFactory(const K & k, const PointerFactory & tf, time_t validUntil)
	{
		Lock(lock);
		cached.insert(std::make_shared<PointerCallCacheable<T, K>>(tf, k, validUntil));
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
			if ((*i)->validUntil > time(nullptr)) {
				return (*i);
			}
		}
		prune();
		return Element();
	}

	template<typename T, typename K>
	typename Cache<T, K>::Value
	Cache<T, K>::get(const K & k) const
	{
		auto i = getItem(k);
		if (i) {
			return i->item();
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
	Cache<T, K>::remove(const K & k)
	{
		Lock(lock);
		cached.template get<byKey>().erase(k);
	}

	template<typename T, typename K>
	void
	Cache<T, K>::clear()
	{
		Lock(lock);
		cached.clear();
	}

	template<typename T, typename K>
	void
	Cache<T, K>::prune() const
	{
		auto now = time(nullptr);
		if (lastPruneTime < now) {
			Lock(lock);
			auto & collection = cached.template get<byValidity>();
			auto range = collection.range(boost::multi_index::unbounded, boost::lambda::_1 < now);
			collection.erase(range.first, range.second);
			lastPruneTime = now;
		}
	}
	/// @endcond

}
