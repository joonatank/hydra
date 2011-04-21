/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2011-04
 */

#include "vrpn_Tracker.hpp"

#include <vector>
#include <string>

#include <cassert>

#include <boost/program_options.hpp>

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

struct data_elem
{
	data_elem(const vrpn_Tracker_Pos pos, const vrpn_Tracker_Quat quat)
	{
		for( size_t i = 0; i < 3; ++i )
		{ position[i] = pos[i]; }

		for( size_t i = 0; i < 4; ++i )
		{ quaternion[i] = quat[i]; }
	}

	data_elem(void)
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

		sensors.at(sensor) = data_elem(pos, quat);
	}


	std::vector< data_elem > sensors;
};

std::ostream &
operator<<(std::ostream &os, data const &d)
{
	for( size_t i = 0; i < d.sensors.size(); ++i )
	{
		os << i << '\t';
		for( size_t j = 0; j < 3; ++j )
			os << d.sensors.at(i).position[j] << " ";
		os << '\t';
		for( size_t j = 0; j < 4; ++j )
			os << d.sensors.at(i).quaternion[j] << " ";
		os << std::endl;
	}

	return os;
}

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
		
		::Sleep(1);
	}

	assert(!opt.file.empty());
	std::ofstream file(opt.file);
	file << "# sensor\t position\t orientation" << std::endl
		<< "# delimeter is tabulator, sensor is integer, "
		<< "position is float64[3] and orientation is float64[4]" << std::endl;
	file << d;

	return 0;
}
