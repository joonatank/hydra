#include "exceptions.hpp" 

#include "server_command.hpp"

// FIXME commands are in disrepair for now as we move from
// Ogre scene graph to our own abstract sharable.

void
vl::server::CreateCmd::operator()( vl::graph::Root *root )
{
	// For now we need eqOgre::Root so we cast
	// later this should be removed.
	//eqOgre::Root *r = static_cast<eqOgre::Root *>( root );
	//EQASSERT( r );

	/*
	Ogre::SceneManager *sm = 0;

	// TODO we should add here the possibility to create child
	// SceneNodes...
	// Also creating MovableObjects without attaching them is bit
	// pointless so we should create and attach them at the same time.
	if( cmd.typeName == "SceneManager" )
	{
		Ogre::SceneManager *sm = r->createSceneManager( cmd.name );
		sm->setAmbientLight( Ogre::ColourValue( 0.7, 0.7, 0.7 ) );
	}
	else if( cmd.typeName == "SceneNode" )
	{
		EQINFO << "Creating SceneNode : " << cmd.name << std::endl;

		if( cmd.creator.empty() )
		{ sm  = r->getActiveScene(); }
		else
		{ sm = r->getSceneManager( cmd.creator ); }

		EQASSERT( sm );

		if( !cmd.parent.empty() )
		{
			// Find parent SceneNode and create this node as a child
			Ogre::SceneNode *parent_node = sm->getSceneNode( cmd.parent );
			parent_node->createChildSceneNode( cmd.name );
		}
		else
		{
			// No parent provided so we create this under RootNode
			sm->getRootSceneNode()->createChildSceneNode( cmd.name );
		}
	}
	else if( cmd.typeName == "Camera" || cmd.typeName == "Entity" 
			|| cmd.typeName == "Light" )
	{
		EQINFO << "Creating MovableObject : " << cmd.name 
			<< " of type = " << cmd.typeName << " with parent = "
			<< cmd.parent << std::endl;

		if( cmd.creator.empty() )
		{ sm  = r->getActiveScene(); }
		else
		{ sm = r->getSceneManager( cmd.creator ); }

		EQASSERT( sm );

		Ogre::MovableObject *movable = 
			sm->createMovableObject( cmd.name, cmd.typeName, &cmd.params );

		// Parent provided so we attach this object
		if( !cmd.parent.empty() )
		{
			Ogre::SceneNode *parent_node = sm->getSceneNode( cmd.parent );
			parent_node->attachObject( movable );
		}
	}
	else
	{
		throw vl::exception(
			"Not handled create type passed.",
			"Node::_processCommands" );
	}
	*/
}

// TODO this hasn't been tested in anyway...
void
vl::server::DeleteCmd::operator()( vl::graph::Root *root )
{
	// TODO remove the casting
	// we need it for Root::getActiveScene(), but if we get rid of that...
	//Ogre::SceneManager *sm = 0;

	/*
	if( cmd.creator.empty() )
	{ sm  = r->getActiveScene(); }
	else
	{ sm = r->getSceneManager( cmd.creator ); }
	EQASSERT( sm );

	if( cmd.typeName == "SceneNode" )
	{
		sm->destroySceneNode( cmd.name );
	}
	else if( cmd.typeName == "Camera" || cmd.typeName == "Entity"
			|| cmd.typeName == "Light" )
	{
		sm->destroyMovableObject( cmd.name, cmd.typeName );
	}
	*/
}

void
vl::server::MoveCmd::operator()( vl::graph::Root *root )
{
	// TODO remove the casting
	// we need it for Root::getActiveScene(), but if we get rid of that...
	//Ogre::SceneManager *sm = 0;

	/*
	if( cmd.creator.empty() )
	{ sm  = r->getActiveScene(); }
	else
	{ sm = r->getSceneManager( cmd.creator ); }
	EQASSERT( sm );

	// TODO add rotation if we have quaternion
	// TODO add checking if the quaternion or vector is zero
	// so we don't do useless function calls
	//
	// TODO add params for SetPosition, LookAt, Translate
	// so we can do them all here.
	//
	// TODO add TransformSpace param
	if( cmd.typeName == "SceneNode" )
	{
		Ogre::SceneNode *node = sm->getSceneNode( cmd.name );
		Ogre::Vector3 vec( cmd.position.x, cmd.position.y, cmd.position.z );
		node->translate( vec );
	}
	else if( cmd.typeName == "Camera" || cmd.typeName == "Entity" 
			|| cmd.typeName == "Light" )
	{
		throw vl::exception(
				"MovableObject can not be moved on it's own ",
				"Node::_processCommands" );
	}
	*/
}

/*
void
vl::server::RotateCmd::operator()( Ogre::Root *root )
{
	eqOgre::Root *r = (eqOgre::Root *)(root);
	Ogre::SceneManager *sm = 0;

	if( cmd.creator.empty() )
	{ sm  = r->getActiveScene(); }
	else
	{ sm = r->getSceneManager( cmd.creator ); }
	EQASSERT( sm );

	if( cmd.typeName == "SceneNode" )
	{
		Ogre::SceneNode *node = sm->getSceneNode( cmd.name);
		node->rotate( cmd.rotation);
	}
	else if( typeName == "Camera" || typeName == "Entity" 
			|| typeName == "Light" )
	{
		EQERROR <<  "MovableObject can not be moved on it's own ";
		EQASSERT( false );
	}
	else
	{
		// This should throw
	}
}
*/

/*
void 
vl::server::LookAtCmd::operator()( Ogre::Root *root )
{
	eqOgre::Root *r = (eqOgre::Root *)(root);
	Ogre::SceneManager *sm = 0;

	if( creator.empty() )
	{ sm  = r->getActiveScene(); }
	else
	{ sm = r->getSceneManager( creator ); }
	EQASSERT( sm );

	if( typeName == "SceneNode" )
	{
		Ogre::SceneNode *node = sm->getSceneNode( name );
		// For now hard-coded to use world space
		node->lookAt( vec, Ogre::Node::TS_WORLD );
	}
	else if( typeName == "Camera" || typeName == "Entity" 
			|| typeName == "Light" )
	{
		EQERROR <<  "MovableObject can not be moved on it's own ";
		EQASSERT( false );
	}
}

*/

