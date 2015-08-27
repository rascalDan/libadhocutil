#define BOOST_TEST_MODULE Context
#include <boost/test/unit_test.hpp>

#include "runtimeContext.h"

class testRuntimeContext : RuntimeContext {
	public:
		void Run()
		{
			log += "a";
			SwapContext();
			log += "b";
			SwapContext();
			log += "c";
		}

		void Callback() override
		{
			log += "d";
			SwapContext();
			log += "e";
			SwapContext();
		}

		std::string log;
};

BOOST_AUTO_TEST_CASE ( basic )
{
	testRuntimeContext trc;
	trc.Run();
	BOOST_REQUIRE_EQUAL("adbec", trc.log);
}

