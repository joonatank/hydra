/*	Joonatan Kuosa
 *	2010-08
 *	Test program that contains both udp::Server and udp::Client
 */
#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE udp_conf_parser

#include <boost/test/unit_test.hpp>

// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/client.hpp"
#include "udp/print_command.hpp"
#include <base/sleep.hpp>

char const *HOST = "localhost";
char const *PORT_STR = "2244";
uint16_t const PORT = 2244;

struct TestUdpFixture
{
public :
	TestUdpFixture( void )
		: server( PORT ), client( HOST, PORT_STR )
	{
	}

	~TestUdpFixture( void )
	{
	}

	void run( void )
	{
		for( size_t j = 0; j < 3; ++j )
		{
			std::vector<double> test_vec(9+j);
			for( size_t i = 0; i < test_vec.size(); ++i )
			{
				test_vec.at(i) = i;
			}

			client.send( test_vec );

			std::cout << "Message sent" << std::endl;

			// Sleep
			// TODO crossplatform sleep
			vl::usleep( 1 );

			server.mainloop();
		}
	}

	vl::udp::Server server;
	vl::udp::Client client;
};

BOOST_FIXTURE_TEST_SUITE( TestUDP, TestUdpFixture )

BOOST_AUTO_TEST_CASE( sending )
{
	std::cout << "Starting UDP server on port " << PORT << std::endl;

	boost::shared_ptr<vl::udp::Command> cmd( new vl::udp::PrintCommand("setPosition", "feet" ) );
	server.addCommand( cmd );
	cmd.reset( new vl::udp::PrintCommand("setPosition", "feet" ) );
	server.addCommand( cmd );
	cmd.reset( new vl::udp::PrintCommand("setQuaternion", "feet" ) );
	server.addCommand( cmd );

	std::cout << "Starting UDP client" << std::endl;

	std::cout << "Client connecting to host : " << HOST << " "
		<< "using port = " << PORT << std::endl;

	std::cout << "Client created" << std::endl;

	run();
}

// Test throwing when too short packet is sent from client
BOOST_AUTO_TEST_CASE( too_short_packet )
{
}

// Test throwing when too long packet is sent from client
BOOST_AUTO_TEST_CASE( too_long_packet )
{
}

BOOST_AUTO_TEST_SUITE_END()