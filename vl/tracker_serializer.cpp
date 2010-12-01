
#include "tracker_serializer.hpp"

#include "base/string_utils.hpp"
#include "vrpn_tracker.hpp"
#include "ogre_xml_helpers.hpp"

/// ---------- Public ----------

vl::TrackerSerializer::TrackerSerializer(vl::ClientsRefPtr clients)
	: _clients( clients )
{
	if( !_clients )
	{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }
}

vl::TrackerSerializer::~TrackerSerializer(void )
{}

void
vl::TrackerSerializer::parseTrackers( const std::string& data )
{
	size_t length = data.length() + 1;
	char *xml_data = new char[length];
	::memcpy( xml_data, data.c_str(), length);

	bool retval = readXML( xml_data );

	delete [] xml_data;

	if( retval )
	{ BOOST_THROW_EXCEPTION( vl::invalid_tracking() ); }
}

void
vl::TrackerSerializer::parseTrackers( vl::Resource &tracking_data )
{
	// Get the ownership of the Resource data
	vl::MemoryBlock mem = tracking_data.release();
	size_t size = mem.size;
	char *xml_data = mem.mem;

	// We replace the EOF with Null Terminator for text files
	xml_data[size-1] = '\0';

	if( ::strlen( xml_data ) != size-1 )
	{
		delete [] xml_data;
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc("MemoryBlock has invalid XML file") );
	}

	bool retval = readXML( xml_data );

	delete [] xml_data;

	if( retval )
	{ BOOST_THROW_EXCEPTION( vl::invalid_tracking() ); }
}


/// ---------- Private ------------
bool
vl::TrackerSerializer::readXML( char *xml_data )
{
	rapidxml::xml_document<> xmlDoc;
	xmlDoc.parse<0>( xml_data );

	rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("clients");
	if( !xmlRoot )
	{
		return false;
	}

	processClients( xmlRoot );

	return true;
}

void
vl::TrackerSerializer::processClients(rapidxml::xml_node< char >* XMLRoot)
{
	rapidxml::xml_node<> *elem = XMLRoot->first_node("client");

	while( elem )
	{
		processClient( elem );
		elem = elem->next_sibling("client");
	}
}

void
vl::TrackerSerializer::processClient(rapidxml::xml_node< char >* XMLNode)
{
	// use defaults to true
	bool use = true;
	rapidxml::xml_attribute<>* attrib = XMLNode->first_attribute("use");
	if( attrib )
	{ use = vl::from_string<bool>( std::string( attrib->value() ) ); }

	std::cerr << "use attrib done" << std::endl;

	std::string type;
	attrib = XMLNode->first_attribute("type");
	// must have a type attribute
	if( attrib )
	{ type = attrib->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }
	// Only VRPN type is supported at the moment
	if( type != "VRPN" )
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	std::string host;
	attrib = XMLNode->first_attribute("host");
	// must have a host attribute
	if( attrib )
	{ host = attrib->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }
	if( host.empty() )
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	uint16_t port = 0;
	attrib = XMLNode->first_attribute("port");
	if( attrib )
	{ port = vl::from_string<uint16_t>( std::string( attrib->value() ) ); }

	Connection connection( host, port );
	rapidxml::xml_node<> *elem = XMLNode->first_node("tracker");
	while( elem )
	{
		processTracker( elem, connection );
		elem = elem->next_sibling("tracker");
	}

}

void
vl::TrackerSerializer::processTracker( rapidxml::xml_node< char >* XMLNode,
									   Connection const &connection)
{
	std::string name;
	rapidxml::xml_attribute<> *attrib = XMLNode->first_attribute("name");
	if( attrib )
	{ name = attrib->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	TrackerRefPtr tracker( new vrpnTracker( connection.hostname, name, connection.port ) );
	_clients->addTracker(tracker);

	std::cerr << "Finding sensors" << std::endl;
	rapidxml::xml_node<> *elem = XMLNode->first_node("sensor");
	while( elem )
	{
		processSensor( elem, tracker );
		elem = elem->next_sibling("sensor");
	}
}

void
vl::TrackerSerializer::processSensor( rapidxml::xml_node< char >* XMLNode,
									  TrackerRefPtr tracker )
{
	uint16_t num;
	rapidxml::xml_attribute<> *attrib = XMLNode->first_attribute("num");
	if( attrib )
	{ num = vl::from_string<uint16_t>( attrib->value() ); }
	else
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	vl::SensorRefPtr sensor( new vl::Sensor );
	tracker->setSensor(num, sensor);

	rapidxml::xml_node<> *elem = XMLNode->first_node("trigger");
	if( elem )
	{ processTrigger(elem, sensor); }
	else
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	// Get extra nodes
	elem = XMLNode->first_node("default");
	if( elem )
	{ processDefault(elem, sensor); }
}

void
vl::TrackerSerializer::processTrigger( rapidxml::xml_node< char >* XMLNode,
									   SensorRefPtr sensor )
{
	std::string name;
	rapidxml::xml_attribute<> *attrib = XMLNode->first_attribute("name");
	if( attrib )
	{ name = attrib->value(); }
	else
	{ BOOST_THROW_EXCEPTION( vl::exception() ); }

	std::cerr << "Creating Trigger " << name << std::endl;
	vl::TrackerTrigger *trigger = new vl::TrackerTrigger;
	trigger->setName(name);
	sensor->setTrigger(trigger);
}

void
vl::TrackerSerializer::processDefault( rapidxml::xml_node< char >* XMLNode,
									   SensorRefPtr sensor )
{
	rapidxml::xml_node<> *elem = XMLNode->first_node("orientation");
	if( elem )
	{
		Ogre::Quaternion q = vl::parseQuaternion(elem);
		sensor->setDefaultOrientation(q);
	}

	elem = XMLNode->first_node("position");
	if( elem )
	{
		Ogre::Vector3 v = vl::parseVector3(elem);
		sensor->setDefaultPosition(v);
	}
}