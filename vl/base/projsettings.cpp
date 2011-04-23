/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *	Removed printing to standard out from all methods
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *	Fixed const correctness and changed unsigned int to size_t
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *	Added non-const retrieval methods
 *	Replaced spaces with tabs
 *	Added destructors
 *	Removed commented out printing
 */

/// Declaration
#include "projsettings.hpp"

#include "filesystem.hpp"
#include "exceptions.hpp"



/// Needed for writing the xml struct to string
#include "rapidxml_print.hpp"

#include <iostream>

////////// CASE SCENE /////////////////////////////////////////////////////////
vl::ProjSettings::Scene &
vl::ProjSettings::Case::addScene( std::string const &name )
{
	_scenes.push_back( vl::ProjSettings::Scene( name ) );
	_changed = true;
	return _scenes.back();
}

void
vl::ProjSettings::Case::addScene( const vl::ProjSettings::Scene& scene )
{
	_scenes.push_back( ProjSettings::Scene( scene ) );
	_changed = true;
}

void
vl::ProjSettings::Case::rmScene( std::string const &name )
{
// 	vl::ProjSettings::Scene const &scene = getScene(name);
	for( size_t i = 0; i < _scenes.size(); i++ )
	{
		if( _scenes.at(i).getName() == name )
		{
			_scenes.erase( _scenes.begin() + i );
		}
	}

	_changed = true;
}

vl::ProjSettings::Scene const &
vl::ProjSettings::Case::getScene( std::string const &sceneName ) const
{
	for( size_t i = 0; i < _scenes.size(); i++ )
	{
		if( _scenes.at(i).getName() == sceneName )
		{
			return _scenes.at(i);
		}
	}

	return vl::ProjSettings::Scene();
}

vl::ProjSettings::Scene const &
vl::ProjSettings::Case::getScene( size_t i ) const
{
	return _scenes.at(i);
}

////////// CASE SCRIPTS ///////////////////////////////////////////////////////////////

vl::ProjSettings::Script &
vl::ProjSettings::Case::addScript( std::string const &name )
{
	_scripts.push_back( ProjSettings::Script( name ) );
	_changed = true;
	return _scripts.back();
}

void
vl::ProjSettings::Case::rmScript( std::string const &name )
{
// 	ProjSettings::Script const &script = getScript(name);
	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getFile() == name )
		{
			_scripts.erase( _scripts.begin() + i );
		}
	}

	_changed = true;
}

vl::ProjSettings::Script const &
vl::ProjSettings::Case::getScript( std::string const &name ) const
{
	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getFile() == name )
		{
			return _scripts.at(i);
		}
	}

	return vl::ProjSettings::Script();
}

vl::ProjSettings::Script &
vl::ProjSettings::Case::getScript( std::string const &name )
{
	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getFile() == name )
		{
			return _scripts.at(i);
		}
	}

	// TODO should throw
}


vl::ProjSettings::Script const &
vl::ProjSettings::Case::getScript( size_t i ) const
{
	return _scripts.at(i);
}



bool
vl::ProjSettings::Case::getChanged( void ) const
{
	if( _changed )
	{ return true; }

	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		if( _scripts.at(i).getChanged() == true )
		{ return true; }
	}
	for( unsigned int i = 0; i < _scenes.size(); i++ )
	{
		if( _scenes.at(i).getChanged() == true )
		{ return true; }
	}

	return false;
}

void
vl::ProjSettings::Case::clearChanged( void )
{
	_changed = false;

	for( unsigned int i = 0; i < _scripts.size(); i++ )
	{
		_scripts.at(i).clearChanged();
	}
	for( unsigned int i = 0; i < _scenes.size(); i++ )
	{
		_scenes.at(i).clearChanged();
	}
}

vl::ProjSettings::ProjSettings( std::string const &file ) :
        _file( file ), _projCase( "" ), _changed(false)
{}

vl::ProjSettings::~ProjSettings( void )
{}

void
vl::ProjSettings::clear( void )
{
	_file.clear();
	_cases.clear();
	_changed = false;
}


///// CHANGED /////////////////////////////////////////////////////
bool
vl::ProjSettings::getChanged( void ) const
{
	if( _changed )
	{
		return true;
	}
	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getChanged() == true )
		{
			return true;
		}
	}

	return false;
}

void
vl::ProjSettings::clearChanged( void )
{
	_changed = false;
	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		_cases.at(i).clearChanged();
	}
}


///// CASES /////////////////////////////////////////////////
vl::ProjSettings::Case &
vl::ProjSettings::addCase( std::string const &caseName  )
{
	_cases.push_back( ProjSettings::Case( caseName ) );
	_changed = true;
	return _cases.back();
}

void
vl::ProjSettings::rmCase( std::string const &caseName )
{
// 	vl::ProjSettings::Case const &rmCase = getCase( caseName );
	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getName() == caseName )
		{
			_cases.erase( _cases.begin() + i );
		}
	}

	_changed = true;
}

vl::ProjSettings::Case const &
vl::ProjSettings::getCase( std::string const &caseName ) const
{
	if( caseName == _projCase.getName() )
	{ return _projCase; }

	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getName() == caseName )
		{ return _cases.at(i); }
	}

	return vl::ProjSettings::Case();
}

vl::ProjSettings::Case &
vl::ProjSettings::getCase( std::string const &caseName )
{
	if( caseName == _projCase.getName() )
	{ return _projCase; }

	for( unsigned int i = 0; i < _cases.size(); i++ )
	{
		if( _cases.at(i).getName() == caseName )
		{ return _cases.at(i); }
	}

	// TODO should throw
}


vl::ProjSettings::Case const &
vl::ProjSettings::getCase( size_t i ) const
{
	return _cases.at(i);
}

bool
vl::ProjSettings::empty ( void ) const
{
	return( _projCase.empty() && _cases.empty() );
}











///////////////////////////////////////////////////////////////////////
////////////////////// --- ProjSettingsSerializer --- //////////////////
///////////////////////////////////////////////////////////////////////

vl::ProjSettingsSerializer::ProjSettingsSerializer( vl::ProjSettingsRefPtr projSettingsPtr )
    : _projSettings(projSettingsPtr), _xml_data(0)
{}


vl::ProjSettingsSerializer::~ProjSettingsSerializer( void )
{
	delete [] _xml_data;
}

bool
vl::ProjSettingsSerializer::readString( std::string const &str )
{
	delete[] _xml_data;
	size_t length = str.length() + 1;
	_xml_data = new char[length];
	memcpy(_xml_data, str.c_str(), length);

	return readXML();
}

bool
vl::ProjSettingsSerializer::readXML()
{
	rapidxml::xml_document<> xmlDoc;
	xmlDoc.parse<0>( _xml_data );

	rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("project_config");
	if( !xmlRoot )
	{
		std::cerr << "Errenous xml. project_config node missing. Wrong file?"
			<< std::endl;
		return false;
	}
	readConfig( xmlRoot );
	return true;
}


void
vl::ProjSettingsSerializer::readConfig( rapidxml::xml_node<>* xml_root )
{
	rapidxml::xml_attribute<>* attrib = xml_root->first_attribute("name");
	if( !attrib )
	{
		std::cerr << "Missing project name. Default is used" << std::endl;
		_projSettings->setName("Default");
	}
	else
	{
		std::string nameFromFile = attrib->value();
		_projSettings->setName(nameFromFile);
	}

	ProjSettings::Case &projectCase = _projSettings->getCase();

	rapidxml::xml_node<>* xml_elem;

	xml_elem = xml_root->first_node("scenes");
	if( xml_elem )
	{
		readScenes( xml_elem, projectCase );
	}

	xml_elem = xml_root->first_node("scripts");
	if( xml_elem )
	{
		readScripts( xml_elem, projectCase );
	}

	xml_elem = xml_root->first_node("cases");
	if( xml_elem )
	{
		readCases( xml_elem );
	}
}

void
vl::ProjSettingsSerializer::readScenes( rapidxml::xml_node<>* xml_node, vl::ProjSettings::Case &c )
{
	rapidxml::xml_node<> *scene_element = xml_node->first_node("scene");

	while( scene_element )
	{
		readScene( scene_element, c );
		scene_element = scene_element->next_sibling("scene");
	}
}

void
vl::ProjSettingsSerializer::readScene( rapidxml::xml_node<>* xml_node, vl::ProjSettings::Case &c )
{
	std::string name;
	// Name
	rapidxml::xml_attribute<>* name_attrib = xml_node->first_attribute("name");
	if( !name_attrib )
	{
		std::cerr << "Name of scene in case " << c.getName()
			<< " missing" << std::endl;
		name = "Default";
	}
	else
	{
		name = name_attrib->value();
	}

	ProjSettings::Scene &s = c.addScene(name);

	// Use
	rapidxml::xml_attribute<>* use_attrib = xml_node->first_attribute("use");
	if( !use_attrib )
	{
		s.setUse( true );
		std::cerr << "Defaultin use of scene " << s.getName()
			<< " to false." << std::endl;
	}
	else
	{
		std::string useStr = use_attrib->value();
		if( useStr == "true" )
		{ s.setUse( true ); }
		else if( useStr == "false" )
		{ s.setUse( false ); }
		else
		{
			s.setUse( false );
			std::cerr << "Errenous use attribute of scene " << s.getName()
				<< ". Defaulting to false." << std::endl;
		}
	}

	// File
	rapidxml::xml_node<>* file_node = xml_node->first_node("file");
	if( !file_node )
	{
		std::cerr << "Scene " << s.getName() << " is missing path!" << std::endl;
	}
	else
	{
		s.setFile( file_node->value() );
	}

	// Attach to
	rapidxml::xml_node<>* attachto_node = xml_node->first_node("attachto");
	if( attachto_node )
	{
		rapidxml::xml_attribute<>* scene_attrib = attachto_node->first_attribute("scene");
		rapidxml::xml_attribute<>* point_attrib = attachto_node->first_attribute("point");
		if( scene_attrib )
		{
			s.setAttachtoScene( scene_attrib->value() );
		}
		if( point_attrib )
		{
			s.setAttachtoPoint( point_attrib->value() );
		}
	}
}

void
vl::ProjSettingsSerializer::readScripts( rapidxml::xml_node<>* xml_node, ProjSettings::Case &c )
{
	rapidxml::xml_node<> *script_element = xml_node->first_node("file");

	if( !script_element )
	{ return; }

	while( script_element )
	{
		ProjSettings::Script &a = c.addScript( script_element->value() );

		// Use
		rapidxml::xml_attribute<>* use_attrib = script_element->first_attribute("use");
		if( !use_attrib )
		{
			std::cerr << "Defaultin use of script " << a.getFile()
				<< " to false." << std::endl;
		}
		else
		{
			std::string useStr = use_attrib->value();
			if( useStr == "true" )
			{
				a.scriptOnOff( true );
			}
			else if( useStr == "false" )
			{
				a.scriptOnOff( false );
			}
			else
			{
				a.scriptOnOff( false );
				std::cerr << "Errenous use attribute of script "
					<< a.getFile() << ". Defaulting to false." << std::endl;
			}
		}

		script_element = script_element->next_sibling("file");
	}
}

void
vl::ProjSettingsSerializer::readCases( rapidxml::xml_node<>* xml_node )
{
	rapidxml::xml_node<> *case_element = xml_node->first_node("case");
	if( !case_element )
	{ return; }
	while( case_element )
	{
		readCase( case_element );
		case_element = case_element->next_sibling("case");
	}
}

void
vl::ProjSettingsSerializer::readCase( rapidxml::xml_node<>* xml_node )
{
	std::string name;
	// Name
	rapidxml::xml_attribute<>* name_attrib = xml_node->first_attribute("name");
	if( !name_attrib )
	{ name = "Default"; }
	else
	{ name = name_attrib->value(); }

	ProjSettings::Case &newCase = _projSettings->addCase( name );

	// Scenes
	rapidxml::xml_node<> *scenes_element = xml_node->first_node("scenes");
	if( scenes_element )
	{
		readScenes( scenes_element, newCase );
	}

	// Scripts
	rapidxml::xml_node<> *script_element = xml_node->first_node("scripts");
	if( script_element )
	{
		readScripts( script_element, newCase );
	}
}



////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
/////// WRITE //////////////////////////////////////

void
vl::ProjSettingsSerializer::writeString( std::string &str )
{
	rapidxml::xml_node<> *xmlRoot = _doc.allocate_node(rapidxml::node_element, "project_config" );
	char *c_name = _doc.allocate_string( _projSettings->getCase().getName().c_str() );
	rapidxml::xml_attribute<> *attrib = _doc.allocate_attribute( "name", c_name );
	xmlRoot->append_attribute( attrib );

	writeConfig(xmlRoot);

	_doc.append_node( xmlRoot );

	rapidxml::print(std::back_inserter( str ), _doc, 0);
}

void
vl::ProjSettingsSerializer::writeConfig( rapidxml::xml_node<> *xml_node )
{
	writeScenes( xml_node, _projSettings->getCase() );
	writeScripts( xml_node, _projSettings->getCase() );
	writeCases( xml_node );
}

const char *
vl::ProjSettingsSerializer::bool2char( bool b ) const
{
	if( b )
	{ return "true"; }
	return "false";
}

void
vl::ProjSettingsSerializer::writeScenes( rapidxml::xml_node<> *xml_node,
										 vl::ProjSettings::Case const &cas )
{
	rapidxml::xml_node<> *scenes_node = _doc.allocate_node(rapidxml::node_element, "scenes" );
	for( size_t i = 0; i < cas.getNscenes(); i++ )
	{
		ProjSettings::Scene const &sc = cas.getScene(i);
		char *c_name = _doc.allocate_string( sc.getName().c_str() );
		char *c_use = _doc.allocate_string( bool2char(sc.getUse()) );
		char *c_file = _doc.allocate_string( sc.getFile().c_str() );
		char *c_attachto_scene = _doc.allocate_string( sc.getAttachtoScene().c_str() );
		char *c_attachto_point = _doc.allocate_string( sc.getAttachtoPoint().c_str() );

		rapidxml::xml_node<> *scene
				=  _doc.allocate_node(rapidxml::node_element, "scene" );
		rapidxml::xml_attribute<> *attrib
				= _doc.allocate_attribute( "name", c_name );
		scene->append_attribute(attrib);
		attrib = _doc.allocate_attribute( "use", c_use );
		scene->append_attribute(attrib);

		rapidxml::xml_node<> *node
				=  _doc.allocate_node(rapidxml::node_element, "file", c_file );
		scene->append_node(node);

		node =  _doc.allocate_node(rapidxml::node_element, "attachto" );
		attrib = _doc.allocate_attribute( "scene", c_attachto_scene );
		node->append_attribute(attrib);
		attrib = _doc.allocate_attribute( "point", c_attachto_point );
		node->append_attribute(attrib);
		scene->append_node(node);

		scenes_node->append_node(scene);
	}
	xml_node->append_node(scenes_node);
}

void
vl::ProjSettingsSerializer::writeScripts( rapidxml::xml_node<> *xml_node,
										  vl::ProjSettings::Case const &cas )
{
	rapidxml::xml_node<> *scripts_node = _doc.allocate_node(rapidxml::node_element, "scripts" );
	for( size_t i = 0; i < cas.getNscripts(); i++ )
	{
		ProjSettings::Script const &script = cas.getScript(i);
		char *fileC = _doc.allocate_string( script.getFile().c_str() );
		char *use = _doc.allocate_string( bool2char(script.getUse()) );

		rapidxml::xml_node<> *file
					=  _doc.allocate_node(rapidxml::node_element, "file", fileC );
		rapidxml::xml_attribute<> *attrib
				= _doc.allocate_attribute( "use", use );
		file->append_attribute(attrib);

		scripts_node->append_node(file);
	}

	xml_node->append_node(scripts_node);
}

void
vl::ProjSettingsSerializer::writeCases( rapidxml::xml_node<> *xml_node )
{
	rapidxml::xml_node<> *cases_node = _doc.allocate_node(rapidxml::node_element, "cases" );
	for( size_t i = 0; i < _projSettings->getNcases(); i++ )
	{
		ProjSettings::Case const &cas = _projSettings->getCase(i);
		char *nameC = _doc.allocate_string( cas.getName().c_str() );

		rapidxml::xml_node<> *case_node
				=  _doc.allocate_node(rapidxml::node_element, "case" );
		rapidxml::xml_attribute<> *attrib
				= _doc.allocate_attribute( "name", nameC );
		case_node->append_attribute(attrib);

		writeScenes( case_node, cas );
		writeScripts( case_node, cas );

		cases_node->append_node(case_node);
	}

	xml_node->append_node(cases_node);
}
