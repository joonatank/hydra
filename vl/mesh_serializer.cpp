/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-02
 *	@file mesh_serializer.cpp
 */

#include "mesh_serializer.hpp"

const unsigned short HEADER_CHUNK_ID = 0x1000;

vl::MeshSerializer::MeshSerializer(void)
	: _ogre_mgr(0)
{
	/// If we don't have Root create temporary Managers
	if(!Ogre::Root::getSingletonPtr())
	{ _ogre_mgr = new OgreManagers(); }
}

vl::MeshSerializer::~MeshSerializer( void )
{
	delete _ogre_mgr;
}

vl::MeshRefPtr
vl::MeshSerializer::createMesh(void)
{
	MeshRefPtr mesh(new Mesh("conversion"));
	return mesh;
}

void 
vl::MeshSerializer::writeMesh(vl::MeshRefPtr mesh, std::string const &filename)
{
	Ogre::MeshPtr og_mesh = vl::create_ogre_mesh("conversion", mesh);
	Ogre::MeshSerializer ser;
	ser.exportMesh(og_mesh.get(), filename);
}

/// @todo this is quite problematic because it needs to read the Data in Ogre
/// format and then convert it to Hydra structure then send it forward
/// before it can be displayed it will be converted back to Ogre format.
void
vl::MeshSerializer::readMesh(vl::MeshRefPtr mesh, vl::Resource &res)
{
	std::clog << "vl::MeshSerializer::readMesh : from resource of size " << res.size() << std::endl;

	ResourceStream stream = res.getStream();
    determineEndianness(stream);

    // Read header and determine the version
    unsigned short headerID;
        
    // Read header ID
    readShorts(stream, &headerID, 1);
        
    if(headerID != HEADER_CHUNK_ID)
    {
		std::clog << "vl::MeshSerializer::readMesh : NO header" << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("File header not found"));
    }

    // Read version
    std::string ver = readString(stream);
    // Jump back to start
    stream.seek(0);

    // Find the implementation to use
	MeshSerializerImpl* impl = new MeshSerializerImpl;

    // Call implementation
    impl->importMesh(stream, mesh.get());
    // Warn on old version of mesh
    /*
	if (ver != mVersionData[0]->versionString)
    {
        std::cout << "WARNING: " << pDest->getName()
            << " is an older format (" << ver << "); you should upgrade it as soon as possible"
            << " using the OgreMeshUpgrade tool.";
    }
	*/
}

/// ------------------------------- Private ----------------------------------
vl::MeshSerializer::OgreManagers::OgreManagers(void)
	: logMgr(0)
	, resGroupMgr(0)
	, math(0)
	, lodMgr(0)
	, meshMgr(0)
	, matMgr(0)
	, bufferMgr(0)
	, skelMgr(0)
{
	logMgr = new Ogre::LogManager();
	resGroupMgr = new Ogre::ResourceGroupManager();
	math = new Ogre::Math();
	lodMgr = new Ogre::LodStrategyManager();
	meshMgr = new Ogre::MeshManager();
	matMgr = new Ogre::MaterialManager();
	matMgr->initialise();
	skelMgr = new Ogre::SkeletonManager();
	// needed because we don't have a rendersystem
	bufferMgr = new Ogre::DefaultHardwareBufferManager();
}

vl::MeshSerializer::OgreManagers::~OgreManagers(void)
{
	delete skelMgr;
	delete matMgr;
	delete meshMgr;
	delete bufferMgr;
	delete lodMgr;
	delete math;
	delete resGroupMgr;
	delete logMgr;
}
