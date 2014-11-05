/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file python/python_module.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PYTHON_MODULE_HPP
#define HYDRA_PYTHON_MODULE_HPP

void export_math(void);

void export_animation(void);

void export_physics(void);

void export_scene_graph(void);

void export_game(void);

void export_events(void);

void export_gui(void);

#endif	// HYDRA_PYTHON_MODULE_HPP
