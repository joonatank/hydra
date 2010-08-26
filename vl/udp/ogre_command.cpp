#include "ogre_command.hpp"

vl::udp::OgreCommand::OgreCommand ( const std::string& typ, Ogre::SceneNode* node ) 
	: _node(node)
{

}


void
vl::udp::OgreCommand::operator() ( void )
{

}

uint16_t
vl::udp::OgreCommand::getSize ( void ) const
{
	if( _type == CMD_POS )
	{ return 3; }
	if( _type == CMD_ROT_QUAT )
	{ return 4; }
	if( _type == CMD_ROT_AA )
	{ return 4; }
	
	return 0;
}
