/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Bullet integration spike
 */

// Necessary for vl::exceptions
#include "base/exceptions.hpp"
// Necessary for vl::msleep
#include "base/sleep.hpp"
// Necessary for settings
#include "eq_cluster/eq_settings.hpp"
// Necessary for creating equalizer nodes
#include "eq_cluster/nodeFactory.hpp"
// Necessary for equalizer client creation
#include "eq_cluster/client.hpp"

#include "eq_cluster/config.hpp"


#include <bullet/btBulletDynamicsCommon.h>

// TODO override config to provide bullet support
namespace physics
{

class MotionState : public btMotionState {
public:
	MotionState(const btTransform &initialpos, eqOgre::SceneNode *node = 0)
	{
		mVisibleobj = node;
		mPos1 = initialpos;
	}

	virtual ~MotionState()
	{}

	void setNode(eqOgre::SceneNode *node)
	{
		mVisibleobj = node;
	}

	virtual void getWorldTransform(btTransform &worldTrans) const
	{
		worldTrans = mPos1;
	}

	virtual void setWorldTransform(const btTransform &worldTrans)
	{
		if( !mVisibleobj)
			return; // silently return before we set a node
		btQuaternion rot = worldTrans.getRotation();
		Ogre::Quaternion ogre_rot( rot.w(), rot.x(), rot.y(), rot.z() );
		mVisibleobj->setOrientation( ogre_rot );
		btVector3 pos = worldTrans.getOrigin();
		Ogre::Vector3 ogre_vec( pos.x(), pos.y(), pos.z() );
		mVisibleobj->setPosition( ogre_vec );
	}

protected:
	eqOgre::SceneNode *mVisibleobj;
	btTransform mPos1;
};

class Config : public eqOgre::Config
{
public :
	Config( eq::base::RefPtr< eq::Server > parent )
		: eqOgre::Config( parent ),
		  _dynamicsWorld(0),
		  _solver(0),
		  _dispatcher(0),
		  _broadphase(0),
		  _collision_config(0),
		  _groundShape(0),
		  _fallShape(0),
		  _groundBody(0),
		  _fallBody(0),
		  _groundMotionState(0),
		  _fallMotionState(0)
	{}

	virtual bool init( uint32_t const initID )
	{
		bool ret = eqOgre::Config::init(initID);

		initPhysics();
		return ret;
	}

	virtual bool exit (void)
	{
		destroyPhysics();

		return eqOgre::Config::exit();
	}

	virtual uint32_t startFrame (const uint32_t frameID)
	{
		static bool physics_inited = false;
		uint32_t ret = eqOgre::Config::startFrame(frameID);

		// This should have been inited here and the SceneNodes should have been created
		// So we can set the MotionState objects
		if( !physics_inited )
		{
			std::cerr << "Initing physics" << std::endl;


			eqOgre::SceneNode *camera = getSceneNode("cameraNode");
			if( camera )
			{
				// TODO these don't seem to work
				camera->setPosition( camera->getPosition() + Ogre::Vector3(0, 0, -20) );
			}
			eqOgre::SceneNode *ogre = getSceneNode("ogre");
			if( !ogre )
			{
				std::cerr << "No SceneNode with the name ogre found!."
					<< " Not initing physics." << std::endl;
			}
			else
			{
				_groundMotionState = new MotionState(btTransform(btQuaternion(0,0,0,1),btVector3(0,-1,0)));
				btRigidBody::btRigidBodyConstructionInfo
						groundRigidBodyCI(0,_groundMotionState,_groundShape,btVector3(0,0,0));
				_groundBody = new btRigidBody(groundRigidBodyCI);
				_dynamicsWorld->addRigidBody(_groundBody);

				std::cerr << "Ground RigidBody created and added." << std::endl;


				Ogre::Vector3 v(0, 50, 0); //= ogre->getPosition();
				Ogre::Quaternion const &q = ogre->getOrientation();
				btTransform transform( btQuaternion(q.x, q.y, q.z, q.w), btVector3(v.x, v.y, v.z ) );
				_fallMotionState = new MotionState( transform, ogre );//btTransform(btQuaternion(0,0,0,1),btVector3(0,50,0)));
				btScalar mass = 1;
				btVector3 fallInertia(0,0,0);
				_fallShape->calculateLocalInertia(mass,fallInertia);
				btRigidBody::btRigidBodyConstructionInfo fallRigidBodyCI(mass,_fallMotionState,_fallShape,fallInertia);
				_fallBody = new btRigidBody(fallRigidBodyCI);
				_dynamicsWorld->addRigidBody(_fallBody);

				std::cerr << "Fall RigidBody created and added." << std::endl;

				physics_inited = true;
			}
		}

		stepPhysics( );

		return ret;
	}

	void initPhysics( void )
	{
		std::cerr << "starting bullet test." << std::endl;

		_broadphase = new btDbvtBroadphase();
		std::cerr << "Broadphase created." << std::endl;


		_collision_config = new btDefaultCollisionConfiguration();
		std::cerr << "CollisionConfiguration created." << std::endl;

		_dispatcher = new btCollisionDispatcher(_collision_config);
		std::cerr << "Dispatcher " << (void *)_dispatcher << " created. size = "
			<< sizeof(btCollisionDispatcher) << std::endl;

		_solver = new btSequentialImpulseConstraintSolver;

		std::cerr << "Solver " << (void *)_solver << " created. size = "
			<< sizeof(btSequentialImpulseConstraintSolver) << std::endl;

		std::cerr << "Trying to create the world." << std::endl;
		_dynamicsWorld = new btDiscreteDynamicsWorld(_dispatcher,_broadphase,_solver,_collision_config);
		std::cerr << "World created." << std::endl;

		_dynamicsWorld->setGravity(btVector3(0,-10,0));
		std::cerr << "Gravity set." << std::endl;

		_groundShape = new btStaticPlaneShape(btVector3(0,1,0), btScalar(1.0));

		_fallShape = new btSphereShape(1);

		std::cerr << "Ground and fall shapes created." << std::endl;
	}

	void stepPhysics( void )
	{
		_dynamicsWorld->stepSimulation(1/60.f,10);

//		btTransform trans;
//		fallRigidBody->getMotionState()->getWorldTransform(trans);

//		std::cout << "sphere height: " << trans.getOrigin().getY() << std::endl;
	}

	void destroyPhysics( void )
	{
		delete _dynamicsWorld;
		delete _solver;
		delete _dispatcher;
		delete _broadphase;
		delete _collision_config;

		delete _groundShape;
		delete _fallShape;
		delete _groundBody;
		delete _fallBody;
		delete _groundMotionState;
		delete _fallMotionState;
	}

	btDiscreteDynamicsWorld *_dynamicsWorld;
	btSequentialImpulseConstraintSolver *_solver;
	btCollisionDispatcher *_dispatcher;
	btBroadphaseInterface *_broadphase;
	btCollisionConfiguration *_collision_config;

	// Collision opbejcts
	btCollisionShape *_groundShape;
	btCollisionShape *_fallShape;
	btRigidBody *_groundBody;
	btRigidBody *_fallBody;
	btMotionState *_groundMotionState;
	btMotionState *_fallMotionState;
};

class NodeFactory : public eqOgre::NodeFactory
{
public:
	virtual eq::Config *createConfig( eq::ServerPtr parent )
	{ return new Config( parent ); }

}; // class NodeFactory

}	//namespace physics


int main( const int argc, char** argv )
{

	std::ofstream log_file;

	eq::base::RefPtr< eqOgre::Client > client;

	bool error = false;
	try
	{
		eqOgre::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		if( !settings )
		{ return -1; }

		// Create the logging directory if it doesn't exist
		// TODO this should create it to the exe path, not current directory
		std::string log_dir("logs");
		if( !fs::exists( log_dir ) )
		{
			fs::create_directory( log_dir );
		}

		// File already exists but it's not a directory
		if( !fs::is_directory( log_dir ) )
		{
			// TODO is there a cleaner way to handle this like changing the name
			std::cerr << "File : " << log_dir << " already exists and it's not "
				<< "a directory. Don't know what to do. Exiting" << std::endl;
			return -1;
		}

		settings->setLogDir( log_dir );
		physics::NodeFactory nodeFactory;

		// TODO test if this can work with relative path
		// Oh right it doesn't for autolaunched clients.
		// NOTE the log file needs to be set before any calls to Equalizer methods
		log_file.open( settings->getEqLogFilePath().c_str() );
		eq::base::Log::setOutput( log_file );
		std::cout << "Equalizer log file = " << settings->getEqLogFilePath()
			<< std::endl;

		vl::Args &arg = settings->getEqArgs();

		std::cout << "Args = " << settings->getEqArgs() << std::endl;
		// 1. Equalizer initialization
		if( !eq::init( arg.size(), arg.getData(), &nodeFactory ) )
		{
			EQERROR << "Equalizer init failed" << std::endl;
			error = true;
		}

		// 2. initialization of local client node
		client = new eqOgre::Client( settings );
		if( !client->initLocal( arg.size(), arg.getData() ) )
		{
			EQERROR << "client->initLocal failed" << std::endl;
			error = true;
		}

		if( !client->initialise() )
		{
			EQERROR << "client->init failed" << std::endl;
			error = true;
		}
		if( !error )
		{
			uint32_t frame = 0;
			while( client->mainloop(++frame) )
			{
				vl::msleep(8);
			}
		}
	}
	catch( vl::exception &e )
	{
		std::cerr << "VL Exception : "<<   boost::diagnostic_information<>(e)
			<< std::endl;
		error = true;
	}
	catch( Ogre::Exception const &e)
	{
		std::cerr << "Ogre Exception: " << e.what() << std::endl;
		error = true;
	}
	catch( std::exception const &e )
	{
		std::cerr << "STD Exception: " << e.what() << std::endl;
		error = true;
	}
	catch( ... )
	{
		error = true;
	}

	// Exit
	client = 0;
	eq::exit();
	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
