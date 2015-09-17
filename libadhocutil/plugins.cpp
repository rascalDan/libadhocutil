#include "plugins.h"
#include <string.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include "buffer.h"

namespace std {
	bool
	operator<(const std::type_info & a, const std::type_info & b)
	{
		return a.hash_code() < b.hash_code();
	}

	bool
	operator<(const AdHoc::PluginManager::PluginResolver & a, const AdHoc::PluginManager::PluginResolver & b)
	{
		return a.boost::function_base::get_vtable() < b.boost::function_base::get_vtable();
	}

	std::ostream &
	operator<<(std::ostream & s, const std::type_info & t)
	{
		char * buf = __cxxabiv1::__cxa_demangle(t.name(), NULL, NULL, NULL);
		s << buf;
		free(buf);
		return s;
	}
}

namespace AdHoc {
	static void createDefaultManager() __attribute__((constructor(101)));
	static void deleteDefaultManager() __attribute__((destructor(101)));

	static AdHoc::PluginManager * defaultPluginManager;

	static
	void
	createDefaultManager()
	{
		defaultPluginManager = new PluginManager();
	}

	static
	void
	deleteDefaultManager()
	{
		delete defaultPluginManager;
		defaultPluginManager = nullptr;
	}

	Plugin::Plugin(const std::string & n, const std::string & f, int l) :
		name(n),
		filename(f),
		lineno(l)
	{
	}

	Plugin::~Plugin() = default;

	NoSuchPluginException::NoSuchPluginException(const std::string & n, const std::type_info & t) :
		std::runtime_error(stringbf("No such plugin: %s of type %s", n, t))
	{
	}

	DuplicatePluginException::DuplicatePluginException(PluginPtr p1, PluginPtr p2) :
		std::runtime_error(stringbf("Duplicate plugin %s for type %s at %s:%d, originally from %s:%d",
				p1->name, p1->type(), p2->filename, p2->lineno, p1->filename, p1->lineno))
	{
	}

	DuplicateResolverException::DuplicateResolverException(const std::type_info & t) :
		std::runtime_error(stringbf("Duplicate resolver function for type %s", t))
	{
	}

	PluginManager::PluginManager() :
		plugins(new PluginStore()),
		resolvers(new TypePluginResolvers())
	{
	}

	PluginManager::~PluginManager()
	{
		delete resolvers;
		delete plugins;
	}

	PluginManager *
	PluginManager::getDefault()
	{
		return defaultPluginManager;
	}

	void
	PluginManager::add(PluginPtr p)
	{
		auto prev = plugins->insert(p);
		if (!prev.second) {
			throw DuplicatePluginException(*prev.first, p);
		}
	}

	void
	PluginManager::remove(const std::string & n, const std::type_info & t)
	{
		auto r = plugins->get<2>().equal_range(boost::make_tuple(n, std::cref(t)));
		plugins->get<2>().erase(r.first, r.second);
	}

	PluginPtr
	PluginManager::get(const std::string & n, const std::type_info & t) const
	{
		auto r = plugins->get<2>().equal_range(boost::make_tuple(n, std::cref(t)));
		if (r.first != r.second) {
			return (*r.first);
		}
		throw NoSuchPluginException(n, t);
	}

	std::set<PluginPtr>
	PluginManager::getAll() const
	{
		std::set<PluginPtr> all;
		for(const auto & p : *plugins) {
			all.insert(p);
		}
		return all;
	}

	std::set<PluginPtr>
	PluginManager::getAll(const std::type_info & t) const
	{
		auto r = plugins->get<1>().equal_range(t);
		return std::set<PluginPtr>(r.first, r.second);
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

