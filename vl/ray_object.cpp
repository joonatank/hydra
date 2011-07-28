/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-07
 *	@file: ray_object.cpp
 *
 *	This file is part of Hydra VR game engine.
 *
 *	description: Creates a ray object that can be attached to a SceneNode and
 *	drawn to screen.
 *
 */

// Interface
#include "ray_object.hpp"

#include "scene_manager.hpp"

#include "cluster/message.hpp"

#include <OGRE/OgreSceneManager.h>

/// ----------------------------- Public -------------------------------------
vl::RayObject::RayObject(std::string const &name, vl::SceneManagerPtr creator)
	: MovableObject(name, creator)
{ _clear(); }

vl::RayObject::RayObject(vl::SceneManagerPtr creator)
	: MovableObject(creator)
{ _clear(); }

vl::RayObject::~RayObject(void)
{
	delete getListener();
	// @todo destroy the manual object
	//myManualObjectNode->detachObject("manual1");
	_creator->getNative()->destroyManualObject(_ogre_object);
}

void
vl::RayObject::setPosition(Ogre::Vector3 const &pos)
{
	if(_position != pos)
	{
		setDirty(DIRTY_TRANSFORM);
		_position = pos;
	}
}

void
vl::RayObject::setDirection(Ogre::Vector3 const &dir)
{
	if(_direction != dir)
	{
		setDirty(DIRTY_TRANSFORM);
		_direction = dir;
	}
}

void
vl::RayObject::setMaterial(std::string const &name)
{
	if(_material != name)
	{
		setDirty(DIRTY_PARAMS);
		_material = name;
	}
}

void
vl::RayObject::setLength(vl::scalar l)
{
	if(_length != l)
	{
		setDirty(DIRTY_PARAMS);
		_length = l;
	}
}

void
vl::RayObject::setSphereRadius(vl::scalar radius)
{
	if(_sphere_radius != radius)
	{
		setDirty(DIRTY_PARAMS);
		_sphere_radius = radius;
	}
}

void
vl::RayObject::setCollisionDetection(bool enable)
{
	if(_collision_detection != enable)
	{
		setDirty(DIRTY_PARAMS);
		_collision_detection = enable;
	}
}

void
vl::RayObject::setDrawCollisionSphere(bool enable)
{
	if(_draw_collision_sphere != enable)
	{
		setDirty(DIRTY_PARAMS);
		_draw_collision_sphere = enable;
	}
}

void
vl::RayObject::_updateRay(void)
{
	if(_collision_detection && _dynamic)
	{
		// @todo get derived position from parent Node
		Ogre::Vector3 result;
		Ogre::Vector3 ray_end = _position + (_direction*_length);
		Ogre::Matrix4 t = _ogre_object->_getParentNodeFullTransform();
		Ogre::Vector3 translate;
		Ogre::Vector3 scale;
		Ogre::Quaternion q;
		t.decomposition(translate, scale, q);
		if(_ray_cast->raycastFromPoint(translate+q*_position, q*_direction, result))
		{
			// Remove parents transformation as the collision detection 
			// is done in the World space
			ray_end = t.inverse() * result;
		}

		_ogre_object->beginUpdate(0);
		_generateLine(_position, ray_end);
		_ogre_object->end();

		if(_draw_collision_sphere)
		{
			_ogre_object->beginUpdate(1);
			_generateCollisionSphere(ray_end);
			_ogre_object->end();
		}

		// @todo should update the bounding box because if the hit state
		// or the hitted object changed from last one
		// the bounding volume will be different than before
		// _ogre_object->setBoundingBox(Box)
	}
}

/// ----------------------------- Private ------------------------------------
bool
vl::RayObject::_doCreateNative(void)
{
	if(!_ogre_object)
	{
		_ogre_object =  _creator->getNative()->createManualObject(_name);
		_create();
	}

	return true;
}

void
vl::RayObject::doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
	if(dirtyBits & DIRTY_TRANSFORM)
	{
		msg << _position << _direction;
	}

	if(dirtyBits & DIRTY_PARAMS)
	{
		msg << _material << _length << _sphere_radius << _draw_collision_sphere 
			<< _collision_detection << _dynamic;
	}
}

void
vl::RayObject::doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	bool dirty = false;
	if(dirtyBits & DIRTY_TRANSFORM)
	{
		msg >> _position >> _direction;
		dirty = true;
	}

	if(dirtyBits & DIRTY_PARAMS)
	{
		msg >> _material >> _length >> _sphere_radius >> _draw_collision_sphere 
			>> _collision_detection >> _dynamic;
		dirty = true;
	}

	if(dirty && _ogre_object)
	{
		_create();
	}
}

void
vl::RayObject::_clear(void)
{
	_position = Ogre::Vector3::ZERO;
	_direction = -Ogre::Vector3::UNIT_Z;
	_length = 1000;
	_sphere_radius = 1;
	_draw_collision_sphere = false;
	_collision_detection = false;
	_dynamic = true;
	_ogre_object = 0;
	_listener = 0;
	_ray_cast = 0;
}

void
vl::RayObject::_create(void)
{	
	// @todo add needs to calculate the number of vertices and indexes
	// from the line and if we are using a sphere
	// _ogre_object->estimateVertexCount((mNumRings+1)*(mNumSegments+1));
	// _ogre_object->estimateIndexCount(mNumRings*(mNumSegments+1)*6);

	assert(_ogre_object);
	_ogre_object->clear();
	Ogre::Vector3 end_position = _position + (_direction*_length); 
	_ogre_object->begin(_material, Ogre::RenderOperation::OT_LINE_LIST);
	_generateLine(_position, end_position);
	_ogre_object->end();
	// Draw the collision sphere
	if(_draw_collision_sphere)
	{
		_ogre_object->begin(_material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		_generateCollisionSphere(end_position);
		_ogre_object->end();
	}

	if(_collision_detection)
	{
		if(!_ray_cast)
		{
			if(!_creator)
			{ BOOST_THROW_EXCEPTION(vl::null_pointer()); }

			_ray_cast = new RayCast(_creator);
		}

		if(!getListener())
		{
			Listener *listener = new RayListener(this);
			setListener(listener);
		}
	}

	_ogre_object->setCastShadows(false);
}

void
vl::RayObject::_generateLine(Ogre::Vector3 const &start_point, Ogre::Vector3 const &end_point)
{
	_ogre_object->position(start_point);
	_ogre_object->index(0);
	_ogre_object->position(end_point);
	_ogre_object->index(1);
}

void
vl::RayObject::_generateCollisionSphere(Ogre::Vector3 const &point)
{
	// Generate the sphere mesh
	// using a 512 tri mesh for the sphere
	uint16_t numRings = 16;
	uint16_t numSegments = 16;
	int offset = 0;
	// Note the less than equal for sphere generation
	for(uint16_t ring = 0; ring <= numRings; ++ring)
	{
		for(uint16_t seg = 0; seg <= numSegments; ++seg)
		{
			vl::scalar x = std::sin(M_PI * ring/numRings) * std::cos(2*M_PI * seg/numSegments);
			vl::scalar y = std::sin(M_PI * ring/numRings) * std::sin(2*M_PI * seg/numSegments);
			vl::scalar z = std::cos(M_PI * ring/numRings);
			Ogre::Vector3 pos = _sphere_radius*Ogre::Vector3(x, y, z);
			_ogre_object->position(point + pos);
			_ogre_object->normal(pos.normalisedCopy());
			// @todo test the UVs
			Ogre::Real u = (Ogre::Real)seg / (Ogre::Real)numSegments;
			Ogre::Real v = (Ogre::Real)ring / (Ogre::Real) numRings;
			_ogre_object->textureCoord(u, v);
			if(ring != numRings)
			{
				// each vertex (except the last) has six indices pointing to it
				// @todo move to using ManualObject::quad 
				// because it's a bit more easier to understand and reduces code
				_ogre_object->index(offset + numSegments + 1);
				_ogre_object->index(offset);
				_ogre_object->index(offset + numSegments);
				_ogre_object->index(offset + numSegments + 1);
				_ogre_object->index(offset + 1);
				_ogre_object->index(offset);
				++offset;
			}
		}
	}
}

void
vl::RayObject::RayListener::frameStart(void)
{
	assert(dynamic_cast<RayObjectPtr>(object));
	static_cast<RayObjectPtr>(object)->_updateRay();
}
