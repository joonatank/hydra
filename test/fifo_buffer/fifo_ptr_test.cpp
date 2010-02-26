#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/time.h>

#include "../fifo_buffer.hpp"

#include "fifo_test.hpp"

void
timer_off_handler( int signum )
{
	if( signum == SIGALRM )
	{ g_timer_off++; }
}

int main( void )
{
	std::cout << "Running fifo buffer test" << std::endl;

	eqOgre::fifo_buffer<char *> fifo_ptr(1024);
	pthread_t read_thread, write_thread;

	if( signal( SIGALRM, timer_off_handler ) == SIG_ERR )
	{
		std::cout << "Failed to set timer signal handler" << std::endl;
		exit(-1);
	}
	// Set timer
	/*
	if( getitimer( ITIMER_VIRTUAL, 0 ) != 0 )
	{
		std::cout << "Timer already set... " << std::endl;
		exit(0);
	}
	else
		*/
	// We are printing every second
	struct itimerval val;
	val.it_interval.tv_sec = 1;
	val.it_interval.tv_usec = 0;
	val.it_value.tv_sec = 1;
	val.it_value.tv_usec = 0;
	if( setitimer( ITIMER_REAL, &val, 0 ) != 0 )
	{
		std::cout << "Timer set failed" << std::endl;
		exit(-1);
	}

	// fifo_buffer T* test
	int ret_read
		= pthread_create( &read_thread, NULL, read_data_ptr, (void*)(&fifo_ptr) );
	int ret_write
		= pthread_create( &write_thread, NULL, write_data_ptr, (void*)(&fifo_ptr) );

	// Wait
	pthread_join( write_thread, NULL );
	pthread_join( read_thread, NULL );

	std::cout << "Completed ptr read/write test : retvalue = " << ret_read
		<< ", " << ret_write << std::endl;

	// Reset timer
	g_timer_off = 0;
	if( setitimer( ITIMER_REAL, &val, 0 ) != 0 )
	{
		std::cout << "Timer set failed" << std::endl;
		exit(-1);
	}

	eqOgre::fifo_buffer<Test> fifo(1024);

	// fifo_buffer T test
	ret_read
		= pthread_create( &read_thread, NULL, read_data, (void*)(&fifo) );
	ret_write
		= pthread_create( &write_thread, NULL, write_data, (void*)(&fifo) );

	// Wait
	pthread_join( write_thread, NULL );
	pthread_join( read_thread, NULL );

	std::cout << "Completed read/write test : retvalue = " << ret_read
		<< ", " << ret_write << std::endl;

	// Reset timer
	g_timer_off = 0;
	if( setitimer( ITIMER_REAL, &val, 0 ) != 0 )
	{
		std::cout << "Timer set failed" << std::endl;
		exit(-1);
	}

	eqOgre::fifo_buffer<Test> fifo_cout(1024);

	// fifo_buffer T test
	ret_read
		= pthread_create( &read_thread, NULL, read_data_cout, (void*)(&fifo_cout) );
	ret_write
		= pthread_create( &write_thread, NULL, write_data, (void*)(&fifo_cout) );

	// Wait
	pthread_join( write_thread, NULL );
	pthread_join( read_thread, NULL );

	std::cout << "Completed read/write test : retvalue = " << ret_read
		<< ", " << ret_write << std::endl;

	return 0;
}

