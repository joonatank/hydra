#include "server.hpp"

#include <iostream>

vl::udp::Server::Server(  uint16_t const port  )
	: socket(io_service, boost::udp::endpoint(boost::udp::v4(), port))
{
}


vl::udp::Server::~Server()
{
}

void
vl::udp::Server::mainloop( void )
{
	std::vector<double> recv_buf(128);
	boost::udp::endpoint remote_endpoint;
	boost::system::error_code error;
	size_t n = socket.receive_from( boost::asio::buffer(recv_buf),
		remote_endpoint, 0, error );

	if (error && error != boost::asio::error::message_size)
	{ throw boost::system::system_error(error); }

	// Remove extra space from the message
	recv_buf.resize( n/sizeof(double) );

	handle( recv_buf );
}

void vl::udp::Server::addCommand(vl::udp::CommandRefPtr cmd)
{
	_commands.push_back(cmd);
}


// Private
void
vl::udp::Server::handle(std::vector< double > msg)
{
	// TODO handle packets with IDs
	// Handler code
	int msg_pos = 0;
	for( size_t i = 0; i < _commands.size(); ++i )
	{
		boost::shared_ptr<udp::Command> cmd = _commands.at(i);

		// Message is too short
		// TODO should throw
		if( msg.size() - msg_pos < cmd->getSize() )
		{
			std::cerr << "Packet size is too small for the next command" << std::endl;
			break;
		}

		// Assign the new values
		for( size_t j = 0; j < cmd->getSize(); ++j )
		{
			cmd->at(i) = msg.at(i+j);
			msg_pos++;
		}

		// execute the command
		(*cmd)();
	}

	if( msg_pos != msg.size() )
	{
		// TODO should throw
		std::cerr << (msg.size()-msg_pos)*sizeof(double)
			<< " unhandled bytes in the message." << std::endl;
	}
}
