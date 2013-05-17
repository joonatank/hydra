/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-01
 *	@file typedefs.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_TYPEDEFS_HPP
#define HYDRA_TYPEDEFS_HPP

#include <map>
#include <string>
#include <vector>

/// Pointers
/// Using boost types rather than tr1 because they can be automatically mapped 
/// with boost::python
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace vl
{
	typedef std::map<std::string, std::string> NamedParamList;

	class Application;
	typedef std::auto_ptr<Application> ApplicationUniquePtr;

	class CadImporter;
	typedef boost::shared_ptr<CadImporter> CadImporterRefPtr;

	class Window;

	/// Settings
	class Settings;
	class ProjSettings;

	typedef boost::shared_ptr<Settings> SettingsRefPtr;
	typedef boost::shared_ptr<ProjSettings> ProjSettingsRefPtr;
	
	namespace config
	{
		class EnvSettings;
		typedef boost::shared_ptr<EnvSettings> EnvSettingsRefPtr;
	}

	class RendererInterface;
	class Config;

	typedef boost::shared_ptr<Config> ConfigRefPtr;
	// Auto ptr because Renderer has a single owner
	// usually created elsewhere and passed to Config or Client
	// Normal pointer for callbacks
	typedef RendererInterface *RendererPtr;
	typedef std::auto_ptr<RendererInterface> RendererUniquePtr;

	// Tracker objects
	class TrackerSensor;
	class Tracker;
	class Clients;
	class analog_sensor;
	class vrpn_analog_client;

	typedef boost::shared_ptr<TrackerSensor> SensorRefPtr;
	typedef boost::shared_ptr<Tracker> TrackerRefPtr;
	typedef boost::shared_ptr<Clients> ClientsRefPtr;
	typedef boost::shared_ptr<analog_sensor> analog_sensor_ref_ptr;
	typedef boost::shared_ptr<vrpn_analog_client> vrpn_analog_client_ref_ptr;

	class Player;
	class GameManager;
	class EventManager;
	class ResourceManager;
	class PythonContext;

	typedef Player * PlayerPtr;
	// Can not be scoped ptr as config owns it but PythonContext needs access to it
	// change to shared_ptr and weak_ptr when possible
	typedef GameManager *GameManagerPtr;
	typedef EventManager * EventManagerPtr;
	typedef ResourceManager * ResourceManagerPtr;
	typedef boost::shared_ptr<ResourceManager> ResourceManagerRefPtr;
	typedef PythonContext * PythonContextPtr;

	class GameObject;
	typedef boost::shared_ptr<GameObject> GameObjectRefPtr;
	typedef std::vector<GameObjectRefPtr> GameObjectList;

	/// SceneObjects
	class SceneManager;
	class SceneNode;
	class MovableObject;
	class Entity;
	class Light;
	class Camera;
	class MovableText;
	class RayObject;

	typedef SceneManager * SceneManagerPtr;
	typedef SceneNode * SceneNodePtr;
	typedef MovableObject * MovableObjectPtr;
	typedef Entity * EntityPtr;
	typedef Light * LightPtr;
	typedef Camera * CameraPtr;
	typedef MovableText * MovableTextPtr;
	typedef RayObject * RayObjectPtr;

	/// Resources
	class Mesh;
	class MeshManager;

	typedef boost::shared_ptr<Mesh> MeshRefPtr;
	typedef boost::shared_ptr<MeshManager> MeshManagerRefPtr;

	class Material;
	class MaterialManager;

	typedef boost::shared_ptr<Material> MaterialRefPtr;
	typedef boost::shared_ptr<MaterialManager> MaterialManagerRefPtr;

	class Recording;
	typedef boost::shared_ptr<Recording> RecordingRefPtr;

	/// Containers
	typedef std::map<std::string, std::string> NamedValuePairList;
	
	typedef std::vector<SceneNodePtr> SceneNodeList;
	typedef std::vector<EntityPtr> EntityList;
	typedef std::vector<MovableObjectPtr> MovableObjectList;
	typedef std::vector<CameraPtr> CameraList;

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

	/// Non-physics constraints
	class KinematicBody;
	class KinematicWorld;
	class ConstraintSolver;
	class Constraint;
	class SixDofConstraint;
	class SliderConstraint;
	class HingeConstraint;
	class FixedConstraint;

	typedef boost::shared_ptr<KinematicBody> KinematicBodyRefPtr;
	typedef boost::shared_ptr<KinematicWorld> KinematicWorldRefPtr;
	typedef boost::shared_ptr<ConstraintSolver> ConstraintSolverRefPtr;
	typedef boost::shared_ptr<Constraint> ConstraintRefPtr;
	typedef boost::shared_ptr<SixDofConstraint> SixDofConstraintRefPtr;
	typedef boost::shared_ptr<SliderConstraint> SliderConstraintRefPtr;
	typedef boost::shared_ptr<HingeConstraint> HingeConstraintRefPtr;
	typedef boost::shared_ptr<FixedConstraint> FixedConstraintRefPtr;

	typedef std::vector<KinematicBodyRefPtr> KinematicBodyList;
	typedef std::vector<ConstraintRefPtr> ConstraintList;

	class InputDevice;
	class Joystick;
	class PCAN;
	class GameJoystick;
	class SerialJoystick;
	class JoystickHandler;
	class ConstraintJoystickHandler;
	typedef boost::shared_ptr<InputDevice> InputDeviceRefPtr;
	typedef boost::shared_ptr<Joystick> JoystickRefPtr;
	typedef boost::shared_ptr<PCAN> PCANRefPtr;
	typedef boost::shared_ptr<GameJoystick> GameJoystickRefPtr;
	typedef boost::shared_ptr<SerialJoystick> SerialJoystickRefPtr;
	typedef boost::shared_ptr<JoystickHandler> JoystickHandlerRefPtr;
	typedef boost::shared_ptr<ConstraintJoystickHandler> ConstraintJoystickHandlerRefPtr;

namespace gui
{
	class GUI;
	class Window;
	class PerformanceOverlay;
	class ConsoleWindow;

	typedef boost::shared_ptr<GUI> GUIRefPtr;
	typedef boost::shared_ptr<Window> WindowRefPtr;
	typedef boost::shared_ptr<PerformanceOverlay> PerformanceOverlayRefPtr;
	typedef boost::shared_ptr<ConsoleWindow> ConsoleWindowRefPtr;
}

namespace physics
{
	class World;
	class RigidBody;
	class Tube;

	typedef boost::shared_ptr<World> WorldRefPtr;
	typedef boost::weak_ptr<World> WorldWeakPtr;
	typedef World * WorldPtr;
	typedef boost::shared_ptr<RigidBody> RigidBodyRefPtr;
	typedef boost::weak_ptr<RigidBody> RigidBodyWeakPtr;
	typedef boost::shared_ptr<Tube> TubeRefPtr;
	typedef boost::shared_ptr<const Tube> TubeConstRefPtr;

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
	class HingeConstraint;

	typedef boost::shared_ptr<vl::physics::Constraint> ConstraintRefPtr;
	typedef boost::shared_ptr<vl::physics::SixDofConstraint> SixDofConstraintRefPtr;
	typedef boost::shared_ptr<vl::physics::SliderConstraint> SliderConstraintRefPtr;
	typedef boost::shared_ptr<vl::physics::HingeConstraint> HingeConstraintRefPtr;

	// Containers
	typedef std::vector<RigidBodyRefPtr> RigidBodyList;
	typedef std::vector<ConstraintRefPtr> ConstraintList;
	typedef std::vector<TubeRefPtr> TubeList;
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
