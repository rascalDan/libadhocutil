#ifndef ADHOCUTIL_PLUGINS_IMPL_H
#define ADHOCUTIL_PLUGINS_IMPL_H

#include "plugins.h"

namespace AdHoc {
	template <typename T>
	PluginOf<T>::PluginOf(const T * t, const std::string & n, const std::string & f, int l) :
		Plugin(n, f, l),
		impl(t)
	{
	}

	template <typename T>
	PluginOf<T>::~PluginOf()
	{
		delete impl;
	}

	/// Get the type of this plugin.
	template <typename T>
	const std::type_info &
	PluginOf<T>::type() const
	{
		return typeid(T);
	}

	/// Get the implementation of this plugin.
	template <typename T>
	const T *
	PluginOf<T>::implementation() const
	{
		return impl;
	}

	template <typename T>
	void
	PluginManager::add(const T * i, const std::string & n, const std::string & f, int l)
	{
		add(PluginPtr(new PluginOf<T>(i, n, f, l)));
	}

	template <typename T>
	void
	PluginManager::remove(const std::string & n)
	{
		remove(n, typeid(T));
	}

	template <typename T>
	boost::shared_ptr<const PluginOf<T>>
	PluginManager::get(const std::string & n) const
	{
		return boost::dynamic_pointer_cast<const PluginOf<T>>(get(n, typeid(T)));
	}

	template <typename T>
	const T *
	PluginManager::getImplementation(const std::string & n) const
	{
		return get<T>(n)->implementation();
	}

	template <typename T>
	std::set<boost::shared_ptr<const PluginOf<T>>>
	PluginManager::getAll() const
	{
		std::set<boost::shared_ptr<const PluginOf<T>>> all;
		for(const auto & p : getAll(typeid(T))) {
			if (auto tp = boost::dynamic_pointer_cast<const PluginOf<T>>(p)) {
				all.insert(tp);
			}
		}
		return all;
	}

	template<typename T>
	void
	PluginManager::addResolver(const PluginResolver & f)
	{
		addResolver(typeid(T), f);
	}

	template<typename T>
	void
	PluginManager::removeResolver()
	{
		removeResolver(typeid(T));
	}
}

#define INSTANTIATEPLUGINOF(T) \
	template class AdHoc::PluginOf<T>; \
	template void AdHoc::PluginManager::add<T>(const T *, const std::string &, const std::string &, int); \
	template void AdHoc::PluginManager::remove<T>(const std::string &); \
	template boost::shared_ptr<const AdHoc::PluginOf<T>> AdHoc::PluginManager::get<T>(const std::string &) const; \
	template const T * AdHoc::PluginManager::getImplementation<T>(const std::string &) const; \
	template std::set<boost::shared_ptr<const AdHoc::PluginOf<T>>> AdHoc::PluginManager::getAll<T>() const; \
	template void AdHoc::PluginManager::addResolver<T>(const AdHoc::PluginManager::PluginResolver & f); \
	template void AdHoc::PluginManager::removeResolver<T>(); \

#define PLUGINRESOLVER(T, F) \
	namespace MAKE_UNIQUE(__plugin__) { \
		static void InstallResolver() __attribute__((constructor(102))); \
		void InstallResolver() { \
			::AdHoc::PluginManager::getDefault()->addResolver<T>(F); \
		} \
		static void UninstallResolver() __attribute__((destructor(102))); \
		void UninstallResolver() { \
			::AdHoc::PluginManager::getDefault()->removeResolver<T>(); \
		} \
	}

#endif

