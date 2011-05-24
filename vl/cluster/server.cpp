/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file server.cpp
 */

#include "server.hpp"

#include "base/exceptions.hpp"

#include "message.hpp"

#include <OGRE/OgreTimer.h>

#include "logger.hpp"

vl::cluster::Server::Server( uint16_t const port )
	: _socket(_io_service, boost::udp::endpoint(boost::udp::v4(), port)), _frame(1)
{
	std::cout << vl::TRACE << "vl::cluster::Server::Server : " << "Creating server to port "
		<< port << std::endl;
	boost::asio::socket_base::receive_buffer_size rec_buf_size;
	_socket.get_option(rec_buf_size);
	boost::asio::socket_base::send_buffer_size send_buf_size;
	_socket.set_option(send_buf_size);
	std::cout << vl::TRACE << "Receive Buffer size = " << rec_buf_size.value() << "." << std::endl;
}


vl::cluster::Server::~Server()
{}

void
vl::cluster::Server::shutdown( void )
{
	Message msg( vl::cluster::MSG_SHUTDOWN );
	std::vector<char> buf;
	msg.dump(buf);

	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		_socket.send_to( boost::asio::buffer(buf), iter->address );
		iter->state = CS_SHUTDOWN;
	}
}

void
vl::cluster::Server::receiveMessages( void )
{
	// TODO
	// We should check that multiple clients work correctly
	// At the moment we use single socket for all clients
	// Better to use multiple sockets, one for each client
	while( _socket.available() != 0 )
	{
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

			case vl::cluster::MSG_COMMAND :
			{
//				std::cout << "Server : vl::cluster::MSG_COMMAND received." << std::endl;
				// TODO there should be a maximum amount of messages stored
				// TODO works only on ASCII at the moment
				assert( msg->size() > sizeof(size_t) );
				size_t size;
				msg->read(size);
				std::vector<char> vec(size);
				msg->read( &vec[0], size );
				_commands.push_back( std::string(&vec[0]) );

				delete msg;
			}
			break;

			case vl::cluster::MSG_REG_OUTPUT :
				_output_receivers.push_back( ClientInfo(remote_endpoint) );
				break;

			default :
			{
				std::cout << vl::ERR << "vl::cluster::Server::mainloop : "
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
vl::cluster::Server::render( vl::Stats &stats )
{
	// Only render if we have rendering threads
	if( _clients.empty() )
	{ return; }

	Ogre::Timer timer;

	_waitCreate();
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendCreate(*iter); }

	timer.reset();
	_waitUpdate();
	// Send update message to all
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendUpdate(*iter); }
	stats.logWaitUpdateTime( (double(timer.getMicroseconds()))/1e3 );

	timer.reset();
	_waitDraw();
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ 
		_sendDraw(iter->address); 
		iter->frame = _frame;
	}
	stats.logWaitDrawTime( (double(timer.getMicroseconds()))/1e3 );

	/*
	timer.reset();
	_waitSwap();
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		_sendSwap(iter->address);
		iter->frame = _frame;
	}
	stats.logWaitSwapTime( (double(timer.getMicroseconds()))/1e3 );
	*/

	++_frame;
}

void
vl::cluster::Server::sendEnvironment( const vl::cluster::Message &msg )
{
	std::cout << vl::TRACE << "vl::cluster::Server::sendEnvironment" << std::endl;

	_env_msg.clear();
	msg.dump(_env_msg);
	_sendEnvironment(_env_msg);
}

void
vl::cluster::Server::sendProject( const vl::cluster::Message &msg )
{
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
vl::cluster::Server::sendCreate( Message const &msg )
{
	// @TODO this needs frame or timestamp information when dealing with multiple
	// slaves which can be in different states.
	assert( msg.getType() == MSG_SG_CREATE );
	_msg_creates.push_back( std::make_pair(_frame, msg) );
}

void
vl::cluster::Server::sendInit( vl::cluster::Message const &msg )
{
	_msg_init.clear();
	msg.dump(_msg_init);
}

void
vl::cluster::Server::sendPrintMessage( vl::cluster::Message const & msg )
{
	assert( msg.getType() == MSG_PRINT );
	assert( !_output_receivers.empty() );

	std::vector<char> buf;
	msg.dump(buf);
	ClientList::iterator iter;
	for( iter = _output_receivers.begin(); iter != _output_receivers.end(); ++iter )
	{
		_socket.send_to( boost::asio::buffer(buf), iter->address );
	}
}

bool
vl::cluster::Server::wantsPrintMessages( void )
{
	return !_output_receivers.empty();
}


bool
vl::cluster::Server::needsInit( void ) const
{
	return true;
// 	ClientList::const_iterator iter;
// 	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
// 	{
// 		if( iter->state == CS_PROJ )
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

std::string vl::cluster::Server::popCommand( void )
{
	std::string tmp = _commands.front();
	_commands.erase(_commands.begin());
	return tmp;
}


/// ----------------------------- Private --------------------------------------
void
vl::cluster::Server::_addClient( boost::udp::endpoint const &endpoint )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->address == endpoint )
		{ return; }
	}

	_clients.push_back( ClientInfo( endpoint ) );
}

void
vl::cluster::Server::_sendEnvironment ( const std::vector< char >& msg )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->state == CS_UNDEFINED )
		{
			_socket.send_to( boost::asio::buffer(msg), iter->address );
			iter->state = CS_REQ;
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
vl::cluster::Server::_sendCreate( ClientInfo const &client )
{
	std::vector<Message> msgs;

	// Copy all the create messages that are newer than the client
	for( std::vector< std::pair<uint32_t, Message> >::const_reverse_iterator iter = _msg_creates.rbegin();
		iter != _msg_creates.rend(); ++iter )
	{
		if( client.frame >= iter->first )
		{ break; }
		msgs.push_back( iter->second );
	}

	for( std::vector<Message>::const_reverse_iterator iter = msgs.rbegin();
		iter != msgs.rend(); ++iter )
	{
		std::vector<char> buf;
		iter->dump(buf);
		_socket.send_to( boost::asio::buffer(buf), client.address );
	}
}

void
vl::cluster::Server::_sendUpdate( ClientInfo const &client )
{
	// @todo should we send zero data messages?
	// the client can not handle not receiving updates but only receiving a draw message
	// at the moment, so should there always be an update message and
	// then the draw message or is the update message optional?
	if( client.frame > 0 )
	{ _socket.send_to( boost::asio::buffer(_msg_update), client.address ); }
	else
	{ _socket.send_to( boost::asio::buffer(_msg_init), client.address ); }
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
		if( iter->address == client )
		{
			// Hack to bypass other ACK messages than Shutdown
			// if it has been started
			if( iter->state == CS_SHUTDOWN && ack_to != MSG_SHUTDOWN )
			{ continue; }

			switch( ack_to )
			{
				case vl::cluster::MSG_ENVIRONMENT :
				{
					std::cout << vl::TRACE << "vl::cluster::Server::_handleAck : MSG_ENVIRONMENT" << std::endl;
					assert( iter->state == CS_REQ );
					// Send the Project message
					_sendProject( iter->address );
					// Change the state of the client
					iter->state = CS_ENV;
				}
				break;

				case vl::cluster::MSG_PROJECT :
				{
					std::cout << vl::TRACE << "vl::cluster::Server::_handleAck : MSG_PROJECT" << std::endl;
					assert( iter->state == CS_ENV );
					// change the state
					iter->state = CS_PROJ;
				}
				break;

				// @todo should be removed
				// changed to SG_CREATE and SG_UPDATE
				case vl::cluster::MSG_SG_CREATE :
				{
					// change the state
					iter->state = CS_CREATE;
				}
				break;

				// The rendering loop Messages don't send any new messages
				// because they are tied to the master rendering loop
				case vl::cluster::MSG_SG_UPDATE :
				{
					//assert( iter->state == CS_INIT || iter->state == CS_SWAP );
					// TODO the rendering loop should be driven from the
					// application loop or at least be configurable from there
					// so the server either needs configuration parameters
					// for example the sleep time or states that are changed
					// from the application loop
					// change the state
					iter->state  = CS_UPDATE;
				}
				break;

				case vl::cluster::MSG_DRAW :
				{
					assert( iter->state == CS_UPDATE );
					// TODO all the clients in the rendering loop needs to be
					// on the same state at this point so that they swap the same
					// time
					// change the state
					//iter->state = CS_DRAW;
					iter->state = CS_SWAP;
				}
				break;

				case vl::cluster::MSG_SWAP :
				{
					assert( iter->state == CS_DRAW );
					// change the state
					iter->state = CS_SWAP;
				}
				break;

				// TODO this ACK is never received by the server
				case vl::cluster::MSG_SHUTDOWN :
				{
					iter->state = CS_UNDEFINED;
				}
				break;

				default:
					assert(false);
			};
		}
	}
}

void
vl::cluster::Server::_waitCreate( void )
{
	bool ready_for_create = false;
	while( !ready_for_create )
	{
		receiveMessages();

		ready_for_create = true;
		ClientList::iterator iter;
		for( iter = _clients.begin(); iter != _clients.end(); ++iter )
		{
			// TODO this should use a ready message
			if( iter->state == CS_SWAP || iter->state == CS_PROJ )
			{}
			else
			{
				ready_for_create = false;
				break;
			}
		}
	}
}

void
vl::cluster::Server::_waitUpdate( void )
{
	bool ready_for_update = false;
	while( !ready_for_update )
	{
		receiveMessages();

		ready_for_update = true;
		ClientList::iterator iter;
		for( iter = _clients.begin(); iter != _clients.end(); ++iter )
		{
			// TODO this should use a ready message
			if( iter->state == CS_SWAP || iter->state == CS_PROJ
				|| iter->state == CS_CREATE )
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
			if( iter->state != CS_UPDATE )
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
			if( iter->state != CS_DRAW )
			{ ready_for_swap = false; }
		}
		// TODO should wait only for a while and then resent the last message
	}
}
