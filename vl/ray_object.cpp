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

#include "recording.hpp"

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
vl::RayObject::setRecording(RecordingRefPtr rec)
{
	if(rec != _recording)
	{
		setDirty(DIRTY_RECORDING);
		_recording = rec;
		if(_recorded_rays_show)
		{ update(); }
	}
}

void
vl::RayObject::setDirection(Ogre::Vector3 const &dir)
{
	if(_direction != dir)
	{
		setDirty(DIRTY_DIRECTION);
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
		if(!_collision_detection)
		{
			setDrawCollisionSphere(false);
			setDrawRay(true);
		}
	}
}

void
vl::RayObject::setDrawCollisionSphere(bool enable)
{
	if(_draw_collision_sphere != enable)
	{
		setDirty(DIRTY_PARAMS);
		_draw_collision_sphere = enable;
		if(_draw_collision_sphere)
		{
			setCollisionDetection(true);
		}
		else
		{
			setDrawRay(true);
		}
	}
}

void
vl::RayObject::setDrawRay(bool enable)
{
	if(_draw_ray != enable)
	{
		setDirty(DIRTY_PARAMS);
		_draw_ray = enable;
		if(!_draw_ray)
		{
			setDrawCollisionSphere(true);
		}
	}
}

void
vl::RayObject::update(void)
{
	setDirty(DIRTY_UPDATE);
	++_update_version;
}

void
vl::RayObject::showRecordedRays(bool show)
{
	if(show != _recorded_rays_show && _recording)
	{
		setDirty(DIRTY_SHOW_RECORDER);
		_recorded_rays_show = show;
		if(_recorded_rays_show)
		{ update(); }
	}
}

void
vl::RayObject::_updateRay(void)
{
	// Nop if no collision detection
	if(!_collision_detection)
	{ return; }

	// Nop for recording if user has not requested recalculation
	// because this is really slow for the number of rays recordings contain
	if(_recording && _recorded_rays_show && !_needs_updating)
	{ return; }

	/// Gather all rays that are to be drawn
	/// if recording is not shown this is a single ray
	// pair of start and end positions
	std::vector<std::pair<Ogre::Vector3, Ogre::Vector3> > ray_positions;
	// pair of vectors, start_position and direction for collision detection
	std::vector<std::pair<Ogre::Vector3, Ogre::Vector3> > collision_det_array;
	// Store the position and direction for collision detection
	if(_recording && _recorded_rays_show)
	{
		// Create a list of start and end positions
		for(std::map<vl::time, Transform>::iterator iter = _recording->sensors.at(0).transforms.begin();
			iter != _recording->sensors.at(0).transforms.end(); ++iter)
		{
			Ogre::Quaternion q = iter->second.quaternion;
			// @todo should the direction be -Z or should it be configurable using the direction parameter?
			Ogre::Vector3 direction = q*_direction;
			Ogre::Vector3 start_position = iter->second.position;

			// No collision detection because this will only initialise the list

			collision_det_array.push_back(std::make_pair(start_position, direction));
		}
	}
	else
	{
		collision_det_array.push_back(std::make_pair(_position, _direction));
	}
	
	// Run collision detection store start and end positions
	// @todo move the collision detection to a separate function
	std::vector<std::pair<Ogre::Vector3, Ogre::Vector3> >::const_iterator iter;
	for( iter = collision_det_array.begin();
		iter != collision_det_array.end(); ++iter)
	{
		Ogre::Vector3 const &start_position = iter->first;
		Ogre::Vector3 const &direction = iter->second;
		
		// Parent transformation
		Ogre::Vector3 const &translate = _ogre_object->getParentNode()->_getDerivedPosition();
		Ogre::Vector3 const &scale = _ogre_object->getParentNode()->_getDerivedScale();
		Ogre::Quaternion const &q = _ogre_object->getParentNode()->_getDerivedOrientation();
		
		// Results
		Ogre::Vector3 result;
		Ogre::Vector3 ray_end = start_position + (direction*_length);
		if(_ray_cast->raycastFromPoint(translate+q*start_position, q*direction, result))
		{
			// Remove parents transformation as the collision detection 
			// is done in the World space	
			ray_end = q.Inverse() * (result - translate);
		}

		ray_positions.push_back(std::make_pair(start_position, ray_end));
	}

	uint16_t sub_obj_index = 0;
	if(_draw_ray)
	{
		_ogre_object->beginUpdate(sub_obj_index);
		for(iter = ray_positions.begin(); iter != ray_positions.end(); ++iter)
		{
			_generateLine(iter->first, iter->second);
		}
		_ogre_object->end();
		++sub_obj_index;
	}

	if(_draw_collision_sphere)
	{
		_ogre_object->beginUpdate(sub_obj_index);
		uint32_t index = 0;
		for(iter = ray_positions.begin(); iter != ray_positions.end(); ++iter)
		{
			index = _generateCollisionSphere(iter->second, index);
		}
		_ogre_object->end();
		++sub_obj_index;
	}

	// @todo should update the bounding box because if the hit state
	// or the hitted object changed from last one
	// the bounding volume will be different than before
	// _ogre_object->setBoundingBox(Box)

	// Clear the update flag
	_needs_updating = false;
}

/// ----------------------------- Private ------------------------------------
bool
vl::RayObject::_doCreateNative(void)
{
	assert(_creator);

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
	if(dirtyBits & DIRTY_DIRECTION)
	{
		msg << _direction;
	}

	if(dirtyBits & DIRTY_PARAMS)
	{
		msg << _material << _length << _sphere_radius << _draw_collision_sphere 
			<< _draw_ray << _collision_detection;
	}

	if(dirtyBits & DIRTY_SHOW_RECORDER)
	{
		msg << _recorded_rays_show;
	}

	// This is really really slow if the program is run for multiple minutes
	// we should use a divide system or an update system
	// either divide the array to multiples or send only updates to the array
	// we could also do a slow update where the array is only updated once every
	// second or so.
	if(dirtyBits & DIRTY_RECORDING)
	{
		if(!_recording)
		{ msg << false; }
		else
		{ msg << true << *_recording; }
	}

	if(dirtyBits & DIRTY_UPDATE)
	{ msg << _update_version; }
}

void
vl::RayObject::doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	bool dirty = false;
	if(dirtyBits & DIRTY_DIRECTION)
	{
		msg >> _direction;
		dirty = true;
	}

	if(dirtyBits & DIRTY_PARAMS)
	{
		msg >> _material >> _length >> _sphere_radius >> _draw_collision_sphere 
			>> _draw_ray >> _collision_detection;
		dirty = true;
	}

	if(dirtyBits & DIRTY_SHOW_RECORDER)
	{
		msg >> _recorded_rays_show;
		
		if(_recording)
		{ dirty = true; }
	}

	if(dirtyBits & DIRTY_RECORDING)
	{
		bool valid;
		msg >> valid;
		
		if(valid)
		{
			if(!_recording)
			{ _recording.reset(new Recording); }
			msg >> *_recording;

			if(_recorded_rays_show)
			{ dirty = true; }
		}
	}

	if(dirtyBits & DIRTY_UPDATE)
	{
		uint32_t version;
		msg >> version;
		if(version > _update_version || version == 0)
		{
			_update_version = version;
			_needs_updating = true;
		}
		else
		{
			std::clog << "Something really odd with the version we should update to : "
				<< "current version " << _update_version << " new version : " << version
				<< std::endl;
		}
	}

	if(dirty && _ogre_object)
	{ _create(); }
}

void
vl::RayObject::_clear(void)
{
	_direction = -Ogre::Vector3::UNIT_Z;
	_length = 1000;
	_sphere_radius = 1;
	_draw_collision_sphere = false;
	_collision_detection = false;
	_draw_ray = true;
	_update_version = 0;
	_needs_updating = false;
	_recorded_rays_show = false;
	_ogre_object = 0;
	_listener = 0;
	_ray_cast = 0;
}

// Collision detection does not work here always
// if created before any other objects it will not work.
void
vl::RayObject::_create(void)
{
	std::clog << "vl::RayObject::_create" << std::endl;

	// Specification does not allow empty objects
	assert((_draw_collision_sphere && _collision_detection) || _draw_ray);

	// Some general definitions for collision detection
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

	if(_recorded_rays_show && _recording)
	{ _createRecordedRays(); }
	else
	{ _createDynamic(); }

	_ogre_object->setCastShadows(false);
}

void
vl::RayObject::_createDynamic(void)
{
	// @todo add needs to calculate the number of vertices and indexes
	// from the line and if we are using a sphere
	// _ogre_object->estimateVertexCount((mNumRings+1)*(mNumSegments+1));
	// _ogre_object->estimateIndexCount(mNumRings*(mNumSegments+1)*6);

	assert(_ogre_object);
	_ogre_object->clear();
	Ogre::Vector3 end_position = _position + (_direction*_length); 
	// @todo this should use the ray casting already
	if(_draw_ray)
	{
		_ogre_object->begin(_material, Ogre::RenderOperation::OT_LINE_LIST);
		_generateLine(_position, end_position);
		_ogre_object->end();
	}

	// Draw the collision sphere
	if(_draw_collision_sphere)
	{
		_ogre_object->begin(_material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		_generateCollisionSphere(end_position);
		_ogre_object->end();
	}
}

void
vl::RayObject::_createRecordedRays(void)
{
	if(!_recording)
	{ BOOST_THROW_EXCEPTION(vl::null_pointer() << vl::desc("Something wrong with the recording.")); }

	// Empty recording
	if(_recording->sensors.size() == 0)
	{ return; }

	std::clog << "vl::RayObject::_createRecordedRays : creating " 
		<< _recording->sensors.at(0).transforms.size() << " recorded rays." << std::endl;

	assert(_ogre_object);
	_ogre_object->clear();

	// Store the positions to an array for creating the segments
	// we want the minimal number of separate segments.
	// pair of start and end positions
	std::vector<std::pair<Ogre::Vector3, Ogre::Vector3> > ray_positions;

	// Create a list of start and end positions
	for(std::map<vl::time, Transform>::iterator iter = _recording->sensors.at(0).transforms.begin();
		iter != _recording->sensors.at(0).transforms.end(); ++iter)
	{
		// @todo should the direction be -Z or should it be configurable using the direction parameter?
		Ogre::Vector3 dir = iter->second.quaternion*_direction;
		Ogre::Vector3 start_position = iter->second.position;

		Ogre::Vector3 end_position = start_position + (dir*_length);

		// collision detection does not work if some objects are created
		// after this object so we run separate update phase every frame.

		ray_positions.push_back(std::make_pair(start_position, end_position));
	}

	/// Draw all the lines
	if(_draw_ray)
	{
		_ogre_object->begin(_material, Ogre::RenderOperation::OT_LINE_LIST);
		for(std::vector<std::pair<Ogre::Vector3, Ogre::Vector3> >::const_iterator iter = ray_positions.begin();
				iter != ray_positions.end(); ++iter)
		{
			_generateLine(iter->first, iter->second);
		}
		_ogre_object->end();
	}

	// Draw the collision spheres
	if(_draw_collision_sphere)
	{
		uint32_t index = 0;
		_ogre_object->begin(_material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		for(std::vector<std::pair<Ogre::Vector3, Ogre::Vector3> >::const_iterator iter = ray_positions.begin();
			iter != ray_positions.end(); ++iter)
		{
			index = _generateCollisionSphere(iter->second, index);
		}
		_ogre_object->end();
	}
}

void
vl::RayObject::_generateLine(Ogre::Vector3 const &start_point, Ogre::Vector3 const &end_point)
{
	assert(_ogre_object);

	// Does not need indexes
	_ogre_object->position(start_point);
	_ogre_object->position(end_point);
}

uint32_t
vl::RayObject::_generateCollisionSphere(Ogre::Vector3 const &point, uint32_t start_index)
{
	assert(_ogre_object);

	// Generate the sphere mesh
	// using a 512 tri mesh for the sphere
	uint16_t numRings = 16;
	uint16_t numSegments = 16;
	int offset = start_index;
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

	return offset;
}

void
vl::RayObject::RayListener::frameStart(void)
{
	assert(dynamic_cast<RayObjectPtr>(object));
	static_cast<RayObjectPtr>(object)->_updateRay();
}
