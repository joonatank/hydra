/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluser/server.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#include "server.hpp"

#include "base/exceptions.hpp"
// Necessary for blocking functions
#include "base/sleep.hpp"

vl::cluster::Server::Server(uint16_t const port)
	: _socket(_io_service, boost::udp::endpoint(boost::udp::v4(), port))
	, _n_log_messages(0)
	, _frame(0)
	, _draw_error(false)
	, _fsm(new ServerFSM())
{
	std::cout << vl::TRACE << "vl::cluster::Server::Server : " << "Creating server to port "
		<< port << std::endl;

	// Using a large send buffer so that we don't need to pause when sending
	// large messages (like Resources). The buffer will be filled with all
	// the send calls made close to each other.
	// @todo make the buffer size configurable, 1Mbyte is reasonable, but there
	// might be a reason for increasing it at least till we have a more
	// permanent solution like keeping track of the buffer usage or using
	// TCP for large resources.
	// @todo sending the buffer size to all clients so that we don't need to
	// reset it both here and there.
	boost::asio::socket_base::send_buffer_size send_buf_size(1024*1024);
	_socket.set_option(send_buf_size);

	boost::asio::socket_base::receive_buffer_size rec_buf_size;
	_socket.get_option(rec_buf_size);

	std::cout << "Receive Buffer size = " << rec_buf_size.value() << "." 
		<< " Send buffer size = " << send_buf_size.value() << "." << std::endl;

	std::cout << "Message part size = " << MSG_PART_SIZE << "." << std::endl;

	// Start the FSM and also init it for now
	// init can be moved outside of the constructor if it's necessary
	_fsm->setServer(this);
	_fsm->start();
}


vl::cluster::Server::~Server()
{}

void
vl::cluster::Server::shutdown(void)
{
	Message msg(vl::cluster::MSG_SHUTDOWN, 0, vl::time());

	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		_sendMessage(*iter, msg);
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
	while( _socket.available() )
	{
		std::vector<char> recv_buf( _socket.available() );
		boost::udp::endpoint remote_endpoint;
		boost::system::error_code error;

		// TODO we should check that all the bytes are read
		_socket.receive_from( boost::asio::buffer(recv_buf),
			remote_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		MessagePart part(recv_buf);

		/// Create new clients for everyone who isn't already present
		ClientInfo *cl_ptr = _find_client_ptr(remote_endpoint);
		if( !cl_ptr )
			cl_ptr = &_add_client(remote_endpoint);

		if(part.parts == 1)
		{
			Message msg_part(part);
			_handle_message(msg_part, *cl_ptr);
		}
		// @todo replace with a real solution
		// for now clients should not send that large messages anyways
		// similar solution that clients have (or the exact same one using inheritance)
		// use RefPtrs and a partial message stack
		else
		{
			std::string msg("Server does not support partial messages.");
			BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg)); 
		}
	}
}

/// @todo this shouldn't really exists...
/// All messages should be sent from the state machine especially those that
/// are part of the Rendering loop
void
vl::cluster::Server::update(uint32_t frame, vl::time const &timestamp)
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

	if(_report_timer.elapsed() > vl::time(10))
	{
		_server_report.finish();
		_report_timer.reset();
	}
}

// TODO
// The mainloop is at best problematic for the rendering loop
// because we should wait for the ACKs from rendering threads between
// update, draw and swap and immediately send the next message when all clients
// have responded.
/// @todo change this to use Barriers, selectable Barriers
/// remove the use of ACKs for state changes
/// Barrier has enter and leave methods. When barrier is entered or left
/// it sends a confirmation to the master.
/// Barriers can be used for FRAME_START (or UPDATE), DRAW and SWAP
/// At the moment we probably only need the first one to keep all the
/// frames in sync.
void
vl::cluster::Server::start_draw(uint32_t frame, vl::time const &timestamp)
{
	_frame = frame;
	process_event(vl::cluster::update(frame, timestamp));

	process_event(vl::cluster::render(frame, timestamp));
}

void
vl::cluster::Server::finish_draw(uint32_t frame, vl::time const &timestamp)
{
	assert(_frame == frame);

	process_event(vl::cluster::swap(frame, timestamp));

	process_event(vl::cluster::swap_done(frame, timestamp));
}

void
vl::cluster::Server::sendMessage(Message const &msg)
{
	ClientInfo *client = 0;
	switch( msg.getType() )
	{
		
		case MSG_SG_CREATE:
			sendCreate(msg);
			break;

		case MSG_SG_UPDATE :
			// this will probably be called for INIT which we should
			// now move to a separate MSG_TYPE
			assert( false && "MSG_SG_UPDATE message type to be sent." );
			break;

		case MSG_SG_INIT :
		case MSG_ENVIRONMENT:
			// Find which client requested environment	
		case MSG_PROJECT:
			// Find which client requested project
		case MSG_RESOURCE:
			{
				// Find which client requested resource
				for(size_t i = 0; i < _requested_msgs.size(); ++i)
				{
					if(_requested_msgs.at(i).second == msg.getType())
					{
						client = &_requested_msgs.at(i).first;
						_requested_msgs.erase(_requested_msgs.begin()+i);
						break;
					}
				}
				assert(client);
				_sendMessage(*client, msg);
			}
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
vl::cluster::Server::sendUpdate( vl::cluster::Message const &msg )
{
	_msg_update = msg;
}

void
vl::cluster::Server::sendCreate(Message const &msg)
{
	assert(msg.getType() == MSG_SG_CREATE);
	assert(msg.getFrame() >= _frame);

	_msg_creates.push_back(std::make_pair(msg.getFrame(), msg));
}

vl::cluster::Message
vl::cluster::Server::popMessage( void )
{
	Message tmp = _messages.front();
	_messages.pop_front();
	return tmp;
}

void
vl::cluster::Server::block_till_initialised(vl::time const &limit)
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

/// ---------------------------- FSM public ------------------------------------
void
vl::cluster::Server::_do_init(init const &evt)
{
}

void
vl::cluster::Server::_do_update(vl::cluster::update const &evt)
{
	vl::chrono tim;

	if(!has_rendering_clients())
	{ return; }

	// Create list of renderers for this loop
	_renderers.clear();
	for(ClientList::iterator iter = _clients.begin();
		iter != _clients.end(); ++iter)
	{
		if( iter->state.wants_render && iter->state.has_init )
		{ _renderers.push_back( &(*iter) ); }
	}

	// Send frame start
	// This is the first message and before this is answered
	// Updates should not be sent
	// this is sent to all
	// why is it sent to all? I'll change it for now.
	for(ClientRefList::iterator iter = _renderers.begin(); 
		iter != _renderers.end(); ++iter )
	{
		(*iter)->state.frame = evt.frame;
		(*iter)->state.set_rendering_state(CS_START);
		vl::cluster::Message msg(vl::cluster::MSG_FRAME_START, evt.frame, evt.timestamp);
		_sendMessage(**iter, msg);
	}

	tim.reset();
	if(!_block_till_state(CS_UPDATE_READY, _renderers, evt.timelimit))
	{
		std::clog << "vl::cluster::Server::start_draw : failed UPDATE_READY" << std::endl;
		// @todo switch to bit cleaner system
		process_event(vl::cluster::timer_expired());
	}

	// Send update message to all
	for( ClientRefList::iterator iter = _renderers.begin();
		iter != _renderers.end(); ++iter )
	{ _sendUpdate(**iter); }

	// @todo this should be somewhere else
	// Send the output messages
	for( ClientRefList::iterator iter = _renderers.begin();
		iter != _renderers.end(); ++iter )
	{ _sendOuput(**iter); }
	
	/// Wait till ready for DRAW

	/// @todo this fails because the slaves are really slow to init
	/// we need to fallback and not render on slaves before they have
	/// been inited
	/// Clear the rendering states if fails
	if(!_block_till_state(CS_DRAW_READY, _renderers, evt.timelimit))
	{
		std::clog << "vl::cluster::Server::start_draw : failed DRAW_READY" << std::endl;
		// @todo switch to bit cleaner system
		process_event(vl::cluster::timer_expired());
	}

	_server_report["wait update time"].push(tim.elapsed());

}

void
vl::cluster::Server::_do_render(render const &evt)
{
	vl::chrono tim;

	for( ClientRefList::iterator iter = _renderers.begin();
		iter != _renderers.end(); ++iter )
	{
		vl::cluster::Message msg( vl::cluster::MSG_DRAW, evt.frame, evt.timestamp);
		_sendMessage(**iter, msg);
	}
	// Check that all clients started drawing
	// @todo on Windows if we use the time limit the block call will fail
	// Some real performance problems with it.
	if(!_block_till_state(CS_DRAW, _renderers, evt.timelimit))
	{
		std::clog << "vl::cluster::Server::_do_render : failed DRAW" << std::endl;
		// @todo switch to bit cleaner system
		process_event(vl::cluster::timer_expired());
	}

	_server_report["wait draw time"].push(tim.elapsed());
}

void
vl::cluster::Server::_do_swap(swap const &evt)
{
	vl::chrono t;
	if(!_block_till_state(CS_DRAW_DONE, _renderers, evt.timelimit))
	{
		std::clog << "vl::cluster::Server::_do_render : failed DRAW_DONE" << std::endl;
		process_event(vl::cluster::timer_expired());
	}

	_server_report["wait draw done time"].push(t.elapsed());
}

void
vl::cluster::Server::_do_swap_done(swap_done const &evt)
{
	// we need to explicitly call _do_rest because it clears the Rendering Context
	// for now we like to keep _do_swap_done also
	_do_rest(vl::none());
}

void
vl::cluster::Server::_do_rest(vl::none const &evt)
{
	// Clear client rendering state
	for(ClientList::iterator iter = _clients.begin(); 
		iter != _clients.end(); ++iter)
	{
		iter->state.clear_rendering_state();
	}

	// Clear renderer list
	_renderers.clear();
}

void
vl::cluster::Server::_report_error(timer_expired const &evt)
{
	std::clog << "Timer has expired in the Remote Rendering loop." << std::endl;

	// block a while so we can clear the message cue
	// also block so we can catch errors more easily as the block is quite noticable
	for(size_t i = 0; i < 10; ++i)
	{
		poll();
		vl::sleep(vl::time(1, 0));
	}

	// just clear the error
	process_event(vl::cluster::clear_error());
	_draw_error = true;
}

bool
vl::cluster::Server::has_clients(void) const
{
	return !_clients.empty();
}

bool
vl::cluster::Server::has_rendering_clients(void) const
{
	// Returning true because we have messed up client initialisation
	//return true;

	for(ClientList::const_iterator iter = _clients.begin(); 
		iter != _clients.end(); ++iter)
	{
		if(iter->state.wants_render)
		{ return true; }
	}

	return false;
}


/// ----------------------------- Private --------------------------------------
void 
vl::cluster::Server::_handle_message(vl::cluster::Message &msg, ClientInfo &client)
{
	switch( msg.getType() )
	{
		case vl::cluster::MSG_REG_UPDATES :
		{
			// Don't send the environment more than once. 
			// @todo Using ACKs and resend would be even better.
			if(client.environment_sent_time.elapsed() > vl::time(1, 0))
			{
				assert(!_request_message.empty());
				_requested_msgs.push_back(std::make_pair(client, MSG_ENVIRONMENT));
				_request_message(MSG_ENVIRONMENT);
			}
		}
		break;

		case vl::cluster::MSG_REG_RENDERING :
			client.state.wants_render = true;
		break;

		case vl::cluster::MSG_ACK :
		{
			vl::cluster::MSG_TYPES type;
			msg.read(type);
			_handle_ack(client, type);
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
			client.state.wants_output = true;
		break;

		case vl::cluster::MSG_SG_UPDATE_READY :
		{
			if(client.state.is_rendering())
			{
				if(msg.getFrame() != _frame)
				{
					std::clog << "Server : MSG_SG_UPDATE_READY Client has incorrect frame number."
						<< " Master frame = " << _frame << " : client frame = " << msg.getFrame()
						<< std::endl;
				}
				else
				{
					// change the state
					client.state.set_rendering_state(CS_UPDATE_READY);
				}
			}
			else
			{
				std::clog << "Server : MSG_SG_UPDATE_READY received even though not rendering." << std::endl;
			}
		}
		break;

		case vl::cluster::MSG_SG_UPDATE_DONE :
		{
			assert( false && "MSG_SG_UPDATE_DONE Not in use");
		}
		break;

		/// Rendering loop
		/// @todo MSG_DRAW_READY should be renamed because it's sent as a reply
		/// for MSG_UPDATE and can be sent by slaves not receiving draw messages
		case vl::cluster::MSG_DRAW_READY :
		{
			/// Change state for those that are rendering, ignore otherwise
			if(client.state.is_rendering())
			{
				if(msg.getFrame() != _frame)
				{
					std::clog << "Server : MSG_DRAW_READY Client has incorrect frame number."
						<< " Master frame = " << _frame << " : client frame = " << msg.getFrame()
						<< std::endl;
				}
				else
				{
					client.state.set_rendering_state(CS_DRAW_READY);
				}
			}
			else
			{
				std::clog << "Server : MSG_DRAW_READY received even though not rendering." << std::endl;
			}
		}
		break;

		case vl::cluster::MSG_DRAWING :
		{
			/// Change state for those that are rendering, ignore otherwise
			if(client.state.is_rendering())
			{
				if(msg.getFrame() != _frame)
				{
					std::clog << "Server : MSG_DRAWING Client has incorrect frame number."
						<< " Master frame = " << _frame << " : client frame = " << msg.getFrame()
						<< std::endl;
				}
				// TODO all the clients in the rendering loop needs to be
				// on the same state at this point so that they swap the same time
				// change the state
				else
				{
					client.state.set_rendering_state(CS_DRAW);
				}
			}
			else
			{
				std::clog << "Server : MSG_DRAWING received even though not rendering." << std::endl;
			}
		}
		break;
		
		case vl::cluster::MSG_DRAW_DONE :
		{
			if(client.state.is_rendering())
			{
				if(msg.getFrame() != _frame)
				{
					std::clog << "Server : MSG_DRAW_DONE Client has incorrect frame number."
						<< " Master frame = " << _frame << " : client frame = " << msg.getFrame()
						<< std::endl;
				}
				// change the state
				else
				{
					client.state.set_rendering_state(CS_DRAW_DONE);
				}
			}
			else
			{
				std::clog << "Server : MSG_DRAW_DONE received even though not rendering." << std::endl;
			}
		}
		break;

		case vl::cluster::MSG_REG_RESOURCE :
		{
			std::clog << "vl::cluster::MSG_REG_RESOURCE message received." << std::endl;
			/// @todo add the real code
			RESOURCE_TYPE type;
			std::string name;
			msg.read(type);
			msg.read(name);
			assert(!_request_message.empty());
			_requested_msgs.push_back(std::make_pair(client, MSG_RESOURCE));
			_request_message(RequestedMessage(MSG_RESOURCE, name, type));
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
vl::cluster::Server::_sendCreate(ClientInfo &client)
{
	std::vector<Message> msgs;

	// Copy all the create messages that are newer than the client
	for( std::vector< std::pair<uint32_t, Message> >::const_reverse_iterator iter = _msg_creates.rbegin();
		iter != _msg_creates.rend(); ++iter )
	{
		if( client.state.create_frame >= int64_t(iter->first) )
		{ break; }
		msgs.push_back( iter->second );
		// Update the update frame so that the same message will not be sent again
		client.state.create_frame = iter->first;
	}

	for( std::vector<Message>::const_reverse_iterator iter = msgs.rbegin();
		iter != msgs.rend(); ++iter )
	{
		assert(iter->getType() == MSG_SG_CREATE);
		_sendMessage(client, *iter);
	}
}

void
vl::cluster::Server::_sendInit(ClientInfo &client)
{
	if(client.state.has_init)
	{
		std::clog << "vl::cluster::Server::_sendInit called on Client with already an init." << std::endl;
	}

	assert(!_request_message.empty());
	_requested_msgs.push_back(std::make_pair(client, MSG_SG_INIT));
	_request_message(MSG_SG_INIT);
}


void
vl::cluster::Server::_sendUpdate(ClientInfo &client)
{
	if( !client.state.has_init )
	{
		std::clog << "vl::cluster::Server::_sendUpdate called on Client without init." << std::endl;
		return;
	}

	// @todo should we send zero data messages?
	// the client can not handle not receiving updates but only receiving a draw message
	// at the moment, so should there always be an update message and
	// then the draw message or is the update message optional?

	// @todo create these here when needed so no need to reset
	// time and frame number
	// also minimises the overhead from useless creations
	_sendMessage(client, _msg_update);
}

void
vl::cluster::Server::_sendOuput(ClientInfo &client)
{
	if( !client.state.wants_output )
	{ return; }

	if( !_new_log_messages.empty() )
	{
		vl::cluster::Message msg(vl::cluster::MSG_PRINT, 0, vl::time());
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
		_sendMessage(client, msg);
	}
}

void 
vl::cluster::Server::_sendMessage(ClientInfo &client, vl::cluster::Message const &msg)
{
	/// @todo remove the copying that is needed both createParts and dump
	std::vector<char> buf;
	std::vector<MessagePart> parts = msg.createParts();
	
	/// Modify state
	if(msg.getType() == MSG_ENVIRONMENT)
	{
		client.environment_sent_time.reset();
	}

	for(size_t i = 0; i < parts.size(); ++i)
	{
		parts.at(i).dump(buf);
		_socket.send_to(boost::asio::buffer(buf), client.address);
		/// @todo we should add them to a sent stack, and verify the sending with ack
	}
}

void
vl::cluster::Server::_handle_ack(ClientInfo &client, vl::cluster::MSG_TYPES ack_to )
{
	// Hack to bypass other ACK messages than Shutdown
	// if it has been started
	if( client.state.shutdown && ack_to != MSG_SHUTDOWN )
	{
		return;
	}

	switch( ack_to )
	{
		case vl::cluster::MSG_ENVIRONMENT :
		{
			// Resetting environment NOT allowed
			// TODO this should have environment sent and environment received field
			if( !client.state.environment )
			{
				// Change the state of the client
				client.state.environment = true;
				// Send the Project message
				// TODO this should be moved to use a separate REQ_PROJECT message
				//assert(_data_cb);
				assert(!_request_message.empty());
				_requested_msgs.push_back(std::make_pair(client, MSG_PROJECT));
				_request_message(MSG_PROJECT);
			}
		}
		break;

		case vl::cluster::MSG_PROJECT :
		{
			// Reseting project NOT allowed
			if( !client.state.project )
			{
				// change the state
				client.state.project = true;
				_sendInit(client);
			}
		}
		break;

		case vl::cluster::MSG_SG_CREATE :
			// TODO this ack should have the frame number in it
		break;

		// The rendering loop Messages don't send any new messages
		// because they are tied to the master rendering loop
		case vl::cluster::MSG_FRAME_START :
			// Move to rendering loop
			// Not yet implemented
			//client.state.rendering = true;
			// @todo this does not work for some reason
			//assert( iter->state.rendering_state == CS_CLEAR );

		break;

		case vl::cluster::MSG_SG_INIT :
			{
				client.state.has_init = true;
			}

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
		}
		break;

		case vl::cluster::MSG_SHUTDOWN :
		{
			client.state.shutdown = true;
		}
		break;

		case vl::cluster::MSG_RESOURCE :
		case vl::cluster::MSG_PRINT :
			break;

		default:
		{
			// TODO change to out after confirmed that this doesn't happen at every frame
			std::clog << "Unhandled ACK message : type = "
				<< vl::cluster::getTypeAsString(ack_to) << std::endl;
		}
	}	// switch
}

bool
vl::cluster::Server::_block_till_state(CLIENT_STATE cs,  ClientRefList clients, vl::time const &limit)
{
	vl::chrono t;
	bool ready = false;
	while( !ready )
	{
		poll();

		ready = true;
		ClientRefList::const_iterator iter;
		for( iter = clients.begin(); iter != clients.end(); ++iter )
		{
			/// Break the block if one of the clients has failed its rendering loop
			if(!(*iter)->state.is_rendering())
			{ return false; }
			
			if( !(*iter)->state.has_rendering_state(cs) )
			{
				ready = false;
				break;
			}
		}

		if( limit != vl::time() && t.elapsed() > limit )
		{
			return false;
		}

		// TODO should wait only for a while and then resent the last message

		// Needs to sleep in Linux at least. Busy wait will cause a huge lag.
		vl::msleep(0);
	}

	return true;
}

bool
vl::cluster::Server::_rendering( void )
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if(iter->state.is_rendering())
		{ return true; }
	}
	return false;
}
