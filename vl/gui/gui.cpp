/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file GUI/gui.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

// Interface
#include "gui.hpp"

/// GUI
#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/CEGUIOgreRenderer.h>
#include <CEGUI/CEGUISystem.h>
#include <CEGUI/CEGUIWindow.h>
#include <CEGUI/CEGUIWindowManager.h>
#include <CEGUI/CEGUIDefaultResourceProvider.h>
#include <CEGUI/CEGUIImageset.h>
#include <CEGUI/CEGUIScheme.h>
#include <CEGUI/CEGUIInputEvent.h>

#include <OGRE/OgreLogManager.h>

// gui window, necessary for the windows this creates
#include "gui_window.hpp"

#include "settings.hpp"
// Necessary for init GUI
#include "window.hpp"

vl::gui::GUI::GUI(vl::Session *session)
	: _session(session)
{
	assert(_session);
	_session->registerObject(this, OBJ_GUI);
}

vl::gui::GUI::GUI(vl::Session *session, uint64_t id, vl::CommandCallback *cb)
	: _cmd_cb(cb)
	, _session(session)
	, _root(0)
{
	assert(_session);

	if(id == vl::ID_UNDEFINED)
	{ BOOST_THROW_EXCEPTION(vl::invalid_id()); }

	_session->registerObject(this, OBJ_GUI, id);
}

vl::gui::WindowRefPtr
vl::gui::GUI::createWindow(std::string const &type, std::string const &name, std::string const &layout)
{
	/// @todo add name support
	vl::gui::WindowRefPtr win;
	OBJ_TYPE t;
	if(type == "console")
	{
		_console.reset(new ConsoleWindow(this));
		win = _console;
		t = OBJ_GUI_CONSOLE;
	}
	else if(type == "editor")
	{
		_editor.reset(new EditorWindow(this));
		win = _editor;
		t = OBJ_GUI_EDITOR;
	}
	else if(type == "window")
	{
		t = OBJ_GUI_WINDOW;
		win.reset(new Window(this, layout));
	}
	else
	{
		std::cout << vl::CRITICAL << "Unknown window type : " << type << std::endl;
		t = OBJ_GUI_WINDOW;
		win.reset(new Window(this, layout));
	}

	assert(win);
	_session->registerObject(win.get(), t);

	_windows.push_back(win);

	return win;
}

vl::gui::WindowRefPtr
vl::gui::GUI::createWindow(vl::OBJ_TYPE t, uint64_t id)
{
	vl::gui::WindowRefPtr win;
	switch(t)
	{
	case OBJ_GUI_CONSOLE:
		_console.reset(new ConsoleWindow(this));
		win = _console;
		break;
	case OBJ_GUI_EDITOR:
		_editor.reset(new EditorWindow(this));
		win = _editor;
		break;
	case OBJ_GUI_WINDOW:
		win.reset(new Window(this));
		break;
	default :
		std::cout << vl::CRITICAL << "GUI::createWindow : Incorrect type for Window" << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception());
	}
	
	assert(win);
	_session->registerObject(win.get(), t, id);

	_windows.push_back(win);

	return win;
}

void
vl::gui::GUI::initGUI(vl::Window *window)
{
	std::string message("vl::gui::GUI::_initGUI");
	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	assert(window);

	// TODO support for multiple windows

	// TODO this should be cleanup, should work with any codec or parser...
#ifdef VL_WIN32
	CEGUI::System::setDefaultImageCodecName( "SILLYImageCodec" );
	CEGUI::System::setDefaultXMLParserName( "ExpatParser" );
#endif

	Ogre::RenderWindow *win = window->getRenderWindow();
	assert(win);

	CEGUI::OgreRenderer& myRenderer = CEGUI::OgreRenderer::create(*win);
	CEGUI::System::create(myRenderer);

	_root = CEGUI::WindowManager::getSingleton().createWindow("DefaultWindow", "root");
	CEGUI::System::getSingleton().setGUISheet(_root);
}

void
vl::gui::GUI::initGUIResources(vl::Settings const &settings)
{
	std::string message( "vl::gui::GUI::initGUIResources" );
	Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_TRIVIAL);

	assert( !settings.empty() );
	// Find global project and add those resources as default
	std::string projName("global");
	if( settings.hasProject(projName) )
	{
		vl::ProjSettings const &proj = settings.findProject(projName);
		message = "Found " + projName + " project";
		Ogre::LogManager::getSingleton().logMessage(message);

		fs::path gui_path = fs::path(settings.getDir(proj)) / "gui";
		if( fs::is_directory( gui_path ) )
		{
			addGUIResourceGroup( "schemes", gui_path / "schemes/" );
			addGUIResourceGroup( "imagesets", gui_path / "imagesets/" );
			addGUIResourceGroup( "fonts", gui_path / "fonts/" );
			addGUIResourceGroup( "layouts", gui_path / "layouts/" );
			addGUIResourceGroup( "looknfeels", gui_path / "looknfeel/" );
			addGUIResourceGroup( "lua_scripts", gui_path / "lua_scripts/" );

			// set the default resource groups to be used
			CEGUI::Imageset::setDefaultResourceGroup("imagesets");
			CEGUI::Font::setDefaultResourceGroup("fonts");
			CEGUI::Scheme::setDefaultResourceGroup("schemes");
			CEGUI::WidgetLookManager::setDefaultResourceGroup("looknfeels");
			CEGUI::WindowManager::setDefaultResourceGroup("layouts");
			CEGUI::ScriptModule::setDefaultResourceGroup("lua_scripts");
		}
		else
		{
			message = projName + " Does not have gui resource directory.";
			Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_CRITICAL);
		}
	}
	else
	{
		message = projName + " NOT found. There will be no GUI.";
		Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_CRITICAL);
	}

	// Find editor project and add those resources to groupd editor
	projName = "editor";
	if( settings.hasProject(projName) )
	{
		message = "Found editor project";
		Ogre::LogManager::getSingleton().logMessage(message);
		settings.findProject(projName);

		// TODO really load the editor resources
	}
	else
	{
		message = projName + " NOT found. There will be no Editor.";
		Ogre::LogManager::getSingleton().logMessage(message, Ogre::LML_CRITICAL);
	}

	// Load default data files used for the GUI
	CEGUI::SchemeManager::getSingleton().create( "TaharezLook.scheme" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-7.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-8.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-10.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-9.font" );
	CEGUI::FontManager::getSingleton().create( "DejaVuSans-6.font" );
	CEGUI::System::getSingleton().setDefaultMouseCursor( "TaharezLook", "MouseArrow" );

	// TODO Add project and add those resources to group projectName
}

void
vl::gui::GUI::addGUIResourceGroup( std::string const &name, fs::path const &path )
{
	CEGUI::DefaultResourceProvider *rp = static_cast<CEGUI::DefaultResourceProvider *>
		(CEGUI::System::getSingleton().getResourceProvider());
	assert(rp);

	if( fs::is_directory( path ) )
	{
		std::string message = "GUI resource " + name + " added "
			" with path " + path.string() + ".";
		Ogre::LogManager::getSingleton().logMessage(message);

		rp->setResourceGroupDirectory( name, path.string() );
	}
	else
	{
		std::string message = "GUI resource " + name + " couldn't be added "
			"because path " + path.string() + " does NOT exist.";
		Ogre::LogManager::getSingleton().logMessage(message);
	}
}

bool
vl::gui::GUI::isVisible(void) const
{
	/// @todo should iterate over all windows, and check all that have
	/// value wantsInput in them
	return _console->isVisible() || _editor->isVisible();
}

void 
vl::gui::GUI::sendCommand(std::string const &cmd)
{
	assert(_cmd_cb);
	(*_cmd_cb)(cmd);
}

/// --------------------------------- Private --------------------------------
void
vl::gui::GUI::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
}

void 
vl::gui::GUI::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
}
