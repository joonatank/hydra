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
  *	TODO add reset flag that starts the sequence (read from file) over again
 *
 *	Program Parameters :
 *	Can appear only once, configuration params
 *	-P port
 *	-I input-file (file describing the values for every frame)
 *	name (unnamed parameter), server name to start for example glassess or Mevea
 *
 * Input file
 * # time\t sensor\t position\t orientation" << std::endl
 * # delimeter is tabulator (\t), vector elements are separated with ','
 * # time is a floating point, the absolute program time in seconds
 * # sensor is an integer
 * # position is (x, y, z) vector in meters
 * # orientation (w, x, y, z) quaternion
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
#include "base/chrono.hpp"

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

	vl::Transform const &getOutput(vl::time time)
	{
		vl::time start = transforms.at(_last_index).first;
		while( time > start )
		{
			// TODO this should interpolate
			++_last_index;
			if( _last_index == transforms.size() )
			{ _last_index--; break; }

			start = transforms.at(_last_index).first;
			// Interpolate
		}
		
		assert( _last_index < transforms.size() );
		return transforms.at(_last_index).second;
	}

	/// Time - Transform pair
	std::vector< std::pair<vl::time, vl::Transform> > transforms;

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
		std::stringstream ss;
		std::string::size_type comment = input_line.find_first_of('#');
		
		ss.str(input_line.substr(0, comment) );

		// Either a comment line or an empty line skip
		if(ss.str().find_first_not_of(" \t\v\n") == std::string::npos )
		{ return; }

		double t = 0;
		int sensor = 0;
		ss >> t >> sensor;
		
		double x = 0, y = 0, z = 0;
		char ch;
		ss >> x >> ch >> y >> ch >> z;
		Ogre::Vector3 vec( x, y, z );

		Ogre::Quaternion quat = Ogre::Quaternion::IDENTITY;
		if( !ss.str().empty() )
		{
			double w = 1;
			ss >> w >> ch >> x >> ch >> y >> ch >> z;
			quat = Ogre::Quaternion( w, x, y, z );
		}

		if(ss.str().find_first_not_of(" \t\v\n") == std::string::npos)
		{
			std::cerr << "Something fishy on line \"" << input_line 
				<< "\" still remaining : \"" << ss.str() << "\"." << std::endl;
		}

		// Add a new sensor
		if( _output.size() >= sensor )
		{
			_output.resize(sensor+1);
		}

		// For now time is stored as a double, in seconds
		_output.at(sensor).push_back( vl::time(t), vl::Transform( vec, quat ) );
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
	/// @param time current time for the output
	/// Get stored transformation for a sensor at a time
	vl::Transform const &getOutput(int sensor, vl::time time)
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
	TrackerServer( void )
		: _port( 3883 ), _name("glasses"),
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
			("help,h", "produce help message")
			("port,P", po::value<int>(), "VRPN server port.")
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
			_port = vm["port"].as<int>();
		}
		if( vm.count("input-file") )
		{
			_input_file = vm["input-file"].as<std::string>();
			if( !fs::exists( _input_file ) )
			{
				std::cout << "No " << _input_file << " found." << std::endl;
				return false;
			}
		}
		if( vm.count("name") )
		{
			_name = vm["name"].as<std::string>();
		}

		return true;
	}

	void start( void )
	{
		std::cout << "Name " << _name << "." << std::endl;
		std::cout << "Port " << _port << "." << std::endl;
		std::cout << "Input file " << _input_file << "." << std::endl;

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

		std::cout << "Creating vrpn tracker with " << _output->nsensors() << " sensors." << std::endl;
		_tracker = new vrpn_Tracker_Server(_name.c_str(), _connection, _output->nsensors() );
		std::cout << "VRPN tracker created." << std::endl;

		_timer.reset();
	}

	void mainloop( void )
	{
		assert(_tracker);
		assert(_output);

		// Hard coded timeval
		// @todo should be configured from command-line
		const uint32_t msecs = 8;
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = msecs*1e3;

		for(int sensor = 0; sensor < _output->nsensors(); ++sensor)
		{
			vl::Transform const &trans = _output->getOutput(sensor, _timer.elapsed());

			vrpn_float64 pos[3];
			vrpn_float64 quat[4];

			pos[Q_X] = trans.position.x;
			pos[Q_Y] = trans.position.y;
			pos[Q_Z] = trans.position.z;
			quat[Q_W] = trans.quaternion.w;
			quat[Q_X] = trans.quaternion.x;
			quat[Q_Y] = trans.quaternion.y;
			quat[Q_Z] = trans.quaternion.z;

			_tracker->report_pose(sensor, t, pos, quat);
		}

		_tracker->mainloop();
		_connection->mainloop();

		vl::msleep(msecs);
	}

private :
	int _port;
	std::string _input_file;
	std::string _name;

	Output *_output;

	vl::chrono _timer;
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
