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
	: _socket(_io_service, boost::udp::endpoint(boost::udp::v4(), port))
	, _frame(1)
	, _update_frame(1)
	, _n_log_messages(0)
{
	std::cout << vl::TRACE << "vl::cluster::Server::Server : " << "Creating server to port "
		<< port << std::endl;
	boost::asio::socket_base::receive_buffer_size buf_size;
	_socket.get_option(buf_size);
	std::cout << vl::TRACE << "Receive Buffer size = " << buf_size.value() << "." << std::endl;
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
		iter->state.shutdown = true;
	}

	// Block till successfull
	// TODO should resent the Shutdown message if no response is received
	// TODO should also have a timeout system which exits with an error
	// TODO also best to create a blockTillState function for blocking
	// while state is not correct
	// The new state structure needs some work for blocking
	//_block_till_state(CS_UNDEFINED);
}

void
vl::cluster::Server::poll(void)
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

		Message msg(recv_buf);
		switch( msg.getType() )
		{
			case vl::cluster::MSG_REG_UPDATES :
			{
				// Do not add client twice
				if(!_has_client(remote_endpoint))
				{
					ClientInfo &client = _add_client( remote_endpoint );
					_sendEnvironment(client);
				}
				// TODO this should save the client as one that requested
				// environment if no environment is set so that one will be sent
				// as soon as one is available.
			}
			break;
			
			case vl::cluster::MSG_REG_RENDERING :
			{
				ClientInfo &client = _find_client(remote_endpoint);
				assert( !client.state.wants_render );
				client.state.wants_render = true;
			}
			break;

			case vl::cluster::MSG_ACK :
			{
				vl::cluster::MSG_TYPES type;
				msg.read(type);
				_handleAck(remote_endpoint, type);
			}
			break;

			case vl::cluster::MSG_INPUT :
			case vl::cluster::MSG_COMMAND :
			{
				// TODO there should be a maximum amount of messages stored
				_messages.push_back(msg);
			}
			break;

			case vl::cluster::MSG_REG_OUTPUT :
			{
				// TODO remove separate data structure, add a token in the ClientInfo
				_find_client(remote_endpoint).state.wants_output = true;
			}
			break;

			case vl::cluster::MSG_SG_UPDATE_READY :
			{
				ClientInfo &client = _find_client(remote_endpoint);
				assert( client.state.rendering );
				// change the state
				client.state.rendering_state = CS_UPDATE_READY;
			}
			break;

			case vl::cluster::MSG_SG_UPDATE_DONE :
			{
				assert( false && "MSG_SG_UPDATE_DONE Not in use");
				ClientInfo &client = _find_client(remote_endpoint);
				assert( client.state.rendering );
				assert( client.state.rendering_state == CS_UPDATE );
				// change the state
				client.state.rendering_state = CS_UPDATE_DONE;
			}
			break;

			/// Rendering loop
			case vl::cluster::MSG_DRAW_READY :
			{
				ClientInfo &client = _find_client(remote_endpoint);
				assert( client.state.rendering );
				//assert( client.state == CS_UPDATE );
				// change the state
				client.state.rendering_state = CS_DRAW_READY;
			}
			break;

			case vl::cluster::MSG_DRAW_DONE :
			{
				ClientInfo &client = _find_client(remote_endpoint);
				assert( client.state.rendering );
				assert( client.state.rendering_state == CS_DRAW );
				// change the state
				client.state.rendering_state = CS_DRAW_DONE;
				client.state.rendering = false;
			}
			break;

			default :
			{
				std::cout << vl::CRITICAL << "vl::cluster::Server::mainloop : "
					<< "Unhandeled message type." << std::endl;
			}
			break;
		}
	}
}

/// TODO should contain only the frame update message
void 
vl::cluster::Server::update(vl::Stats &stats)
{
	// Create is not part of the Rendering loop and should be separated
	// from it.
	// TODO this is really bad system
	// The creates should be sent from sendCreate directly with sequence
	// and stored for new clients
	// for whome they should be sent after project
	// And similar system for init messages also so we can remove the stupid frame checking
	ClientList::iterator iter;
	// Create does not need to wait for, they should how ever be
	// timestamped with frames so the correct order can be established
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->state.environment && iter->state.project )
		{ _sendCreate(*iter); }
	}

	++_update_frame;
}

// TODO
// The mainloop is at best problematic for the rendering loop
// because we should wait for the ACKs from rendering threads between
// update, draw and swap and immediately send the next message when all clients
// have responded.
bool
vl::cluster::Server::start_draw(vl::Stats &stats)
{
	// Only render if we have rendering threads
	if( _clients.empty() )
	{ return false; }
	
	Ogre::Timer timer;

	// TODO We can skip here all clients that are not yet rendering
	ClientRefList renderers;
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->state.wants_render )
		renderers.push_back( &(*iter) );
	}

	// TODO even if client is not rendering it should get update messages
	if( renderers.empty() )
	{ return false; }

	// Send frame start
	// This is the first message and before this is answered
	// Updates should not be sent
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		vl::cluster::Message msg( vl::cluster::MSG_FRAME_START );
		msg.write(_frame);
		_sendMessage(iter->address, msg);
	}

	timer.reset();
	_block_till_state(CS_UPDATE_READY);
	// Send update message to all
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendUpdate(*iter); }

	// Not in use because DRAW_READY is sent so soon after this one
//	_block_till_state(CS_UPDATE);
	// Not in use
//	_block_till_state(CS_UPDATE_DONE);
	
	// Send the output messages
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ _sendOuput(*iter); }
	_block_till_state(CS_DRAW_READY);
	stats.logWaitUpdateTime( (double(timer.getMicroseconds()))/1e3 );

	timer.reset();
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{ 
		vl::cluster::Message msg( vl::cluster::MSG_DRAW );
		_sendMessage(iter->address, msg);
		iter->state.frame = _frame;
	}
	// Check that all clients started drawing
	_block_till_state(CS_DRAW);
	stats.logWaitDrawTime( (double(timer.getMicroseconds()))/1e3 );

	return true;
}

void 
vl::cluster::Server::finish_draw(vl::Stats &stats, double timelimit)
{
	// TODO replace assert with if
	if(_rendering())
	{
		Ogre::Timer timer;
		_block_till_state(CS_DRAW_DONE);
		stats.logWaitDrawDoneTime( (double(timer.getMicroseconds()))/1e3 );
		++_frame;
	}
}

void 
vl::cluster::Server::sendMessage(Message const &msg)
{
	switch( msg.getType() )
	{	
		case MSG_ENVIRONMENT:
			sendEnvironment(msg);
			break;
		case MSG_PROJECT:
			sendProject(msg);
			break;

		case MSG_SG_CREATE:
			sendCreate(msg);
			break;

		case MSG_SG_UPDATE:
			sendUpdate(msg);
			break;

		// Uses the LogReceiver interface
		case MSG_PRINT:
		// Not yet supported
		case MSG_DRAW:
		default :
			assert( false && "Not allowed message type to be sent." );
	}
}

void
vl::cluster::Server::sendEnvironment( const vl::cluster::Message &msg )
{
	std::cout << vl::TRACE << "vl::cluster::Server::sendEnvironment" << std::endl;

	assert(msg.getType() == MSG_ENVIRONMENT );

	// Resetting environment NOT supported
	assert( _env_msg.empty() );
	msg.dump(_env_msg);
	
	ClientList::iterator iter;
	for(iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		// We don't allow sending the environment twice
		assert( !iter->state.environment );
		_sendEnvironment(*iter);
	}
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
	_msg_creates.push_back( std::make_pair(_update_frame, msg) );
}

void
vl::cluster::Server::sendInit( vl::cluster::Message const &msg )
{
	_msg_init.clear();
	msg.dump(_msg_init);
}

vl::cluster::Message
vl::cluster::Server::popMessage( void )
{
	Message tmp = _messages.front();
	_messages.pop_front();
	return tmp;
}

void 
vl::cluster::Server::block_till_initialised(double timelimit)
{
	// This doesn't work at the moment
	//_block_till_state(CS_PROJ, timelimit);
}

bool 
vl::cluster::Server::logEnabled(void) const
{
	ClientList::const_iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->state.wants_output )
		{ return true; }
	}

	// No client is wanting output
	return false;
}

void 
vl::cluster::Server::logMessage(vl::LogMessage const &msg)
{
	if( !logEnabled() )
	{
		std::cout << vl::CRITICAL << "Trying to Log a message even though logging not enabled." << std::endl;
		return;
	}

	_new_log_messages.push_back(msg);

	++_n_log_messages;
}

uint32_t 
vl::cluster::Server::nLoggedMessages(void) const
{
	return _n_log_messages;
}

/// ----------------------------- Private --------------------------------------
vl::cluster::Server::ClientInfo &
vl::cluster::Server::_add_client( boost::udp::endpoint const &endpoint )
{
	if( _has_client(endpoint) )
	{ assert(false && "Trying to add client twice"); }
	
	_clients.push_back( ClientInfo( endpoint ) );

	return _clients.back();
}

bool 
vl::cluster::Server::_has_client(boost::udp::endpoint const &address) const
{
	return _find_client_ptr(address);
}

vl::cluster::Server::ClientInfo &
vl::cluster::Server::_find_client(boost::udp::endpoint const &address)
{
	// TODO should throw on NULL
	return *_find_client_ptr(address);
}

vl::cluster::Server::ClientInfo *
vl::cluster::Server::_find_client_ptr(boost::udp::endpoint const &address)
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->address == address )
		{ return &(*iter); }
	}

	return 0;
}

vl::cluster::Server::ClientInfo const *
vl::cluster::Server::_find_client_ptr(boost::udp::endpoint const &address) const
{
	ClientList::const_iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->address == address )
		{ return &(*iter); }
	}

	return 0;
}


void
vl::cluster::Server::_sendEnvironment(ClientInfo &client)
{
	if( !_env_msg.empty() )
	{
		_socket.send_to( boost::asio::buffer(_env_msg), client.address );
		//client.state = CS_REQ;
	}
}

void
vl::cluster::Server::_sendCreate(ClientInfo &client)
{
	std::vector<Message> msgs;

	// Copy all the create messages that are newer than the client
	for( std::vector< std::pair<uint32_t, Message> >::const_reverse_iterator iter = _msg_creates.rbegin();
		iter != _msg_creates.rend(); ++iter )
	{
		if( client.state.update_frame >= iter->first )
		{ break; }
		msgs.push_back( iter->second );
		// Update the update frame so that the same message will not be sent again
		client.state.update_frame = iter->first;
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
vl::cluster::Server::_sendUpdate(ClientInfo &client)
{
	// @todo should we send zero data messages?
	// the client can not handle not receiving updates but only receiving a draw message
	// at the moment, so should there always be an update message and
	// then the draw message or is the update message optional?
	if( !client.state.has_init )
	{ 
		_socket.send_to( boost::asio::buffer(_msg_update), client.address );
		client.state.has_init = true;
	}
	else
	{ _socket.send_to( boost::asio::buffer(_msg_init), client.address ); }
}

void
vl::cluster::Server::_sendOuput(ClientInfo &client)
{
	if( !client.state.wants_output )
	{ return; }

	if( !_new_log_messages.empty() )
	{
		vl::cluster::Message msg( vl::cluster::MSG_PRINT );
		msg.write(_new_log_messages.size());
		std::vector<vl::LogMessage>::iterator iter;
		for( iter = _new_log_messages.begin(); iter != _new_log_messages.end(); ++iter )
		{
			msg.write(iter->type);
			msg.write(iter->time);
			msg.write(iter->message);
			msg.write(iter->level);
		}

		_new_log_messages.clear();
		_sendMessage(client.address, msg);
	}
}

void 
vl::cluster::Server::_sendMessage(boost::udp::endpoint const &endpoint, vl::cluster::Message const &msg)
{
	std::vector<char> buf;
	msg.dump(buf);
	_socket.send_to( boost::asio::buffer(buf), endpoint );
}

void
vl::cluster::Server::_handleAck( const boost::udp::endpoint &client, vl::cluster::MSG_TYPES ack_to )
{
	// TODO this is pretty darn idiotic, it has one extra loop
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->address == client )
		{
			// Hack to bypass other ACK messages than Shutdown
			// if it has been started
			if( iter->state.shutdown && ack_to != MSG_SHUTDOWN )
			{ continue; }

			switch( ack_to )
			{
				case vl::cluster::MSG_ENVIRONMENT :
				{
					std::cout << vl::TRACE << "vl::cluster::Server::_handleAck : MSG_ENVIRONMENT" << std::endl;
					// Resetting environment NOT allowed
					assert( !iter->state.environment );
					// Change the state of the client
					iter->state.environment = true;
					// Send the Project message
					_socket.send_to( boost::asio::buffer(_proj_msg), iter->address);
				}
				break;

				case vl::cluster::MSG_PROJECT :
				{
					std::cout << vl::TRACE << "vl::cluster::Server::_handleAck : MSG_PROJECT" << std::endl;
					// Reseting project NOT allowed
					assert( !iter->state.project );
					// change the state
					iter->state.project = true;
				}
				break;

				case vl::cluster::MSG_SG_CREATE :
					// TODO this ack should have the frame number in it
				break;

				case vl::cluster::MSG_FRAME_START :
					// Move to rendering loop
					// Not yet implemented
					iter->state.rendering = true;
				break;

				// The rendering loop Messages don't send any new messages
				// because they are tied to the master rendering loop
				case vl::cluster::MSG_SG_UPDATE :
				{
					// Completely useless as the CS_DRAW_READY is sent at the same time
				//	assert( iter->state == CS_UPDATE_READY );
					// TODO the rendering loop should be driven from the
					// application loop or at least be configurable from there
					// so the server either needs configuration parameters
					// for example the sleep time or states that are changed
					// from the application loop
					// change the state
				//	iter->state  = CS_UPDATE;
				}
				break;

				case vl::cluster::MSG_DRAW :
				{
					assert( iter->state.rendering && iter->state.rendering_state == CS_DRAW_READY );
					// TODO all the clients in the rendering loop needs to be
					// on the same state at this point so that they swap the same
					// time
					// change the state
					iter->state.rendering_state = CS_DRAW;
				}
				break;

				case vl::cluster::MSG_SHUTDOWN :
				{
					iter->state.shutdown = true;
				}
				break;

				case vl::cluster::MSG_PRINT :
					break;

				default:
				{
					// TODO change to out after confirmed that this doesn't happen at every frame
					std::clog << "Unhandled ACK message : type = " 
						<< vl::cluster::getTypeAsString(ack_to) << std::endl;
				}
			}	// switch
		}	// if
	}	// for
}

// TODO implement timelimit
bool 
vl::cluster::Server::_block_till_state(CLIENT_STATE cs, double timelimit)
{
	bool ready = false;
	while( !ready )
	{
		poll();

		ready = true;
		ClientList::iterator iter;
		for( iter = _clients.begin(); iter != _clients.end(); ++iter )
		{
			if( iter->state.rendering_state != cs )
			{
				ready = false;
				break;
			}
		}
		// TODO should wait only for a while and then resent the last message
	}

	return true;
}

bool 
vl::cluster::Server::_rendering( void )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( iter->state.rendering )
		{ return true; }
	}
	return false;
}
