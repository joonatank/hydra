/**
 *	Copyright (c) 2011-2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file channel.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "channel.hpp"
/// Necessary for retrieving Ogre SceneManager
#include "ogre_root.hpp"
/// Necessary for copying frustum attributes
#include "player.hpp"

#include "camera.hpp"

/// Necessary for rendering to FBO
#include <OGRE/OgreRenderTexture.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgrePass.h>
// Necessary for glDrawBuffer
#include <GL/gl.h>

/// ---------------------------------- Public --------------------------------
vl::Channel::Channel(vl::config::Channel config, Ogre::Viewport *view, 
		bool use_fbo, uint32_t fsaa)
	: _name(config.name)
	, camera()
	, viewport(view)
	, _fbo(0)
	, _use_fbo(use_fbo)
	, _fsaa(fsaa)
	, _stereo_eye_cfg(HS_UNDEFINED)
{
	assert(viewport);
 
	viewport->setBackgroundColour(config.background_colour);
	viewport->setAutoUpdated(false);

	std::clog << "Channel::Channel : name " << _name << std::endl;
}

/// @todo this seems to be called at every frame
void
vl::Channel::setCamera(vl::CameraPtr cam)
{
	Ogre::Camera *og_cam = 0;
	if(cam)
	{ og_cam = (Ogre::Camera *)cam->getNative(); }

	camera.setCamera(cam);
	viewport->setCamera(og_cam);

	if(_use_fbo && cam)
	{
		/// FBO needs to be initialised here because we need Ogre::SceneManager for it
		if(!_fbo)
		{
			_initialise_fbo(cam);
		}
		else
		{
			_fbo->getViewport(0)->setCamera(og_cam);
		}
	}
}

void
vl::Channel::update(void)
{
	// It's possible that update is called without Player beign set
	// just return then, we are assuming that the engine will set
	// valid player when one is available.
	if(!_player)
	{ return; }

	camera.setHead(_player->getHeadTransform());

	/// @todo these shouldn't be copied at every frame use the distribution
	/// system to distribute the Frustum.
	camera.getFrustum().setHeadTransformation(_player->getCyclopWorldTransform());
	camera.setIPD(_player->getIPD());

	if(_fbo)
	{ _render_to_fbo(); }
}


void
vl::Channel::draw(void)
{
	// @todo this should be replaced with single inline function call
	// that hides the implementation (OpenGL) details
	// @todo this does not work when using FBO for rendering
	// My assumption would be that the FBO is still bound when we try to switch
	// draw buffers. OpenGL does not allow switching BACK buffers with FBOs
	// naturally.
	// HydraGL has been fixed for this and debugger gives no errors for this.
	// Stereo needs to be tested still.
	if(_stereo_eye_cfg == HS_LEFT)
	{
		glDrawBuffer(GL_BACK_LEFT);
	}
	else if(_stereo_eye_cfg == HS_RIGHT)
	{
		glDrawBuffer(GL_BACK_RIGHT);
	}
	else
	{
		// This needs to be set because we want to be able to switch
		// stereo mode on/off
		// Draw both back buffers
		glDrawBuffer(GL_BACK);
	}

	assert(camera.getCamera());
	
	// Camera only needs to be updated if we are rendering directly to screen.
	if(!_use_fbo)
	{ camera.update(_stereo_eye_cfg); }

	viewport->update();
}


vl::scalar
vl::Channel::getLastFPS(void) const
{
	if(_fbo)
	{ return _fbo->getLastFPS(); }
	else
	{
		assert(viewport->getTarget());
		return viewport->getTarget()->getLastFPS();
	}
}

size_t
vl::Channel::getTriangleCount(void) const
{
	if(_fbo)
	{ return _fbo->getTriangleCount(); }
	else
	{
		assert(viewport->getTarget());
		return viewport->getTarget()->getTriangleCount();
	}
}

size_t
vl::Channel::getBatchCount(void) const
{

	if(_fbo)
	{ return _fbo->getBatchCount(); }
	else
	{
		assert(viewport->getTarget());
		return viewport->getTarget()->getBatchCount();
	}
}

/// ---------------------------------- Private -------------------------------
void
vl::Channel::_render_to_fbo(void)
{
	// @todo when this is working with stereo we can replace with auto updating
	// the whole FBO
	// remove _begin and _end update, viewport updating and separate swapBuffers.
	_fbo->_beginUpdate();
	assert(camera.getCamera());
	assert(_fbo->getViewport(0));

	camera.update(_stereo_eye_cfg);
	_fbo->getViewport(0)->update();
	_fbo->swapBuffers();

	_fbo->_endUpdate();
}

void
vl::Channel::_initialise_fbo(vl::CameraPtr camera)
{
	std::clog << "vl::Channel::_initialise_fbo" << std::endl;

	// Texture size will be complete garbage till it's used
	// so do not check the texture size here. Use a OpenGL debugger instead.
	std::string fbo_tex_name("internal/" + getName() + "/rtt_tex");
	_fbo_texture = Ogre::TextureManager::getSingleton()
		.createManual(fbo_tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				Ogre::TEX_TYPE_2D, viewport->getActualWidth(), viewport->getActualHeight(), 
				0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET, 0, false, _fsaa);

	_fbo = _fbo_texture->getBuffer()->getRenderTarget();
	_fbo->setAutoUpdated (false);

	std::string material_name = "internal/" + getName() + "/fbo_material";
	/* For some reason dynamically created material does not work
	_fbo_material = Ogre::MaterialManager::getSingleton().create(material_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* matTechnique = _fbo_material->createTechnique();
	matTechnique->createPass();
	matTechnique->getPass(0)->setLightingEnabled(false);
	matTechnique->getPass(0)->createTextureUnitState(_fbo_texture->getName());
	*/
	Ogre::ResourcePtr base_mat_res = Ogre::MaterialManager::getSingleton().getByName("rtt");
	_fbo_material = static_cast<Ogre::Material *>(base_mat_res.get())->clone(material_name);
	Ogre::AliasTextureNamePairList alias_list;
	std::clog << "Trying to replace diffuseTexture alias with " << _fbo_texture->getName() << std::endl;
	alias_list["rtt_texture"] = _fbo_texture->getName();
	if(_fbo_material->applyTextureAliases(alias_list))
	{
		std::clog << "Succesfully replaced diffuseTexture." << std::endl;
	}
	
	/// The channel will not be rendering the Scene
	// We are using z order to mask different viewports
	if(viewport->getZOrder() + 1 >= 32)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	uint32_t window_mask = 1 << (viewport->getZOrder()+1);
	uint32_t fbo_mask = 1;
	if(window_mask == fbo_mask)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	viewport->setVisibilityMask(window_mask);
	viewport->setShadowsEnabled(false);
	viewport->setSkiesEnabled(false);
	viewport->setOverlaysEnabled(false);

	assert(camera);
	assert(camera->getNative());
	assert(_fbo);

	_fbo->addViewport((Ogre::Camera *)camera->getNative());
	_fbo->getViewport(0)->setBackgroundColour(viewport->getBackgroundColour());
	_fbo->getViewport(0)->setVisibilityMask(fbo_mask);
	_fbo->getViewport(0)->setAutoUpdated(false);

	Ogre::Rectangle2D *quad = new Ogre::Rectangle2D(true);
	quad->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
	quad->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	quad->setVisibilityFlags(window_mask);

	std::string sn_name = "internal/" + getName() + "/fbo_screen";
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);
	Ogre::SceneNode* miniScreenNode = sm->getRootSceneNode()->createChildSceneNode(sn_name);
	miniScreenNode->attachObject(quad);

	quad->setMaterial(_fbo_material->getName());

	// @todo we should use a separate default camera for FBO
	// not one with all the VIEW and PROJECTION modifications we use for
	// scene rendering.
}
