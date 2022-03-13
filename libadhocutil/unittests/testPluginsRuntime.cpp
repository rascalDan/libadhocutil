#define BOOST_TEST_MODULE Plugins
#include <boost/test/unit_test.hpp>

#include "plugins.impl.h"
#include "utilTestClasses.h"
#include <dlfcn.h>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <typeinfo>

using namespace AdHoc;

std::filesystem::path lib;
struct GetLibPath {
	GetLibPath()
	{
		BOOST_REQUIRE_EQUAL(2, boost::unit_test::framework::master_test_suite().argc);
		lib = boost::unit_test::framework::master_test_suite().argv[1];
	}
};
BOOST_TEST_GLOBAL_FIXTURE(GetLibPath);

static std::optional<std::string> nullResolver(const std::type_info &, const std::string_view);
static std::optional<std::string> badResolver(const std::type_info &, const std::string_view);
static std::optional<std::string> goodResolver(const std::type_info &, const std::string_view);

BOOST_AUTO_TEST_CASE(ready)
{
	BOOST_REQUIRE(PluginManager::getDefault());
	BOOST_REQUIRE(std::filesystem::exists(lib));
}

BOOST_AUTO_TEST_CASE(clean)
{
	BOOST_REQUIRE_EQUAL(0, AdHoc::PluginManager::getDefault()->getAll<BaseThing>().size());
}

BOOST_AUTO_TEST_CASE(loadAndUnloadlib)
{
	void * handle = dlopen(lib.c_str(), RTLD_NOW);
	BOOST_REQUIRE(handle);
	BOOST_REQUIRE_EQUAL(1, AdHoc::PluginManager::getDefault()->getAll<BaseThing>().size());
	dlclose(handle);
	BOOST_REQUIRE_EQUAL(0, AdHoc::PluginManager::getDefault()->getAll<BaseThing>().size());
}

std::optional<std::string>
nullResolver(const std::type_info &, const std::string_view)
{
	return {};
}

std::optional<std::string>
badResolver(const std::type_info &, const std::string_view)
{
	return std::string("dontexist");
}

std::optional<std::string>
goodResolver(const std::type_info & t, const std::string_view n)
{
	BOOST_REQUIRE_EQUAL(typeid(BaseThing), t);
	BOOST_REQUIRE_EQUAL("ImplOfThing", n);
	return lib;
}

BOOST_AUTO_TEST_CASE(addAndRemoveResolver)
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

BOOST_AUTO_TEST_CASE(null)
{
	auto pm = AdHoc::PluginManager::getDefault();
	pm->addResolver<BaseThing>(nullResolver);
	BOOST_REQUIRE_THROW((void)pm->get<BaseThing>("ImplOfThing"), AdHoc::NoSuchPluginException);
	pm->removeResolver<BaseThing>();
}

BOOST_AUTO_TEST_CASE(bad)
{
	auto pm = AdHoc::PluginManager::getDefault();
	pm->addResolver<BaseThing>(badResolver);
	BOOST_REQUIRE_THROW((void)pm->get<BaseThing>("ImplOfThing"), AdHoc::LoadLibraryException);
	pm->removeResolver<BaseThing>();
}

BOOST_AUTO_TEST_CASE(good)
{
	auto pm = AdHoc::PluginManager::getDefault();
	pm->addResolver<BaseThing>(goodResolver);
	BOOST_REQUIRE(pm->get<BaseThing>("ImplOfThing"));
	pm->removeResolver<BaseThing>();
}
