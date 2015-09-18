#define BOOST_TEST_MODULE Plugins
#include <boost/test/unit_test.hpp>

#include "plugins.h"
#include "plugins.impl.h"
#include "utilTestClasses.h"
#include <definedDirs.h>
#include <dlfcn.h>

using namespace AdHoc;

auto variant	= selfExe.parent_path().leaf();
auto toolset	= selfExe.parent_path().parent_path().leaf();
auto lib = rootDir / "bin" / toolset / variant / "libutilTestClasses.so";

static boost::optional<std::string> nullResolver(const std::type_info &, const std::string &);
static boost::optional<std::string> badResolver(const std::type_info &, const std::string &);
static boost::optional<std::string> goodResolver(const std::type_info &, const std::string &);

BOOST_AUTO_TEST_CASE( ready )
{
	BOOST_REQUIRE(PluginManager::getDefault());
	BOOST_REQUIRE(boost::filesystem::exists(lib));
}

BOOST_AUTO_TEST_CASE( clean )
{
	BOOST_REQUIRE_EQUAL(0, AdHoc::PluginManager::getDefault()->getAll<BaseThing>().size());
}

BOOST_AUTO_TEST_CASE( loadAndUnloadlib )
{
	void * handle = dlopen(lib.c_str(), RTLD_NOW);
	BOOST_REQUIRE(handle);
	BOOST_REQUIRE_EQUAL(1, AdHoc::PluginManager::getDefault()->getAll<BaseThing>().size());
	dlclose(handle);
	BOOST_REQUIRE_EQUAL(0, AdHoc::PluginManager::getDefault()->getAll<BaseThing>().size());
}

boost::optional<std::string>
nullResolver(const std::type_info &, const std::string &)
{
	return nullptr;
}

boost::optional<std::string>
badResolver(const std::type_info &, const std::string &)
{
	return std::string("dontexist");
}

boost::optional<std::string>
goodResolver(const std::type_info & t, const std::string & n)
{
	BOOST_REQUIRE_EQUAL(typeid(BaseThing), t);
	BOOST_REQUIRE_EQUAL("ImplOfThing", n);
	return lib.string();
}

BOOST_AUTO_TEST_CASE( addAndRemoveResolver )
{
	auto pm = AdHoc::PluginManager::getDefault();
	BOOST_REQUIRE_EQUAL(0, pm->countResolvers());
	pm->addResolver<BaseThing>(nullResolver);
	BOOST_REQUIRE_EQUAL(1, pm->countResolvers());
	BOOST_REQUIRE_THROW(pm->addResolver<BaseThing>(nullResolver), DuplicateResolverException);
	BOOST_REQUIRE_EQUAL(1, pm->countResolvers());
	pm->removeResolver<BaseThing>();
	BOOST_REQUIRE_EQUAL(0, pm->countResolvers());
}

BOOST_AUTO_TEST_CASE( null )
{
	auto pm = AdHoc::PluginManager::getDefault();
	pm->addResolver<BaseThing>(nullResolver);
	BOOST_REQUIRE_THROW(pm->get<BaseThing>("ImplOfThing"), AdHoc::NoSuchPluginException);
	pm->removeResolver<BaseThing>();
}

BOOST_AUTO_TEST_CASE( bad )
{
	auto pm = AdHoc::PluginManager::getDefault();
	pm->addResolver<BaseThing>(badResolver);
	BOOST_REQUIRE_THROW(pm->get<BaseThing>("ImplOfThing"), AdHoc::LoadLibraryException);
	pm->removeResolver<BaseThing>();
}

BOOST_AUTO_TEST_CASE( good )
{
	auto pm = AdHoc::PluginManager::getDefault();
	pm->addResolver<BaseThing>(goodResolver);
	BOOST_REQUIRE(pm->get<BaseThing>("ImplOfThing"));
	pm->removeResolver<BaseThing>();
}

