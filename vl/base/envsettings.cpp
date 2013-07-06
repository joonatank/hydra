/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/envsettings.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 */

/// Declaration
#include "envsettings.hpp"


#include "filesystem.hpp"
#include "exceptions.hpp"
#include "string_utils.hpp"

#include <iostream>

/// ------------------------ vl::EnvSettings::Node -----------------------------
void
vl::config::Node::addWindow(vl::config::Window const &window)
{
	// Check that there is not already a Window with the same name
	std::vector<Window>::iterator iter;
	for( iter = windows.begin(); iter != windows.end(); ++iter )
	{
		if( iter->name == window.name )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() ); }
	}

	windows.push_back(window);
}

vl::config::Window &
vl::config::Node::getWindow(size_t i)
{ return windows.at(i); }

vl::config::Window const &
vl::config::Node::getWindow( size_t i ) const
{ return windows.at(i); }

/// ------------------------------ vl::EnvSettings -----------------------------
vl::config::EnvSettings::EnvSettings( void )
	: _camera_rotations_allowed( 1 | 1<<1 | 1<<2 )
	, _stereo_type(ST_OFF)
	, _nv_swap_sync(false)
	, _swap_group(0)
	, _swap_barrier(0)
	, _ipd(0)
	, _slave(false)
	, display_n(0)
	, _fps(60)
{}

vl::config::EnvSettings::~EnvSettings( void )
{}

void
vl::config::EnvSettings::clear( void )
{
	_plugins.clear();
	_tracking.clear();

	_camera_rotations_allowed = 0;
}

std::string
vl::config::EnvSettings::getPluginsDirFullPath( void ) const
{
	fs::path env_path = getFile();
	fs::path env_dir = env_path.parent_path();
	fs::path path =  env_dir / "plugins";

	return path.string();
}

void
vl::config::EnvSettings::addPlugin(const std::pair< std::string, bool >& plugin)
{
	std::vector< std::pair<std::string, bool> >::iterator iter;
	for( iter = _plugins.begin(); iter != _plugins.end(); ++iter )
	{
		if( iter->first == plugin.first )
		{ return; }
	}

	_plugins.push_back( plugin );
}

bool
vl::config::EnvSettings::pluginOnOff(const std::string &pluginName, bool newState)
{
	bool found = false;
	for( unsigned int i = 0; i < _plugins.size(); i++ )
	{
		if( _plugins.at(i).first == pluginName )
		{
			_plugins.at(i).second = newState;
			found = true;
			// TODO should return here
			// but there can be same plugin multiple times o_O
		}
	}
	if(!found)
	{
		std::cerr << "Tried to toggle plugin " << pluginName
			<< " which is not present" << std::endl;
		return false;
	}
	return true;
}

std::vector< std::string >
vl::config::EnvSettings::getTrackingFiles( void ) const
{
	std::vector<std::string> vec;

	for( size_t i = 0; i < getTracking().size(); ++i )
	{
		vl::config::Tracking const &track = getTracking().at(i);
		if( track.use )
		{ vec.push_back(track.file); }
	}

	return vec;
}

void
vl::config::EnvSettings::addTracking(vl::config::Tracking const &track)
{
	// Check that we don't add the same file twice
	std::vector<Tracking>::iterator iter;
	for( iter = _tracking.begin(); iter != _tracking.end(); ++iter )
	{
		if( iter->file == track.file )
		{ return; }
	}

	// Not a duplicate
	_tracking.push_back(track);
}

void
vl::config::EnvSettings::removeTracking(std::string const &track)
{
	removeTracking( Tracking(track) );
}

void
vl::config::EnvSettings::removeTracking(Tracking const &track)
{
	std::vector<Tracking>::iterator iter;
	for( iter = _tracking.begin(); iter != _tracking.end(); ++iter )
	{
		if( *iter == track )
		{
			_tracking.erase( iter );
			// TODO this should return here if we assume that no file can exist
			// more than ones.
			return;
		}
	}
}

void
vl::config::EnvSettings::addWall(vl::Wall const &wall)
{
	// Check that there is not already a wall with the same channel_name
	std::vector<Wall>::iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->name == wall.name )
		{
			std::string desc("Trying to add wall with already existing name.");
			BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
		}
	}

	_walls.push_back(wall);
}

vl::Wall const &
vl::config::EnvSettings::getWall(size_t i) const
{
	return _walls.at(i);
}

bool
vl::config::EnvSettings::hasWall(std::string const &name) const
{
	return( findWallPtr(name) != 0 );
}

vl::Wall &
vl::config::EnvSettings::findWall(std::string const &name)
{
	Wall *wall = findWallPtr(name);
	if(wall)
	{ return *wall; }

	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Wall not found."));
}


vl::Wall const &
vl::config::EnvSettings::findWall(std::string const &name) const
{
	Wall const *wall = findWallPtr(name);
	if(wall)
	{ return *wall; }

	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Wall not found."));
}

vl::Wall *
vl::config::EnvSettings::findWallPtr(std::string const &name)
{
	std::vector<vl::Wall>::iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->name == name )
		{ return &(*iter); }
	}

	return 0;
}

vl::Wall const *
vl::config::EnvSettings::findWallPtr(std::string const &name) const
{
	std::vector<vl::Wall>::const_iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->name == name )
		{ return &(*iter); }
	}

	return 0;
}


vl::config::Node const &
vl::config::EnvSettings::findSlave(std::string const &name) const
{
	std::vector<vl::config::Node>::const_iterator iter;
	for( iter = _slaves.begin(); iter != _slaves.end(); ++iter )
	{
		if( iter->name == name )
		{ return *iter; }
	}

	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Slave not found."));
}

std::vector<vl::config::Window *>
vl::config::EnvSettings::get_all_windows()
{
	std::vector<Window *> windows;
	for(size_t i = 0; i < _master.getNWindows(); ++i)
	{
		windows.push_back(&_master.getWindow(i));
	}

	for(size_t i = 0; i < _slaves.size(); ++i)
	{
		for(size_t j = 0; j <_slaves.at(i).windows.size(); ++j)
		{
			windows.push_back(&_slaves.at(i).windows.at(j));
		}
	}

	return windows;
}

std::vector<vl::config::Window const *>
vl::config::EnvSettings::get_all_windows() const
{
	std::vector<Window const *> windows;
	for(size_t i = 0; i < _master.getNWindows(); ++i)
	{
		windows.push_back(&_master.getWindow(i));
	}

	for(size_t i = 0; i < _slaves.size(); ++i)
	{
		for(size_t j = 0; j <_slaves.at(i).windows.size(); ++j)
		{
			windows.push_back(&_slaves.at(i).windows.at(j));
		}
	}

	return windows;
}

std::vector<vl::config::Channel *>
vl::config::EnvSettings::get_all_channels()
{
	std::vector<Channel *> channels;

	std::vector<Window *> const &windows = get_all_windows();

	for(size_t i = 0; i < windows.size(); ++i)
	{
		for(size_t j = 0; j < windows.at(i)->get_n_channels(); ++j)
		{
			channels.push_back(&windows.at(i)->get_channel(j));
		}
	}

	return channels;
}

std::vector<vl::config::Channel const *>
vl::config::EnvSettings::get_all_channels() const
{
	std::vector<Channel const *> channels;

	std::vector<Window const *> const &windows = get_all_windows();

	for(size_t i = 0; i < windows.size(); ++i)
	{
		for(size_t j = 0; j < windows.at(i)->get_n_channels(); ++j)
		{
			channels.push_back(&windows.at(i)->get_channel(j));
		}
	}

	return channels;
}

bool
vl::config::EnvSettings::hasWindow(std::string const &name) const
{
	return( findWindowPtr(name) != 0 );
}

vl::config::Window &
vl::config::EnvSettings::findWindow(std::string const &name)
{
	Window *win = findWindowPtr(name);
	if(win)
	{ return *win; }
	
	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Window not found."));
}

vl::config::Window const &
vl::config::EnvSettings::findWindow(std::string const &name) const
{
	Window const *win = findWindowPtr(name);
	if(win)
	{ return *win; }
	
	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Window not found."));
}

vl::config::Window *
vl::config::EnvSettings::findWindowPtr(std::string const &name)
{
	std::vector<Window *> windows = get_all_windows();
	
	for(size_t i = 0; i < windows.size(); ++i)
	{
		if(windows.at(i)->name == name)
		{ return windows.at(i); }
	}

	return 0;
}

vl::config::Window const *
vl::config::EnvSettings::findWindowPtr(std::string const &name) const
{
	std::vector<Window const *> windows = get_all_windows();
	
	for(size_t i = 0; i < windows.size(); ++i)
	{
		if(windows.at(i)->name == name)
		{ return windows.at(i); }
	}

	return 0;
}

bool
vl::config::EnvSettings::hasChannel(std::string const &name) const
{
	return( findChannelPtr(name) != 0 );
}

vl::config::Channel &
vl::config::EnvSettings::findChannel(std::string const &name)
{
	Channel *chan = findChannelPtr(name);
	if(chan)
	{ return *chan; }
	
	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Channel not found."));
}

vl::config::Channel const &
vl::config::EnvSettings::findChannel(std::string const &name) const
{
	Channel const *chan = findChannelPtr(name);
	if(chan)
	{ return *chan; }
	
	BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Channel not found."));
}

vl::config::Channel *
vl::config::EnvSettings::findChannelPtr(std::string const &name)
{
	std::vector<Channel *> channels = get_all_channels();
	
	for(size_t i = 0; i < channels.size(); ++i)
	{
		if(channels.at(i)->name == name)
		{ return channels.at(i); }
	}

	return 0;
}

vl::config::Channel const *
vl::config::EnvSettings::findChannelPtr(std::string const &name) const
{
	std::vector<Channel const *> channels = get_all_channels();
	
	for(size_t i = 0; i < channels.size(); ++i)
	{
		if(channels.at(i)->name == name)
		{ return channels.at(i); }
	}

	return 0;
}

std::string
vl::config::EnvSettings::getLogDir(vl::PATH_TYPE const type) const
{
	if( type == PATH_REL )
	{ return _log_dir; }
	else
	{
		fs::path path = fs::absolute(_log_dir);
		return path.string();
	}
}

std::string
vl::config::EnvSettings::getEnvironementDir(void) const
{
	if(!getFile().empty())
	{
		fs::path envFile( getFile() );
		fs::path envDir = envFile.parent_path();
	
		if( !fs::exists( envDir ) )
		{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( envDir.string() ) ); }

		return envDir.string();
	}
	
	return std::string();
}

void
vl::config::EnvSettings::addProgram(vl::config::Program const &prog)
{
	/// Check for unique name
	for(size_t i = 0; i < _programs.size(); ++i)
	{
		if( prog.name == _programs.at(i).name )
		{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Program name needs to be unique.")); }
	}

	_programs.push_back(prog);
}
	
std::vector<vl::config::Program>
vl::config::EnvSettings::getUsedPrograms(void) const
{
	std::vector<vl::config::Program> progs;
	for(size_t i = 0; i < _programs.size(); ++i)
	{
		if(_programs.at(i).use)
		{ progs.push_back(_programs.at(i)); }
	}

	return progs;
}

///////////////////////////////////////////////////////////////////////
////////////////////// --- EnvSettingsSerializer --- //////////////////
///////////////////////////////////////////////////////////////////////

vl::config::EnvSerializer::EnvSerializer(vl::config::EnvSettingsRefPtr env)
    : _env(env), _xml_data(0)
{}


vl::config::EnvSerializer::~EnvSerializer(void)
{
    delete [] _xml_data;
}

bool
vl::config::EnvSerializer::readString(std::string const &str)
{
    delete[] _xml_data;
    size_t length = str.length() + 1;
    _xml_data = new char[length];
    memcpy(_xml_data, str.c_str(), length);

    return readXML();
}

bool
vl::config::EnvSerializer::readXML()
{
    rapidxml::xml_document<> xmlDoc;
    xmlDoc.parse<0>( _xml_data );

    rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("env_config");
    if( !xmlRoot )
    {
        std::cerr << "Errenous xml. env_config node missing. Wrong file?" << std::endl;
        return false;
    }
    
	processConfig( xmlRoot );

	finalise();

    return true;
}

void
vl::config::EnvSerializer::processConfig( rapidxml::xml_node<>* xml_root )
{
	rapidxml::xml_node<>* xml_elem;

	xml_elem = xml_root->first_node("plugins");
	if( xml_elem )
	{ processPlugins( xml_elem ); }

	xml_elem = xml_root->first_node("tracking");
	if( xml_elem )
	{ processTracking( xml_elem ); }

	xml_elem = xml_root->first_node("camera_rotations");
	if( xml_elem )
	{ processCameraRotations( xml_elem ); }

	xml_elem = xml_root->first_node("walls");
	if( xml_elem )
	{ processWalls( xml_elem ); }

	xml_elem = xml_root->first_node("server");
	if( xml_elem )
	{ processServer( xml_elem ); }

	// These need to be befoce processing Windows e.g. master and slave
	xml_elem = xml_root->first_node("stereo");
	if( xml_elem )
	{ processStereo( xml_elem ); }

	xml_elem = xml_root->first_node("nv_swap_sync");
	if( xml_elem )
	{ processNVSwapSync( xml_elem ); }

	xml_elem = xml_root->first_node("ipd");
	if( xml_elem )
	{ processIPD( xml_elem ); }

	xml_elem = xml_root->first_node("fps");
	if( xml_elem )
	{ processFPS( xml_elem ); }

	// renderer and projection needs to be processed before Nodes (master and slave)
	// because the global configs are copied to every node
	xml_elem = xml_root->first_node("renderer");
	if( xml_elem )
	{ processRenderer(xml_elem, _env->getRenderer()); }

	xml_elem = xml_root->first_node("projection");
	if( xml_elem )
	{ processProjection(xml_elem, _env->getRenderer().projection); }

	xml_elem = xml_root->first_node("master");
	if( xml_elem )
	{
		processNode( xml_elem, _env->getMaster() );
		_env->getMaster().gui_enabled = true;
	}

	xml_elem = xml_root->first_node("programs");
	if(xml_elem)
	{ processPrograms(xml_elem); }

	xml_elem = xml_root->first_node("slave");
	while( xml_elem )
	{
		vl::config::Node slave;
		processNode( xml_elem, slave );
		_env->getSlaves().push_back(slave);

		xml_elem = xml_elem->next_sibling("slave");
	}

	if( _env->getMaster().getNWindows() == 0 )
	{
		std::clog << "Warning! : Master without windows." << std::endl;
		// GUI is available on master (because it has no window) 
		// so enable it on first slave that has a Window
		for(size_t i = 0; i < _env->getSlaves().size(); ++i)
		{
			if(_env->getSlaves().at(i).getNWindows() > 0)
			{
				std::clog << "Enabling GUI on slave : " << _env->getSlaves().at(i).name << std::endl;
				_env->getSlaves().at(i).gui_enabled = true;
				break;
			}
		}
	}

}


void
vl::config::EnvSerializer::processPlugins( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *pElement = xml_node->first_node("plugin");

	if( !pElement )
	{
		std::cerr << "Plugins list missing from env_config node." << std::endl;
		return;
	}

	std::pair<std::string, bool> plugin;
	std::string useStr;
	std::string name;
	bool use;
	rapidxml::xml_attribute<> *attrib;

	while( pElement )
	{
		attrib = pElement->first_attribute("use");
		if( !attrib )
		{
			std::cerr << "Missing use attrib. Defaulting to false." << std::endl;
			useStr = "false";
		}
		else
		{
			useStr = attrib->value();
		}

		if( useStr == "true" )
		{ use = true; }
		else if( useStr == "false" )
		{ use = false; }
		else
		{
			std::cerr << "One plugin has errenous use attribute. Defaulting to false." << std::endl;
			use = false;
		}
		name = pElement->value();
		plugin = std::make_pair( name, use );
		_env->addPlugin(plugin);

		pElement = pElement->next_sibling("plugin");
	}
}

void
vl::config::EnvSerializer::processTracking( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *pElement = xml_node->first_node("file");

	while( pElement )
	{
		bool use = true;
		rapidxml::xml_attribute<> *attrib = pElement->first_attribute("use");
		if( attrib && std::string( attrib->value() ) == "false" )
		{
			use = false;
		}

		_env->addTracking( vl::config::Tracking(pElement->value(), use) );
		pElement = pElement->next_sibling("file");
	}
}

void
vl::config::EnvSerializer::processCameraRotations(rapidxml::xml_node<> *xml_node)
{
	uint32_t flags = 0;
	std::string const F("false");

	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("x");
	// Defautls to true
	if( !attrib || std::string(attrib->value()) != F )
	{ flags |= 1; }

	attrib = xml_node->first_attribute("y");
	// Defautls to true
	if( !attrib || std::string(attrib->value()) != F )
	{ flags |= 1<<1; }

	attrib = xml_node->first_attribute("z");
	// Defautls to true
	if( !attrib || std::string(attrib->value()) != F )
	{ flags |= 1<<2; }

	_env->setCameraRotationAllowed(flags);
}

void
vl::config::EnvSerializer::processWalls(rapidxml::xml_node<>* xml_node)
{
	rapidxml::xml_node<> *pWall = xml_node->first_node("wall");

	while( pWall )
	{
		std::string name("default-wall");

		rapidxml::xml_attribute<> *attrib = pWall->first_attribute("name");
		if( attrib )
		{ name = attrib->value(); }
		else
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("wall has no name") ); }

		// Process bottom_left
		rapidxml::xml_node<> *pElement = pWall->first_node("bottom_left");
		if( !pElement )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no bottom_left") ); }
		std::vector<double> bottom_left = getVector( pElement );

		// Process bottom_right
		pElement = pWall->first_node("bottom_right");
		if( !pElement )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no bottom_right") ); }
		std::vector<double> bottom_right = getVector( pElement );

		// Process top_left
		pElement = pWall->first_node("top_left");
		if( !pElement )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no top_left") ); }
		std::vector<double> top_left = getVector( pElement );

		// Add the wall
		vl:Wall wall( name, bottom_left, bottom_right, top_left );
		_env->addWall( wall );

		pWall = pWall->next_sibling("wall");
	}

	if( xml_node->next_sibling("walls") )
	{
		std::string desc( "Only one walls token is supported" );
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

void
vl::config::EnvSerializer::processServer(rapidxml::xml_node<> *xml_node)
{
	std::string hostname;
	uint16_t port;
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("hostname");
	if( attrib )
	{
		hostname = attrib->value();
	}

	attrib = xml_node->first_attribute("port");
	if( attrib )
	{
		port = vl::from_string<uint16_t>( attrib->value() );
	}

	_env->setServer(vl::config::Server(port, hostname));
}

void
vl::config::EnvSerializer::processNode(rapidxml::xml_node<> *xml_node, vl::config::Node &node)
{
	std::string name;
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("name");
	if( attrib )
	{ name = attrib->value(); }

	node.name = name;
	rapidxml::xml_node<> *xml_elem = xml_node->first_node("windows");
	if( xml_elem )
	{ processWindows( xml_elem, node ); }
}

void
vl::config::EnvSerializer::processWindows(rapidxml::xml_node<> *xml_node, vl::config::Node &node)
{
	rapidxml::xml_node<> *pWindow = xml_node->first_node("window");

	while( pWindow )
	{
		rapidxml::xml_attribute<> *attrib = pWindow->first_attribute("name");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no name") ); }
		std::string name = attrib->value();

		// Process w, h, x, y
		// @todo these should have default values
		Rect<int> area;
		attrib = pWindow->first_attribute("w");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no w") ); }
		area.w = vl::from_string<int>( attrib->value() );

		attrib = pWindow->first_attribute("h");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no h") ); }
		area.h = vl::from_string<int>( attrib->value() );

		attrib = pWindow->first_attribute("x");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no x") ); }
		area.x = vl::from_string<int>( attrib->value() );

		attrib = pWindow->first_attribute("y");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no y") ); }
		area.y = vl::from_string<int>( attrib->value() );

		vl::config::Window window(name, area);
		
		// Copy env settings values
		window.stereo_type = _env->getStereoType();
		// @todo we can override stereo type in window config
		// xml node stereo, attribute "type" and "use"
		rapidxml::xml_node<> *xml_stereo = pWindow->first_node("stereo");
		if(xml_stereo)
		{
			attrib = xml_stereo->first_attribute("type");
			if(attrib)
			{
				std::string type_str(attrib->value());
				vl::to_lower(type_str);
				if(type_str == "side_by_side")
				{ window.stereo_type = ST_SIDE_BY_SIDE; }
				else if(type_str == "quad_buffer" || type_str.empty())
				{ window.stereo_type = ST_QUAD_BUFFER; }
				else if(type_str == "top_bottom")
				{
					std::clog << "EnvConfig : stereo type is top bottom" << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Top bottom stereo is not supported."));
				}
				else
				{
					std::clog << "EnvConfig : stereo type " << type_str << std::endl;
					BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Stereo type not recognised."));
				}
			}
			// @todo add use attrib
		}

		window.nv_swap_sync = _env->hasNVSwapSync();
		window.nv_swap_group = _env->getNVSwapGroup();
		window.nv_swap_barrier = _env->getNVSwapBarrier();
		attrib = pWindow->first_attribute("display");
		if(attrib)
		{ window.n_display = vl::from_string<int>( attrib->value() ); }

		attrib = pWindow->first_attribute("vert_sync");
		if(attrib)
		{ window.vert_sync= vl::from_string<bool>(attrib->value()); }

		attrib = pWindow->first_attribute("fsaa");
		if(attrib)
		{ window.fsaa = vl::from_string<int>(attrib->value()); }

		/// Process channels
		rapidxml::xml_node<> *channel_elem = pWindow->first_node("channel");
		while(channel_elem)
		{
			processChannel(channel_elem, window);
			channel_elem = channel_elem->next_sibling(channel_elem->name());
		}

		// Copy render parameters
		window.renderer = _env->getRenderer();

		// Add the window
		node.addWindow( window );

		pWindow = pWindow->next_sibling("window");
	}

	_checkUniqueNode(xml_node);
}

void
vl::config::EnvSerializer::processChannel( rapidxml::xml_node<>* xml_node,
										   vl::config::Window& window )
{
	assert( xml_node );

	std::string channel_name;
	std::string wall_name;

	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("name");
	if( attrib )
	{ channel_name = attrib->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no name") ); }

	Rect<double> r(1, 1, 0, 0);
	attrib = xml_node->first_attribute("w");
	if(attrib)
	{ r.w = vl::from_string<double>(attrib->value()); }
	attrib = xml_node->first_attribute("h");
	if(attrib)
	{ r.h = vl::from_string<double>(attrib->value()); }
	attrib = xml_node->first_attribute("x");
	if(attrib)
	{ r.x = vl::from_string<double>(attrib->value()); }
	attrib = xml_node->first_attribute("y");
	if(attrib)
	{ r.y = vl::from_string<double>(attrib->value()); }

	Ogre::ColourValue background_col(0, 0, 0);
	rapidxml::xml_node<> *background = xml_node->first_node("background");
	if(background)
	{
		std::clog << "EnvConfig : Processing channel" << std::endl;
		background_col = parseColour(background);
	}

	rapidxml::xml_node<> *wall_elem = xml_node->first_node("wall");
	if( wall_elem )
	{ wall_name = wall_elem->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no wall") ); }

	
	window.add_channel(Channel(channel_name, r, Wall(), background_col));
	// this needs to be a reference because we use it to set the proper
	// channel later on
	// Nope we should not add pointers because window is a temporary object
	_wall_map[channel_name] = wall_name;
}

void
vl::config::EnvSerializer::processStereo( rapidxml::xml_node<>* xml_node )
{
	bool stereo = vl::from_string<bool>(xml_node->value());

	// @todo this is bad it's confusing when side-by-side stereo is used
	if(stereo)
	{ _env->setStereoType(ST_QUAD_BUFFER); }
	else
	{ _env->setStereoType(ST_OFF); }

	_checkUniqueNode(xml_node);
}

void 
vl::config::EnvSerializer::processNVSwapSync(rapidxml::xml_node<> *xml_node)
{
	rapidxml::xml_node<> *group_elem = xml_node->first_node("swap_group");
	rapidxml::xml_node<> *barrier_elem = xml_node->first_node("swap_barrier");

	if(group_elem)
	{
		_env->setNVSwapSync(true);
		uint32_t group = vl::from_string<uint32_t>(group_elem->value());
		std::cout << "Setting swap group to = " << group << std::endl;
		_env->setNVSwapGroup(group);
	}

	if(barrier_elem)
	{
		_env->setNVSwapSync(true); 
		uint32_t barrier = vl::from_string<uint32_t>(barrier_elem->value());
		std::cout << "Setting swap barrier to = " << barrier << std::endl;
		_env->setNVSwapBarrier(barrier);
	}

	_checkUniqueNode(xml_node);
}

void
vl::config::EnvSerializer::processIPD(rapidxml::xml_node<>* xml_node)
{
	/// IPD can be either negative or positive doesn't matter
	/// if it's negative it will do swap eyes
	double ipd = vl::from_string<double>(xml_node->value());

	_env->setIPD(ipd);

	_checkUniqueNode(xml_node);
}

void
vl::config::EnvSerializer::processFPS(rapidxml::xml_node<> *xml_node)
{
	uint32_t fps = vl::from_string<uint32_t>(xml_node->value());

	_env->setFPS(fps);

	_checkUniqueNode(xml_node);
}

void
vl::config::EnvSerializer::processPrograms(rapidxml::xml_node<> *xml_node)
{
	rapidxml::xml_node<> *xml_elem = xml_node->first_node("program");
	// @todo should check for the autolaunch attribute
	while(xml_elem)
	{
		processProgram(xml_elem);
		xml_elem = xml_elem->next_sibling("program");
	}

	_checkUniqueNode(xml_node);
}

void
vl::config::EnvSerializer::processProgram(rapidxml::xml_node<> *xml_node)
{
	vl::config::Program prog;

	// Attributes: use, name, directory, command
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("use");
	if(attrib)
	{ prog.use = vl::from_string<bool>(attrib->value()); }

	attrib = xml_node->first_attribute("new_console");
	if(attrib)
	{ prog.new_console = vl::from_string<bool>(attrib->value()); }
	
	attrib = xml_node->first_attribute("name");
	if(!attrib)
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("name is a necessary attribute for Program") ); }
	prog.name = attrib->value();

	attrib = xml_node->first_attribute("command");
	if(!attrib)
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("command is a necessary attribute for Program") ); }
	prog.command = attrib->value();

	attrib = xml_node->first_attribute("directory");
	if(attrib)
	{ prog.directory = attrib->value(); }

	// Get params
	rapidxml::xml_node<> *xml_elem = xml_node->first_node("param");
	while(xml_elem)
	{
		if(xml_elem->value() && xml_elem->value() != "")
		{ prog.params.push_back(xml_elem->value()); }

		xml_elem = xml_elem->next_sibling("param");
	}

	_env->addProgram(prog);
}

void
vl::config::EnvSerializer::processRenderer(rapidxml::xml_node<> *xml_node, vl::config::Renderer &renderer)
{
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("type");
	if(attrib)
	{
		std::string type(attrib->value());
		vl::to_lower(type);
		if(type == "fbo")
		{
			renderer.type = Renderer::FBO;
		}
		else if(type == "deferred")
		{
			renderer.type = Renderer::DEFERRED;
		}
		else
		{
			// Assume type window otherwise
			renderer.type = Renderer::WINDOW;
		}
	}

	attrib = xml_node->first_attribute("hardware_gamma");
	if(attrib)
	{
		renderer.hardware_gamma = vl::from_string<bool>(attrib->value());
	}
}

void
vl::config::EnvSerializer::processProjection(rapidxml::xml_node<> *xml_node, vl::config::Projection &projection)
{
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("type");
	if(attrib)
	{
		std::string type(attrib->value());
		vl::to_lower(type);
		if(type == "ortho")
		{
			projection.type = Projection::ORTHO;
		}
		else
		{
			// Assume perspective otherwise
			projection.type = Projection::PERSPECTIVE;
		}
	}

	rapidxml::xml_node<> *xml_pers = xml_node->first_node("perspective");
	if(xml_pers)
	{
		attrib = xml_pers->first_attribute("type");
		if(attrib)
		{
			// Process perspective type
			std::string type(attrib->value());
			vl::to_lower(type);
			if(type == "fov")
			{
				projection.perspective_type = Projection::FOV;
			}
			else if(type == "wall")
			{
				projection.perspective_type = Projection::WALL;
			}
		}

		attrib = xml_pers->first_attribute("asymmetric_stereo_frustum");
		if(attrib)
		{
			projection.use_asymmetric_stereo = vl::from_string<bool>(attrib->value());
		}

		rapidxml::xml_node<> *wall = xml_pers->first_node("wall");
		rapidxml::xml_node<> *fov = xml_pers->first_node("fov");
		if(wall)
		{
			rapidxml::xml_node<> *head = wall->first_node("head");
			if(head)
			{
				attrib = head->first_attribute("x");
				if(attrib)
				{ projection.head_x = vl::from_string<bool>(attrib->value()); }
				attrib = head->first_attribute("y");
				if(attrib)
				{ projection.head_y = vl::from_string<bool>(attrib->value()); }
				attrib = head->first_attribute("z");
				if(attrib)
				{ projection.head_z = vl::from_string<bool>(attrib->value()); }
			}

			rapidxml::xml_node<> *modify = wall->first_node("modify_transformations");
			if(modify)
			{
				attrib = modify->first_attribute("use");
				if(attrib)
				{ projection.modify_transformations = vl::from_string<bool>(attrib->value()); }
			}
		}

		if(fov)
		{
			attrib = fov->first_attribute("angle");
			if(attrib)
			{ projection.fov = vl::from_string<double>(attrib->value()); }
			attrib = fov->first_attribute("horizontal");
			if(attrib)
			{ projection.horizontal = vl::from_string<double>(attrib->value()); }
		}
	}
}


void
vl::config::EnvSerializer::finalise(void)
{
	/// Set the walls
	for(std::map<std::string, std::string>::iterator iter = _wall_map.begin();
		iter != _wall_map.end(); ++iter)
	{
		// @todo this will throw if the Channel or Wall can not be found by name
		Channel &chan = _env->findChannel(iter->first);
		Wall const &wall = _env->findWall(iter->second);
		chan.wall = wall;
	}
}

std::vector<double>
vl::config::EnvSerializer::getVector( rapidxml::xml_node<>* xml_node )
{
	std::vector<double> tmp(3);

	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute("x");
	if( !attrib || ::strlen( attrib->value() ) == 0 )
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("x in vector") ); }
	tmp.at(0) = vl::from_string<double>( attrib->value() );

	attrib = xml_node->first_attribute("y");
	if( !attrib || ::strlen( attrib->value() ) == 0 )
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("y in vector") ); }
	tmp.at(1) = vl::from_string<double>( attrib->value() );

	attrib = xml_node->first_attribute("z");
	if( !attrib || ::strlen( attrib->value() ) == 0 )
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("z in vector") ); }
	tmp.at(2) = vl::from_string<double>( attrib->value() );

	return tmp;
}

void 
vl::config::EnvSerializer::_checkUniqueNode(rapidxml::xml_node<> *xml_node)
{
	assert(xml_node);

	if( xml_node->next_sibling(xml_node->name()) )
	{
		std::string desc = "Only one " + std::string(xml_node->name()) + " token is supported";
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}
