/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-04
 *
 *	@update 2011-07 - Extended for recording large sequenzes of VRPN data
 */

/// @todo add tracker element with time support

#include "base/chrono.hpp"
#include "base/sleep.hpp"

#include <vrpn_Tracker.h>
#include <quat.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

#include <cassert>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

struct options
{
	options(void)
		: file("default.log")
	{}

	bool parseOptions( int argc, char **argv )
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce a help message")
			("tracker,t",po::value<std::string>(), "tracker name to connect to, Tracker@host:port")
			("file,f", po::value<std::string>(&file)->default_value("vrpn_record.log"), "file to log")
			("samples,s", po::value<int>(&samples)->default_value(0), "Samples to gather before exiting. Use zero for inifinite.")
			("frequenzy", po::value<double>(&frequenzy)->default_value(60.0), "The frequenzy which to gather samples. Valid values greater than zero.")
		;

		// Parse command line
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Print help
		if( vm.count("help") )
		{
			std::cout << "Help : " << desc << "\n";
			return false;
		}

		if( vm.count("tracker") )
		{
			tracker = vm["tracker"].as<std::string>();
		}

		if( vm.count("file") )
		{
			file = vm["file"].as<std::string>();
		}

		if( vm.count("samples") )
		{
			samples = vm["samples"].as<int>();
		}

		if( vm.count("frequenzy") )
		{
			frequenzy = vm["frequenzy"].as<double>();
		}

		// Needs to be greater than zero
		frequenzy = frequenzy > 0 ? frequenzy : 1.0;

		return true;
	}

	std::string tracker;
	std::string file;
	int samples;
	double frequenzy;
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

vl::chrono g_timer;

std::ostream &
operator<<(std::ostream &os, data const &d)
{
	for( size_t i = 0; i < d.sensors.size(); ++i )
	{
		os << (double)g_timer.elapsed() << '\t'
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

void mainloop(vrpn_Tracker_Remote *tkr, uint32_t sleep_time)
{
	// Purge all of the old reports
	tkr->mainloop();

	vl::msleep(sleep_time);
}

int main(int argc, char **argv)
{
	options opt;
	if(!opt.parseOptions(argc, argv))
	{ return 0; }
	std::cout << "Options parsed." << std::endl;

	if( opt.tracker.empty() )
	{
		std::cout << "Tracker name can not be empty." << std::endl;
		return -1;
	}

	if(opt.file.empty())
	{
		std::cout << "Output file can not be empty." << std::endl;
		return -1;
	}

	std::cout << "Connecting to tracker " << opt.tracker << std::endl;
	vrpn_Tracker_Remote* tkr = new vrpn_Tracker_Remote(opt.tracker.c_str());

	// Set up the tracker callback handler
	data d;
	tkr->register_change_handler((void *)&d, handle_tracker);

	std::ofstream file(opt.file.c_str());
	if(!file.is_open())
	{
		std::cerr << "Couldn't open file: " << opt.file << " for writing." << std::endl;
		return -1;
	}

	// Write the header for the file
	file << "# time\t sensor\t position\t orientation" << std::endl
		<< "# delimeter is tabulator, vector elements are separated with ','" << std::endl
		<< "# time is a floating point, the absolute program time in seconds" << std::endl
		<< "# sensor is an integer" << std::endl 
		<< "# position is (x, y, z) vector in meters" << std::endl
		<< "# orientation (w, x, y, z) quaternion" << std::endl;

	g_timer.reset();

	// Sleep time in milliseconds
	uint32_t sleep_time = 1;
	double st = (double)(1000)/opt.frequenzy;
	if(st < 0)
	{ std::cerr << "ERROR: can not sleep negative time!" << std::endl; }
	else
	{ sleep_time = (uint32_t)st; }

	if(opt.samples > 0)
	{
		int count = 0;
		while(count < opt.samples) 
		{
			mainloop(tkr, sleep_time);
			file << d;
		}
	}
	// Infinite loop
	else
	{
		while(true)
		{
			mainloop(tkr, sleep_time);
			file << d;
		}
	}

	delete tkr;

	return 0;
}
