/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_manager.hpp
 */

#ifndef VL_SCENE_MANAGER_HPP
#define VL_SCENE_MANAGER_HPP

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreColourValue.h>

// Necessary for PREFAB type
#include "entity.hpp"

#include "distributed.hpp"
#include "session.hpp"

#include "typedefs.hpp"

namespace vl
{

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

enum ShadowTechnique
{
	SHADOWTYPE_NOT_VALID,
	SHADOWTYPE_NONE,
	SHADOWTYPE_TEXTURE_MODULATIVE,
	SHADOWTYPE_TEXTURE_ADDITIVE,
	SHADOWTYPE_STENCIL_MODULATIVE,
	SHADOWTYPE_STENCIL_ADDITIVE,
	SHADOWTYPE_TEXTURE_MODULATIVE_INTEGRATED,
	SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED,
};

std::string getShadowTechniqueAsString(ShadowTechnique tech);

ShadowTechnique getShadowTechniqueFromString(std::string const &str);

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
 */
class ShadowInfo
{
public :
	/// @brief constructor
	/// Shadows are disabled by default, so call to enable is necessary.
	ShadowInfo(std::string const &tech = "none", 
		Ogre::ColourValue const &col = Ogre::ColourValue(0.3, 0.3, 0.3), 
		std::string const &cam = "default");

	/// @brief enable the shadows
	/// Remembers the technique user selected.
	/// If no technique is selected uses the current default.
	void enable(void);

	/// @brief disable the shadows
	void disable(void);

	bool isEnabled(void) const
	{ return _enabled && (_technique != SHADOWTYPE_NONE); }

	/// @brief set the shadow technique using a string, mostly for python
	/// valid strings (upper or lower case):
	/// texture_modulative, texture_additive, none, stencil_modulative, stencil_additive,
	/// texture and stencil (for the modulative version)
	/// Be mindful that setting the technique can cause instabilities as they
	/// are not quarantied to work, this is mostly a development features.
	void setShadowTechnique(std::string const &tech);

	/// @brief the shadow technique
	/// Be mindful that setting the technique can cause instabilities as they
	/// are not quarantied to work, this is mostly a development features.
	///
	/// Additative shadows create almost black shadows. Good if one want's to
	/// see which objects are in shadow, for visual appeal they need some extra work.
	/// Use case visibility checking.
	/// Additative shadows need a shader that handles the addition of multiple lights.
	///
	/// Also by default they don't do shadowed side of an object correctly
	/// object is lit too uniformly everywhere with little regard to the light position.
	///
	/// Stencil shadows will crash at least the test model, 
	/// problems with bounding boxes.
	void setShadowTechnique(ShadowTechnique tech);

	std::string getShadowTechniqueName(void) const;

	ShadowTechnique getShadowTechnique(void) const
	{ return _technique; }

	/// Valid values for camera are "Default", "LiSPSM"
	/// others maybe added later. Values are case insensitive.
	/// Any other value will default to "Default" camera
	/// @todo LiSPSM camera crashes Ogre Release version so don't use it
	/// @todo "PlaneOptimal" camera needs a plane of interest
	void setCamera(std::string const &str);

	std::string const &getCamera(void) const
	{ return _camera; }

	Ogre::ColourValue const &getColour(void) const
	{ return _colour; }

	void setColour(Ogre::ColourValue const &col);

	bool isDirty(void) const
	{ return _dirty; }

	void clearDirty(void)
	{ _dirty = false; }

private :
	void _setDirty(void)
	{ _dirty = true; }

	ShadowTechnique _technique;

	Ogre::ColourValue _colour;

	std::string _camera;

	bool _enabled;

	bool _dirty;
};

inline
bool operator==(ShadowInfo const &a, ShadowInfo const &b)
{
	return( a.getShadowTechnique() == b.getShadowTechnique()
		&& a.getColour() == b.getColour()
		&& a.getCamera() == b.getCamera());
}

inline
bool operator!=(ShadowInfo const &a, ShadowInfo const &b)
{
	return !(a==b);
}

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

	/// Internal
	/// This should only be called on from slaves
	/// This can not be private because it's called from the Renderer
	EntityPtr _createEntity(uint64_t id);

	bool hasEntity( std::string const &name ) const;

	EntityPtr getEntity( std::string const &name ) const;


	/// --------- Light -------
	LightPtr createLight(std::string const &name);

	/// Internal
	/// This should only be called on from slaves
	/// This can not be private because it's called from the Renderer
	LightPtr _createLight(uint64_t id);

	bool hasLight(std::string const &name) const;

	LightPtr getLight(std::string const &name) const;
	

	/// --------- Camera -------
	CameraPtr createCamera(std::string const &name);

	/// Internal
	/// This should only be called on from slaves
	/// This can not be private because it's called from the Renderer
	CameraPtr _createCamera(uint64_t id);

	bool hasCamera(std::string const &name) const;

	CameraPtr getCamera(std::string const &name) const;


	/// --------- MovableText --------------

	MovableTextPtr createMovableText(std::string const &name, std::string const &text);

	/// @internal
	/// This should only be called on from slaves
	/// This can not be private because it's called from the Renderer
	MovableTextPtr _createMovableText(uint64_t id);


	/// --------- RayObject ----------------
	MovableObjectPtr createRayObject(std::string const &name, std::string const &material_name);


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

	OBJ_TYPE getMovableObjectType(std::string const &type) const;

	std::string getMovableObjectTypeName(OBJ_TYPE type) const;

	bool hasMovableObject(vl::OBJ_TYPE, std::string const &name) const;

	bool hasMovableObject(std::string const &type, std::string const &name) const;

	/// --------- List access -------
	SceneNodeList const &getSceneNodeList(void) const
	{ return _scene_nodes; }

	MovableObjectList const &getMovableObjectList(void) const
	{ return _objects; }

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
	
	void mapCollisionBarriers(void);

	/// @brief hides Scene Nodes based on the pattern
	/// use asteriks for any number of any characters
	/// for example pattern cb_* hides any Node with 'cb_' in start of the name
	/// For now asteriks is only supported at the end of the name string
	/// so for example cb_*sp is equavivalent to cb_*
	void hideSceneNodes(std::string const &pattern, bool caseInsensitive = true);
	
	enum DirtyBits
	{
		DIRTY_RELOAD_SCENE = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_SKY_DOME = vl::Distributed::DIRTY_CUSTOM << 1,
		DIRTY_FOG = vl::Distributed::DIRTY_CUSTOM << 2,
		DIRTY_AMBIENT_LIGHT = vl::Distributed::DIRTY_CUSTOM << 3,
		DIRTY_BACKGROUND_COLOUR = vl::Distributed::DIRTY_CUSTOM << 4,
		DIRTY_SHADOW_INFO = vl::Distributed::DIRTY_CUSTOM << 5,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 6,
	};

	Ogre::SceneManager *getNative( void )
	{ return _ogre_sm; }

	/// @internal
	/// @brief step the SceneManager does things like progression of automatic mappings
	void _step(vl::time const &t);

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

	SceneNodePtr _root;
	SceneNodeList _scene_nodes;
	MovableObjectList _objects;

	std::map<SceneNode *, SceneNode *> _mapped_nodes;

	/// Selected SceneNodes
	/// @remarks
	/// At least for now not distributed
	/// the attributes for showing the selected nodes are distributed in the
	/// nodes them selves
	SceneNodeList _selection;

	SkyDomeInfo _sky_dome;
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

}	// namespace cluster
}	// namespace vl

#endif	// VL_SCENE_MANAGER_HPP
