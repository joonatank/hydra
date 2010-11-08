#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include "projsettings.h"
#include "exceptions.hpp"
#include "rapidxml_print.hpp"


#include <boost/filesystem.hpp>
#include <iostream>

#include <OGRE/OgreStringConverter.h>

namespace fs
{
    using namespace boost::filesystem;
}

// TODO test creation of settings
// TODO test copying of settings
BOOST_AUTO_TEST_CASE( constructors )
{

}

// Invalid xml file, without config element
BOOST_AUTO_TEST_CASE( invalid_xml )
{
        rapidxml::xml_document<> doc;

        // First root node
        rapidxml::xml_node<> *maps = doc.allocate_node(rapidxml::node_element, "action_maps" );
        doc.append_node( maps );
        rapidxml::xml_node<> *file = doc.allocate_node(rapidxml::node_element, "file", "/not/real/file" );
        maps->append_node(file);

        ProjSettingsRefPtr settings( new ProjSettings() );
        ProjSettingsSerializer ser( settings );

        // Create the test data
        std::string data;
        rapidxml::print(std::back_inserter(data), doc, 0);

        // Parse
        BOOST_CHECK( !ser.readString(data) );
}

BOOST_AUTO_TEST_CASE( test_name_defaulting )
{
        rapidxml::xml_document<> doc;

        // First root node
        rapidxml::xml_node<> *conf = doc.allocate_node(rapidxml::node_element, "project_config" );
        doc.append_node( conf );

        ProjSettingsRefPtr settings( new ProjSettings() );
        ProjSettingsSerializer ser( settings );

        // Create the test data
        std::string data;
        rapidxml::print(std::back_inserter(data), doc, 0);

        // Parse
        BOOST_CHECK( ser.readString(data) );

        BOOST_CHECK_EQUAL( settings->getName(), "Default" );
}


struct SettingsFixture
{
        SettingsFixture( void )
                : _settings( new ProjSettings() ), _ser(0), _doc(), _config(0)
        {
            _ser = new ProjSettingsSerializer( _settings );
            _config = _doc.allocate_node(rapidxml::node_element, "project_config" );
            rapidxml::xml_attribute<> *attrib
                    = _doc.allocate_attribute( "name", "project" );
            _config->append_attribute( attrib );
            _doc.append_node(_config);
        }

        ~SettingsFixture( void )
        {
                delete _ser;
        }

        bool readXML( void )
        {
                _data.clear();
                rapidxml::print(std::back_inserter(_data), _doc, 0);
                // Print to string so we can use SettingsSerializer for the data
                BOOST_TEST_MESSAGE( _data );
                if( _ser->readString( _data ) )
                {
                    return true;
                }
                return false;
        }

        bool checkEmpty( void )
        {
                return _settings->getActionMaps().empty()
                        && _settings->getCases().empty()
                        && _settings->getName().empty()
                        && _settings->getScenes().empty();

        }

        rapidxml::xml_node<> *createScene( std::string const &name = "name",
                                           std::string const &use = "true",
                                           std::string const &file = "file",
                                           std::string const &attachto_scene = "attachto_scene",
                                           std::string const &attachto_point = "attachto_point" )

        {
            // allocating to ensure string is copied and NOT temporary
            // this copy lasts untill _doc is destroyed
            char *c_name = _doc.allocate_string( name.c_str() );
            char *c_use = _doc.allocate_string( use.c_str() );
            char *c_file = _doc.allocate_string( file.c_str() );
            char *c_attachto_scene = _doc.allocate_string( attachto_scene.c_str() );
            char *c_attachto_point = _doc.allocate_string( attachto_point.c_str() );

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

            return scene;
        }

        rapidxml::xml_node<> *createScenes()
        {
            rapidxml::xml_node<> *scenes
                    =  _doc.allocate_node(rapidxml::node_element, "scenes" );
            scenes->append_node( createScene() );
            scenes->append_node( createScene("name2", "false", "file2", "attachto_scene2", "attachto_point2"));
            return scenes;
        }


        rapidxml::xml_node<> *createActionMaps( std::string const &a1 = "map1",  std::string const &u1 = "true",
                                             std::string const &a2 = "map2",  std::string const &u2 ="false" )
        {
            // allocating to ensure string is copied and NOT temporary
            // this copy lasts untill _doc is destroyed
            char *c_a1 = _doc.allocate_string( a1.c_str() );
            char *c_a2 = _doc.allocate_string( a2.c_str() );
            char *c_u1 = _doc.allocate_string( u1.c_str() );
            char *c_u2 = _doc.allocate_string( u2.c_str() );

            rapidxml::xml_node<> *action_maps
                    =  _doc.allocate_node(rapidxml::node_element, "action_maps" );

            rapidxml::xml_node<> *file
                        =  _doc.allocate_node(rapidxml::node_element, "file", c_a1 );
            rapidxml::xml_attribute<> *attrib
                    = _doc.allocate_attribute( "use", c_u1 );
            file->append_attribute(attrib);
            action_maps->append_node(file);

            file =  _doc.allocate_node(rapidxml::node_element, "file", c_a2 );
            attrib = _doc.allocate_attribute( "use", c_u2 );
            file->append_attribute(attrib);
            action_maps->append_node(file);

            return action_maps;
        }

        rapidxml::xml_node<> *createCase( std::string const &n = "case_name")
        {
            // allocating to ensure string is copied and NOT temporary
            // this copy lasts untill _doc is destroyed
            char *c_n = _doc.allocate_string( n.c_str() );

            rapidxml::xml_node<> *case_node
                    =  _doc.allocate_node(rapidxml::node_element, "case" );
            rapidxml::xml_attribute<> *attrib
                    = _doc.allocate_attribute( "name", c_n );
            case_node->append_attribute(attrib);
            case_node->append_node( createScenes() );
            case_node->append_node( createActionMaps());
            return case_node;
        }

        rapidxml::xml_node<> *createCases( std::string const &c1 = "case1", std::string const &c2 = "case2")
        {
            rapidxml::xml_node<> *cases
                    =  _doc.allocate_node(rapidxml::node_element, "cases" );
            cases->append_node( createCase( "c1" ) );
            cases->append_node( createCase( "c2" ) );
            return cases;
        }


        void createAll()
        {
            rapidxml::xml_node<> *scenes = createScenes();
            _config->append_node( scenes );

            rapidxml::xml_node<> *maps = createActionMaps();
            _config->append_node( maps );

            rapidxml::xml_node<> *cases = createCases();
            _config->append_node( cases );
        }


        std::string _data;
        ProjSettingsRefPtr _settings;
        ProjSettingsSerializer* _ser;
        rapidxml::xml_document<> _doc;
        rapidxml::xml_node<> *_config;
};

// TODO files are unreliable we should generate the xml file here
// bit by bit and check that each individual element is processed correctly
// We can then save these files and read them from disk if necessary.
BOOST_FIXTURE_TEST_SUITE( ConfigUnitTests, SettingsFixture )

BOOST_AUTO_TEST_CASE( test_valid_config )
{
        BOOST_CHECK_NO_THROW( readXML() );
}

BOOST_AUTO_TEST_CASE( name_test )
{
    //is xml valid?
    BOOST_CHECK( readXML() );

    // checking
    BOOST_CHECK_EQUAL( _settings->getName(), "project" );

    // test changing
    _settings->setName("new");
    BOOST_CHECK_EQUAL( _settings->getName(), "new" );
}


//
// SCENES
//
BOOST_AUTO_TEST_CASE( scenes_test )
{
    rapidxml::xml_node<> *scenes = createScenes();
    _config->append_node(scenes);

    //is xml valid?
    BOOST_CHECK( readXML() );

    // Checking
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).name, "name" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).name, "name2" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, true );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).file, "file" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).file, "file2" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).attachto_scene, "attachto_scene2" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).attachto_point, "attachto_point2" );

    // on-off
    _settings->sceneOnOff("name", false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, false );
    // toggle same state
    _settings->sceneOnOff("name", false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, false );
    // toggle back
    _settings->sceneOnOff("name", true );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, true );

}

BOOST_AUTO_TEST_CASE( scenes_set_test )
{
    rapidxml::xml_node<> *scenes = createScenes();
    _config->append_node(scenes);

    //is xml valid?
    BOOST_CHECK( readXML() );

    std::vector<ProjSettings::Scene> scenes_vec;
    ProjSettings::Scene scene("scene",false,"filex","one");
    scenes_vec.push_back(scene);

    _settings->setScenes(scenes_vec);

    // checking
    BOOST_CHECK_EQUAL( _settings->getScenes().size(), 1u );

    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).name, "scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).file, "filex" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_scene, "one" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_point, "" );
}

BOOST_AUTO_TEST_CASE( scene_add_test )
{
    rapidxml::xml_node<> *scenes = createScenes();
    _config->append_node(scenes);

    //is xml valid?
    BOOST_CHECK( readXML() );

    ProjSettings::Scene scene("scene",false,"filex","one");

    _settings->addScene(scene);;

    // checking
    BOOST_CHECK_EQUAL( _settings->getScenes().size(), 3u );

    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).name, "scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).file, "filex" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).attachto_scene, "one" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).attachto_point, "" );
}

BOOST_AUTO_TEST_CASE( scene_add_toggle_test )
{
    rapidxml::xml_node<> *scenes = createScenes();
    _config->append_node(scenes);

    //is xml valid?
    BOOST_CHECK( readXML() );

    std::vector<ProjSettings::Scene> scenes_vec;
    ProjSettings::Scene scene("scene",false,"filex","one");
    scenes_vec.push_back(scene);

    _settings->addScene(scene);

    // checking
    BOOST_CHECK_EQUAL( _settings->getScenes().size(), 3u );

    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).name, "scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).file, "filex" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).attachto_scene, "one" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).attachto_point, "" );

    // adding scene with same name should fail
    _settings->addScene(scene);
    BOOST_CHECK_EQUAL( _settings->getScenes().size(), 3u );

    // toggle
    _settings->sceneOnOff("scene", true);
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).use, true );
    // toggle same
    _settings->sceneOnOff("scene", true);
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).use, true );
    // toggle back
    _settings->sceneOnOff("scene", false);
    BOOST_CHECK_EQUAL( _settings->getScenes().at(2).use, false );
    // toggle phantom
    BOOST_CHECK_NO_THROW( _settings->sceneOnOff("not_present_scene", false) );
}

BOOST_AUTO_TEST_CASE( scene_remove_test )
{
    rapidxml::xml_node<> *scenes = createScenes();
    _config->append_node(scenes);

    //is xml valid?
    BOOST_CHECK( readXML() );

    std::vector<ProjSettings::Scene> scenes_vec;
    ProjSettings::Scene scene("scene",false,"filex","one");
    scenes_vec.push_back(scene);

    _settings->addScene(scene);

    _settings->rmScene("name2");

    //checking
    BOOST_CHECK_EQUAL( _settings->getScenes().size(), 2u );

    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).name, "name" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, true );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).file, "file" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).name, "scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).file, "filex" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).attachto_scene, "one" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).attachto_point, "" );
}

BOOST_AUTO_TEST_CASE( scenes_defaults_test )
{
    rapidxml::xml_node<> *scenes
            =  _doc.allocate_node(rapidxml::node_element, "scenes" );
    rapidxml::xml_node<> *scene
            =  _doc.allocate_node(rapidxml::node_element, "scene" );
    rapidxml::xml_attribute<> *attrib
            = _doc.allocate_attribute( "use", "tru" );
    scene->append_attribute(attrib);
    scenes->append_node(scene);
    _config->append_node(scenes);

    // is xml valid?
    BOOST_CHECK( readXML() );

    // checking
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).name, "Default" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).file, "" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_scene, "" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_point, "" );

}


//
// ACTION MAPS
//
BOOST_AUTO_TEST_CASE( maps_test )
{
    _config->append_node( createActionMaps() );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // checking
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).file, "map1" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(1).file, "map2" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).use, true );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(1).use, false );
}

BOOST_AUTO_TEST_CASE( maps_set_test )
{
    _config->append_node( createActionMaps() );

    // is xml valid?
    BOOST_CHECK( readXML() );

    std::vector<ProjSettings::ActionMap> maps;
    ProjSettings::ActionMap map;
    map.use = false;
    map.file = "new_file";
    maps.push_back(map);

    _settings->setActionMaps(maps);

    // checking
    BOOST_CHECK_EQUAL( _settings->getActionMaps().size(), 1u );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).file, "new_file" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).use, false );
}

BOOST_AUTO_TEST_CASE( map_add_toggle_test )
{
    _config->append_node( createActionMaps() );

    // is xml valid?
    BOOST_CHECK( readXML() );

    ProjSettings::ActionMap map;
    map.use = false;
    map.file = "new_file";

    _settings->addActionMap(map);

    // checking
    BOOST_CHECK_EQUAL( _settings->getActionMaps().size(), 3u );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(2).file, "new_file" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(2).use, false );

    // toggle
    _settings->actionMapOnOff("new_file",true);
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(2).use, true );
    // toggle same
    _settings->actionMapOnOff("new_file",true);
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(2).use, true );
    // toggle back
    _settings->actionMapOnOff("new_file",false);
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(2).use, false );
    // toggle phantom
    BOOST_CHECK_NO_THROW( _settings->actionMapOnOff("Not_present_file",false) );
}

BOOST_AUTO_TEST_CASE( map_rm_test )
{
    _config->append_node( createActionMaps() );

    // is xml valid?
    BOOST_CHECK( readXML() );

    ProjSettings::ActionMap map;
    map.use = false;
    map.file = "new_file";

    _settings->addActionMap(map);

    _settings->rmActionMap("map2");

    // checking
    BOOST_CHECK_EQUAL( _settings->getActionMaps().size(), 2u );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(1).file, "new_file" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(1).use, false );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).file, "map1" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).use, true );
}

BOOST_AUTO_TEST_CASE( maps_defaulting_test )
{
    _config->append_node( createActionMaps( "map", "tru" ) );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // checking
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).file, "map" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).use, false );
}


//
// CASES
//
BOOST_AUTO_TEST_CASE( cases_test )
{
    rapidxml::xml_node<> *cases = createCases();
    _config->append_node(cases);

    //is xml valid?
    BOOST_CHECK( readXML() );

    // cheking case name
    BOOST_CHECK_EQUAL( _settings->getCases().at(0).name, "c1" );
    BOOST_CHECK_EQUAL( _settings->getCases().at(1).name, "c2" );

    // checking scenes of c1
    std::vector<ProjSettings::Scene> scenes = _settings->getCases().at(0).scenes;

    BOOST_CHECK_EQUAL( scenes.at(0).name, "name" );
    BOOST_CHECK_EQUAL( scenes.at(1).name, "name2" );
    BOOST_CHECK_EQUAL( scenes.at(0).use, true );
    BOOST_CHECK_EQUAL( scenes.at(1).use, false );
    BOOST_CHECK_EQUAL( scenes.at(0).file, "file" );
    BOOST_CHECK_EQUAL( scenes.at(1).file, "file2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_scene, "attachto_scene2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_point, "attachto_point2" );

    // checking scenes of c2
    scenes = _settings->getCases().at(1).scenes;

    BOOST_CHECK_EQUAL( scenes.at(0).name, "name" );
    BOOST_CHECK_EQUAL( scenes.at(1).name, "name2" );
    BOOST_CHECK_EQUAL( scenes.at(0).use, true );
    BOOST_CHECK_EQUAL( scenes.at(1).use, false );
    BOOST_CHECK_EQUAL( scenes.at(0).file, "file" );
    BOOST_CHECK_EQUAL( scenes.at(1).file, "file2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_scene, "attachto_scene2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_point, "attachto_point2" );

    // checking action maps of c1
    std::vector<ProjSettings::ActionMap> maps = _settings->getCases().at(0).actionMaps;

    BOOST_CHECK_EQUAL( maps.at(0).file, "map1" );
    BOOST_CHECK_EQUAL( maps.at(1).file, "map2" );
    BOOST_CHECK_EQUAL( maps.at(0).use, true );
    BOOST_CHECK_EQUAL( maps.at(1).use, false );

    // checking action maps of c2
    maps = _settings->getCases().at(1).actionMaps;

    BOOST_CHECK_EQUAL( maps.at(0).file, "map1" );
    BOOST_CHECK_EQUAL( maps.at(1).file, "map2" );
    BOOST_CHECK_EQUAL( maps.at(0).use, true );
    BOOST_CHECK_EQUAL( maps.at(1).use, false );

    // removing map

}

BOOST_AUTO_TEST_CASE( cases_set_test )
{
    rapidxml::xml_node<> *cases = createCases();
    _config->append_node(cases);

    //is xml valid?
    BOOST_CHECK( readXML() );

    // scenes
    std::vector<ProjSettings::Scene> scenes_vec;
    ProjSettings::Scene scene("scene",false,"filex","one");
    scenes_vec.push_back(scene);
    // action maps
    std::vector<ProjSettings::ActionMap> maps;
    ProjSettings::ActionMap map;
    map.use = false;
    map.file = "new_file";
    maps.push_back(map);

    ProjSettings::Case c( "new_case", scenes_vec, maps );
    std::vector<ProjSettings::Case> cases_vec;
    cases_vec.push_back(c);

    _settings->setCases(cases_vec);

    // checking
    BOOST_CHECK_EQUAL( _settings->getCases().size(), 1u );

    std::vector<ProjSettings::Scene> scenes = _settings->getCases().at(0).scenes;

    BOOST_CHECK_EQUAL( scenes.size(), 1u );
    BOOST_CHECK_EQUAL( scenes.at(0).name, "scene" );
    BOOST_CHECK_EQUAL( scenes.at(0).use, false );
    BOOST_CHECK_EQUAL( scenes.at(0).file, "filex" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_scene, "one" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_point, "" );

    std::vector<ProjSettings::ActionMap> maps_vec = _settings->getCases().at(0).actionMaps;

    BOOST_CHECK_EQUAL( maps_vec.at(0).file, "new_file" );
    BOOST_CHECK_EQUAL( maps_vec.at(0).use, false );

}

BOOST_AUTO_TEST_CASE( cases_add_test )
{
    rapidxml::xml_node<> *cases = createCases();
    _config->append_node(cases);

    //is xml valid?
    BOOST_CHECK( readXML() );

    // scenes
    std::vector<ProjSettings::Scene> scenes_vec;
    ProjSettings::Scene scene("scene",false,"filex","one");
    scenes_vec.push_back(scene);
    // action maps
    std::vector<ProjSettings::ActionMap> maps;
    ProjSettings::ActionMap map;
    map.use = false;
    map.file = "new_file";
    maps.push_back(map);

    ProjSettings::Case c( "new_case", scenes_vec, maps );

    _settings->addCase(c);

    // checking
    BOOST_CHECK_EQUAL( _settings->getCases().size(), 3u );

    std::vector<ProjSettings::Scene> scenes = _settings->getCases().at(2).scenes;

    BOOST_CHECK_EQUAL( scenes.size(), 1u );
    BOOST_CHECK_EQUAL( scenes.at(0).name, "scene" );
    BOOST_CHECK_EQUAL( scenes.at(0).use, false );
    BOOST_CHECK_EQUAL( scenes.at(0).file, "filex" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_scene, "one" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_point, "" );

    std::vector<ProjSettings::ActionMap> maps_vec = _settings->getCases().at(2).actionMaps;

    BOOST_CHECK_EQUAL( maps_vec.at(0).file, "new_file" );
    BOOST_CHECK_EQUAL( maps_vec.at(0).use, false );

    // adding scene with same name should fail
    _settings->addCase(c);
    BOOST_CHECK_EQUAL( _settings->getCases().size(), 3u );
}

BOOST_AUTO_TEST_CASE( cases_rm_test )
{
    rapidxml::xml_node<> *cases = createCases();
    _config->append_node(cases);

    //is xml valid?
    BOOST_CHECK( readXML() );

    // scenes
    std::vector<ProjSettings::Scene> scenes_vec;
    ProjSettings::Scene scene("scene",false,"filex","one");
    scenes_vec.push_back(scene);
    // action maps
    std::vector<ProjSettings::ActionMap> maps;
    ProjSettings::ActionMap map;
    map.use = false;
    map.file = "new_file";
    maps.push_back(map);

    ProjSettings::Case c( "new_case", scenes_vec, maps );

    _settings->addCase(c);

    _settings->rmCase("c2");

    // checking
    BOOST_CHECK_EQUAL( _settings->getCases().size(), 2u );

    BOOST_CHECK_EQUAL( _settings->getCases().at(0).name, "c1" );
    BOOST_CHECK_EQUAL( _settings->getCases().at(1).name, "new_case" );
}

BOOST_AUTO_TEST_CASE( complete_test )
{
    createAll();

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Checking scenes
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).name, "name" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).name, "name2" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).use, true );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).use, false );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).file, "file" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).file, "file2" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).attachto_scene, "attachto_scene2" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( _settings->getScenes().at(1).attachto_point, "attachto_point2" );

    //checking action maps
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).file, "map1" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(1).file, "map2" );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(0).use, true );
    BOOST_CHECK_EQUAL( _settings->getActionMaps().at(1).use, false );

    // checking cases
    // cheking case name
    BOOST_CHECK_EQUAL( _settings->getCases().at(0).name, "c1" );
    BOOST_CHECK_EQUAL( _settings->getCases().at(1).name, "c2" );

    // checking scenes of c1
    std::vector<ProjSettings::Scene> scenes = _settings->getCases().at(0).scenes;

    BOOST_CHECK_EQUAL( scenes.at(0).name, "name" );
    BOOST_CHECK_EQUAL( scenes.at(1).name, "name2" );
    BOOST_CHECK_EQUAL( scenes.at(0).use, true );
    BOOST_CHECK_EQUAL( scenes.at(1).use, false );
    BOOST_CHECK_EQUAL( scenes.at(0).file, "file" );
    BOOST_CHECK_EQUAL( scenes.at(1).file, "file2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_scene, "attachto_scene2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_point, "attachto_point2" );

    // checking scenes of c2
    scenes = _settings->getCases().at(1).scenes;

    BOOST_CHECK_EQUAL( scenes.at(0).name, "name" );
    BOOST_CHECK_EQUAL( scenes.at(1).name, "name2" );
    BOOST_CHECK_EQUAL( scenes.at(0).use, true );
    BOOST_CHECK_EQUAL( scenes.at(1).use, false );
    BOOST_CHECK_EQUAL( scenes.at(0).file, "file" );
    BOOST_CHECK_EQUAL( scenes.at(1).file, "file2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_scene, "attachto_scene" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_scene, "attachto_scene2" );
    BOOST_CHECK_EQUAL( scenes.at(0).attachto_point, "attachto_point" );
    BOOST_CHECK_EQUAL( scenes.at(1).attachto_point, "attachto_point2" );

    // checking action maps of c1
    std::vector<ProjSettings::ActionMap> maps = _settings->getCases().at(0).actionMaps;

    BOOST_CHECK_EQUAL( maps.at(0).file, "map1" );
    BOOST_CHECK_EQUAL( maps.at(1).file, "map2" );
    BOOST_CHECK_EQUAL( maps.at(0).use, true );
    BOOST_CHECK_EQUAL( maps.at(1).use, false );

    // checking action maps of c2
    maps = _settings->getCases().at(1).actionMaps;

    BOOST_CHECK_EQUAL( maps.at(0).file, "map1" );
    BOOST_CHECK_EQUAL( maps.at(1).file, "map2" );
    BOOST_CHECK_EQUAL( maps.at(0).use, true );
    BOOST_CHECK_EQUAL( maps.at(1).use, false );
}

BOOST_AUTO_TEST_SUITE_END()
