/**
 *	Copyright (c) 2011 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-10
 *	@file hydra.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_DEFINES_HPP
#define HYDRA_DEFINES_HPP

// Determine the Operating System
// Only supported OS are Windows and Linux for now
#ifdef _WIN32
#define HYDRA_WIN32
#else
#define HYDRA_LINUX
#endif

// Determine the compiler
#ifdef _MSC_VER
#define HYDRA_MSVC
#else
#ifdef __GNUC__
#define HYDRA_GCC
#endif
#endif

// Determine the build type
// Only supported on Windows as this does not at this moment make sense on Linux
#ifdef HYDRA_WIN32
#ifdef _DEBUG
#define HYDRA_DEBUG
#endif
#endif

// Determine the Architecture
// MSVC
#if _WIN32 || _WIN64
#if _WIN64
#define HYDRA_ARCHITECTURE_64
#else
#define HYDRA_ARCHITECTURE_32
#endif
#endif

// GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define HYDRA_ARCHITECTURE_64
#else
#define HYDRA_ARCHITECTURE_32
#endif
#endif

#ifdef HYDRA_WIN32
#ifdef HYDRA_SHARED_LIB
#define HYDRA_API __declspec(dllexport)
#else
#define HYDRA_API
#endif
#else
#define HYDRA_API
#endif

#endif	// HYDRA_DEFINES_HPP
