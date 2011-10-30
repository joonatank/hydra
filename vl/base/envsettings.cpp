/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2010-11
 *	@file base/envsettings.cpp
 *
 *	This file is part of Hydra VR game engine.
 *
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
	, _stereo(false)
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

vl::Wall
vl::config::EnvSettings::findWall(std::string const &wall_name) const
{
	std::vector<vl::Wall>::const_iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->name == wall_name )
		{ return *iter; }
	}

	return vl::Wall();
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

	return vl::config::Node();
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
	fs::path envFile( getFile() );
	fs::path envDir = envFile.parent_path();
	if( !fs::exists( envDir ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( envDir.string() ) ); }

	return envDir.string();
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

	xml_elem = xml_root->first_node("master");
	if( xml_elem )
	{ processNode( xml_elem, _env->getMaster() ); }

	xml_elem = xml_root->first_node("programs");
	if(xml_elem)
	{ processPrograms(xml_elem); }

	if( _env->getMaster().getNWindows() == 0 )
	{
		std::string str("Master needs to have at least one window.");
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(str) );
	}

	xml_elem = xml_root->first_node("slave");
	while( xml_elem )
	{
		vl::config::Node slave;
		processNode( xml_elem, slave );
		_env->getSlaves().push_back(slave);

		xml_elem = xml_elem->next_sibling("slave");
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
vl::config::EnvSerializer::processCameraRotations( rapidxml::xml_node<>* xml_node )
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
vl::config::EnvSerializer::processWalls( rapidxml::xml_node<>* xml_node )
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
vl::config::EnvSerializer::processServer(rapidxml::xml_node< char > *xml_node)
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
vl::config::EnvSerializer::processNode(rapidxml::xml_node< char > *xml_node, vl::config::Node &node)
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
		attrib = pWindow->first_attribute("w");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no w") ); }
		int w = vl::from_string<int>( attrib->value() );

		attrib = pWindow->first_attribute("h");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no h") ); }
		int h = vl::from_string<int>( attrib->value() );

		attrib = pWindow->first_attribute("x");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no x") ); }
		int x = vl::from_string<int>( attrib->value() );

		attrib = pWindow->first_attribute("y");
		if( !attrib )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no y") ); }
		int y = vl::from_string<int>( attrib->value() );

		vl::config::Window window(name, vl::config::Channel(), w, h, x, y, x);
		window.stereo = _env->hasStereo();
		window.nv_swap_sync = _env->hasNVSwapSync();
		window.nv_swap_group = _env->getNVSwapGroup();
		window.nv_swap_barrier = _env->getNVSwapBarrier();
		attrib = pWindow->first_attribute("display");
		if(attrib)
		window.n_display = vl::from_string<int>( attrib->value() );

		if( attrib = pWindow->first_attribute("vert_sync"))
		{
			window.vert_sync= vl::from_string<bool>(attrib->value());
		}

		rapidxml::xml_node<> *channel_elem = pWindow->first_node("channel");
		processChannel(channel_elem, window);

		// Add the window
		node.addWindow( window );

		pWindow = pWindow->next_sibling("window");
	}

	_checkUniqueNode(xml_node);
}

void
vl::config::EnvSerializer::processChannel( rapidxml::xml_node< char >* xml_node,
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

	rapidxml::xml_node<> *wall_elem = xml_node->first_node("wall");
	if( wall_elem )
	{ wall_name = wall_elem->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no wall") ); }

	window.channel = vl::config::Channel( channel_name, wall_name );
}

void
vl::config::EnvSerializer::processStereo( rapidxml::xml_node<>* xml_node )
{
	bool stereo = vl::from_string<bool>(xml_node->value());

	if(stereo)
	{ _env->setStereo(true); }
	else
	{ _env->setStereo(false); }

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
