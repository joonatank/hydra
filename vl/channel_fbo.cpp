/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2014-11
 *	@file channel_fbo.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#include "channel_fbo.hpp"

/// Necessary for rendering to FBO
#include <OGRE/OgreRenderTexture.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgrePass.h>

#include <OGRE/OgreRoot.h>

// Necessary for getting the OpenGL id of the texture
#include <HydraGL/src/OgreGLFrameBufferObject.h>

vl::ChannelFBO::ChannelFBO(vl::config::Channel config,
		STEREO_EYE stereo_cfg, vl::Window *parent)
	: Channel(config, stereo_cfg, parent)
	, _fbo_viewport(0)
	, _fbo(0)
{
}

vl::ChannelFBO::~ChannelFBO(void)
{
}

void
vl::ChannelFBO::setCamera(vl::CameraPtr cam)
{
	Ogre::Camera *og_cam = 0;
	if(cam)
	{ og_cam = (Ogre::Camera *)cam->getNative(); }

	_camera.setCamera(cam);

	_set_fbo_camera(cam, "rtt");
}

void
vl::ChannelFBO::_predraw(void)
{
	assert(_quad_node);
	_quad_node->setVisible(true);

	// Clearing the doesn't make a difference (well not noticable anyway)

	// beginUpdate and endUpdate screws the GUI and makes the
	// second window black in both windowed and FBO modes
	// without them though FBO bleeds to second channel
	//_fbo->_beginUpdate();
	_win_viewport->clear();
	_update_camera();
	_fbo->update();
}

void
vl::ChannelFBO::_draw(void)
{
	// @todo do we need to set the draw buffer
	//glDrawBuffer(_draw_buffer);

	// draw to screen
	_win_viewport->getTarget()->_updateViewport(_win_viewport, true);

	assert(_quad_node);
	_quad_node->setVisible(false);
}

Ogre::Viewport *
vl::ChannelFBO::getRenderViewport(void)
{
	assert(_fbo_viewport);
	return _fbo_viewport;
}

vl::scalar
vl::ChannelFBO::getLastFPS(void) const
{
	assert(_fbo);
	return _fbo->getLastFPS();
}

size_t
vl::ChannelFBO::getTriangleCount(void) const
{
	assert(_fbo);
	return _fbo->getTriangleCount();
}

size_t
vl::ChannelFBO::getBatchCount(void) const
{
	assert(_fbo);
	return _fbo->getBatchCount();
}
	
void
vl::ChannelFBO::resetStatistics(void)
{
	assert(_fbo);
	_fbo->resetStatistics();
}

uint32_t
vl::ChannelFBO::getTextureID(void) const
{
	// @fixme this is bad, should work for FBO targets, but not Deferred
	assert(_fbo_texture.get());

	Ogre::GLTexture *tex = dynamic_cast<Ogre::GLTexture *>(_fbo_texture.get());
	GLuint id;
	tex->getCustomAttribute("GLID", &id);
	return id;
}

uint32_t
vl::ChannelFBO::getFBOID(void) const
{
	assert(_fbo);

	GLuint fbo = 0;
	_fbo->getCustomAttribute("GL_FBOID", &fbo);
	
	return fbo;
}

vl::Rect<uint32_t>
vl::ChannelFBO::getTextureSize(void) const
{
	assert(_fbo);
	Rect<uint32_t> size;
	uint32_t depth;
	_fbo->getMetrics(size.w, size.h, depth);
	return size;
}

/// -------------------------- Private ---------------------------------------
void
vl::ChannelFBO::_set_fbo_camera(vl::CameraPtr camera, std::string const &base_material)
{
	/// @todo useless checking
	if(!camera)
	{ return; }

	/// FBO needs to be initialised here because we need Ogre::SceneManager for it
	if(!_fbo)
	{
		_initialise_fbo(camera, base_material);
	}
	else
	{
		_fbo->getViewport(0)->setCamera((Ogre::Camera *)camera->getNative());
	}
}

void
vl::ChannelFBO::_initialise_fbo(vl::CameraPtr camera, std::string const &base_material)
{
	std::clog << "vl::Channel::_initialise_fbo : with material " << base_material << std::endl;

	std::string name("internal/" + getName());
	std::string material_name(name + "/fbo_material");
	std::string texture_name(name + "/rtt_tex");
	assert(!_fbo_texture.get());
	_fbo_texture = _create_fbo(name, Ogre::PF_R8G8B8);
	assert(_fbo_texture.get());
	_fbo = _fbo_texture->getBuffer()->getRenderTarget();

	Ogre::ResourcePtr base_mat_res = Ogre::MaterialManager::getSingleton().getByName(base_material);
	_fbo_material = static_cast<Ogre::Material *>(base_mat_res.get())->clone(material_name);
	_fbo_material->load();
	Ogre::AliasTextureNamePairList alias_list;
	std::clog << "Trying to replace rtt_texture alias with " << texture_name << std::endl;
	alias_list["rtt_texture"] = texture_name;
	if(_fbo_material->applyTextureAliases(alias_list))
	{
		// @todo this should throw if unsuccesfull
		std::clog << "Succesfully replaced rtt_texture." << std::endl;
	}
	
	/// Create viewport, necessary for FBO
	/// @todo we should save the viewport to member data
	assert(!_fbo_viewport);
	_fbo_viewport = _fbo->addViewport((Ogre::Camera *)camera->getNative());
	assert(_fbo_viewport);
	_fbo_viewport->setBackgroundColour(_win_viewport->getBackgroundColour());
	// Needs the scene mask
	_fbo_viewport->setVisibilityMask(_get_fbo_mask());
	_fbo_viewport->setClearEveryFrame(true, Ogre::FBT_COLOUR|Ogre::FBT_DEPTH);

	// Implementation specifics for FBO rendering
	_fbo->setAutoUpdated (false);

	_create_screen_quad(name, material_name);

	// Set viewport attributes
	// @todo there is a possibility to use material scheme in the viewport
	// this could help us with deferred and forward rendering selection
	// Without setVisisbilityMask the second channel in Oculus gets copied from another FBO
	// with it only the first one gets copied from the original FBO
	_win_viewport->setVisibilityMask(_get_window_mask());
	_win_viewport->setShadowsEnabled(false);
	_win_viewport->setSkiesEnabled(false);
	_win_viewport->setOverlaysEnabled(false);

	// Create camera that is not movable for rendering 2D screens
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);
	std::string camera_name = getName() + "/rtt_camera";
	_rtt_camera = sm->createCamera(camera_name);
	_rtt_camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

	_win_viewport->setCamera(_rtt_camera);
}

