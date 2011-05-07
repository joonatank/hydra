/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file mesh_serializer.cpp
 */

#include "mesh_serializer.hpp"

vl::MeshWriter::MeshWriter( void )
	: _logMgr(0)
	, _resourcegm(0)
	, _math(0)
	, _lodMgr(0)
	, _meshMgr(0)
	, _matMgr(0)
	, _meshSerializer(0)
	, _bufferMgr(0)
	, _skelMgr(0)
	, _skeletonSerializer(0)
{
	_logMgr = new Ogre::LogManager();
	_resourcegm = new Ogre::ResourceGroupManager();
	_math = new Ogre::Math();
	_lodMgr = new Ogre::LodStrategyManager();
	_meshMgr = new Ogre::MeshManager();
	_matMgr = new Ogre::MaterialManager();
	_matMgr->initialise();
	_skelMgr = new Ogre::SkeletonManager();

	_meshSerializer = new Ogre::MeshSerializer();
	_skeletonSerializer = new Ogre::SkeletonSerializer();
	_bufferMgr = new Ogre::DefaultHardwareBufferManager(); // needed because we don't have a rendersystem
}

vl::MeshWriter::~MeshWriter( void )
{
	delete _skeletonSerializer;
	delete _meshSerializer;
	delete _skelMgr;
	delete _matMgr;
	delete _meshMgr;
	delete _bufferMgr;
	delete _lodMgr;
	delete _math;
	delete _resourcegm;
	delete _logMgr;
}

vl::MeshRefPtr
vl::MeshWriter::createMesh( void )
{
	MeshRefPtr mesh(new Mesh);
	return mesh;
}

void 
vl::MeshWriter::writeMesh(vl::MeshRefPtr mesh, std::string const &filename)
{
	Ogre::MeshPtr newMesh = Ogre::MeshManager::getSingleton().createManual("conversion", 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	// TODO add the mesh to an array, because otherwise it can get destroyed
	// also we want to have the refptr at hand.

	// Create shared VertexData for ease of use
	newMesh->sharedVertexData = new Ogre::VertexData();

	// TODO copy from mesh to newMesh
	convert_ogre_geometry(mesh.get(), newMesh->sharedVertexData, newMesh.get());
	convert_ogre_submeshes(mesh.get(), newMesh.get());

	Ogre::MeshSerializer ser;
	ser.exportMesh( newMesh.get(), filename );
}
