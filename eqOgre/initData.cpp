#include "root.hpp"
#include "camera.hpp"

#include "initData.hpp"

eq::net::DataOStream &
eqOgre::operator<<( eq::net::DataOStream& os, eqOgre::NameIDPair const &pair )
{
	os << pair.first << pair.second;
	return os;
}

eq::net::DataIStream &
eqOgre::operator>>( eq::net::DataIStream& is, eqOgre::NameIDPair &pair )
{
	is >> pair.first >> pair.second;
	return is;
}

eqOgre::InitData::InitData( void ) //eqOgre::Root *root, eqOgre::Camera *cam);
	: _rootID( EQ_ID_INVALID )
{
	_scene_manager.second = EQ_ID_INVALID;
	_camera.second = EQ_ID_INVALID;
}

eqOgre::InitData::~InitData()
{
 //   setFrameDataID( EQ_ID_INVALID );
}

/*
unsigned int
eqOgre::InitData::getFrame( void ) const
{ 
	if( _root )
	{ return _root->getNextFrameNumber(); }
	else
	{ return 0; }
}
*/

void
eqOgre::InitData::getInstanceData( eq::net::DataOStream& os )
{
	os << _rootID << _scene_manager << _camera;
	/*
	if( _root )
	{
		bool ret = getSession()->registerObject( _root );
		EQASSERT( ret );
		os << _root->getID();
	}
	else
	{ os << EQ_ID_INVALID; }
	*/

	/*
	if( _camera )
	{ os << _camera->getID(); }
	else
	{ os << EQ_ID_INVALID; }
	*/
//    os << _frameDataID << _windowSystem << _renderMode << _useGLSL << _invFaces
//       << _logo;
}

void
eqOgre::InitData::applyInstanceData( eq::net::DataIStream& is )
{
//	uint32_t root = EQ_ID_INVALID;
//	uint32_t camera = EQ_ID_INVALID;
	is >> _rootID >> _scene_manager >> _camera;

	//EQASSERT( root != EQ_ID_INVALID && camera != EQ_ID_INVALID );
	//EQASSERT( _root ); //&&_camera );

//	getSession()->mapObject( _root, root );	
//	getSession()->mapObject( _camera, camera );	

 //   is >> _frameDataID >> _windowSystem >> _renderMode >> _useGLSL >> _invFaces
  //     >> _logo;

  //  EQASSERT( _frameDataID != EQ_ID_INVALID );
    EQINFO << "New InitData instance" << std::endl;
}

