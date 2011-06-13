/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-01
 *	@file scene_node.cpp
 */

/// Interface
#include "scene_node.hpp"

#include "scene_manager.hpp"
#include "entity.hpp"

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
vl::SceneNode::SceneNode( std::string const &name, vl::SceneManager *creator )
	: _name(name)
	, _scale(Ogre::Vector3::UNIT_SCALE)
	, _visible(true)
	, _show_boundingbox(false)
	, _inherit_scale(false)
	, _parent(0)
	, _ogre_node(0)
	, _creator(creator)
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

void 
vl::SceneNode::transform(Ogre::Matrix4 const &m)
{
	if(m != Ogre::Matrix4::IDENTITY)
	{
		Ogre::Vector3 s, pos;
		Ogre::Quaternion q;
		m.decomposition(pos, s, q);

		translate(pos);
		rotate(q);
		scale(s);
	}
}

void 
vl::SceneNode::setTransform(Ogre::Matrix4 const &m)
{
	Ogre::Vector3 s, pos;
	Ogre::Quaternion q;
	m.decomposition(pos, s, q);
	
	setOrientation(q);
	setPosition(pos);
	setScale(s);
}

void 
vl::SceneNode::transform(vl::Transform const &trans)
{
	if(!trans.isIdentity())
	{
		setTransform(trans*_transform);
	}
}

void
vl::SceneNode::setTransform(vl::Transform const &trans)
{
	if(_transform != trans)
	{
		setDirty(DIRTY_TRANSFORM);
		_transform = trans;
	}
}

void 
vl::SceneNode::setTransform(vl::Transform const &trans, vl::SceneNodePtr reference)
{
	vl::Transform world_trans;
	if(_parent)
	{ world_trans = _parent->getWorldTransform(); }
	world_trans.invert();
	setTransform(world_trans * reference->getWorldTransform() * trans);
}

vl::Transform 
vl::SceneNode::getTransform(vl::SceneNodePtr reference) const
{
	vl::Transform world_trans;
	if(_parent)
	{ world_trans = _parent->getWorldTransform(); }
	world_trans.invert();
	return reference->getWorldTransform() * world_trans * _transform;
}

void 
vl::SceneNode::setWorldTransform(vl::Transform const &trans)
{
	vl::Transform world_trans;
	if(_parent)
	{ world_trans = _parent->getWorldTransform(); }
	world_trans.invert();
	setTransform(world_trans*trans);
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
vl::SceneNode::setPosition(Ogre::Vector3 const &v)
{
	if( v != _transform.position )
	{
		setDirty(DIRTY_TRANSFORM);
		_transform.position = v;
	}
}

void 
vl::SceneNode::translate(Ogre::Vector3 const &v, vl::SceneNodePtr reference)
{
	/// World space
	if(!reference)
	{
		setPosition(_transform.position + v);
	}
	/// Local space
	else if(reference == this)
	{
		setPosition(_transform.position + _transform.rotate(v));
	}
	/// Reference space
	else
	{
		vl::Transform ref_world = reference->getWorldTransform();
		setPosition(_transform.position + ref_world.rotate(v));
	}
}

void 
vl::SceneNode::translate(Ogre::Vector3 const &v, vl::TransformSpace space)
{
	if(space == TS_LOCAL)
	{
		translate(v, this);
	}
	else if(space == TS_PARENT)
	{
		translate(v, _parent);
	}
	else
	{
		translate(v, 0);
	}
}

void 
vl::SceneNode::translate(Ogre::Vector3 const &v)
{
	translate(v, this);
}

void 
vl::SceneNode::setOrientation( Ogre::Quaternion const &q )
{
	if( q != _transform.quaternion )
	{
		setDirty(DIRTY_TRANSFORM);
		_transform.quaternion = q;
	}
}

void
vl::SceneNode::rotate(Ogre::Quaternion const &q, vl::SceneNodePtr reference)
{
	/// With references, this function is rotate around a point
	/// the matrix to represent such a rotation is T = inv(P)*R*P
	/// where P is translate matrix for the point and R is the rotation matrix
	/// this matrix can be represented with [Tq, Tv] = [Rq, Rq*p - p]
	/// where Tq is the quaternion part of T and Tv is the vector part
	/// Rq is the quaternion representation of R and p is the vector of P
	/// seems like this is for left handed coordinate system, switch inv(P) and P for right handed.

	// Um seems like the rotate around doesn't not work as expected when
	// using multiple rotations. Though the usefulness of this feature is depatable.
	vl::Transform t;
	/// World space
	if(!reference)
	{
		t.quaternion = q;
		t.position = getWorldTransform().position - q*getWorldTransform().position;
	}
	/// Local space
	else if(reference == this)
	{
		t.quaternion = q;
	}
	/// Reference space
	else
	{
		vl::Transform ref_world = reference->getWorldTransform();
		Transform inv_world = getWorldTransform();
		//inv_world.invert();
		t.quaternion = q;
		t.position = ref_world.position - q*ref_world.position;
	}

	setTransform(_transform*t);
}

void
vl::SceneNode::rotate(Ogre::Quaternion const &q, vl::TransformSpace space)
{
	if(space == TS_LOCAL)
	{
		rotate(q, this);
	}
	else if(space == TS_PARENT)
	{
		rotate(q, _parent);
	}
	else
	{
		rotate(q, 0);
	}
}

void
vl::SceneNode::rotate(Ogre::Quaternion const &q)
{
	rotate(q, this);
}

void 
vl::SceneNode::scale(Ogre::Real s)
{
	if( s != 0 && s != 1 )
	{
		setDirty(DIRTY_SCALE);
		_scale *= s;
	}
}

void 
vl::SceneNode::scale(Ogre::Vector3 const &s)
{
	if( !s.isNaN() && !s.isZeroLength() && s != Ogre::Vector3(1,1,1) )
	{
		setDirty(DIRTY_SCALE);
		_scale *= s;
	}
}

void
vl::SceneNode::setScale(Ogre::Vector3 const &s)
{
	if( !s.isNaN() && !s.isZeroLength() && s != _scale )
	{
		setDirty(DIRTY_SCALE);
		_scale = s;
	}
}

void 
vl::SceneNode::setVisible(bool visible)
{
	if( _visible != visible )
	{
		setDirty( DIRTY_VISIBILITY );
		_visible = visible;

		// Cascade to childs
		for(SceneNodeList::iterator iter = _childs.begin(); iter != _childs.end(); ++iter)
		{ (*iter)->setVisible(_visible); }
		
		for(MovableObjectList::iterator iter = _objects.begin(); iter != _objects.end(); ++iter)
		{ (*iter)->setVisible(_visible); }
	}
}

void 
vl::SceneNode::setShowBoundingBox(bool show)
{
	if( _show_boundingbox != show )
	{
		setDirty(DIRTY_BOUNDING_BOX);
		_show_boundingbox = show;
	}
}

void
vl::SceneNode::setInheritScale(bool b)
{
	if(_inherit_scale != b)
	{
		setDirty(DIRTY_INHERIT_SCALE);
		_inherit_scale = b;
	}
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

	return doClone(append_to_name, _parent);
}

vl::SceneNodePtr 
vl::SceneNode::doClone(std::string const &append_to_name, vl::SceneNodePtr parent) const
{
	SceneNodePtr node = parent->createChildSceneNode(_name + append_to_name);

	for(SceneNodeList::const_iterator iter = _childs.begin(); 
		iter != _childs.end(); ++iter)
	{
		SceneNodePtr child = (*iter)->doClone(append_to_name, node);
	}
	
	for(MovableObjectList::const_iterator iter = _objects.begin(); 
		iter != _objects.end(); ++iter)
	{
		node->attachObject((*iter)->clone(append_to_name));
	}

	node->setTransform(_transform);
	node->setScale(_scale);
	node->setVisible(_visible);

	// Not adding to selection because it would be more confusing than useful

	return node;
}

void 
vl::SceneNode::attachObject(vl::MovableObjectPtr obj)
{
	assert(obj);
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
	assert(obj);
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
	assert(obj);
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
	assert(child);

	if( child == this )
	{
		BOOST_THROW_EXCEPTION( vl::this_pointer() );
	}

	if( !hasChild(child) )
	{
		setDirty(DIRTY_CHILDS);
		_childs.push_back(child);

		// Copy cascading parameters
		child->setVisible(_visible);

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
	assert(child);
	if( child == this )
	{
		BOOST_THROW_EXCEPTION( vl::this_pointer() );
	}

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
	assert(child);
	std::vector<vl::SceneNodePtr>::const_iterator iter;
	for( iter = _childs.begin(); iter != _childs.end(); ++iter )
	{
		if( *iter == child )
		{ return true; }
	}

	return false;
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

	if( dirtyBits & DIRTY_BOUNDING_BOX )
	{
		msg << _show_boundingbox;
	}

	if(dirtyBits & DIRTY_INHERIT_SCALE)
	{
		msg << _inherit_scale;
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
		{ _ogre_node->setVisible(_visible); }
	}

	if( dirtyBits & DIRTY_BOUNDING_BOX )
	{
		msg >> _show_boundingbox;
		if( _ogre_node )
		{ _ogre_node->showBoundingBox(_show_boundingbox); }
	}

	if(dirtyBits & DIRTY_INHERIT_SCALE)
	{
		msg >> _inherit_scale;
		_ogre_node->setInheritScale(_inherit_scale);
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
}

/// --------- Actions ----------
void
vl::HideAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->hide();
}

void
vl::ShowAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->show();
}

void
vl::SetTransformation::execute(const vl::Transform& trans)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setPosition( trans.position );
	_node->setOrientation( trans.quaternion );
}

