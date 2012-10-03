/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file ogre_python.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
 *	@todo division operators do not work with Python 3.1, need to upgrade boost::python
 */

#include <boost/python.hpp>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "mesh_serializer.hpp"

// Necessary for exposing vectors
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace py
{
    using namespace boost::python;
}

namespace math
{
	inline
	Ogre::Matrix4 buildReflectionMatrix(Ogre::Plane const &p)
	{ return Ogre::Math::buildReflectionMatrix(p); }
}

BOOST_PYTHON_MODULE(pyogre)
{
    using namespace boost::python;

	class_<Ogre::Vector2>( "Vector2", init<>() )
		.def(py::init<Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::Vector2>())
		.def_readwrite("x", &Ogre::Vector2::x)
		.def_readwrite("y", &Ogre::Vector2::y)
		.def("length", &Ogre::Vector2::length)
		.def("normalise", &Ogre::Vector2::normalise)
		// Operators
		.def(-self )
		.def(self + self )
		.def(self - self )
		.def(self * self )
		.def(self * Ogre::Real() )
		.def(self / self )
		.def(self / Ogre::Real() )
		.def(self += self )
		.def(self += Ogre::Real() )
		.def(self -= self )
		.def(self -= Ogre::Real() )
		.def(self *= self )
		.def(self *= Ogre::Real() )
		.def(self /= self )
		.def(self /= Ogre::Real() )
		// Comparison
		.def(self == self )
		.def(self != self )
		.def(self_ns::str(self_ns::self))
	;

	class_<Ogre::Vector3>("Vector3", init<>())
		.def(py::init< Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::Real>())
		.def(py::init<Ogre::Vector3>())
		.def_readwrite("x", &Ogre::Vector3::x)
		.def_readwrite("y", &Ogre::Vector3::y)
		.def_readwrite("z", &Ogre::Vector3::z)
		.def("length", &Ogre::Vector3::length)
		.def("normalise", &Ogre::Vector3::normalise)
		.def("dot", &Ogre::Vector3::dotProduct)
		.def("abs_dot", &Ogre::Vector3::absDotProduct)
		.def("angle_between", &Ogre::Vector3::angleBetween)
		.def("cross", &Ogre::Vector3::crossProduct)
		.def("distance", &Ogre::Vector3::distance)
		.def("rotation_to", &Ogre::Vector3::getRotationTo)
		// Operators
		.def(-self )
		.def(self + self )
		.def(self - self )
		.def(self * self )
		.def(Ogre::Real() * self)
		.def(self * Ogre::Real())
		.def(self / self )
		.def(self / Ogre::Real() )
		.def(self += self )
		.def(self += Ogre::Real() )
		.def(self -= self )
		.def(self -= Ogre::Real() )
		.def(self *= self )
		.def(self *= Ogre::Real() )
		.def(self /= self )
		.def(self /= Ogre::Real() )
		// Comparison
		.def(self == self )
		.def(self != self )
		.def(self_ns::str(self_ns::self))
		.add_static_property("unit_x", py::make_getter(&Ogre::Vector3::UNIT_X, py::return_value_policy<py::return_by_value>()) )
		.add_static_property("unit_y", py::make_getter(&Ogre::Vector3::UNIT_Y, py::return_value_policy<py::return_by_value>()) )
		.add_static_property("unit_z", py::make_getter(&Ogre::Vector3::UNIT_Z, py::return_value_policy<py::return_by_value>()) )
		.add_static_property("zero", py::make_getter(&Ogre::Vector3::ZERO, py::return_value_policy<py::return_by_value>()) )
	;

	py::class_<Ogre::ColourValue>("ColourValue", py::init<>())
		.def(py::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::ColourValue>())
		.def_readwrite("r", &Ogre::ColourValue::r)
		.def_readwrite("g", &Ogre::ColourValue::g)
		.def_readwrite("b", &Ogre::ColourValue::b)
		.def_readwrite("a", &Ogre::ColourValue::a)
		// Operators
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(Ogre::Real() * py::self)
		.def(py::self * Ogre::Real())
		.def(py::self / Ogre::Real())
		.def(py::self += py::self)
		.def(py::self -= py::self)
		.def(py::self *= Ogre::Real())
		.def(py::self /= Ogre::Real())
		// Comparison
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(py::self_ns::str(py::self_ns::self))
	;

	py::class_<Ogre::Quaternion>("Quaternion", py::init<>())
		.def(py::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::Radian, Ogre::Vector3>())
		.def(py::init<Ogre::Quaternion>())
		.def_readwrite("x", &Ogre::Quaternion::x)
		.def_readwrite("y", &Ogre::Quaternion::y)
		.def_readwrite("z", &Ogre::Quaternion::z)
		.def_readwrite("w", &Ogre::Quaternion::w)
		.def("Norm", &Ogre::Quaternion::Norm)
		.def("normalise", &Ogre::Quaternion::normalise)
		.def("equals", &Ogre::Quaternion::equals)
		.def("isNaN", &Ogre::Quaternion::isNaN)
		.def("inverse", &Ogre::Quaternion::Inverse)
		.def("unit_inverse", &Ogre::Quaternion::UnitInverse)
		// Operators
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self * Ogre::Vector3())
		.def(Ogre::Real()*py::self)
		.def(py::self * Ogre::Real())
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(py::self_ns::str(py::self_ns::self))
		.add_static_property("identity", py::make_getter(&Ogre::Quaternion::IDENTITY, py::return_value_policy<py::return_by_value>()) )
	;

	py::class_<Ogre::Matrix4>("Matrix", py::init<>())
		.def(py::init<Ogre::Quaternion>())
		.def(py::init<Ogre::Matrix4>())
		.def("transpose", &Ogre::Matrix4::transpose)
		.add_property("has_scale", &Ogre::Matrix4::hasScale)
		.add_property("has_negative_scale", &Ogre::Matrix4::hasNegativeScale)
		.add_property("to_quaternion", &Ogre::Matrix4::extractQuaternion)
		// Operators
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self * Ogre::Plane())
		.def(py::self * Ogre::Vector3())
		.def(py::self * Ogre::Real())
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(py::self_ns::str(py::self_ns::self))
		.add_static_property("identity", py::make_getter(&Ogre::Matrix4::IDENTITY, py::return_value_policy<py::return_by_value>()) )
	;

	py::class_<Ogre::Degree>("Degree", py::init< py::optional<Ogre::Real> >() )
		.def(py::init<Ogre::Radian>())
		.def(py::init<Ogre::Degree>())
		.def("valueDegrees", &Ogre::Degree::valueDegrees)
		.def("valueRadians", &Ogre::Degree::valueRadians)
		.def("valueAngleUnits", &Ogre::Degree::valueAngleUnits)
		// Operators
		.def(-py::self)
		.def(py::self + py::self )
		.def(py::self + Ogre::Radian() )
		.def(py::self += py::self )
		.def(py::self + Ogre::Radian() )
		.def(py::self - py::self )
		.def(py::self - Ogre::Radian() )
		.def(py::self -= py::self )
		.def(py::self -= Ogre::Radian() )
		.def(py::self * py::self)
		.def(py::self * Ogre::Real())
		.def(py::self *= Ogre::Real())
		.def(py::self / Ogre::Real())
		.def(py::self /= Ogre::Real())
		// Comparison operators
		.def(py::self < py::self )
		.def(py::self <= py::self )
		.def(py::self == py::self )
		.def(py::self != py::self )
		.def(py::self > py::self )
		.def(py::self >= py::self )
		.def(py::self_ns::str(py::self_ns::self))
	;

	py::class_<Ogre::Radian>("Radian", py::init< py::optional<Ogre::Real> >())
		.def(py::init<Ogre::Degree>())
		.def(py::init<Ogre::Radian>())
		.def("valueDegrees", &Ogre::Radian::valueDegrees)
		.def("valueRadians", &Ogre::Radian::valueRadians)
		.def("valueAngleUnits", &Ogre::Radian::valueAngleUnits)
		// Operators
		.def(-py::self)
		.def(py::self + py::self )
		.def(py::self + Ogre::Degree() )
		.def(py::self += py::self )
		.def(py::self + Ogre::Degree() )
		.def(py::self - py::self )
		.def(py::self - Ogre::Degree() )
		.def(py::self -= py::self )
		.def(py::self -= Ogre::Degree() )
		.def(py::self * py::self)
		.def(py::self * Ogre::Real())
		.def(py::self *= Ogre::Real())
		.def(py::self / Ogre::Real())
		.def(py::self /= Ogre::Real())
		// Comparison operators
		.def(py::self < py::self )
		.def(py::self <= py::self )
		.def(py::self == py::self )
		.def(py::self != py::self )
		.def(py::self > py::self )
		.def(py::self >= py::self )
		.def(py::self_ns::str(py::self_ns::self))
	;

	py::class_<Ogre::Plane>("Plane", py::init<>())
		.def(py::init<Ogre::Vector3, Ogre::Real>())
		.def(py::init<Ogre::Vector3, Ogre::Vector3, Ogre::Vector3>())
		.def(py::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::Plane>())
		.def("projectVector", &Ogre::Plane::projectVector)
		.def("getDistance", &Ogre::Plane::getDistance)
		.def("normalise", &Ogre::Plane::normalise)
		.def(py::self_ns::str(py::self_ns::self))
	;

	def("buildReflectionMatrix", math::buildReflectionMatrix);

	Ogre::Vector3 const &(Ogre::AxisAlignedBox::*box_min2)() const = &Ogre::AxisAlignedBox::getMinimum;
	Ogre::Vector3 const &(Ogre::AxisAlignedBox::*box_max2)() const = &Ogre::AxisAlignedBox::getMaximum;
	void (Ogre::AxisAlignedBox::*box_set_max)(Ogre::Vector3 const &) = &Ogre::AxisAlignedBox::setMaximum;
	void (Ogre::AxisAlignedBox::*box_set_min)(Ogre::Vector3 const &) = &Ogre::AxisAlignedBox::setMinimum;
	
	class_<Ogre::AxisAlignedBox>("AxisAlignedBox", init<>())
		.def(init<Ogre::Vector3, Ogre::Vector3>())
		.def(init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::AxisAlignedBox>())
		.add_property("minimum", make_function(box_min2, return_value_policy<copy_const_reference>()), box_set_min )
		.add_property("maximum", make_function(box_max2, return_value_policy<copy_const_reference>()), box_set_max )
	;

	/*
	class_<vl::Vertex>("Vertex")
		.def_readwrite("position", &vl::Vertex::position)
		.def_readwrite("normal", &vl::Vertex::normal)
		.def_readwrite("diffuse", &vl::Vertex::diffuse)
		.def_readwrite("specular", &vl::Vertex::specular)
		.def_readwrite("uv", &vl::Vertex::uv)
		.def(self_ns::str(self_ns::self))
	;
	*/

	enum_<Ogre::VertexElementSemantic>("VES")
		.value("position", Ogre::VES_POSITION)
		.value("blend_weights", Ogre::VES_BLEND_WEIGHTS)
		.value("blend_indices", Ogre::VES_BLEND_INDICES)
	    .value("normal", Ogre::VES_NORMAL)
		.value("diffuse", Ogre::VES_DIFFUSE)
		.value("specular", Ogre::VES_SPECULAR)
		.value("texture_coordinates", Ogre::VES_TEXTURE_COORDINATES)
		.value("binormal", Ogre::VES_BINORMAL)
		.value("tangent", Ogre::VES_TANGENT)
	;

	enum_<Ogre::VertexElementType>("VET")
		.value("float1", Ogre::VET_FLOAT1)
		.value("float2", Ogre::VET_FLOAT2)
		.value("float3", Ogre::VET_FLOAT3)
		.value("float4", Ogre::VET_FLOAT4)
		.value("colour", Ogre::VET_COLOUR)
		.value("short1", Ogre::VET_SHORT1)
		.value("short2", Ogre::VET_SHORT2)
		.value("short3", Ogre::VET_SHORT3)
		.value("short4", Ogre::VET_SHORT4)
		.value("ubyte4", Ogre::VET_UBYTE4)
	;

	class_<vl::VertexDeclaration>("VertexDeclaration")
//		.def("addSemantic", &vl::VertexDeclaration::addSemantic)
//		.def("getNSemantics", &vl::VertexDeclaration::getNSemantics)
//		.def("getSemantic", &vl::VertexDeclaration::getSemantic)
	;

//	vl::Vertex &(vl::VertexData::*getVertex_ov0)(size_t) = &vl::VertexData::getVertex;

	class_<vl::VertexData>("VertexData")
//		.def("addVertex", &vl::VertexData::addVertex)
//		.def("getVertex", getVertex_ov0)
//		.add_property("n_vertices", &vl::VertexData::getNVertices)
		.def_readwrite("vertex_declaration", &vl::VertexData::vertexDeclaration)
	;

	class_<std::vector<vl::SubMesh *> >("SubMeshList")
		.def(vector_indexing_suite<std::vector<vl::SubMesh *> >())
	;

	vl::SubMesh *(vl::Mesh::*getSubMesh_ov0)(uint16_t) = &vl::Mesh::getSubMesh;
	Ogre::AxisAlignedBox &(vl::Mesh::*getBounds_ov0)() = &vl::Mesh::getBounds;
	Ogre::Real &(vl::Mesh::*getBoundingSphere_ov0)() = &vl::Mesh::getBoundingSphereRadius;
	vl::Mesh::SubMeshList const &(vl::Mesh::*getSubMeshes_ov0)() const = &vl::Mesh::getSubMeshes;

	/// @todo add list access to submeshes
	class_< vl::Mesh, vl::MeshRefPtr, boost::noncopyable>("Mesh", no_init )
		.def("createSubMesh", make_function( &vl::Mesh::createSubMesh, return_value_policy<reference_existing_object>() ) )
		.def("getNumSubMeshes", &vl::Mesh::getNumSubMeshes)
		.def("getSubMesh", make_function(getSubMesh_ov0, return_value_policy<reference_existing_object>()) )
		.add_property("sub_meshes", make_function(getSubMeshes_ov0, return_value_policy<copy_const_reference>()) )
		.add_property("bounding_sphere", make_function(getBoundingSphere_ov0, return_value_policy<copy_non_const_reference>()), &vl::Mesh::setBoundingSphereRadius)
		.add_property("bounds", make_function(getBounds_ov0, return_value_policy<copy_non_const_reference>()), &vl::Mesh::setBounds)
		.def("calculateBounds", &vl::Mesh::calculateBounds)
		.def("createVertexData", &vl::Mesh::createSharedVertexData)
		.def_readonly("sharedVertexData", &vl::Mesh::sharedVertexData)

		/*
		.def("setSkeletonName", &vl::Mesh::setSkeletonName)
		.def("hasSkeleton", &vl::Mesh::hasSkeleton)
		.def("getSkeletonName", make_function( &vl::Mesh::getSkeletonName, return_value_policy<copy_const_reference>() ) )
		.def("addBoneAssignment", &vl::Mesh::addBoneAssignment)
		.def("clearBoneAssignments", &vl::Mesh::clearBoneAssignments)
		*/
		.def(self_ns::str(self_ns::self))
	;

	enum_<vl::INDEX_SIZE>("IT")
		.value("16BIT", vl::IT_16BIT)
		.value("32BIT", vl::IT_32BIT)
	;

	void (vl::IndexBuffer::*push_back_ov0)(uint32_t) = &vl::IndexBuffer::push_back;
	
	class_<vl::IndexBuffer, boost::noncopyable>("IndexBuffer", no_init )
		.def("push_back", push_back_ov0)
		.add_property("index_size", &vl::IndexBuffer::getIndexSize, &vl::IndexBuffer::setIndexSize)
	;


	enum_<Ogre::RenderOperation::OperationType>("OT")
		.value("point_list", Ogre::RenderOperation::OT_POINT_LIST)
	    .value("line_list", Ogre::RenderOperation::OT_LINE_LIST)
		.value("line_stip", Ogre::RenderOperation::OT_LINE_STRIP)
		.value("triangle_list", Ogre::RenderOperation::OT_TRIANGLE_LIST)
		.value("triangle_strip", Ogre::RenderOperation::OT_TRIANGLE_STRIP)
        .value("triangle_fan", Ogre::RenderOperation::OT_TRIANGLE_FAN)
	;

	class_<vl::SubMesh, boost::noncopyable>("SubMesh", no_init )
		.add_property("material", make_function( &vl::SubMesh::getMaterial, return_value_policy<copy_const_reference>() ), &vl::SubMesh::setMaterial )
		.add_property("name", make_function( &vl::SubMesh::getName, return_value_policy<copy_const_reference>() ), &vl::SubMesh::setName )
		.def("allocateFaces", &vl::SubMesh::allocateFaces)
		.def_readonly("index_data", &vl::SubMesh::indexData)
		.def("setFace", &vl::SubMesh::setFace)
		.def_readwrite("operation_type", &vl::SubMesh::operationType)
		
		/*
		.def("addBoneAssignment", &Ogre::SubMesh::addBoneAssignment)
		.def("clearBoneAssignments", &Ogre::SubMesh::clearBoneAssignments)
		.def("addTextureAlias", &Ogre::SubMesh::addTextureAlias)
		.def("removeTextureAlias", &Ogre::SubMesh::removeTextureAlias)
		.def("removeAllTextureAliases", &Ogre::SubMesh::removeAllTextureAliases)
		.def("hasTextureAliases", &Ogre::SubMesh::hasTextureAliases)
		.def("getTextureAliasCount", &Ogre::SubMesh::getTextureAliasCount)
		*/
		.def(self_ns::str(self_ns::self))
	;

	class_<vl::MeshSerializer, boost::noncopyable>("MeshSerializer")
		.def("createMesh", &vl::MeshSerializer::createMesh)
		.def("writeMesh", &vl::MeshSerializer::writeMesh)
	;

}

