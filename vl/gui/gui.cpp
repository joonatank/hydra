/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file gui.cpp
 *
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
	: _editor_shown(false)
	, _console_shown(false)
	, _stats_shown(false)
	, _loading_screen_shown(false)
{
	session->registerObject(this, OBJ_GUI);
}

vl::gui::GUI::GUI(vl::Session *session, uint64_t id, vl::CommandCallback *cb)
	: _editor_shown(false)
	, _console_shown(false)
	, _stats_shown(false)
	, _loading_screen_shown(false)
	, _cmd_cb(cb)
{
	if(id == vl::ID_UNDEFINED)
	{ BOOST_THROW_EXCEPTION(vl::invalid_id()); }

	session->registerObject(this, OBJ_GUI, id);

	/// @todo the slave version should hold CEGUI instance and create the windows
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

void
vl::gui::GUI::createGUI(void )
{
	std::cout << vl::TRACE << "vl::gui::GUI::createGUI" << std::endl;

	/// @todo should create the Window wrappers
	CEGUI::Window *myRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout( "editor.layout" );
	CEGUI::System::getSingleton().setGUISheet( myRoot );
	CEGUI::Window *editor = myRoot->getChild("editor");
	_editor.reset(new EditorWindow(editor, this));

	CEGUI::Window *console = CEGUI::WindowManager::getSingleton().loadWindowLayout( "console.layout" );
	myRoot->addChildWindow(console);
	_console.reset(new ConsoleWindow(console, this));

	CEGUI::Window *loading_screen = CEGUI::WindowManager::getSingleton().loadWindowLayout( "loading_screen.layout" );
	myRoot->addChildWindow(loading_screen);
	loading_screen->hide();
	_loading_screen.reset(new Window(loading_screen, this));

	CEGUI::Window *stats = CEGUI::WindowManager::getSingleton().loadWindowLayout( "stats.layout" );
	myRoot->addChildWindow(stats);
	stats->hide();
	_stats.reset(new Window(stats, this));

	// TODO support for multiple windows
	// at the moment every window will get the same GUI window layout
}


void 
vl::gui::GUI::setEditorVisibility(bool vis)
{
	if(_editor_shown != vis)
	{
		setDirty(DIRTY_EDITOR);
		_editor_shown = vis;
	}
}

void 
vl::gui::GUI::setConsoleVisibility(bool vis)
{
	if(_console_shown != vis)
	{
		setDirty(DIRTY_CONSOLE);
		_console_shown = vis;
	}
}

void 
vl::gui::GUI::setStatsVisibility(bool vis)
{
	if(_stats_shown != vis)
	{
		setDirty(DIRTY_STATS);
		_stats_shown = vis;
	}
}

void 
vl::gui::GUI::setLoadingScreenVisibility(bool vis)
{
	if(_loading_screen_shown != vis)
	{
		setDirty(DIRTY_LOADING_SCREEN);
		_loading_screen_shown = vis;
	}
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
	if( DIRTY_EDITOR & dirtyBits )
	{
		msg << _editor_shown;
	}

	if( DIRTY_CONSOLE & dirtyBits )
	{
		msg << _console_shown;
	}

	if( DIRTY_STATS & dirtyBits )
	{
		msg << _stats_shown;
	}

	if( DIRTY_LOADING_SCREEN & dirtyBits )
	{
		msg << _loading_screen_shown;
	}
}

void 
vl::gui::GUI::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( DIRTY_EDITOR & dirtyBits )
	{
		msg >> _editor_shown;
		if( _editor )
		{ _editor->setVisible(_editor_shown); }
	}

	if( DIRTY_CONSOLE & dirtyBits )
	{
		msg >> _console_shown;
		if( _console )
		{ _console->setVisible(_console_shown); }
	}

	if( DIRTY_STATS & dirtyBits )
	{
		msg >> _stats_shown;
		if( _stats )
		{ _stats->setVisible(_stats_shown); }
	}

	if( DIRTY_LOADING_SCREEN & dirtyBits )
	{
		msg >> _loading_screen_shown;
		if( _loading_screen )
		{ _loading_screen->setVisible(_loading_screen_shown); }
	}
}
