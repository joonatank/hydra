/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 */

#include "config.hpp"

#include "vrpn_tracker.hpp"
#include "fake_tracker.hpp"

#include "math/conversion.hpp"

#include "config_python.hpp"
#include "config_events.hpp"

#include "dotscene_loader.hpp"

#include "tracker_serializer.hpp"
#include "base/filesystem.hpp"

#include <OIS/OISKeyboard.h>
#include <OIS/OISMouse.h>

#include "eq_resource.hpp"

eqOgre::Config::Config( eq::base::RefPtr< eq::Server > parent )
	: eq::Config ( parent ), _event_manager( new vl::EventManager ),
	  _audio_manager(0), _background_sound(0)
{
	// Add events
	_event_manager->addEventFactory( new vl::BasicEventFactory );
	_event_manager->addEventFactory( new vl::ToggleEventFactory );
	_event_manager->addEventFactory( new eqOgre::TransformationEventFactory );
	// Add triggers
	_event_manager->addTriggerFactory( new vl::KeyTriggerFactory );
	_event_manager->addTriggerFactory( new vl::KeyPressedTriggerFactory );
	_event_manager->addTriggerFactory( new vl::KeyReleasedTriggerFactory );
	_event_manager->addTriggerFactory( new vl::FrameTriggerFactory );
	_event_manager->addTriggerFactory( new vl::TrackerTriggerFactory );
	// Add actions
	_event_manager->addActionFactory( new eqOgre::QuitOperationFactory );
	_event_manager->addActionFactory( new eqOgre::ReloadSceneFactory );
	_event_manager->addActionFactory( new eqOgre::AddTransformOperationFactory );
	_event_manager->addActionFactory( new eqOgre::RemoveTransformOperationFactory );
	_event_manager->addActionFactory( new eqOgre::HideActionFactory );
	_event_manager->addActionFactory( new eqOgre::ShowActionFactory );
	_event_manager->addActionFactory( new eqOgre::ToggleMusicFactory );
	_event_manager->addActionFactory( new eqOgre::ActivateCameraFactory );
	_event_manager->addActionFactory( new eqOgre::SetTransformationFactory );
	_event_manager->addActionFactory( new eqOgre::HeadTrackerActionFactory );
}

eqOgre::Config::~Config()
{}

bool
eqOgre::Config::init( eq::uint128_t const & )
{
	_loadScenes();

	// Create Tracker needs the SceneNodes for mapping
	_createTracker(_settings);

	_initAudio();

	try {
		// Init the embedded python
		_initPython();

		std::vector<std::string> scripts = _settings->getScripts();

		EQINFO << "Running " << scripts.size() << " python scripts." << std::endl;

		for( size_t i = 0; i < scripts.size(); ++i )
		{
			// Run init python scripts
			_runPythonScript( scripts.at(i) );
		}
	}
	// Some error handling so that we can continue the application
	catch( ... )
	{
		std::cout << "Exception occured in python script." << std::endl;

	}
	if (PyErr_Occurred())
	{
		PyErr_Print();
	}

	_createQuitEvent();
	_createTransformToggle();

	/// Register data
	EQINFO << "Registering data." << std::endl;

	_frame_data.registerData(this);
	EQASSERT( registerObject( &_resource_manager ) );

	_distrib_settings.setFrameDataID( _frame_data.getID() );
	_distrib_settings.setResourceManagerID( _resource_manager.getID() );

	EQINFO << "Registering Settings" << std::endl;
	EQASSERT( registerObject( &_distrib_settings ) );

	if( !eq::Config::init( _distrib_settings.getID() ) )
	{ return false; }

	EQINFO << "Config::init DONE" << std::endl;

	return true;
}

bool
eqOgre::Config::exit( void )
{
	// First let the children clean up
	bool retval = eq::Config::exit();

	EQINFO << "Deregistering distributed data." << std::endl;

	_frame_data.deregisterData();
	_distrib_settings.setFrameDataID( eq::base::UUID::ZERO );

	deregisterObject( &_resource_manager );
	_distrib_settings.setResourceManagerID( eq::base::UUID::ZERO );

	deregisterObject( &_distrib_settings );

	_exitAudio();

	EQINFO << "Config exited." << std::endl;
	return retval;
}

void
eqOgre::Config::setSettings( vl::SettingsRefPtr settings )
{
	if( settings )
	{
		_settings = settings;
		_createResourceManager();
		_distrib_settings.copySettings(_settings, &_resource_manager);
	}
}

eqOgre::SceneNodePtr
eqOgre::Config::createSceneNode(const std::string& name)
{
	SceneNodePtr node = SceneNode::create( name );
	_addSceneNode( node );
	return node;
}

// TODO implement
void
eqOgre::Config::removeSceneNode(eqOgre::SceneNodePtr node)
{
	EQASSERTINFO( false, "NOT IMPLEMENTED" );
}

eqOgre::SceneNode *
eqOgre::Config::getSceneNode(const std::string& name)
{
	return _frame_data.getSceneNode(name);
}

vl::TrackerTrigger *
eqOgre::Config::getTrackerTrigger(const std::string& name)
{
	EQINFO << "Trying to find TrackerTrigger " << name << std::endl;
	for( size_t i = 0; i < _clients->getNTrackers(); ++i )
	{
		vl::TrackerRefPtr tracker = _clients->getTracker(i);
		for( size_t j = 0; j < tracker->getNSensors(); ++j )
		{
			vl::SensorRefPtr sensor = tracker->getSensor(j);
			if( sensor && sensor->getTrigger() &&
				sensor->getTrigger()->getName() == name )
			{ return( sensor->getTrigger() ); }
		}
	}

	EQINFO << "TrackerTrigger " << name << " not found." << std::endl;
	return 0;
}


// TODO implement
void
eqOgre::Config::resetScene( void )
{
	EQASSERTINFO( false, "NOT IMPLEMENTED" );
}

void
eqOgre::Config::toggleBackgroundSound()
{
	if( !_background_sound )
	{
		EQERROR << "NO background sound to toggle." << std::endl;
		return;
	}

	if( _background_sound->isPlaying() )
	{ _background_sound->pause(); }
	else
	{ _background_sound->play2d(false); }
}


uint32_t
eqOgre::Config::startFrame( eq::uint128_t const &frameID )
{
	// Process Tracking
	// If we have a tracker object update it, the update will handle all the
	// callbacks and appropriate updates (head matrix and scene nodes).
	for( size_t i = 0; i <  _clients->getNTrackers(); ++i )
	{
		_clients->getTracker(i)->mainloop();
	}

	// ProcessEvents does not store the pointer anywhere
	// so it's safe to allocate to the stack
	vl::FrameTrigger frame_trig;
	_event_manager->processEvents( &frame_trig );

	eq::uint128_t version = _frame_data.commitAll();

	return eq::Config::startFrame( version );
}


void
eqOgre::Config::setHeadMatrix( Ogre::Matrix4 const &m )
{
	// Note: real applications would use one tracking device per observer
	const eq::Observers& observers = getObservers();
	for( eq::Observers::const_iterator i = observers.begin();
		i != observers.end(); ++i )
	{
		// When head matrix is set equalizer automatically applies it to the
		// GL Modelview matrix as first transformation
		(*i)->setHeadMatrix( vl::math::convert(m) );
	}
}





/// ------------ Private -------------
void
eqOgre::Config::_createResourceManager(void )
{
	EQINFO << "Creating Resource Manager" << std::endl;

	EQINFO << "Adding project directories to resources. "
		<< "Only project directory and global directory is added." << std::endl;

	EQASSERT( _resource_manager.addResourcePath( _settings->getProjectDir() ) );
	EQASSERT( _resource_manager.addResourcePath( _settings->getGlobalDir() ) );

	// TODO add case directory

	// Add environment directory, used for tracking configurations
	EQINFO << "Adding environment directory to the resources." << std::endl;
	EQASSERT( _resource_manager.addResourcePath( _settings->getEnvironementDir() ) );

	// Add all scene resources
	std::vector<vl::ProjSettings::Scene> const &scenes = _settings->getScenes();
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		_resource_manager.addResource( scenes.at(i).getFile() );
	}

	// Add all tracking resources
	std::vector<std::string> tracking_files = _settings->getTrackingFiles();
	for( size_t i = 0; i < tracking_files.size(); ++i )
	{
		_resource_manager.addResource( tracking_files.at(i) );
	}

	_resource_manager.loadAllResources();
}


void
eqOgre::Config::_addSceneNode(eqOgre::SceneNode* node)
{
	// Implement checking
	// TODO should be in the frame data, as it can neither store multiple
	// SceneNodes with same names
	for( size_t i = 0; i < _frame_data.getNSceneNodes(); ++i )
	{
		SceneNodePtr ptr = _frame_data.getSceneNode(i);
		if( ptr == node || ptr->getName() == node->getName() )
		{
			// TODO is this the right exception?
			BOOST_THROW_EXCEPTION( vl::duplicate() );
		}
	}

	_frame_data.addSceneNode( node );
}

void
eqOgre::Config::_initAudio(void )
{
	EQINFO << "Init audio." << std::endl;

	//Create an Audio Manager
	_audio_manager = cAudio::createAudioManager(true);

	//Create an audio source and load a sound from a file
	std::string file_path;
	EQASSERT( _resource_manager.findResource( "The_Dummy_Song.ogg", file_path ) );
	_background_sound = _audio_manager->create("The_Dummy_Song", file_path.c_str() ,true);
}


void
eqOgre::Config::_exitAudio(void )
{
	EQINFO << "Exit audio." << std::endl;

	//Shutdown cAudio
	if( _audio_manager )
	{
		_audio_manager->shutDown();
		cAudio::destroyAudioManager(_audio_manager);
	}
}

void
eqOgre::Config::_createTracker( vl::SettingsRefPtr settings )
{
	EQINFO << "Creating Trackers." << std::endl;

	_clients.reset( new vl::Clients );
	std::vector<std::string> tracking_files = settings->getTrackingFiles();

	EQINFO << "Processing " << tracking_files.size() << " tracking files."
		<< std::endl;

	for( std::vector<std::string>::const_iterator iter = tracking_files.begin();
		 iter != tracking_files.end(); ++iter )
	{
		// Read a file
		EQINFO << "Copy tracking resource : " << *iter << std::endl;

		// TODO this should be moved to distrib resources
		// and we should get here a copy of the resource
		eqOgre::Resource resource = _resource_manager.copyResource( *iter );

		vl::TrackerSerializer ser( _clients );
		EQASSERTINFO( ser.parseTrackers(resource), "Error in Tracker XML reader." );
	}

	// Start the trackers
	EQINFO << "Starting " << _clients->getNTrackers() << " trackers." << std::endl;
	for( size_t i = 0; i < _clients->getNTrackers(); ++i )
	{
		_clients->getTracker(i)->init();
	}

	// Create Action
	eqOgre::HeadTrackerAction *action = (eqOgre::HeadTrackerAction *)_event_manager->createAction("HeadTrackerAction");
	action->setConfig(this);

	// This will get the head sensor if there is one
	// If not it will create a FakeTracker instead
	vl::TrackerTrigger *head_trigger = getTrackerTrigger( "glassesTrigger" );
	if( head_trigger )
	{
		head_trigger->setAction( action );
	}
	else
	{
		EQINFO << "Creating a fake head tracker" << std::endl;
		vl::TrackerRefPtr tracker( new vl::FakeTracker );
		vl::SensorRefPtr sensor( new vl::Sensor );
		sensor->setDefaultPosition( Ogre::Vector3(0, 1.5, 0) );

		// Create the trigger
		head_trigger = (vl::TrackerTrigger *)_event_manager->createTrigger("TrackerTrigger");
		head_trigger->setName("glassesTrigger");
		head_trigger->setAction( action );
		sensor->setTrigger( head_trigger );

		// Add the tracker
		tracker->setSensor( 0, sensor );
		_clients->addTracker(tracker);
	}
}

void
eqOgre::Config::_loadScenes(void )
{
	EQINFO << "Loading Scenes for Project : " << _settings->getProjectName()
		<< std::endl;

	// Get scenes
	std::vector<eqOgre::Resource> scenes = _resource_manager.getSceneResources();

	// If we don't have Scenes there is no point loading them
	if( !scenes.size() )
	{
		EQINFO << "Project does not have any scene files." << std::endl;
		return;
	}
	else
	{
		EQINFO << "Project has " << scenes.size() << " scene files."
			<< std::endl;
	}

	// Clean up old scenes
	// TODO this should be implemented

	// TODO support for multiple scene files should be tested
	// TODO support for case needs to be tested
	for( size_t i = 0; i < scenes.size(); ++i )
	{
		std::string scene_file_name = scenes.at(i).getName();

		EQINFO << "Loading scene file = " << scene_file_name << std::endl;

		vl::DotSceneLoader loader;
		// TODO pass attach node based on the scene
		// TODO add a prefix to the SceneNode names ${scene_name}/${node_name}
		loader.parseDotScene( scenes.at(i), this );

		EQINFO << "Scene " << scene_file_name << " loaded." << std::endl;
	}
}

void
eqOgre::Config::_initPython(void )
{
	EQINFO << "Initing python context." << std::endl;

	Py_Initialize();

	// Add the module to the python interpreter
	// NOTE the name parameter does not rename the module
	// No idea why it's there
	if (PyImport_AppendInittab("eqOgre_python", initeqOgre_python) == -1)
		throw std::runtime_error("Failed to add eqOgre to the interpreter's "
				"builtin modules");

	// Retrieve the main module
	python::object main = python::import("__main__");

	// Retrieve the main module's namespace
	_global = main.attr("__dict__");

	// Import eqOgre module
    python::handle<> ignored(( PyRun_String("from eqOgre_python import *\n"
                                    "print 'eqOgre imported'       \n",
                                    Py_file_input,
                                    _global.ptr(),
                                    _global.ptr() ) ));

	// Add a global managers i.e. this and EventManager
	_global["config"] = python::ptr<>( this );
	_global["event_manager"] = python::ptr<>( _event_manager );
}

void eqOgre::Config::_runPythonScript(const std::string& scriptFile)
{
	EQINFO << "Running python script file " << scriptFile << "." << std::endl;
	std::string script_path;

	EQASSERT( _resource_manager.findResource( scriptFile, script_path ) );

	// Run a python script.
	python::object result = python::exec_file(script_path.c_str(), _global, _global);
}

void
eqOgre::Config::_createQuitEvent(void )
{
	EQINFO << "Creating QuitEvent" << std::endl;

	// Add a trigger event to Quit the Application
	QuitOperation *quit
		= (QuitOperation *)( _event_manager->createAction( "QuitOperation" ) );
	quit->setConfig(this);
	vl::Event *event = _event_manager->createEvent( "Event" );
	event->setAction(quit);
	// Add trigger
	vl::KeyTrigger *trig = (vl::KeyTrigger *)( _event_manager->createTrigger( "KeyTrigger" ) );
	trig->setKey( OIS::KC_ESCAPE );
	event->addTrigger(trig);
	_event_manager->addEvent( event );
}

void
eqOgre::Config::_createTransformToggle(void )
{
	EQINFO << "Creating TransformToggle " << " Not in use atm." << std::endl;

	// Find ogre event so we can toggle it on/off
	// TODO add function to find Events
	// Ogre Rotation event, used to toggle the event on/off
	/*	FIXME new design
	TransformationEvent ogre_event;
	for( size_t i = 0; i < _trans_events.size(); ++i )
	{
		SceneNodePtr node = _trans_events.at(i).getSceneNode();
		if( node )
		{
			if( node->getName() == "ogre" )
			{
				ogre_event = _trans_events.at(i);
				break;
			}
		}
	}

	Trigger *trig = new KeyTrigger( OIS::KC_SPACE, false );
	Operation *add_oper = new AddTransformEvent( this, ogre_event );
	Operation *rem_oper = new RemoveTransformEvent( this, ogre_event );
	Event *event = new ToggleEvent( hasEvent(ogre_event), add_oper, rem_oper, trig );
	_events.push_back( event );
	*/
}


/// Event Handling
char const *CB_INFO_TEXT = "Config : OIS event received : ";

bool
eqOgre::Config::handleEvent( const eq::ConfigEvent* event )
{

	bool redraw = false;
	switch( event->data.type )
	{
		case eq::Event::KEY_PRESS :

			redraw = _handleKeyPressEvent(event->data.keyPress);
			break;

		case eq::Event::KEY_RELEASE :
			redraw = _handleKeyReleaseEvent(event->data.keyRelease);

			break;

		case eq::Event::POINTER_BUTTON_PRESS:
			redraw = _handleMousePressEvent(event->data.pointerButtonPress);
			break;

		case eq::Event::POINTER_BUTTON_RELEASE:
			redraw = _handleMouseReleaseEvent(event->data.pointerButtonRelease);
			break;

		case eq::Event::POINTER_MOTION:
			redraw = _handleMouseMotionEvent(event->data.pointerMotion);
			break;

		case eq::Event::WINDOW_CLOSE :
		case eq::Event::WINDOW_HIDE :
		case eq::Event::WINDOW_EXPOSE :
		case eq::Event::WINDOW_RESIZE :
		case eq::Event::WINDOW_SHOW :
			break;

		default :
			break;
	}

	return redraw;
}

bool
eqOgre::Config::_handleKeyPressEvent( const eq::KeyEvent& event )
{
	vl::KeyPressedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _event_manager->processEvents( &trig );
}

bool
eqOgre::Config::_handleKeyReleaseEvent(const eq::KeyEvent& event)
{
	vl::KeyReleasedTrigger trig;
	trig.setKey( (OIS::KeyCode )(event.key) );
	return _event_manager->processEvents( &trig );
}

bool
eqOgre::Config::_handleMousePressEvent(const eq::PointerEvent& event)
{
//	std::cerr << "Config received mouse button press event. Button = "
//		<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseReleaseEvent(const eq::PointerEvent& event)
{
//	std::cerr << "Config received mouse button release event. Button = "
//		<< event->data.pointer.button << std::endl;
	return false;
}

bool
eqOgre::Config::_handleMouseMotionEvent(const eq::PointerEvent& event)
{
	return true;
}

bool
eqOgre::Config::_handleJoystickEvent(const eq::MagellanEvent& event)
{
	return false;
}
