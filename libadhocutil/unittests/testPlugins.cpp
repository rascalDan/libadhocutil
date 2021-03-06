#define BOOST_TEST_MODULE Plugins
#include <boost/test/unit_test.hpp>

#include "plugins.impl.h"
#include "utilTestClasses.h"
#include <memory>
#include <set>

using namespace AdHoc;

BOOST_AUTO_TEST_CASE(ready)
{
	BOOST_REQUIRE(PluginManager::getDefault());
}

BOOST_AUTO_TEST_CASE(registered)
{
	BOOST_REQUIRE_EQUAL(1, PluginManager::getDefault()->count());
}

BOOST_AUTO_TEST_CASE(private_instance)
{
	std::make_unique<PluginManager>();
}

BOOST_AUTO_TEST_CASE(get)
{
	auto implOfThingPlugin = PluginManager::getDefault()->get<BaseThing>("ImplOfThing");
	BOOST_REQUIRE(implOfThingPlugin);
	auto implOfThing = implOfThingPlugin->implementation();
	BOOST_REQUIRE(implOfThing);
	auto implOfThingDirect = PluginManager::getDefault()->getImplementation<BaseThing>("ImplOfThing");
	BOOST_REQUIRE_EQUAL(implOfThing, implOfThingDirect);
}

BOOST_AUTO_TEST_CASE(getAll)
{
	auto all = PluginManager::getDefault()->getAll();
	BOOST_REQUIRE_EQUAL(1, all.size());
	auto base = (*all.begin())->instance();
	BOOST_REQUIRE(base);
	BOOST_REQUIRE(std::dynamic_pointer_cast<const BaseThing>(base));
	BOOST_REQUIRE(std::dynamic_pointer_cast<const ImplOfThing>(base));
	auto allOf = PluginManager::getDefault()->getAll<BaseThing>();
	BOOST_REQUIRE_EQUAL(1, allOf.size());
}

BOOST_AUTO_TEST_CASE(addManual)
{
	auto o1 = PluginManager::getDefault()->get<BaseThing>("ImplOfThing");
	PluginManager::getDefault()->add(
			std::make_shared<PluginOf<BaseThing>>(std::make_shared<ImplOfThing>(), "custom1", __FILE__, __LINE__));
	BOOST_REQUIRE_EQUAL(2, PluginManager::getDefault()->count());
	auto c1 = PluginManager::getDefault()->get<BaseThing>("custom1");
	PluginManager::getDefault()->add<BaseThing>(std::make_shared<ImplOfThing>(), "custom2", __FILE__, __LINE__);
	BOOST_REQUIRE_EQUAL(3, PluginManager::getDefault()->count());
	auto c2 = PluginManager::getDefault()->get<BaseThing>("custom2");
	auto o2 = PluginManager::getDefault()->get<BaseThing>("ImplOfThing");
	BOOST_REQUIRE(o1);
	BOOST_REQUIRE(c1);
	BOOST_REQUIRE(c2);
	BOOST_REQUIRE(o2);
	BOOST_REQUIRE_EQUAL(o1, o2);
	BOOST_REQUIRE_NE(c1, o1);
	BOOST_REQUIRE_NE(c2, o1);
}

BOOST_AUTO_TEST_CASE(removeManual)
{
	BOOST_REQUIRE_EQUAL(3, PluginManager::getDefault()->count());
	PluginManager::getDefault()->remove<BaseThing>("custom1");
	BOOST_REQUIRE_EQUAL(2, PluginManager::getDefault()->count());
	BOOST_REQUIRE_THROW((void)PluginManager::getDefault()->get<BaseThing>("custom1"), NoSuchPluginException);
	BOOST_REQUIRE(PluginManager::getDefault()->get<BaseThing>("custom2"));
	BOOST_REQUIRE(PluginManager::getDefault()->get<BaseThing>("ImplOfThing"));
	PluginManager::getDefault()->remove<BaseThing>("custom2");
	BOOST_REQUIRE_EQUAL(1, PluginManager::getDefault()->count());
	BOOST_REQUIRE_THROW((void)PluginManager::getDefault()->get<BaseThing>("custom2"), NoSuchPluginException);
	BOOST_REQUIRE(PluginManager::getDefault()->get<BaseThing>("ImplOfThing"));
}

BOOST_AUTO_TEST_CASE(createAndRemove)
{
	PluginManager::getDefault()->create<BaseThing, ImplOfThing>("custom1", __FILE__, __LINE__);
	BOOST_REQUIRE_EQUAL(2, PluginManager::getDefault()->count());
	auto c1 = PluginManager::getDefault()->get<BaseThing>("custom1");
	BOOST_REQUIRE(c1);
	PluginManager::getDefault()->remove<BaseThing>("custom1");
}

BOOST_AUTO_TEST_CASE(nameAndTypeClash)
{
	// Same name, different type
	PluginManager::getDefault()->add<OtherBase>(std::make_shared<OtherImpl>(), "ImplOfThing", __FILE__, __LINE__);
	// Different name, same type
	PluginManager::getDefault()->add<BaseThing>(std::make_shared<ImplOfThing>(), "Different", __FILE__, __LINE__);
	// Same name, same thing, should error
	BOOST_REQUIRE_THROW(PluginManager::getDefault()->add<BaseThing>(
								std::make_shared<OtherImplOfThing>(), "ImplOfThing", __FILE__, __LINE__),
			DuplicatePluginException);
	PluginManager::getDefault()->remove<OtherBase>("ImplOfThing");
	PluginManager::getDefault()->remove<BaseThing>("Different");
}

BOOST_AUTO_TEST_CASE(otherTypes)
{
	PluginManager::getDefault()->add<OtherBase>(std::make_shared<OtherImpl>(), "ImplOfThing", __FILE__, __LINE__);
	BOOST_REQUIRE_EQUAL(2, PluginManager::getDefault()->count());
	BOOST_REQUIRE_EQUAL(2, PluginManager::getDefault()->getAll().size());
	BOOST_REQUIRE_EQUAL(1, PluginManager::getDefault()->getAll<BaseThing>().size());
	BOOST_REQUIRE_EQUAL(1, PluginManager::getDefault()->getAll<OtherBase>().size());
	PluginPtr p1 = PluginManager::getDefault()->get<BaseThing>("ImplOfThing");
	PluginPtr p2 = PluginManager::getDefault()->get<OtherBase>("ImplOfThing");
	BOOST_REQUIRE(p1);
	BOOST_REQUIRE(p2);
	BOOST_REQUIRE_NE(p1, p2);
	PluginManager::getDefault()->remove<OtherBase>("ImplOfThing");
	BOOST_REQUIRE_EQUAL(1, PluginManager::getDefault()->count());
	BOOST_REQUIRE(PluginManager::getDefault()->get<BaseThing>("ImplOfThing"));
}
