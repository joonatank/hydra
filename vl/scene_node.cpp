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
	os << "SceneNode = " << a.getName() << " with ID = " << a.getID()
		<< " with position " << a.getPosition()
		<< " and orientation " << a.getOrientation();
	if( a._parent )
	{ os << " with parent " << a._parent->getName() << ".\n"; }
	else
	{ os << " without a parent.\n"; }
		
	if( !a._childs.empty() )
	{
		os << "with childs : ";
		for( std::vector<vl::SceneNodePtr>::const_iterator iter = a._childs.begin();
			iter != a._childs.end(); ++iter )
		{
			os << (*iter)->getName();
			if( iter != a._childs.end()-1 )
				os << ", ";
		}
		os << "\n";
	}

	if( !a._entities.empty() )
	{
		os << "with entities : ";
		for( std::vector<vl::EntityPtr>::const_iterator iter = a._entities.begin();
			 iter != a._entities.end(); ++iter )
		{
			os << (*iter)->getName();
			if( iter != a._entities.end()-1 )
				os << ", ";
		}
	}

	return os;
}


/// ------------------------------ Public ------------------------------------
vl::SceneNode::SceneNode( std::string const &name, vl::SceneManager *creator )
	: _name(name)
	, _position( Ogre::Vector3::ZERO )
	, _orientation( Ogre::Quaternion::IDENTITY )
	, _scale(Ogre::Vector3(1,1,1))
	, _visible(true)
	, _show_boundingbox(false)
	, _parent(0)
	, _ogre_node(0)
	, _creator(creator)
{
	assert( _creator );
}

void 
vl::SceneNode::setPosition( Ogre::Vector3 const &v )
{
	if( v != _position )
	{
		setDirty( DIRTY_POSITION );
		_position = v;
	}
}

void 
vl::SceneNode::setOrientation( Ogre::Quaternion const &q )
{
	if( q != _orientation )
	{
		setDirty( DIRTY_ORIENTATION );
		_orientation = q;
	}
}

void 
scale(Ogre::Real s)
{
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
vl::SceneNode::setVisibility( bool visible )
{
	if( _visible != visible )
	{
		setDirty( DIRTY_VISIBILITY );
		_visible = visible;
	}
}

void 
vl::SceneNode::showBoundingBox( bool show )
{
	if( _show_boundingbox != show )
	{
		setDirty(DIRTY_BOUNDING_BOX);
		_show_boundingbox = show;
	}
}

void 
vl::SceneNode::addEntity(vl::EntityPtr ent)
{
	assert(ent);
	if( hasEntity(ent) )
	{ return; }
	else
	{
		setDirty(DIRTY_ENTITIES);
		_entities.push_back(ent);

		if( _ogre_node )
		{ _ogre_node->attachObject(ent->getNative()); }
	}
}

void 
vl::SceneNode::removeEntity(vl::EntityPtr ent)
{
	assert(ent);
	std::vector<vl::EntityPtr>::iterator iter;
	for( iter = _entities.begin(); iter != _entities.end(); ++iter )
	{
		if( *iter == ent)
		{
			setDirty(DIRTY_ENTITIES);
			_entities.erase(iter);

			if( _ogre_node )
			{ _ogre_node->detachObject(ent->getNative()); }
			
			break;
		}
	}
}

bool 
vl::SceneNode::hasEntity(vl::EntityPtr ent) const
{
	assert(ent);
	std::vector<vl::EntityPtr>::const_iterator iter;
	for( iter = _entities.begin(); iter != _entities.end(); ++iter )
	{
		if( *iter == ent )
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

	if( hasChild(child) )
	{ return; }
	else
	{
		setDirty(DIRTY_CHILDS);
		_childs.push_back(child);

		/// Remove from current parent
		if( child->getParent() )
		{
			child->getParent()->removeChild(child);
		}

		child->_parent = this;

		if( _ogre_node )
		{
			Ogre::SceneNode *og_child = child->getNative();
			if( og_child )
			{
				// Hack to remove parents that are in the Ogre SC but not in ours
				// Should be removed when the DotScene serializer reads 
				// hierarchy correctly
				if( og_child->getParent() )
				{ og_child->getParent()->removeChild(og_child); }

				_ogre_node->addChild(child->getNative());
			}
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
			assert(child->getParent() == this);
			child->_parent = 0;

			setDirty(DIRTY_CHILDS);
			_childs.erase(iter);

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
vl::SceneNode::serialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	if( dirtyBits & DIRTY_NAME )
	{
		msg << _name;
	}
	// Serialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		msg << _position;
	}
	// Serialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		msg << _orientation;
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

	if( dirtyBits & DIRTY_CHILDS )
	{
		msg << _childs.size();
		std::vector<vl::SceneNodePtr>::iterator iter;
		for( iter = _childs.begin(); iter != _childs.end(); ++iter )
		{
			msg << (*iter)->getID();
		}
	}

	if( dirtyBits & DIRTY_ENTITIES )
	{
		msg << _entities.size();
		std::vector<vl::EntityPtr>::iterator iter;
		for( iter = _entities.begin(); iter != _entities.end(); ++iter )
		{
			msg << (*iter)->getID();
		}
	}
}

void
vl::SceneNode::deserialize( vl::cluster::ByteStream &msg, const uint64_t dirtyBits )
{
	// Deserialize name
	if( dirtyBits & DIRTY_NAME )
	{
		msg >> _name;
		// name should never be empty
		// @todo add exception throwing
		assert( !_name.empty() );
		_createNative();
	}
	// Deserialize position
	if( dirtyBits & DIRTY_POSITION )
	{
		msg >> _position;

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setPosition(_position); }
	}
	// Deserialize orientation
	if( dirtyBits & DIRTY_ORIENTATION )
	{
		msg >> _orientation;

		// If we have a correct node we need to transform it
		if( _ogre_node )
		{ _ogre_node->setOrientation(_orientation); }
	}

	
	if( DIRTY_SCALE & dirtyBits )
	{
		msg >> _scale;
		if( _ogre_node )
		{
			_ogre_node->setScale(_scale);
		}
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

	if( dirtyBits & DIRTY_ENTITIES )
	{
		std::vector<uint64_t> ent_ids;
		msg >> ent_ids;
		
		std::vector<EntityPtr> removed_ents;

		// Compare the old and new
		for( std::vector<EntityPtr>::iterator iter = _entities.begin(); 
			iter != _entities.end(); ++iter )
		{
			std::vector<uint64_t>::iterator id_iter 
				= std::find( ent_ids.begin(), ent_ids.end(), (*iter)->getID() );
			// Old id not found, we'll remove the child
			if( id_iter == ent_ids.end() )
			{
				removed_ents.push_back(*iter);
			}
			// Old found
			else
			{
				// remove from the list of ids
				ent_ids.erase(id_iter);
			}
		}

		for( std::vector<EntityPtr>::iterator iter = removed_ents.begin();
			 iter != removed_ents.end(); ++iter )
		{
			removeEntity(*iter);
		}

		/// Check the list and add remaining childs
		std::vector<uint64_t>::iterator id_iter;
		for( id_iter = ent_ids.begin(); id_iter != ent_ids.end(); ++id_iter )
		{
			addEntity(_creator->getEntityID(*id_iter));
		}
	}
}

void
vl::SceneNode::_createNative(void)
{
	if( _ogre_node )
	{ return; }

	assert( _creator );
	assert( _creator->getNative() );

	if( _creator->getNative()->hasSceneNode( _name ) )
	{
		_ogre_node = _creator->getNative()->getSceneNode(_name);
	}
	else
	{
		_ogre_node = _creator->getNative()->createSceneNode(_name);
		// Attach to root so that this Node can be used
		_creator->getNative()->getRootSceneNode()->addChild(_ogre_node);
	}

	assert(_ogre_node);
	_ogre_node->setOrientation(_orientation);
	_ogre_node->setPosition(_position);
	_ogre_node->setScale(_scale);
	_ogre_node->setVisible(_visible);
}


/// --------- Actions ----------
void
vl::HideAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setVisibility( false );
}

void
vl::ShowAction::execute(void )
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setVisibility( true );
}

void
vl::SetTransformation::execute(const vl::Transform& trans)
{
	if( !_node )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

	_node->setPosition( trans.position );
	_node->setOrientation( trans.quaternion );
}

