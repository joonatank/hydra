/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "server.hpp"

#include "base/exceptions.hpp"

#include "message.hpp"

vl::cluster::Server::Server(  uint16_t const port  )
	: socket(io_service, boost::udp::endpoint(boost::udp::v4(), port))
{
}


vl::cluster::Server::~Server()
{
}

void
vl::cluster::Server::mainloop( void )
{
// 	std::cout << "vl::cluster::Server::mainloop" << std::endl;

	if( socket.available() != 0 )
	{
		std::cout << "vl::cluster::Server::mainloop has a message" << std::endl;
		// TODO this really should handle all the messages
		std::vector<char> recv_buf(512);
		boost::udp::endpoint remote_endpoint;
		boost::system::error_code error;

		size_t n = socket.receive_from( boost::asio::buffer(recv_buf),
			remote_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		handle( recv_buf );
	}
}

void
vl::cluster::Server::sendToAll( vl::cluster::Message const &msg )
{
// 	std::cout << "vl::cluster::Server::sendToAll" << std::endl;
}

// Private
void
vl::cluster::Server::handle( std::vector<char> msg )
{
	std::cout << "vl::cluster::Server::handle" << std::endl;
// 	// TODO handle packets with IDs
// 	// Handler code
// 	for( size_t i = 0; i < _commands.size(); ++i )
// 	{
// 		boost::shared_ptr<udp::Command> cmd = _commands.at(i);
//
// 		(*cmd) << msg;
//
// 		// execute the command
// 		(*cmd)();
// 	}
//
// 	if( msg.size() != 0u )
// 	{
// 		size_t extra_bytes = msg.size()*sizeof(double);
// 		BOOST_THROW_EXCEPTION( vl::long_message() << vl::bytes(extra_bytes) );
// 	}
}
