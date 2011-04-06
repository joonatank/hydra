
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
	for( std::vector<RigidBody *>::iterator iter = _rigid_bodies.begin();
			iter != _rigid_bodies.end(); ++iter )
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

vl::physics::RigidBody *
vl::physics::World::createRigidBodyEx(std::string const &name, btRigidBody::btRigidBodyConstructionInfo const &info)
{
	RigidBody *body = new RigidBody(name, info);
	addRigidBody(name, body);
	return body;
}


vl::physics::RigidBody *
vl::physics::World::createRigidBody( const std::string& name, vl::scalar mass,
									 vl::physics::MotionState *state,
									 btCollisionShape *shape,
									 Ogre::Vector3 const &inertia)
{
	btVector3 i(vl::math::convert_bt_vec(inertia));
	btRigidBody::btRigidBodyConstructionInfo info(mass, state, shape, i);
	return createRigidBodyEx(name, info);
}

void
vl::physics::World::addRigidBody(std::string const &name, vl::physics::RigidBody *body)
{
	if( !hasRigidBody(name) )
	{
		_rigid_bodies.push_back(body);
		_dynamicsWorld->addRigidBody(body->getNative());
	}
	else
	{
		std::string err( "RigidBody with that name is already in the scene." );
		BOOST_THROW_EXCEPTION( vl::duplicate() << vl::desc(err) );
	}
}

vl::physics::RigidBody *
vl::physics::World::getRigidBody( const std::string& name )
{
	RigidBody *body = _findRigidBody(name);
	if( !body )
	{
		// TODO add a better exception to this
		BOOST_THROW_EXCEPTION( vl::null_pointer() );
	}

	return body;
}

vl::physics::RigidBody *
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
	return new btStaticPlaneShape( vl::math::convert_bt_vec(normal), constant );
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
vl::physics::RigidBody *vl::physics::World::_findRigidBody(const std::string& name)
{
	std::vector<RigidBody *>::iterator iter;
	for( iter = _rigid_bodies.begin(); iter != _rigid_bodies.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{ return *iter; }
	}

	return 0;
}
