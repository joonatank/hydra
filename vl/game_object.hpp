/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file game_object.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_GAME_OBJECT_HPP
#define HYDRA_GAME_OBJECT_HPP

#include "typedefs.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"

// Abstract base
#include "object_interface.hpp"

namespace vl
{

class HYDRA_API GameObject : public ObjectInterface
{
public :
	/// @todo fix the constructor to take is at least the basic parameters
	GameObject(std::string const &name, GameManager *manager, bool dynamic);

	~GameObject(void);

	// Creation methods
	// @todo should this be replaced with settting the parameters inertia and mass
	// and then doing a change body type?
	void createRigidBody(vl::scalar mass, Ogre::Vector3 const &inertia);

	// Get bodies
	SceneNodePtr getGraphicsNode(void) const
	{ return _graphics_node; }

	physics::RigidBodyRefPtr getPhysicsNode(void) const
	{ return _rigid_body; }

	KinematicBodyRefPtr getKinematicsNode(void) const
	{ return _kinematic_body; }


	// Set parameters
	/// @brief Collision detection parameters
	/// pre-condition valid collision shape has been set
	void enableCollisionDetection(bool enable);
	bool isCollisionDetectionEnabled(void) const;

	void setKinematic(bool kinematic);
	bool isKinematic(void) const;

	/// @brief sets the collision model used for this object
	/// this call will always enable collision detection
	/// Uses ConvexShape so do not pass large or concave meshes here.
	/// For more control use the overload function that takes user created shape instead.
	void setCollisionModel(MeshRefPtr mesh);

	void setCollisionModel(physics::CollisionShapeRefPtr shape);

	physics::CollisionShapeRefPtr getCollisionModel(void) const;

	// ObjectInterface overrides
	std::string const &getName(void) const
	{ return _name; }

	/// Base overrides for transforms
	Transform const &getTransform(void) const;

	void setTransform(Transform const &t);

	void setWorldTransform(vl::Transform const &trans);

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	vl::Transform getWorldTransform(void) const;

	void scale(Ogre::Vector3 const &v) {}

	void setScale(Ogre::Vector3 const &v) {}

	Ogre::Vector3 const &getScale(void) const
	{ return Ogre::Vector3(1, 1, 1); }

	void setVisibility(bool visible);

	bool isVisible(void) const;

	bool isDynamic(void) const
	{ return _is_dynamic; }

	/// Callbacks
	virtual int addListener(TransformedCB::slot_type const &slot);

	/// Convienience functions so our interface resemplaces SceneNodes

	/// @brief adds a game object as a child, only works without physics
	/// @todo should this work with physics also, creates a constraint
	/// between the two bodies?
	void addChild(GameObjectRefPtr child);

	/// Private methdos
private :
	/// @brief creates the collision shape from entities added to this object
	void _createCollisionShape(void);

	/// Data
private :
	std::string _name;

	SceneNode *_graphics_node;
	physics::RigidBodyRefPtr _rigid_body;
	KinematicBodyRefPtr _kinematic_body;

	physics::CollisionShapeRefPtr _collision_shape;

	bool _is_dynamic;

	GameManager *_creator;

};	// class GameObject

std::ostream &operator<<(std::ostream &os, GameObject const &obj);

std::ostream &operator<<(std::ostream &os, GameObjectList const &list);

}	// namespace vl

#endif	// HYDRA_GAME_OBJECT_HPP
