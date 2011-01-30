/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "server.hpp"

#include "base/exceptions.hpp"

#include "message.hpp"

vl::cluster::Server::Server( uint16_t const port )
	: _socket(_io_service, boost::udp::endpoint(boost::udp::v4(), port))
{
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
		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
			remote_endpoint, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		Message *msg = new Message(recv_buf);
		switch( msg->getType() )
		{
			case vl::cluster::MSG_REG_UPDATES :
			{
				_addClient( remote_endpoint );
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
vl::cluster::Server::sendToAll( vl::cluster::Message const &msg )
{
// 	std::cout << "Sending message " << msg;

// 	std::cout << "vl::cluster::Server::sendToAll" << std::endl;
	std::vector<boost::udp::endpoint>::iterator iter;
	for( iter = _clients.begin(); iter != _clients.end(); ++iter )
	{
// 		std::cout << "Sending to client = " << *iter << std::endl;
		std::vector<char> buf;
		msg.dump(buf);
		_socket.send_to( boost::asio::buffer(buf), *iter );
	}
}

void
vl::cluster::Server::sendToNewClients( vl::cluster::Message const &msg )
{
// 	std::cout << "vl::cluster::Server::sendToNewClients" << std::endl;
// 	std::cout << "Sending message " << msg;

	std::vector<boost::udp::endpoint>::iterator iter;
	for( iter = _new_clients.begin(); iter != _new_clients.end(); ++iter )
	{
// 		std::cout << "Sending to client = " << *iter << std::endl;
		std::vector<char> buf;
		msg.dump(buf);
		_socket.send_to( boost::asio::buffer(buf), *iter );
		_clients.push_back( *iter );
	}
	_new_clients.clear();
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
	std::vector<boost::udp::endpoint>::iterator iter
		= std::find( _clients.begin(), _clients.end(), endpoint );

	if( iter == _clients.end() )
	{
		iter = std::find( _new_clients.begin(), _new_clients.end(), endpoint );
		if( iter == _new_clients.end() )
		{ _new_clients.push_back( endpoint ); }
	}
}
