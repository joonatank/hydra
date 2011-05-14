/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 */

#ifndef VL_TYPEDEFS_HPP
#define VL_TYPEDEFS_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <memory>
#include <boost/scoped_ptr.hpp>

namespace vl
{
	// Forward declarations
	class Settings;
	class EnvSettings;
	class ProjSettings;
	class Player;
	class GameManager;
	class EventManager;
	class ResourceManager;
	class PythonContext;
	
	class Window;
	
	class Sensor;
	class Tracker;
	class Clients;

	class SceneManager;
	class SceneNode;
	class MovableObject;
	class RendererInterface;
	class Config;
	class Entity;
	class Light;
	class Camera;

	typedef boost::shared_ptr<Settings> SettingsRefPtr;
	typedef boost::shared_ptr< EnvSettings > EnvSettingsRefPtr;
	typedef boost::shared_ptr< ProjSettings > ProjSettingsRefPtr;
	
	// Tracker objects
	typedef boost::shared_ptr<Sensor> SensorRefPtr;
	typedef boost::shared_ptr<Tracker> TrackerRefPtr;
	typedef boost::shared_ptr<Clients> ClientsRefPtr;
	typedef boost::shared_ptr<Config> ConfigRefPtr;

	// Auto ptr because Renderer has a single owner
	// usually created elsewhere and passed to Config or Client
	typedef std::auto_ptr<RendererInterface> RendererInterfacePtr;

	typedef Player * PlayerPtr;
	// Can not be scoped ptr as config owns it but PythonContext needs access to it
	// change to shared_ptr and weak_ptr when possible
	typedef GameManager *GameManagerPtr;
	typedef EventManager * EventManagerPtr;
	typedef ResourceManager * ResourceManagerPtr;
	typedef boost::shared_ptr<ResourceManager> ResourceManagerRefPtr;
	typedef PythonContext * PythonContextPtr;

	typedef SceneManager * SceneManagerPtr;
	typedef SceneNode * SceneNodePtr;
	typedef MovableObject * MovableObjectPtr;
	typedef Entity * EntityPtr;
	typedef Light * LightPtr;
	typedef Camera * CameraPtr;

	class Mesh;
	class MeshManager;

	typedef boost::shared_ptr<Mesh> MeshRefPtr;
	typedef boost::shared_ptr<MeshManager> MeshManagerRefPtr;

	/// Containers
	typedef std::map<std::string, std::string> NamedValuePairList;
	
	typedef std::vector<SceneNodePtr> SceneNodeList;
	typedef std::vector<EntityPtr> EntityList;
	typedef std::vector<MovableObjectPtr> MovableObjectList;

	struct null_deleter
	{
		void operator()(void const *) const
		{
		}
	};

	/// Dummy struct makes easier to store all callbacks to same data structure
	struct Callback
	{
		virtual ~Callback(void) {}
	};

namespace gui
{
	class GUI;
	class Window;
	class EditorWindow;
	class ConsoleWindow;

	typedef boost::shared_ptr<GUI> GUIRefPtr;
	typedef boost::shared_ptr<Window> WindowRefPtr;
	typedef boost::shared_ptr<EditorWindow> EditorWindowRefPtr;
	typedef boost::shared_ptr<ConsoleWindow> ConsoleWindowRefPtr;
}

namespace physics
{
	class World;
	class RigidBody;

	typedef boost::shared_ptr<World> WorldRefPtr;
	typedef boost::weak_ptr<World> WorldWeakPtr;
	typedef boost::shared_ptr<RigidBody> RigidBodyRefPtr;
	typedef boost::weak_ptr<RigidBody> RigidBodyWeakPtr;

	// collision shapes
	class CollisionShape;
	class SphereShape;
	class BoxShape;
	class ConvexHullShape;
	class StaticPlaneShape;
	class StaticTriangleMeshShape;
	class CylinderShape;
	class ConeShape;
	class CapsuleShape;

	typedef boost::shared_ptr<CollisionShape> CollisionShapeRefPtr;
	typedef boost::shared_ptr<SphereShape> SphereShapeRefPtr;
	typedef boost::shared_ptr<BoxShape> BoxShapeRefPtr;
	typedef boost::shared_ptr<ConvexHullShape> ConvexHullShapeRefPtr;
	typedef boost::shared_ptr<StaticPlaneShape> StaticPlaneShapeRefPtr;
	typedef boost::shared_ptr<StaticTriangleMeshShape> StaticTriangleMeshShapeRefPtr;
	typedef boost::shared_ptr<CylinderShape> CylinderShapeRefPtr;
	typedef boost::shared_ptr<ConeShape> ConeShapeRefPtr;
	typedef boost::shared_ptr<CapsuleShape> CapsuleShapeRefPtr;

	class MotionState;
	class Constraint;
	class SixDofConstraint;
	class SliderConstraint;

	typedef boost::shared_ptr<Constraint> ConstraintRefPtr;
	typedef boost::shared_ptr<SixDofConstraint> SixDofConstraintRefPtr;
	typedef boost::shared_ptr<SliderConstraint> SliderConstraintRefPtr;
}

namespace cluster
{
	class Client;
	class Server;
	class Message;

	typedef boost::shared_ptr<Client> ClientRefPtr;
	typedef boost::shared_ptr<Server> ServerRefPtr;
	typedef boost::shared_ptr<Message> MessageRefPtr;
}

namespace ogre
{
	class Root;

	typedef boost::shared_ptr<Root> RootRefPtr;

}	// namespace ogre

}	// namespace vl

#endif	// VL_TYPEDEFS_HPP
