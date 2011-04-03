#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE time

#include <boost/test/unit_test.hpp>

/// tested class
#include "base/timer.hpp"

#include "base/sleep.hpp"

BOOST_AUTO_TEST_CASE( time_arithmetic )
{
	vl::time t1(10, 9);
	vl::time t2(0, 20);
	BOOST_CHECK_EQUAL( t1 + t1, vl::time(20, 18) );
	BOOST_CHECK_EQUAL( t1 - t1, vl::time() );
	BOOST_CHECK_EQUAL( t1 + t2, vl::time(10, 29) );
	BOOST_CHECK_EQUAL( t1 - t2, vl::time(9, 1e6-11) );

	// TODO check no overflows to usec when adding two times

	// TODO check dividing and multiplication with doubles
	// TODO check that negative numbers are disallowed

	// TODO check dividing and multiplication with unsigned ints
}

BOOST_AUTO_TEST_CASE( timer_sleep )
{
	vl::timer t;

	// Probably does not work on Windows, for sure not on XP
	// because they sleep way too much if you allow them to sleep at all :)
	uint32_t sleep_time = 1;
	// TODO we need a sleep with time input
	vl::msleep(sleep_time);
	vl::time timed = t.elapsed();

	// Need to give a small epsilon
	const vl::time epsilon(0, 100);
	std::cout << "slept for = " << timed << std::endl;
	BOOST_CHECK_LT( timed - epsilon, vl::time(0, sleep_time*1e3) );
	BOOST_CHECK_GT( timed + epsilon, vl::time(0, sleep_time*1e3) );

	sleep_time = 20;
	t.reset();
	vl::msleep(sleep_time);
	timed = t.elapsed();

	std::cout << "slept for = " << timed << std::endl;
	BOOST_CHECK_LT( timed - epsilon, vl::time(0, sleep_time*1e3) );
	BOOST_CHECK_GT( timed + epsilon, vl::time(0, sleep_time*1e3) );
}
