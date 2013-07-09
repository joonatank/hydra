/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-02
 *	@file distrib_settings.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/**	Classes for distributing Environment Settings and Project settings
 *	using ByteStream
 */

#ifndef HYDRA_DISTRIB_SETTINGS_HPP
#define HYDRA_DISTRIB_SETTINGS_HPP

// Base class
#include "cluster/message.hpp"

// Which classes these operate
#include "base/envsettings.hpp"
#include "base/projsettings.hpp"

namespace vl
{

namespace cluster
{

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::EnvSettingsRefPtr const &env );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::EnvSettingsRefPtr &env );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::Node const &node );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::Node &node );

template<>
ByteStream &operator<<( ByteStream &msg, vl::Wall const &wall );

template<>
ByteStream &operator>>( ByteStream &msg, vl::Wall &wall );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::Window const &window );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::Window &window );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::Channel const &chan );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::Channel &chan );

template<>
ByteStream &operator<<(ByteStream &msg, vl::config::Renderer const &rend);

template<>
ByteStream &operator>>(ByteStream &msg, vl::config::Renderer &rend);

template<>
ByteStream &operator<<(ByteStream &msg, vl::config::Projection const &projection);

template<>
ByteStream &operator>>(ByteStream &msg, vl::config::Projection &projection);

template<>
ByteStream &operator<<( ByteStream &msg, vl::Settings const &settings);

template<>
ByteStream &operator>>( ByteStream &msg, vl::Settings &settings );

template<>
ByteStream &operator<<( ByteStream &msg, vl::config::ProjSettings const &proj );

template<>
ByteStream &operator>>( ByteStream &msg, vl::config::ProjSettings &proj );

}	// namespace cluster

}	// namespace vl

#endif // HYDRA_DISTRIB_SETTINGS_HPP
