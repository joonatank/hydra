/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *	@file movable_object.hpp
 *
 */

#ifndef HYDRA_MOVABLE_OBJECT_HPP
#define HYDRA_MOVABLE_OBJECT_HPP

// Base class
#include "distributed.hpp"

#include "typedefs.hpp"

namespace vl
{

/**	@class MovableObject
 *	@brief A base class for anything that can be attached to a SceneNode
 *	Does not distribute anything for now.
 */
class MovableObject : public vl::Distributed
{
public :
	MovableObject(std::string const &name, vl::SceneManagerPtr creator)
		: _name(name), _creator(creator)
	{}

	std::string const &getName(void) const
	{ return _name; }

	vl::SceneNodePtr getParent(void) const
	{ return _parent; }

	/// Internal
	void setParent(vl::SceneNodePtr parent);

	virtual Ogre::MovableObject *getNative(void) const = 0;

	virtual std::string getTypeName(void) const = 0;

	enum DirtyBits
	{
		DIRTY_NAME = vl::Distributed::DIRTY_CUSTOM << 0,
		DIRTY_CUSTOM = vl::Distributed::DIRTY_CUSTOM << 1,
	};

private :
	/// Virtual private methods

	virtual bool _doCreateNative(void) = 0;
	virtual void doSerialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) = 0;
	virtual void doDeserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) = 0;

	/// Using template method pattern for creating the native object
	/// This manages the the common functionality
	bool _createNative(void);

	/// Template method patter for the serialization
	void serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );


	void deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits );

/// Protected data
protected :
	std::string _name;

	vl::SceneManagerPtr _creator;
	
	vl::SceneNodePtr _parent;

};	// class MovableObject

}	// namespace vl

#endif	// HYDRA_MOVABLE_OBJECT_HPP