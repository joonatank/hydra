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

namespace vl
{
	// Forward declarations
	class Settings;
	class Tracker;
	class EnvSettings;
	class ProjSettings;
	class Player;
	class GameManager;
	class EventManager;
	class ResourceManager;
	class Clients;
	class SceneManager;
	class SceneNode;
	class Entity;
	class PythonContext;

	typedef boost::shared_ptr<Settings> SettingsRefPtr;
	typedef boost::shared_ptr< EnvSettings > EnvSettingsRefPtr;
	typedef boost::shared_ptr< ProjSettings > ProjSettingsRefPtr;
	typedef boost::shared_ptr<Tracker> TrackerRefPtr;
	typedef boost::shared_ptr<Clients> ClientsRefPtr;

	typedef Player * PlayerPtr;
	typedef GameManager * GameManagerPtr;
	typedef EventManager * EventManagerPtr;
	typedef ResourceManager * ResourceManagerPtr;
	typedef SceneManager * SceneManagerPtr;
	typedef SceneNode * SceneNodePtr;
	typedef Entity * EntityPtr;
	typedef PythonContext * PythonContextPtr;

	typedef std::map<std::string, std::string> NamedValuePairList;

	struct null_deleter
	{
		void operator()(void const *) const
		{
		}
	};

namespace cluster
{
	class Client;
	class Server;

	typedef boost::shared_ptr<Client> ClientRefPtr;
	typedef boost::shared_ptr<Server> ServerRefPtr;
}

namespace ogre
{
	class Root;

	typedef boost::shared_ptr<Root> RootRefPtr;

}	// namespace ogre

}	// namespace vl

#endif	// VL_TYPEDEFS_HPP
