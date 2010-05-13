#include "settings.hpp"

#include "base/filesystem.hpp"
#include "base/exceptions.hpp"

#include <iostream>
#include <fstream>

vl::Settings::Settings( void )
{
}

vl::Settings::~Settings( void )
{
}

void
vl::Settings::setExePath( std::string const &path )
{
	_exe_path = path;

	updateArgs();
}

void
vl::Settings::setEqConfig( Settings::Eqc const &eqc )
{
	_eq_config = eqc;

	updateArgs();
}

void
vl::Settings::addPlugins( Settings::Plugins const &plugins )
{
	_plugins = plugins;
}

void
vl::Settings::addResources( Settings::Resources const &resource )
{
	_resources.push_back( resource );
}

void
vl::Settings::addScene( Settings::Scene const &scene )
{
	_scenes.push_back( scene );
}

// --------- Settings Private --------
void
vl::Settings::updateArgs( void )
{
	// Update args
	_eq_args.clear();
	if( !_exe_path.empty() )
	{ _eq_args.add( _exe_path.string().c_str() ); }

	fs::path path = _eq_config.getPath();
	if( !path.empty() )
	{
		_eq_args.add( "--eq-config" );
		_eq_args.add( path.string().c_str() );
	}
}

// --- SettingsSerializer ---
vl::SettingsSerializer::SettingsSerializer( Settings *settings )
	: _settings(settings), _xml_data(0)
{
}

vl::SettingsSerializer::~SettingsSerializer( void )
{
	delete _xml_data;
}

void
vl::SettingsSerializer::readFile( std::string const &file_path )
{
	if( !fs::exists( file_path ) )
	{ throw vl::missing_file( "vl::SettingsSerializer::readFile" ); }
	
	// Set the file path for the last file loaded, so settings can be saved
	// back to that file.
	_settings->_file_path = file_path;

	// Open in binary mode, so we don't mess up the file
	std::ifstream stream( file_path.c_str(), std::ios::binary );

	// Read the stream using FileStream class
	delete _xml_data;
	_xml_data = new vl::FileString( );
	_xml_data->readStream( stream );

	// Pass the data to dataReader.
	readData();
}

void
vl::SettingsSerializer::readData( std::string const &xml_data )
{
	delete _xml_data;
	_xml_data = new vl::FileString( xml_data );
	readData();
}

void
vl::SettingsSerializer::readData( char *xml_data )
{
	delete _xml_data;
	_xml_data = new vl::FileString( xml_data );
	readData();
}

void
vl::SettingsSerializer::readData( )
{
	rapidxml::xml_document<> xmlDoc;
	rapidxml::xml_node<> *xmlRoot;
	
	assert( _xml_data );
	xmlDoc.parse<0>( _xml_data->data );

	xmlRoot = xmlDoc.first_node("config");
	if( !xmlRoot )
	{ throw vl::invalid_xml( "vl::SettingsSerailizer::readData" ); }

	processConfig( xmlRoot );
}

void
vl::SettingsSerializer::processConfig( rapidxml::xml_node<>* xml_root )
{	
	rapidxml::xml_node<>* xml_elem;

	xml_elem = xml_root->first_node("root");
	if( xml_elem )
	{ processRoot( xml_elem ); }

	xml_elem = xml_root->first_node("scene");
	if( xml_elem )
	{ processScene( xml_elem ); }

	xml_elem = xml_root->first_node("plugins");
	if( xml_elem )
	{ processPlugins( xml_elem ); }

	xml_elem = xml_root->first_node("resources");
	if( xml_elem )
	{ processResources( xml_elem ); }

	xml_elem = xml_root->first_node("eqc");
	if( xml_elem )
	{ processEqc( xml_elem ); }
}

void
vl::SettingsSerializer::processRoot( rapidxml::xml_node<>* xml_node )
{
	std::string name, path;
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute( "name" );
	if( attrib )
	{ name = attrib->value(); }

	rapidxml::xml_node<> *pElement= xml_node->first_node("path");
	if( pElement )
	{ path = pElement->value(); }

	// Both name and path should be present
	if( name.empty() || path.empty() )
	{
		_settings->clear();
		throw vl::invalid_xml( "vl::SettingsSerializer::processRoot" );
	}

	// Name should be unique
	if( _settings->findRoot( name ) )
	{
		_settings->clear();
		throw vl::invalid_xml( "vl::SettingsSerializer::processRoot" );
	}
	
	Settings::Root root( name, path );
	_settings->addRoot( root );

	// Process the rest of root nodes
	pElement = xml_node->next_sibling("root");
	if( pElement )
	{ processRoot( pElement ); }
}

void
vl::SettingsSerializer::processPlugins( rapidxml::xml_node<>* xml_node )
{
	vl::Settings::Root *root = getRootAttrib( xml_node );
	std::string file;
	
	rapidxml::xml_node<> *pElement = xml_node->first_node("file");
	if( pElement )
	{
		file = pElement->value();
	}

	// File is a must have element
	if( file.empty() )
	{
		_settings->clear();
		throw vl::invalid_xml( "vl::SettingsSerailizer::processPlugins" );
	}
	
	Settings::Plugins plugins( file, root );
	_settings->addPlugins( plugins );
}

void
vl::SettingsSerializer::processResources( rapidxml::xml_node<>* xml_node )
{
	vl::Settings::Root *root = getRootAttrib( xml_node );
	std::string file;
	
	rapidxml::xml_node<> *pElement = xml_node->first_node("file");
	if( pElement )
	{ file = pElement->value(); }

	if( file.empty() )
	{
		_settings->clear();
		throw vl::invalid_xml( "vl::SettingsSerailizer::processResources" );
	}
	
	Settings::Resources resource( file, root );
	_settings->addResources( resource );

	// Process the rest of resource nodes
	pElement = xml_node->next_sibling("resources");
	if( pElement )
	{ processResources( pElement ); }
}

void 
vl::SettingsSerializer::processEqc( rapidxml::xml_node<>* xml_node )
{
	std::string file;
	Settings::Root *root = getRootAttrib( xml_node );
	rapidxml::xml_node<> *pElement = xml_node->first_node( "file" );
	if( pElement )
	{ file = pElement->value(); }

	if( file.empty() )
	{ throw vl::invalid_xml( "vl::SettingsSerializer::processEqc" ); }
	
	Settings::Eqc eqc( file, root );
	_settings->setEqConfig( eqc );
}

void
vl::SettingsSerializer::processScene( rapidxml::xml_node<>* xml_node )
{
	std::string name, file, attach, type;
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute( "name" );
	if( attrib )
	{ name = attrib->value(); }

	rapidxml::xml_node<> *pElement = xml_node->first_node( "file" );
	if( pElement )
	{ file = pElement->value(); }

	// File element is a must
	if( file.empty() )
	{
		_settings->clear();
		throw vl::invalid_xml( "vl::SettingsSerailizer::processScene" );
	}

	pElement = xml_node->first_node( "attach" );
	if( pElement )
	{ attach = pElement->value(); }
	
	pElement = xml_node->first_node( "type" );
	if( pElement )
	{ type = pElement->value(); }
	
	Settings::Scene scene( file, name, attach, type );
	_settings->addScene( scene );

	// Process the rest of scene nodes
	pElement = xml_node->next_sibling("scene");
	if( pElement )
	{ processScene( pElement ); }
}

std::string
vl::SettingsSerializer::getAttrib(rapidxml::xml_node<>* xml_node,
		        const std::string &attrib, const std::string &default_value )
{
    if( xml_node->first_attribute(attrib.c_str()) )
    { return xml_node->first_attribute(attrib.c_str())->value(); }
    else
    { return default_value; }
}

