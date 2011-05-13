/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file mesh.cpp
 */

#include "mesh.hpp"

#include "base/exceptions.hpp"

#include <OGRE/OgreMesh.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreDefaultHardwareBufferManager.h>
#include <OGRE/OgreMeshManager.h>

/// ------------------------------- Global -----------------------------------

Ogre::MeshPtr
vl::create_ogre_mesh(std::string const &name, vl::MeshRefPtr mesh)
{
	std::clog << "vl::create_ogre_mesh : " << name << std::endl;

	Ogre::MeshPtr og_mesh = Ogre::MeshManager::getSingleton().createManual(name, 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// Copy the shared geometry
	// Skip empty
	if( !mesh->sharedVertexData || mesh->sharedVertexData->getNVertices() == 0 ) 
	{
		std::clog << "Empty shared geometry " << mesh->getName() << std::endl;
	}
	else
	{
		std::clog << "Converting shared geometry for " << mesh->getName() << std::endl;
		// Create shared VertexData for ease of use
		og_mesh->sharedVertexData = new Ogre::VertexData;
		convert_ogre_geometry(mesh->sharedVertexData, og_mesh->sharedVertexData);
	}

	// Set bounding box
	og_mesh->_setBoundingSphereRadius(mesh->getBoundingSphereRadius());
	og_mesh->_setBounds(mesh->getBounds(), false);

	// Convert submeshes
	convert_ogre_submeshes(mesh.get(), og_mesh.get());

	return og_mesh;
}

void 
vl::convert_ogre_geometry(vl::VertexData const *vertexData, Ogre::VertexData *og_vertexData)
{
	assert(vertexData && og_vertexData);

	unsigned char *pVert = 0;
	float *pFloat = 0;
	Ogre::uint16 *pShort = 0;
	Ogre::uint8 *pChar = 0;
	Ogre::ARGB *pCol = 0;

	Ogre::VertexDeclaration *decl = og_vertexData->vertexDeclaration;
	Ogre::VertexBufferBinding *bind = og_vertexData->vertexBufferBinding;
	unsigned short bufCount = 0;
	unsigned short totalTexCoords = 0; // across all buffers

	// Assume single vertexbuffer

	size_t offset = 0;

	// Add element
	std::clog << "Adding " << vertexData->vertexDeclaration.getNSemantics() << " vertex schematics." << std::endl;
	for(size_t i = 0; i < vertexData->vertexDeclaration.getNSemantics(); ++i)
	{
		VertexDeclaration::Semantic semantic = vertexData->vertexDeclaration.getSemantic(i);
		decl->addElement(bufCount, offset, semantic.second, semantic.first);
		offset += Ogre::VertexElement::getTypeSize(semantic.second);
	}

	og_vertexData->vertexCount = vertexData->getNVertices();

	std::clog << "Converting " << og_vertexData->vertexCount << " vertices." << std::endl;

	assert(og_vertexData->vertexDeclaration->getVertexSize(bufCount) == vertexData->vertexDeclaration.vertexSize());

	// Now create the vertex buffer
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(offset, og_vertexData->vertexCount, 
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	// Bind it
	bind->setBinding(bufCount, vbuf);
	// Lock it
	pVert = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	// Get the element list for this buffer alone
	Ogre::VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufCount);

	// Now the buffer is set up, parse all the vertices
	/// @todo this can probably use a direct copy of the Vertex data
	for(size_t i = 0; i < vertexData->getNVertices(); ++i)
	{
		// Now parse the elements, ensure they are all matched
		Ogre::VertexDeclaration::VertexElementList::const_iterator ielem, ielemend;

		ielemend = elems.end();
		for (ielem = elems.begin(); ielem != ielemend; ++ielem)
		{
			const Ogre::VertexElement& elem = *ielem;
			// Find child for this element
			switch(elem.getSemantic())
			{
				case Ogre::VES_POSITION:
				{
					elem.baseVertexPointerToElement(pVert, &pFloat);

					Ogre::Vector3 const &pos = vertexData->getVertex(i).position;
					*pFloat++ = pos.x;
					*pFloat++ = pos.y;
					*pFloat++ = pos.z;
				}
				break;

			case Ogre::VES_NORMAL:
				elem.baseVertexPointerToElement(pVert, &pFloat);

				*pFloat++ = vertexData->getVertex(i).normal.x;
				*pFloat++ = vertexData->getVertex(i).normal.y;
				*pFloat++ = vertexData->getVertex(i).normal.z;
				break;

			case Ogre::VES_TANGENT:
				elem.baseVertexPointerToElement(pVert, &pFloat);

				*pFloat++ = vertexData->getVertex(i).tangent.x;
				*pFloat++ = vertexData->getVertex(i).tangent.y;
				*pFloat++ = vertexData->getVertex(i).tangent.z;
				break;
			case Ogre::VES_BINORMAL:
				/*	Binormals are not supported
				elem.baseVertexPointerToElement(pVert, &pFloat);

				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("x"));
				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("y"));
				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("z"));
				*/
				break;
			case Ogre::VES_DIFFUSE:
//				std::cout << "Writing Vertex diffuse data." << std::endl;
				elem.baseVertexPointerToElement(pVert, &pCol);
				{
					Ogre::ColourValue cv = vertexData->getVertex(i).diffuse;
					*pCol++ = Ogre::VertexElement::convertColourValue(cv, elem.getType());
				}
				break;
			case Ogre::VES_SPECULAR:
//				std::cout << "Writing Vertex specular data." << std::endl;
				elem.baseVertexPointerToElement(pVert, &pCol);
				{
					Ogre::ColourValue cv = vertexData->getVertex(i).specular;
					*pCol++ = Ogre::VertexElement::convertColourValue(cv, elem.getType());
				}
				break;
			case Ogre::VES_TEXTURE_COORDINATES:
//				std::cout << "Writing Vertex texture coordinates." << std::endl;
				switch (elem.getType()) 
				{
				case Ogre::VET_FLOAT1:
					elem.baseVertexPointerToElement(pVert, &pFloat);
					*pFloat++ = vertexData->getVertex(i).uv.x;
					break;

				case Ogre::VET_FLOAT2:
					elem.baseVertexPointerToElement(pVert, &pFloat);
					*pFloat++ = vertexData->getVertex(i).uv.x;
					*pFloat++ = vertexData->getVertex(i).uv.y;
					break;

				case Ogre::VET_FLOAT3:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_FLOAT3"));
					break;

				case Ogre::VET_FLOAT4:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_FLOAT4"));
					break;

				case Ogre::VET_SHORT1:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT1"));
					break;

				case Ogre::VET_SHORT2:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT2"));
					break;

				case Ogre::VET_SHORT3:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT3"));
					break;

				case Ogre::VET_SHORT4:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT4"));
					break;

				case Ogre::VET_UBYTE4:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_UBYTE4"));
					break;

				case Ogre::VET_COLOUR:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_COLOUR"));
					break;

				case Ogre::VET_COLOUR_ARGB:
				case Ogre::VET_COLOUR_ABGR: 
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_COLOUR_ABGR"));
					break;
				}

				break;
			default:
				break;
			}
		}	// semantic
		pVert += vbuf->getVertexSize();
	}	// vertices

	bufCount++;
    vbuf->unlock();
	// Vertexbuffer done
}

void 
vl::convert_ogre_submeshes(vl::Mesh const *mesh, Ogre::Mesh *og_mesh)
{
	std::clog << "vl::convert_ogre_submeshes : " << og_mesh->getName() << std::endl;
	for(size_t i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
		// All children should be submeshes 
		Ogre::SubMesh *og_sm = og_mesh->createSubMesh();

		vl::SubMesh const *sm = mesh->getSubMesh(i); 
		convert_ogre_submesh(sm, og_sm);

		og_mesh->nameSubMesh(sm->getName(), i);
    }
}

void 
vl::convert_ogre_submesh(vl::SubMesh const *sm, Ogre::SubMesh *og_sm)
{
	std::clog << "vl::convert_ogre_submesh" << std::endl;
	assert(sm);
	assert(og_sm);

	og_sm->setMaterialName(sm->getMaterial());

	if(sm->indexData.indexCount() == 0)
	{
		std::clog << "Something really fishy SubMesh index count = 0" << std::endl;
	}
	else
	{
		// Indeces
		og_sm->operationType = sm->operationType;
		og_sm->indexData->indexCount = sm->indexData.indexCount();

		std::clog << "Converting index buffers : operation type = " << og_sm->operationType 
			<< " index count = " << og_sm->indexData->indexCount << std::endl;

		if(sm->indexData.getIndexSize() == vl::IT_32BIT)
		{
			std::clog << "Copying 32bit index buffers." << std::endl;
			// Allocate space
			Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
				createIndexBuffer(Ogre::HardwareIndexBuffer::IT_32BIT, 
					og_sm->indexData->indexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC,
					false);

			og_sm->indexData->indexBuffer = ibuf;
			unsigned int *pInt = 0;
			unsigned short *pShort = 0;
		
			pInt = static_cast<unsigned int*>(ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

			assert(sm->indexData.getVec32().size() == sm->indexData.indexCount());
			/// @todo replace with memcpy
			::memcpy(pInt, sm->indexData.getBuffer32(), sm->indexData.indexCount()*sizeof(uint32_t));

			ibuf->unlock();
		}
		else
		{
			std::clog << "Copying 16bit index buffers." << std::endl;
			// Allocate space
			Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
				createIndexBuffer(Ogre::HardwareIndexBuffer::IT_16BIT, 
					og_sm->indexData->indexCount,
					Ogre::HardwareBuffer::HBU_DYNAMIC,
					false);

			og_sm->indexData->indexBuffer = ibuf;
			unsigned short *pShort = static_cast<unsigned short *>(ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

			assert(sm->indexData.getVec16().size() == sm->indexData.indexCount());
			::memcpy(pShort, sm->indexData.getBuffer16(), sm->indexData.indexCount()*sizeof(uint16_t));

			ibuf->unlock();
		}
	}


	// Geometry
	og_sm->useSharedVertices = sm->useSharedGeometry;
	if(!sm->useSharedGeometry)
	{
		std::clog << "Converting non shared geometry." << std::endl;
		assert(sm->vertexData);
		og_sm->vertexData = new Ogre::VertexData;
		convert_ogre_geometry(sm->vertexData, og_sm->vertexData);
	}

	// texture aliases, NOT supported
	/*
	TiXmlElement* textureAliasesNode = smElem->FirstChildElement("textures");
	if(textureAliasesNode)
	{ readTextureAliases(textureAliasesNode, sm); }
	*/

	// Bone assignments, NOT supported
	/*
	TiXmlElement* boneAssigns = smElem->FirstChildElement("boneassignments");
	if(boneAssigns)
	{ readBoneAssignments(boneAssigns, sm); }
	*/
}

void 
vl::calculate_bounds(vl::VertexData const *vertexData, Ogre::AxisAlignedBox &box, Ogre::Real &sphere)
{
	Ogre::Vector3 min, max;
	Ogre::Real maxSquaredRadius;

	Ogre::Vector3 pos = vertexData->getVertex(0).position;
	min = max = pos;
	maxSquaredRadius = pos.squaredLength();	
	for(size_t i = 1; i < vertexData->getNVertices(); ++i)
	{
		pos = vertexData->getVertex(i).position;
		min.makeFloor(pos);
		max.makeCeil(pos);
		maxSquaredRadius = std::max(pos.squaredLength(), maxSquaredRadius);
	}
	
	box.setExtents(min, max);
	sphere = Ogre::Math::Sqrt(maxSquaredRadius);
}

std::ostream &
vl::operator<<(std::ostream &os, vl::Vertex const &v)
{
	os << "Vertex : position " << v.position << " : normal " << v.normal
		<< " : tangent " << v.tangent << " : uvs " << v.uv;
	return os;
}

std::ostream &
vl::operator<<( std::ostream &os, vl::Mesh const &m )
{
	if(m.sharedVertexData)
	{
		os << "Mesh " << m.getName() << " : vertices = ";
		for( size_t i = 0; i < m.sharedVertexData->getNVertices(); ++i )
		{
			os << m.sharedVertexData->getVertex(i).position << ", ";
		}
		os << std::endl;
	}
	else
	{
		os << "Mesh " << m.getName() << " has no vertices." << std::endl;
	}
	return os;
}

std::ostream &
vl::operator<<( std::ostream &os, vl::SubMesh const &m )
{
	return os;
}

/// ----------------------------- SubMesh ------------------------------------
/// @todo the face system does not really work this way
/// different GL modes (TRIANGLE_FAN and so on) determine
/// what size a face is, it's not always a tuple of three
/// No I'm probably wrong here, the number of Vertices is
/// determined by the draw mode but the face number is always divisable
/// with three.
/// If operation type = TRIANGLE_LIST then this is fine otherwise
/// this does not work right.
void 
vl::SubMesh::addFace(uint32_t i1, uint32_t i2, uint32_t i3)
{
	bool firstTri = false;
	if(indexData.indexCount() == 0)
	{
		firstTri = true;
	}

	indexData.push_back(i1);
	if(operationType == Ogre::RenderOperation::OT_LINE_LIST)
	{
		indexData.push_back(i2);
	}
	// only need all 3 vertices if it's a trilist or first tri
	else if(operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST || firstTri)
	{
		indexData.push_back(i2);
		indexData.push_back(i3);
	}
}

/// --------------------------------- VertexData -----------------------------
void 
vl::VertexData::setVertex(size_t i, char const *buf, size_t size)
{
	assert(i < _vertices.size());

	Vertex &vert = _vertices.at(i);
	size_t offset = 0;
	for(size_t i = 0; i < vertexDeclaration.getNSemantics(); ++i)
	{
		/// Parse the buffer based on the current Declaration
		VertexDeclaration::Semantic semantic = vertexDeclaration.getSemantic(i);
		
		assert(offset+vertexDeclaration.getTypeSize(semantic.second) <= size);
		switch(semantic.first)
		{
		case Ogre::VES_POSITION:
			if(semantic.second == Ogre::VET_FLOAT3)
			{
				vert.position = Ogre::Vector3((float const *)(buf+offset));
			}
			else
			{ BOOST_THROW_EXCEPTION(vl::not_implemented()); }
			break;
		case Ogre::VES_NORMAL:
			if(semantic.second == Ogre::VET_FLOAT3)
			{
				vert.normal = Ogre::Vector3((float const *)(buf+offset));
			}
			else
			{ BOOST_THROW_EXCEPTION(vl::not_implemented()); }
			break;
		case Ogre::VES_TEXTURE_COORDINATES:
			if(semantic.second == Ogre::VET_FLOAT2)
			{
				vert.uv = Ogre::Vector2((float const *)(buf+offset));
			}
			else
			{ BOOST_THROW_EXCEPTION(vl::not_implemented()); }
			break;
		case Ogre::VES_TANGENT:
			if(semantic.second == Ogre::VET_FLOAT3)
			{
				vert.tangent = Ogre::Vector3((float const *)(buf+offset));
			}
			else
			{ BOOST_THROW_EXCEPTION(vl::not_implemented()); }
			break;
		case Ogre::VES_BLEND_WEIGHTS:
		case Ogre::VES_BLEND_INDICES:
		case Ogre::VES_DIFFUSE:
		case Ogre::VES_SPECULAR:
		case Ogre::VES_BINORMAL:
		default :
			BOOST_THROW_EXCEPTION(vl::not_implemented());
		}
		offset += vertexDeclaration.getTypeSize(semantic.second);
	}

	assert(offset == size);
}

/// --------------------------VertexDeclaration ------------------------------
size_t 
vl::VertexDeclaration::vertexSize(void) const
{
	size_t size = 0;
		
	for(size_t i =0; i < _semantics.size(); ++i)
	{
		size += getTypeSize(_semantics.at(i).second);
	}

	return size;
}

size_t vl::VertexDeclaration::getTypeSize(Ogre::VertexElementType type)
{
	switch(type)
	{
	case Ogre::VET_FLOAT1:
		return 1*sizeof(float);
	case Ogre::VET_FLOAT2:
		return 2*sizeof(float);
	case Ogre::VET_FLOAT3:
		return 3*sizeof(float);
	case Ogre::VET_FLOAT4:
		return 4*sizeof(float);
	case Ogre::VET_COLOUR:
		return 4*sizeof(float);
	case Ogre::VET_SHORT1:
		return 1*sizeof(short);
	case Ogre::VET_SHORT2:
		return 2*sizeof(short);
	case Ogre::VET_SHORT3:
		return 3*sizeof(short);
	case Ogre::VET_SHORT4:
		return 4*sizeof(short);
	case Ogre::VET_UBYTE4:
		return 4*1;
	case Ogre:: VET_COLOUR_ARGB:
		return 4*sizeof(float);
	case Ogre:: VET_COLOUR_ABGR:
		return 4*sizeof(float);
	default:
		return 0;
	}
}

/// ---------------------------- IndexBuffer ---------------------------------
vl::IndexBuffer::IndexBuffer(void)
	: _index_count(0)
	, _buffer_size(IT_16BIT)
{}

vl::IndexBuffer::~IndexBuffer(void)
{}

void 
vl::IndexBuffer::setIndexCount(size_t count)
{
	if(indexCount() != count)
	{
		_resize_buffer(count);
	}
}

size_t 
vl::IndexBuffer::indexCount(void) const
{
	if(_buffer_size == IT_32BIT)
	{
		return _buffer_32.size();
	}
	else
	{
		return _buffer_16.size();
	}
}

void 
vl::IndexBuffer::setIndexSize(vl::INDEX_SIZE size)
{
	if( size != _buffer_size )
	{
		_buffer_size = size;
		_buffer_32.clear();
		_buffer_16.clear();
	}
}

void
vl::IndexBuffer::push_back(uint16_t index)
{
	push_back((uint32_t)(index));
}

void
vl::IndexBuffer::push_back(uint32_t index)
{
	if(_buffer_size == IT_32BIT)
	{
		_buffer_16.push_back(index);
	}
	else
	{
		_buffer_16.push_back((uint16_t)index);
	}
	++_index_count;
}

void 
vl::IndexBuffer::_resize_buffer(size_t size)
{
	if(_buffer_size == IT_32BIT)
	{
		_buffer_32.resize(size);
	}
	else
	{
		_buffer_16.resize(size);
	}
}


/// -------------------------------- Mesh ------------------------------------
vl::Mesh::Mesh(std::string const &name)
	: sharedVertexData(0)
	, _name(name)
	, _bound_radius(0)
{}

void
vl::Mesh::removeSubMesh(uint16_t index)
{
	if(_sub_meshes.size() > index)
	{
		delete _sub_meshes.at(index);
		_sub_meshes.erase(_sub_meshes.begin()+index);
	}
}


vl::Mesh::~Mesh(void)
{
	for( size_t i = 0; i < _sub_meshes.size(); ++i )
	{
		delete _sub_meshes.at(i);
	}
}

vl::SubMesh *
vl::Mesh::createSubMesh(void)
{
	SubMesh *sm = new SubMesh;
	_sub_meshes.push_back(sm);
	return sm;
}

void 
vl::Mesh::nameSubMesh(std::string const &name, uint16_t index)
{
}

void
vl::Mesh::calculateBounds(void)
{
	if(!sharedVertexData || sharedVertexData->getNVertices() == 0)
	{ return; }

	calculate_bounds(sharedVertexData, _bounds, _bound_radius);

	// Iterate over the sub meshes for dedicated geometry
	for(size_t i = 0; i < _sub_meshes.size(); ++i)
	{
		SubMesh *sm = _sub_meshes.at(i);
		if(sm->vertexData)
		{
			Ogre::AxisAlignedBox box;
			Ogre::Real sphere;
			calculate_bounds(sm->vertexData, box, sphere);
			_bound_radius = std::max(_bound_radius, sphere);
			_bounds.merge(box);
		}
	}
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::VertexDeclaration const &decl)
{
	msg << decl.getSemantics();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::VertexDeclaration &decl)
{
	msg >> decl.getSemantics();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::VertexData const &vbuf)
{
	msg << vbuf.vertexDeclaration << vbuf._vertices;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::VertexData &vbuf)
{
	msg >> vbuf.vertexDeclaration >> vbuf._vertices;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::IndexBuffer const &ibf)
{
	msg << ibf.getIndexSize();
	if(ibf.getIndexSize() == IT_32BIT)
	{
		msg << ibf.getVec32();
	}
	else
	{
		msg << ibf.getVec16();
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::IndexBuffer &ibf)
{
	INDEX_SIZE index_size;
	msg >> index_size;
	ibf.setIndexSize(index_size);
	if(index_size == IT_32BIT)
	{
		msg >> ibf.getVec32();
	}
	else
	{
		msg >> ibf.getVec16();
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::SubMesh const &sm)
{
	msg << sm.getName() << sm.getMaterial() << sm.operationType << sm.indexData << sm.useSharedGeometry;

	if(!sm.useSharedGeometry)
	{ msg << *sm.vertexData; }

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::SubMesh &sm)
{
	std::string name;
	std::string material;

	msg >> name >> material >> sm.operationType >> sm.indexData >> sm.useSharedGeometry;
	sm.setName(name);
	sm.setMaterial(material);

	if(!sm.useSharedGeometry)
	{
		if(!sm.vertexData)
			sm.vertexData = new VertexData;
		msg >> *sm.vertexData;
	}
	else if(sm.vertexData)
		delete sm.vertexData;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::Mesh const &mesh)
{
	if(mesh.sharedVertexData)
		msg << true << *mesh.sharedVertexData;
	else
		msg << false;

	msg << mesh.getBoundingSphereRadius() << mesh.getBounds();
	
	msg << mesh.getSubMeshes().size();
	for(size_t i = 0; i < mesh.getSubMeshes().size(); ++i)
	{ msg << *mesh.getSubMeshes().at(i); }

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::Mesh &mesh)
{
	bool has_shared_vertex_data;
	msg >> has_shared_vertex_data;

	if(has_shared_vertex_data)
	{
		if(!mesh.sharedVertexData)
			mesh.sharedVertexData = new VertexData;
		msg >> *mesh.sharedVertexData;
	}
	else
	{ delete mesh.sharedVertexData; }
	
	size_t n_s_meshes;
	msg >> mesh.getBoundingSphereRadius() >> mesh.getBounds() >> n_s_meshes;

	/// Remove old sub meshes if more than the new ones
	while( n_s_meshes < mesh.getNumSubMeshes() )
	{
		mesh.removeSubMesh(mesh.getNumSubMeshes()-1);
	}

	/// Add new sub meshes
	for(size_t i = 0; i < n_s_meshes; ++i)
	{
		SubMesh *sm = 0;
		if(mesh.getNumSubMeshes() > i)
		{ sm = mesh.getSubMesh(i); }
		else
		{ sm = mesh.createSubMesh(); }
		msg >> *sm;
	}

	return msg;
}
