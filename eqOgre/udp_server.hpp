#ifndef VL_UDP_SERVER
#define VL_UDP_SERVER

// RakNet
#include <raknet/RakPeerInterface.h>

#include "fifo_buffer.hpp"
#include "server_command.hpp"

namespace vl
{
	namespace udp
	{
		size_t const FIFO_SIZE = 128;

		class Server
		{
		public :
			Server( unsigned int port );
			~Server( void );

			// Receives network commands through RakNet peer
			void receive( void );

			// Pops a command from queue
			std::auto_ptr<vl::server::Command> popCommand( void );

			// For now hard-coded max clients
			static unsigned int const MAX_CLIENTS;

		protected :
			// For now using static size fifo_buffer
			// TODO replace with either dynamic size ring buffer or
			// add callbacks called when message is received.
			//
			// For dynamic ring buffer, we should use leaps in size like
			// 32 -> 128 -> 512, and shrinking only occuring when the
			// buffer is empty.
			vl::base::fifo_buffer<vl::server::Command *> _cmd_queue;

			// Network specif data, for now RakNet
			RakPeerInterface *_peer;

		};	// class Server

	}	// namespace udp

}	// namespace vl

#endif
