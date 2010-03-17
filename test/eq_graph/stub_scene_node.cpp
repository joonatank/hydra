#include "eq_graph/eq_scene_node.hpp"

namespace vl
{
namespace cl
{
	// TODO provide stub implementation of vl::cl::SceneNode here
	SceneNode::SceneNode( vl::graph::SceneManager *creator,
			std::string const &name )
	{}

	void
	SceneNode::destroy( void )
	{}

	void
	SceneNode::attachObject( vl::graph::MovableObject *object )
	{}

	void
	SceneNode::detachObject( vl::graph::MovableObject *object )
	{}

	vl::graph::SceneNode *
	SceneNode::createChild( std::string const &name )
	{}

	void
	SceneNode::setParent( vl::graph::SceneNode *parent )
	{}

	void
	SceneNode::addChild( vl::graph::SceneNode *child )
	{}

	void
	SceneNode::removeChild( vl::graph::SceneNode *child )
	{}

	void
	SceneNode::serialize( eq::net::DataOStream& os, const uint64_t dirtyBits )
	{}

	void
	SceneNode::deserialize( eq::net::DataIStream& is, const uint64_t dirtyBits )
	{}
}
}
