/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-03
 *	@file remote_launcher_helper.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "remote_launcher_helper.hpp"

vl::RemoteLauncherHelper::RemoteLauncherHelper(uint16_t port)
	: _io_service()
	, _socket( _io_service )
{	
	_socket.open(boost::udp::v4());
	_socket.set_option(boost::asio::socket_base::broadcast(true));

	_bc_address = boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4::broadcast(), port);
}

void
vl::RemoteLauncherHelper::send_start(uint16_t server_port)
{
	std::stringstream ss;
	ss << "start " << server_port;
	/// @todo should sent the Hydra server port with the start command
	_socket.send_to(boost::asio::buffer(ss.str()), _bc_address);
}

void
vl::RemoteLauncherHelper::send_kill(void)
{
	std::string msg("kill");
	_socket.send_to(boost::asio::buffer(msg), _bc_address);
}
