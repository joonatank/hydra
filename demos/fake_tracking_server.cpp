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
 * vector \t quaternion \t time
 * Where vector is the position and quaternion is the rotation of the object
 * vector is in meters, quaternion is w,x,y,z quaternion
 * time is the time before moving to the next element, used for interpolation
 * time is in seconds and a double
 *
 * example:
 * 2 \t 0,1,2 \t 1,0,0,0
 * 10 \t 0,0,1 \t 0,1,0,0
 */

#include "base/sleep.hpp"
#include "vrpn_Tracker.h"
#include "math/math.hpp"
#include "math/conversion.hpp"
#include "base/filesystem.hpp"

#include <iostream>

// For MSV compilers to get M_PI
#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

class Output
{
public :
	// Default values
	Output( void )
		: _last_index(0)
	{
		_output.push_back( std::make_pair(1, vl::Transform( Ogre::Vector3(0, 1.5, 0) ) ) );
	}

	Output( std::ifstream &input )
		: _last_index(0)
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

	void parse_line( std::string const &line )
	{
		// TODO add comment support, any line starting with #

		std::string::size_type pos = 0;
		std::string::size_type end_pos = line.find_first_of('\t', pos);
		
		std::stringstream t( line.substr(pos, end_pos) );
		double time = 0;
		t >> time;

		std::cerr << "Time parsed : time = " << time << std::endl;
		if( end_pos == std::string::npos )
		{
			return;
		}
		
		pos = end_pos+1;
		end_pos = line.find_first_of('\t', pos);
		
		std::stringstream v(line.substr(pos, end_pos));
		double x = 0, y = 0, z = 0;
		char ch;
		v >> x >> ch >> y >> ch >> z;
		Ogre::Vector3 vec( x, y, z );
		std::cerr << "Vector parsed : vector = " << vec << std::endl;
		if( end_pos == std::string::npos )
		{
			// TODO this should not return
			// The quaternion can be IDENTITY by default
			return;
		}

		pos = end_pos+1;
		end_pos = line.find_first_of('\t', pos);
		
		std::stringstream q(line.substr(pos, end_pos));
		double w = 0;
		q >> w >> ch >> x >> ch >> y >> ch >> z;
		Ogre::Quaternion quat( w, x, y, z );

		std::cerr << "Quaternion parsed : quat = " << quat << std::endl;
		if( end_pos != std::string::npos )
		{
			std::cerr << "Something fishy going on here, found another tabulator." 
				<< std::endl;
		}
		_output.push_back( std::make_pair(time, vl::Transform( vec, quat ) ) );
	}

	void print( void ) const
	{
		std::cerr << "Output : " << std::endl;
		for( size_t i = 0; i < _output.size(); ++i )
		{
			std::cerr << "time = " << _output.at(i).first << " value " 
				<< _output.at(i).second << std::endl;
		}
	}

	vl::Transform const &getOutput( double ms )
	{
		double t = ms/1000;
		double start = _output.at(_last_index).first; 
		while( t > start )
		{
			// TODO this should interpolate
			++_last_index;
			if( _last_index == _output.size() )
			{ _last_index--; break; }
			
			start = _output.at(_last_index).first;
			// Interpolate
			/* TODO implement
			double stop = _output.at(_last_index).first;
			if( t < stop )
			{
				double progress = (stop - start)/(t - start); 
			}
			*/
		}

		return _output.at(_last_index).second;
	}

private :
	size_t _last_index;
	std::vector< std::pair<double, vl::Transform> > _output;
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
		_tracker = new vrpn_Tracker_Server(_name.c_str(), _connection );

		// Set the output parameters
		if( !_input_file.empty() )
		{
			std::ifstream ifs( _input_file );
			_output = new Output( ifs );
		}
		else
		{
			_output = new Output();
		}
		_time = 0;
	}

	void mainloop( void )
	{
		const int msecs = 8;
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = msecs*1e3;

		vl::Transform const &trans = _output->getOutput(_time);

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
		_tracker->report_pose( 0, t, pos, quat );

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
