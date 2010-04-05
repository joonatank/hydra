#ifndef VL_TYPEDEFS_HPP
#define VL_TYPEDEFS_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <memory>

namespace vl
{
	typedef std::map<std::string, std::string> NamedValuePairList;

	// Graph typedefs
namespace graph
{
	// Forward declarations
	class SceneManager;
	class SceneNode;
	class MovableObject;
	class Entity;
	class Camera;
	class SceneNodeFactory;
	class MovableObjectFactory;
	class Viewport;
	class RenderWindow;
	class Root;

	typedef boost::shared_ptr<Root> RootRefPtr;

	typedef boost::shared_ptr<Viewport> ViewportRefPtr;
	typedef boost::shared_ptr<RenderWindow> RenderWindowRefPtr;

	typedef boost::shared_ptr<MovableObject> MovableObjectRefPtr;
	typedef boost::shared_ptr<Entity> EntityRefPtr;
	typedef boost::shared_ptr<Camera> CameraRefPtr;

	typedef boost::shared_ptr<SceneManager> SceneManagerRefPtr;
	typedef boost::weak_ptr<SceneManager> SceneManagerWeakPtr;

	typedef boost::shared_ptr<SceneNode> SceneNodeRefPtr;
	typedef boost::weak_ptr<SceneNode> SceneNodeWeakPtr;

	typedef std::vector<SceneNodeRefPtr> ChildContainer;
	typedef std::vector<MovableObjectRefPtr> ObjectContainer;

	typedef boost::shared_ptr<SceneNodeFactory> SceneNodeFactoryPtr;
	typedef boost::shared_ptr<MovableObjectFactory> MovableObjectFactoryPtr;

}

}	// namespace vl

#endif
