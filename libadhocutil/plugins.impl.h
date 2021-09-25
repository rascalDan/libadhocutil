#ifndef ADHOCUTIL_PLUGINS_IMPL_H
#define ADHOCUTIL_PLUGINS_IMPL_H

#include "plugins.h" // IWYU pragma: export
#include "unique.h"
#include "visibility.h"
#include <memory>
#include <set>
#include <string_view>
#include <typeinfo>

namespace AdHoc {
	template<typename T>
	PluginOf<T>::PluginOf(std::shared_ptr<T> t, const std::string_view & n, const std::string_view & f, int l) :
		Plugin(n, f, l), impl(std::move(t))
	{
	}

	/// Get the type of this plugin.
	template<typename T>
	const std::type_info &
	PluginOf<T>::type() const
	{
		return typeid(T);
	}

	/// Get the implementation of this plugin.
	template<typename T>
	std::shared_ptr<T>
	PluginOf<T>::implementation() const
	{
		return impl;
	}

	template<typename T>
	std::shared_ptr<AbstractPluginImplementation>
	PluginOf<T>::instance() const
	{
		return impl;
	}

	template<typename T>
	void
	PluginManager::add(const std::shared_ptr<T> & i, const std::string_view & n, const std::string_view & f, int l)
	{
		add(std::make_shared<PluginOf<T>>(i, n, f, l));
	}

	template<typename T>
	void
	PluginManager::remove(const std::string_view & n)
	{
		remove(n, typeid(T));
	}

	template<typename T>
	std::shared_ptr<const PluginOf<T>>
	PluginManager::get(const std::string_view & n) const
	{
		return std::dynamic_pointer_cast<const PluginOf<T>>(get(n, typeid(T)));
	}

	template<typename T>
	std::shared_ptr<T>
	PluginManager::getImplementation(const std::string_view & n) const
	{
		return std::static_pointer_cast<T>(get<T>(n)->implementation());
	}

	template<typename T>
	std::set<std::shared_ptr<const PluginOf<T>>>
	PluginManager::getAll() const
	{
		std::set<std::shared_ptr<const PluginOf<T>>> all;
		for (const auto & p : getAll(typeid(T))) {
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
	template DLL_PUBLIC void AdHoc::PluginManager::add<__VA_ARGS__>( \
			const std::shared_ptr<__VA_ARGS__> &, const std::string_view &, const std::string_view &, int); \
	template DLL_PUBLIC void AdHoc::PluginManager::remove<__VA_ARGS__>(const std::string_view &); \
	template DLL_PUBLIC std::shared_ptr<const AdHoc::PluginOf<__VA_ARGS__>> AdHoc::PluginManager::get<__VA_ARGS__>( \
			const std::string_view &) const; \
	template DLL_PUBLIC std::shared_ptr<__VA_ARGS__> AdHoc::PluginManager::getImplementation<__VA_ARGS__>( \
			const std::string_view &) const; \
	template DLL_PUBLIC std::set<std::shared_ptr<const AdHoc::PluginOf<__VA_ARGS__>>> \
	AdHoc::PluginManager::getAll<__VA_ARGS__>() const; \
	template DLL_PUBLIC void AdHoc::PluginManager::addResolver<__VA_ARGS__>( \
			const AdHoc::PluginManager::PluginResolver & f); \
	template DLL_PUBLIC void AdHoc::PluginManager::removeResolver<__VA_ARGS__>();

#define PLUGINRESOLVER(T, F) \
	namespace MAKE_UNIQUE(__plugin__) \
	{ \
		static void InstallResolver() __attribute__((constructor(102))); \
		void InstallResolver() \
		{ \
			::AdHoc::PluginManager::getDefault()->addResolver<T>(F); \
		} \
		static void UninstallResolver() __attribute__((destructor(102))); \
		void UninstallResolver() \
		{ \
			::AdHoc::PluginManager::getDefault()->removeResolver<T>(); \
		} \
	}

#endif
