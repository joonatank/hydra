/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2014-11
 *	@file channel_deferred.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#ifndef HYDRA_CHANNEL_DEFERRED_HPP
#define HYDRA_CHANNEL_DEFERRED_HPP

#include "channel.hpp"

namespace vl
{

class ChannelDeferred : public Channel
{
public :
	/// Constructor
	/// @todo add FBO size as a parameter
	/// we can add it to channel config, no need to distribute it for now though
	/// @todo also move fsaa and rm to Channel config, they are annoying
	ChannelDeferred(vl::config::Channel config, STEREO_EYE stereo_cfg, vl::Window *parent);

	~ChannelDeferred(void);

	/// Overrides
	virtual void setCamera(vl::CameraPtr cam);

	virtual Ogre::Viewport *getRenderViewport(void);

	virtual vl::scalar getLastFPS(void) const;

	virtual size_t getTriangleCount(void) const;

	virtual size_t getBatchCount(void) const;
	
	virtual void resetStatistics(void);

private :
	// template method pattern
	void _predraw(void);

	void _draw(void);

		/// nop if null parameter is passed here
	void _set_mrt_camera(vl::CameraPtr cam);

	void _initialise_mrt(vl::CameraPtr camera);

	void _deferred_geometry_pass(void);

	void _deferred_light_pass(void);

/// data
private :
	Ogre::MultiRenderTarget *_mrt;

	Ogre::Camera *_rtt_camera;

	std::vector<Ogre::MaterialPtr> _fbo_materials;
	std::vector<Ogre::TexturePtr> _fbo_textures;

};

}

#endif // HYDRA_CHANNEL_DEFERRED_HPP
