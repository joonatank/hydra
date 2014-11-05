/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-03
 *	@file remote_launcher_helper.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_REMOTE_LAUCHER_HELPER_HPP
#define HYDRA_REMOTE_LAUCHER_HELPER_HPP

#include <boost/asio.hpp>

#include <stdint.h>

namespace boost
{
	using boost::asio::ip::udp;
}

namespace vl
{

class RemoteLauncherHelper
{
public :
	RemoteLauncherHelper(uint16_t port);

	void send_start(uint16_t server_port);

	void send_kill(void);

private :

	boost::asio::io_service _io_service;
	boost::udp::socket _socket;
	boost::asio::ip::udp::endpoint _bc_address;

};	// class RemoteLauncherHelper

}	// namespace vl

#endif	// HYDRA_REMOTE_LAUCHER_HELPER_HPP