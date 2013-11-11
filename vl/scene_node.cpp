/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file scene_node.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/// Interface
#include "scene_node.hpp"

#include "scene_manager.hpp"
#include "entity.hpp"

#include "math/math.hpp"

/// ---------------------------- Global --------------------------------------
std::ostream &
vl::operator<<(std::ostream &os, vl::SceneNode const &a)
{
	os << "SceneNode = " << a.getName() << "\t ID = " << a.getID();
	if(!a._visible)
	{ os << "\t Hidden"; }
	os << "\n transform " << a.getTransform() << "\n";

	if( a._parent )
	{ os << "\t with parent " << a._parent->getName() << ".\n"; }
	else
	{ os << "\t without a parent.\n"; }
		
	if( !a._childs.empty() )
	{
		os << "\t with childs : ";
		for( std::vector<vl::SceneNodePtr>::const_iterator iter = a._childs.begin();
			iter != a._childs.end(); ++iter )
		{
			os << (*iter)->getName();
			if( iter != a._childs.end()-1 )
				os << ", ";
		}
		os << "\n";
	}

	if( !a._objects.empty() )
	{
		os << "\t with objects : ";
		for( std::vector<vl::MovableObjectPtr>::const_iterator iter = a._objects.begin();
			 iter != a._objects.end(); ++iter )
		{
			os << (*iter)->getName();
			if( iter != a._objects.end()-1 )
				os << ", ";
		}
	}

	return os;
}


/// ------------------------------ Public ------------------------------------
vl::SceneNode::SceneNode(std::string const &name, vl::SceneManager *creator, bool is_dynamic)
	: _name(name)
	, _scale(Ogre::Vector3::UNIT_SCALE)
	, _visible(true)
	, _show_boundingbox(false)
	, _inherit_scale(true)
	, _show_debug_display(false)
	, _show_axes(false)
	, _axes_size(3.0)
	, _parent(0)
	, _ogre_node(0)
	, _debug_axes(0)
	, _creator(creator)
	, _is_dynamic(is_dynamic)
{
	assert( _creator );
	// On renderers (where the native version is available)
	// Create an unamed Ogre SceneNode because we use our SceneNodes to handle
	// the retrieval by name.
	if( _creator->getNative() )
	{
		_ogre_node = _creator->getNative()->createSceneNode();
		/// Attach all nodes to Ogre Root Node for now so that they are in the SG
		/// Should really map our root node to Ogre Root
		/// which would implicitly put every node into the SG (because they are attached to our Root)
		_creator->getNative()->getRootSceneNode()->addChild(_ogre_node);

		/// Not setting any attributes, they will be overriden by Deserialize anyway
	}
}

vl::SceneNode::~SceneNode(void)
{
	if(_ogre_node && _name != "Root")
	{
		assert(_creator->getNative());
		_creator->getNative()->destroySceneNode(_ogre_node);
	}
}



void
vl::SceneNode::setTransform(vl::Transform const &trans)
{
	if(_transform != trans)
	{
		setDirty(DIRTY_TRANSFORM);
		_transform = trans;
		_transformed_cb(_transform);
	}
}

void 
vl::SceneNode::setWorldTransform(vl::Transform const &trans)
{
	vl::Transform wt;
	if(_parent)
	{
		wt = _parent->getWorldTransform();
		wt.invert();
	}

	setTransform(wt*trans);
}

vl::Transform 
vl::SceneNode::getWorldTransform(void) const
{
	if(_parent)
	{
		return _parent->getWorldTransform()*_transform;
	}

	return _transform; 
}


void 
vl::SceneNode::scale(Ogre::Real s)
{
	setScale(s*_scale);
}

void 
vl::SceneNode::scale(Ogre::Vector3 const &s)
{
	setScale(s*_scale);
}

void
vl::SceneNode::setScale(Ogre::Vector3 const &s)
{
	if( !s.isNaN() && !s.isZeroLength() )
	{
		update_variable(_scale, s, DIRTY_SCALE);
	}
}

void 
vl::SceneNode::setVisibility(bool visible, bool cascade)
{
	setDirty( DIRTY_VISIBILITY );
	_visible = visible;

	// Cascade to childs
	if(cascade)
	{
		for(SceneNodeList::iterator iter = _childs.begin(); iter != _childs.end(); ++iter)
		{ (*iter)->setVisibility(_visible, cascade); }
	}

	for(MovableObjectList::iterator iter = _objects.begin(); iter != _objects.end(); ++iter)
	{ (*iter)->setVisible(_visible); }
}

void 
vl::SceneNode::setShowBoundingBox(bool show)
{
	update_variable(_show_boundingbox, show, DIRTY_PARAMS);
}

void 
vl::SceneNode::setShowDebugDisplay(bool show)
{
	update_variable(_show_debug_display, show, DIRTY_PARAMS);
}

void
vl::SceneNode::setShowAxes(bool show)
{
	update_variable(_show_axes, show, DIRTY_PARAMS);
}

void
vl::SceneNode::setAxesSize(vl::scalar size)
{
	update_variable(_axes_size, size, DIRTY_PARAMS);
}

void
vl::SceneNode::setInheritScale(bool b)
{
	update_variable(_inherit_scale, b, DIRTY_PARAMS);
}

vl::SceneNodePtr
vl::SceneNode::clone(void) const
{
	std::string name = vl::generate_random_string();
	assert(!name.empty());

	return clone(name);
}

vl::SceneNodePtr
vl::SceneNode::clone(std::string const &append_to_name) const
{
	if(append_to_name.empty())
	{ return clone(); }

	return _do_clone(append_to_name, _parent, false);
}

vl::SceneNodePtr
vl::SceneNode::cloneDynamic(void) const
{
	std::string name = vl::generate_random_string();
	assert(!name.empty());

	return cloneDynamic(name);
}

vl::SceneNodePtr
vl::SceneNode::cloneDynamic(std::string const &append_to_name) const
{
	if(append_to_name.empty())
	{ return cloneDynamic(); }

	return _do_clone(append_to_name, _parent, true);
}

vl::SceneNodePtr 
vl::SceneNode::_do_clone(std::string const &append_to_name, vl::SceneNodePtr parent, bool dynamic) const
{
	SceneNodePtr node = 0;
	if(dynamic)
	{
		node = parent->getCreator()->createDynamicSceneNode(_name + append_to_name);
		parent->addChild(node);
	}
	else
	{ node = parent->createChildSceneNode(_name + append_to_name); }

	for(SceneNodeList::const_iterator iter = _childs.begin(); 
		iter != _childs.end(); ++iter)
	{
		SceneNodePtr child = (*iter)->_do_clone(append_to_name, node, dynamic);
	}
	
	for(MovableObjectList::const_iterator iter = _objects.begin(); 
		iter != _objects.end(); ++iter)
	{
		node->attachObject((*iter)->clone(append_to_name));
	}

	node->setTransform(_transform);
	node->setScale(_scale);
	node->setVisibility(_visible);

	// Not adding to selection because it would be more confusing than useful

	return node;
}

void 
vl::SceneNode::attachObject(vl::MovableObjectPtr obj)
{
	if(!obj)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing object to attach.")); }

	if( hasObject(obj) )
	{ return; }
	else
	{
		setDirty(DIRY_ATTACHED);
		_objects.push_back(obj);

		obj->setParent(this);
		obj->setVisible(_visible);
	}
}

void 
vl::SceneNode::detachObject(vl::MovableObjectPtr obj)
{
	if(!obj)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing object to detach.")); }

	std::vector<vl::MovableObjectPtr>::iterator iter;
	for( iter = _objects.begin(); iter != _objects.end(); ++iter )
	{
		if( *iter == obj )
		{
			setDirty(DIRY_ATTACHED);
			_objects.erase(iter);

			if( _ogre_node )
			{ _ogre_node->detachObject(obj->getNative()); }
			
			break;
		}
	}
}

bool 
vl::SceneNode::hasObject(vl::MovableObjectPtr obj) const
{
	if(!obj)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing object to search for.")); }

	std::vector<vl::MovableObjectPtr>::const_iterator iter;
	for( iter = _objects.begin(); iter != _objects.end(); ++iter )
	{
		if( *iter == obj )
		{ return true; }
	}

	return false;
}

vl::SceneNodePtr 
vl::SceneNode::createChildSceneNode(std::string const &name)
{
	assert(_creator);
	vl::SceneNodePtr node = _creator->createFreeSceneNode(name);
	addChild(node);
	return node;
}

void 
vl::SceneNode::addChild(vl::SceneNodePtr child)
{
	if(!child)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing Child node to add.")); }


	if( child == this )
	{ BOOST_THROW_EXCEPTION( vl::this_pointer() ); }

	if( !hasChild(child) )
	{
		setDirty(DIRTY_CHILDS);
		_childs.push_back(child);

		// Copy cascading parameters
		child->setVisibility(_visible);

		/// Remove from current parent
		if( child->getParent() )
		{
			child->getParent()->removeChild(child);
		}

		assert(child->getParent() == 0);
		vl::Transform child_world = child->getWorldTransform();

		child->_parent = this;

		// Keep the current transform of the child
		child->setWorldTransform(child_world);

		if( _ogre_node )
		{
			Ogre::SceneNode *og_child = child->getNative();
			assert(og_child);

			// Hack to remove parents that are in the Ogre SC but not in ours
			// Should be removed when the DotScene serializer reads 
			// hierarchy correctly
			if( og_child->getParent() )
			{ og_child->getParent()->removeChild(og_child); }

			_ogre_node->addChild(child->getNative());
		}
	}
}

void 
vl::SceneNode::removeChild(vl::SceneNodePtr child)
{
	if(!child)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing Child node to remove.")); }

	if( child == this )
	{ BOOST_THROW_EXCEPTION( vl::this_pointer() ); }

	std::vector<vl::SceneNodePtr>::iterator iter;
	for( iter = _childs.begin(); iter != _childs.end(); ++iter )
	{
		if( *iter == child )
		{
			// Reset the position relative to world
			vl::Transform child_world = child->getWorldTransform();

			assert(child->getParent() == this);
			child->_parent = 0;

			setDirty(DIRTY_CHILDS);
			_childs.erase(iter);

			child->setWorldTransform(child_world);

			if( _ogre_node && child->getNative() )
			{ _ogre_node->removeChild(child->getNative()); }
			
			break;
		}
	}
}

bool 
vl::SceneNode::hasChild(vl::SceneNodePtr child) const
{
	if(!child)
	{ BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("Missing Child node to search for.")); }

	std::vector<vl::SceneNodePtr>::const_iterator iter;
	for( iter = _childs.begin(); iter != _childs.end(); ++iter )
	{
		if( *iter == child )
		{ return true; }
	}

	return false;
}

void
vl::SceneNode::removeAllChildren(void)
{
	// Make a copy as removeChild will modify the original list
	SceneNodeList childs = _childs;
	for(SceneNodeList::iterator iter = childs.begin(); iter != childs.end(); ++iter)
	{
		removeChild(*iter);
	}
}

/// ------------------------- Protected --------------------------------------
// Does no transformation on the Ogre Node as the master copy should be in
// AppNode which does not have Ogre SceneGraph
void
vl::SceneNode::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits ) const
{
	if( dirtyBits & DIRTY_NAME )
	{
		msg << _name;
	}
	// Serialize position
	if(dirtyBits & DIRTY_TRANSFORM)
	{
		msg << _transform;
	}

	if( DIRTY_SCALE & dirtyBits )
	{
		msg << _scale;
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		msg << _visible;
	}

	if( dirtyBits & DIRTY_CHILDS )
	{
		msg << _childs.size();
		vl::SceneNodeList::const_iterator iter;
		for( iter = _childs.begin(); iter != _childs.end(); ++iter )
		{
			msg << (*iter)->getID();
		}
	}

	if( dirtyBits & DIRY_ATTACHED )
	{
		msg << _objects.size();
		vl::MovableObjectList::const_iterator iter;
		for( iter = _objects.begin(); iter != _objects.end(); ++iter )
		{
			msg << (*iter)->getID();
		}
	}

	if(dirtyBits & DIRTY_PARAMS)
	{
		msg << _show_boundingbox << _inherit_scale << _show_debug_display 
			<< _show_axes << _axes_size;
	}

}

void
vl::SceneNode::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	// Only renderers should deserialize
	assert(_ogre_node);

	// Deserialize name
	if( dirtyBits & DIRTY_NAME )
	{
		msg >> _name;
	}
	// Deserialize Transformation
	if(dirtyBits & DIRTY_TRANSFORM)
	{
		msg >> _transform;

		_ogre_node->setOrientation(_transform.quaternion);
		_ogre_node->setPosition(_transform.position);
	}

	
	if( DIRTY_SCALE & dirtyBits )
	{
		msg >> _scale;
		if( _ogre_node )
		{ _ogre_node->setScale(_scale); }
	}

	if( dirtyBits & DIRTY_VISIBILITY )
	{
		msg >> _visible;
		if( _ogre_node )
		{ _ogre_node->setVisible(_visible, false); }
	}

	if( dirtyBits & DIRTY_CHILDS )
	{
		std::vector<uint64_t> child_ids;
		msg >> child_ids;
		
		std::vector<SceneNodePtr> removed_childs;

		// Compare the old and new
		// Save the results to temporary arrays for later processing
		// This is because otherwise we would screw up the iterator here
		for( std::vector<SceneNodePtr>::const_iterator iter = _childs.begin(); 
			iter != _childs.end(); ++iter )
		{
			std::vector<uint64_t>::iterator id_iter 
				= std::find( child_ids.begin(), child_ids.end(), (*iter)->getID() );
			// Old id not found, we'll remove the child
			if( id_iter == child_ids.end() )
			{
				removed_childs.push_back(*iter);
			}
			// Old found
			else
			{
				// remove from the list of ids
				child_ids.erase(id_iter);
			}
		}

		for( std::vector<SceneNodePtr>::iterator iter = removed_childs.begin();
			 iter != removed_childs.end(); ++iter )
		{
			removeChild(*iter);
		}

		/// Check the list and add remaining childs
		std::vector<uint64_t>::iterator id_iter;
		for( id_iter = child_ids.begin(); id_iter != child_ids.end(); ++id_iter )
		{
			addChild(_creator->getSceneNodeID(*id_iter));
		}
	}

	if( dirtyBits & DIRY_ATTACHED )
	{
		std::vector<uint64_t> obj_ids;
		msg >> obj_ids;
		
		MovableObjectList removed_ents;

		// Compare the old and new
		for( MovableObjectList::iterator iter = _objects.begin(); 
			iter != _objects.end(); ++iter )
		{
			std::vector<uint64_t>::iterator id_iter 
				= std::find( obj_ids.begin(), obj_ids.end(), (*iter)->getID() );
			// Old id not found, we'll remove the child
			if( id_iter == obj_ids.end() )
			{
				removed_ents.push_back(*iter);
			}
			// Old found
			else
			{
				// remove from the list of ids
				obj_ids.erase(id_iter);
			}
		}

		for( MovableObjectList::iterator iter = removed_ents.begin();
			 iter != removed_ents.end(); ++iter )
		{
			detachObject(*iter);
		}

		/// Check the list and add remaining childs
		std::vector<uint64_t>::iterator id_iter;
		for( id_iter = obj_ids.begin(); id_iter != obj_ids.end(); ++id_iter )
		{
			attachObject(_creator->getMovableObjectID(*id_iter));
		}
	}


	if(dirtyBits & DIRTY_PARAMS)
	{
		msg >> _show_boundingbox >> _inherit_scale >> _show_debug_display 
			>> _show_axes >> _axes_size;
		assert(_ogre_node);
		
		_ogre_node->showBoundingBox(_show_boundingbox);
		_ogre_node->setInheritScale(_inherit_scale);
		_ogre_node->setDebugDisplayEnabled(_show_debug_display, true);
		
		if(_show_axes)
		{
			if(!_debug_axes)
			{
				_debug_axes = new ogre::Axes(_axes_size/_scale.length());
				_ogre_node->attachObject(_debug_axes);
			}

			_debug_axes->setVisible(true);
			_debug_axes->setLength(_axes_size/_scale.length());
		}
		else
		{
			if(_debug_axes)
			{ _debug_axes->setVisible(false); }
		}
	}
}
