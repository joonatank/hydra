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

/// The rendering state that the client is in rendering loop
/// TODO should be divided to Rendering state and client state
enum CLIENT_STATE
{
	CS_UNDEFINED,
	CS_UPDATE_READY,// Ready to receive an update
	CS_UPDATE,		// NOT IN USE, Update has been sent
	CS_UPDATE_DONE,	// NOT IN USE, An update has been completed
	CS_DRAW_READY,
	CS_DRAW,		// Rendering loop : Draw has been sent
	CS_DRAW_DONE,
};

/// Holds a compound state and allows complex queries
/// Possible for non exclusive states
/// TODO change booleans to FLAGS
/// TODO change rendering state to FLAGS
struct ClientState
{
	ClientState(void)
		: environment(false), project(false), rendering(false)
		, wants_render(false), wants_output(false)
		, has_init(false), frame(0), update_frame(0)
		, rendering_state(CS_UNDEFINED), shutdown(false)
	{}

	// TODO implement print
	// TODO implement comparison

	// Has environment config
	bool environment;
	// Has project config
	bool project;
	// Is currently rendering
	bool rendering;
	// Wants rendering commands
	bool wants_render;
	// Wants output
	bool wants_output;
	// true if init message has been sent
	bool has_init;
	// Last renderer frame
	uint32_t frame;
	// Last received update
	uint32_t update_frame;
	// Rendering state, only valid if rendering is true
	CLIENT_STATE rendering_state;
	bool shutdown;
};

inline
char const *getStateAsString(CLIENT_STATE cs)
{
	switch(cs)
	{
	case CS_UNDEFINED :
		return "CS_UNDEFINED";
//	case CS_REQ :
//		return "CS_REQ";
//	case CS_ENV :
//		return "CS_ENV";
//	case CS_PROJ :
//		return "CS_PROJ";
//	case CS_INIT :
//		return "CS_INIT";
	case CS_UPDATE_READY :
		return "CS_UPDATE_READY";
	case CS_UPDATE :
		return "CS_UPDATE";
	case CS_UPDATE_DONE :
		return "CS_UPDATE_DONE";
	case CS_DRAW_READY :
		return "CS_DRAW_READY";
	case CS_DRAW_DONE :
		return "CS_DRAW_DONE";
//	case CS_SHUTDOWN :
//		return "CS_SHUTDOWN";
	default :
		return "";
	}
}

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_STATES_HPP
