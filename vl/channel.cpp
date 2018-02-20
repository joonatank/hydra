/**
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-11
 *	@file channel.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 */

#include "channel.hpp"
/// Necessary for retrieving Ogre SceneManager
#include "ogre_root.hpp"
/// Necessary for copying frustum attributes
#include "player.hpp"

#include "camera.hpp"
// Parent
#include "window.hpp"

/// necessary for retreiving Player when rendering
#include "renderer.hpp"
#include "pipe.hpp"

/// Necessary for rendering to FBO
#include <OGRE/OgreHardwarePixelBuffer.h>
// Necessary for glDrawBuffer
#include <GL/gl.h>

/// ---------------------------------- Public --------------------------------
vl::Channel::Channel(vl::config::Channel config,
		STEREO_EYE stereo_cfg, vl::Window *parent)
	: _name(config.name)
	, _camera()
	, _texture_size(config.texture_size)
	, _win_viewport(0)
	, _fsaa(config.fsaa)
	, _stereo_eye(0.0)
	, _quad_node(0)
	, _draw_buffer(GL_BACK)
	, _use_custom_view(false)
	, _use_custom_proj(false)
	, _custom_view()
	, _custom_proj()
	, _parent(parent)
{
	/// @todo should we autogenerate the name if it's not valid?
	if(_name.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Channel has no name")); }

	assert(_parent);

	/// We don't yet have a valid SceneManager, so we don't have valid camera here
	vl::Rect<double> const &rect = config.area;
	assert(rect.valid());
	size_t n = _parent->getChannels().size();
	_win_viewport = _parent->getNative()->addViewport(0, n, rect.x, rect.y, rect.w, rect.h);

	Ogre::ColourValue col(config.background_colour.r, config.background_colour.g,
		config.background_colour.b, config.background_colour.a);
	_win_viewport->setBackgroundColour(col);
	_win_viewport->setAutoUpdated(false);
	_size = config.area;

	std::clog << "Channel::Channel : name " << _name << std::endl;

	/// Configure Frustum
	
	_camera.getFrustum().enableAsymmetricStereoFrustum(config.projection.use_asymmetric_stereo);
	/// @todo these can be removed when we have checked that this config is working
	if(_camera.getFrustum().isAsymmetricStereoFrustum())
	{ std::clog << "Using asymmetric stereo frustum." << std::endl; }
	else
	{ std::clog << "NOT Using asymmetric stereo frustum." << std::endl; }

	_camera.getFrustum().setWall(config.projection.wall);
	_camera.getFrustum().setFov(Ogre::Degree(config.projection.fov));

	// Old stuff for Oculus, to be removed
	assert(config.projection.perspective_type != vl::config::Projection::USER);
	if(config.projection.perspective_type == vl::config::Projection::FOV)
	{
		std::clog << "Setting channel " << getName() << " to use FOV frustum." << std::endl;
		_camera.getFrustum().setType(Frustum::FOV);
	}
	else
	{
		std::clog << "Setting channel " << getName() << " to use Wall frustum." << std::endl;
		_camera.getFrustum().setType(Frustum::WALL);
	}

	/// Need to modify the rendering to include head orientation in view matrix

	/// Select draw buffer
	//
	// Switching draw buffer only works with HydraGL not Ogre RenderingSystem_GL
	// because Ogre does not correctly unbind framebuffers after using them.
	//
	// Switching backbuffers has no negative effects if quad buffering 
	// is not available they fallback to GL_BACK 
	// (and probably to GL_FRONT if ever we had no back buffer).
	if(stereo_cfg == HS_LEFT)
	{
		_draw_buffer = GL_BACK_LEFT;
		_stereo_eye = -1.0;
	}
	else if(stereo_cfg == HS_RIGHT)
	{
		_draw_buffer = GL_BACK_RIGHT;
		_stereo_eye = 1.0;
	}
	else
	{
		// This needs to be set because we want to be able to switch
		// stereo mode on/off
		// Draw both back buffers
		_draw_buffer = GL_BACK;
	}
}

vl::Channel::~Channel(void)
{
}

Ogre::Viewport *
vl::Channel::getRenderViewport(void)
{
	assert(_win_viewport);
	return _win_viewport;
}

/// @todo this seems to be called at every frame
void
vl::Channel::setCamera(vl::CameraPtr cam)
{
	Ogre::Camera *og_cam = 0;
	if(cam)
	{ og_cam = (Ogre::Camera *)cam->getNative(); }

	_camera.setCamera(cam);

	_win_viewport->setCamera(og_cam);
}

void
vl::Channel::update(void)
{
	_copy_player();

	_predraw();
}

/// @todo some of the code here is common to all implementations
/// we should divide it with template method pattern and use
/// one special function for the draw command.
void
vl::Channel::draw(void)
{
	glDrawBuffer(_draw_buffer);

	_draw();
}

void
vl::Channel::_draw(void)
{
	// update camera handles drawing automatically for direct window rendering
	_update_camera();
	
	// Must be using this stupid syntax for
	// keeping track of the batches and triangles
	_win_viewport->getTarget()->_updateViewport(_win_viewport, true);
}

vl::scalar
vl::Channel::getLastFPS(void) const
{
	assert(_win_viewport->getTarget());
	return _win_viewport->getTarget()->getLastFPS();
}

size_t
vl::Channel::getTriangleCount(void) const
{
	assert(_win_viewport->getTarget());
	return _win_viewport->getTarget()->getTriangleCount();
}

size_t
vl::Channel::getBatchCount(void) const
{
	/// Deferred rendering needs both viewport and mrt
	/// Because skies and overlays are not deferred
	///
	/// FBO needs almost exculisively the FBO stats
	/// Because only one quad is rendered outside of the FBO

	assert(_win_viewport->getTarget());
	return _win_viewport->getTarget()->getBatchCount();
}

void
vl::Channel::resetStatistics(void)
{
	assert(_win_viewport->getTarget());
	_win_viewport->getTarget()->resetStatistics();
}

void
vl::Channel::setCustomProjMatrix(bool use, vl::Matrix4 const proj)
{
	_use_custom_proj = use;
	_custom_proj = proj;
}

void
vl::Channel::setCustomViewMatrix(bool use, vl::Matrix4 const view)
{
	_use_custom_view = use;
	_custom_view = view;
}

/// ---------------------------------- Private -------------------------------
void
vl::Channel::_update_camera()
{
	// dirty branching to handle Oculus
	if(_use_custom_proj && _use_custom_view)
	{
		_camera.update(_custom_view, _custom_proj);
	}
	else
	{
		// checking since we can't allow one of them to be overriden
		assert(!_use_custom_proj && !_use_custom_view);
		_camera.update(_stereo_eye);
	}
}

void
vl::Channel::_copy_player(void)
{
	assert(_parent && _parent->getPipe() && _parent->getPipe()->getRenderer());
	Player *player = _parent->getPipe()->getRenderer()->getPlayer();
	assert(player);

	_camera.setHead(player->getHeadTransform());

	/// @todo these shouldn't be copied at every frame use the distribution
	/// system to distribute the stereo camera.
	/// This would actually need the StereoCamera to replace our Camera.
	_camera.setIPD(player->getIPD());
}

void
vl::Channel::_create_screen_quad(std::string const &name, std::string const &material_name)
{
	Ogre::Rectangle2D *quad = new Ogre::Rectangle2D(true);
	quad->setCorners(-1.0f, 1.0f, 1.0f, -1.0f);
	quad->setBoundingBox(Ogre::AxisAlignedBox(-100000.0f * Ogre::Vector3::UNIT_SCALE, 100000.0f * Ogre::Vector3::UNIT_SCALE));
	quad->setVisibilityFlags(_get_window_mask());

	std::string sn_name = name + "/screen_quad";
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);
	_quad_node = sm->getRootSceneNode()->createChildSceneNode(sn_name);
	_quad_node->attachObject(quad);

	quad->setMaterial(material_name);
}

Ogre::TexturePtr
vl::Channel::_create_fbo(std::string const &name, Ogre::PixelFormat pf)
{
	std::string fbo_tex_name(name + "/rtt_tex");
	std::string material_name(name + "/fbo_material");

	// Texture size will be complete garbage till it's used
	// so do not check the texture size here. Use an OpenGL debugger instead.
	//
	// Use viewport size if no size was set
	int w = _win_viewport->getActualWidth();
	int h = _win_viewport->getActualHeight();
	if(_texture_size.x > 0 && _texture_size.y > 0)
	{ w = _texture_size.x; h = _texture_size.y; }

	Ogre::TexturePtr fbo_texture = Ogre::TextureManager::getSingleton()
		.createManual(fbo_tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				Ogre::TEX_TYPE_2D, w, h, 0, pf, Ogre::TU_RENDERTARGET, 0, false, _fsaa);

	return fbo_texture;
}
