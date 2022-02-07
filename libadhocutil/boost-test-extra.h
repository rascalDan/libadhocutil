#pragma once

#define BOOST_CHECK_IF(VAR, EXPR) \
	if (auto VAR = (EXPR); !VAR) { \
		BOOST_CHECK(VAR); \
	} \
	else

#define BOOST_CHECK_EQUAL_IF(EQ, EXPR) \
	if (auto VAR = (EXPR); !(VAR == EQ)) { \
		BOOST_CHECK_EQUAL(VAR, EQ); \
	} \
	else
