#ifndef ADHOCUTIL_SAFEMAPFIND_H
#define ADHOCUTIL_SAFEMAPFIND_H

#include <algorithm>

namespace AdHoc {

	/*
	 * Find the key and return the iterator to the pair,
	 * or throw Ex(key) if not found.
	 */
	template<class Ex, class Map>
	typename Map::const_iterator
	safeMapFind(const Map & map, const typename Map::key_type & key)
	{
		if (auto i = map.find(key); i != map.end()) {
			return i;
		}
		throw Ex(key);
	}

	/*
	 * Find the key and return the mapped value
	 * or the provided default value if not found.
	 */
	template<class Map>
	typename Map::mapped_type
	defaultMapLookup(const Map & map, const typename Map::key_type & key,
			const typename Map::mapped_type & def = typename Map::mapped_type())
	{
		if (auto i = map.find(key); i != map.end()) {
			return i->second;
		}
		return def;
	}

	/*
	 * Find the key and return the mapped value
	 * or throw Ex(key) if not found.
	 */
	template<class Ex, class Map>
	const typename Map::mapped_type &
	safeMapLookup(const Map & map, const typename Map::key_type & key)
	{
		if (auto i = map.find(key); i != map.end()) {
			return i->second;
		}
		throw Ex(key);
	}

	/*
	 * Simple search, true if found, false otherwise
	 */
	template<class Cont>
	bool
	containerContains(const Cont & c, const typename Cont::value_type & v)
	{
		return (std::find(c.begin(), c.end(), v) != c.end());
	}

}

#endif
