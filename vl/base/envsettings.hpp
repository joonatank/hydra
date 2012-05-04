/**
 *	Copyright (c) 2010-2011 Tampere University of Technology
 *	Copyright (c) 2011-2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/envsettings.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

/**
 *	2010-11-29 Added camera rotations to env file
 *	moved ref ptr definition to typedefs.hpp
 *
 *	2011-01-06 Added wall and window to env file
 *	Added stereo and inter pupilar distance to env file
 */

#ifndef HYDRA_ENVSETTINGS_HPP
#define HYDRA_ENVSETTINGS_HPP

// Necessary for HYDRA_API
#include "defines.hpp"

#include <string>
#include <vector>
#include <iostream>

#include <stdint.h>

#include "filesystem.hpp"
#include "rapidxml.hpp"
#include "typedefs.hpp"
#include "exceptions.hpp"

#include "wall.hpp"

#include "xml_helpers.hpp"

namespace vl
{

namespace config
{

enum LogLevel
{
	LL_LOW = 0,
	LL_NORMAL = 1,
	LL_BOREME = 2,
};

enum StereoType
{
	ST_OFF,
	ST_QUAD_BUFFER,
	ST_SIDE_BY_SIDE,
	ST_TOP_BOTTOM,
};

template<typename T>
struct Rect
{
	Rect(void) : w(0), h(0), x(0), y(0) {}

	Rect(T pw, T ph, T px, T py) : w(pw), h(ph), x(px), y(py) {}

	bool operator==(Rect const &other) const
	{
		if(other.w == w && other.h == h && other.x == x && other.y == y)
		{ return true; }
		return false;
	}

	bool valid(void) const
	{ return (w > 0 && h > 0); }

	T w;
	T h;
	T x;
	T y;
};


struct Tracking
{
	Tracking( std::string const &file_name, bool u = "true" )
		: file(file_name), use(u)
	{}

	bool operator==( Tracking const &other ) const
	{
		if( file == other.file )
		{ return true; }

		return false;
	}

	std::string file;
	bool use;
};

struct HYDRA_API Channel
{
	Channel(std::string const &nam, std::string const &wall, 
			Rect<double> const &a, Ogre::ColourValue const &colour)
		: name(nam), wall_name(wall), area(a), background_colour(colour)
	{}

	/// Default constructor for vector resizes
	Channel( void )
	{}

	bool empty( void ) const
	{ return( name.empty() && wall_name.empty() && area == Rect<double>() ); }

	std::string name;

	// Name of the wall used for this window
	std::string wall_name;

	Rect<double> area;

	Ogre::ColourValue background_colour;

};	// struct Channel

struct HYDRA_API Projection
{
	enum Type
	{
		PERSPECTIVE,
		ORTHO,
	};

	enum PerspectiveType
	{
		WALL,
		FOV,
	};

	Projection(void)
		: type(PERSPECTIVE)
		, perspective_type(FOV)
		, fov(60)
		, horizontal(-1)
		, head_x(false)
		, head_y(true)
		, head_z(false)
		, use_asymmetric_stereo(false)
	{}

	Type type;

	PerspectiveType perspective_type;

	// Field-of-View in Degrees
	double fov;

	// Horizontal in percentage zero is floor and one is ceiling
	double horizontal;

	bool head_x;
	bool head_y;
	bool head_z;

	bool modify_transformations;

	bool use_asymmetric_stereo;
};

/// The rendering element that can be individually rendered
/// usually either a window or a FBO
/// this is completely independent of the two though and does not reference 
/// either of them.
struct HYDRA_API Renderer
{
	enum Type
	{
		WINDOW,
		FBO,
		DEFERRED,
	};

	Renderer(void)
		: type(WINDOW)
		, projection()
		, hardware_gamma(false)
	{}

	Type type;
	Projection projection;

	bool hardware_gamma;
};

struct HYDRA_API Window
{
	Window( std::string const &nam, int width, int height, int px, int py,
			StereoType stereo_t = ST_OFF )
	{
		clear();

		name = nam;
		rect = Rect<int>(width, height, px, py);
		stereo_type = stereo_t;

		if( rect.h < 0 || rect.w < 0 )
		{
			std::string desc("Width or height of a Window can not be negative");
			BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
		}
	}

	Window( std::string const &nam, Rect<int> area, StereoType stereo_t = ST_OFF )
	{
		clear();
	
		name = nam;
		rect = area;
		stereo_type = stereo_t;

		if( rect.h < 0 || rect.w < 0 )
		{
			std::string desc("Width or height of a Window can not be negative");
			BOOST_THROW_EXCEPTION( vl::invalid_settings() << vl::desc(desc) );
		}
	}


	// Default constructor to allow vector resize
	Window(void)
	{
		clear();
	}

	void clear(void)
	{
		stereo_type = ST_OFF;
		vert_sync = false;
		input_handler = true;

		n_display = -1;

		fsaa = 0;

		nv_swap_sync = false;
		nv_swap_group = 0;
		nv_swap_barrier = 0;
	}

	// Wether or not the Window has been initialised
	bool empty( void ) const
	{
		return( name.empty() && _channels.empty() && rect == Rect<int>() );
	}

	// Name of the window
	std::string name;

	Renderer renderer;

	Rect<int> rect;

	NamedParamList params;


	StereoType stereo_type;

	bool nv_swap_sync;
	uint32_t nv_swap_group;
	uint32_t nv_swap_barrier;

	bool vert_sync;
	bool input_handler;
	int n_display;

	int fsaa;

	void add_channel(Channel const &channel)
	{
		if(has_channel(channel.name))
		{ BOOST_THROW_EXCEPTION(vl::invalid_settings() << vl::desc("Can't have two channels with same name")); }

		_channels.push_back(channel);
	}

	bool has_channel(std::string const &name) const
	{
		for(size_t i = 0; i < _channels.size(); ++i)
		{
			if(_channels.at(i).name == name)
			{ return true; }
		}

		return false;
	}

	Channel const &get_channel(size_t i) const
	{ return _channels.at(i); }

	Channel &get_channel(size_t i)
	{ return _channels.at(i); }

	size_t get_n_channels(void) const
	{ return _channels.size(); }


	/// @internal distribution access
	std::vector<Channel> const &get_channels(void) const
	{ return _channels; }

	std::vector<Channel> &get_channels(void)
	{ return _channels; }

private :
	std::vector<Channel> _channels;

};	// struct Window

struct HYDRA_API Node
{
	Node( std::string const &nam )
		: name(nam)
	{}

	// Default constructor to allow vector resize
	Node( void )
	{}

	bool empty( void ) const
	{ return name.empty() && windows.empty(); }

	void addWindow( Window const &window );

	Window &getWindow(size_t i);
	
	Window const &getWindow(size_t i) const;

	size_t getNWindows( void ) const
	{ return windows.size(); }

	std::vector<Window> const &getWindows( void ) const
	{ return windows; }

	std::string name;
	std::vector<Window> windows;
};

struct HYDRA_API Server
{
	Server( uint16_t por, std::string const hostnam )
		: port(por), hostname(hostnam)
	{}

	// Default constructor
	Server( void )
	{}

	uint16_t port;
	std::string hostname;
};

/// External program description
struct HYDRA_API Program
{
	std::string name;
	std::string directory;
	std::string command;
	std::vector<std::string> params;
	bool use;
	bool new_console;

	Program(void)
		: use(false)
		, new_console(false)
	{}
};

/**	@class EnvSettings
 *	@brief Settings for the environment the program is run.
 *
 *	Includes configuration for the Windows, Walls, Tracking, Cluster, Stereo,
 *	and Server.
 */
class HYDRA_API EnvSettings
{
public :
	/// Constructor
	EnvSettings( void );

	/// Destructor
	~EnvSettings( void );

	/// Clear the settings structure to default values
	void clear( void );

	/** @brief Is this configuration for a slave node
	 *	@return true if this is slave configuration false if not
	 *
	 *	isSlave always returns !isMaster
	 */
	bool isSlave( void ) const
	{ return _slave; }

	/** @brief Is this configuration for a master node
	 *	@return true if this is master configuration false if not
	 *
	 *	isMaster always returns !isSlave
	 */
	bool isMaster( void ) const
	{ return !_slave; }

	/** @brief Set this confiration to be for a slave
	 *
	 *	after call to this isSlave returns true and isMaster returns false
	 */
	void setSlave( void )
	{ _slave = true; }

	/** @brief Set this confiration to be for a master
	 *
	 *	after call to this isSlave returns false and isMaster returns true
	 */
	void setMaster( void )
	{ _slave = false; }

	/** @brief Get the relative path to the file this configuration was in
	 *	@return string to relative path
	 *
	 *	@TODO is this a necessary function
	 *	@TODO possibility to return an absolute path
	 */
	std::string const &getFile( void ) const
	{ return _file_path; }

	void setFile( std::string const &file )
	{ _file_path = file; }

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
	/// Returns a vector of tracking configs
	std::vector<Tracking> const &getTracking( void ) const
	{ return _tracking; }

	/**	@brief Get the tracking files which are used
	 *	@return a vector of the names of the tracking files
	 *
	 *	Only tracking files that are in use are returned
	 *	@TODO add checking that the files are valid
	 */
	std::vector< std::string > getTrackingFiles (void ) const;

	/**	@brief Adds a tracking file to the tracking file stack.
	 *	@param track a Tracking configuration to add to the file stack.
	 *
	 *	Checks that the same file tracking file is not added twice, NOP if the
	 *	tracking file is already in the stack.
	 */
	void addTracking( Tracking const &track );

	/**	@brief Remove a tracking file from the stack.
	 *	@param track a string with a tracking filename
	 *
	 *	Removes a tracking file with the same filename if such tracking file is
	 *	not found in these settings this function is a NOP.
	 */
	void removeTracking( std::string const &track );

	/**	@brief Remove a tracking file from the stack.
	 *	@param track a Tracking configuration comparison done with == operator
	 *
	 * 	Removes a tracking file with the same filename if such tracking file is
	 *	not found in these settings this function is a NOP.
	 */
	void removeTracking( Tracking const &track );

	/// Returns a flags of around which axes the camera rotations are allowed
	/// Fist bit is the x axis, second y axis, third z axis
	uint32_t getCameraRotationAllowed( void ) const
	{ return _camera_rotations_allowed; }

	void setCameraRotationAllowed( uint32_t const flags )
	{ _camera_rotations_allowed = flags; }

	void addWall(vl::Wall const &wall);

	Wall const &getWall( size_t i ) const;

	size_t getNWalls( void ) const
	{ return _walls.size(); }

	std::vector<Wall> &getWalls( void )
	{ return _walls; }

	std::vector<Wall> const &getWalls( void ) const
	{ return _walls; }

	Wall findWall( std::string const &wall_name ) const;

	std::vector<Node> &getSlaves( void )
	{ return _slaves; }

	std::vector<Node> const &getSlaves( void ) const
	{ return _slaves; }

	Node const &findSlave( std::string const &name ) const;

	std::string const &getName(void) const
	{ return _master.name; }

	/** @brief Get the master nodes configuration
	 *	@return Node representing the Master configuration
	 */
	Node &getMaster( void )
	{ return _master; }

	/** @brief Get the master nodes configuration
	 *	@return Node representing the Master configuration
	 */
	Node const &getMaster( void ) const
	{ return _master; }

	/** @brief Get the server configuration
	 *	@return Server representing the configuration of the server
	 */
	Server const &getServer( void ) const
	{ return _server; }

	/** @brief Set the master Server configuration
	 *	@param server representing the Server configuration
	 */
	void setServer( Server const &server )
	{ _server = server; }

	/**	@brief get the configuration of stereo
	 *	@return true if should use stereo false otherwise
	 */
	StereoType getStereoType( void ) const
	{ return _stereo_type; }

	/**	@brief set if we use stereo or not
	 *	@param stereo true if should use stereo false otherwise
	 */
	void setStereoType(StereoType type)
	{ _stereo_type = type; }

	bool hasHardwareGamma(void) const
	{ return _renderer.hardware_gamma; }

	void setHardwareGamma(bool g)
	{ _renderer.hardware_gamma = g; }


	/**	@brief using NVidia swap sync or not
	 *	@return true if should use NVidia swap sync false otherwise
	 */
	bool hasNVSwapSync(void) const
	{ return _nv_swap_sync; }

	/**	@brief set wether or not to use NVidia swap sync
	 *	@param val true if should use NVidia swap sync false otherwise
	 */
	void setNVSwapSync(bool val)
	{ _nv_swap_sync = val; }

	uint32_t getNVSwapGroup(void) const
	{ return _swap_group; }
	
	void setNVSwapGroup(uint32_t val)
	{ _swap_group = val; }

	uint32_t getNVSwapBarrier(void) const
	{ return _swap_barrier; }
	
	void setNVSwapBarrier(uint32_t val)
	{ _swap_barrier = val; }

	/**	@brief set the amount of interpupilar distance used for stereo
	 *	@param ipd the distance in meters
	 */
	void setIPD( double ipd )
	{ _ipd = ipd; }

	/**	@brief get the amount of interpupilar distance used for stereo
	 *	@return interpupilar distance in meters
	 */
	double getIPD( void ) const
	{ return _ipd; }

	/**	@brief Set how much information user wants
	 *	@param level vl::LogLevel
	 *
	 *	If set to true the application will print to std::cerr
	 *	instead of or in addition to printing to log file
	 */
	void setLogLevel(vl::config::LogLevel level)
	{ _level = level; }

	/**	@brief How much information user wants
	 *	@return
	 *
	 */
	vl::config::LogLevel getLogLevel( void ) const
	{ return _level; }

	/**	@brief Set the directory logs are stored
	 *	@param dir relative path to the log directory.
	 *
	 *	Path is assumed to be relative, though absolute might work it's
	 *	not guaranteed.
	 *	The directory is assumed to be valid directory. Not defined what will
	 *	happen in the program if it's not valid.
	 */
	void setLogDir( std::string const &dir )
	{ _log_dir = dir; }

	/** @brief Get the directory logs are stored.
	 *	@param type which kind of path to return, relative or absolute.
	 *	@return a path to the log dir, no checking is provided so it might be invalid.
	 *
	 *	Relative and absolute can be chosen using type parameter.
	 *	Defaults to returning absolute path.
	 *	No checking anywhere is provided so returned path might be invalid.
	 */
	std::string getLogDir( vl::PATH_TYPE const type = vl::PATH_ABS ) const;

	/** @brief Set the exe path i.e. the command used to start the program.
	 *	@param path the command used to start program, usually argv[0]
	 *	@TODO Is this necessary? Where is it used?
	 *		  We would need this for launching out of the exe directory
	 *		  or adding the exe directory to PATH to be used for dlls.
	 *	@TODO should be replaced with exe_name or program_name
	 *		  and program directory
	 */
// 	void setExePath( std::string const &path );

	/** @brief Get the directory where this environment file is stored.
	 *	@return valid path to the file where this configuration is stored.
	 *			empty string if this is not stored into a file.
	 */
	std::string getEnvironementDir( void ) const;

	/// @brief get and set the FPS limit used for this session
	/// if zero FPS is not limited
	/// Defaults to 60.
	/// Only meaningful for the master as it controls the slaves.
	uint32_t getFPS(void) const
	{ return _fps; }

	void setFPS(uint32_t fps)
	{ _fps = fps; }

	/// @brief the display number to use
	/// Only useful for X11 for now and needs to be a valid X11 display
	int display_n;
	
	void addProgram(Program const &prog);
	
	std::vector<Program> getUsedPrograms(void) const;

	Renderer const &getRenderer(void) const
	{ return _renderer; }
	Renderer &getRenderer(void)
	{ return _renderer; }

private :

	std::string _file_path;

	std::vector<std::pair<std::string, bool> > _plugins;
	std::vector<Tracking> _tracking;

	std::vector<Program> _programs;

	uint32_t _camera_rotations_allowed;

	Server _server;

	Node _master;
	std::vector<Node> _slaves;

	std::vector<Wall> _walls;

	StereoType _stereo_type;

	bool _nv_swap_sync;
	uint32_t _swap_group;
	uint32_t _swap_barrier;

	// Inter pupilar distance
	double _ipd;

	// Is this structure for a slave or a master
	bool _slave;

	vl::config::LogLevel _level;

	std::string _log_dir;

	uint32_t _fps;

	Renderer _renderer;

};	// class EnvSettings





class HYDRA_API EnvSerializer
{
public :
	/// Will completely over-ride the provided EnvSettings
	/// when XML data is processed.
	/// Any error in processing will result to defaulting EnvSettings.
	EnvSerializer(EnvSettingsRefPtr env);

	~EnvSerializer( void );

	/// Read data from string buffer. Buffer is not modified.
	bool readString( std::string const &xml_data );


protected :
	bool readXML( void );

	void processConfig( rapidxml::xml_node<>* XMLNode );

	void processPlugins( rapidxml::xml_node<>* XMLNode );

	void processTracking( rapidxml::xml_node<>* XMLNode );

	void processCameraRotations( rapidxml::xml_node<>* XMLNode );

	void processWalls( rapidxml::xml_node<>* XMLNode );

	void processServer( rapidxml::xml_node<>* XMLNode );

	void processNode( rapidxml::xml_node<>* XMLNode, vl::config::Node &node );

	void processWindows( rapidxml::xml_node<>* XMLNode, vl::config::Node &node );

	void processChannel( rapidxml::xml_node<>* XMLNode, vl::config::Window &window );

	void processStereo(rapidxml::xml_node<> *xml_node);

	void processNVSwapSync(rapidxml::xml_node<> *xml_node);

	void processIPD(rapidxml::xml_node<> *xml_node);

	void processFPS(rapidxml::xml_node<> *xml_node);

	void processPrograms(rapidxml::xml_node<> *xml_node);

	void processProgram(rapidxml::xml_node<> *xml_node);

	void processRenderer(rapidxml::xml_node<> *xml_node, Renderer &renderer);

	void processProjection(rapidxml::xml_node<> *xml_node, Projection &projection);

	void _checkUniqueNode(rapidxml::xml_node<> *xml_node);

	std::vector<double> getVector( rapidxml::xml_node<>* xml_node );

	EnvSettingsRefPtr _env;

	/// file content needed for rapidxml
	char *_xml_data;

};	// class EnvSettingsSerializer


}	// namespace config


}	// namespace vl

#endif	// HYDRA_ENVSETTINGS_HPP
