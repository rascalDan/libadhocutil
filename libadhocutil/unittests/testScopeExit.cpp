#define BOOST_TEST_MODULE ScopeExit
#include <boost/test/unit_test.hpp>

#include "scopeExit.h"
#include <string>

using namespace AdHoc;

BOOST_AUTO_TEST_CASE(cleanexit)
{
	std::string log;
	{
		ScopeExit se(
				[&log] {
					log += "before";
				},
				[&log] {
					log += "clean";
				},
				[&log] {
					log += "error";
				},
				[&log] {
					log += "after";
				});
	}
	BOOST_REQUIRE_EQUAL(log, "beforecleanafter");
}

BOOST_AUTO_TEST_CASE(uncaught)
{
	BOOST_REQUIRE_THROW(
			{
				std::string log;
				{
					ScopeExit se(
							[&log] {
								log += "before";
							},
							[&log] {
								log += "clean";
							},
							[&log] {
								log += "error";
							},
							[&log] {
								log += "after";
							});
					throw std::runtime_error("test unclean exit");
				}
				BOOST_REQUIRE_EQUAL(log, "beforeerrorafter");
			},
			std::runtime_error);
}
