#ifndef EQ_OGRE_INITDATA_H
#define EQ_OGRE_INITDATA_H

#include <eq/eq.h>

#include <OgreString.h>

namespace eqOgre
{
	class Root;
	class Camera;

	typedef std::pair<Ogre::String, uint32_t> NameIDPair;

	eq::net::DataOStream &
	operator<<( eq::net::DataOStream& os, NameIDPair const &pair );

	eq::net::DataIStream &
	operator>>( eq::net::DataIStream& is, NameIDPair &pair );

    class InitData : public eq::Object
    {
    public:
        InitData( void );
        virtual ~InitData (void);

		void setRootID( uint32_t id )
		{ _rootID = id; }

		uint32_t getRootID( void ) const
		{ return _rootID; }

		NameIDPair const& getSceneManager( void ) const
		{ return _scene_manager; }

		void setSceneManager( NameIDPair const &pair )
		{ _scene_manager = pair; }

		void setSceneManager( Ogre::String const &name, uint32_t const id )
		{
			_scene_manager.first = name;
			_scene_manager.second = id;
		}

		NameIDPair const& getCamera( void ) const
		{ return _camera; }

		void setCamera( NameIDPair const &pair )
		{ _camera = pair; }

		void setCamera( Ogre::String const &name, uint32_t const id )
		{
			_camera.first = name;
			_camera.second = id;
		}
		/*
		void setRoot( eqOgre::Root *root )
		{ _root = root; }

		eqOgre::Root *getRoot( void )
		{ return _root; }
		*/
		/*
		void setCamera( eqOgre::Camera *cam)
		{ _camera = cam; }
		*/

		unsigned int getFrame( void ) const;

 //       void setFrameDataID ( const uint32_t id )   { _frameDataID = id; }

//        uint32_t           getFrameDataID() const   { return _frameDataID; }
		/*
        eq::WindowSystem   getWindowSystem() const  { return _windowSystem; }
        mesh::RenderMode   getRenderMode() const    { return _renderMode; }
		bool               useGLSL() const          { return _useGLSL; }
		bool               useInvertedFaces() const { return _invFaces; }
		bool               showLogo() const         { return _logo; }
		*/

    protected:
        virtual void getInstanceData( eq::net::DataOStream& os );
        virtual void applyInstanceData( eq::net::DataIStream& is );

		/*
        void setWindowSystem( const eq::WindowSystem windowSystem )
            { _windowSystem = windowSystem; }
        void setRenderMode( const mesh::RenderMode renderMode )
            { _renderMode = renderMode; }
		void enableGLSL()          { _useGLSL  = true; }
		void enableInvertedFaces() { _invFaces = true; }
		void disableLogo()         { _logo     = false; }
		*/

    private:
//        uint32_t         _frameDataID;
		/*
        eq::WindowSystem _windowSystem;
        mesh::RenderMode _renderMode;
        bool             _useGLSL;
        bool             _invFaces;
        bool             _logo;
		*/
	//	eqOgre::Root *_root;

		uint32_t _rootID;
		NameIDPair _scene_manager;
		NameIDPair _camera;

//		eqOgre::Camera *_camera;
    };
}

#endif // EQ_OGRE_INITDATA_H

