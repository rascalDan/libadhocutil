#pragma once

#include "c++11Helpers.h"
#include "visibility.h"
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/tag.hpp>
#include <boost/multi_index_container.hpp>
#include <ctime>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <variant>

namespace AdHoc {

	/// @cond
	template<typename T, typename K> class DLL_PUBLIC Cacheable {
	public:
		using Value = const std::shared_ptr<const T>;
		Cacheable(K k, time_t validUntil);
		SPECIAL_MEMBERS_DEFAULT_MOVE_NO_COPY(Cacheable);

		virtual ~Cacheable() = default;

		const K key;
		const time_t validUntil;

		[[nodiscard]] virtual Value item() const = 0;
	};

	template<typename T, typename K> class DLL_PUBLIC ObjectCacheable : public Cacheable<T, K> {
	public:
		ObjectCacheable(const T & t, const K & k, time_t validUtil);
		ObjectCacheable(typename Cacheable<T, K>::Value t, const K & k, time_t validUtil);

		[[nodiscard]] typename Cacheable<T, K>::Value item() const override;

	private:
		typename Cacheable<T, K>::Value value;
	};

	template<typename T, typename K> class DLL_PUBLIC CallCacheable : public Cacheable<T, K> {
	public:
		using Factory = std::function<T()>;
		CallCacheable(const Factory & t, const K & k, time_t validUtil);

		[[nodiscard]] typename Cacheable<T, K>::Value item() const override;

	private:
		mutable std::variant<std::shared_ptr<const T>, Factory> value;
		mutable std::shared_mutex lock;
	};

	template<typename T, typename K> class DLL_PUBLIC PointerCallCacheable : public Cacheable<T, K> {
	public:
		using Factory = std::function<typename Cacheable<T, K>::Value()>;
		PointerCallCacheable(const Factory & t, const K & k, time_t validUtil);

		[[nodiscard]] typename Cacheable<T, K>::Value item() const override;

	private:
		mutable std::variant<std::shared_ptr<const T>, Factory> value;
		mutable std::shared_mutex lock;
	};

	struct byValidity {
	};
	struct byKey {
	};
	/// @endcond

	/// In-memory cache of T, keyed by K.
	template<typename T, typename K> class DLL_PUBLIC Cache {
	public:
		/// @cond
		using Key = K;
		using Value = const std::shared_ptr<const T>;
		using Factory = std::function<T()>;
		using PointerFactory = std::function<Value()>;
		using Item = Cacheable<T, K>;
		using Element = std::shared_ptr<Item>;
		/// @endcond

		/** Construct a default empty cache. */
		Cache();

		/** Add a known item to the cache.
		 * @param k The key of the cache item.
		 * @param t The item to cache.
		 * @param validUntil The absolute time the cache item should expire.
		 */
		void add(const K & k, const T & t, time_t validUntil);
		/** Add a known item to the cache.
		 * @param k The key of the cache item.
		 * @param t The item to cache.
		 * @param validUntil The absolute time the cache item should expire.
		 */
		void addPointer(const K & k, Value & t, time_t validUntil);
		/** Add a callback item to the cache.
		 * The callback will be called on first hit of the cache item, at which
		 * point the return value of the function will be cached.
		 * @param k The key of the cache item.
		 * @param tf The callback function to cache.
		 * @param validUntil The absolute time the cache item should expire.
		 */
		void addFactory(const K & k, const Factory & tf, time_t validUntil);
		/** Add a pointer callback item to the cache.
		 * The callback will be called on first hit of the cache item, at which
		 * point the return value of the function will be cached.
		 * @param k The key of the cache item.
		 * @param tf The callback function to cache.
		 * @param validUntil The absolute time the cache item should expire.
		 */
		void addPointerFactory(const K & k, const PointerFactory & tf, time_t validUntil);
		/** Get an Element from the cache. The element represents the key, item and expiry time.
		 * Returns null on cache-miss.
		 * @param k Cache key to get. */
		Element getItem(const K & k) const;
		/** Get an Item from the cache. Returns null on cache-miss.
		 * @param k Cache key to get. */
		Value get(const K & k) const;
		/** Get the size of the cache (number of items). @warning This cannot be reliably used to
		 * determine or estimate the amount of memory used by items in the cache without further
		 * knowledge of the items themselves. */
		size_t size() const;
		/** Explicitly remove an item from the cache.
		 * @param k Cache key to remove. */
		void remove(const K & k);
		/** Explicitly remove ALL items from the cache. */
		void clear();

	private:
		void DLL_PRIVATE prune() const;
		mutable time_t lastPruneTime;

		mutable std::shared_mutex lock;

		using Cached = boost::multi_index::multi_index_container<Element,
				boost::multi_index::indexed_by<boost::multi_index::ordered_unique<boost::multi_index::tag<byKey>,
													   BOOST_MULTI_INDEX_MEMBER(Item, const K, key)>,
						boost::multi_index::ordered_non_unique<boost::multi_index::tag<byValidity>,
								BOOST_MULTI_INDEX_MEMBER(Item, const time_t, validUntil)>>>;
		mutable Cached cached;
	};

}
