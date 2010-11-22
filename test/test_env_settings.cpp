#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif
#define BOOST_TEST_MODULE settings

#include <boost/test/unit_test.hpp>

#include "base/envsettings.hpp"
#include "base/exceptions.hpp"
#include "base/rapidxml_print.hpp"

#include <boost/filesystem.hpp>
#include <iostream>

#include <OGRE/OgreStringConverter.h>

using vl::EnvSettings;
using vl::EnvSettingsRefPtr;
using vl::EnvSettingsSerializer;

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
        rapidxml::xml_node<> *root = doc.allocate_node(rapidxml::node_element, "eqc" );
        doc.append_node( root );
        rapidxml::xml_node<> *path = doc.allocate_node(rapidxml::node_element, "file", "/not/real/path" );
        root->append_node(path);

        EnvSettingsRefPtr settings( new EnvSettings() );
        EnvSettingsSerializer ser( settings );

        // Create the test data
        std::string data;
        rapidxml::print(std::back_inserter(data), doc, 0);

        // Parse
        BOOST_CHECK( !ser.readString(data) );
}

struct SettingsFixture
{
        SettingsFixture( void )
                : _settings( new EnvSettings() ), _ser(0), _doc(), _config(0)
        {
            _ser = new EnvSettingsSerializer( _settings );
            _config = _doc.allocate_node(rapidxml::node_element, "env_config" );
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
                return _settings->getPlugins().empty()
                        && _settings->getEqc().empty()
                        && _settings->getTracking().empty();

        }


        rapidxml::xml_node<> *createPlugins( std::string const &p1,  std::string const &u1,
                                             std::string const &p2,  std::string const &u2 )
        {
            // allocating to ensure string is copied and NOT temporary
            // this copy lasts untill _doc is destroyed
            char *c_p1 = _doc.allocate_string( p1.c_str() );
            char *c_p2 = _doc.allocate_string( p2.c_str() );
            char *c_u1 = _doc.allocate_string( u1.c_str() );
            char *c_u2 = _doc.allocate_string( u2.c_str() );

            rapidxml::xml_node<> *plugins
                    =  _doc.allocate_node(rapidxml::node_element, "plugins" );

            rapidxml::xml_node<> *plugin
                        =  _doc.allocate_node(rapidxml::node_element, "plugin", c_p1 );
            rapidxml::xml_attribute<> *attrib
                    = _doc.allocate_attribute( "use", c_u1 );
            plugin->append_attribute(attrib);
            plugins->append_node(plugin);

            plugin =  _doc.allocate_node(rapidxml::node_element, "plugin", c_p2 );
            attrib = _doc.allocate_attribute( "use", c_u2 );
            plugin->append_attribute(attrib);
            plugins->append_node(plugin);

            return plugins;
        }

        rapidxml::xml_node<> *createTracking( std::vector<std::string> const &trackingFiles )
        {
                rapidxml::xml_node<> *tracking
                        = _doc.allocate_node(rapidxml::node_element, "tracking" );

                for( unsigned int i = 0; i < trackingFiles.size(); i++ )
                {
                    // allocating to ensure string is copied and NOT temporary
                    // this copy lasts untill _doc is destroyed
                    char *c_track = _doc.allocate_string( trackingFiles.at(i).c_str() );

                    rapidxml::xml_node<> *node = _doc.allocate_node(
                            rapidxml::node_element, "file", c_track );
                    tracking->append_node( node );
                }
                return tracking;
        }

        rapidxml::xml_node<> *createEqc( std::string const &eqcFile )
        {
                // allocating to ensure string is copied and NOT temporary
                // this copy lasts untill _doc is destroyed
                char *c_eqc = _doc.allocate_string( eqcFile.c_str() );

                rapidxml::xml_node<> *eqc
                        = _doc.allocate_node(rapidxml::node_element, "eqc" );
                rapidxml::xml_node<> *node = _doc.allocate_node(
                        rapidxml::node_element, "file", c_eqc );
                eqc->append_node( node );

                return eqc;
        }

        void createAll()
        {
            // Plugin node
            rapidxml::xml_node<> *plugins = createPlugins("plugin_1", "false",
                                                          "plugin_2", "true" );
            _config->append_node( plugins );

            // Eqc node
            rapidxml::xml_node<> *eqc = createEqc("/eqc/path" );
            _config->append_node( eqc );

            // Vector of tracking files
            std::vector<std::string> tracks;
            tracks.push_back("/1/track");
            tracks.push_back("/2/track");

            // Tracking node
            rapidxml::xml_node<> *tracking = createTracking( tracks );
            _config->append_node( tracking );
        }


        std::string _data;
        EnvSettingsRefPtr _settings;
        EnvSettingsSerializer* _ser;
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

BOOST_AUTO_TEST_CASE( plugins_test )
{
    // plugin node
    std::string const BOOL_F("false");
    std::string const BOOL_T("true");
    std::string const PLUGIN1("test1");
    std::string const PLUGIN2("test2");

    rapidxml::xml_node<> *plugins = createPlugins(PLUGIN1, BOOL_F,
                                                  PLUGIN2, BOOL_T);
    _config->append_node( plugins );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Does getPlugins and serializer work correctly
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).first, "test1" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, false );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(1).first, "test2" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(1).second, true );

    // does pluginOnOff work?
    _settings->pluginOnOff("test1",true);
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, true );
    // toggle to same state
    _settings->pluginOnOff("test1",true);
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, true );
    // back to false
    _settings->pluginOnOff("test1",false);
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, false );
}

BOOST_AUTO_TEST_CASE( plugins_add_test )
{
    // is xml valid?
    BOOST_CHECK( readXML() );

    _settings->addPlugin( std::make_pair("plug", true) );

    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).first, "plug" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, true );
}

BOOST_AUTO_TEST_CASE( plugins_bad_use_attribute )
{
    // plugin node
    rapidxml::xml_node<> *plugins = createPlugins("test1", "",
                                                  "test2", "tue" );
    _config->append_node( plugins );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Does use empty and errenous use attributes default to false?
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).first, "test1" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, false );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(1).first, "test2" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(1).second, false );
}

BOOST_AUTO_TEST_CASE( eqc_test )
{
    // Eqc node
    rapidxml::xml_node<> *eqc = createEqc("/test/path" );
    _config->append_node( eqc );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Does getEqc and serializer work correctly?
    BOOST_CHECK_EQUAL( _settings->getEqc(), "/test/path" );
}

BOOST_AUTO_TEST_CASE( eqc_test_multiple_files )
{
    // Eqc node
    rapidxml::xml_node<> *eqc = createEqc("/test/path" );
    rapidxml::xml_node<> *node = _doc.allocate_node(
            rapidxml::node_element, "file", "extra/path" );
    eqc->append_node( node );
    _config->append_node( eqc );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Does getEqc work and values stored correctly
    // Only first eqc should be taken
    BOOST_CHECK_EQUAL( _settings->getEqc(), "/test/path" );
}

BOOST_AUTO_TEST_CASE( tracking_and_remove_test )
{
    // Vector of tracking files
    std::vector<std::string> tracks;
    tracks.push_back("/first/track");
    tracks.push_back("/second/track");

    // Tracking node
    rapidxml::xml_node<> *tracking = createTracking( tracks );
    _config->append_node( tracking );

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Does getTracking and serializer work correctly?
    BOOST_CHECK_EQUAL( _settings->getTracking().at(0), "/first/track" );
    BOOST_CHECK_EQUAL( _settings->getTracking().at(1), "/second/track" );

    // rm
    _settings->removeTracking("no_match");
    BOOST_CHECK_EQUAL( _settings->getTracking().at(0), "/first/track" );
    BOOST_CHECK_EQUAL( _settings->getTracking().at(1), "/second/track" );
    _settings->removeTracking("/first/track");
    BOOST_CHECK_EQUAL( _settings->getTracking().size(), 1u );
    BOOST_CHECK_EQUAL( _settings->getTracking().at(0), "/second/track" );
}

BOOST_AUTO_TEST_CASE( tracking_add_test )
{
    // is xml valid?
    BOOST_CHECK( readXML() );

    _settings->addTracking("new_track");

    // Does getTracking and serializer work correctly?
    BOOST_CHECK_EQUAL( _settings->getTracking().at(0), "new_track" );
}

BOOST_AUTO_TEST_CASE( complete_test )
{
    createAll();

    // is xml valid?
    BOOST_CHECK( readXML() );

    // Does things work?
    // Does getPlugins and serializer work correctly
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).first, "plugin_1" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(0).second, false );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(1).first, "plugin_2" );
    BOOST_CHECK_EQUAL( _settings->getPlugins().at(1).second, true );

    // Does getEqc and serializer work correctly?
    BOOST_CHECK_EQUAL( _settings->getEqc(), "/eqc/path" );

    // Does getTracking and serializer work correctly?
    BOOST_CHECK_EQUAL( _settings->getTracking().at(0), "/1/track" );
    BOOST_CHECK_EQUAL( _settings->getTracking().at(1), "/2/track" );
}

BOOST_AUTO_TEST_SUITE_END()
