/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file channel.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#ifndef HYDRA_CHANNEL_HPP
#define HYDRA_CHANNEL_HPP


// Necessary for Window config and Wall
#include "base/config.hpp"

// need to be included before including any other stuff otherwise we get errors for GLEW
#include <HydraGL/src/OgreGLPrerequisites.h>

#include <OGRE/OgreViewport.h>
#include <OGRE/OgreTexture.h>
#include <OGRE/OgreRectangle2D.h>

#include "stereo_camera.hpp"

namespace vl
{

/// @todo should these be flags instead? or is it too hard to implement
enum RENDER_MODE
{
	// Default mode render directly to screen
	RM_WINDOW,
	// Render to FBO first then Blit the FBO directly to screen
	RM_FBO,		// Working fine for both mono and stereo buffers
	// Render first G-Buffer and then lights
	RM_DEFERRED,	// Work in progress
	RM_OCULUS,
};

inline std::string getRenderModeName(RENDER_MODE rm)
{
	switch(rm)
	{
	case RM_WINDOW:
		return "RM_WINDOW";
	case RM_FBO:
		return "RM_FBO";
	case RM_DEFERRED:
		return "RM_DEFERRED";
	case RM_OCULUS:
		return "RM_OCULUS";
	default:
		return "RM_UNKNOWN";
	}
	
}

/**	@brief Channel
 *	@todo remove direct Window rendering
 *	cleans up the the code a lot, both Deferred and Oculus use FBOs anyway
 *	FBO rendering is just slightly slower than direct window while
 *	it has the possibilities to use post processing effects etc.
 *
 *	@todo break into multiple versions, FBO and Deferred at least
 *
 *	@todo Deferred with Oculus?
 */
class Channel
{
public:
	/// Constructor
	/// @todo add FBO size as a parameter
	/// we can add it to channel config, no need to distribute it for now though
	/// @todo also move fsaa and rm to Channel config, they are annoying
	/// we can probably remove rm when we remove Window rendering and split the Channel
	/// to multiple classes
	/// can we move stereo_cfg to config?
	Channel(vl::config::Channel config, STEREO_EYE stereo_cfg, vl::Window *parent);

	virtual ~Channel(void);

	std::string const &getName(void) const
	{ return _name; }

	/// the default implementation for Window
	virtual void setCamera(vl::CameraPtr cam);

	/// Predraw step
	/// use the private template to override
	void update(void);

	/// Draw step
	/// use the private template to override
	void draw(void);

	/// @brief get the size of the Channel in homogeneous coordinates
	/// No setSize for now
	Rect<double> getSize(void) const
	{ return _size; }

	/// Per frame statistics
	virtual vl::scalar getLastFPS(void) const;

	/// @brief returns the number of triangles rendered in a frame
	/// For window targets this returns zero intentionally because
	/// windows can't provide statistics per viewport.
	virtual size_t getTriangleCount(void) const;

	/// @brief returns the number of batches rendered in a frame
	/// For window targets this returns zero intentionally because
	/// windows can't provide statistics per viewport.
	virtual size_t getBatchCount(void) const;

	virtual void resetStatistics(void);

	StereoCamera &getCamera(void)
	{ return _camera; }

	Ogre::Viewport *getWindowViewport(void)
	{ return _win_viewport; }

	virtual Ogre::Viewport *getRenderViewport(void);

	/// For Oculus
	void setCustomProjMatrix(bool use, vl::Matrix4 const proj);
	void setCustomViewMatrix(bool use, vl::Matrix4 const view);

	/// @todo do we need these here? they are only implemted for FBO targets
	// Dirty hack to retrieve the texture ID for the RenderTarget
	// DO NOT use either of these on non-fbo targets or Deferred targets
	//
	// we are going to remove non FBO targets so these will just assert
	// fail for old Windowed rendering
	virtual uint32_t getTextureID(void) const
	{ assert(false); return 0; }

	virtual uint32_t getFBOID(void) const
	{ assert(false); return 0; }

	/// @brief get the Texture size
	/// only valid for FBO render targets
	virtual Rect<uint32_t> getTextureSize(void) const
	{ assert(false); return Rect<uint32_t>(); }

private :
	// template method pattern
	virtual void _predraw(void) {}

	virtual void _draw(void);

	/// Methods
	// @todo sort out private/protected
protected :

	void _update_camera();

	void _copy_player(void);

	Ogre::TexturePtr _create_fbo(std::string const &name, Ogre::PixelFormat pf);

	void _create_screen_quad(std::string const &name, std::string const &material_name);

	uint32_t _get_window_mask(void)
	{
		// minimum 1<<2 because 1<<0 is the scene mask
		// and 1<<1 is the light mask
		return 1 << (_win_viewport->getZOrder()+2); 
	}

	uint32_t _get_scene_mask(void)
	{ return 1; }

	uint32_t _get_light_mask(void)
	{ return 1<<1; }

	// convenience function for fbo rendering
	uint32_t _get_fbo_mask(void)
	{ return _get_scene_mask() | _get_light_mask(); }

	/// Data
	// @todo sort out private/protected
protected :
	std::string _name;
	Rect<double> _size;
	// saved variable for FBO creation, might not be actual texture size
	vl::vec2i _texture_size;

	StereoCamera _camera;
	/// Window viewport, for FBOs this is the non render viewport
	/// For windows this is the render viewport
	Ogre::Viewport *_win_viewport;

	uint32_t _fsaa;

	// -1 for left, +1 for right
	vl::scalar _stereo_eye;

	Ogre::SceneNode* _quad_node;

	/// OpenGL buffer we use for drawing (used for switching quad buffer eyes)
	int _draw_buffer;

	/// Oculus hacks
	// don't really need these for anything other than FBO but they logically
	// don't belong there
	bool _use_custom_view;
	bool _use_custom_proj;
	Matrix4 _custom_view;
	Matrix4 _custom_proj;

	vl::Window *_parent;

};	// class Channel

}	// namespace vl

#endif	// HYDRA_CHANNEL_HPP
