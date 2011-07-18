/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-04
 */

/// @todo add tracker element with time support

#include <vrpn_Tracker.h>
#include <quat.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <cassert>

#include <boost/program_options.hpp>

#ifndef _WIN32
#include <time.h>
#endif

namespace po = boost::program_options;

struct options
{
	options(void)
		: file("default.log")
	{}

	void parseOptions( int argc, char **argv )
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce a help message")
			("tracker,t",po::value<std::string>(), "tracker name to connect to, Tracker@host:port")
			("file,f", po::value<std::string>(), "file to log")
		;

		// Parse command line
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Print help
		if( vm.count("help") )
		{
			std::cout << "Help : " << desc << "\n";
			return;
		}

		if( vm.count("tracker") )
		{
			tracker = vm["tracker"].as<std::string>();
		}

		if( vm.count("file") )
		{
			file = vm["file"].as<std::string>();
		}
	}

	std::string tracker;
	std::string file;
};

struct sensor_elem
{
	sensor_elem(const vrpn_Tracker_Pos pos, const vrpn_Tracker_Quat quat)
	{
		for( size_t i = 0; i < 3; ++i )
		{ position[i] = pos[i]; }

		for( size_t i = 0; i < 4; ++i )
		{ quaternion[i] = quat[i]; }
	}

	sensor_elem(void)
	{
		for( size_t i = 0; i < 3; ++i )
		{ position[i] = 0; }

		for( size_t i = 0; i < 4; ++i )
		{ quaternion[i] = 0; }
	}

	vrpn_Tracker_Pos position;
	vrpn_Tracker_Quat quaternion;
};

struct data
{
	void add_sensor_data(vrpn_int32 sensor, const vrpn_Tracker_Pos pos, const vrpn_Tracker_Quat quat)
	{
		if( sensors.size() < sensor+1 )
		{ sensors.resize(sensor+1); }

		sensors.at(sensor) = sensor_elem(pos, quat);
	}

	std::vector<sensor_elem> sensors;
};

std::ostream &
operator<<(std::ostream &os, data const &d)
{
	for( size_t i = 0; i < d.sensors.size(); ++i )
	{
		// Print zero time for all now
		os << 0 << '\t'
			<< i << '\t';
		sensor_elem const &elem = d.sensors.at(i);
		os << elem.position[Q_X] << "," << elem.position[Q_Y] << "," << elem.position[Q_Z] 
			<< '\t';
		os << elem.quaternion[Q_W] << "," << elem.quaternion[Q_X] << "," 
			<< elem.quaternion[Q_Y] << "," << elem.quaternion[Q_Z];

		os << std::endl;
	}

	return os;
}

/// @brief VRPN callback handler
void handle_tracker(void *userdata, const vrpn_TRACKERCB t)
{
	assert(userdata);
	data *d = (data *)userdata;

	d->add_sensor_data(t.sensor, t.pos, t.quat);
}

int main(int argc, char **argv)
{
	options opt;
	opt.parseOptions(argc, argv);

	// Open the tracker

	if( opt.tracker.empty() )
	{
		std::cout << "Tracker name can not be empty." << std::endl;
		return 0;
	}

	std::cout << "Connecting to tracker " << opt.tracker << std::endl;
	vrpn_Tracker_Remote* tkr = new vrpn_Tracker_Remote(opt.tracker.c_str());

	// Set up the tracker callback handler
	data d;
	tkr->register_change_handler((void *)&d, handle_tracker);

	bool done = false;
	while(!done) 
	{
		// Purge all of the old reports
		tkr->mainloop();
		
		// Just the first input
		if( !d.sensors.empty() )
		{ done = true; }
		
#ifdef _WIN32
		::Sleep(1);
#else
		timespec tv;
		tv.tv_sec = 0;
		tv.tv_nsec = 1e6;
		::nanosleep( &tv, 0 );
#endif
	}

	assert(!opt.file.empty());
	std::ofstream file(opt.file.c_str());
	file << "# time\t sensor\t position\t orientation" << std::endl
		<< "# delimeter is tabulator, vector elements are separated with ','"
		<< "# sensor is an integer" << std::endl 
		<< "# position is (x, y, z) vector" << std::endl
		<< "# orientation (w, x, y, z) quaternion" << std::endl;

	file << d;

	return 0;
}
