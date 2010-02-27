/*	Joonatan Kuosa
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
 */

#ifndef VL_OGRE_ROOT_HPP
#define VL_OGRE_ROOT_HPP

#include <eq/client/object.h>

#include "ogre_scene_manager.hpp"

#include "../graph/root.hpp"

#include <OGRE/OgreRoot.h>

#include "ogre_render_window.hpp"
#include "../typedefs.hpp"

namespace vl
{
namespace ogre 
{
	class Root : public vl::graph::Root
	{
		public :
			Root( void )
				: _ogre_root(0)
			{
				_ogre_root = new Ogre::Root( "", "" );
			}

			virtual ~Root( void )
			{}
			
			// Hack to provide native access, we should really never
			// need this for the Root object.
			Ogre::Root *getNative( void )
			{ return _ogre_root; }

			virtual void createRenderingSystem( void )
			{
				EQASSERT( _ogre_root );
#if defined(_DEBUG)
				_ogre_root->loadPlugin("/usr/local/lib/OGRE/RenderSystem_GL_d");
#else
				_ogre_root->loadPlugin("/usr/local/lib/OGRE/RenderSystem_GL");
#endif

				Ogre::RenderSystemList::iterator r_it;
				Ogre::RenderSystemList renderSystems
					= _ogre_root->getAvailableRenderers();
				EQASSERT( !renderSystems.empty() );
				r_it = renderSystems.begin();
				_ogre_root->setRenderSystem(*r_it);
			}

			virtual void init( void )
			{
				_ogre_root->initialise( false );
				Ogre::ResourceGroupManager::getSingleton()
					.addResourceLocation( "resources",
							"FileSystem", "General" );
			}

			virtual vl::graph::RenderWindow *createWindow(
					std::string const &name, unsigned int width,
					unsigned int height,
					vl::NamedValuePairList const &params )
			{
				if( _ogre_root )
				{
					Ogre::NameValuePairList misc;
					vl::NamedValuePairList::const_iterator iter 
						= params.begin();
					for( ; iter != params.end(); ++iter )
					{ misc[iter->first] = iter->second; }

					Ogre::RenderWindow *win =
						_ogre_root->createRenderWindow( name, width, height,
							false, &misc );
					return new vl::ogre::RenderWindow( win );
				}
				return 0;
			}

			// For now we only allow one SceneManager to exists per
			// instance.
			/*
			virtual vl::graph::SceneManager *getSceneManager(
					std::string const &name = std::string() )
			{
				if( !_scene_manager )
				{ _scene_manager = new SceneManager( name ); }
				return _scene_manager;
			}
			*/

		protected :
			virtual vl::graph::SceneManager *_createSceneManager(
					std::string const &name = std::string() )
			{
				return new SceneManager( name );
			}

			Ogre::Root *_ogre_root;

	};	// class Root

}	// namespace ogre 

}	// namespace vl

#endif
