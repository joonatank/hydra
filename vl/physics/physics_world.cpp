
#include "physics_world.hpp"

vl::physics::World::World( void )
	: _broadphase( new btDbvtBroadphase() ),
	  _collision_config( new btDefaultCollisionConfiguration() ),
	  _dispatcher( new btCollisionDispatcher(_collision_config) ),
	  _solver( new btSequentialImpulseConstraintSolver )
{
	_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher,_broadphase,_solver,_collision_config);
	_dynamicsWorld->setGravity( btVector3(0,-9.81,0) );
}

vl::physics::World::~World( void )
{
	// Cleanup the rigid bodies
	for( std::map<std::string, btRigidBody *>::iterator iter = _bodies.begin();
			iter != _bodies.end(); ++iter )
	{
		delete iter->second;
	}

	// cleanup the world
	delete _dynamicsWorld;
	delete _solver;
	delete _dispatcher;
	delete _collision_config;
	delete _broadphase;
}

void
vl::physics::World::step( void )
{
	// Some hard-coded parameters for the simulation
	_dynamicsWorld->stepSimulation(1/60.f,10);
}

Ogre::Vector3
vl::physics::World::getGravity(void )
{
	btVector3 v = _dynamicsWorld->getGravity();
	return Ogre::Vector3( v.x(), v.y(), v.z() );
}


void
vl::physics::World::setGravity(const Ogre::Vector3& gravity)
{
	_dynamicsWorld->setGravity( btVector3(gravity.x, gravity.y, gravity.z) );
}


btRigidBody *
vl::physics::World::createRigidBody( const std::string& name, vl::scalar mass,
									 vl::physics::MotionState *state,
									 btCollisionShape *shape,
									 Ogre::Vector3 const &inertia,
									 bool user_driven )
{
	// TODO move to using btRigidBodyConstructionInfo
	btRigidBody *body = new btRigidBody(mass, state, shape );
	btVector3 i( inertia.x, inertia.y, inertia.z );
	body->setMassProps(mass, i);
	addRigidBody( name, body, user_driven );
	return body;
}

void
vl::physics::World::addRigidBody( const std::string name, btRigidBody *body,
								  bool user_driven )
{
	if( _bodies.find(name) == _bodies.end() )
	{
		_bodies.insert( std::make_pair(name, body) );
		_dynamicsWorld->addRigidBody( body );
	}
	else
	{
		std::string err( "RigidBody with that name is already in the scene." );
		BOOST_THROW_EXCEPTION( vl::duplicate() << vl::desc(err) );
	}

	// Every object that is controlled by the user should have
	// DISABLE_DEACTIVATION set
	// Because activation only happens when other bodies are come near
	// NOTE might work also by using body->activate() before moving it
	if( user_driven )
	{
		body->setActivationState(DISABLE_DEACTIVATION);
	}
}

btRigidBody *
vl::physics::World::getRigidBody( const std::string& name )
{
	btRigidBody *body = _findRigidBody(name);
	if( !body )
	{
		// TODO add a better exception to this
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	return body;
}

btRigidBody *
vl::physics::World::removeRigidBody( const std::string& name )
{
	BOOST_THROW_EXCEPTION( vl::not_implemented() );
}

bool
vl::physics::World::hasRigidBody( const std::string& name )
{
	return _findRigidBody(name);
}

vl::physics::MotionState *
vl::physics::World::createMotionState( const vl::Transform &trans, vl::SceneNode *node )
{
	return new MotionState( trans, node );
}

void
vl::physics::World::destroyMotionState( vl::physics::MotionState *state )
{
	delete state;
}

btStaticPlaneShape *
vl::physics::World::createPlaneShape( const Ogre::Vector3& normal, vl::scalar constant )
{
	return new btStaticPlaneShape( btVector3( normal.x, normal.y, normal.z ),
								   constant );
}

btSphereShape *
vl::physics::World::createSphereShape( vl::scalar radius )
{
	return new btSphereShape( radius );
}

void
vl::physics::World::destroyShape( btCollisionShape *shape )
{
	delete shape;
}

/// --------------------------------- Private ----------------------------------
btRigidBody* vl::physics::World::_findRigidBody(const std::string& name)
{
	std::map<std::string, btRigidBody *>::iterator iter = _bodies.find(name);
	if( iter != _bodies.end() )
	{
		return iter->second;
	}

	return 0;
}
