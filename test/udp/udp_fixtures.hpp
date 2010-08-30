#include <vector>

#include "udp/client.hpp"
#include "udp/server.hpp"

// Some hard coded variables
char const *HOST = "localhost";
char const *PORT_STR = "2244";
uint16_t const PORT = 2244;

struct TestUdpFixture
{
public :
	TestUdpFixture( void )
		: server( PORT ), client( HOST, PORT ),
		  msg_pos(3), msg_rot_quat(4), msg_rot_aa(4)
	{
		BOOST_TEST_MESSAGE( "Starting UDP server on port " << PORT );

		BOOST_TEST_MESSAGE( "Starting UDP client" );

		BOOST_TEST_MESSAGE( "Client connecting to host : " << HOST << " "
			<< "using port = " << PORT );

		// Initialize some test vectors
		for( size_t i = 0; i < msg_pos.size(); ++i )
		{
			msg_pos.at(i) = (double)(i) + 0.02*i;
		}

		// Rotation of 90 degs (half a PI) around Z axis
		msg_rot_quat.at(0) = 0.7071;
		msg_rot_quat.at(1) = 0;
		msg_rot_quat.at(2) = 0;
		msg_rot_quat.at(3) = 0.7071;

		// Rotation of 90 degs (half a PI) around Z axis
		msg_rot_aa.at(0) = 1.5707;
		msg_rot_aa.at(1) = 0;
		msg_rot_aa.at(2) = 0;
		msg_rot_aa.at(3) = 1;
	}

	~TestUdpFixture( void )
	{
	}

	void send( std::vector<double> msg )
	{
		client.send( msg );

		vl::msleep( 1 );

		server.mainloop();
	}

	vl::udp::Server server;
	vl::udp::Client client;

	// Test data vectors
	std::vector<double> msg_pos;
	std::vector<double> msg_rot_quat;
	std::vector<double> msg_rot_aa;
};

void add_vec( std::vector<double> &sink, std::vector<double> const &source )
{
	for( size_t i = 0; i < source.size(); ++i )
	{
		sink.push_back( source.at(i) );
	}
}
