#define BOOST_TEST_MODULE Factory
#include <boost/test/unit_test.hpp>

#include "factory.h"
#include "factory.impl.h"
#include "plugins.h"
#include "plugins.impl.h"

using namespace AdHoc;

class BaseThing {
	public:
		BaseThing(int, const std::string &){}
		virtual ~BaseThing() = default;

		virtual void execute() const = 0;
};

class ImplOfThing : public BaseThing {
	public:
		ImplOfThing(int i, const std::string & s) : BaseThing(i, s) {}
		void execute() const { }
};
class OtherImplOfThing : public BaseThing {
	public:
		OtherImplOfThing(int i, const std::string & s) : BaseThing(i, s) {}
		void execute() const { }
};

typedef AdHoc::Factory<BaseThing, int, std::string> BaseThingFactory;

NAMEDFACTORY("a", ImplOfThing, BaseThingFactory);
FACTORY(OtherImplOfThing, BaseThingFactory);

INSTANTIATEFACTORY(BaseThing, int, std::string);
// Multiple factories in one compilation unit
INSTANTIATEFACTORY(BaseThing, std::string, std::string);
// Factories of things with commas
INSTANTIATEFACTORY(BaseThing, std::map<std::string, std::string>);

BOOST_AUTO_TEST_CASE( ready )
{
	BOOST_REQUIRE_EQUAL(2, PluginManager::getDefault()->count());
}

BOOST_AUTO_TEST_CASE( get )
{
	auto all = PluginManager::getDefault()->getAll();
	auto factory1 = PluginManager::getDefault()->get<BaseThingFactory>("a")->implementation();
	auto factory2 = BaseThingFactory::get("a");
	auto factory3 = BaseThingFactory::get("OtherImplOfThing");
	BOOST_REQUIRE(factory1);
	BOOST_REQUIRE_EQUAL(factory1, factory2);
	BOOST_REQUIRE(factory3);
	BOOST_REQUIRE(factory1 != factory3);
}

BOOST_AUTO_TEST_CASE( create )
{
	auto factory1 = BaseThingFactory::get("a");
	auto factory2 = BaseThingFactory::get("OtherImplOfThing");
	auto i1 = factory1->create(1, "std");
	auto i2 = factory1->create(1, "std");
	auto i3 = factory2->create(1, "std");
	BOOST_REQUIRE(i1);
	BOOST_REQUIRE(i2);
	BOOST_REQUIRE(i3);
	BOOST_REQUIRE(i1 != i2);
	BOOST_REQUIRE(i1 != i3);
	BOOST_REQUIRE(i2 != i3);
}

BOOST_AUTO_TEST_CASE( createNew )
{
	auto i = BaseThingFactory::createNew("a", 1, "std");
	BOOST_REQUIRE(i);
	delete i;
}

