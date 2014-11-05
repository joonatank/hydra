/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-05
 *	@file mesh.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

/// Declaration
#include "mesh.hpp"

#include "base/exceptions.hpp"
/// Necessary for timing the serialisation
#include "base/chrono.hpp"

/// ------------------------------- Global -----------------------------------
void 
vl::calculate_bounds(vl::VertexData const *vertexData, Ogre::AxisAlignedBox &box, Ogre::Real &sphere)
{
	Ogre::Vector3 min, max;
	Ogre::Real maxSquaredRadius;

	// @todo const casting is bad we need to add const iterator
	vl::VertexData *data = const_cast<vl::VertexData *>(vertexData);
	PositionIterator iter = data->getPositionIterator();
	assert(iter != PositionIterator() && !iter.end());
	min = max = *iter;
	maxSquaredRadius = iter->squaredLength();
	while(!iter.end())
	{
		min.makeFloor(*iter);
		max.makeCeil(*iter);
		maxSquaredRadius = std::max(iter->squaredLength(), maxSquaredRadius);
		++iter;
	}
	
	box.setExtents(min, max);
	sphere = Ogre::Math::Sqrt(maxSquaredRadius);
}

std::ostream &
vl::operator<<( std::ostream &os, vl::Mesh const &m )
{
	os << "Mesh " << m.getName();
	if(m.sharedVertexData)
	{
		os << " : has " << m.sharedVertexData->getVertexCount() << " vertices";
	}
	else
	{
		os << " : has no vertices." << std::endl;
	}

	if(m.getNumSubMeshes() > 0)
	{
		os << " : has " << m.getNumSubMeshes() << " submeshes";
	}
	else
	{
		os << " : has no submeshes";
	}

	return os;
}

std::ostream &
vl::operator<<( std::ostream &os, vl::SubMesh const &m )
{
	os << "Submesh with material " << m.getMaterial();

	return os;
}

/// ----------------------------- SubMesh ------------------------------------

void
vl::SubMesh::allocateFaces(size_t n_faces)
{
	if(operationType == Ogre::RenderOperation::OT_LINE_LIST)
	{ indexData.setIndexCount(n_faces*2); }
	else if(operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST)
	{ indexData.setIndexCount(n_faces*3); }
	// Other types use only three vertices for the first triangle
	// rest is defined by a single vertex
	else
	{ indexData.setIndexCount(n_faces+2); }
}

size_t
vl::SubMesh::getNumFaces(void) const
{
	if(operationType == Ogre::RenderOperation::OT_LINE_LIST)
	{ return indexData.indexCount()/2; }
	else if(operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST)
	{ return indexData.indexCount()/3; }
	// Other types use only three vertices for the first triangle
	// rest is defined by a single vertex
	else
	{ return indexData.indexCount()-2; }
}

void
vl::SubMesh::setFace(size_t face, uint32_t  i1, uint32_t  i2, uint32_t i3)
{
	// @todo replace with a real exception
	if(getNumFaces()-1 > face)
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	if(operationType == Ogre::RenderOperation::OT_LINE_LIST)
	{ 
		indexData.set(face*2+1, i1);
		indexData.set(face*2+2, i2);
	}
	else if(operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST)
	{ 
		indexData.set(face*3, i1);
		indexData.set(face*3+1, i2);
		indexData.set(face*3+2, i3);
	}
	// Other types use only three vertices for the first triangle
	// rest is defined by a single vertex
	else
	{
		if(face == 0)
		{
			indexData.set(face, i1);
			indexData.set(face+1, i2);
			indexData.set(face+2, i3);
		}
		else
		{
			indexData.set(face+2, i3);
		}
	}
}

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

/// --------------------------VertexDeclaration ------------------------------
size_t
vl::VertexDeclaration::getVertexBindingSize(size_t bind) const
{
	size_t size = 0;
		
	for(size_t i = 0; i < _elements.size(); ++i)
	{
		if(_elements.at(i).getSource() == bind)
		{ size += _elements.at(i).getSize(); }
	}

	return size;
}

size_t 
vl::VertexDeclaration::getVertexSize(void) const
{
	size_t size = 0;
		
	for(size_t i = 0; i < _elements.size(); ++i)
	{
		size += _elements.at(i).getSize();
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

/// ---------------------------- VertexData ----------------------------------
vl::VertexBufferRefPtr
vl::VertexData::getPositionBuffer(void)
{
	if(vertexDeclaration.hasSemantic(Ogre::VES_POSITION))
	{
		Ogre::VertexElement elem = vertexDeclaration.getVertexElement(Ogre::VES_POSITION);

		VertexBufferRefPtr buffer = _bindings.at(elem.getSource());
		return buffer;
	}

	return vl::VertexBufferRefPtr();
}

vl::PositionIterator
vl::VertexData::getPositionIterator(void)
{
	if(vertexDeclaration.hasSemantic(Ogre::VES_POSITION))
	{
		Ogre::VertexElement elem = vertexDeclaration.getVertexElement(Ogre::VES_POSITION);

		VertexBufferRefPtr buffer = _bindings.at(elem.getSource());
		return PositionIterator(buffer.get(), elem.getOffset());
	}
	else
	{
		std::clog << "VertexData has no Position semantics" << std::endl;
		throw;
	}
}

vl::NormalIterator
vl::VertexData::getNormalIterator(void)
{
	if(vertexDeclaration.hasSemantic(Ogre::VES_NORMAL))
	{
		Ogre::VertexElement elem = vertexDeclaration.getVertexElement(Ogre::VES_NORMAL);

		VertexBufferRefPtr buffer = _bindings.at(elem.getSource());
		return NormalIterator(buffer.get(), elem.getOffset());
	}
	else
	{
		std::clog << "VertexData has no Normal semantics" << std::endl;
		throw;
	}		
}


/// ---------------------------- IndexBuffer ---------------------------------
void
vl::IndexBuffer::set(size_t i, uint32_t index)
{
	// No checking for the index provided, vector::at will do that for us
	if(i >= indexCount())
	{ BOOST_THROW_EXCEPTION(vl::exception()); }

	_buffer.at(i) = index;
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
	if(sharedVertexData && !sharedVertexData->empty() && sharedVertexData->getVertexCount() > 0)
	{ calculate_bounds(sharedVertexData, _bounds, _bound_radius); }

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
	msg << decl.getElements();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::VertexDeclaration &decl)
{
	msg >> decl.getElements();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, Ogre::VertexElement const &elem)
{
	msg << elem.getSource() << elem.getOffset() << elem.getType() << elem.getSemantic()
		<< elem.getIndex();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, Ogre::VertexElement &elem)
{
	unsigned short source;
	size_t offset;
	Ogre::VertexElementType type;
	Ogre::VertexElementSemantic semantic;
	unsigned short index;

	msg >> source >> offset >> type >> semantic >> index;

	elem = Ogre::VertexElement(source, offset, type, semantic, index);

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::VertexBuffer const &vbuf)
{
	msg << vbuf.getNVertices() << vbuf.getVertexSize();
	msg.write(vbuf._buffer, vbuf.size());

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::VertexBuffer &vbuf)
{
	size_t vertex_size, vertices;
	msg >> vertices >> vertex_size;
	
	vbuf._reset(vertex_size, vertices);
	msg.read(vbuf._buffer, vbuf.size());

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::VertexData const &vbuf)
{
	msg << vbuf.vertexDeclaration;

	/// We need to write this open because one of the stored values is a pointer
	/// yes we could write partial template specializations for them
	/// but for now lets leave it.
	msg << vbuf._bindings.size();
	for(std::map<size_t, VertexBufferRefPtr>::const_iterator iter = vbuf._bindings.begin();
		iter != vbuf._bindings.end(); ++iter)
	{
		msg << iter->first << *iter->second;
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::VertexData &vbuf)
{
	msg >> vbuf.vertexDeclaration;
	
	size_t bindings_size;
	msg >> bindings_size;
	for(size_t i = 0; i < bindings_size; ++i)
	{
		size_t bind;
		VertexBufferRefPtr buffer = VertexBuffer::create(0, 0);
		msg >> bind >> *buffer;
		vbuf.setBinding(bind, buffer);
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::IndexBuffer const &ibf)
{
	msg << ibf.getVec();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::IndexBuffer &ibf)
{
	msg >> ibf.getVec();

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::SubMesh const &sm)
{
	msg << sm.getName() << sm.getMaterial() << sm.operationType << sm.indexData << sm.useSharedGeometry;

	if(sm.vertexData)
	{ msg << *sm.vertexData; }

	// Check that we have some kind of geometry either shared or local
	assert(sm.useSharedGeometry || sm.vertexData);

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
	{
		delete sm.vertexData;
		sm.vertexData = 0;
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::Mesh const &mesh)
{
	std::clog << "Serializing mesh : " << mesh.getName() << std::endl;
	vl::chrono clock;

	if(mesh.sharedVertexData)
		msg << true << *mesh.sharedVertexData;
	else
		msg << false;

	msg << mesh.getBoundingSphereRadius() << mesh.getBounds();
	
	msg << mesh.getSubMeshes().size();
	for(size_t i = 0; i < mesh.getSubMeshes().size(); ++i)
	{ msg << *mesh.getSubMeshes().at(i); }

	// Serialisation doesn't take that long (deserialisation is the problem)
	std::clog << "Mesh took : " << clock.elapsed() << std::endl;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::Mesh &mesh)
{
	std::clog << "Deserializing mesh : " << mesh.getName() << std::endl;
	vl::chrono clock;

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

	std::clog << "Mesh took : " << clock.elapsed() << std::endl;

	return msg;
}
