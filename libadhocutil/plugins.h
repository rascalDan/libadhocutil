#pragma once

#include "c++11Helpers.h"
#include "unique.h"
#include "visibility.h"
#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <typeinfo>

namespace std {
	DLL_PUBLIC
	std::ostream & operator<<(std::ostream & s, const std::type_info & t);
}

namespace AdHoc {
	/// Thrown when no matching plugin can be found.
	class NoSuchPluginException : public std::runtime_error {
	public:
		/// Constructor taking name and type of plugin requested.
		NoSuchPluginException(const std::string_view &, const std::type_info &);
	};

	/// Base class for all plugin implementations.
	class DLL_PUBLIC AbstractPluginImplementation {
	public:
		AbstractPluginImplementation() = default;
		virtual ~AbstractPluginImplementation() = 0;
		/// Standard move/copy support
		SPECIAL_MEMBERS_DEFAULT(AbstractPluginImplementation);
	};

	/// Base class for untyped plugins.
	class DLL_PUBLIC Plugin {
	public:
		/// Constructor taking name, filename and line of install.
		Plugin(const std::string_view &, const std::string_view &, int);
		/// Standard move/copy support
		SPECIAL_MEMBERS_DELETE(Plugin);
		virtual ~Plugin() = default;

		/// Get the plugin type from the subclass.
		[[nodiscard]] virtual const std::type_info & type() const = 0;

		/// Get the abstract base plugin implementation.
		[[nodiscard]] virtual std::shared_ptr<AbstractPluginImplementation> instance() const = 0;

		/// The name the plugin was installed with.
		const std::string name;
		/// The filename the plugin was installed in.
		const std::string filename;
		/// The line of file the plugin was installed in.
		const int lineno;
	};
	using PluginPtr = std::shared_ptr<const Plugin>;

	/// Thrown when a plugin with the same name and base is loaded into a manager.
	class DuplicatePluginException : public std::runtime_error {
	public:
		/// Constructor taking the original and offending plugin.
		DuplicatePluginException(const PluginPtr & p1, const PluginPtr & p2);
	};

	/// Thrown when a resolver function is added a second time.
	class DuplicateResolverException : public std::runtime_error {
	public:
		/// Constuctor taking resolver type.
		explicit DuplicateResolverException(const std::type_info &);
	};

	/// Thrown when an attempt to load a library fails.
	class LoadLibraryException : public std::runtime_error {
	public:
		/// Constuctor taking syscall error details.
		LoadLibraryException(const std::string_view & f, const std::string_view & msg);
	};

	template<typename T>
	/// Typed plugin and handle to implementation.
	class DLL_PUBLIC PluginOf : public Plugin {
	public:
		/// Constructor taking an instance and name, filename and line of install for Plugin.
		PluginOf(std::shared_ptr<T> t, const std::string_view & n, const std::string_view & f, int l);

		/// Get the type of this plugin.
		[[nodiscard]] const std::type_info & type() const override;
		/// Get the implementation of this plugin.
		[[nodiscard]] std::shared_ptr<T> implementation() const;

		[[nodiscard]] std::shared_ptr<AbstractPluginImplementation> instance() const override;

	private:
		const std::shared_ptr<T> impl;
	};

	/// Container for loaded plugins.
	class DLL_PUBLIC PluginManager {
	public:
		/// Callback definition to resolve a plugin type and name to a potential library
		/// containing an implementation.
		using PluginResolver
				= std::function<std::optional<std::string>(const std::type_info &, const std::string_view &)>;

		PluginManager();
		virtual ~PluginManager();

		/// Standard move/copy support
		SPECIAL_MEMBERS_DELETE(PluginManager);

		/// Install a plugin.
		void add(const PluginPtr &);
		/// Uninstall a plugin.
		void remove(const std::string_view &, const std::type_info &);
		/// Get a specific plugin.
		[[nodiscard]] PluginPtr get(const std::string_view &, const std::type_info &) const;
		/// Get all plugins.
		[[nodiscard]] std::set<PluginPtr> getAll() const;
		/// Get all plugins of a specific type.
		[[nodiscard]] std::set<PluginPtr> getAll(const std::type_info &) const;

		/**
		 * Install a plugin.
		 * @param i Implementation instance.
		 * @param n Name of plugin.
		 * @param f Filename of plugin.
		 * @param l Line number.
		 */
		template<typename T>
		void add(const std::shared_ptr<T> & i, const std::string_view & n, const std::string_view & f, int l);

		/**
		 * Create and install a plugin
		 * @tparam T Base type of plugin
		 * @tparam I Implementation type of plugin
		 * @tparam Args Constructor arguments types
		 * @param n Name of plugin.
		 * @param f Filename of plugin.
		 * @param l Line number.
		 * @param args Arguments to construct an instance of I with.
		 */
		template<typename T, typename I, typename... Args>
		void
		create(const std::string_view & n, const std::string_view & f, int l, const Args &... args)
		{
			add<T>(std::make_shared<I>(args...), n, f, l);
		}

		/**
		 * Uninstall a plugin.
		 * @param n Name of plugin.
		 */
		template<typename T> void remove(const std::string_view & n);

		/**
		 * Get a specific plugin.
		 * @param n Name of plugin.
		 */
		template<typename T> [[nodiscard]] std::shared_ptr<const PluginOf<T>> get(const std::string_view & n) const;

		/**
		 * Get the implementation from specific plugin.
		 * @param n Name of plugin.
		 */
		template<typename T> [[nodiscard]] std::shared_ptr<T> getImplementation(const std::string_view & n) const;

		/**
		 * Get all plugins of a given time.
		 */
		template<typename T> [[nodiscard]] std::set<std::shared_ptr<const PluginOf<T>>> getAll() const;

		/**
		 * The number of installed plugins.
		 */
		[[nodiscard]] size_t count() const;

		/**
		 * Add a type plugin resolver function.
		 * @param t The resolver type.
		 * @param f The resolver function.
		 */
		void addResolver(const std::type_info & t, const PluginResolver & f);

		/**
		 * Add a type plugin resolver function.
		 * @param f The resolver function.
		 */
		template<typename T> void addResolver(const PluginResolver & f);

		/**
		 * Remove a type plugin resolver function.
		 * @param t The resolver type.
		 */
		void removeResolver(const std::type_info & t);

		/**
		 * Remove a type plugin resolver function.
		 */
		template<typename T> void removeResolver();

		/**
		 * The number of installed plugins.
		 */
		[[nodiscard]] size_t countResolvers() const;

		/**
		 * Get the default plugin manager instance.
		 */
		static PluginManager * getDefault();

	private:
		static void loadLibrary(const std::string &);

		class PluginStore;
		std::unique_ptr<PluginStore> plugins;
		class TypePluginResolvers;
		std::unique_ptr<TypePluginResolvers> resolvers;
	};
}

#define NAMEDPLUGIN(Name, Implementation, Base) \
	namespace MAKE_UNIQUE(__plugin__) \
	{ \
		static void InstallPlugin() __attribute__((constructor(102))); \
		void InstallPlugin() \
		{ \
			::AdHoc::PluginManager::getDefault()->add<Base>( \
					std::make_shared<Implementation>(), Name, __FILE__, __LINE__); \
		} \
		static void UninstallPlugin() __attribute__((destructor(102))); \
		void UninstallPlugin() \
		{ \
			::AdHoc::PluginManager::getDefault()->remove<Base>(Name); \
		} \
	}
#define PLUGIN(Implementation, Base) NAMEDPLUGIN(#Implementation, Implementation, Base)
