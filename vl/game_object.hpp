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
 */

#ifndef HYDRA_GAME_OBJECT_HPP
#define HYDRA_GAME_OBJECT_HPP

#include "typedefs.hpp"

// Abstract base
#include "object_interface.hpp"

namespace vl
{

class GameObject : public ObjectInterface
{
public :
	/// @todo fix the constructor to take is at least the basic parameters
	GameObject(std::string const &name, GameManager *manager);

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
	virtual std::string const &getName(void) const
	{ return _name; }

	virtual Transform const &getTransform(void) const;

	virtual void setTransform(Transform const &t);

	virtual void transform(vl::Transform const &t);

	virtual void translate(Ogre::Vector3 const &v);

	virtual void rotate(Ogre::Quaternion const &q);

	virtual void setWorldTransform(vl::Transform const &trans);

	/// @brief get the transformation in the world space
	/// @return Transformation in world space
	virtual vl::Transform getWorldTransform(void) const;

	virtual Ogre::Vector3 const &getPosition(void) const;

	virtual void setPosition(Ogre::Vector3 const &v);

	virtual Ogre::Quaternion const &getOrientation(void) const;

	virtual void setOrientation(Ogre::Quaternion const &q);

	virtual void setVisibility(bool visible);

	virtual bool isVisible(void) const;

	/// Callbacks
	virtual int addListener(TransformedCB::slot_type const &slot);

	/// Convienience functions so our interface resemplaces SceneNodes

	/// @brief adds a game object as a child, only works without physics
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

	GameManager *_creator;

};	// class GameObject

}	// namespace vl

#endif	// HYDRA_GAME_OBJECT_HPP
