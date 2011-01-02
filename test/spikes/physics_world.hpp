/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Bullet Physics World used to initialise the physics with some default
 *	values. Later they might be controllable by the user.
 *
 *	Also contains mixed physics classes for now.
 */

#ifndef VL_PHYSICS_WORLD_HPP
#define VL_PHYSICS_WORLD_HPP

// This class initialises Bullet physics so they are necessary
#include <bullet/btBulletDynamicsCommon.h>

#include <vector>
#include <string>

#include "base/exceptions.hpp"

namespace vl
{

namespace physics
{

/**
 *
 */
class MotionState : public btMotionState
{
public:
	MotionState( Ogre::Vector3 const &pos,
				 Ogre::Quaternion const &orient,
				 eqOgre::SceneNode *node = 0)
		: _visibleobj(node)
	{
		btQuaternion q(orient.x, orient.y, orient.z, orient.w);
		btVector3 v(pos.x, pos.y, pos.z);
		_trans = btTransform( q, v );
	}

	virtual ~MotionState()
	{}

	void setNode(eqOgre::SceneNode *node)
	{
		_visibleobj = node;
	}

	virtual void getWorldTransform(btTransform &worldTrans) const
	{
		worldTrans = _trans;
	}

	virtual void setWorldTransform(const btTransform &worldTrans)
	{
		_trans = worldTrans;

		if( !_visibleobj)
			return; // silently return before we set a node

		btQuaternion rot = worldTrans.getRotation();
		Ogre::Quaternion ogre_rot( rot.w(), rot.x(), rot.y(), rot.z() );
		_visibleobj->setOrientation( ogre_rot );
		btVector3 pos = worldTrans.getOrigin();
		Ogre::Vector3 ogre_vec( pos.x(), pos.y(), pos.z() );
		_visibleobj->setPosition( ogre_vec );
	}

protected:
	eqOgre::SceneNode *_visibleobj;
	btTransform _trans;
};

/**
 *
 */
class World
{
public :
	World( void )
		: _broadphase( new btDbvtBroadphase() ),
		  _collision_config( new btDefaultCollisionConfiguration() ),
		  _dispatcher( new btCollisionDispatcher(_collision_config) ),
		  _solver( new btSequentialImpulseConstraintSolver )
	{
		_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher,_broadphase,_solver,_collision_config);
		_dynamicsWorld->setGravity( btVector3(0,-9.81,0) );
	}

	~World( void )
	{
		// Cleanup the rigid bodies
		for( std::vector<btRigidBody *>::iterator iter = _bodies.begin();
			 iter != _bodies.end(); ++iter )
		{
			delete *iter;
		}

		// cleanup the world
		delete _dynamicsWorld;
		delete _solver;
		delete _dispatcher;
		delete _collision_config;
		delete _broadphase;
	}

	void addRigidBody( btRigidBody *body )
	{
		_bodies.push_back( body );
		_dynamicsWorld->addRigidBody( body );
	}

	// TODO implement
	btRigidBody *getRigidBody( std::string const &name )
	{
		BOOST_THROW_EXCEPTION( vl::not_implemented() );
	}

	// TODO implement
	btRigidBody *removeRigidBody( std::string const &name )
	{
		BOOST_THROW_EXCEPTION( vl::not_implemented() );
	}

	// TODO implement
	void removeRigidBody( btRigidBody *body)
	{
		BOOST_THROW_EXCEPTION( vl::not_implemented() );
	}


	// TODO implement
	bool hasRigidBody( std::string const &name )
	{
		BOOST_THROW_EXCEPTION( vl::not_implemented() );
	}

	void step( void )
	{
		// Some hard-coded parameters for the simulation
		_dynamicsWorld->stepSimulation(1/60.f,10);
	}

private :
	// Bullet physics world objects
	// The order of them is important don't change it.
	btBroadphaseInterface *_broadphase;
	btCollisionConfiguration *_collision_config;
	btCollisionDispatcher *_dispatcher;
	btSequentialImpulseConstraintSolver *_solver;
	btDiscreteDynamicsWorld *_dynamicsWorld;

	// Rigid bodies
	// World owns all of them
	std::vector< btRigidBody * > _bodies;
};

}	// namespace physics

}	// namespace vl

#endif // VL_PHYSICS_EVENTS_HPP

