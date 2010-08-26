
#include "print_handler.hpp"

#include <iostream>

vl::udp::PrintHandler::PrintHandler(int packet_id)
	: Handler(packet_id)
{
}

vl::udp::PrintHandler::~PrintHandler()
{
}

void
vl::udp::PrintHandler::handle(std::vector< double > msg)
{
	/*
	std::cout << "Message handled : message = ";
	for( size_t i = 0; i < msg.size(); ++i )
	{
		std::cout << msg.at(i) << " ";
	}
	std::cout << std::endl;
	*/

	int msg_pos = 0;
	for( size_t i = 0; i < _packet._commands.size(); ++i )
	{
		boost::shared_ptr<udp::Command> cmd = _packet._commands.at(i);

		// Message is too short
		if( msg.size() - msg_pos < cmd->getSize() )
		{
			std::cerr << "Packet size is too small for the next command" << std::endl;
			break;
		}

		// Assign the new values
		for( size_t j = 0; j < cmd->getSize(); ++j )
		{
			cmd->at(i) = msg.at(i+j);
			msg_pos++;
		}

		// execute the command
		(*cmd)();
	}

	if( msg_pos != msg.size() )
	{
		std::cerr << (msg.size()-msg_pos)*sizeof(double)
			<< " unhandled bytes in the message." << std::endl;
	}

//	std::cout << std::endl;
}
