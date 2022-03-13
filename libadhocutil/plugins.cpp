#include "plugins.h"
#include "compileTimeFormatter.h"
#include "globalStatic.impl.h"
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/operators.hpp>
#include <cstdlib>
#include <cxxabi.h>
#include <dlfcn.h>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>

namespace std {
	bool
	operator<(const std::type_info & a, const std::type_info & b)
	{
		return a.hash_code() < b.hash_code();
	}

	std::ostream &
	operator<<(std::ostream & s, const std::type_info & t)
	{
		auto buf = std::unique_ptr<char, decltype(&std::free)>(
				__cxxabiv1::__cxa_demangle(t.name(), nullptr, nullptr, nullptr), std::free);
		s << buf.get();
		return s;
	}
}

namespace AdHoc {
	template class GlobalStatic<PluginManager>;

	AbstractPluginImplementation::~AbstractPluginImplementation() = default;

	Plugin::Plugin(const std::string_view n, const std::string_view f, int l) : name(n), filename(f), lineno(l) { }

	AdHocFormatter(NoSuchPluginExceptionMsg, "No such plugin: %? of type %?");
	NoSuchPluginException::NoSuchPluginException(const std::string_view n, const std::type_info & t) :
		std::runtime_error(NoSuchPluginExceptionMsg::get(n, t))
	{
	}

	AdHocFormatter(DuplicatePluginExceptionMsg, "Duplicate plugin %? for type %? at %?:%?, originally from %?:%?");
	DuplicatePluginException::DuplicatePluginException(const PluginPtr & p1, const PluginPtr & p2) :
		std::runtime_error(DuplicatePluginExceptionMsg::get(
				p1->name, p1->type(), p2->filename, p2->lineno, p1->filename, p1->lineno))
	{
	}

	AdHocFormatter(DuplicateResolverExceptionMsg, "Duplicate resolver function for type %?");
	DuplicateResolverException::DuplicateResolverException(const std::type_info & t) :
		std::runtime_error(DuplicateResolverExceptionMsg::get(t))
	{
	}

	AdHocFormatter(LoadLibraryExceptionMsg, "Failed to load library [%?]; %?");
	LoadLibraryException::LoadLibraryException(const std::string_view f, const std::string_view msg) :
		std::runtime_error(LoadLibraryExceptionMsg::get(f, msg))
	{
	}

	class PluginManager::PluginStore :
		public boost::multi_index_container<PluginPtr,
				boost::multi_index::indexed_by<
						boost::multi_index::ordered_non_unique<
								boost::multi_index::member<Plugin, const std::string, &Plugin::name>, std::less<>>,
						boost::multi_index::ordered_non_unique<
								boost::multi_index::const_mem_fun<Plugin, const std::type_info &, &Plugin::type>>,
						boost::multi_index::ordered_unique<
								boost::multi_index::composite_key<Plugin,
										boost::multi_index::member<Plugin, const std::string, &Plugin::name>,
										boost::multi_index::const_mem_fun<Plugin, const std::type_info &,
												&Plugin::type>>,
								std::less<>>>> {
	};

	class PluginManager::TypePluginResolvers : public std::map<size_t, PluginResolver> {
	};

	PluginManager::PluginManager() :
		plugins(std::make_unique<PluginStore>()), resolvers(std::make_unique<TypePluginResolvers>())
	{
	}

	PluginManager::~PluginManager() = default;

	PluginManager *
	PluginManager::getDefault()
	{
		return GlobalStatic<PluginManager>::get();
	}

	void
	PluginManager::add(const PluginPtr & p)
	{
		auto prev = plugins->insert(p);
		if (!prev.second) {
			throw DuplicatePluginException(*prev.first, p);
		}
	}

	void
	PluginManager::remove(const std::string_view n, const std::type_info & t)
	{
		auto r = plugins->get<2>().equal_range(std::make_tuple(n, std::cref(t)));
		plugins->get<2>().erase(r.first, r.second);
	}

	PluginPtr
	PluginManager::get(const std::string_view n, const std::type_info & t) const
	{
		auto r = plugins->get<2>().equal_range(std::make_tuple(n, std::cref(t)));
		if (r.first == r.second) {
			auto tr = resolvers->find(t.hash_code());
			if (tr != resolvers->end()) {
				if (auto lib = tr->second(t, n)) {
					loadLibrary(*lib);
				}
			}
			r = plugins->get<2>().equal_range(std::make_tuple(n, std::cref(t)));
		}
		if (r.first != r.second) {
			return (*r.first);
		}
		throw NoSuchPluginException(n, t);
	}

	void
	PluginManager::loadLibrary(const std::string & f)
	{
		void * handle = dlopen(f.c_str(), RTLD_NOW);
		if (!handle) {
			throw LoadLibraryException(f, dlerror());
		}
	}

	std::set<PluginPtr>
	PluginManager::getAll() const
	{
		return {plugins->begin(), plugins->end()};
	}

	std::set<PluginPtr>
	PluginManager::getAll(const std::type_info & t) const
	{
		auto r = plugins->get<1>().equal_range(t);
		return {r.first, r.second};
	}

	size_t
	PluginManager::count() const
	{
		return plugins->size();
	}

	void
	PluginManager::addResolver(const std::type_info & t, const PluginResolver & f)
	{
		auto prev = resolvers->insert(TypePluginResolvers::value_type(t.hash_code(), f));
		if (!prev.second) {
			throw DuplicateResolverException(t);
		}
	}

	void
	PluginManager::removeResolver(const std::type_info & t)
	{
		resolvers->erase(t.hash_code());
	}

	size_t
	PluginManager::countResolvers() const
	{
		return resolvers->size();
	}
}
