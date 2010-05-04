#include "settings.hpp"

#include "base/filesystem.hpp"
#include "base/exceptions.hpp"

#include <iostream>
#include <fstream>

vl::Settings::Settings( void ) { }

vl::Settings::~Settings( void ) { }

// --- SettingsSerializer ---
vl::SettingsSerializer::SettingsSerializer( Settings *settings )
	: _settings(settings), xml_data(0)
{
}

vl::SettingsSerializer::~SettingsSerializer( void )
{
	delete [] xml_data;
}

void
vl::SettingsSerializer::readFile( std::string const &file_path )
{
	if( !fs::exists( file_path ) )
	{ throw vl::missing_file( "vl::SettingsSerializer::readFile" ); }
	_settings->_file_path = file_path;

	std::ifstream stream( file_path.c_str(), std::ios::binary );

	// Read the file to memory
	stream.seekg( 0, std::ios::end );
	int length = stream.tellg();
	stream.seekg( 0, std::ios::beg );
	delete [] xml_data;
	xml_data = new char[length];
	stream.read( xml_data, length-1 );
	stream.close();
	xml_data[length-1] = '\0';
//	std::cout << "data = " << xml_data << std::endl;

	rapidxml::xml_document<> xmlDoc;
	rapidxml::xml_node<> *xmlRoot;
	xmlDoc.parse<0>( xml_data );

	xmlRoot = xmlDoc.first_node("config");
	if( !xmlRoot )
	{ throw vl::invalid_file( "vl::SettingsSerailizer::readFile" ); }

	processConfig( xmlRoot );
}

void
vl::SettingsSerializer::processConfig( rapidxml::xml_node<>* xml_root )
{
	rapidxml::xml_node<>* xml_elem;

	xml_elem = xml_root->first_node("path");
	if( xml_elem )
	{ processPath( xml_elem ); }

	xml_elem = xml_root->first_node("plugins");
	{ processPlugins( xml_elem ); }

	xml_elem = xml_root->first_node("resources");
	{ processResources( xml_elem ); }

	xml_elem = xml_root->first_node("eqc");
	{ processEqc( xml_elem ); }

	xml_elem = xml_root->first_node("scene");
	{ processScene( xml_elem ); }
}

void
vl::SettingsSerializer::processPath( rapidxml::xml_node<>* xml_node )
{
	_settings->_root_path = xml_node->value();
}

void
vl::SettingsSerializer::processPlugins( rapidxml::xml_node<>* xml_node )
{
	_settings->_plugin_file = xml_node->value();
}

void
vl::SettingsSerializer::processResources( rapidxml::xml_node<>* xml_node )
{
	_settings->_resource_file = xml_node->value();
}

void 
vl::SettingsSerializer::processEqc( rapidxml::xml_node<>* xml_node )
{
	_settings->_eq_config = xml_node->value();
}

void
vl::SettingsSerializer::processScene( rapidxml::xml_node<>* xml_node )
{
	_settings->_scene_file = xml_node->value();
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

// --- SettingsDeserializer ---
vl::SettingsDeserializer::SettingsDeserializer( Settings *settings )
{
}

vl::SettingsDeserializer::~SettingsDeserializer( void ) {}

void
vl::SettingsDeserializer::writeFile( std::string const &file_path ) {}
