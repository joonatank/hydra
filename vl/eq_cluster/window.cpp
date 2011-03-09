/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file window.hpp
 */

#include "window.hpp"

#include "base/exceptions.hpp"
#include "eq_cluster/config.hpp"
#include "eq_ogre/ogre_dotscene_loader.hpp"

#include "channel.hpp"
#include "pipe.hpp"

#include <OGRE/OgreLogManager.h>

/// GUI
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/elements/CEGUIFrameWindow.h>
#include <CEGUI/elements/CEGUIEditbox.h>
#include <CEGUI/elements/CEGUIMultiColumnList.h>
#include <CEGUI/elements/CEGUIListboxTextItem.h>

namespace
{
	/// @todo Missing conversions for the extra buttons in OIS
	/// they could be mapped to other buttons in CEGUI if necessary
	CEGUI::MouseButton OISButtonToGUI( OIS::MouseButtonID button )
	{
		switch( button )
		{
			case OIS::MB_Left :
				return CEGUI::LeftButton;
			case OIS::MB_Right :
				return CEGUI::RightButton;
			case OIS::MB_Middle :
				return CEGUI::MiddleButton;
			case OIS::MB_Button3 :
				return CEGUI::X1Button;
			case OIS::MB_Button4 :
				return CEGUI::X2Button;
			default :
				return CEGUI::NoButton;
		}
	}
}

/// ----------------------------- Public ---------------------------------------
vl::Window::Window( std::string const &name, vl::Pipe *parent )
	: _name(name)
	, _pipe( parent )
	, _channel(0)
	, _ogre_window(0)
	, _viewport(0)
	, _input_manager(0)
	, _keyboard(0)
	, _mouse(0)
{
	assert( _pipe );
	std::string msg = "vl::Window::Window : " + getName();
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	vl::EnvSettings::Window winConf = _pipe->getWindowConf( getName() );

	_createOgreWindow( winConf );
	_createInputHandling();

	// If the channel has a name we try to find matching wall

	vl::EnvSettings::Wall wall;
	if( !winConf.channel.name.empty() )
	{
		msg = "Finding Wall for channel : " + getName();
		Ogre::LogManager::getSingleton().logMessage(msg);
		wall = getEnvironment()->findWall( winConf.channel.wall_name );
	}

	// Get the first wall definition if no named one was found
	if( wall.empty() && getEnvironment()->getWalls().size() > 0 )
	{
		wall = getEnvironment()->getWall(0);
		msg = "No wall found : using the first one " + wall.name;
		Ogre::LogManager::getSingleton().logMessage(msg);
	}

	_channel = new vl::Channel( winConf.channel, wall, getEnvironment()->getIPD() );

	_viewport = _ogre_window->addViewport( getCamera() );
	// Set some parameters to the viewport
	// TODO this should be configurable
	_viewport->setBackgroundColour( Ogre::ColourValue(1.0, 0.0, 0.0, 0.0) );
	// This is necessary because we are using single camera and single viewport
	// to draw to both backbuffers when using stereo.
	_viewport->setAutoUpdated(false);
}

vl::Window::~Window( void )
{
	std::string msg("vl::Window::~Window");
	Ogre::LogManager::getSingleton().logMessage( msg, Ogre::LML_TRIVIAL );

	msg = "Cleaning out OIS";
	Ogre::LogManager::getSingleton().logMessage(msg);
	if( _input_manager )
	{
		msg = "Destroy OIS input manager.";
		Ogre::LogManager::getSingleton().logMessage(msg);
        OIS::InputManager::destroyInputSystem(_input_manager);
		_input_manager = 0;
	}

	delete _channel;

	getOgreRoot()->getNative()->detachRenderTarget(_ogre_window);
}

vl::EnvSettingsRefPtr
vl::Window::getEnvironment( void )
{ return _pipe->getEnvironment(); }

vl::Player const &
vl::Window::getPlayer( void ) const
{ return _pipe->getPlayer(); }

vl::ogre::RootRefPtr
vl::Window::getOgreRoot( void )
{ return _pipe->getRoot(); }

void
vl::Window::setCamera( Ogre::Camera *camera )
{
	assert( _channel );
	_viewport->setCamera(camera);
	_channel->setCamera(camera);
}

Ogre::Camera *
vl::Window::getCamera( void )
{ return _pipe->getCamera(); }

	Ogre::SceneManager *
vl::Window::getSceneManager( void )
{ return _pipe->getSceneManager(); }

void
vl::Window::takeScreenshot( const std::string& prefix, const std::string& suffix )
{
	std::string real_suffix;
	if( getName().empty() )
	{ real_suffix = suffix; }
	else
	{ real_suffix = "-" + getName()+ suffix; }

	_ogre_window->writeContentsToTimestampedFile(prefix, real_suffix);
}

/// ------------------------ Public OIS Callbacks ------------------------------
bool
vl::Window::keyPressed( OIS::KeyEvent const &key )
{
	// Check if we have a GUI that uses user input
	if( getPipe()->guiShown() )
	{
		switch( key.key )
		{
			// Keys that go to the Application always, used to control the GUI
			// TODO should be configurable from python script
			// and should match the one used there
			case OIS::KC_ESCAPE :
				break;
			case OIS::KC_GRAVE :
				break;
			case OIS::KC_F2 :
				break;
			case OIS::KC_F10 :
				break;
			default :
				// TODO this is missing auto-repeat for text
				CEGUI::System::getSingleton().injectKeyDown(key.key);
				CEGUI::System::getSingleton().injectChar(key.text);
				return true;
		}
	}

	vl::cluster::EventData data( vl::cluster::EVT_KEY_PRESSED );
	// TODO add support for the device ID from where the event originated
	vl::cluster::ByteStream stream = data.getStream();
	stream << key;
	_sendEvent( data );

	return true;
}

bool
vl::Window::keyReleased( OIS::KeyEvent const &key )
{
	if( getPipe()->guiShown() )
	{
		// TODO should check if GUI window is active
		// TODO this might need translation for the key codes
		CEGUI::System::getSingleton().injectKeyUp(key.key);
// 		CEGUI::System::getSingleton().injectChar(key.text);
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_KEY_RELEASED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteStream stream = data.getStream();
		stream << key;
		_sendEvent( data );
	}

	return true;
}

bool
vl::Window::mouseMoved( OIS::MouseEvent const &evt )
{
	if( getPipe()->guiShown() )
	{
		// TODO should check if GUI window is active
		CEGUI::System::getSingleton().injectMousePosition(evt.state.X.abs, evt.state.Y.abs);
		// NOTE this has a problem of possibly consuming some mouse movements
		// Assuming that mouse is moved and the wheel is changed at the same time
		// CEGUI might consume either one of them and the other one will not be
		// passed on.
		// This is so rare that we don't care about it.
		if( evt.state.Z.rel != 0 )
		{ CEGUI::System::getSingleton().injectMouseWheelChange(evt.state.Z.rel); }
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_MOVED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteStream stream = data.getStream();
		stream << evt;
		_sendEvent( data );
	}

	return true;
}

bool
vl::Window::mousePressed( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	if( getPipe()->guiShown() )
	{
		CEGUI::System::getSingleton().injectMouseButtonDown( OISButtonToGUI(id) );
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_PRESSED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteStream stream = data.getStream();
		stream << id << evt;
		_sendEvent( data );
	}

	return true;
}

bool
vl::Window::mouseReleased( OIS::MouseEvent const &evt, OIS::MouseButtonID id )
{
	if( getPipe()->guiShown() )
	{
		CEGUI::System::getSingleton().injectMouseButtonUp( OISButtonToGUI(id) );
	}
	else
	{
		vl::cluster::EventData data( vl::cluster::EVT_MOUSE_RELEASED );
		// TODO add support for the device ID from where the event originated
		vl::cluster::ByteStream stream = data.getStream();
		stream << id << evt;
		_sendEvent( data );
	}

	return true;
}

/// ------------------------ Public CEGUI callbacks ----------------------------
bool
vl::Window::onNewClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onNewClicked" << std::endl;
	return true;
}

bool
vl::Window::onOpenClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onOpenClicked" << std::endl;
	return true;
}

bool
vl::Window::onSaveClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onSaveClicked" << std::endl;
	return true;
}

bool
vl::Window::onQuitClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onQuitClicked" << std::endl;
	getPipe()->sendCommand( "quit()" );

	return true;
}

bool
vl::Window::onResetClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onResetClicked" << std::endl;
	return true;
}

bool
vl::Window::onImportSceneClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onImportSceneClicked" << std::endl;
	return true;
}

bool
vl::Window::onReloadScenesClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onReloadScenesClicked" << std::endl;
	return true;
}


bool
vl::Window::onNewScriptClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onNewScriptClicked" << std::endl;
	return true;
}

bool
vl::Window::onAddScriptClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onAddScriptClicked" << std::endl;
	return true;
}

bool
vl::Window::onReloadScriptsClicked( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onReloadScriptsClicked" << std::endl;
	return true;
}

bool
vl::Window::onShowAxisChanged( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onShowAxisChanged" << std::endl;
	return true;
}

bool
vl::Window::onShowNamesChanged( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onShowNamesChanged" << std::endl;
	return true;
}

bool
vl::Window::onShowJointsChanged( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onShowJointsChanged" << std::endl;
	return true;
}

bool
vl::Window::onConsoleTextAccepted( CEGUI::EventArgs const &e )
{
	std::cout << "vl::Window::onConsoleTextAccepted" << std::endl;

	CEGUI::Window *console = getPipe()->getConsole();
	assert( console );
	CEGUI::Editbox *input = static_cast<CEGUI::Editbox *>( console->getChild("console/input") );
	assert( input );
	CEGUI::MultiColumnList *output = static_cast<CEGUI::MultiColumnList *>( console->getChild("console/output") );
	assert( output );

	std::string command( input->getText().c_str() );
	input->setText("");
	CEGUI::ListboxItem *item = new CEGUI::ListboxTextItem(CEGUI::String(command));
	output->addRow(item, 1);

	getPipe()->sendCommand(command);
	return true;
}

/// ----------------------------- Public methods -------------------------------
void
vl::Window::draw( void )
{
	// TODO this has broken the Stereo support
	// _ogre_window needs two Viewports one for each eye
	// or window->update needs to render to both back buffers
	_channel->setHeadMatrix( getPlayer().getHeadMatrix() );
	_channel->draw();
	_ogre_window->update(false);
	// TODO support for multiple channels
}

void
vl::Window::swap( void )
{
	_ogre_window->swapBuffers();
}


void
vl::Window::capture( void )
{
	if( _keyboard && _mouse )
	{
		_keyboard->capture();
		_mouse->capture();
	}
}


void
vl::Window::createGUIWindow(void )
{
	std::string message = "vl::Window::createGUIWindow";
	Ogre::LogManager::getSingleton().logMessage(message);
}

/// ------------------------------- Protected ----------------------------------
void
vl::Window::_sendEvent( vl::cluster::EventData const &event )
{
	_pipe->sendEvent(event);
}

void
vl::Window::_createOgreWindow( vl::EnvSettings::Window const &winConf )
{
	// Info
	std::string message = "Creating Ogre RenderWindow.";
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	Ogre::NameValuePairList params;

	assert( !winConf.empty() );

	params["left"] = vl::to_string( winConf.x );
	params["top"] = vl::to_string( winConf.y );
	params["border"] = "none";

	/// @todo should be configurable
	/// though these should fallback to default without the hardware
	params["stereo"] = "true";
	params["nvSwapSync"] = "true";
	params["swapGroup"] = "1";

	_ogre_window = getOgreRoot()->createWindow( "Hydra-"+getName(), winConf.w, winConf.h, params );
	_ogre_window->setAutoUpdated(false);
}


void
vl::Window::_createInputHandling( void )
{
	std::string message( "Creating OIS Input system." );
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	assert( _ogre_window );

	std::ostringstream windowHndStr;
	size_t windowHnd = 0;
	_ogre_window->getCustomAttribute("WINDOW", &windowHnd);
	windowHndStr << windowHnd;

	OIS::ParamList pl;
	pl.insert( std::make_pair(std::string("WINDOW"), windowHndStr.str()) );
	// Some extra parameters to avoid getting stuck with the window
	// TODO should move these to the configuration file
	pl.insert( std::make_pair(std::string("x11_keyboard_grab"), std::string("false") ) );
	pl.insert( std::make_pair(std::string("x11_mouse_grab"), std::string("false") ) );

	// Info
	message = "Creating OIS Input Manager";
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	_input_manager = OIS::InputManager::createInputSystem( pl );

	_printInputInformation();

	_keyboard = static_cast<OIS::Keyboard*>(_input_manager->createInputObject(OIS::OISKeyboard, true));
	_keyboard->setEventCallback(this);

	_mouse = static_cast<OIS::Mouse*>(_input_manager->createInputObject(OIS::OISMouse, true));

	_mouse ->getMouseState().height = _ogre_window->getHeight();
	_mouse ->getMouseState().width	= _ogre_window->getWidth();

	_mouse->setEventCallback(this);

	message = "Input system created.";
 	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);
}

void
vl::Window::_printInputInformation( void )
{
	// Print debugging information
	// TODO debug information should go to Ogre Log file
	unsigned int v = _input_manager->getVersionNumber();
	std::stringstream ss;
	ss << "OIS Version: " << (v>>16 ) << "." << ((v>>8) & 0x000000FF) << "." << (v & 0x000000FF)
		<< "\nRelease Name: " << _input_manager->getVersionName()
		<< "\nManager: " << _input_manager->inputSystemName()
		<< "\nTotal Keyboards: " << _input_manager->getNumberOfDevices(OIS::OISKeyboard)
		<< "\nTotal Mice: " << _input_manager->getNumberOfDevices(OIS::OISMouse)
		<< "\nTotal JoySticks: " << _input_manager->getNumberOfDevices(OIS::OISJoyStick)
		<< '\n';
 	Ogre::LogManager::getSingleton().logMessage( ss.str() );

	ss.str("");
	// List all devices
	// TODO should go to Ogre Log file
	OIS::DeviceList list = _input_manager->listFreeDevices();
	for( OIS::DeviceList::iterator i = list.begin(); i != list.end(); ++i )
	{ ss << "\n\tDevice: " << " Vendor: " << i->second; }
 	Ogre::LogManager::getSingleton().logMessage( ss.str() );
}
