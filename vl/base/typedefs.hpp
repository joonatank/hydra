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

namespace eqOgre
{
	class PythonContext;
	class SceneManager;

	typedef PythonContext * PythonContextPtr;
	typedef SceneManager * SceneManagerPtr;

}	// namespace eqOgre

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

	struct null_deleter
	{
		void operator()(void const *) const
		{
		}
	};

	typedef boost::shared_ptr<Settings> SettingsRefPtr;
	typedef boost::shared_ptr< EnvSettings > EnvSettingsRefPtr;
	typedef boost::shared_ptr< ProjSettings > ProjSettingsRefPtr;
	typedef boost::shared_ptr<Tracker> TrackerRefPtr;
	typedef boost::shared_ptr<Clients> ClientsRefPtr;

	typedef Player * PlayerPtr;
	typedef GameManager * GameManagerPtr;
	typedef EventManager * EventManagerPtr;
	typedef ResourceManager * ResourceManagerPtr;

	typedef std::map<std::string, std::string> NamedValuePairList;

namespace ogre
{
	class Root;

	typedef boost::shared_ptr<Root> RootRefPtr;

}	// namespace ogre

}	// namespace vl

#endif	// VL_TYPEDEFS_HPP
