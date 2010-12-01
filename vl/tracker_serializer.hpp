/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 *
 *	Tracker Configuration. A struct that holds the configuration for the trackers.
 *	Tracker types supported VRPN and custom UDP proto.
 *
 *	Also contains an XML deserializer
 */

#ifndef VL_TRACKER_SERIALIZER_HPP
#define VL_TRACKER_SERIALIZER_HPP

#include "tracker.hpp"
#include "base/rapidxml.hpp"
#include "base/typedefs.hpp"

#include "resource_manager.hpp"

namespace vl
{

class Clients
{
public :
	void addTracker( TrackerRefPtr tracker )
	{ _trackers.push_back( tracker ); }

	TrackerRefPtr getTracker( size_t index )
	{ return _trackers.at(index); }

	size_t getNTrackers( void ) const
	{ return _trackers.size(); }

protected :
	std::vector<TrackerRefPtr> _trackers;
};

typedef boost::shared_ptr<Clients> ClientsRefPtr;

class TrackerSerializer
{
public :
	class Connection
	{
	public :
		/**	Constructor
		*
		*	parameters :
		*	hostname is mandatory
		*	port depends on the protocol, if the protocol has default value passing
		*	a zero as a port will use the default.
		*/
		Connection( std::string const &host, uint16_t por = 0 )
			: hostname( host ), port( por )
		{}

		std::string hostname;
		uint16_t port;
	};

	/// Constructor
	/// Pass a pointer to the configuration where the data is deserialized
	TrackerSerializer( ClientsRefPtr trackers );

	~TrackerSerializer( void );

	/// Read the xml data from a string
	void parseTrackers( std::string const &data );

	/// Read the xml data from a memory resource
	void parseTrackers( vl::Resource &tracking_data );

private :
	bool readXML( char *xml_data );

	/// Process the root node, the clients node
	void processClients( rapidxml::xml_node<>* XMLRoot );

	/// Process a client node
	void processClient( rapidxml::xml_node<>* XMLNode );

	/// Process a tracker node
	void processTracker( rapidxml::xml_node<>* XMLNode, Connection const &connection );

	/// Process a sensor node
	void processSensor( rapidxml::xml_node<>* XMLNode, TrackerRefPtr tracker );

	/// Process a trigger node
	void processTrigger( rapidxml::xml_node<>* XMLNode, SensorRefPtr sensor );

	void processDefault( rapidxml::xml_node<>* XMLNode, SensorRefPtr sensor );

	/// Config where the xml data is deserialized into
	ClientsRefPtr _clients;

};	// class TrackerSerializer

}	// namespace vl

#endif	// VL_TRACKER_SERIALIZER_HPP
