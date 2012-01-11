/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file scene_manager.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#ifndef HYDRA_SCENE_MANAGER_HPP
#define HYDRA_SCENE_MANAGER_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreColourValue.h>

// Necessary for PREFAB type
#include "entity.hpp"

#include "distributed.hpp"
#include "session.hpp"

#include "typedefs.hpp"

#include "math/transform.hpp"

// Necessary for sky simulation
#include <SkyX.h>

namespace vl
{

extern const char *EDITOR_CAMERA;

enum FogMode
{
	FOG_NONE,
	FOG_LINEAR,
	FOG_EXP,
	FOG_EXP2,
};

inline std::string
getFogModeAsString(FogMode mode)
{
	switch(mode)
	{
	case FOG_NONE:
		return "none";
	case FOG_LINEAR:
		return "linear";
	case FOG_EXP:
		return "exp";
	case FOG_EXP2:
		return "exp2";
	default:
		return "unknown";
	}
}

struct SkyDomeInfo
{
	SkyDomeInfo( std::string const &mat_name = std::string(), Ogre::Real curv = 10, 
			Ogre::Real tile = 8, Ogre::Real dist = 50, bool drawFirst = true, 
			Ogre::Quaternion const &orient = Ogre::Quaternion::IDENTITY, 
			int xseg=16, int yseg=16, int yseg_keep=-1 )
		: material_name(mat_name)
		, curvature(curv)
		, tiling(tile)
		, distance(dist)
		, draw_first(drawFirst)
		, orientation(orient)
		, xsegments(xseg)
		, ysegments(yseg)
		, ysegments_keep(yseg_keep)
	{}

	bool empty(void) const
	{ return material_name.empty(); }
	
	std::string material_name;
	Ogre::Real curvature;
	Ogre::Real tiling;
	Ogre::Real distance;
	bool draw_first;
	Ogre::Quaternion orientation;
	int xsegments;
	int ysegments;
	int ysegments_keep;
};

/** @class SkyInfo
 *	For more realistic and complex Sky rendering.
 *	
 *	For now only supports presets
 */
class SkyInfo
{
public :
	SkyInfo(std::string const &preset_name = "none");

	// Valid presets "none", "clear", "desert", "thunderstorm", "sunset", and "night"
	// case insensitive
	// none is a special case that will disable the Sky and switch to regular SkyDome
	// passing an incorrect preset or an empty string will switch to "none"
	void setPreset(std::string const &preset_name);

	std::string const &getPreset(void) const
	{ return _preset; }

	bool isDirty(void) const
	{ return _dirty; }

	void clearDirty(void)
	{ _dirty = false; }

private :
	void _setDirty(void)
	{ _dirty = true; }

	std::string _preset;

	bool _dirty;
};

struct FogInfo
{
	FogInfo( FogMode fog_mode = FOG_NONE, 
			 Ogre::ColourValue const &diff = Ogre::ColourValue::White,
			 Ogre::Real expDensity = 0.001, Ogre::Real linearStart = 0,
			 Ogre::Real linearEnd = 1 )
		: mode(fog_mode), colour_diffuse(diff), exp_density(expDensity)
		, linear_start(linearStart), linear_end(linearEnd)
	{}

	/// Constructor for python (without the enum)
	FogInfo( std::string const &fog_mode, 
			 Ogre::ColourValue const &diff = Ogre::ColourValue::White,
			 Ogre::Real expDensity = 0.001, Ogre::Real linearStart = 0,
			 Ogre::Real linearEnd = 1 )
		: mode(FOG_NONE), colour_diffuse(diff), exp_density(expDensity)
		, linear_start(linearStart), linear_end(linearEnd)
	{
		setMode(fog_mode);
	}

	bool valid(void) const
	{ return mode != FOG_NONE; }

	/// For python, so that we don't need to expose enums
	void setMode(std::string const &fog_mode)
	{
		std::string s(fog_mode);
		vl::to_lower(s);
		if( s == "linear" )
		{ mode = FOG_LINEAR; }
		else if( s == "exp" )
		{ mode = FOG_EXP; }
		else if( s == "exp2" )
		{ mode = FOG_EXP2; }
	}

	std::string getMode(void) const
	{ 
		return getFogModeAsString(mode);
	}

	FogMode mode;
	Ogre::ColourValue colour_diffuse;
	Ogre::Real exp_density;
	Ogre::Real linear_start;
	Ogre::Real linear_end;
};

/**	@class ShadowInfo
 *	@todo add real disablation of the Shadows distinction between not updating
 *	them and disabled. 
 *	Disabled would need the SceneManager to reset the shadow textures.
 *
 *	@todo add shadow debug colours
 *	Shadow colours should only be used for debugging (as long as we have no
 *	easy access to the depth map.
 *	So they should not be accessable by the user except for a boolean debug value.
 */
class ShadowInfo
{
public :
	/// @brief constructor
	/// Shadows are disabled by default, so call to enable is necessary.
	ShadowInfo(std::string const &cam = "default");

	/// @brief enable the shadows
	void enable(void)
	{ setEnabled(true); }

	/// @brief disable the shadows
	void disable(void)
	{ setEnabled(false); }

	void setEnabled(bool enabled);

	bool isEnabled(void) const
	{ return _enabled; }

	void setMaxDistance(vl::scalar dist);

	vl::scalar getMaxDistance(void) const
	{ return _max_distance; }

	void setShadowCasterMaterial(std::string const &material_name);

	std::string const &getShadowCasterMaterial(void) const
	{ return _caster_material; }

	void setShelfShadowEnabled(bool enable);

	bool isShelfShadowEnabled(void) const
	{ return _shelf_shadow; }

	void setDirLightTextureOffset(vl::scalar offset);

	vl::scalar getDirLightTextureOffset(void) const
	{ return _dir_light_texture_offset; }

	/// Valid values for camera are "Default", "LiSPSM", "Focused", "PSSM"
	/// others may be added later. Values are case insensitive.
	/// Any other value will default to "Default" camera
	/// PSSM is completely experimental and WILL NOT work as excepted.
	///
	/// Default is generally the best and should always be used 
	/// for benchmarking other methods.
	/// Other methods not only cost more computing resources but also
	/// use aproximation algorithms that might or might not lower the
	/// quality depending on the scene.
	///
	/// At the moment the real draw back with Default is that it does not
	/// work with directional lights so one needs to use one of the others
	/// for directional lights.
	/// This is at the moment handleded by overriding shadow camera setup
	/// for directional lights. They use Focused camera and this can not
	/// be overriden by the user.
	///
	/// The quality draw back with Default is that it produces hard shadows
	/// with lots of aliasing e.g. jagged edges. Also the aliasing might produce
	/// lots of tiny shadow artifacts where one or two pixels are in shadow,
	/// depending on the scene.
	/// These are more of errors in the shadow mapping shader than the camera though.
	///
	/// All the other methods except Default (and PSSM might be an exception but
	/// is not yet supported) produce softer shadows
	/// but there might be really significant amount of shadow "swimming"
	/// where the shadow seems to be moving when camera is moved.
	/// also moving camera parrallel to the light might produce huge amount of
	/// artifacts.
	/// Unless the scene is just perfect for the particular shadow camera
	/// and does not have these artifacts the jagged shadows produced
	/// by Default are usually ten times better.
	///
	/// Focused is not necessary better than default but it only uses
	/// one shadow map per light as the default and sometimes provides better
	/// results. Focused is the only one which works with directional lights
	/// at the moment.
	/// 
	/// LiSPSM has creately softer shadows compared to the default one
	/// i.e. less pixelisation in the edges.
	/// If the camera is not parallel to the light, 
	/// if the camera is parallel to light LiSPSM shadows can be really crappy.
	///
	/// PSSM needs more than one shadow map per light
	/// so it's hugely more inefficient than other methods.
	///
	/// Also PSSM needs all shadow maps to be exposed and used in shaders
	/// so PSSM is not supported at the moment because our shaders
	/// can not handle more than one shadow map per light.
	///
	/// PSSM is probably the "best" shadow technique assuming you use at least
	/// three textures per shadow. PSSM is most useful in large scenes e.g. out-door.
	///
	/// @fixme LiSPSM crashes Ogre Release version so don't use it
	/// @todo "PlaneOptimal" camera needs a plane of interest
	/// @todo add PSSM shaders
	/// @todo add a separate variable for choosing what camera to use for 
	/// directional lights (as opposed to spot lights)
	///
	/// @note point lights will not be supported anytime soon.
	void setCamera(std::string const &str);

	std::string const &getCamera(void) const
	{ return _camera; }

	void setTextureSize(int);

	int getTextureSize(void) const
	{ return _texture_size; }

	bool isDirty(void) const
	{ return _dirty; }

	void clearDirty(void)
	{ _dirty = false; }

private :
	void _setDirty(void)
	{ _dirty = true; }

	std::string _camera;

	bool _enabled;

	int _texture_size;

	vl::scalar _max_distance;
	vl::scalar _dir_light_texture_offset;

	std::string _caster_material;

	bool _shelf_shadow;

	bool _dirty;
};

inline
bool operator==(ShadowInfo const &a, ShadowInfo const &b)
{
	return( a.getTextureSize() == b.getTextureSize()
		&& a.getCamera() == b.getCamera() );
}

inline
bool operator!=(ShadowInfo const &a, ShadowInfo const &b)
{
	return !(a==b);
}

/** SkyX settings struct
    @remarks These just are the most important SkyX parameters, not all SkyX parameters.
 */
struct SkyXSettings
{
	/** Constructor
	 */
	SkyXSettings(const Ogre::Vector3 t, const Ogre::Real& tm, const Ogre::Real& mp, const SkyX::AtmosphereManager::Options& atmOpt,
		const bool& lc, const bool& vc, const Ogre::Real& vcws, const bool& vcauto, const Ogre::Radian& vcwd, 
		const Ogre::Vector3& vcac, const Ogre::Vector4& vclr,  const Ogre::Vector4& vcaf, const Ogre::Vector2& vcw)
		: time(t), timeMultiplier(tm), moonPhase(mp), atmosphereOpt(atmOpt), layeredClouds(lc), volumetricClouds(vc), vcWindSpeed(vcws)
		, vcAutoupdate(vcauto), vcWindDir(vcwd), vcAmbientColor(vcac), vcLightResponse(vclr), vcAmbientFactors(vcaf), vcWheater(vcw)
	{}

	/** Constructor
	 */
	SkyXSettings(const Ogre::Vector3 t, const Ogre::Real& tm, const Ogre::Real& mp, const SkyX::AtmosphereManager::Options& atmOpt,
		const bool& lc, const bool& vc)
		: time(t), timeMultiplier(tm), moonPhase(mp), atmosphereOpt(atmOpt), layeredClouds(lc), volumetricClouds(vc)
	{}

	// Must be for std::map
	SkyXSettings(void)
	{}

	/// Time
	Ogre::Vector3 time;
	/// Time multiplier
	Ogre::Real timeMultiplier;
	/// Moon phase
	Ogre::Real moonPhase;
	/// Atmosphere options
	SkyX::AtmosphereManager::Options atmosphereOpt;
	/// Layered clouds?
	bool layeredClouds;
	/// Volumetric clouds?
	bool volumetricClouds;
	/// VClouds wind speed
	Ogre::Real vcWindSpeed;
	/// VClouds autoupdate
	bool vcAutoupdate;
	/// VClouds wind direction
	Ogre::Radian vcWindDir;
	/// VClouds ambient color
	Ogre::Vector3 vcAmbientColor;
	/// VClouds light response
	Ogre::Vector4 vcLightResponse;
	/// VClouds ambient factors
	Ogre::Vector4 vcAmbientFactors;
	/// VClouds wheater
	Ogre::Vector2 vcWheater;
};

class SceneManager : public vl::Distributed
{
public :
	/// Master constructor
	/// @param session a session this SceneManager belongs to, used for registering objects.
	SceneManager(vl::Session *session, vl::MeshManagerRefPtr mesh_man);

	/// Renderer constructor
	/// @param session a session this SceneManager belongs to, used for mapping objects.
	/// @param id identifier of the master object this is mapped to.
	/// @param native the Ogre SceneManager this manager drives.
	SceneManager(vl::Session *session, uint64_t id, Ogre::SceneManager *native, vl::MeshManagerRefPtr mesh_man);

	virtual ~SceneManager( void );


	vl::MeshManagerRefPtr getMeshManager(void) const
	{
		return _mesh_manager;
	}

	/// ---------- SceneNode ---------------
	SceneNodePtr getRootSceneNode(void)
	{ return _root; }

	/// @brief create a SceneNode that is attached to the Root node
	SceneNodePtr createSceneNode(std::string const &name);

	/// @brief create a SceneNode that is not attached anywhere
	/// Mostly for internal use and maybe for file parsers
	SceneNodePtr createFreeSceneNode(std::string const &name);

	/// Internal
	SceneNodePtr _createSceneNode(uint64_t id);

	bool hasSceneNode( std::string const &name ) const;

	SceneNodePtr getSceneNode( std::string const &name ) const;

	SceneNodePtr getSceneNodeID(uint64_t id) const;

	size_t getNSceneNodes( void ) const
	{ return _scene_nodes.size(); }

	// TODO add SceneNode removal


	/// --- Entity ---
	EntityPtr createEntity(std::string const &name, vl::PREFAB type);

	/// @brief create and entity
	/// @return a newly create Entity
	/// @param name the name of the entity, needs to be unique
	/// @param mesh_name the name of the Mesh used for this entity, file name usually
	/// @param use_new_mesh_manager wether to use the new MeshManager or not
	/// @throw (add exception type) if mesh already exists
	/// Don't use the new Mesh Manager other than for testing it's not stable yet
	/// Mesh name can either be a file name or a precreated mesh name
	EntityPtr createEntity(std::string const &name, 
		std::string const &mesh_name);
	EntityPtr createEntity(std::string const &name, 
		std::string const &mesh_name, bool use_new_mesh_manager);

	bool hasEntity( std::string const &name ) const;

	EntityPtr getEntity( std::string const &name ) const;


	/// --------- Light -------
	LightPtr createLight(std::string const &name);

	bool hasLight(std::string const &name) const;

	LightPtr getLight(std::string const &name) const;
	

	/// --------- Camera -------
	CameraPtr createCamera(std::string const &name);

	bool hasCamera(std::string const &name) const;

	CameraPtr getCamera(std::string const &name) const;


	/// --------- MovableText --------------

	MovableTextPtr createMovableText(std::string const &name, std::string const &text);

	bool hasMovableText(std::string const &name) const;

	MovableTextPtr getMovableText(std::string const &name) const;


	/// --------- RayObject ----------------
	RayObjectPtr createRayObject(std::string const &name, std::string const &material_name);

	bool hasRayObject(std::string const &name) const;

	RayObjectPtr getRayObject(std::string const &name) const;

	/// --------- MovableObject ------------
	/// @brief Common creator for all the movable objects, extra params are passed using a param list
	MovableObjectPtr createMovableObject(std::string const &type_name, std::string const &name, vl::NamedParamList const &params = vl::NamedParamList());
	MovableObjectPtr createMovableObject(vl::OBJ_TYPE type, std::string const &name, vl::NamedParamList const &params = vl::NamedParamList());

	/// @internal
	MovableObjectPtr _createMovableObject(std::string const &type, uint64_t id);

	/// @internal
	MovableObjectPtr _createMovableObject(vl::OBJ_TYPE type, uint64_t id);

	MovableObjectPtr getMovableObjectID(uint64_t id) const;

	MovableObjectPtr getMovableObject(std::string const &type, std::string const &name) const;

	MovableObjectPtr getMovableObject(vl::OBJ_TYPE type, std::string const &name) const;

	OBJ_TYPE getMovableObjectType(std::string const &type) const;

	std::string getMovableObjectTypeName(OBJ_TYPE type) const;

	bool hasMovableObject(vl::OBJ_TYPE, std::string const &name) const;

	bool hasMovableObject(std::string const &type, std::string const &name) const;

	/// ---------------------- Editor objects --------------------------------
	CameraPtr getEditorCamera(void)
	{ return getCamera(EDITOR_CAMERA); }

	/// --------- List access -------
	SceneNodeList const &getSceneNodeList(void) const
	{ return _scene_nodes; }

	MovableObjectList const &getMovableObjectList(void) const
	{ return _objects; }

	CameraList getCameraList(void) const;

	/// --------- Scene parameters ---------------
	void setSkyDome(SkyDomeInfo const &dome);

	/// @todo these are problematic when used from python. 
	/// User expects to be able to modify the object but because these return
	/// const refs and are copied in python. User is editing temp object.
	/// Can not be solved by changing to non-const refs because
	/// the SceneManager has no knowledge that they have been modified
	/// so we need to add DIRTIES to the SkyDomeInfo and FogInfo for this to work.
	SkyDomeInfo const &getSkyDome(void) const
	{ return _sky_dome; }

	/// @brief set and get Sky value for Sky simulator
	/// Setting a valid SkyInfo (not none) will override any SkyDome settings
	void setSky(SkyInfo const &sky);
	SkyInfo &getSky(void)
	{ return _sky; }

	void setFog(FogInfo const &fog);

	FogInfo const &getFog(void) const
	{ return _fog; }

	Ogre::ColourValue const &getAmbientLight(void) const
	{ return _ambient_light; }

	void setAmbientLight(Ogre::ColourValue const &colour);

	Ogre::ColourValue const &getBackgroundColour(void) const
	{ return _background_colour; }

	void setBackgroundColour(Ogre::ColourValue const &colour);

	/// @brief get the shadow structure
	/// Non-const version easier to edit. Uses the new dirty object system 
	/// which allows for editing non-distributed objects and keeps track of the
	/// edits for distribution.
	ShadowInfo &getShadowInfo(void)
	{ return _shadows; }

	ShadowInfo const &getShadowInfo(void) const
	{ return _shadows; }

	void setShadowInfo(ShadowInfo const &info);

	/// @todo should be removed, this is going to the GameManager anyway
	void reloadScene( void );

	uint32_t getSceneVersion( void ) const
	{ return _scene_version; }

	void printBoundingBoxes(void);

	/// --------------------- Selection -----------------------
	void addToSelection( SceneNodePtr node );

	void removeFromSelection( SceneNodePtr node );

	bool isInSelection(vl::SceneNode const *node) const;

	SceneNodeList const &getSelection( void ) const
	{ return _selection; }

	void clearSelection(void);

	SceneNodePtr getActiveObject(void) const
	{ return _active_object; }

	void setActiveObject(SceneNodePtr node);

	void mapCollisionBarriers(void);

	/// @brief hides Scene Nodes based on the pattern
	/// use asteriks for any number of any characters
	/// for example pattern cb_* hides any Node with 'cb_' in start of the name
	/// For now asteriks is only supported at the end of the name string
	/// so for example cb_*sp is equavivalent to cb_*
	void hideSceneNodes(std::string const &pattern, bool cascade = true, bool caseInsensitive = true);
	
	/// @brief records a Transform which is used to generate a ray
	void addRecordedRay(vl::Transform const &t);

	enum DirtyBits
	{
		DIRTY_RELOAD_SCENE = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_SKY = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_FOG = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_AMBIENT_LIGHT = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_BACKGROUND_COLOUR = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_SHADOW_INFO = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 6,
	};

	Ogre::SceneManager *getNative( void )
	{ return _ogre_sm; }

	SkyX::SkyX *getSkyX(void)
	{ return _skyX; }

	/// @internal
	/// @brief step the SceneManager does things like progression of automatic mappings
	void _step(vl::time const &t);

	/// @internal
	/// @brief slave method for notifying the SceneManager that the frame is about to be rendered
	void _notifyFrameStart(void);

	/// @internal
	/// @brief slave method for notifying the SceneManager that the frame has been rendered
	void _notifyFrameEnd(void);

private :
	virtual void recaluclateDirties(void);
	virtual void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const;
	virtual void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

	MovableObjectPtr _createEntity(std::string const &name, vl::NamedParamList const &params);
	MovableObjectPtr _createLight(std::string const &name, vl::NamedParamList const &params);
	MovableObjectPtr _createCamera(std::string const &name, vl::NamedParamList const &params);
	MovableObjectPtr _createMovableText(std::string const &name, vl::NamedParamList const &params);
	MovableObjectPtr _createRayObject(std::string const &name, vl::NamedParamList const &params);

	// @todo rename to avoid confusion
	SceneNodePtr _createSceneNode(std::string const &name, uint64_t id);

	// @brief helper functions for creating a Sky and changing presets
	// only working on Rendering copies
	void _initialiseSky(SkyXSettings const &preset);
	void _changeSkyPreset(SkyXSettings const &preset);
	bool _isSkyEnabled(void) const;

	SceneNodePtr _root;
	SceneNodeList _scene_nodes;
	MovableObjectList _objects;

	std::map<SceneNode *, SceneNode *> _mapped_nodes;

	/// Selected SceneNodes
	/// @remarks
	/// At least for now not distributed
	/// the attributes for showing the selected nodes are distributed in the
	/// nodes them selves
	/// @todo active object and selection should be moved to separate class
	/// and moved the interface to GameManager
	/// because they work on Game objects (SceneNodes, KinematicBodies, RigidBodies)
	/// use the common object interface for them.
	/// Also they are not distributed because they need not be. We can use some
	/// visual cues that can be separate MovableObjects to show them in the renderer
	/// or bounding boxes as they are for the moment.
	SceneNodeList _selection;
	SceneNodePtr _active_object;

	SkyDomeInfo _sky_dome;
	SkyInfo _sky;
	FogInfo _fog;

	// Reload the scene
	uint32_t _scene_version;

	Ogre::ColourValue _ambient_light;

	Ogre::ColourValue _background_colour;

	ShadowInfo _shadows;

	vl::Session *_session;

	vl::MeshManagerRefPtr _mesh_manager;

	// SceneManager used for creating mapping between vl::SceneNode and
	// Ogre::SceneNode
	// Only valid on slaves and only needed when the SceneNode is mapped
	Ogre::SceneManager *_ogre_sm;

	/// SkyX stuff
	SkyX::BasicController *_skyX_controller;
	SkyX::SkyX *_skyX;
	std::map<std::string, SkyXSettings> _sky_presets;

};	// class SceneManager

std::ostream &operator<<(std::ostream &os, vl::SceneManager const &scene);

std::ostream &operator<<(std::ostream &os, vl::SkyDomeInfo const &sky);

std::ostream &operator<<(std::ostream &os, vl::FogInfo const &fog);

std::ostream &operator<<(std::ostream &os, vl::ShadowInfo const &shadows);

namespace cluster
{

template<>
ByteStream &operator<<(ByteStream &msg, vl::SkyDomeInfo const &sky);

template<>
ByteStream &operator>>(ByteStream &msg, vl::SkyDomeInfo &sky);

template<>
ByteStream &operator<<(ByteStream &msg, vl::FogInfo const &fog);

template<>
ByteStream &operator>>(ByteStream &msg, vl::FogInfo &fog);

template<>
ByteStream &operator<<(ByteStream &msg, vl::ShadowInfo const &shadows);

template<>
ByteStream &operator>>(ByteStream &msg, vl::ShadowInfo &shadows);

template<>
ByteStream &operator<<(ByteStream &msg, vl::SkyInfo const &sky);

template<>
ByteStream &operator>>(ByteStream &msg, vl::SkyInfo &sky);

}	// namespace cluster
}	// namespace vl

#endif	// HYDRA_SCENE_MANAGER_HPP
