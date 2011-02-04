/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "server.hpp"

#include "base/exceptions.hpp"

#include "message.hpp"

vl::cluster::Server::Server( uint16_t const port )
	: _socket(_io_service, boost::udp::endpoint(boost::udp::v4(), port))
{
	std::cout << "vl::cluster::Server::Server : " << "Creating server to port "
		<< port << std::endl;
}


vl::cluster::Server::~Server()
{
}

void
vl::cluster::Server::mainloop( void )
{
// 	std::cout << "vl::cluster::Server::mainloop" << std::endl;

	// TODO
	// We should check that multiple clients work correctly
	// At the moment we use single socket for all clients
	// Better to use multiple sockets, one for each client
	if( _socket.available() != 0 )
	{
// 		std::cout << "vl::cluster::Server::mainloop has a message" << std::endl;
		std::vector<char> recv_buf( _socket.available() );
		boost::udp::endpoint remote_endpoint;
		boost::system::error_code error;

		// TODO we should check that all the bytes are read
		_socket.receive_from( boost::asio::buffer(recv_buf),
			remote_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		Message *msg = new Message(recv_buf);
		switch( msg->getType() )
		{
			case vl::cluster::MSG_REG_UPDATES :
			{
				std::cout << "vl::cluster::MSG_REG_UPDATES message received."
					<< std::endl;
				_addClient( remote_endpoint );
				if( !_env_msg.empty() )
				{
					_sendEnvironment(_env_msg);

					// TODO this should be sent after ACK for MSG_ENVIRONMENT
					if( !_proj_msg.empty() )
					{ _sendProject(_proj_msg); }
				}
				delete msg;
			}
			break;

			case vl::cluster::MSG_INPUT :
			{
				// TODO there should be a maximum amount of messages stored
				_input_msgs.push_back( msg );
			}
			break;

			default :
			{
				std::cout << "vl::cluster::Server::mainloop : "
					<< "Unhandeled message type." << std::endl;
				delete msg;
			}
			break;
		}
	}
}

void
vl::cluster::Server::sendEnvironment( const vl::cluster::Message &msg )
{
	std::cout << "vl::cluster::Server::sendEnvironment" << std::endl;

	_env_msg.clear();
	msg.dump(_env_msg);
	_sendEnvironment(_env_msg);
}

/// TODO
/// This is problematic because we allow for sending new project messages
/// But this calls _sendProject which only sends the project to those clients
/// That are on the correct state
/// This one needs to reset every clients state or we need to use different
/// system.
void
vl::cluster::Server::sendProject( const vl::cluster::Message &msg )
{
	std::cout << "vl::cluster::Server::sendProject" << std::endl;

	_proj_msg.clear();
	msg.dump(_proj_msg);
	_sendProject(_proj_msg);
}

void
vl::cluster::Server::sendUpdate( vl::cluster::Message const &msg )
{
// 	std::cout << "Sending message " << msg;

// 	std::cout << "vl::cluster::Server::sendToAll" << std::endl;
	std::vector<char> buf;
	msg.dump(buf);
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		// Check that the client is in the correct state
		if( iter->second == CS_INIT || iter->second == CS_SWAP )
		{
			_socket.send_to( boost::asio::buffer(buf), iter->first );
		}
	}
}

void
vl::cluster::Server::sendInit( vl::cluster::Message const &msg )
{
// 	std::cout << "vl::cluster::Server::sendToNewClients" << std::endl;
// 	std::cout << "Sending message " << msg;
	std::vector<char> buf;
	msg.dump(buf);

	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->second == CS_PROJ )
		{
	// 		std::cout << "Sending to client = " << *iter << std::endl;
			_socket.send_to( boost::asio::buffer(buf), iter->first );
			iter->second = CS_INIT;
		}
	}
}

void
vl::cluster::Server::sendDraw( const vl::cluster::Message& msg )
{
	std::cout << "vl::cluster::Server::sendDraw" << std::endl;
}

void
vl::cluster::Server::sendSwap( const vl::cluster::Message& msg )
{
	std::cout << "vl::cluster::Server::sendSwap" << std::endl;
}

bool
vl::cluster::Server::needsInit( void ) const
{
	ClientList::const_iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->second == CS_PROJ )
		{
			return true;
		}
	}

	return false;
}

vl::cluster::Message *
vl::cluster::Server::popInputMessage( void )
{
	Message *tmp = _input_msgs.front();
	_input_msgs.erase( _input_msgs.begin() );
	return tmp;
}


/// ----------------------------- Private --------------------------------------
void
vl::cluster::Server::_addClient( boost::udp::endpoint const &endpoint )
{
	std::cout << "vl::cluster::Server::_addClient : endpoint " << endpoint << std::endl;
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->first == endpoint )
		{
			std::cerr << "vl::cluster::Server::_addClient : "
				<< "Trying to add an already added client." << std::endl;
			return;
		}
	}

	_clients.push_back( std::make_pair( endpoint, CS_REQ ) );
}

void
vl::cluster::Server::_sendEnvironment ( const std::vector< char >& msg )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->second == CS_REQ )
		{
	// 		std::cout << "Sending to client = " << *iter << std::endl;
			_socket.send_to( boost::asio::buffer(msg), iter->first );
			iter->second = CS_ENV;
		}
	}
}

void
vl::cluster::Server::_sendProject ( const std::vector< char >& msg )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->second == CS_ENV )
		{
	// 		std::cout << "Sending to client = " << *iter << std::endl;
			_socket.send_to( boost::asio::buffer(msg), iter->first );
			iter->second = CS_PROJ;
		}
	}
}
