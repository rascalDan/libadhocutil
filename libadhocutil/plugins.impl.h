#ifndef ADHOCUTIL_PLUGINS_IMPL_H
#define ADHOCUTIL_PLUGINS_IMPL_H

#include "plugins.h"

namespace AdHoc {
	template <typename T>
	PluginOf<T>::PluginOf(const std::shared_ptr<T> & t, const std::string & n, const std::string & f, int l) :
		Plugin(n, f, l),
		impl(t)
	{
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
	std::shared_ptr<T>
	PluginOf<T>::implementation() const
	{
		return impl;
	}

	template <typename T>
	std::shared_ptr<AbstractPluginImplementation>
	PluginOf<T>::instance() const
	{
		return impl;
	}

	template <typename T>
	void
	PluginManager::add(const std::shared_ptr<T> & i, const std::string & n, const std::string & f, int l)
	{
		add(std::make_shared<PluginOf<T>>(i, n, f, l));
	}

	template <typename T>
	void
	PluginManager::remove(const std::string & n)
	{
		remove(n, typeid(T));
	}

	template <typename T>
	std::shared_ptr<const PluginOf<T>>
	PluginManager::get(const std::string & n) const
	{
		return std::dynamic_pointer_cast<const PluginOf<T>>(get(n, typeid(T)));
	}

	template <typename T>
	std::shared_ptr<T>
	PluginManager::getImplementation(const std::string & n) const
	{
		return std::static_pointer_cast<T>(get<T>(n)->implementation());
	}

	template <typename T>
	std::set<std::shared_ptr<const PluginOf<T>>>
	PluginManager::getAll() const
	{
		std::set<std::shared_ptr<const PluginOf<T>>> all;
		for(const auto & p : getAll(typeid(T))) {
			if (auto tp = std::dynamic_pointer_cast<const PluginOf<T>>(p)) {
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

#define INSTANTIATEPLUGINOF(...) \
	template class AdHoc::PluginOf<__VA_ARGS__>; \
	template void AdHoc::PluginManager::add<__VA_ARGS__>(const std::shared_ptr<__VA_ARGS__> &, const std::string &, const std::string &, int); \
	template void AdHoc::PluginManager::remove<__VA_ARGS__>(const std::string &); \
	template std::shared_ptr<const AdHoc::PluginOf<__VA_ARGS__>> AdHoc::PluginManager::get<__VA_ARGS__>(const std::string &) const; \
	template std::shared_ptr<__VA_ARGS__> AdHoc::PluginManager::getImplementation<__VA_ARGS__>(const std::string &) const; \
	template std::set<std::shared_ptr<const AdHoc::PluginOf<__VA_ARGS__>>> AdHoc::PluginManager::getAll<__VA_ARGS__>() const; \
	template void AdHoc::PluginManager::addResolver<__VA_ARGS__>(const AdHoc::PluginManager::PluginResolver & f); \
	template void AdHoc::PluginManager::removeResolver<__VA_ARGS__>(); \

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

