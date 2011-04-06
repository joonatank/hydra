/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file states.hpp
 */

#ifndef VL_CLUSTER_STATES_HPP
#define VL_CLUSTER_STATES_HPP

#include <stdint.h>

namespace vl
{

namespace cluster
{

/// The rendering state that the client is in rendering loop
/// Flags
enum CLIENT_STATE
{
	CS_CLEAR = 0,
	CS_UPDATE_READY = 1<<0,	// Ready to receive an update
	CS_UPDATE		= 1<<1,	// NOT IN USE, Update has been sent
	CS_UPDATE_DONE	= 1<<2,	// NOT IN USE, An update has been completed
	CS_DRAW_READY	= 1<<3,
	CS_DRAW			= 1<<4,	// Rendering loop : Draw has been sent
	CS_DRAW_DONE	= 1<<5,
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
		, rendering_state(CS_CLEAR), shutdown(false)
	{}

	// TODO implement print
	// TODO implement comparison

	void clear_rendering_state(void)
	{ rendering_state = CS_CLEAR; }

	/// @brief check if client is in a particular state
	bool has_rendering_state(CLIENT_STATE cs) const
	{ return (rendering_state & cs) == cs; }

	void set_rendering_state(CLIENT_STATE cs)
	{ rendering_state = rendering_state | cs; }

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
	uint32_t rendering_state;
	bool shutdown;
};

inline
char const *getStateAsString(CLIENT_STATE cs)
{
	switch(cs)
	{
	case CS_CLEAR :
		return "CS_CLEAR";
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
	default :
		return "";
	}
}

}	// namespace cluster

}	// namespace vl

#endif // VL_CLUSTER_STATES_HPP
