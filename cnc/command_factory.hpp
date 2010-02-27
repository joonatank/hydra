#ifndef VL_SERVER_COMMAND_FACTORY_HPP
#define VL_SERVER_COMMAND_FACTORY_HPP

#include "command.hpp"

namespace vl
{

namespace server 
{

	class CommandFactory
	{
		public :
			CommandFactory( void )
			{ }

			virtual ~CommandFactory( void )
			{}

			vl::base::Command getCommand( vl::base::CommandStruct const &cmd ) = 0;


	};	// class CommandFactory

	class ServerCommandFactory
	{
		public :
			ServerCommandFactory( void )
			{}

			virtual ServerCommandFactory( void )
			{}

			std::auto_ptr<vl::base::Command>
				getCommand( vl::base::CommandStruct const &cc )
			{
				std::auto_ptr<vl::server::Command> cmd;
				switch( cmd.cmdType )
				{
					case CMD_CREATE:
						cmd.reset( new CreateCmd( cc ) );
						break;
					case CMD_DELETE:
						cmd.reset( new DeleteCmd( cc ) );
						break;
					case CMD_MOVE:
						cmd.reset( new MoveCmd( cc ) );
						break;
					case CMD_ROTATE:
						cmd.reset( new RotateCmd( cc ) );
						break;
					case CMD_LOOKAT:
						cmd.reset( new RotateCmd( cc ) );
						break;
					default :
						break;
				}
				return cmd;
			}
	};

	};	// class ServerCommandFactory

}	// namespace base

}	// namespace vl

#endif
