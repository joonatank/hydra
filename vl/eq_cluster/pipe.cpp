
/* 
 * Copyright (c) 2006-2009, Stefan Eilemann <eile@equalizergraphics.com> 
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 2.1 as published
 * by the Free Software Foundation.
 *  
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 */

#include <eq/eq.h>

#include "config.hpp"

#include "pipe.hpp"

eqOgre::Pipe::Pipe( eq::Node *parent )
	: eq::Pipe( parent ),
	  _read_node_fifo(0),
	  _root(0)
{}

eqOgre::Pipe::~Pipe()
{
}

bool
eqOgre::Pipe::configInit( const uint32_t initID )
{
	EQINFO << "Pipe::configInit" << std::endl;
	if( !eq::Pipe::configInit( initID ))
	{ return false; }

	// TODO we should get Root from config
	eqOgre::Config *config = (eqOgre::Config *)getConfig();

	// Get fifo buffer for reading commands from node
	_read_node_fifo = config->getNodeFifo();
	EQASSERT( _read_node_fifo );

	return true;
}

bool
eqOgre::Pipe::configExit()
{
	return eq::Pipe::configExit();
}

void
eqOgre::Pipe::frameStart( const uint32_t /*frameID*/, const uint32_t frameNumber )
{
	// Wait till Node has finished updating SceneGraph
	// We don't have multi-buffered SceneGraph so we need to call this,
	// removing this will result in data corruptions when both Node amd Pipe
	// threads are accessing the SceneGraph.
	getNode()->waitFrameStarted( frameNumber );

	// Process the command queue from Node
	_processCommands();

	// Removed for now
	// TODO implement command for transmitting Ogre::Root after that
	// these can be used.
	//_root->_fireFrameStarted();

	startFrame( frameNumber );

	//_root->_fireFrameEnded();

	//	We don't need to call frameStart, it only calls startFrame( frameNumber )
//	eq::Pipe::frameStart( frameID, frameNumber );
}

void
eqOgre::Pipe::_processCommands( void )
{
	EQASSERT( _read_node_fifo );

	vl::base::Message *cmd = 0;
	while( (cmd = _read_node_fifo->pop()) )
	{
		// TODO add command processing
		//
		// TODO add commands to transmit Ogre::Root, Ogre::Camera, Ogre::Viewport
		// Ogre::Window.
		// All are to be created in NodeThread and whose pointers are based
		// to here.
		switch( cmd->cmdType )
		{
			/*
			case vl::base::CMD_ROOT_TRANS :
			{
				vl::base::RootTransfer *cc = (vl::base::RootTransfer *)cmd;
				EQASSERT( !_root || _root == cc->root );
				_root = cc->root;
			}
			break;
			*/

			default :
				EQWARN << "Unhandled command!" << std::endl;
				break;
		}
		delete cmd;
	}
}

