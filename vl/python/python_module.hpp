/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file python_module.hpp
 *
 *	This file is part of Hydra VR game engine.
 */

#ifndef HYDRA_PYTHON_MODULE_HPP
#define HYDRA_PYTHON_MODULE_HPP

// Callback helpers
#include <toast/python/callback.hpp>

// Python global
#include "python.hpp"

void export_math(void);

void export_animation(void);

void export_physics(void);

void export_scene_graph(void);

void export_game(void);

void export_events(void);

void export_gui(void);

#endif	// HYDRA_PYTHON_MODULE_HPP
