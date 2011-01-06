/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	2010-11-29 Added camera rotations to env file
 *	moved ref ptr definition to typedefs.hpp
 *
 *	2011-01-06 Added wall and window to env file
 *	Added stereo and inter pupilar distance to env file
 */

#ifndef VL_ENVSETTINGS_HPP
#define VL_ENVSETTINGS_HPP

#include <string>
#include <vector>
#include <iostream>

#include <stdint.h>

#include "filesystem.hpp"
#include "rapidxml.hpp"
#include "typedefs.hpp"
#include "exceptions.hpp"

namespace vl
{

class EnvSettings
{
public :
	struct Tracking
	{
		Tracking( std::string const &file_name, bool u = "true" )
			: file(file_name), use(u)
		{}

		std::string file;
		bool use;
	};

	struct Wall
	{
		Wall( std::string const &nam, std::string const &channel_nam,
			  std::vector<double> b_left, std::vector<double> b_right, 
			  std::vector<double> t_left )
			: name(nam), channel_name(channel_nam), bottom_left(b_left),
			  bottom_right(b_right), top_left(t_left)
		{}

		// Default constructor to allow vector resize
		Wall( void )
		{}

		// Wether or not the Wall has been initialised
		bool empty( void ) const
		{ 
			return( name.empty() && channel_name.empty() && bottom_left.empty() 
					&& bottom_right.empty() && top_left.empty() );
		}

		// Name of the wall
		std::string name;
		// Name of the channel which this wall is mapped to
		std::string channel_name;
		// Bottom left coordinates for this wall
		std::vector<double> bottom_left;
		// Bottom right coordinates for this wall
		std::vector<double> bottom_right;
		// Top left coordinates for this wall
		std::vector<double> top_left;

	};	// struct Wall

	struct Window
	{
		Window( std::string const &nam, int width, int height, int px, int py )
			: name(nam), w(width), h(height), x(px), y(py)
		{
			if( h < 0 || w < 0 || x < 0 || y < 0 )
			{
				BOOST_THROW_EXCEPTION( vl::invalid_settings() );
			}
		}
		
		// Default constructor to allow vector resize
		Window( void )
			: w(0), h(0), x(0), y(0)
		{}

		// Wether or not the Window has been initialised
		bool empty( void ) const
		{ 
			return( name.empty() && w == 0 && h == 0 && x == 0 && y == 0 );
		}

		// Name of the window
		std::string name;
		// Width of the window
		int w;
		// Height of the window
		int h;
		// x coordinate of the window
		int x;
		// y coordinate of the window
		int y;

	};	// struct Window

	/// Configurations tokens for boolean values
	enum CFG
	{
		OFF,
		ON,
		REQUIRED,
	};

	/// Constructor
	EnvSettings( void );

	/// Destructor
	~EnvSettings( void );

	/// Clear the settings structure to default values
	void clear( void );


	/// File path
	std::string const &getFile( void ) const
	{ return _file_path; }

	void setFile( std::string const &file )
	{ _file_path = file; }

	///// EQC /////////////////////////////////////////////////////
	/// get the equalizer config file
	std::string const &getEqc( void ) const
	{ return _eqc; }

	/// set the equalizer config file
	void setEqc( std::string const &e )
	{ _eqc = e; }

	/// Get the absoulute path to eqc file
	std::string getEqcFullPath( void ) const;

	std::string getPluginsDirFullPath( void ) const;

	///// PLUGINS /////////////////////////////////////////////////
	/// get vector containing plugin names and which are active
	std::vector<std::pair<std::string, bool> > const &getPlugins( void ) const
	{ return _plugins; }

	/// Add a plugin by name and boolean wether it's in use or not
	/// Checks that the same plugin is not added twice, NOP if the plugin is
	/// already in the plugins stack.
	void addPlugin( std::pair<std::string, bool> const &plugin );

	/// set the plugin named pluginName use to on or off
	/// returns true if plugin found
	/// prints an error message and returns false if no such plugin found
	bool pluginOnOff( std::string const &pluginName, bool newState );

	///// TRACKING /////////////////////////////////////////////////
	/// Returns a vector of tracking files
	std::vector<Tracking> const &getTracking( void ) const
	{ return _tracking; }

	/// Adds a tracking file to the tracking file stack
	/// Checks that the same file tracking file is not added twice, NOP if the
	/// tracking file is already in the stack.
	void addTracking( Tracking const &track );

	/// Remove a tracking file from the stack
	void removeTracking( std::string const &track );

	void removeTracking( Tracking const &track );

	/// Returns a flags of around which axes the camera rotations are allowed
	/// Fist bit is the x axis, second y axis, third z axis
	uint32_t getCameraRotationAllowed( void ) const
	{ return _camera_rotations_allowed; }

	void setCameraRotationAllowed( uint32_t const flags )
	{ _camera_rotations_allowed = flags; }

	void addWall( Wall const &wall );

	Wall const &getWall( size_t i ) const;

	size_t getNWalls( void ) const
	{ return _walls.size(); }

	std::vector<Wall> const &getWalls( void ) const
	{ return _walls; }

	void addWindow( Window const &window );

	Window const &getWindow( size_t i ) const;

	size_t getNWindows( void ) const
	{ return _windows.size(); }

	std::vector<Window> const &getWindows( void ) const
	{ return _windows; }

	void setStereo( CFG stereo )
	{ _stereo = stereo; }

	CFG getStereo( void ) const
	{ return _stereo; }

	void setIPD( double ipd )
	{ _ipd = ipd; }

	double getIPD( void ) const
	{ return _ipd; }

private :

	std::string _file_path;

	std::string _eqc;

	std::vector<std::pair<std::string, bool> > _plugins;
	std::vector<Tracking> _tracking;

	uint32_t _camera_rotations_allowed;

	std::vector<Wall> _walls;
	std::vector<Window> _windows;

	CFG _stereo;
	// Inter pupilar distance
	double _ipd;

};	// class EnvSettings





class EnvSettingsSerializer
{
public :
	/// Will completely over-ride the provided EnvSettings
	/// when XML data is processed.
	/// Any error in processing will result to defaulting EnvSettings.
	EnvSettingsSerializer( EnvSettingsRefPtr );

	~EnvSettingsSerializer( void );

	/// Read data from string buffer. Buffer is not modified.
	bool readString( std::string const &xml_data );


protected :
	bool readXML( void );

	void processConfig( rapidxml::xml_node<>* XMLNode );

	void processPlugins( rapidxml::xml_node<>* XMLNode );

	void processEqc( rapidxml::xml_node<>* XMLNode );

	void processTracking( rapidxml::xml_node<>* XMLNode );

	void processCameraRotations( rapidxml::xml_node<>* XMLNode );

	void processWalls( rapidxml::xml_node<>* XMLNode );

	void processWindows( rapidxml::xml_node<>* XMLNode );

	void processStereo( rapidxml::xml_node<>* XMLNode );

	void processIPD( rapidxml::xml_node<>* XMLNode );

	std::vector<double> getVector( rapidxml::xml_node<>* xml_node );

	EnvSettingsRefPtr _envSettings;

	/// file content needed for rapidxml
	char *_xml_data;

};	// class EnvSettingsSerializer

}	// namespace vl

#endif // VL_ENVSETTINGS_HPP
