/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *	2011-01 Added support for command-line parameters
 *			Input file support, basic no interpolation
 *			Removed hard-coded Mevea and glasses trackers
 *			Supports only single tracker anymore
 *
 *	Dummy tracking server that sends position and orientation data from input file
 *	Used for testing.
 *
 *	Supports single tracker with single sensor.
 *	Tracker name and port are configurable.
 *	Supports reading an input file where the transformation is defined based on time.
 *
 *	TODO tracker address does not yet work, bind to localhost for now
 *	TODO add reset flag that starts the sequence (read from file) over again
 *
 *	Program Parameters :
 *	Can appear only once, configuration params
 *	-P port
 *	-A address (localhost, ip address, dns name)
 *	-I input-file (file describing the values for every frame)
 *	name (unnamed parameter), server name to start for example glassess or Mevea
 *
 * Input file
 * time\t sensor\t vector (x,y,z) \t quaternion (w, x, y, z)
 * Where vector is the position and quaternion is the rotation of the object
 * vector is in meters, quaternion is w,x,y,z quaternion
 * time is the time before moving to the next element, used for interpolation
 * time is in seconds and a double
 *
 * example:
 * # This is a comment
 * 2 \t 0 \t 0,1,2 \t 1,0,0,0
 * 2 \t 1 \t 0,1,2 \t 1,0,0,0
 * 10 \t 0 \t 0,0,1 \t 0,1,0,0
 * 10 \t 1 \t 0,0,2 \t 1,0,0,0
 */

#include "math/math.hpp"
#include "math/conversion.hpp"
#include "base/filesystem.hpp"
#include "base/sleep.hpp"

#include <vrpn_Tracker.h>

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

struct sensor_elem
{
	sensor_elem(void)
		: _last_index(0)
	{}

	void push_back(vl::Transform const &t)
	{
		transforms.push_back(std::make_pair(0, t) );
	}

	void push_back(double time, vl::Transform const &t)
	{
		transforms.push_back(std::make_pair(time, t) );
	}

	vl::Transform const &getOutput(double time)
	{
		double start = transforms.at(_last_index).first;
		while( time > start )
		{
			// TODO this should interpolate
			++_last_index;
			if( _last_index == transforms.size() )
			{ _last_index--; break; }

			start = transforms.at(_last_index).first;
			// Interpolate
			/* TODO implement
			double stop = _output.at(_last_index).first;
			if( t < stop )
			{
				double progress = (stop - start)/(t - start);
			}
			*/
		}
/*
		sensor_elem const &elem = _output.at(_last_index).second;
			std::cout << "Increasing value for sensor " << elem.sensor <<
				" : next value = " << elem.transform << std::endl;
*/
		
		assert( _last_index < transforms.size() );
		return transforms.at(_last_index).second;
	}

	/// Time - Transform pair
	std::vector< std::pair<double, vl::Transform> > transforms;

	size_t _last_index;
};

inline std::ostream &
operator<<(std::ostream &os, sensor_elem const &elem)
{
	for( size_t i = 0; i < elem.transforms.size(); ++i )
	{
		os << "time : " << elem.transforms.at(i).first 
			<< " transform : " << elem.transforms.at(i).second;
	}

	return os;
}

class Output
{
public :
	// Default values
	Output( void )
	{
		_output.resize(1);
		_output.at(0).push_back(vl::Transform( Ogre::Vector3(0, 1.5, 0) ));
	}

	Output( std::ifstream &input )
	{
		// TODO read the data
		if( !input.is_open() )
		{
			// TODO add exceptions
			std::cerr << "Output::Output file not opened." << std::endl;
			return;
		}

		while( input.good() )
		{
			std::string line;
			getline( input, line );
			// Parse line
			parse_line( line );
		}
	}

	void parse_line( std::string const &input_line )
	{
		// TODO add comment support, any line starting with #
		std::stringstream ss;
		std::string::size_type comment = input_line.find_first_of('#');
		
		ss.str(input_line.substr(0, comment) );

		if(ss.str().find_first_not_of(" \t\v\n") == std::string::npos )
		{
			std::cout << "Either a comment line or empty line, skipping" << std::endl;
			return;
		}

		double time = 0;
		int sensor = 0;
		ss >> time >> sensor;
		
		double x = 0, y = 0, z = 0;
		char ch;
		ss >> x >> ch >> y >> ch >> z;
		Ogre::Vector3 vec( x, y, z );

		std::cerr << "Time parsed : time = " << time << std::endl;
		std::cerr << "Sensor parsed : sensor = " << sensor << std::endl;
		std::cerr << "Vector parsed : vector = " << vec << std::endl;

		Ogre::Quaternion quat = Ogre::Quaternion::IDENTITY;
		if( !ss.str().empty() )
		{
			double w = 1;
			ss >> w >> ch >> x >> ch >> y >> ch >> z;
			quat = Ogre::Quaternion( w, x, y, z );
			std::cerr << "Quaternion parsed : quat = " << quat << std::endl;
		}

		// TODO this does not account for whitespace in the end of the line
		if(!ss.str().empty())
		{
			std::cerr << "Something fishy going on here, found another tabulator."
				<< std::endl;
		}

		// Add a new sensor
		if( _output.size() >= sensor )
		{
			_output.resize(sensor+1);
		}

		// For now time is stored as a double, in seconds
		// The playback program uses milliseconds
		_output.at(sensor).push_back( time/1000, vl::Transform( vec, quat ) );
	}

	void print( void ) const
	{
		std::cout << "Output : " << std::endl;
		for( size_t i = 0; i < _output.size(); ++i )
		{
			std::cout << "sensor " << i << " element : " << _output.at(i) << std::endl;
		}
	}

	/// @brief retrieve the Transformation for particular sensor at a particular time
	/// @param sensor which of the trackers sensors
	/// @param time time in milliseconds
	/// Get stored transformation for a sensor at a time
	vl::Transform const &getOutput(int sensor, double time)
	{
		/// @todo replace with error throwing
		assert( sensor < _output.size() );
		return _output.at(sensor).getOutput(time);
	}

	int nsensors(void) const
	{ return _output.size(); }

private :
	std::vector<sensor_elem> _output;
};	// class Output

class TrackerServer
{
public :
	// TODO address is not yet supported
	TrackerServer( void )
		: _port( 3883 ), _address( "localhost" ), _name("glasses"),
		_output(0), _connection(0), _tracker(0)
	{}

	~TrackerServer( void )
	{
		delete _output;
		delete _tracker;
		if( _connection )
		{ _connection->removeReference(); }
	}

	bool parse_options( int argc, char **argv )
	{
		// Declare the supported options.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("port,P", po::value<unsigned int>(), "VRPN server port.")
			("address,A", po::value<std::string>(), "VRPN server bind to address.")
			("input-file,I", po::value<std::string>(), "Input file for the values to send.")
			("name", po::value<std::string>(), "Name of the VRPN server.")
		;

		po::positional_options_description p;
		p.add("name", -1);

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
			_port = vm["port"].as<unsigned int>();
			std::cout << "Port was set to " << _port << "." << std::endl;
		}
		if( vm.count("address") )
		{
			_address = vm["address"].as<std::string>();
			std::cout << "Address was set to " << _address << "." << std::endl;
		}
		if( vm.count("input-file") )
		{
			_input_file = vm["input-file"].as<std::string>();
			std::cout << "Input file was set to " << _input_file << "." << std::endl;
			if( !fs::exists( _input_file ) )
			{
				std::cout << "No " << _input_file << " found." << std::endl;
				return false;
			}
		}
		if( vm.count("name") )
		{
			_name = vm["name"].as<std::string>();
			std::cout << "Name was set to "	<< _name << "." << std::endl;
		}

		return true;
	}

	void start( void )
	{
		std::cout << "Name " << _name << "." << std::endl;
		std::cout << "Address " << _address << "." << std::endl;
		std::cout << "Port " << _port << "." << std::endl;
		std::cout << "Input file " << _input_file << "." << std::endl;

		// TODO add support for address
		_connection = vrpn_create_server_connection( _port );

		// Set the output parameters
		if( !_input_file.empty() )
		{
			std::ifstream ifs( _input_file.c_str() );
			_output = new Output( ifs );
		}
		else
		{
			_output = new Output();
		}
		_time = 0;

		std::cout << "Creating vrpn tracker with " << _output->nsensors() << " sensors." << std::endl;
		_tracker = new vrpn_Tracker_Server(_name.c_str(), _connection, _output->nsensors() );
		std::cout << "VRPN tracker created." << std::endl;
	}

	void mainloop( void )
	{
		assert(_tracker);
		assert(_output);

		const int msecs = 8;
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = msecs*1e3;

		for( int sensor = 0; sensor < _output->nsensors(); ++sensor )
		{
			vl::Transform const &trans = _output->getOutput(sensor, _time);

			vrpn_float64 pos[3];
			vrpn_float64 quat[4];

			pos[Q_X] = trans.position.x;
			pos[Q_Y] = trans.position.y;
			pos[Q_Z] = trans.position.z;
			quat[Q_W] = trans.quaternion.w;
			quat[Q_X] = trans.quaternion.x;
			quat[Q_Y] = trans.quaternion.y;
			quat[Q_Z] = trans.quaternion.z;

			//		getHeadData(time, pos, quat);
			_tracker->report_pose(sensor, t, pos, quat);
		}

		_tracker->mainloop();
		_connection->mainloop();

		vl::msleep(msecs);
		_time += double(msecs);
	}

private :
	unsigned int _port;
	std::string _address;
	std::string _input_file;
	std::string _name;

	Output *_output;

	double _time;
	vrpn_Connection *_connection;
	vrpn_Tracker_Server *_tracker;

};	// class TrackerServer


int main (int argc, char **argv)
{
	TrackerServer tracker;
	// Failed to parse options, should have printed the error report already
	if( !tracker.parse_options(argc, argv) )
	{
		return 1;
	}

	tracker.start();
	// Loop forever calling the mainloop()s for all devices and the connection
	while(1)
	{
		tracker.mainloop();
	}

	return 0;
}
