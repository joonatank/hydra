/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file mesh_manager.cpp
 *
 */

/// Interface
#include "mesh_manager.hpp"

/// Necessary for writing and reading mesh files
#include "mesh_serializer.hpp"

/// Necessary for not implemented exception
#include "base/exceptions.hpp"
/// Necessary for M_PI
#include "math/math.hpp"

#include "resource_manager.hpp"

void 
vl::ManagerMeshLoadedCallback::meshLoaded(vl::MeshRefPtr mesh)
{
	assert(owner);
	owner->meshLoaded(name, mesh);
}

vl::MasterMeshLoaderCallback::MasterMeshLoaderCallback(vl::ResourceManagerRefPtr res_man)
	: manager(res_man)
{}

/// Blocks till the mesh is loaded and passes a valid mesh to callback
void 
vl::MasterMeshLoaderCallback::loadMesh(std::string const &fileName, vl::MeshLoadedCallback *cb)
{
	std::clog << "vl::MasterMeshLoaderCallback::loadMesh : " << fileName << std::endl;
	if(!manager)
	{
		BOOST_THROW_EXCEPTION(vl::null_pointer());
	}
	assert(cb);

	Resource data;
	manager->loadMeshResource(fileName, data);

	/// @todo this needs to convert the Resource to a Mesh
	MeshSerializer ser;
	MeshRefPtr mesh(new Mesh(fileName));
	ser.readMesh(mesh, data);

	std::clog << "Read mesh " << mesh->getName() << " from file resource." << std::endl;
	
	cb->meshLoaded(mesh);
}

/// ------------------------------- MeshManager ------------------------------
vl::MeshRefPtr
vl::MeshManager::loadMesh(std::string const &file_name)
{
	vl::MeshRefPtr mesh;
	/// Mesh has already been created
	if(hasMesh(file_name))
	{
		mesh = getMesh(file_name);
	}
	else
	{
		if(!_load_callback)
		{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

		MeshLoadedCallback *cb = new ManagerMeshLoadedCallback(file_name, this);
		/// Blocking callback
		_load_callback->loadMesh(file_name, cb);
		delete cb;	// can delete because loader was blocking
		
		// mesh is now loaded
		assert(hasMesh(file_name));
		mesh = getMesh(file_name);
	}

	return mesh;
}

void
vl::MeshManager::loadMesh(std::string const &file_name,  MeshLoadedCallback *cb)
{
	assert(cb);

	if(hasMesh(file_name))
	{
		cb->meshLoaded(getMesh(file_name));
	}
	else
	{
		ListenerMap::iterator iter = _waiting_for_loading.find(file_name);
		if(iter != _waiting_for_loading.end())
		{
			/// assume that this is not called twice with same cb
			iter->second.push_back(cb);
			// No need to call load it's already called
		}
		else
		{
			_waiting_for_loading[file_name].push_back(cb);
			_load_callback->loadMesh(file_name, new ManagerMeshLoadedCallback(file_name, this));
		}
	}
}

void
vl::MeshManager::writeMesh(vl::MeshRefPtr, std::string const &file_name)
{
	std::clog << "vl::MeshManager::writeMesh" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

vl::MeshRefPtr
vl::MeshManager::createPlane(std::string const &name, Ogre::Real size_x, Ogre::Real size_y, 
	Ogre::Vector3 normal, uint16_t tesselation_x, uint16_t tesselation_y)
{
	if(normal.isZeroLength() || tesselation_x == 0 || tesselation_y == 0 || size_x <= 0 || size_y <= 0)
	{
		// @todo replace with real exception
		BOOST_THROW_EXCEPTION(vl::exception());
	}
	if(hasMesh(name))
	{
		BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name));
	}

	/// Mesh is created with the assumption that normal is +y
	/// so we rotate every vertex to the normal
	Ogre::Quaternion vert_rot = Ogre::Vector3(0, 1, 0).getRotationTo(normal);

	MeshRefPtr mesh(new Mesh(name));
	mesh->sharedVertexData = new VertexData;

	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_POSITION, Ogre::VET_FLOAT3);
	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_NORMAL, Ogre::VET_FLOAT3);
	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_TEXTURE_COORDINATES, Ogre::VET_FLOAT2);
	/// @todo add tangents

	uint16_t M = tesselation_x;
	uint16_t N = tesselation_y;
	for(uint16_t m = 0; m < M+1; ++m)
	{
		for(uint16_t n = 0; n < N+1; ++n)
		{
			Vertex vert;
			Ogre::Vector3 pos(size_x*m/M - size_x/2, 0, size_y*n/N - size_y/2);
			vert.position = vert_rot*pos;
			vert.normal = normal;
			vert.uv = Ogre::Vector2(((double)m)/M, ((double)n)/N);
			mesh->sharedVertexData->addVertex(vert);
		}
	}

	mesh->calculateBounds();

	SubMesh *sub = mesh->createSubMesh();
	/// @todo add material (or not?) some clear default would be good
	for(uint16_t m = 0; m < M; ++m)
	{
		for(uint16_t n = 0; n < N; ++n)
		{
			sub->addFace(m*(N+1)+n, m*(N+1)+(n+1), (m+1)*(N+1)+n);
			sub->addFace((m+1)*(N+1)+n, m*(N+1)+(n+1), (m+1)*(N+1)+(n+1));
		}
	}

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr
vl::MeshManager::createSphere(std::string const &name, Ogre::Real radius, uint16_t longitude, uint16_t latitude)
{
	std::clog << "vl::MeshManager::createSphere" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
	if(hasMesh(name))
	{
		BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name));
	}
	
	// (x, y, z) = (sin(Pi * m/M) cos(2Pi * n/N), sin(Pi * m/M) sin(2Pi * n/N), cos(Pi * m/M))
	// where M is latitude and N is longitude
	// might need to adjust M with 1 or 2
	MeshRefPtr mesh(new Mesh(name));
	uint16_t M = latitude;
	uint16_t N = longitude;
	for(uint16_t m = 0; m < M; ++m)
	{
		for(uint16_t n = 0; n < N; ++n)
		{
			vl::scalar x = std::sin(M_PI * m/M) * std::cos(2*M_PI * n/N);
			vl::scalar y = std::sin(M_PI * m/M) * std::sin(2*M_PI * n/N);
			vl::scalar z = std::cos(M_PI * m/M);
		}
	}

}

vl::MeshRefPtr
vl::MeshManager::createCube(std::string const &name, Ogre::Vector3 size)
{
	std::clog << "vl::MeshManager::createCube" << std::endl;
	if(size.isZeroLength())
	{
		// @todo replace with real exception
		BOOST_THROW_EXCEPTION(vl::exception());
	}
	if(hasMesh(name))
	{
		BOOST_THROW_EXCEPTION(vl::duplicate() << vl::name(name));
	}

	MeshRefPtr mesh(new Mesh(name));
	mesh->sharedVertexData = new VertexData;

	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_POSITION, Ogre::VET_FLOAT3);
	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_NORMAL, Ogre::VET_FLOAT3);
	mesh->sharedVertexData->vertexDeclaration.addSemantic(Ogre::VES_TEXTURE_COORDINATES, Ogre::VET_FLOAT2);
	/// @todo add tangents

	// y direction
	for(uint16_t m = 0; m < 2; ++m)
	{
		// x direction
		for(uint16_t n = 0; n < 2; ++n)
		{
			// z direction
			for(uint16_t l = 0; l < 2; ++l)
			{
				Vertex vert;
				vl::scalar x = size.x *(n - 0.5);
				vl::scalar y = size.z *(m - 0.5);
				vl::scalar z = size.y *(l - 0.5);
				vert.position = Ogre::Vector3(x, y, z);
				vert.normal = vert.position;
				vert.normal.normalise();
				// @todo fix UV coords
				//vert.uv = Ogre::Vector2(((double)m)/M, ((double)n)/N);
				mesh->sharedVertexData->addVertex(vert);
			}
		}
	}

	mesh->calculateBounds();

	SubMesh *sub = mesh->createSubMesh();
	/// @todo add material (or not?) some clear default would be good
	// bottom (0, 1, 2, 3)
	sub->addFace(0, 2, 1);
	sub->addFace(1, 2, 3);
	// left side (0, 1, 4, 5)
	sub->addFace(0, 1, 4);
	sub->addFace(4, 1, 5);
	// back side (0, 2, 4, 6)
	sub->addFace(0, 4, 2);
	sub->addFace(2, 4, 6);
	// fron side (1, 3, 5, 7)
	sub->addFace(1, 3, 5);
	sub->addFace(5, 3, 7);
	// right side
	sub->addFace(3, 2, 6);
	sub->addFace(3, 6, 7);
	// top side
	sub->addFace(4, 5, 6);
	sub->addFace(6, 5, 7);

	_meshes[name] = mesh;
	return mesh;
}

vl::MeshRefPtr 
vl::MeshManager::getMesh(std::string const &name)
{
	MeshMap::iterator iter = _meshes.find(name);
	if(iter != _meshes.end())
	{ return iter->second; }

	/// @todo replace with real exception
	BOOST_THROW_EXCEPTION(vl::exception());
}

bool 
vl::MeshManager::hasMesh(std::string const &name) const
{
	MeshMap::const_iterator iter = _meshes.find(name);
	if(iter != _meshes.end())
	{ return true; }

	return false;
}

void
vl::MeshManager::cleanup_unused(void)
{
	std::clog << "vl::MeshManager::cleanup_unused" << std::endl;
	BOOST_THROW_EXCEPTION(vl::not_implemented());
}

void 
vl::MeshManager::meshLoaded(std::string const &mesh_name, vl::MeshRefPtr mesh)
{
	std::clog << "vl::MeshManager::meshLoaded : " << mesh_name 
		<< " :  waiting for loading size " << _waiting_for_loading.size() << std::endl;
	// add to loaded stack
	assert(!hasMesh(mesh_name));
	_meshes[mesh_name] = mesh;

	// check listeners
	ListenerMap::iterator iter = _waiting_for_loading.find(mesh_name);
	if(iter != _waiting_for_loading.end())
	{
		for(size_t i = 0; i < iter->second.size(); ++i)
		{
			iter->second.at(i)->meshLoaded(mesh);
		}
		_waiting_for_loading.erase(iter);
	}
	// else this was called using a blocking loader
}
