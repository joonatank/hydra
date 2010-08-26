#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE udp_conf_parser

#include <boost/test/unit_test.hpp>

#include "base/rapidxml_print.hpp"

#include "udp/udp_conf.hpp"

struct null_deleter
{
    void operator()(void const *) const
    {
    }
};

struct ConfFixture
{
	ConfFixture( void )
	{
		boost::shared_ptr<vl::udp::UdpConf> conf_ptr( &conf, null_deleter() );
		ser.reset( new vl::udp::UdpConfSerializer( conf_ptr ) );
		
		server = doc.allocate_node(rapidxml::node_element, "server" );
		doc.append_node(server);
	}

	void readXML( void )
	{
		xml_data.clear();
		rapidxml::print(std::back_inserter(xml_data), doc, 0);
		// Print to string so we can use SettingsSerializer for the data
		BOOST_TEST_MESSAGE( xml_data );
		
		char *data = ::strdup( xml_data.c_str() );
		ser->readData( data );
	}

	rapidxml::xml_node<> *createPort( std::string const &port_num )
	{
		char *c_port_num = doc.allocate_string( port_num.c_str() );

		rapidxml::xml_node<> *port
			= doc.allocate_node(rapidxml::node_element, "port", c_port_num );
		server->append_node(port);

		return server;
	}


	vl::udp::UdpConf conf;
	boost::shared_ptr<vl::udp::UdpConfSerializer> ser;

	// String containing the whole xml file, used as an input for the parser
	std::string xml_data;
	// The actual xml document
	rapidxml::xml_document<> doc;
	// Root node
	rapidxml::xml_node<> *server;
};

BOOST_FIXTURE_TEST_SUITE( UdpConfig, ConfFixture )

BOOST_AUTO_TEST_CASE( server )
{

}

BOOST_AUTO_TEST_CASE( port )
{
	
}

BOOST_AUTO_TEST_CASE( packet )
{

}

BOOST_AUTO_TEST_SUITE_END()