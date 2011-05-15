/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-05
 *	@file mesh.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

/// Declaration
#include "mesh.hpp"

#include "base/exceptions.hpp"

/// ------------------------------- Global -----------------------------------
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
