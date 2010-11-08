/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 * 2010-11
 */

/// Declaration
#include "projsettings.hpp"

#include "filestringer.hpp"
#include "exceptions.hpp"



/// Needed for writing the xml struct to string
#include "rapidxml_print.hpp"

#include <iostream>

////////// CASE SCENE ///////////////////////////////////////////////////////////////

ProjSettings::Scene* ProjSettings::Case::addScene( std::string const &name )
{
    std::cout << "ProjSettings Case addScene: name " << name << std::endl;

    ProjSettings::Scene* scene = new ProjSettings::Scene( name );
    _scenes.push_back( scene );
    _changed = true;
    return scene;
}

void ProjSettings::Case::rmScene( std::string const &name )
{
    std::cout << "ProjSettings Case rmScene: name " << name << std::endl;

    ProjSettings::Scene* scene = getScenePtr(name);
    for( unsigned int i = 0; i < _scenes.size(); i++ )
    {
        if( _scenes.at(i)->getName() == name )
        {
            _scenes.erase( _scenes.begin() + i );
        }
    }
    delete scene;
    _changed = true;
}

ProjSettings::Scene* ProjSettings::Case::getScenePtr( std::string const &sceneName )
{
    std::cout << "ProjSettings Case getScenePtr: name " << sceneName << std::endl;

    for( unsigned int i = 0; i < _scenes.size(); i++ )
    {
        if( _scenes.at(i)->getName() == sceneName )
        {
            return _scenes.at(i);
        }
    }
    return 0;
}

ProjSettings::Scene* ProjSettings::Case::getScenePtr( unsigned int i ) const
{
    if( i < _scenes.size() )
    {
        return _scenes.at(i);
    }
    else
    {
        std::cerr << "Trying to get scene pointer from case " << _name << "with invalid index!" << std::endl;
        return 0;
    }
}


////////// CASE ACTION MAP ///////////////////////////////////////////////////////////////

ProjSettings::ActionMap* ProjSettings::Case::addMap( std::string const &name )
{
    std::cout << "ProjSettings Case addMap: name " << name << std::endl;

    ProjSettings::ActionMap* map = new ProjSettings::ActionMap( name );
    _actionMaps.push_back( map );
    _changed = true;
    return map;
}

void ProjSettings::Case::rmMap( std::string const &name )
{
    std::cout << "ProjSettings Case rmMap: name " << name << std::endl;

    ProjSettings::ActionMap* map = getMapPtr(name);
    for( unsigned int i = 0; i < _actionMaps.size(); i++ )
    {
        if( _actionMaps.at(i)->getFile() == name )
        {
            _actionMaps.erase( _actionMaps.begin() + i );
        }
    }
    delete map;
    _changed = true;
}

ProjSettings::ActionMap* ProjSettings::Case::getMapPtr( std::string const &mapName )
{
    std::cout << "ProjSettings Case getMapPtr: mapName " << mapName << std::endl;

    for( unsigned int i = 0; i < _actionMaps.size(); i++ )
    {
        if( _actionMaps.at(i)->getFile() == mapName )
        {
            return _actionMaps.at(i);
        }
    }
    return 0;
}

ProjSettings::ActionMap* ProjSettings::Case::getMapPtr( unsigned int i ) const
{
    std::cout << "ProjSettings Case getMapPtr:" << std::endl;

    if( i < _actionMaps.size() )
    {
        return _actionMaps.at(i);
    }
    else
    {
        std::cerr << "Trying to get action map pointer from case " << _name << "with invalid index!" << std::endl;
        return 0;
    }
}


////////// CASE SCRIPTS ///////////////////////////////////////////////////////////////

ProjSettings::Script* ProjSettings::Case::addScript( std::string const &name )
{
    std::cout << "ProjSettings Case addScript: name " << name << std::endl;

    ProjSettings::Script* script = new ProjSettings::Script( name );
    _scripts.push_back( script );
    _changed = true;
    return script;
}

void ProjSettings::Case::rmScript( std::string const &name )
{
    std::cout << "ProjSettings Case rmScript: name " << name << std::endl;

    ProjSettings::Script* script = getScriptPtr(name);
    for( unsigned int i = 0; i < _scripts.size(); i++ )
    {
        if( _scripts.at(i)->getFile() == name )
        {
            _scripts.erase( _scripts.begin() + i );
        }
    }
    delete script;
    _changed = true;
}

ProjSettings::Script* ProjSettings::Case::getScriptPtr( std::string const &name )
{
    std::cout << "ProjSettings Case getScriptPtr: name " << name << std::endl;

    for( unsigned int i = 0; i < _scripts.size(); i++ )
    {
        if( _scripts.at(i)->getFile() == name )
        {
            return _scripts.at(i);
        }
    }
    return 0;
}

ProjSettings::Script* ProjSettings::Case::getScriptPtr( unsigned int i ) const
{
    std::cout << "ProjSettings Case getScriptPtr " << std::endl;

    if( i < _scripts.size() )
    {
        return _scripts.at(i);
    }
    else
    {
        std::cerr << "Trying to get script pointer from case " << _name << "with invalid index!" << std::endl;
        return 0;
    }
}



bool ProjSettings::Case::getChanged()
{
    std::cout << "ProjSettings Case getChanged: case name " << _name << std::endl;

    if( _changed )
    {
        return true;
    }
    for( unsigned int i = 0; i < _actionMaps.size(); i++ )
    {
        if( _actionMaps.at(i)->getChanged() == true )
        {
            return true;
        }
    }
    for( unsigned int i = 0; i < _scripts.size(); i++ )
    {
        if( _scripts.at(i)->getChanged() == true )
        {
            return true;
        }
    }
    for( unsigned int i = 0; i < _scenes.size(); i++ )
    {
        if( _scenes.at(i)->getChanged() == true )
        {
            return true;
        }
    }
    return false;
}

void ProjSettings::Case::clearChanged()
{
    std::cout << "ProjSettings Case clearChanged: case name " << _name << std::endl;

    _changed = false;
    for( unsigned int i = 0; i < _actionMaps.size(); i++ )
    {
        _actionMaps.at(i)->clearChanged();
    }
    for( unsigned int i = 0; i < _scripts.size(); i++ )
    {
        _scripts.at(i)->clearChanged();
    }
    for( unsigned int i = 0; i < _scenes.size(); i++ )
    {
        _scenes.at(i)->clearChanged();
    }
}

ProjSettings::ProjSettings( std::string const &file ) :
        _file( file ), _changed(false)
{
    std::cout << "ProjSettings constructor: file name " << file << std::endl;
    _projCase = new ProjSettings::Case("");
}

ProjSettings::~ProjSettings( void )
{
    std::cout << "ProjSettings destructor: project name " << _projCase->getName() << std::endl;
}

void
ProjSettings::clear( void )
{
    std::cout << "ProjSettings clear: project name " << _projCase->getName() << std::endl;

    _file.clear();
    _cases.clear();
    _changed = false;
}


///// CHANGED /////////////////////////////////////////////////////
bool ProjSettings::getChanged( void )
{
    std::cout << "ProjSettings getChanged: project name " << _projCase->getName() << std::endl;

    if( _changed )
    {
        return true;
    }
    for( unsigned int i = 0; i < _cases.size(); i++ )
    {
        if( _cases.at(i)->getChanged() == true )
        {
            return true;
        }
    }
    return false;
}

void ProjSettings::clearChanged()
{
    std::cout << "ProjSettings clearChanged: project name " << _projCase->getName() << std::endl;

    _changed = false;
    for( unsigned int i = 0; i < _cases.size(); i++ )
    {
        _cases.at(i)->clearChanged();
    }
}


///// CASES /////////////////////////////////////////////////
ProjSettings::Case* ProjSettings::addCase( std::string const &caseName  )
{
    std::cout << "ProjSettings addCase: " << " : case name " << caseName <<  std::endl;

    ProjSettings::Case* newCase = new ProjSettings::Case( caseName );
    _cases.push_back( newCase );
    _changed = true;
    return newCase;
}

void ProjSettings::rmCase( std::string const &caseName )
{
    std::cout << "ProjSettings rmCase: project name " << _projCase->getName() << " : case name " << caseName <<  std::endl;

    ProjSettings::Case* rmCase = getCasePtr( caseName );
    for( unsigned int i = 0; i < _cases.size(); i++ )
    {
        if( _cases.at(i)->getName() == caseName )
        {
            _cases.erase( _cases.begin() + i );
        }
    }
    delete rmCase;
    _changed = true;
}

ProjSettings::Case* ProjSettings::getCasePtr( std::string const &caseName )
{
    std::cout << "ProjSettings getCasePtr: project name " << _projCase->getName() << " : case name " << caseName <<  std::endl;

    if( caseName == _projCase->getName() )
    {
        return _projCase;
    }

    for( unsigned int i = 0; i < _cases.size(); i++ )
    {
        if( _cases.at(i)->getName() == caseName )
        {
            return _cases.at(i);
        }
    }
    return 0;
}

ProjSettings::Case* ProjSettings::getCasePtr( unsigned int i ) const
{
    if( i < _cases.size() )
    {
        return _cases.at(i);
    }
    else
    {
        std::cerr << "Trying to get case pointer from project " << _projCase->getName() << "with invalid index!" << std::endl;
        return 0;
    }
}
















///////////////////////////////////////////////////////////////////////
////////////////////// --- ProjSettingsSerializer --- //////////////////
///////////////////////////////////////////////////////////////////////

ProjSettingsSerializer::ProjSettingsSerializer( ProjSettingsRefPtr projSettingsPtr )
    : _projSettings(projSettingsPtr), _xml_data(0)
{
    std::cout << "ProjSettingsSerializer constructor" << std::endl;
}


ProjSettingsSerializer::~ProjSettingsSerializer( void )
{
    std::cout << "ProjSettingsSerializer destructor" << std::endl;

    delete [] _xml_data;
}

bool
ProjSettingsSerializer::readString( std::string const &str )
{
    std::cout << "ProjSettingsSerializer readString" << std::endl;

    delete[] _xml_data;
    size_t length = str.length() + 1;
    _xml_data = new char[length];
    memcpy(_xml_data, str.c_str(), length);
    
    return readXML();
}

bool ProjSettingsSerializer::readXML()
{
    std::cout << "ProjSettingsSerializer readXML" << std::endl;

    rapidxml::xml_document<> xmlDoc;
    xmlDoc.parse<0>( _xml_data );

    rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("project_config");
    if( !xmlRoot )
    {
        std::cerr << "Errenous xml. project_config node missing. Wrong file?" << std::endl;
        return false;
    }
    readConfig( xmlRoot );
    return true;
}


void
ProjSettingsSerializer::readConfig( rapidxml::xml_node<>* xml_root )
{
    std::cout << "ProjSettingsSerializer readConfig" << std::endl;

    rapidxml::xml_attribute<>* attrib = xml_root->first_attribute("name");
    if( !attrib )
    {
        std::cerr << "Missing project name. Default is used" << std::endl;
        _projSettings->getCasePtr()->setName("Default");
    }
    else
    {
        std::string nameFromFile = attrib->value();
        std::cerr << "koe" << std::endl;

        _projSettings->getCasePtr()->setName(nameFromFile);
    }

    std::cerr << "koe" << std::endl;
    ProjSettings::Case* projectCase = _projSettings->getCasePtr();

    rapidxml::xml_node<>* xml_elem;

    xml_elem = xml_root->first_node("scenes");
    if( xml_elem )
    {
        readScenes( xml_elem, projectCase );
    }

    xml_elem = xml_root->first_node("action_maps");
    if( xml_elem )
    {
        readActionMaps( xml_elem, projectCase );
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
ProjSettingsSerializer::readScenes( rapidxml::xml_node<>* xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer readScenes: case " << c->getName() << std::endl;

    rapidxml::xml_node<> *scene_element = xml_node->first_node("scene");

    if( !scene_element )
    {
        std::cerr << "No scenes in the case " << c->getName() <<  ". Strange?" << std::endl;
        return;
    }

    while( scene_element )
    {
        readScene( scene_element, c );
        scene_element = scene_element->next_sibling("scene");
    }
}

void
ProjSettingsSerializer::readScene( rapidxml::xml_node<>* xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer readScene: case " << c->getName() << std::endl;

    std::string name;
    // Name
    rapidxml::xml_attribute<>* name_attrib = xml_node->first_attribute("name");
    if( !name_attrib )
    {
        std::cerr << "Name of scene in case " << c->getName() << " missing" << std::endl;
        name = "Default";
    }
    else
    {
        name = name_attrib->value();
    }

    ProjSettings::Scene* s = c->addScene(name);

    // Use
    rapidxml::xml_attribute<>* use_attrib = xml_node->first_attribute("use");
    if( !use_attrib )
    {
        s->sceneOnOff( true );
        std::cerr << "Defaultin use of scene " << s->getName() << " to false." << std::endl;
    }
    else
    {
        std::string useStr = use_attrib->value();
        if( useStr == "true" )
        { s->sceneOnOff( true ); }
        else if( useStr == "false" )
        { s->sceneOnOff( false ); }
        else
        {
            s->sceneOnOff( false );
            std::cerr << "Errenous use attribute of scene " << s->getName() << ". Defaulting to false." << std::endl;
        }
    }

    // File
    rapidxml::xml_node<>* file_node = xml_node->first_node("file");
    if( !file_node )
    {
        std::cerr << "Scene " << s->getName() << " is missing path!" << std::endl;
    }
    else
    {
        s->setFile( file_node->value() );
    }

    // Attach to
    rapidxml::xml_node<>* attachto_node = xml_node->first_node("attachto");
    if( attachto_node )
    {
        rapidxml::xml_attribute<>* scene_attrib = attachto_node->first_attribute("scene");
        rapidxml::xml_attribute<>* point_attrib = attachto_node->first_attribute("point");
        if( scene_attrib )
        {
            s->setAttachtoScene( scene_attrib->value() );
        }
        if( point_attrib )
        {
            s->setAttachtoPoint( point_attrib->value() );
        }
    }
}

void
ProjSettingsSerializer::readActionMaps( rapidxml::xml_node<>* xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer readActionMaps: case " << c->getName() << std::endl;

    rapidxml::xml_node<> *map_element = xml_node->first_node("file");

    if( !map_element )
    { return; }

    while( map_element )
    {
        ProjSettings::ActionMap* a = c->addMap( map_element->value() );

        // Use
        rapidxml::xml_attribute<>* use_attrib = map_element->first_attribute("use");
        if( !use_attrib )
        {
            std::cerr << "Defaultin use of action map " << a->getFile() << " to false." << std::endl;
        }
        else
        {
            std::string useStr = use_attrib->value();
            if( useStr == "true" )
            {
                a->mapOnOff( true );
            }
            else if( useStr == "false" )
            {
                a->mapOnOff( false );
            }
            else
            {
                a->mapOnOff( false );
                std::cerr << "Errenous use attribute of action map " << a->getFile() << ". Defaulting to false." << std::endl;
            }
        }

        map_element = map_element->next_sibling("file");
    }
}

void
ProjSettingsSerializer::readScripts( rapidxml::xml_node<>* xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer readScripts: case " << c->getName() << std::endl;

    rapidxml::xml_node<> *script_element = xml_node->first_node("file");

    if( !script_element )
    { return; }

    while( script_element )
    {
        ProjSettings::Script* a = c->addScript( script_element->value() );

        // Use
        rapidxml::xml_attribute<>* use_attrib = script_element->first_attribute("use");
        if( !use_attrib )
        {
            std::cerr << "Defaultin use of script " << a->getFile() << " to false." << std::endl;
        }
        else
        {
            std::string useStr = use_attrib->value();
            if( useStr == "true" )
            {
                a->scriptOnOff( true );
            }
            else if( useStr == "false" )
            {
                a->scriptOnOff( false );
            }
            else
            {
                a->scriptOnOff( false );
                std::cerr << "Errenous use attribute of script " << a->getFile() << ". Defaulting to false." << std::endl;
            }
        }

        script_element = script_element->next_sibling("file");
    }
}

void
 ProjSettingsSerializer::readCases( rapidxml::xml_node<>* xml_node )
{
    std::cout << "ProjSettingsSerializer readCases" << std::endl;

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
ProjSettingsSerializer::readCase( rapidxml::xml_node<>* xml_node )
{
    std::cout << "ProjSettingsSerializer readCasee" << std::endl;

    std::string name;
    // Name
    rapidxml::xml_attribute<>* name_attrib = xml_node->first_attribute("name");
    if( !name_attrib )
    { name = "Default"; }
    else
    { name = name_attrib->value(); }

    ProjSettings::Case* newCase = _projSettings->addCase( name );

    // Scenes
    rapidxml::xml_node<> *scenes_element = xml_node->first_node("scenes");
    if( scenes_element )
    {
        readScenes( scenes_element, newCase );
    }

    // Action maps
    rapidxml::xml_node<> *map_element = xml_node->first_node("action_maps");
    if( map_element )
    {
        readActionMaps( map_element, newCase );
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
ProjSettingsSerializer::writeString( std::string &str )
{
    std::cout << "ProjSettingsSerializer writeString" << std::endl;

    rapidxml::xml_node<> *xmlRoot = _doc.allocate_node(rapidxml::node_element, "project_config" );
    char *c_name = _doc.allocate_string( _projSettings->getCasePtr()->getName().c_str() );
    rapidxml::xml_attribute<> *attrib = _doc.allocate_attribute( "name", c_name );
    xmlRoot->append_attribute( attrib );

    writeConfig(xmlRoot);

    _doc.append_node( xmlRoot );

    rapidxml::print(std::back_inserter( str ), _doc, 0);
}

void ProjSettingsSerializer::writeConfig( rapidxml::xml_node<> *xml_node )
{
    std::cout << "ProjSettingsSerializer writeConfig" << std::endl;

    writeScenes( xml_node, _projSettings->getCasePtr() );
    writeMaps( xml_node, _projSettings->getCasePtr() );
    writeScripts( xml_node, _projSettings->getCasePtr() );
    writeCases( xml_node );

}

const char* ProjSettingsSerializer::bool2char( bool b )
{
    if( b )
    {
        return "true";
    }
    return "false";
}

void ProjSettingsSerializer::writeScenes( rapidxml::xml_node<> *xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer writeScenes" << std::endl;

    rapidxml::xml_node<> *scenes_node = _doc.allocate_node(rapidxml::node_element, "scenes" );
    for( unsigned int i = 0; i < c->getNscenes(); i++ )
    {
        ProjSettings::Scene* sc = c->getScenePtr(i);
        char *c_name = _doc.allocate_string( sc->getName().c_str() );
        char *c_use = _doc.allocate_string( bool2char(sc->getUse()) );
        char *c_file = _doc.allocate_string( sc->getFile().c_str() );
        char *c_attachto_scene = _doc.allocate_string( sc->getAttachtoScene().c_str() );
        char *c_attachto_point = _doc.allocate_string( sc->getAttachtoPoint().c_str() );

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

void ProjSettingsSerializer::writeMaps( rapidxml::xml_node<> *xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer writeMaps" << std::endl;

    rapidxml::xml_node<> *maps_node = _doc.allocate_node(rapidxml::node_element, "action_maps" );
    for( unsigned int i = 0; i < c->getNmaps(); i++ )
    {
        ProjSettings::ActionMap* mp = c->getMapPtr(i);
        char *fileC = _doc.allocate_string( mp->getFile().c_str() );
        char *use = _doc.allocate_string( bool2char(mp->getUse()) );

        rapidxml::xml_node<> *file
                    =  _doc.allocate_node(rapidxml::node_element, "file", fileC );
        rapidxml::xml_attribute<> *attrib
                = _doc.allocate_attribute( "use", use );
        file->append_attribute(attrib);

        maps_node->append_node(file);
    }
    xml_node->append_node(maps_node);
}

void ProjSettingsSerializer::writeScripts( rapidxml::xml_node<> *xml_node, ProjSettings::Case* c )
{
    std::cout << "ProjSettingsSerializer writeScripts" << std::endl;

    rapidxml::xml_node<> *scripts_node = _doc.allocate_node(rapidxml::node_element, "scripts" );
    for( unsigned int i = 0; i < c->getNscripts(); i++ )
    {
        ProjSettings::Script* mp = c->getScriptPtr(i);
        char *fileC = _doc.allocate_string( mp->getFile().c_str() );
        char *use = _doc.allocate_string( bool2char(mp->getUse()) );

        rapidxml::xml_node<> *file
                    =  _doc.allocate_node(rapidxml::node_element, "file", fileC );
        rapidxml::xml_attribute<> *attrib
                = _doc.allocate_attribute( "use", use );
        file->append_attribute(attrib);

        scripts_node->append_node(file);
    }
    xml_node->append_node(scripts_node);
}

void ProjSettingsSerializer::writeCases( rapidxml::xml_node<> *xml_node )
{
    std::cout << "ProjSettingsSerializer writeCases" << std::endl;

    rapidxml::xml_node<> *cases_node = _doc.allocate_node(rapidxml::node_element, "cases" );
    for( unsigned int i = 0; i < _projSettings->getNcases(); i++ )
    {
        ProjSettings::Case* ca = _projSettings->getCasePtr(i);
        char *nameC = _doc.allocate_string( ca->getName().c_str() );

        rapidxml::xml_node<> *case_node
                =  _doc.allocate_node(rapidxml::node_element, "case" );
        rapidxml::xml_attribute<> *attrib
                = _doc.allocate_attribute( "name", nameC );
        case_node->append_attribute(attrib);

        writeScenes( case_node, ca );
        writeMaps( case_node, ca );
        writeScripts( case_node, ca );

        cases_node->append_node(case_node);
    }
    xml_node->append_node(cases_node);
}
