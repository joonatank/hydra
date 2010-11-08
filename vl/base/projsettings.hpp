/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *	Added possibility to inherit from this class,
 *	used by distributed project settings to share the project data with slaves.
 */

#ifndef PROJSETTINGS_H
#define PROJSETTINGS_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "rapidxml.hpp"

// TODO add namespace

class ProjSettings
{

public :


        struct ActionMap
        {
            ActionMap( std::string const &f = "",
                       bool u = true,
                       bool c = false )
                : _use(u), _file(f), _changed(c) { }

            void mapOnOff( bool newState )              { _use = newState; _changed = true; }
            bool getUse() const                         { return _use; }
            void setFile( std::string const &fileName ) { _file = fileName; _changed = true; }
            std::string const &getFile() const          { return _file; }
            bool getChanged()                           { return _changed; }
            void clearChanged()                         { _changed = false; }

        private :
            bool _use;
            std::string _file;
            bool _changed;
        };


        struct Script
        {
            Script( std::string const &f = "",
                       bool u = true,
                       bool c = false )
                : _use(u), _file(f), _changed(c) { }

            void scriptOnOff( bool newState )              { _use = newState; _changed = true; }
            bool getUse() const                         { return _use; }
            void setFile( std::string const &fileName ) { _file = fileName; _changed = true; }
            std::string const &getFile() const          { return _file; }
            bool getChanged()                           { return _changed; }
            void clearChanged()                         { _changed = false; }

        private :
            bool _use;
            std::string _file;
            bool _changed;
        };


        struct Scene
        {
            Scene( std::string const &n = "",
                   bool u = false,
                   std::string const &f = "",
                   std::string const &s ="",
                   std::string const &p = "",
                   bool c = false )
               : _name(n), _use(u), _file(f), _attachto_scene(s), _attachto_point(p), _changed(c) { }

            void setName( std::string const &name )             { _name = name; _changed = true; }
            std::string const &getName() const                  { return _name; }
            void sceneOnOff( bool newState )                    { _use = newState; _changed = true; }
            bool getUse() const                                 { return _use; }
            void setFile( std::string const &file )             { _file = file; _changed = true; }
            std::string const &getFile() const                  { return _file; }
            void setAttachtoScene( std::string const &scene )   { _attachto_scene = scene; _changed = true; }
            std::string const &getAttachtoScene() const         { return _attachto_scene; }
            void setAttachtoPoint( std::string const &point )   { _attachto_point = point; _changed = true; }
            std::string const &getAttachtoPoint() const         { return _attachto_point; }
            bool getChanged()                                   { return _changed; }
            void clearChanged()                                 { _changed = false; }

        private :
            std::string _name;
            bool _use;
            std::string _file;
            std::string _attachto_scene;
            std::string _attachto_point;
            bool _changed;
        };


        struct Case
        {
            Case( std::string const &n = "",
                  std::vector<Scene*> const &s = std::vector<Scene*>(),
                  std::vector<ActionMap*> const &a = std::vector<ActionMap*>(),
                  bool c = false )
                : _name(n), _scenes(s), _actionMaps(a), _changed(c) {}

            void setName( std::string const &name )             { _name = name; _changed = true; }
            std::string const &getName() const                  { return _name; }

            Scene* addScene( std::string const &name );
            void rmScene( std::string const &name );
            Scene* getScenePtr( std::string const &sceneName );
            Scene* getScenePtr( unsigned int i ) const;
            unsigned int getNscenes() const { return _scenes.size(); }

            ActionMap* addMap( std::string const &file );
            void rmMap( std::string const &name );
            ActionMap* getMapPtr( std::string const &mapName );
            ActionMap* getMapPtr( unsigned int i ) const;
            unsigned int getNmaps() const { return _actionMaps.size(); }

            Script* addScript( std::string const &file );
            void rmScript( std::string const &name );
            Script* getScriptPtr( std::string const &name );
            Script* getScriptPtr( unsigned int i ) const;
            unsigned int getNscripts() const { return _scripts.size(); }

            bool getChanged();
            void clearChanged();

        private :
            std::string _name;
            std::vector<Scene*> _scenes;
            std::vector<ActionMap*> _actionMaps;
            std::vector<Script*> _scripts;
            bool _changed;
        };

        ProjSettings( std::string const &file = "" );

        virtual ~ProjSettings( void );

        void clear( void );


        ///// CHANGED /////////////////////////////////////////////////////
        bool getChanged( void );
        void clearChanged();


        ///// FILE /////////////////////////////////////////////////////
        std::string const &getFile( void ) const { return _file; }
        void setFile( std::string const &f ) { _file = f; _changed = true; }


        ///// CASES /////////////////////////////////////////////////
        Case* addCase( std::string const &caseName  );
        void rmCase( std::string const &caseName );
        Case* getCasePtr( std::string const &caseName );
        Case* getCasePtr( unsigned int i ) const;
        unsigned int getNcases() const { return _cases.size(); }


        ///// PROJECT GETTERS /////////////////////////////////////////////////
        Case* getCasePtr() { return _projCase; }


protected :

    std::string _file;
    Case* _projCase;
    std::vector<Case*> _cases;
    bool _changed;


};	// class ProjSettings

typedef boost::shared_ptr< ProjSettings > ProjSettingsRefPtr;














class ProjSettingsSerializer
{
public :
        /// Will completely over-ride the provided ProjSettings
        /// when XML data is readed.
        /// Any error in reading will result to defaulting ProjSettings.
        ProjSettingsSerializer( ProjSettingsRefPtr );

        ~ProjSettingsSerializer( void );


        // Read data from string buffer. Buffer is not modified.
        bool readString( std::string const &str );

        void writeString( std::string &str );

protected :
        bool readXML( void );

        void readConfig( rapidxml::xml_node<>* XMLNode );

        void readScenes( rapidxml::xml_node<>* XMLNode, ProjSettings::Case* c );

        void readScene( rapidxml::xml_node<>* XMLNode, ProjSettings::Case* c );

        void readActionMaps( rapidxml::xml_node<>* XMLNode, ProjSettings::Case* c );

        void readScripts( rapidxml::xml_node<>* XMLNode, ProjSettings::Case* c );

        void readCases( rapidxml::xml_node<>* XMLNode );

        void readCase( rapidxml::xml_node<>* XMLNode );


        //write
        void writeConfig( rapidxml::xml_node<> *xml_node );

        void writeScenes( rapidxml::xml_node<> *xml_node, ProjSettings::Case* c );

        void writeMaps( rapidxml::xml_node<> *xml_node, ProjSettings::Case* c );

        void writeScripts( rapidxml::xml_node<> *xml_node, ProjSettings::Case* c );

        void writeCases( rapidxml::xml_node<> *xml_node );


        const char* bool2char( bool b );

        ProjSettingsRefPtr _projSettings;

        // file content needed for rapidxml
        char *_xml_data;

        // doc for writing
        rapidxml::xml_document<> _doc;
};

typedef boost::shared_ptr< ProjSettingsSerializer > ProjSettingsSerializerRefPtr;

#endif // PROJSETTINGS_H
