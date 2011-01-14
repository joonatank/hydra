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

#include "eq_cluster/event_manager.hpp"

// Physics headers
#include "physics/physics_events.hpp"
#include "physics/physics_world.hpp"

#include <bullet/btBulletDynamicsCommon.h>

#include "game_manager.hpp"

namespace physics
{

class Config : public eqOgre::Config
{
public :
	Config( eq::base::RefPtr< eq::Server > parent )
		: eqOgre::Config( parent ),
		  _groundShape(0),
		  _fallShape(0),
		  _groundMotionState(0),
		  _fallMotionState(0)
	{}

	virtual bool init( eq::uint128_t const &initID )
	{
		bool ret = eqOgre::Config::init(initID);

		initPhysics();
		return ret;
	}

	virtual bool exit (void)
	{
		bool retval = eqOgre::Config::exit();
		destroyPhysics();

		return retval;
	}

	virtual uint32_t startFrame( eq::uint128_t const &frameID )
	{
		static bool physics_inited = false;
		uint32_t ret = eqOgre::Config::startFrame(frameID);

		// This should have been inited here and the SceneNodes should have been created
		// So we can set the MotionState objects
		if( !physics_inited )
		{
			eqOgre::SceneNode *ogre = getSceneNode("ogre");
			if( !ogre )
			{
				std::cerr << "No SceneNode with the name ogre found!."
					<< " Not initing physics." << std::endl;
			}
			else
			{
				// TODO move to using the World to create the shapes
				_groundShape = _world->createPlaneShape( Ogre::Vector3(0,1,0), 1.0 );
				_fallShape = _world->createSphereShape(1);

				vl::Transform trans( Ogre::Vector3(0, -1, 0), Ogre::Quaternion::IDENTITY );
				_groundMotionState = _world->createMotionState( trans );
				_world->createRigidBody("ground", 0, _groundMotionState, _groundShape);

				std::cerr << "Ground RigidBody created and added." << std::endl;

				Ogre::Vector3 v(0, 20, 0);
				Ogre::Quaternion const &q = ogre->getOrientation();
				trans = vl::Transform( v, q );
				_fallMotionState = _world->createMotionState( trans, ogre );

				// TODO we should have basic RigidBody class which owns the
				// MotionState and Shape. Easier to manage and for now we are not
				// sharing them anyway.
				// This class can also easily be exposed to python
				vl::scalar mass = 1;
				btRigidBody *fallBody = _world->createRigidBody("ogre", mass, _fallMotionState, _fallShape );

				std::cerr << "Fall RigidBody created and added." << std::endl;

				physics_inited = true;

				// Create a physics test Event
				// TODO move to python
				vl::physics::ApplyForce *action = vl::physics::ApplyForce::create();
				vl::KeyPressedTrigger *trig = _game_manager->getEventManager()
					->createKeyPressedTrigger( OIS::KC_F );

				action->setRigidBody( fallBody );
				action->setForce( Ogre::Vector3(0, 500, 0) );
				trig->addAction( action );
			}
		}

		_world->step();

		return ret;
	}

	void initPhysics( void )
	{
		_world = new vl::physics::World;
	}

	void destroyPhysics( void )
	{
		// Bodies
		_world->destroyMotionState( _groundMotionState );
		_world->destroyMotionState( _fallMotionState );
		_world->destroyShape(_fallShape);
		_world->destroyShape(_groundShape);

		// World
		delete _world;
	}

	vl::physics::World *_world;
	// Collision opbejcts
	btCollisionShape *_groundShape;
	btCollisionShape *_fallShape;
	vl::physics::MotionState *_groundMotionState;
	vl::physics::MotionState *_fallMotionState;
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
	std::ofstream *log_file = 0;

	eq::base::RefPtr< eqOgre::Client > client;

	bool error = false;
	try
	{
		vl::SettingsRefPtr settings = eqOgre::getSettings( argc, argv );
		if( !settings )
		{ return -1; }

		physics::NodeFactory nodeFactory;

		if( !settings->getVerbose() )
		{
			// NOTE uses absolute paths
			// NOTE the log file needs to be set before any calls to Equalizer methods
			log_file = new std::ofstream( settings->getEqLogFilePath().c_str() );
			eq::base::Log::setOutput( *log_file );
			std::cout << "Equalizer log file = " << settings->getEqLogFilePath()
				<< std::endl;
		}

		vl::Args &arg = settings->getEqArgs();

		std::cout << "Equalizer arguments = " << settings->getEqArgs() << std::endl;

		int eq_argc = arg.size();
		char **eq_argv = arg.getData();

		// 1. Equalizer initialization
		if( !eq::init( eq_argc, eq_argv, &nodeFactory ) )
		{
			EQERROR << "Equalizer init failed" << std::endl;
			error = true;
		}

		// 2. initialization of local client node
		client = new eqOgre::Client( settings );
		if( !client->initLocal( eq_argc, eq_argv ) )
		{
			EQERROR << "client->initLocal failed" << std::endl;
			error = true;
		}

		if( !error )
		{
			error = !client->run();
			if( error )
			{ std::cerr << "Client run returned an error." << std::endl; }
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

	if( log_file )
	{
		log_file->close();
		delete log_file;
		log_file = 0;
	}

	if( error )
	{ std::cerr << "Application exited with an error." << std::endl; }
	else
	{ std::cerr << "Application exited fine." << std::endl; }

	return error ? EXIT_FAILURE : EXIT_SUCCESS;
}
