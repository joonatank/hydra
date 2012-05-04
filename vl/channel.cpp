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
		RENDER_MODE rm, uint32_t fsaa)
	: _name(config.name)
	, camera()
	, viewport(view)
	, _fbo(0)
	, _fsaa(fsaa)
	, _stereo_eye_cfg(HS_UNDEFINED)
	, _render_mode(rm)
	, _mrt(0)
{
	assert(viewport);
 
	viewport->setBackgroundColour(config.background_colour);
	viewport->setAutoUpdated(false);

	std::clog << "Channel::Channel : name " << _name << std::endl;
}

void
vl::Channel::_initialise_mrt(vl::CameraPtr camera)
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

	// Ogre::PF_FLOAT32_RGBA
	Ogre::RenderTexture *fbo0 = _create_fbo(camera, name + "/fbo_0", Ogre::PF_FLOAT32_RGBA);
	Ogre::RenderTexture *fbo1 = _create_fbo(camera, name + "/fbo_1", Ogre::PF_FLOAT32_RGBA);
	Ogre::RenderTexture *fbo2 = _create_fbo(camera, name + "/fbo_2", Ogre::PF_FLOAT32_RGBA);

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
 

	fbo0->setAutoUpdated(false);
	fbo1->setAutoUpdated(false);
	fbo2->setAutoUpdated(false);

	_mrt->bindSurface(0, fbo0);
	_mrt->bindSurface(1, fbo1);
	_mrt->bindSurface(2, fbo2);
	_mrt->setAutoUpdated(false);
	
	Ogre::Viewport *view = _mrt->addViewport((Ogre::Camera *)camera->getNative());
	// Overlays and Sky need to be rendered using forward renderer
	//view->setClearEveryFrame(true);
	view->setOverlaysEnabled(false);
	view->setSkiesEnabled(false);
	view->setAutoUpdated(false);
	// Needs the scene mask without lights
	view->setVisibilityMask(_get_scene_mask());
	view->setBackgroundColour(viewport->getBackgroundColour());
	view->setMaterialScheme("Deferred");

	// Set screen viewport attributes
	// Needs both screen and light mask because we are doing lighting
	// in this pass.
	viewport->setVisibilityMask(_get_window_mask() | _get_light_mask());
	// Overlays and Sky need to be rendered using forward renderer
	//viewport->setShadowsEnabled(false);
	viewport->setSkiesEnabled(true);
	viewport->setOverlaysEnabled(true);
	// @todo there is a possibility for selecting material scheme on the viewport

	// Create camera that is not movable for rendering 2D screens
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);	
	std::string camera_name = getName() + "/rtt_camera";
	_rtt_camera = sm->createCamera(camera_name);
	_rtt_camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

	viewport->setCamera(_rtt_camera);
}

/// nop if null parameter is passed here
void
vl::Channel::_set_fbo_camera(vl::CameraPtr camera)
{
	if(!camera)
	{ return; }

	/// FBO needs to be initialised here because we need Ogre::SceneManager for it
	if(!_fbo)
	{
		_initialise_fbo(camera);
	}
	else
	{
		_fbo->getViewport(0)->setCamera((Ogre::Camera *)camera->getNative());
	}
}

void
vl::Channel::_set_mrt_camera(vl::CameraPtr camera)
{
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

/// @todo this seems to be called at every frame
void
vl::Channel::setCamera(vl::CameraPtr cam)
{
	Ogre::Camera *og_cam = 0;
	if(cam)
	{ og_cam = (Ogre::Camera *)cam->getNative(); }

	// @todo viewport camera should only be set if we are rendering directly to screen
	// for now FBO does not have separate camera as it should
	// instead it is using view masking.
	camera.setCamera(cam);
	viewport->setCamera(og_cam);

	if(_render_mode == RM_FBO)
	{
		_set_fbo_camera(cam);
	}
	else if(_render_mode == RM_DEFERRED)
	{
		_set_mrt_camera(cam);
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

	if(_render_mode == RM_FBO)
	{ _render_to_fbo(); }
	else if(_render_mode == RM_DEFERRED)
	{
		_deferred_geometry_pass();
	}
}

void
vl::Channel::draw(void)
{
	// @todo this should be replaced with single inline function call
	// that hides the implementation (OpenGL) details
	//
	// Switching back buffers only works with HydraGL not Ogre RenderingSystem_GL
	// because Ogre does not correctly unbind framebuffers after using them.
	//
	// Switching backbuffers has no negative effects if quad buffering 
	// is not available they fallback to GL_BACK 
	// (and probably to GL_FRONT if ever we had no back buffer).
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
	if(_render_mode == RM_WINDOW)
	{ camera.update(_stereo_eye_cfg); }
	else if(_render_mode == RM_DEFERRED)
	{
		// Does not do anything at the moment
		// viewport->update() handles all that.
		// _deferred_light_pass();

		// @todo this is not proper or useful
		// we need to expose two custom matrix parameters
		// one for the shadow camera texture view projection matrix
		// one for the view matrix used for rendering (camera modelview)
		//_rtt_camera->setPosition(_player->getCamera()->getWorldPosition());
		//_rtt_camera->setOrientation(_player->getCamera()->getWorldOrientation());
	}

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
	// @todo should swap buffers be here or after _endUpdate?
	_fbo->swapBuffers();

	_fbo->_endUpdate();
}

void
vl::Channel::_deferred_geometry_pass(void)
{
	assert(_mrt);
	// @todo is there something special about this?
	_mrt->_beginUpdate();

	camera.update(_stereo_eye_cfg);
	_mrt->getViewport(0)->update(); //update();

	_mrt->_endUpdate();

	_mrt->swapBuffers();
}

void
vl::Channel::_deferred_light_pass(void)
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
}


void
vl::Channel::_initialise_fbo(vl::CameraPtr camera)
{
	std::clog << "vl::Channel::_initialise_fbo" << std::endl;

	std::string name("internal/" + getName());
	std::string material_name(name + "/fbo_material");
	std::string texture_name(name + "/rtt_tex");
	_fbo = _create_fbo(camera, name, Ogre::PF_R8G8B8);

	/* For some reason dynamically created material does not work
	_fbo_material = Ogre::MaterialManager::getSingleton().create(material_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	Ogre::Technique* matTechnique = _fbo_material->createTechnique();
	matTechnique->createPass();
	matTechnique->getPass(0)->setLightingEnabled(false);
	matTechnique->getPass(0)->createTextureUnitState(_fbo_texture->getName());
	*/

	// shouldn't be done for MRT
	Ogre::ResourcePtr base_mat_res = Ogre::MaterialManager::getSingleton().getByName("rtt");
	Ogre::MaterialPtr fbo_material = static_cast<Ogre::Material *>(base_mat_res.get())->clone(material_name);
	fbo_material->load();
	_fbo_materials.push_back(fbo_material);
	Ogre::AliasTextureNamePairList alias_list;
	std::clog << "Trying to replace diffuseTexture alias with " << texture_name << std::endl;
	alias_list["rtt_texture"] = texture_name;
	if(fbo_material->applyTextureAliases(alias_list))
	{
		std::clog << "Succesfully replaced diffuseTexture." << std::endl;
	}
	
	/// Create viewport, necessary for FBO
	Ogre::Viewport *view = _fbo->addViewport((Ogre::Camera *)camera->getNative());
	assert(view);
	view->setBackgroundColour(viewport->getBackgroundColour());
	// Needs the scene mask
	view->setVisibilityMask(_get_fbo_mask());
	view->setClearEveryFrame(true, Ogre::FBT_COLOUR|Ogre::FBT_DEPTH);

	// Implementation specifics for FBO rendering
	_fbo->setAutoUpdated (false);
	view->setAutoUpdated(false);

	_create_screen_quad(name, material_name);

	// Set viewport attributes
	// @todo there is a possibility to use material scheme in the viewport
	// this could help us with deferred and forward rendering selection
	viewport->setVisibilityMask(_get_window_mask());
	viewport->setShadowsEnabled(false);
	viewport->setSkiesEnabled(false);
	viewport->setOverlaysEnabled(false);

	// @todo we should use a separate default camera for FBO
	// not one with all the VIEW and PROJECTION modifications we use for
	// scene rendering.
}

Ogre::SceneNode *
vl::Channel::_create_screen_quad(std::string const &name, std::string const &material_name)
{
	Ogre::Rectangle2D *quad = new Ogre::Rectangle2D(true);
	quad->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
	//quad->setCorners(-1.0f, -1.0f, 1.0f, 1.0f);
	quad->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	quad->setVisibilityFlags(_get_window_mask());

	std::string sn_name = name + "/screen_quad";
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);
	Ogre::SceneNode* miniScreenNode = sm->getRootSceneNode()->createChildSceneNode(sn_name);
	miniScreenNode->attachObject(quad);

	quad->setMaterial(material_name);

	return miniScreenNode;
}

/// @todo add PixelFormat to parameters
Ogre::RenderTexture *
vl::Channel::_create_fbo(vl::CameraPtr camera, std::string const &name, Ogre::PixelFormat pf)
{
	std::string fbo_tex_name(name + "/rtt_tex");
	std::string material_name(name + "/fbo_material");

	// Texture size will be complete garbage till it's used
	// so do not check the texture size here. Use an OpenGL debugger instead.
	Ogre::TexturePtr fbo_texture = Ogre::TextureManager::getSingleton()
		.createManual(fbo_tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				Ogre::TEX_TYPE_2D, viewport->getActualWidth(), viewport->getActualHeight(), 
				0, pf, Ogre::TU_RENDERTARGET, 0, false, _fsaa);
	_fbo_textures.push_back(fbo_texture);

	Ogre::RenderTexture *fbo = fbo_texture->getBuffer()->getRenderTarget();

	return fbo;
}
