#include "udp_server.hpp"

// RakNet
#include <raknet/RakNetworkFactory.h>
#include <raknet/MessageIdentifiers.h>
#include <raknet/RakNetTypes.h>

// Equalizer, used for EQASSERT (network assert)
//#include <eq/eq.h>

#include "cmd_network.hpp"

unsigned int const vl::udp::Server::MAX_CLIENTS = 8;

// Constructor
vl::udp::Server::Server( unsigned int port )
	: _cmd_queue( FIFO_SIZE )
{
	// Create the RakNet server
	_peer = RakNetworkFactory::GetRakPeerInterface();
	//EQASSERT( _peer );

	_peer->Startup( MAX_CLIENTS, 0, &SocketDescriptor( port, 0 ), 1 );
	_peer->SetMaximumIncomingConnections( MAX_CLIENTS );
}

// Destructor
vl::udp::Server::~Server( void )
{

}

void
vl::udp::Server::receive( void )
{
//	EQASSERT( _peer );

	// Using auto_ptr so we can use operator>> to create the command
	// and also to not worry about it... :)

	// TODO packets can have multiple commands, we need to pump all commands
	// from one message. (size of command ~= 16*4+16*2+16+12 = 124 bytes )
	// and one normal packet can contain ~= 63 kbytes, so one packet
	// can contain 528 commands... using ipv4.
	std::auto_ptr< vl::server::Command > cmd;
	Packet *rak_packet = _peer->Receive();
	while( rak_packet )
	{
		switch( rak_packet->data[0] )
		{
			case ID_REMOTE_DISCONNECTION_NOTIFICATION:
				printf("Another client has disconnected.\n");
				break;

			case ID_REMOTE_CONNECTION_LOST:
				printf("Another client has lost the connection.\n");
				break;

			case ID_REMOTE_NEW_INCOMING_CONNECTION:
				printf("Another client has connected.\n");
				break;

			case ID_CONNECTION_REQUEST_ACCEPTED:
				printf("Our connection request has been accepted.\n");
				break;

			case ID_NEW_INCOMING_CONNECTION:
				printf("A connection is incoming.\n");
				break;

			case ID_NO_FREE_INCOMING_CONNECTIONS:
				printf("The server is full.\n");
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				printf("A client has disconnected.\n");
				break;

			case ID_CONNECTION_LOST:
				printf("A client lost the connection.\n");
				break;

			case ID_VL_COMMAND:
				printf("Command arrived.\n");
				// TODO add the real stuff here. Creation of command and so on...
				// Push new command to queue
				if( cmd.get() )
				{ _cmd_queue.push( cmd.release() ); }
				break;

			default:
				printf("Message with identifier %i has arrived.\n", rak_packet->data[0]);
				break;
		}

		_peer->DeallocatePacket( rak_packet );
		rak_packet = _peer->Receive();
	}
}

// Get the next command from the queue, also removes it from the queue
std::auto_ptr<vl::server::Command>
vl::udp::Server::popCommand( void )
{
	return std::auto_ptr<vl::server::Command>( _cmd_queue.pop() );
}

