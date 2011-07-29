/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-07
 *	@file recording.cpp
 *
 *	This file is part of Hydra a VR game engine.
 *
 *	Recording of the the user interaction
 *	Can be loaded using ResourceManager and integrates to the clusters resource system.
 *
 *	At the moment only supports only VRPN data recordings.
 */

#include "recording.hpp"

vl::Recording::Recording(std::string const &name)
	: _name(name)
{
}

vl::Recording::~Recording(void)
{
}

void
vl::Recording::read(vl::Resource &res)
{
//	TextResource const &input = dynamic_cast<TextResource const &>(res);
	ResourceStream stream = res.getStream();
	while(!stream.eof())
	{
		_parse_line(stream.getLine());
	}
}

void
vl::Recording::_parse_line(std::string const &input_line)
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
	if(sensors.size() >= sensor)
	{
		sensors.resize(sensor+1);
	}

	// For now time is stored as a double, in seconds
	sensors.at(sensor).transforms[vl::time(t)] = vl::Transform(vec, quat);
}


std::ostream &
vl::operator<<(std::ostream &os, vl::Recording const &rec)
{
	for(size_t i = 0; i < rec.sensors.size(); ++i)
	{
		vl::Recording::sensor const &sensor= rec.sensors.at(i);
		
		for(std::map<vl::time, vl::Transform>::const_iterator t_iter = sensor.transforms.begin();
			t_iter != sensor.transforms.end(); ++t_iter)
		{
			os << (double)t_iter->first << '\t'
				<< i << '\t';
			Transform const &t = t_iter->second;
			os << t.position.x << "," << t.position.y << "," << t.position.z << '\t';
			os << t.quaternion.w << "," << t.quaternion.x << "," 
				<< t.quaternion.y << "," << t.quaternion.z;

			os << std::endl;
		}
	}

	return os;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::Recording const &rec)
{
	msg << rec.sensors.size();
	for(size_t i = 0; i < rec.sensors.size(); ++i)
	{
		vl::Recording::sensor const &sensor= rec.sensors.at(i);

		msg << sensor.transforms.size();
		for(std::map<vl::time, vl::Transform>::const_iterator t_iter = sensor.transforms.begin();
			t_iter != sensor.transforms.end(); ++t_iter)
		{
			msg << t_iter->first << t_iter->second;
		}
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::Recording &rec)
{
	size_t size;
	msg >> size;
	rec.sensors.resize(size);
	for(size_t i = 0; i < rec.sensors.size(); ++i)
	{
		vl::Recording::sensor &sensor= rec.sensors.at(i);
		
		msg >> size;
		//sensor.transforms.reserve(size);
		for(size_t j = 0; j < size; ++j)
		{
			vl::time time;
			Transform trans;
			msg >> time >> trans;
			sensor.transforms[time] = trans;
		}
	}

	return msg;
}
