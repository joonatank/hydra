/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2014-11
 *	@file channel_deferred.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#include "channel_deferred.hpp"

/// Necessary for rendering to FBO
#include <OGRE/OgreRenderTexture.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgrePass.h>

#include <OGRE/OgreRoot.h>

vl::ChannelDeferred::ChannelDeferred(vl::config::Channel config,
	STEREO_EYE stereo_cfg, vl::Window *parent)
	: Channel(config, stereo_cfg, parent)
	, _mrt(0)
	, _rtt_camera(0)
{
}

vl::ChannelDeferred::~ChannelDeferred(void)
{
}

void
vl::ChannelDeferred::setCamera(vl::CameraPtr cam)
{
	Ogre::Camera *og_cam = 0;
	if(cam)
	{ og_cam = (Ogre::Camera *)cam->getNative(); }

	_camera.setCamera(cam);

	_set_mrt_camera(cam);
}

void
vl::ChannelDeferred::_predraw(void)
{
	assert(_quad_node);
	_quad_node->setVisible(true);

	_deferred_geometry_pass();
}

void
vl::ChannelDeferred::_draw(void)
{	
	// actually this isn't light pass per ce it only updates the shader uniforms
	// the actual light pass is automatically called by Viewport::update
	_deferred_light_pass();

	// draw to screen
	_win_viewport->getTarget()->_updateViewport(_win_viewport, true);

	assert(_quad_node);
	_quad_node->setVisible(false);
}

Ogre::Viewport *
vl::ChannelDeferred::getRenderViewport(void)
{
	/// @todo this is probably wrong, since we actually need the end result
	/// FBO for GUI/Oculus, but since Deferred isn't yet working with Oculus
	/// it's w/e.
	assert(_win_viewport);
	return _win_viewport;
}

vl::scalar
vl::ChannelDeferred::getLastFPS(void) const
{
	assert(_mrt);
	return _mrt->getLastFPS();
}

size_t
vl::ChannelDeferred::getTriangleCount(void) const
{
	assert(_mrt);
	return _mrt->getTriangleCount();
}

size_t
vl::ChannelDeferred::getBatchCount(void) const
{
	assert(_mrt);
	return _mrt->getBatchCount();
}
	
void
vl::ChannelDeferred::resetStatistics(void)
{
	assert(_mrt);
	_mrt->resetStatistics();
}


/// -------------------------- Private ---------------------------------------

void
vl::ChannelDeferred::_initialise_mrt(vl::CameraPtr camera)
{
	assert(camera);
	assert(camera->getNative());

	// Create FBOs for G-buffer
	// For testing we are using 3 x 128 bit g-buffer
	// yes it's huge and unnecessary
	// 32-bit colours are mighty wasteful
	// | Pos.x	| Pos.y		| Pos.z		| Spec	|
	// | Norm.x | Norm.y	| Norm.z	| Power |
	// | Diff.x | Diff.y	| Diff.z	| GA	|
	// Where Pos is position vector
	// Norm is normal vector
	// Diff is the diffuse colour of the material
	// Spec is the specular component (grey scale)
	// Power is the specular power
	// GA is the global ambient (grey scale)
	// Memory requirements should be 2.3Mbytes for 1024x768
	// and 95Mbytes for Full HD
	// For stereo the scene is rendered again so the memory
	// requirements are the same.
	

	std::string name(getName() + "/MRT");

	for(size_t i = 0; i < 3; ++i)
	{
		std::stringstream ss;
		ss << name << "/fbo_" << i;
		std::clog << "Creating FBO : " << ss.str() << std::endl;
		Ogre::TexturePtr fbo_tex = _create_fbo(ss.str(), Ogre::PF_FLOAT32_RGBA);
		_fbo_textures.push_back(fbo_tex);
		Ogre::RenderTexture *fbo = fbo_tex->getBuffer()->getRenderTarget();
		
		fbo->setAutoUpdated(false);
	}

	// Using a material defined in the deferred_shading materials
	// basic lighting material that iterates over all lights
	// and reads the g-buffer.
	std::string base_material_name("deferred_shading/light_pass");
	std::string material_name(base_material_name + std::string("/") + getName());

	// Create the MRT material 
	Ogre::ResourcePtr base_mat_res = Ogre::MaterialManager::getSingleton().getByName(base_material_name);
	Ogre::MaterialPtr fbo_material = static_cast<Ogre::Material *>(base_mat_res.get())->clone(material_name);
	fbo_material->load();
	_fbo_materials.push_back(fbo_material);
	Ogre::AliasTextureNamePairList alias_list;
	alias_list["fbo0"] = name + "/fbo_0/rtt_tex";
	alias_list["fbo1"] = name + "/fbo_1/rtt_tex";
	alias_list["fbo2"] = name + "/fbo_2/rtt_tex";
	if(fbo_material->applyTextureAliases(alias_list))
	{
		std::clog << "MRT : Succesfully replaced fbos." << std::endl;
	}

	// Where we really render
	_create_screen_quad(name, material_name);

	// Quite easy
	// We need to create all the FBOs before this and bind them to the MRT
	_mrt = Ogre::Root::getSingleton().getRenderSystem()->createMultiRenderTarget(name);

	for(size_t i = 0; i < 3; ++i)
	{
		Ogre::TexturePtr fbo_tex = _fbo_textures.at(i);
		Ogre::RenderTexture *fbo = fbo_tex->getBuffer()->getRenderTarget();
		_mrt->bindSurface(i, fbo);
	}

	_mrt->setAutoUpdated(false);
	
	Ogre::Viewport *view = _mrt->addViewport((Ogre::Camera *)camera->getNative());
	// Overlays and Sky need to be rendered using forward renderer
	// Also transparent objects need to be rendered using forward renderer
	// but for those we need to use material schemes or something.
	// Shadows are rendered here because we need the forward renderer for them.
	// Ogre's shadow system needs all lights and objects to be present
	// for the shadow maps to be correct.
	// Using custom shadow map rendering would solve this issue but
	// it's an optimisation not a functionality issue.
	view->setShadowsEnabled(true);
	view->setOverlaysEnabled(false);
	view->setSkiesEnabled(false);
	// Needs the scene mask without lights
	// With lights till we have custom shadow mapping that is rendered
	// before the geometric pass.
	view->setVisibilityMask(_get_scene_mask() | _get_light_mask());
	view->setBackgroundColour(_win_viewport->getBackgroundColour());
	view->setMaterialScheme("Deferred");

	// Set screen viewport attributes
	// Needs both screen and light mask because we are doing lighting
	// in this pass.
	_win_viewport->setVisibilityMask(_get_window_mask() | _get_light_mask());
	// Overlays and Sky need to be rendered using forward renderer
	// Overlays are working nicely
	// @todo skies are not working
	_win_viewport->setShadowsEnabled(false);
	_win_viewport->setSkiesEnabled(true);
	_win_viewport->setOverlaysEnabled(true);

	// Create camera that is not movable for rendering 2D screens
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);	
	std::string camera_name = getName() + "/rtt_camera";
	_rtt_camera = sm->createCamera(camera_name);
	_rtt_camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

	_win_viewport->setCamera(_rtt_camera);
}


void
vl::ChannelDeferred::_set_mrt_camera(vl::CameraPtr camera)
{
	/// @todo useless checking
	if(!camera)
	{ return; }

	if(!_mrt)
	{
		_initialise_mrt(camera);
	}
	else
	{
		Ogre::Camera * og_cam = (Ogre::Camera *)camera->getNative();
		_mrt->getViewport(0)->setCamera(og_cam);
	}
}

void
vl::ChannelDeferred::_deferred_geometry_pass(void)
{
	assert(_mrt);

	_update_camera();

	_mrt->update();
}

void
vl::ChannelDeferred::_deferred_light_pass(void)
{
	// This should only render every light with a special shape
	// so we can clip as many fragments as possible
	// For now this optimisation is not implemented so we use
	// a fullscreen quad for rendering once for every light
	
	// We use our screen quad as it already has the correct visibility mask
	// the only difference to FBO rendering here is that we
	// don't use a texture for the quad but rather a material that
	// has lighting enabled on it (well special deferred shading material).

	// Don't think there is any need for this function... at least without
	// the light shape optimisation.

	assert(_fbo_materials.size() > 0);
	Ogre::Technique *tech = _fbo_materials.at(0)->getBestTechnique();
	// First pass is the ambient pass
	Ogre::Pass *pass = tech->getPass(1);
	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
	// @todo does this take into account IPD?
	Ogre::Vector3 const &pos =_camera.getCamera()->getWorldPosition();
	params->setNamedConstant("g_camera_position", Ogre::Vector4(pos.x, pos.y, pos.z, 1));
}
