/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file states.hpp
 */

#ifndef VL_CLUSTER_STATES_HPP
#define VL_CLUSTER_STATES_HPP

namespace vl
{

namespace cluster
{

/// The state the client is in the update and rendering pipe
/// Used to determine what message is next send to that client
/// Moving from one to the next state is done when server sends the message
/// for the current state.
/// TODO state change should be done only after client ACKs
enum CLIENT_STATE
{
	CS_UNDEFINED,
	CS_REQ,		// Client has requested updates
	CS_ENV,		// Environment settings have been sent
	CS_PROJ,	// Project settings have been sent
	CS_INIT,	// Initial SceneGraph has been sent
	CS_UPDATE,	// Rendering loop : Update has been sent
	CS_DRAW,	// Rendering loop : Draw has been sent
	CS_SWAP,	// Rendering loop : Swap has been sent
	CS_SHUTDOWN,	// Has been shutdown
};

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_STATES_HPP
