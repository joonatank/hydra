/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file ogre_python.cpp
 */

#include <boost/python.hpp>

#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

#include "mesh_writer.hpp"

namespace py
{
    using namespace boost::python;
}

BOOST_PYTHON_MODULE(pyogre)
{
    using namespace boost::python;

	class_<Ogre::Vector2>( "Vector2", init<>() )
		.def(init<Ogre::Real, Ogre::Real>())
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
		.def(init< Ogre::Real, Ogre::Real, Ogre::Real>() )
		.def_readwrite("x", &Ogre::Vector3::x)
		.def_readwrite("y", &Ogre::Vector3::y)
		.def_readwrite("z", &Ogre::Vector3::z)
		.def("length", &Ogre::Vector3::length)
		.def("normalise", &Ogre::Vector3::normalise)
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

	py::class_<Ogre::ColourValue>("ColourValue", py::init<>())
		.def(py::init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.def(py::init<Ogre::Real, Ogre::Real, Ogre::Real>())
		.def_readwrite("r", &Ogre::ColourValue::r)
		.def_readwrite("g", &Ogre::ColourValue::g)
		.def_readwrite("b", &Ogre::ColourValue::b)
		.def_readwrite("a", &Ogre::ColourValue::a)
		// Operators
		.def(py::self + py::self)
		.def(py::self - py::self)
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
		.def_readwrite("x", &Ogre::Quaternion::x)
		.def_readwrite("y", &Ogre::Quaternion::y)
		.def_readwrite("z", &Ogre::Quaternion::z)
		.def_readwrite("w", &Ogre::Quaternion::w)
		.def("Norm", &Ogre::Quaternion::Norm)
		.def("normalise", &Ogre::Quaternion::normalise)
		.def("equals", &Ogre::Quaternion::equals)
		.def("isNaN", &Ogre::Quaternion::isNaN)
		// Operators
		.def(py::self + py::self)
		.def(py::self - py::self)
		.def(py::self * py::self)
		.def(py::self * Ogre::Vector3())
		.def(py::self * Ogre::Real())
		.def(py::self == py::self)
		.def(py::self != py::self)
		.def(py::self_ns::str(py::self_ns::self))
	;

	py::class_<Ogre::Degree>("Degree", py::init< py::optional<Ogre::Real> >() )
		.def(py::init<Ogre::Radian>())
		.def("valueDegrees", &Ogre::Degree::valueDegrees)
		.def("valueRadians", &Ogre::Degree::valueRadians)
		.def("valueAngleUnits", &Ogre::Degree::valueAngleUnits)
		// Operators
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
		.def("valueDegrees", &Ogre::Radian::valueDegrees)
		.def("valueRadians", &Ogre::Radian::valueRadians)
		.def("valueAngleUnits", &Ogre::Radian::valueAngleUnits)
		// Operators
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


	Ogre::Vector3 const &(Ogre::AxisAlignedBox::*box_min2)() const = &Ogre::AxisAlignedBox::getMinimum;
	Ogre::Vector3 const &(Ogre::AxisAlignedBox::*box_max2)() const = &Ogre::AxisAlignedBox::getMaximum;
	void (Ogre::AxisAlignedBox::*box_set_max)(Ogre::Vector3 const &) = &Ogre::AxisAlignedBox::setMaximum;
	void (Ogre::AxisAlignedBox::*box_set_min)(Ogre::Vector3 const &) = &Ogre::AxisAlignedBox::setMinimum;
	
	class_<Ogre::AxisAlignedBox>("AxisAlignedBox", init<>())
		.def(init<Ogre::Vector3, Ogre::Vector3>())
		.def(init<Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real, Ogre::Real>())
		.add_property("minimum", make_function(box_min2, return_value_policy<copy_const_reference>()), box_set_min )
		.add_property("maximum", make_function(box_max2, return_value_policy<copy_const_reference>()), box_set_max )
	;

	class_<vl::Vertex>("Vertex")
		.def_readwrite("position", &vl::Vertex::position)
		.def_readwrite("normal", &vl::Vertex::normal)
		.def_readwrite("diffuse", &vl::Vertex::diffuse)
		.def_readwrite("specular", &vl::Vertex::specular)
		.def_readwrite("uv", &vl::Vertex::uv)
		.def(self_ns::str(self_ns::self))
	;

	class_< vl::Mesh, boost::noncopyable>("Mesh", no_init )
		.def("createSubMesh", make_function( &vl::Mesh::createSubMesh, return_value_policy<reference_existing_object>() ) )
		.def("getNumSubMeshes", &vl::Mesh::getNumSubMeshes)
		.def("getSubMesh", make_function( &vl::Mesh::getSubMesh, return_value_policy<reference_existing_object>() ) )
		.def("addVertex", &vl::Mesh::addVertex)
		.def("getSubMesh", make_function(&vl::Mesh::getSubMesh, return_value_policy<reference_existing_object>() ) )
		.add_property("bounding_sphere", &vl::Mesh::getBoundingSphereRadius, &vl::Mesh::setBoundingSphereRadius)
		.add_property("bounds", make_function( &vl::Mesh::getBounds, return_value_policy<copy_const_reference>() ), &vl::Mesh::setBounds)
		/*
		.def("setSkeletonName", &vl::Mesh::setSkeletonName)
		.def("hasSkeleton", &vl::Mesh::hasSkeleton)
		.def("getSkeletonName", make_function( &vl::Mesh::getSkeletonName, return_value_policy<copy_const_reference>() ) )
		.def("addBoneAssignment", &vl::Mesh::addBoneAssignment)
		.def("clearBoneAssignments", &vl::Mesh::clearBoneAssignments)
		*/
		.def(self_ns::str(self_ns::self))
	;

	class_<vl::SubMesh, boost::noncopyable>("SubMesh", no_init )
		.add_property("material", make_function( &vl::SubMesh::getMaterial, return_value_policy<copy_const_reference>() ), &vl::SubMesh::setMaterial )
		.add_property("name", make_function( &vl::SubMesh::getName, return_value_policy<copy_const_reference>() ), &vl::SubMesh::setName )
		.def("addFace", &vl::SubMesh::addFace)
		.def("getNumFaces", &vl::SubMesh::getNumFaces)
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

	class_<vl::MeshWriter, boost::noncopyable>("MeshWriter")
		.def("createMesh", make_function( &vl::MeshWriter::createMesh, return_value_policy<reference_existing_object>() ) )
		.def("writeMesh", &vl::MeshWriter::writeMesh)
	;

}

