#include "server.hpp"

#include "base/exceptions.hpp"

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
	for( size_t i = 0; i < _commands.size(); ++i )
	{
		boost::shared_ptr<udp::Command> cmd = _commands.at(i);

		// Message is too short
		if( msg.size() < cmd->getSize() )
		{
			// Calculate the number of bytes needed for the next command
			size_t missing_bytes = cmd->getSize() - msg.size();
			// Add the number of bytes needed for all the remaining commands
			for( size_t j = i+i; j < _commands.size(); ++j )
			{
				missing_bytes =+ sizeof(double) * _commands.at(j)->getSize();
			}
			BOOST_THROW_EXCEPTION( vl::short_message() << vl::bytes(missing_bytes) );
		}

		// Assign the new values
		(*cmd)<<msg;

		// execute the command
		(*cmd)();
	}

	if( msg.size() != 0u )
	{
		size_t extra_bytes = msg.size()*sizeof(double);
		BOOST_THROW_EXCEPTION( vl::long_message() << vl::bytes(extra_bytes) );
	}
}
