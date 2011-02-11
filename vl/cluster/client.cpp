/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-01
 */

#include "client.hpp"

#include "base/exceptions.hpp"

#include <sstream>

#include <iostream>

const size_t MSG_BUFFER_SIZE = 128;

vl::cluster::Client::Client( char const *hostname, uint16_t port )
	: _io_service(), _socket( _io_service ), _master()
{
	std::cout << "vl::cluster::Client::Client : Connecting to host " 
		<< hostname << " at port " << port << "." << std::endl;

	std::stringstream ss;
	ss << port;
	boost::udp::resolver resolver( _io_service );
	boost::udp::resolver::query query( boost::udp::v4(), hostname, ss.str().c_str() );
	_master = *resolver.resolve(query);

	_socket.open( boost::udp::v4() );

	// @todo this is not a real solution but allowes us to receive large
	// messages
	// replace this by dividing the messages to multiple parts before
	// sending
	boost::asio::socket_base::receive_buffer_size buf_size(8*8192);
	_socket.set_option(buf_size);
	std::cout << "vl::cluster::Client::Client : Receive Buffer size = " 
		<< buf_size.value() << "." << std::endl;
}

vl::cluster::Client::~Client( void )
{

}

void
vl::cluster::Client::mainloop( void )
{
// 	std::cout << "vl::cluster::Client::mainloop" << std::endl;
	if( _socket.available() != 0 )
	{
		// @todo does this system work for multiple messages also?
		// This doesn't seem to have any effect what so ever
		boost::asio::socket_base::receive_buffer_size buf_size;
		_socket.get_option(buf_size);
		if( buf_size.value() < _socket.available() )
		{
			std::stringstream ss;
			std::string err("Receive buffer is too small : ");
			ss << err << _socket.available() << " bytes available : buffer size " 
				<< buf_size.value() << " bytes.";
			BOOST_THROW_EXCEPTION( vl::long_message() << vl::desc(ss.str()) );
		}

		std::vector<char> recv_buf( _socket.available() );
		boost::system::error_code error;

		size_t n = _socket.receive_from( boost::asio::buffer(recv_buf),
				_master, 0, error );

		if (error && error != boost::asio::error::message_size)
		{ throw boost::system::system_error(error); }

		// Some constraints for the number of messages
		if( _messages.size() > MSG_BUFFER_SIZE )
		{
			std::cerr << "Message stack full, cleaning out the oldest." << std::endl;
			while( _messages.size() > MSG_BUFFER_SIZE )
			{ delete popMessage(); }
		}

		Message *msg = new Message( recv_buf );
		_messages.push_back(msg);
//		std::cout << "Message of type " << getTypeAsString(msg->getType()) << " with " << msg->size()
//			<< " bytes received." << std::endl;
	}
}

void
vl::cluster::Client::registerForUpdates( void )
{
	std::cout << "vl::cluster::Client::registerForUpdates" << std::endl;
	Message msg( MSG_REG_UPDATES );
	std::vector<char> buf;
	msg.dump(buf);
// 	std::cout << "sending data" << std::endl;
	_socket.send_to( boost::asio::buffer(buf), _master);
}

vl::cluster::Message *
vl::cluster::Client::popMessage( void )
{
	Message *tmp = _messages.front();
	_messages.erase( _messages.begin() );
	return tmp;
}

void
vl::cluster::Client::sendMessage( vl::cluster::Message *msg )
{
	std::vector<char> buf;
	msg->dump(buf);
// 	std::cout << "sending data" << std::endl;
	_socket.send_to( boost::asio::buffer(buf), _master );
}

void
vl::cluster::Client::sendAck ( vl::cluster::MSG_TYPES type )
{
	Message msg( vl::cluster::MSG_ACK );
	msg.write(type);
	sendMessage(&msg);
}

/// ------------------------ Private -------------------------------------------
