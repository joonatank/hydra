/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2014-11
 *	@file channel_fbo.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#ifndef HYDRA_CHANNEL_FBO_HPP
#define HYDRA_CHANNEL_FBO_HPP

#include "channel.hpp"

namespace vl
{

class ChannelFBO : public Channel
{
public :

	/// Constructor
	/// @todo add FBO size as a parameter
	/// we can add it to channel config, no need to distribute it for now though
	/// @todo also move fsaa and rm to Channel config, they are annoying
	ChannelFBO(vl::config::Channel config,
		STEREO_EYE stereo_cfg, vl::Window *parent);

	~ChannelFBO(void);

	/// Overrides
	virtual void setCamera(vl::CameraPtr cam);

	virtual Ogre::Viewport *getRenderViewport(void);

	virtual vl::scalar getLastFPS(void) const;

	virtual size_t getTriangleCount(void) const;

	virtual size_t getBatchCount(void) const;
	
	virtual void resetStatistics(void);

	virtual uint32_t getTextureID(void) const;

	virtual uint32_t getFBOID(void) const;

	virtual Rect<uint32_t> getTextureSize(void) const;

private :
	virtual void _predraw(void);

	virtual void _draw(void);

	/// nop if null parameter is passed here
	void _set_fbo_camera(vl::CameraPtr cam, std::string const &base_material);

	/// @param base_material is used to select post processing effects
	/// rtt has no post processing, oculus_rtt has distortion for Oculus Rift
	void _initialise_fbo(vl::CameraPtr camera, std::string const &base_material);

	/// For FBO targets this is the render viewport, for others it's null
	Ogre::Viewport *_fbo_viewport;

	// FBO is only available for FBO rendering
	Ogre::RenderTexture *_fbo;

	Ogre::Camera *_rtt_camera;

	Ogre::MaterialPtr _fbo_material;
	Ogre::TexturePtr _fbo_texture;

};

}

#endif // HYDRA_CHANNEL_FBO_HPP
