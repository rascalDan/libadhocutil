#define BOOST_TEST_MODULE Context
#include <boost/test/unit_test.hpp>

#include "runtimeContext.h"

class TestRuntimeContext : public RuntimeContext {
	public:
		void run()
		{
			log += "a";
			swapContext();
			log += "b";
			swapContext();
			log += "c";
			swapContext();
			log += "d";
		}

		void callback() override
		{
			log += "e";
			swapContext();
			log += "f";
			swapContext();
		}

		std::string log;
};

BOOST_AUTO_TEST_CASE ( basic )
{
	TestRuntimeContext trc;
	trc.run();
	BOOST_REQUIRE_EQUAL("aebfcd", trc.log);
	BOOST_REQUIRE(trc.hasCompleted());
}

