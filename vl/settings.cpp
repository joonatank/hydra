#include "settings.hpp"

#include "base/filesystem.hpp"
#include "base/exceptions.hpp"

#include <iostream>
#include <fstream>

vl::Settings::Settings( void ) { }

vl::Settings::~Settings( void ) { }

void
vl::Settings::setExePath( std::string const &path )
{
	_exe_path = path;

	// Update args
	_eq_args.clear();
	if( !_exe_path.empty() )
	{ _eq_args.add( _exe_path.string().c_str() ); }
	if( !_eq_config.empty() )
	{
		_eq_args.add( "--eq-config" );
		_eq_args.add( _eq_config.string().c_str() );
	}
}

void
vl::Settings::setEqConfigPath( std::string const &path )
{
	_eq_config = path;
	/*
	fs::path tmp( path );
	if( tmp.root_directory() == "/" )
	{ _eq_config = tmp; }
	else
	{ _eq_config = _root_path / tmp; }
	*/

	// Update args
	_eq_args.clear();
	if( !_exe_path.empty() )
	{ _eq_args.add( _exe_path.string().c_str() ); }
	if( !_eq_config.empty() )
	{
		_eq_args.add( "--eq-config" );
		_eq_args.add( _eq_config.string().c_str() );
	}
}

void
vl::Settings::setOgrePluginsPath( std::string const &path )
{
	_plugin_file = path;
	/*
	fs::path tmp( path );
	if( tmp.root_directory() == "/" )
	{ _plugin_file = tmp; }
	else
	{ _plugin_file = _root_path / tmp; }
	*/
}

/*
void 
vl::Settings::setOgreResourcePath( std::string const &path )
{
	fs::path tmp( path );
	if( tmp.root_directory() == "/" )
	{ _resource_file = tmp; }
	else
	{ _resource_file = _root_path / tmp; }
}
*/

void
vl::Settings::setScene( std::string const &scene )
{
	_scene = scene;
	/*
	fs::path tmp( path );
	if( tmp.root_directory() == "/" )
	{ _scene_file = tmp; }
	else
	{ _scene_file = _root_path / tmp; }
	*/
}

// --- SettingsSerializer ---
vl::SettingsSerializer::SettingsSerializer( Settings *settings )
	: _settings(settings), xml_data()
{
}

vl::SettingsSerializer::~SettingsSerializer( void )
{
}

void
vl::SettingsSerializer::readFile( std::string const &file_path )
{
	if( !fs::exists( file_path ) )
	{ throw vl::missing_file( "vl::SettingsSerializer::readFile" ); }
	_settings->_file_path = file_path;

	std::ifstream stream( file_path.c_str(), std::ios::binary );

	xml_data.readStream( stream );

	rapidxml::xml_document<> xmlDoc;

	rapidxml::xml_node<> *xmlRoot;
	xmlDoc.parse<0>( xml_data.data );

	xmlRoot = xmlDoc.first_node("config");
	if( !xmlRoot )
	{ throw vl::invalid_file( "vl::SettingsSerailizer::readFile" ); }

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
	{ processScene( xml_elem ); }
}

void
vl::SettingsSerializer::processRoot( rapidxml::xml_node<>* xml_node )
{
	vl::Settings::Root root;
	rapidxml::xml_attribute<> *attrib = xml_node->first_attribute( "path" );
	if( attrib )
	{ root.setPath( attrib->value() ); }

	rapidxml::xml_node<> *xml_elem = xml_node->first_node("plugins");
	if( xml_elem )
	{ processPlugins( xml_elem, root ); }

	xml_elem = xml_node->first_node("resources");
	if( xml_elem )
	{ processResources( xml_elem, root ); }

	xml_elem = xml_node->first_node("eqc");
	if( xml_elem )
	{ processEqc( xml_elem, root ); }

	xml_elem = xml_node->first_node("scene");
	if( xml_elem )
	{ processScene( xml_elem, root ); }

	_settings->addRoot( root );

	// Process the rest root nodes
	rapidxml::xml_node<> *pElement = xml_node->next_sibling("root");
	if( pElement )
	{ processRoot( pElement ); }
}

void
vl::SettingsSerializer::processPlugins( rapidxml::xml_node<>* xml_node,
		vl::Settings::Root const &root )
{
	_settings->setOgrePluginsPath( root.path.file_string() +"/" + xml_node->value() );
}

void
vl::SettingsSerializer::processResources( rapidxml::xml_node<>* xml_node,
		vl::Settings::Root &root )
{
	root.resources.push_back( xml_node->value() );
}

void 
vl::SettingsSerializer::processEqc( rapidxml::xml_node<>* xml_node,
		vl::Settings::Root const &root )
{
	_settings->setEqConfigPath( root.path.file_string() + "/" + xml_node->value() );
}

void
vl::SettingsSerializer::processScene( rapidxml::xml_node<>* xml_node,
		vl::Settings::Root const &root )
{
	_settings->setScene( xml_node->value() );
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
