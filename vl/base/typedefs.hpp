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

	struct null_deleter
	{
		void operator()(void const *) const
		{
		}
	};

	typedef boost::shared_ptr<Settings> SettingsRefPtr;
	typedef boost::shared_ptr<Tracker> TrackerRefPtr;

	typedef std::map<std::string, std::string> NamedValuePairList;

	// TODO define angle class
	typedef double angle;

namespace ogre
{
	class Root;

	typedef boost::shared_ptr<Root> RootRefPtr;

}

}	// namespace vl

#endif
