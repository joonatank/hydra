#include "command.hpp"
#include "cmd_network.hpp"
#include "exceptions.hpp"

#include <RakNetworkFactory.h>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>

unsigned int const PORT = 5333;
char const *SERVER_IP = "127.0.0.1";

int main( int argc, char **argv )
{
	// TODO this should send commands to server on pre defined port
	RakPeerInterface *peer = RakNetworkFactory::GetRakPeerInterface();
	if( !peer )
	{ return -1; }

	peer->Startup( 1, 0, &SocketDescriptor(), 1 );
	
	std::cout << "Starting the client." << std::endl;
	peer->Connect( SERVER_IP, PORT, 0, 0 );

	Packet *packet = 0;
	size_t count = 0;
	while( 1 )
	{
		packet = peer->Receive();

		if( count == 1e3 )
		{
			count = 0;
			RakNet::BitStream stream;
			vl::base::CreateCmd cmd( "Rob", "SceneNode" );
			stream << cmd;
			peer->Send( &stream, HIGH_PRIORITY, RELIABLE, 0, UNASSIGNED_SYSTEM_ADDRESS,
					false );
		}
		count++;

		while( packet )
		{
			switch( packet->data[0] )
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
					printf("We have been disconnected.\n");
				break;
			case ID_CONNECTION_LOST:
					printf("Connection lost.\n");
				break;
			default:
				printf("Message with identifier %i has arrived.\n", packet->data[0]);
				break;
			}

			peer->DeallocatePacket(packet);
			// Stay in the loop as long as there are more packets.
			packet = peer->Receive();
		}
	}

	return 0;
}
