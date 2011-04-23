
#include "physics_world.hpp"

/// -------------------------------- Global ----------------------------------
std::ostream &
vl::physics::operator<<(std::ostream &os, vl::physics::World const &w)
{
	os << "Physics World : "
		<< " gravity " << w.getGravity()
		<< " with " << w._rigid_bodies.size() << " rigid bodies "
		<< " and " << w._shapes.size() << " shapes."
		<< "\n";
	
	os << "Bodies : \n";
	RigidBodyList::const_iterator iter;
	for( iter = w._rigid_bodies.begin(); iter != w._rigid_bodies.end(); ++iter )
	{
		os << (*iter)->getName();
		if( iter+1 != w._rigid_bodies.end() )
		{ os << "\n"; }
	}

	return os;
}

/// -------------------------------- Public ----------------------------------
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
vl::physics::World::getGravity(void) const
{
	return vl::math::convert_vec(_dynamicsWorld->getGravity());
}


void
vl::physics::World::setGravity(const Ogre::Vector3& gravity)
{
	_dynamicsWorld->setGravity( vl::math::convert_bt_vec(gravity) );
}

vl::physics::RigidBody *
vl::physics::World::createRigidBodyEx(std::string const &name, btRigidBody::btRigidBodyConstructionInfo const &info)
{
	RigidBody *body = new RigidBody(name, info);
	_addRigidBody(name, body);
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

vl::physics::RigidBody *
vl::physics::World::getRigidBody( const std::string& name ) const
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
vl::physics::World::hasRigidBody( const std::string& name ) const
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
	btStaticPlaneShape *plane = new btStaticPlaneShape( vl::math::convert_bt_vec(normal), constant );
	_shapes.push_back(plane);
	return plane;
}

btBoxShape *
vl::physics::World::createBoxShape(Ogre::Vector3 const &bounds)
{
	btBoxShape *box = new btBoxShape(vl::math::convert_bt_vec(bounds));
	_shapes.push_back(box);
	return box;
}

btSphereShape *
vl::physics::World::createSphereShape( vl::scalar radius )
{
	btSphereShape *sphere = new btSphereShape( radius );
	_shapes.push_back(sphere);
	return sphere;
}

void
vl::physics::World::destroyShape( btCollisionShape *shape )
{
	CollisionShapeList::iterator iter;
	for(iter = _shapes.begin(); iter != _shapes.end(); ++iter)
	{
		if( *iter == shape )
		{
			delete shape;
			_shapes.erase(iter);
			break;
		}
	}
}

/// --------------------------------- Private ----------------------------------
void
vl::physics::World::_addRigidBody(std::string const &name, vl::physics::RigidBody *body)
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
vl::physics::World::_findRigidBody(const std::string& name) const
{
	std::vector<RigidBody *>::const_iterator iter;
	for( iter = _rigid_bodies.begin(); iter != _rigid_bodies.end(); ++iter )
	{
		if( (*iter)->getName() == name )
		{ return *iter; }
	}

	return 0;
}
