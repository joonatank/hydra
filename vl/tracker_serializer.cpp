/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-10
 */

#include "tracker_serializer.hpp"

#include "base/string_utils.hpp"
#include "vrpn_tracker.hpp"
#include "ogre_xml_helpers.hpp"

#include "event_manager.hpp"


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
vl::TrackerSerializer::parseTrackers( vl::TextResource &tracking_data )
{
	char *xml_data = tracking_data.get();

	readXML( xml_data );
}


/// ---------- Private ------------
void
vl::TrackerSerializer::readXML( char *xml_data )
{
	rapidxml::xml_document<> xmlDoc;
	xmlDoc.parse<0>( xml_data );

	rapidxml::xml_node<> *xmlRoot = xmlDoc.first_node("clients");
	if( !xmlRoot )
	{ BOOST_THROW_EXCEPTION( vl::invalid_tracking() ); }

	processClients( xmlRoot );
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

	rapidxml::xml_node<> *elem = XMLNode->first_node("transformation");
	if( elem )
	{
		Ogre::Matrix4 trans = Ogre::Matrix4::IDENTITY;
		processTransformation( elem, trans );
		tracker->setTransformation( trans );
	}

	std::cerr << "Finding sensors" << std::endl;
	elem = XMLNode->first_node("sensor");
	while( elem )
	{
		processSensor( elem, tracker );
		elem = elem->next_sibling("sensor");
	}
}

void
vl::TrackerSerializer::processTransformation( rapidxml::xml_node<>* XMLNode, Ogre::Matrix4 &trans )
{
	Ogre::Matrix4 m = Ogre::Matrix4::IDENTITY;

	rapidxml::xml_node<> *elem = XMLNode->first_node("sign");
	if( elem )
	{
		Ogre::Vector3 v = parseVector3( elem );
		m.setScale(v);
	}

	Ogre::Quaternion q = Ogre::Quaternion::IDENTITY;
	elem = XMLNode->first_node("quaternion");
	if( elem )
	{ q = parseQuaternion( elem ); }

	Ogre::Vector3 v = Ogre::Vector3::ZERO;
	elem = XMLNode->first_node("vector");
	if( elem )
	{ v = parseVector3( elem ); }

	Ogre::Matrix4 m2(q);
	m2.setTrans(v);

	// Update the original matrix
	trans = m2 * m * trans;
	elem = XMLNode->first_node("transformation");
	if( elem )
	{ processTransformation( elem, trans ); }
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

	// TODO should use Ogre LogManager
	// Needs to be created in Config
	std::cerr << "Creating Trigger " << name << std::endl;
	vl::TrackerTrigger *trigger = _clients->getEventManager()->createTrackerTrigger(name);
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
