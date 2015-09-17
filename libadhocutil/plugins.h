#ifndef ADHOCUTIL_PLUGINS_H
#define ADHOCUTIL_PLUGINS_H

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/multi_index_container_fwd.hpp>
#include <boost/multi_index/ordered_index_fwd.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <typeinfo>
#include <set>
#include <map>
#include <algorithm>
#include "visibility.h"
#include "unique.h"

namespace std {
	DLL_PUBLIC
	std::ostream &
	operator<<(std::ostream & s, const std::type_info & t);
}

namespace AdHoc {
	/// Thrown when no matching plugin can be found.
	class NoSuchPluginException : public std::runtime_error {
		public:
			/// Constructor taking name and type of plugin requested.
			NoSuchPluginException(const std::string &, const std::type_info &);
	};

	/// Base class for untyped plugins.
	class DLL_PUBLIC Plugin {
		public:
			/// Constructor taking name, filename and line of install.
			Plugin(const std::string &, const std::string &, int);
			virtual ~Plugin();

			/// Get the plugin type from the subclass.
			virtual const std::type_info & type() const = 0;

			/// The name the plugin was installed with.
			const std::string name;
			/// The filename the plugin was installed in.
			const std::string filename;
			/// The line of file the plugin was installed in.
			const int lineno;
	};
	typedef boost::shared_ptr<const Plugin> PluginPtr;

	/// Thrown when a plugin with the same name and base is loaded into a manager.
	class DuplicatePluginException : public std::runtime_error {
		public:
			/// Constructor taking the original and offending plugin.
			DuplicatePluginException(PluginPtr p1, PluginPtr p2);
	};

	/// Thrown when a resolver function is added a second time.
	class DuplicateResolverException : public std::runtime_error {
		public:
			/// Constuctor taking resolver type.
			DuplicateResolverException(const std::type_info &);
	};

	template <typename T>
	/// Typed plugin and handle to implementation.
	class DLL_PUBLIC PluginOf : public Plugin {
		public:
			/// Constructor taking an instance and name, filename and line of install for Plugin.
			PluginOf(const T * t, const std::string & n, const std::string & f, int l);
			~PluginOf();

			/// Get the type of this plugin.
			const std::type_info & type() const override;
			/// Get the implementation of this plugin.
			const T * implementation() const;

		private:
			const T * impl;
	};

	/// Container for loaded plugins.
	class DLL_PUBLIC PluginManager {
		public:
			typedef boost::function<boost::optional<std::string> (const std::type_info &, const std::string &)> PluginResolver;

			PluginManager();
			virtual ~PluginManager();

			/// Install a plugin.
			void add(PluginPtr);
			/// Uninstall a plugin.
			void remove(const std::string &, const std::type_info &);
			/// Get a specific plugin.
			PluginPtr get(const std::string &, const std::type_info &) const;
			/// Get all plugins.
			std::set<PluginPtr> getAll() const;
			/// Get all plugins of a specific type.
			std::set<PluginPtr> getAll(const std::type_info &) const;

			/**
			 * Install a plugin.
			 * @param i Implementation instance.
			 * @param n Name of plugin.
			 * @param f Filename of plugin.
			 * @param l Line number.
			 */
			template<typename T> void add(const T * i, const std::string & n, const std::string & f, int l);

			/**
			 * Uninstall a plugin.
			 * @param n Name of plugin.
			 */
			template<typename T> void remove(const std::string & n);

			/**
			 * Get a specific plugin.
			 * @param n Name of plugin.
			 */
			template<typename T> boost::shared_ptr<const PluginOf<T>> get(const std::string & n) const;

			/**
			 * Get the implementation from specific plugin.
			 * @param n Name of plugin.
			 */
			template<typename T> const T * getImplementation(const std::string & n) const;

			/**
			 * Get all plugins of a given time.
			 */
			template<typename T> std::set<boost::shared_ptr<const PluginOf<T>>> getAll() const;

			/**
			 * The number of installed plugins.
			 */
			size_t count() const;

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
			 * @param f The resolver type.
			 */
			void removeResolver(const std::type_info & t);

			/**
			 * Remove a type plugin resolver function.
			 */
			template<typename T> void removeResolver();

			/**
			 * The number of installed plugins.
			 */
			size_t countResolvers() const;

			/**
			 * Get the default plugin manager instance.
			 */
			static PluginManager * getDefault();

		private:
			typedef boost::multi_index_container<PluginPtr,
							boost::multi_index::indexed_by<
								boost::multi_index::ordered_non_unique<boost::multi_index::member<Plugin, const std::string, &Plugin::name>>,
								boost::multi_index::ordered_non_unique<boost::multi_index::const_mem_fun<Plugin, const std::type_info &, &Plugin::type>>,
								boost::multi_index::ordered_unique<
									boost::multi_index::composite_key<
										Plugin,
										boost::multi_index::member<Plugin, const std::string, &Plugin::name>,
										boost::multi_index::const_mem_fun<Plugin, const std::type_info &, &Plugin::type>
										>>
								>> PluginStore;

			typedef std::map<size_t, PluginResolver> TypePluginResolvers;

			PluginStore * plugins;
			TypePluginResolvers * resolvers;
	};
}

#define NAMEDPLUGIN(Name, Implementation, Base) \
	namespace MAKE_UNIQUE(__plugin__) { \
		static void InstallPlugin() __attribute__((constructor(102))); \
		void InstallPlugin() { \
			::AdHoc::PluginManager::getDefault()->add<Base>(new Implementation(), Name, __FILE__, __LINE__); \
		} \
		static void UninstallPlugin() __attribute__((destructor(102))); \
		void UninstallPlugin() { \
			::AdHoc::PluginManager::getDefault()->remove<Base>(Name); \
		} \
	}
#define PLUGIN(Implementation, Base) \
	NAMEDPLUGIN(#Implementation, Implementation, Base)

#endif

