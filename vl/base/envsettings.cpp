/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 * 2010-11
 */


/// Declaration
#include "envsettings.hpp"


#include "filesystem.hpp"
#include "exceptions.hpp"
#include "string_utils.hpp"

#include <iostream>

/// ------------------------ vl::EnvSettings::Node -----------------------------
void
vl::EnvSettings::Node::addWindow( vl::EnvSettings::Window const &window )
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

vl::EnvSettings::Window const &
vl::EnvSettings::Node::getWindow( size_t i ) const
{
	return windows.at(i);
}


/// ------------------------------ vl::EnvSettings -----------------------------
vl::EnvSettings::EnvSettings( void )
	: _camera_rotations_allowed( 1 | 1<<1 | 1<<2 ), _slave(false)
{}

vl::EnvSettings::~EnvSettings( void )
{}

void
vl::EnvSettings::clear( void )
{
// 	_eqc.clear();

	_plugins.clear();
	_tracking.clear();

	_camera_rotations_allowed = 0;
}

// std::string
// vl::EnvSettings::getEqcFullPath( void ) const
// {
// 	if( getEqc().empty() )
// 	{ return std::string(); }
//
// 	fs::path env_path = getFile();
// 	fs::path env_dir = env_path.parent_path();
// 	fs::path path =  env_dir / "eqc" / getEqc();
//
// 	return path.file_string();
// }

std::string
vl::EnvSettings::getPluginsDirFullPath( void ) const
{
	fs::path env_path = getFile();
	fs::path env_dir = env_path.parent_path();
	fs::path path =  env_dir / "plugins";

	return path.file_string();
}

void
vl::EnvSettings::addPlugin(const std::pair< std::string, bool >& plugin)
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
vl::EnvSettings::pluginOnOff(const std::string &pluginName, bool newState)
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
vl::EnvSettings::getTrackingFiles( void ) const
{
	std::vector<std::string> vec;

	for( size_t i = 0; i < getTracking().size(); ++i )
	{
		EnvSettings::Tracking const &track = getTracking().at(i);
		if( track.use )
		{ vec.push_back(track.file); }
	}

	return vec;
}

void
vl::EnvSettings::addTracking(const vl::EnvSettings::Tracking& track)
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
vl::EnvSettings::removeTracking( std::string const &track )
{
	removeTracking( Tracking(track) );
}

void
vl::EnvSettings::removeTracking( Tracking const &track )
{
	std::vector<Tracking>::iterator iter;
	for( iter = _tracking.begin(); iter != _tracking.end(); ++iter )
	{
		if( iter->file == track.file )
		{
			_tracking.erase( iter );
			// TODO this should return here if we assume that no file can exist
			// more than ones.
			return;
		}
	}
}

void
vl::EnvSettings::addWall( vl::EnvSettings::Wall const &wall )
{
	// Check that there is not already a wall with the same channel_name
	std::vector<Wall>::iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->channel_name == wall.channel_name )
		{
			std::string desc("Trying to add multiple walls to same channel.");
			BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
		}
	}

	_walls.push_back(wall);
}

vl::EnvSettings::Wall const &
vl::EnvSettings::getWall( size_t i ) const
{
	return _walls.at(i);
}

vl::EnvSettings::Wall
vl::EnvSettings::findWall( std::string const &channel_name ) const
{
	std::vector<vl::EnvSettings::Wall>::const_iterator iter;
	for( iter = _walls.begin(); iter != _walls.end(); ++iter )
	{
		if( iter->channel_name == channel_name )
		{ return *iter; }
	}

	return vl::EnvSettings::Wall();
}

std::string
vl::EnvSettings::getLogDir( vl::PATH_TYPE const type ) const
{
	if( type == PATH_REL )
	{ return _log_dir; }
	else
	{
		fs::path path = fs::complete( _log_dir );
		return path.file_string();
	}
}

void
vl::EnvSettings::setExePath( std::string const &path )
{
	_exe_path = path;
}

std::string
vl::EnvSettings::getEnvironementDir( void ) const
{
	fs::path envFile( getFile() );
	fs::path envDir = envFile.parent_path();
	if( !fs::exists( envDir ) )
	{ BOOST_THROW_EXCEPTION( vl::missing_dir() << vl::file_name( envDir.file_string() ) ); }

	return envDir.file_string();
}


///////////////////////////////////////////////////////////////////////
////////////////////// --- EnvSettingsSerializer --- //////////////////
///////////////////////////////////////////////////////////////////////

vl::EnvSettingsSerializer::EnvSettingsSerializer( EnvSettingsRefPtr envSettingsPtr )
    : _envSettings(envSettingsPtr), _xml_data(0)
{
}


vl::EnvSettingsSerializer::~EnvSettingsSerializer( void )
{
    delete [] _xml_data;
}

bool
vl::EnvSettingsSerializer::readString( std::string const &str )
{
    delete[] _xml_data;
    size_t length = str.length() + 1;
    _xml_data = new char[length];
    memcpy(_xml_data, str.c_str(), length);

    return readXML();
}

bool
vl::EnvSettingsSerializer::readXML()
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
vl::EnvSettingsSerializer::processConfig( rapidxml::xml_node<>* xml_root )
{
	rapidxml::xml_node<>* xml_elem;

	xml_elem = xml_root->first_node("plugins");
	if( xml_elem )
	{ processPlugins( xml_elem ); }

// 	xml_elem = xml_root->first_node("eqc");
// 	if( xml_elem )
// 	{ processEqc( xml_elem ); }

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

	xml_elem = xml_root->first_node("master");
	if( xml_elem )
	{ processNode( xml_elem, _envSettings->getMaster() ); }

	xml_elem = xml_root->first_node("slave");
	while( xml_elem )
	{
		EnvSettings::Node slave;
		processNode( xml_elem, slave );
		_envSettings->getSlaves().push_back(slave);

		xml_elem = xml_elem->next_sibling("slave");
	}

	xml_elem = xml_root->first_node("stereo");
	if( xml_elem )
	{ processStereo( xml_elem ); }

	xml_elem = xml_root->first_node("ipd");
	if( xml_elem )
	{ processIPD( xml_elem ); }
}


void
vl::EnvSettingsSerializer::processPlugins( rapidxml::xml_node<>* xml_node )
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
		_envSettings->addPlugin(plugin);

		pElement = pElement->next_sibling("plugin");
	}
}

// void
// vl::EnvSettingsSerializer::processEqc( rapidxml::xml_node<>* xml_node )
// {
//     rapidxml::xml_node<> *pElement = xml_node->first_node("file");
//
//     if( !pElement )
//     { return; }
//
//     _envSettings->setEqc( pElement->value() );
//
//     // No more than one eqc
//     pElement = pElement->next_sibling("file");
//     if( pElement )
//     {
//         std::cerr << "More than one eqc file? Only first one taken." << std::endl;
//         return;
//     }
// }


void
vl::EnvSettingsSerializer::processTracking( rapidxml::xml_node<>* xml_node )
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

		_envSettings->addTracking( EnvSettings::Tracking(pElement->value(), use) );
		pElement = pElement->next_sibling("file");
	}
}

void
vl::EnvSettingsSerializer::processCameraRotations( rapidxml::xml_node<>* xml_node )
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

	_envSettings->setCameraRotationAllowed(flags);
}

void
vl::EnvSettingsSerializer::processWalls( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *pWall = xml_node->first_node("wall");

	while( pWall )
	{
		std::string name("default-wall");

		rapidxml::xml_attribute<> *attrib = pWall->first_attribute("name");
		if( attrib )
		{ name = attrib->value(); }

		// Process channel
		rapidxml::xml_node<> *pElement = pWall->first_node("channel");
		if( !pElement )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no channel") ); }
		attrib = pElement->first_attribute("name");
		if( !attrib || ::strlen( attrib->value() ) == 0 )
		{ BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc("no channel name") ); }
		std::string channel = attrib->value();

		// Process bottom_left
		pElement = pWall->first_node("bottom_left");
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
		EnvSettings::Wall wall( name, channel, bottom_left, bottom_right, top_left );
		_envSettings->addWall( wall );

		pWall = pWall->next_sibling("wall");
	}

	if( xml_node->next_sibling("walls") )
	{
		std::string desc( "Only one walls token is supported" );
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

void
vl::EnvSettingsSerializer::processServer( rapidxml::xml_node< char > *xml_node )
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

	_envSettings->setServer( EnvSettings::Server( port, hostname ) );
}

void
vl::EnvSettingsSerializer::processNode( rapidxml::xml_node< char > *xml_node, vl::EnvSettings::Node& node )
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
vl::EnvSettingsSerializer::processWindows( rapidxml::xml_node<> *xml_node, EnvSettings::Node &node )
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

		// Find wall reference
		std::string wall_name;
		rapidxml::xml_node<> *wall_elem = pWindow->first_node("wall");
		if( wall_elem )
		{ wall_name = wall_elem->value(); }

		// Add the window
		EnvSettings::Window window( name, wall_name, w, h, x, y );
		node.addWindow( window );

		pWindow = pWindow->next_sibling("window");
	}

	if( xml_node->next_sibling("windows") )
	{
		std::string desc( "Only one windows token is supported" );
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

void
vl::EnvSettingsSerializer::processStereo( rapidxml::xml_node<>* xml_node )
{
	EnvSettings::CFG cfg_val = EnvSettings::ON;
	std::string stereo = xml_node->value();
	vl::to_lower(stereo);
	if( stereo == "on" )
	{}
	else if( stereo == "off" )
	{
		cfg_val = EnvSettings::OFF;
	}
	else if( stereo == "required" )
	{
		cfg_val = EnvSettings::REQUIRED;
	}

	_envSettings->setStereo( cfg_val );

	if( xml_node->next_sibling("stereo") )
	{
		std::string desc( "Only one stereo token is supported" );
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

void
vl::EnvSettingsSerializer::processIPD( rapidxml::xml_node<>* xml_node )
{
	double ipd = vl::from_string<double>(xml_node->value());
	if( ipd < 0 )
	{
		std::string desc( "IPD can not be less than zero." );
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}

	_envSettings->setIPD( ipd);

	if( xml_node->next_sibling("stereo") )
	{
		std::string desc( "Only one IPD token is supported" );
		BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
	}
}

std::vector<double>
vl::EnvSettingsSerializer::getVector( rapidxml::xml_node<>* xml_node )
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
