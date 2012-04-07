/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file cluser/server.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *
 */

#include "server.hpp"

#include "base/exceptions.hpp"
// Necessary for blocking functions
#include "base/sleep.hpp"

/// Server::Client
void
vl::cluster::Server::ClientFSM::_do_rest(void)
{
	assert(_server);
}

void
vl::cluster::Server::ClientFSM::_do_quit(vl::cluster::event::quit const &evt)
{
	assert(_server);
}

void
vl::cluster::Server::ClientFSM::_do_init(vl::cluster::event::init_done const &evt)
{
	assert(_server);
}

void
vl::cluster::Server::ClientFSM::_do_load(vl::cluster::event::load_done const &evt)
{
	assert(_server);
}

void
vl::cluster::Server::ClientFSM::_do_init_requested(vl::cluster::event::init_requested const &evt)
{
	assert(_server);
}
void
vl::cluster::Server::ClientFSM::_do_load_requested(vl::cluster::event::load_requested const &evt)
{
	assert(_server);

	// Send the Project message
	_server->_request_message(this, MSG_PROJECT);
}

void
vl::cluster::Server::ClientFSM::_do_graph_requested(vl::cluster::event::graph_requested const &evt)
{
	assert(_server);

	_server->_request_message(this, MSG_SG_INIT);
}

void
vl::cluster::Server::ClientFSM::_do_graph_load(vl::cluster::event::graph_loaded const &evt)
{
	assert(_server);

	_ready_for_rendering = true;
	_ready_for_updates = true;
}

void
vl::cluster::Server::ClientFSM::_do_start_frame(vl::cluster::event::start_frame const &evt)
{
	assert(_server);

	vl::cluster::Message msg(vl::cluster::MSG_FRAME_START, evt.frame, evt.timestamp);
	_server->_send_message(this, msg);
}

void
vl::cluster::Server::ClientFSM::_do_frame_done(vl::cluster::event::frame_done const &evt)
{
	assert(_server);
}

void
vl::cluster::Server::ClientFSM::_do_handle_error(vl::cluster::event::timer_expired const &evt)
{
	assert(_server);
}

/// Server
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
		_sendMessage(**iter, msg);
		(*iter)->process_event(vl::cluster::quit());
	}
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
		Client *cl_ptr = _find_client_ptr(remote_endpoint);
		if( !cl_ptr )
			cl_ptr = _add_client(remote_endpoint);

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
	Client *client = 0;
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
						client = _requested_msgs.at(i).first;
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

bool
vl::cluster::Server::logEnabled(void) const
{
	ClientList::const_iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if((*iter)->is_output_enabled())
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


void
vl::cluster::Server::_request_message(ClientFSM *client, MSG_TYPES type)
{
	assert(!_request_message_signal.empty());
	assert(dynamic_cast<Client *>(client));
	Client *c = static_cast<Client *>(client);
	_requested_msgs.push_back(std::make_pair(c, type));
	_request_message_signal(type);
}

void
vl::cluster::Server::_send_message(Server::ClientFSM *client, Message const &msg)
{
	assert(dynamic_cast<Client *>(client));
	Client *c = static_cast<Client *>(client);

	_sendMessage(*c, msg);
}

/// ---------------------------- FSM public ------------------------------------
void
vl::cluster::Server::_do_init(vl::cluster::event::init const &evt)
{
}

void
vl::cluster::Server::_do_update(vl::cluster::event::update const &evt)
{
	vl::chrono tim;

	if(!has_rendering_clients())
	{ return; }

	// Create list of renderers for this loop
	_renderers.clear();
	for(ClientList::iterator iter = _clients.begin();
		iter != _clients.end(); ++iter)
	{
		if((*iter)->is_ready_for_rendering())
		{ _renderers.push_back(*iter); }
	}

	// Send frame start
	// This is the first message and before this is answered
	// Updates should not be sent
	// this is sent to all
	// why is it sent to all? I'll change it for now.
	for(ClientList::iterator iter = _renderers.begin(); 
		iter != _renderers.end(); ++iter )
	{
		// Send SG_CREATE which is not part of the Rendering Loop darned.
		_sendCreate(**iter);
		// Start the rendering loop on slaves.
		vl::cluster::event::start_frame evt(evt.frame, evt.timestamp);
		(*iter)->process_event(evt);
	}

	tim.reset();

	// @todo this should be somewhere else
	// Send the output messages
	/*
	for( ClientList::iterator iter = _renderers.begin();
		iter != _renderers.end(); ++iter )
	{ _sendOuput(**iter); }
	*/

	// Block till update done
	_block_till_state_has_flag<UpdateDoneFlag>();

	_server_report["wait update time"].push(tim.elapsed());

}

void
vl::cluster::Server::_do_render(vl::cluster::event::render const &evt)
{
	vl::chrono tim;

	// Block till draw started
	_block_till_state_has_flag<DrawStartedFlag>();

	_server_report["wait draw time"].push(tim.elapsed());
}

void
vl::cluster::Server::_do_swap(vl::cluster::event::swap const &evt)
{
	vl::chrono t;

	// Block till draw done
	_block_till_state_has_flag<NotRenderingFlag>();

	_server_report["wait draw done time"].push(t.elapsed());
}

void
vl::cluster::Server::_do_swap_done(vl::cluster::event::swap_done const &evt)
{
	for( ClientList::iterator iter = _renderers.begin();
		iter != _renderers.end(); ++iter )
	{
		(*iter)->process_event(vl::cluster::event::frame_done(evt.frame, evt.timestamp));
	}

	// we need to explicitly call _do_rest because it clears the Rendering Context
	// for now we like to keep _do_swap_done also
	_do_rest(vl::none());
}

void
vl::cluster::Server::_do_rest(vl::none const &evt)
{
	// Clear renderer list
	_renderers.clear();
}

void
vl::cluster::Server::_report_error(vl::cluster::event::timer_expired const &evt)
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
	process_event(vl::cluster::event::clear_error());
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
		if((*iter)->is_ready_for_rendering())
		{ return true; }
	}

	return false;
}


/// ----------------------------- Private --------------------------------------
void 
vl::cluster::Server::_handle_message(vl::cluster::Message &msg, Client &client)
{
	switch( msg.getType() )
	{
		case vl::cluster::MSG_REG_UPDATES :
		{
			std::clog << "vl::cluster::MSG_REG_UPDATES : received" << std::endl;
			// Don't send the environment more than once. 
			// @todo Using ACKs and resend would be even better.
			if(client.environment_sent_time.elapsed() > vl::time(1, 0))
			{
				assert(!_request_message_signal.empty());
				client.process_event(event::init_requested());
				_requested_msgs.push_back(std::make_pair(&client, MSG_ENVIRONMENT));
				_request_message_signal(MSG_ENVIRONMENT);
			}
		}
		break;

		case vl::cluster::MSG_REG_RENDERING :
			// @todo replace with an event
			client.enable_rendering(true);
		break;

		case vl::cluster::MSG_ACK :
		{
			vl::cluster::MSG_TYPES type;
			msg.read(type);
			_handle_ack(client, type, msg);
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
			client.enable_output(true);
		break;

		case vl::cluster::MSG_REQ_SG_UPDATE :
		{
			event::update_requested evt(msg.getFrame(), msg.getTimestamp());
			evt.callback = boost::bind(&Server::_send_message, this, &client, _msg_update);
			client.process_event(evt);
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
			// @todo add evt.callback
			event::update_done evt(msg.getFrame(), msg.getTimestamp());
			Message reply(MSG_DRAW, msg.getFrame(), msg.getTimestamp());
			evt.callback = boost::bind(&Server::_send_message, this, &client, reply);
			client.process_event(evt);
		}
		break;

		case vl::cluster::MSG_DRAWING :
		{
			client.process_event(event::draw(msg.getFrame(), msg.getTimestamp()));
		}
		break;
		
		case vl::cluster::MSG_DRAW_DONE :
		{
			client.process_event(event::draw_done(msg.getFrame(), msg.getTimestamp()));
		}
		break;

		case vl::cluster::MSG_REG_RESOURCE :
		{
			std::clog << "vl::cluster::MSG_REG_RESOURCE message received." << std::endl;
			RESOURCE_TYPE type;
			std::string name;
			msg.read(type);
			msg.read(name);
			assert(!_request_message_signal.empty());
			_requested_msgs.push_back(std::make_pair(&client, MSG_RESOURCE));
			_request_message_signal(RequestedMessage(MSG_RESOURCE, name, type));
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

vl::cluster::Server::Client *
vl::cluster::Server::_add_client( boost::udp::endpoint const &endpoint )
{
	if( _has_client(endpoint) )
	{ assert(false && "Trying to add client twice"); }

	Client *client = new Server::Client();
	client->start();
	client->address = endpoint;
	client->_server = this;
	_clients.push_back(client);

	return client;
}

bool
vl::cluster::Server::_has_client(boost::udp::endpoint const &address) const
{
	return _find_client_ptr(address);
}

vl::cluster::Server::Client &
vl::cluster::Server::_find_client(boost::udp::endpoint const &address)
{
	return *_find_client_ptr(address);
}

vl::cluster::Server::Client *
vl::cluster::Server::_find_client_ptr(boost::udp::endpoint const &address)
{
	ClientList::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( (*iter)->address == address )
		{ return *iter; }
	}

	return 0;
}

vl::cluster::Server::Client const *
vl::cluster::Server::_find_client_ptr(boost::udp::endpoint const &address) const
{
	ClientList::const_iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
		if( (*iter)->address == address )
		{ return *iter; }
	}

	return 0;
}

void
vl::cluster::Server::_sendCreate(Client &client)
{
	std::vector<Message> msgs;

	// Copy all the create messages that are newer than the client
	for( std::vector< std::pair<uint32_t, Message> >::const_reverse_iterator iter = _msg_creates.rbegin();
		iter != _msg_creates.rend(); ++iter )
	{
		if( client.create_frame >= int64_t(iter->first) )
		{ break; }
		msgs.push_back( iter->second );
		// Update the update frame so that the same message will not be sent again
		client.create_frame = iter->first;
	}

	for( std::vector<Message>::const_reverse_iterator iter = msgs.rbegin();
		iter != msgs.rend(); ++iter )
	{
		assert(iter->getType() == MSG_SG_CREATE);
		_sendMessage(client, *iter);
	}
}

void
vl::cluster::Server::_sendOuput(Client &client)
{
	if(!client.is_output_enabled())
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
vl::cluster::Server::_sendMessage(Client &client, vl::cluster::Message const &msg)
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
vl::cluster::Server::_handle_ack(Client &client, vl::cluster::MSG_TYPES ack_to,  vl::cluster::Message const &msg)
{
	switch( ack_to )
	{
		case vl::cluster::MSG_ENVIRONMENT :
		{
			std::clog << "Received an ACK for vl::cluster::MSG_ENVIRONMENT " << std::endl;
			client.process_event(event::init_done());
			client.process_event(event::load_requested());
		}
		break;

		case vl::cluster::MSG_PROJECT :
		{
			std::clog << "Received an ACK for vl::cluster::MSG_PROJECT " << std::endl;
			client.process_event(event::load_done());
			client.process_event(event::graph_requested());
		}
		break;

		case vl::cluster::MSG_SG_CREATE :
			// TODO this ack should have the frame number in it
		break;

		// The rendering loop Messages don't send any new messages
		// because they are tied to the master rendering loop
		case vl::cluster::MSG_FRAME_START :
		break;

		case vl::cluster::MSG_SG_INIT :
		{
			std::clog << "Received an ACK for vl::cluster::MSG_SG_INIT" << std::endl;
			// @todo is this correct event for this? and do we need separate loaded?
			client.process_event(event::graph_loaded());
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
			client.process_event(event::draw_requested(msg.getFrame(), msg.getTimestamp()));
		}
		break;

		case vl::cluster::MSG_SHUTDOWN :
		{
			// @todo is this in the wrong place?
			// or should we have a preliminary quit state which is waiting for the reply?
			client.process_event(event::quit());
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
