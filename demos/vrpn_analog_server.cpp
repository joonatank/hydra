/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-09
 */

#include "base/time.hpp"
#include "base/sleep.hpp"

#include <vrpn_Analog.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

struct AnalogServer
{
	AnalogServer(void)
		: _port(3883)
		, _name("Analog")
		, _connection(0)
		, _vrpn_server(0)
		, _value(0)
	{}

	bool parse_options( int argc, char **argv )
	{
		// Declare the supported options.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("port,P", po::value<int>(), "VRPN server port.")
			("name", po::value<std::string>(), "Name of the VRPN server.")
			("value", po::value<double>(), "Value to send.")
		;

		po::positional_options_description p;
		p.add("value", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				  options(desc).positional(p).run(), vm);
		po::notify(vm);

		if( vm.count("help") )
		{
			std::cout << desc << std::endl;
			return false;
		}

		if( vm.count("port") )
		{
			_port = vm["port"].as<int>();
		}
		if( vm.count("value") )
		{
			_value = vm["value"].as<double>();
		}
		if( vm.count("name") )
		{
			_name = vm["name"].as<std::string>();
		}

		return true;
	}

	void start(void)
	{
		std::cout << "Starting server : " << _name << " in port " << _port
			<< " with analog value = " << _value << std::endl;
		_connection = vrpn_create_server_connection( _port );
		_vrpn_server = new vrpn_Analog_Server(_name.c_str(), _connection);
		_vrpn_server->channels()[0] = _value;
	}

	void mainloop(void)
	{
		assert(_connection && _vrpn_server);
		// Needs to call report at every frame because otherwise newly connected
		// client things there is no values in the server.
		// what a dumb fuck.
		_vrpn_server->report();
		_vrpn_server->mainloop();
		_connection->mainloop();
	}

	std::string _name;
	double _value;
	int _port;
	vrpn_Analog_Server *_vrpn_server;
	vrpn_Connection *_connection;

};	// class AnalogServer

int main (int argc, char **argv)
{
	AnalogServer server;
	if(!server.parse_options(argc, argv))
	{ return 0; }

	server.start();

	while(true)
	{
		server.mainloop();
		// Needs to be larger than 1ms because otherwise client will go into
		// infinite loop O_o
		// dunno how much larger though
		vl::msleep((uint32_t)100);
	}
}
