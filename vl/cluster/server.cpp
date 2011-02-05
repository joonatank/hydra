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
vl::cluster::Server::receiveMessages( void )
{
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
				std::cout << "vl::cluster::Server::mainloop : MSG_REG_UPDATES message received."
					<< std::endl;
				_addClient( remote_endpoint );
				if( !_env_msg.empty() )
				{
					_sendEnvironment(_env_msg);
				}
				// TODO this should save the client as one that requested
				// environment if no environment is set so that one will be sent
				// as soon as one is available.
				delete msg;
			}
			break;

			case vl::cluster::MSG_ACK :
			{
				vl::cluster::MSG_TYPES type;
				msg->read(type);
				_handleAck(remote_endpoint, type);
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

// TODO
// The mainloop is at best problematic for the rendering loop
// because we should wait for the ACKs from rendering threads between
// update, draw and swap and immediately send the next message when all clients
// have responded.
void
vl::cluster::Server::render( void )
{
// 	std::cout << "vl::cluster::Server::render" << std::endl;
	_waitUpdate();
	// Send update message to all
// 	std::cout << "vl::cluster::Server::render : sending update" << std::endl;
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendUpdate(iter->first); }
// 	std::cout << "vl::cluster::Server::render : wait draw" << std::endl;
	_waitDraw();
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendDraw(iter->first); }
// 	std::cout << "vl::cluster::Server::render : wait swap" << std::endl;
	_waitSwap();
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendSwap(iter->first); }
// 	std::cout << "vl::cluster::Server::render : done" << std::endl;
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
}

void
vl::cluster::Server::sendUpdate( vl::cluster::Message const &msg )
{
	_msg_update.clear();
	msg.dump(_msg_update);
}

void
vl::cluster::Server::sendInit( vl::cluster::Message const &msg )
{
	_msg_init.clear();
	msg.dump(_msg_init);
}

bool
vl::cluster::Server::needsInit( void ) const
{
	return true;
// 	ClientList::const_iterator iter;
// 	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
// 	{
// 		if( iter->second == CS_PROJ )
// 		{
// 			return true;
// 		}
// 	}
//
// 	return false;
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

	_clients.push_back( std::make_pair( endpoint, CS_UNDEFINED ) );
}

void
vl::cluster::Server::_sendEnvironment ( const std::vector< char >& msg )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->second == CS_UNDEFINED )
		{
	// 		std::cout << "Sending to client = " << *iter << std::endl;
			_socket.send_to( boost::asio::buffer(msg), iter->first );
			iter->second = CS_REQ;
		}
	}
}

void
vl::cluster::Server::_sendEnvironment( const boost::udp::endpoint &endpoint )
{
	_socket.send_to( boost::asio::buffer(_env_msg), endpoint );
}

void
vl::cluster::Server::_sendProject( const boost::udp::endpoint &endpoint )
{
	_socket.send_to( boost::asio::buffer(_proj_msg), endpoint );
}

void
vl::cluster::Server::_sendInit( const boost::asio::ip::udp::endpoint& endpoint )
{
	_socket.send_to( boost::asio::buffer(_msg_init), endpoint );
}

void
vl::cluster::Server::_sendUpdate( const boost::asio::ip::udp::endpoint& endpoint )
{
	_socket.send_to( boost::asio::buffer(_msg_update), endpoint );
}

void
vl::cluster::Server::_sendDraw( const boost::asio::ip::udp::endpoint& endpoint )
{
	vl::cluster::Message msg( vl::cluster::MSG_DRAW );
	std::vector<char> buf;
	msg.dump(buf);
	_socket.send_to( boost::asio::buffer(buf), endpoint );
}


void
vl::cluster::Server::_sendSwap( const boost::asio::ip::udp::endpoint& endpoint )
{
	vl::cluster::Message msg( vl::cluster::MSG_SWAP );
	std::vector<char> buf;
	msg.dump(buf);
	_socket.send_to( boost::asio::buffer(buf), endpoint );
}



void
vl::cluster::Server::_handleAck( const boost::udp::endpoint &client, vl::cluster::MSG_TYPES ack_to )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->first == client )
		{
			switch( ack_to )
			{
				case vl::cluster::MSG_ENVIRONMENT :
				{
					std::cout << "vl::cluster::Server::_handleAck : MSG_ENVIRONMENT" << std::endl;
					assert( iter->second == CS_REQ );
					// Send the Project message
					_sendProject( iter->first );
					// Change the state of the client
					iter->second = CS_ENV;
				}
				break;

				case vl::cluster::MSG_PROJECT :
				{
					std::cout << "vl::cluster::Server::_handleAck : MSG_PROJECT" << std::endl;
					assert( iter->second == CS_ENV );
					_sendInit(iter->first);
					// change the state
					iter->second = CS_PROJ;
				}
				break;

				case vl::cluster::MSG_INITIAL_STATE :
				{
					std::cout << "vl::cluster::Server::_handleAck : MSG_INITIAL_STATE" << std::endl;
					assert( iter->second == CS_PROJ );
					// change the state
					iter->second = CS_INIT;
				}
				break;

				// The rendering loop Messages don't send any new messages
				// because they are tied to the master rendering loop
				case vl::cluster::MSG_UPDATE :
				{
// 					std::cout << "vl::cluster::Server::_handleAck : MSG_UPDATE" << std::endl;
					assert( iter->second == CS_INIT || iter->second == CS_SWAP );
					// TODO the rendering loop should be driven from the
					// application loop or at least be configurable from there
					// so the server either needs configuration parameters
					// for example the sleep time or states that are changed
					// from the application loop
// 					_sendDraw(iter->first);
					// change the state
					iter->second = CS_UPDATE;
				}
				break;

				case vl::cluster::MSG_DRAW :
				{
// 					std::cout << "vl::cluster::Server::_handleAck : MSG_DRAW" << std::endl;
					assert( iter->second == CS_UPDATE );
					// TODO all the clients in the rendering loop needs to be
					// on the same state at this point so that they swap the same
					// time
// 					_sendSwap(iter->first);
					// change the state
					iter->second = CS_DRAW;
				}
				break;

				case vl::cluster::MSG_SWAP :
				{
// 					std::cout << "vl::cluster::Server::_handleAck : MSG_SWAP" << std::endl;
					assert( iter->second == CS_DRAW );
// 					_sendUpdate(iter->first);
					// change the state
					iter->second = CS_SWAP;
				}
				break;

				default:
					assert(false);
			};
		}
	}
}

void
vl::cluster::Server::_waitUpdate( void )
{
	bool ready_for_update = false;
	while( !ready_for_update )
	{
// 		std::cout << "vl::cluster::Server::_waitUpdate" << std::endl;
		receiveMessages();

		ready_for_update = true;
		ClientList::iterator iter;
		for( iter = _clients.begin(); iter != _clients.end(); ++iter )
		{
			if( iter->second == CS_SWAP || iter->second == CS_INIT )
			{}
			else
			{
				ready_for_update = false;
				break;
			}
		}
		// TODO should wait only for a while and then resent the last message
	}
}

void
vl::cluster::Server::_waitDraw( void )
{
	bool ready_for_draw = false;
	while( !ready_for_draw )
	{
		receiveMessages();

		ready_for_draw = true;
		ClientList::iterator iter;
		for( iter = _clients.begin(); iter != _clients.end(); ++iter )
		{
			if( iter->second != CS_UPDATE )
			{ ready_for_draw = false; }
		}
		// TODO should wait only for a while and then resent the last message
	}
}

void
vl::cluster::Server::_waitSwap( void )
{
	bool ready_for_swap = false;
	while( !ready_for_swap )
	{
		receiveMessages();

		ready_for_swap = true;
		ClientList::iterator iter;
		for( iter = _clients.begin(); iter != _clients.end(); ++iter )
		{
			if( iter->second != CS_DRAW )
			{ ready_for_swap = false; }
		}
		// TODO should wait only for a while and then resent the last message
	}
}
