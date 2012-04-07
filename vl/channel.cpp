/**
 *	Copyright (c) 2012 Savant Simulators
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
// Necessary for glDrawBuffer
#include <GL/gl.h>

/// ---------------------------------- Public --------------------------------
vl::Channel::Channel(vl::config::Channel config, Ogre::Viewport *view, bool use_fbo)
	: _name(config.name)
	, camera()
	, viewport(view)
	, _fbo(0)
{
	assert(viewport);

	std::clog << "Channel::Channel" << std::endl;
	if(use_fbo)
	{
		assert(viewport->getActualWidth() > 0);
		assert(viewport->getActualHeight() > 0);
		std::clog << "Channel::Channel : FBO of size : " << viewport->getActualWidth() << "x" 
			<< viewport->getActualHeight() << std::endl;
		std::string name(getName() + "_rtt_text");
		_fbo_texture = Ogre::TextureManager::getSingleton()
			.createManual(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
					Ogre::TEX_TYPE_2D, viewport->getActualWidth(), viewport->getActualHeight(), 
					0, Ogre::PF_R8G8B8, Ogre::TU_RENDERTARGET);

		std::clog << "Channel::Channel : FBO texture created." << std::endl;

		_fbo = _fbo_texture->getBuffer()->getRenderTarget();

		std::clog << "Channel::Channel : setting viewport settings" << std::endl;
		/// The channel will not be rendering the Scene
		viewport->setVisibilityMask(1<<1);
		viewport->setShadowsEnabled(false);
		viewport->setSkiesEnabled(false);
		viewport->setOverlaysEnabled(false);
	}
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

	if(_fbo && cam)
	{
		if(_fbo->getNumViewports() == 0)
		{
			_initialise_fbo(cam);
		}
		else
		{
			if(cam)
			{
				_fbo->getViewport(0)->setCamera(og_cam);
			}
		}
	}
}

void
vl::Channel::update(vl::Player const &player)
{
	camera.setHead(player.getHeadTransform());

	/// @todo these shouldn't be copied at every frame
	/// use the distribution system to distribute
	/// the Frustum.
	camera.getFrustum().setHeadTransformation(player.getCyclopWorldTransform());
	camera.getFrustum().enableHeadFrustum(player.isHeadFrustumX(), player.isHeadFrustumY(), player.isHeadFrustumZ());
	camera.getFrustum().enableAsymmetricStereoFrustum(player.isAsymmetricStereoFrustum());

	if(_fbo)
	{ _render_to_fbo(); }
}


void
vl::Channel::draw(double ipd, bool eye_left )
{
	// @todo this is messy
	// replace by removing the draw method
	// and passing Channel to RenderTargetListener
	if(ipd != 0)
	{
		if(eye_left)
		{ glDrawBuffer(GL_BACK_LEFT); }
		else
		{ glDrawBuffer(GL_BACK_RIGHT); }
	}

	assert(camera.getCamera());
	camera.update(ipd);
	// @todo this needs to toggle the RenderingTarget listener for stereo
	// Render the window as many times as there is buffers
	// once for mono viewing and twice for stereo

	viewport->update();
}

/// ---------------------------------- Private -------------------------------
void
vl::Channel::_render_to_fbo(void)
{
	assert(camera.getCamera());
	assert(_fbo->getViewport(0));

	// for now hard coded to zero ipd because we need right and left fbo
	// for stereo rendering.
	camera.update(0);
	_fbo->update(false);
}

void
vl::Channel::_initialise_fbo(vl::CameraPtr camera)
{
	assert(camera);
	assert(_fbo);

	_fbo->addViewport((Ogre::Camera *)camera->getNative());
	_fbo->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
	_fbo->getViewport(0)->setOverlaysEnabled(false);
	_fbo->getViewport(0)->setVisibilityMask(1);

	Ogre::Rectangle2D *quad = new Ogre::Rectangle2D(true);
	quad->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
	quad->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	quad->setVisibilityFlags(1<<1);

	std::string sn_name = "internal/" + getName() + "/fbo_screen";
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);
	Ogre::SceneNode* miniScreenNode = sm->getRootSceneNode()->createChildSceneNode(sn_name);
	miniScreenNode->attachObject(quad);

	std::string material_name = "internal/" + getName() + "/fbo_material";
	Ogre::ResourcePtr base_mat_res = Ogre::MaterialManager::getSingleton().getByName("rtt");
	_fbo_material = static_cast<Ogre::Material *>(base_mat_res.get())->clone(material_name);
	Ogre::AliasTextureNamePairList alias_list;
	std::clog << "Trying to replace diffuseTexture alias with " << _fbo_texture->getName() << std::endl;
	alias_list["rtt_texture"] = _fbo_texture->getName();
	if(_fbo_material->applyTextureAliases(alias_list))
	{
		std::clog << "Succesfully replaced diffuseTexture." << std::endl;
	}

	quad->setMaterial(material_name);
}
