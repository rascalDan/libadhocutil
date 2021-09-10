#ifndef ADHOCUTIL_BOOST_TEST_EXTRA_H
#define ADHOCUTIL_BOOST_TEST_EXTRA_H

#define BOOST_CHECK_IF(VAR, EXPR) \
	if (auto VAR = (EXPR); !VAR) { \
		BOOST_CHECK(VAR); \
	} \
	else

#endif
