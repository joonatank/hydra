/**
 *	Copyright (c) 2011-2013 Savant Simulators
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
#include <OGRE/OgreRenderTexture.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreSceneNode.h>
#include <OGRE/OgreHardwarePixelBuffer.h>
#include <OGRE/OgreMaterial.h>
#include <OGRE/OgreTechnique.h>
#include <OGRE/OgrePass.h>
// Necessary for glDrawBuffer
#include <GL/gl.h>

#include <HydraGL/src/OgreGLFrameBufferObject.h>

/// ---------------------------------- Public --------------------------------
vl::Channel::Channel(vl::config::Channel config, Ogre::Viewport *view, 
		RENDER_MODE rm, uint32_t fsaa, STEREO_EYE stereo_cfg, vl::Window *parent)
	: _name(config.name)
	, _camera()
	, _viewport(view)
	, _fbo(0)
	, _fsaa(fsaa)
	, _stereo_eye(0.0)
	, _render_mode(rm)
	//, _player(0)
	, _mrt(0)
	, _quad_node(0)
	, _draw_buffer(GL_BACK)
	, _use_custom_view(false)
	, _use_custom_proj(false)
	, _custom_view()
	, _custom_proj()
	, _parent(parent)
{
	if(_name.empty())
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Channel has no name")); }

	assert(_viewport);
	assert(_parent);

	Ogre::ColourValue col(config.background_colour.r, config.background_colour.g,
		config.background_colour.b, config.background_colour.a);
	_viewport->setBackgroundColour(col);
	_viewport->setAutoUpdated(false);
	_size = config.area;

	std::clog << "Channel::Channel : name " << _name 
		<< " with render mode " << getRenderModeName(_render_mode) << std::endl;

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
	Ogre::RenderTexture *fbo0 = _create_fbo(name + "/fbo_0", Ogre::PF_FLOAT32_RGBA);
	Ogre::RenderTexture *fbo1 = _create_fbo(name + "/fbo_1", Ogre::PF_FLOAT32_RGBA);
	Ogre::RenderTexture *fbo2 = _create_fbo(name + "/fbo_2", Ogre::PF_FLOAT32_RGBA);

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
	view->setBackgroundColour(_viewport->getBackgroundColour());
	view->setMaterialScheme("Deferred");

	// Set screen viewport attributes
	// Needs both screen and light mask because we are doing lighting
	// in this pass.
	_viewport->setVisibilityMask(_get_window_mask() | _get_light_mask());
	// Overlays and Sky need to be rendered using forward renderer
	// Overlays are working nicely
	// @todo skies are not working
	_viewport->setShadowsEnabled(false);
	_viewport->setSkiesEnabled(true);
	_viewport->setOverlaysEnabled(true);

	// Create camera that is not movable for rendering 2D screens
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);	
	std::string camera_name = getName() + "/rtt_camera";
	_rtt_camera = sm->createCamera(camera_name);
	_rtt_camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

	_viewport->setCamera(_rtt_camera);
}

void
vl::Channel::_set_fbo_camera(vl::CameraPtr camera, std::string const &base_material)
{
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

	_camera.setCamera(cam);

	// @todo viewport camera should only be set if we are rendering directly to screen
	// for now FBO does not have separate camera as it should
	// instead it is using view masking.
	if(_render_mode == RM_WINDOW)
	{
		_viewport->setCamera(og_cam);
	}
	else if(_render_mode == RM_FBO || _render_mode == RM_OCULUS)
	{
		_set_fbo_camera(cam, "rtt");
	}
	else if(_render_mode == RM_DEFERRED)
	{
		_set_mrt_camera(cam);
	}
}

void
vl::Channel::update(void)
{
	// @todo retrieve Player from Renderer
	assert(_parent && _parent->getPipe() && _parent->getPipe()->getRenderer());
	Player *player = _parent->getPipe()->getRenderer()->getPlayer();
	assert(player);

	_camera.setHead(player->getHeadTransform());

	/// @todo these shouldn't be copied at every frame use the distribution
	/// system to distribute the stereo camera.
	/// This would actually need the StereoCamera to replace our Camera.
	_camera.setIPD(player->getIPD());

	if(_quad_node)
	{ _quad_node->setVisible(true); }

	// doesn't make a difference (well not noticable anyway)
	//_viewport->clear();
	if(_render_mode == RM_FBO || _render_mode == RM_OCULUS)
	{ _render_to_fbo(); }
	else if(_render_mode == RM_DEFERRED)
	{ _deferred_geometry_pass(); }
}

void
vl::Channel::draw(void)
{
	// Clearing a viewport here fucks up the FBO rendering (no kidding)
	
	glDrawBuffer(_draw_buffer);

	// Camera only needs to be updated if we are rendering directly to screen.
	if(_render_mode == RM_WINDOW)
	{ _update_camera(); }
	else if(_render_mode == RM_DEFERRED)
	{
		// actually this isn't light pass per ce it only updates the shader uniforms
		// the actual light pass is automatically called by Viewport::update
		_deferred_light_pass();
	}

	// Must be using this stupid syntax for
	// keeping track of the batches and triangles
	_viewport->getTarget()->_updateViewport(_viewport, true);

	if(_quad_node)
	{ _quad_node->setVisible(false); }
}

vl::scalar
vl::Channel::getLastFPS(void) const
{
	if(_fbo)
	{ return _fbo->getLastFPS(); }
	else if(_mrt)

	{ return _mrt->getLastFPS(); }
	else
	{
		assert(_viewport->getTarget());
		return _viewport->getTarget()->getLastFPS();
	}
}

size_t
vl::Channel::getTriangleCount(void) const
{
	switch(_render_mode)
	{
		case RM_FBO:
		case RM_OCULUS:
			assert(_fbo);
			return _fbo->getTriangleCount();
		case RM_DEFERRED: 
			assert(_mrt);
			return _mrt->getTriangleCount();
		default:
			return 0;
	}
}

size_t
vl::Channel::getBatchCount(void) const
{
	/// Deferred rendering needs both viewport and mrt
	/// Because skies and overlays are not deferred
	///
	/// FBO needs almost exculisively the FBO stats
	/// Because only one quad is rendered outside of the FBO
	switch(_render_mode)
	{
		case RM_FBO:
		case RM_OCULUS:
			assert(_fbo);
			return _fbo->getBatchCount();
		case RM_DEFERRED: 
			assert(_mrt);
			return _mrt->getBatchCount();
		default:
			return 0;
	}
}

void
vl::Channel::resetStatistics(void)
{
	switch(_render_mode)
	{
		case RM_FBO:
		case RM_OCULUS :
			if(_fbo)
			{ _fbo->resetStatistics(); }
			break;
		case RM_DEFERRED: 
			if(_mrt)
			{ _mrt->resetStatistics(); }
			break;
	}
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

uint32_t
vl::Channel::getTextureID(void) const
{
	// @fixme this is bad, should work for FBO targets, but not Deferred
	assert(_fbo_textures.size() == 1);

	Ogre::GLTexture *tex = dynamic_cast<Ogre::GLTexture *>(_fbo_textures.at(0).get());
	GLuint id;
	tex->getCustomAttribute("GLID", &id);
	return id;
}

uint32_t
vl::Channel::getFBOID(void) const
{
	assert(_fbo);

	GLuint fbo = 0;
	_fbo->getCustomAttribute("GL_FBOID", &fbo);
	
	return fbo;
}

vl::Rect<uint32_t>
vl::Channel::getTextureSize(void) const
{
	assert(_fbo);
	Rect<uint32_t> size;
	uint32_t depth;
	_fbo->getMetrics(size.w, size.h, depth);
	return size;
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
vl::Channel::_render_to_fbo(void)
{
	assert(_fbo);

	// beginUpdate and endUpdate screws the GUI and makes the
	// second window black in both windowed and FBO modes
	// without them though FBO bleeds to second channel
	//_fbo->_beginUpdate();
	_viewport->clear();
	_update_camera();
	_fbo->update();
}

void
vl::Channel::_deferred_geometry_pass(void)
{
	assert(_mrt);

	_update_camera();

	_mrt->update();
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

	assert(_fbo_materials.size() > 0);
	Ogre::Technique *tech = _fbo_materials.at(0)->getBestTechnique();
	// First pass is the ambient pass
	Ogre::Pass *pass = tech->getPass(1);
	Ogre::GpuProgramParametersSharedPtr params = pass->getFragmentProgramParameters();
	// @todo does this take into account IPD?
	Ogre::Vector3 const &pos =_camera.getCamera()->getWorldPosition();
	params->setNamedConstant("g_camera_position", Ogre::Vector4(pos.x, pos.y, pos.z, 1));
}


void
vl::Channel::_initialise_fbo(vl::CameraPtr camera, std::string const &base_material)
{
	std::clog << "vl::Channel::_initialise_fbo : with material " << base_material << std::endl;

	std::string name("internal/" + getName());
	std::string material_name(name + "/fbo_material");
	std::string texture_name(name + "/rtt_tex");
	_fbo = _create_fbo(name, Ogre::PF_R8G8B8);

	Ogre::ResourcePtr base_mat_res = Ogre::MaterialManager::getSingleton().getByName(base_material);
	Ogre::MaterialPtr fbo_material = static_cast<Ogre::Material *>(base_mat_res.get())->clone(material_name);
	fbo_material->load();
	_fbo_materials.push_back(fbo_material);
	Ogre::AliasTextureNamePairList alias_list;
	std::clog << "Trying to replace rtt_texture alias with " << texture_name << std::endl;
	alias_list["rtt_texture"] = texture_name;
	if(fbo_material->applyTextureAliases(alias_list))
	{
		// @todo this should throw if unsuccesfull
		std::clog << "Succesfully replaced rtt_texture." << std::endl;
	}
	
	/// Create viewport, necessary for FBO
	Ogre::Viewport *view = _fbo->addViewport((Ogre::Camera *)camera->getNative());
	assert(view);
	view->setBackgroundColour(_viewport->getBackgroundColour());
	// Needs the scene mask
	view->setVisibilityMask(_get_fbo_mask());
	view->setClearEveryFrame(true, Ogre::FBT_COLOUR|Ogre::FBT_DEPTH);

	// Implementation specifics for FBO rendering
	_fbo->setAutoUpdated (false);

	_create_screen_quad(name, material_name);

	// Set viewport attributes
	// @todo there is a possibility to use material scheme in the viewport
	// this could help us with deferred and forward rendering selection
	// Without setVisisbilityMask the second channel in Oculus gets copied from another FBO
	// with it only the first one gets copied from the original FBO
	_viewport->setVisibilityMask(_get_window_mask());
	_viewport->setShadowsEnabled(false);
	_viewport->setSkiesEnabled(false);
	_viewport->setOverlaysEnabled(false);

	// Create camera that is not movable for rendering 2D screens
	Ogre::SceneManager *sm = Ogre::Root::getSingleton().getSceneManagerIterator().current()->second;
	assert(sm);
	std::string camera_name = getName() + "/rtt_camera";
	_rtt_camera = sm->createCamera(camera_name);
	_rtt_camera->setProjectionType(Ogre::PT_ORTHOGRAPHIC);

	_viewport->setCamera(_rtt_camera);
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

Ogre::RenderTexture *
vl::Channel::_create_fbo(std::string const &name, Ogre::PixelFormat pf)
{
	std::string fbo_tex_name(name + "/rtt_tex");
	std::string material_name(name + "/fbo_material");

	// Texture size will be complete garbage till it's used
	// so do not check the texture size here. Use an OpenGL debugger instead.
	Ogre::TexturePtr fbo_texture = Ogre::TextureManager::getSingleton()
		.createManual(fbo_tex_name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, 
				Ogre::TEX_TYPE_2D, _viewport->getActualWidth(), _viewport->getActualHeight(), 
				0, pf, Ogre::TU_RENDERTARGET, 0, false, _fsaa);
	_fbo_textures.push_back(fbo_texture);

	Ogre::RenderTexture *fbo = fbo_texture->getBuffer()->getRenderTarget();

	return fbo;
}
