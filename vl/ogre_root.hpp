/**	Joonatan Kuosa
 *	2010-02
 *
 *	Class for managing the application, this is basicly a Singleton
 *	the number of instances depends on the number of pipe threads we have.
 *	No enforcement for the Singleton is done.
 *
 *	Manages global settings, like paths and so on.
 *	Also manages the SceneManagers.
 *
 *	This class should not depend on any external library.
 *	All conversions to external libraries should be done in separate class
 *	either through inheritance or aggregation.
 **/

#ifndef VL_OGRE_ROOT_HPP
#define VL_OGRE_ROOT_HPP

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreSceneManager.h>
#include <OGRE/OgreRenderWindow.h>

#include "typedefs.hpp"
#include "base/filesystem.hpp"

#include "base/envsettings.hpp"

namespace vl
{
namespace ogre
{
	class Root
	{
		public :
			Root(vl::config::LogLevel level);

			virtual ~Root( void );

			// Hack to provide native access, we should really never
			// need this for the Root object.
			Ogre::Root *getNative( void )
			{ return _ogre_root; }

			virtual void createRenderSystem( void );

			virtual void init( void );

			virtual Ogre::RenderWindow *createWindow(
					std::string const &name, unsigned int width,
					unsigned int height,
					Ogre::NameValuePairList const &params
						= Ogre::NameValuePairList() );

			virtual Ogre::SceneManager *createSceneManager(
					std::string const &name );

			virtual void addResource( std::string const &resource_path );

			virtual void setupResources( void );

			virtual void loadResources(void);

		protected :
			void _loadPlugins( void );

			void _setupResourceDir( std::string const &dir );

			void _setupResource( std::string const &file, std::string const &typeName );

			Ogre::Root *_ogre_root;

			Ogre::LogManager *_log_manager;

			// Wether we own the Ogre::Root instance
			bool _primary;

			std::vector<std::string> _resources;

	};	// class Root

}	// namespace ogre

}	// namespace vl

#endif
