/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file: movable_text.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 *	description: Creates a ray object that can be attached to a SceneNode and
 *	drawn to screen.
 *
 */

#ifndef HYDRA_RAY_OBJECT_HPP
#define HYDRA_RAY_OBJECT_HPP

/// Base class
#include "movable_object.hpp"

#include "scene_manager.hpp"
#include <OGRE/OgreManualObject.h>

#include "cluster/message.hpp"

namespace vl
{

class RayObject : public vl::MovableObject
{
public :
	/// @brief Master constructor
	RayObject(std::string const &name, vl::SceneManagerPtr creator)
		: MovableObject(name, creator)
		, _position(Ogre::Vector3::ZERO)
		, _direction(-Ogre::Vector3::UNIT_Z)
		, _ogre_object(0)
	{}

	/// @brief Slave constructor
	RayObject(vl::SceneManagerPtr creator)
		: MovableObject(creator)
		, _position(Ogre::Vector3::ZERO)
		, _direction(-Ogre::Vector3::UNIT_Z)
		, _ogre_object(0)
	{}

	/// @brief Destructor
	virtual ~RayObject(void)
	{
		// @todo destroy the manual object
		//myManualObjectNode->detachObject("manual1");
		//mSceneMgr->destroyManualObject ("manual1"
	}

	void setPosition(Ogre::Vector3 const &pos)
	{
		if(_position != pos)
		{
			setDirty(DIRTY_TRANSFORM);
			_position = pos;
		}
	}

	Ogre::Vector3 const &getPosition(void) const
	{ return _position; }

	void setDirection(Ogre::Vector3 const &dir)
	{
		if(_position != dir)
		{
			setDirty(DIRTY_TRANSFORM);
			_direction = dir;
		}
	}

	Ogre::Vector3 const &getDirection(void) const
	{ return _direction; }

	void setMaterial(std::string const &name)
	{
		if(_material != name)
		{
			setDirty(DIRTY_PARAMS);
			_material = name;
		}
	}

	std::string const &getMaterial(void) const
	{ return _material; }

	// @todo add legth

	virtual Ogre::MovableObject *getNative(void) const
	{ return _ogre_object; }

	virtual std::string getTypeName(void) const
	{ return "RayObject"; }

	/// @todo not implemented
	virtual MovableObjectPtr clone(std::string const &append_to_name) const
	{ return 0; }

	enum DirtyBits
	{
		DIRTY_TRANSFORM = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_PARAMS = vl::MovableObject::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::MovableObject::DIRTY_CUSTOM << 2,
	};


private :
	virtual bool _doCreateNative(void)
	{
		if(!_ogre_object)
		{
			_ogre_object =  _creator->getNative()->createManualObject(_name);
			assert(_ogre_object);
			_ogre_object->begin(_material, Ogre::RenderOperation::OT_LINE_LIST);
			_ogre_object->position(_position);
			_ogre_object->position(_position + (_direction*1000));
			_ogre_object->end();
		}

		return true;
	}

	virtual void doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
	{
		if(dirtyBits & DIRTY_TRANSFORM)
		{
			msg << _position << _direction;
		}

		if(dirtyBits & DIRTY_PARAMS)
		{
			msg << _material;
		}
	}

	virtual void doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
	{
		if(dirtyBits & DIRTY_TRANSFORM)
		{
			msg >> _position >> _direction;
			if(_ogre_object)
			{
				// @todo update the manual object
			}
		}

		if(dirtyBits & DIRTY_PARAMS)
		{
			msg >> _material;
			if(_ogre_object)
			{
				// @todo update the manual object
			}
		}
	}

	Ogre::Vector3 _direction;
	Ogre::Vector3 _position;
	std::string _material;

	Ogre::ManualObject *_ogre_object;
};

}

#endif	// HYDRA_RAY_OBJECT_HPP