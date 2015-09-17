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

static boost::optional<std::string> resolver(const std::type_info &, const std::string &);

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
resolver(const std::type_info &, const std::string &)
{
	return std::string();
}

BOOST_AUTO_TEST_CASE( addAndRemoveResolver )
{
	auto pm = AdHoc::PluginManager::getDefault();
	BOOST_REQUIRE_EQUAL(0, pm->countResolvers());
	pm->addResolver<BaseThing>(resolver);
	BOOST_REQUIRE_EQUAL(1, pm->countResolvers());
	BOOST_REQUIRE_THROW(pm->addResolver<BaseThing>(resolver), DuplicateResolverException);
	BOOST_REQUIRE_EQUAL(1, pm->countResolvers());
	pm->removeResolver<BaseThing>();
	BOOST_REQUIRE_EQUAL(0, pm->countResolvers());
}

