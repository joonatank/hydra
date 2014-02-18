/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/envsettings.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
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

#include <string>
#include <vector>
#include <iostream>
// Necessary for uint types
#include <stdint.h>

// Necessary for HYDRA_API
#include "defines.hpp"
// Config types
#include "config.hpp"
// Parser
#include "rapidxml.hpp"
// Necessary for PATH_TYPE and filesystem paths
#include "filesystem.hpp"


namespace vl
{

namespace config
{

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

	/**	@brief Get the relative path to the file this configuration was in
	 *	@return string to relative path
	 *
	 *	Used when reading a env file so we can retrieve it's path
	 *	for tracking files.
	 *
	 *	@TODO possibility to return an absolute path
	 */
	std::string const &getFile( void ) const
	{ return _file_path; }

	void setFile( std::string const &file )
	{ _file_path = file; }

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

	void addWall(vl::Wall const &wall);

	Wall const &getWall( size_t i ) const;

	size_t getNWalls( void ) const
	{ return _walls.size(); }

	std::vector<Wall> &getWalls( void )
	{ return _walls; }

	std::vector<Wall> const &getWalls( void ) const
	{ return _walls; }

	bool hasWall(std::string const &name) const;
	Wall &findWall(std::string const &name);
	Wall const &findWall(std::string const &name) const;
	Wall *findWallPtr(std::string const &name);
	Wall const *findWallPtr(std::string const &name) const;

	std::vector<Node> &getSlaves( void )
	{ return _slaves; }

	std::vector<Node> const &getSlaves( void ) const
	{ return _slaves; }

	Node const &findSlave( std::string const &name ) const;

	std::vector<Window *> get_all_windows();
	std::vector<Window const *> get_all_windows() const;

	std::vector<Channel *> get_all_channels();
	std::vector<Channel const *> get_all_channels() const;

	bool hasWindow(std::string const &name) const;
	Window &findWindow(std::string const &name);
	Window const &findWindow(std::string const &name) const;
	Window *findWindowPtr(std::string const &name);
	Window const *findWindowPtr(std::string const &name) const;

	bool hasChannel(std::string const &name) const;
	Channel &findChannel(std::string const &name);
	Channel const &findChannel(std::string const &name) const;
	Channel *findChannelPtr(std::string const &name);
	Channel const *findChannelPtr(std::string const &name) const;

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

	/**	@brief get the amount of interpupilar distance used for stereo
	 *	@return interpupilar distance in meters
	 *	@todo move to Renderer
	 */
	double getIPD( void ) const
	{ return _renderer.ipd; }

	/**	@brief Set how much information user wants
	 *	@param level vl::LogLevel
	 *
	 *	If set to true the application will print to std::cerr
	 *	instead of or in addition to printing to log file
	 */
	void setLogLevel(vl::config::LogLevel level)
	{ _level = level; }

	/// @brief How much information user wants
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
	{ return _renderer.fps; }

	void addProgram(Program const &prog);
	
	std::vector<Program> getUsedPrograms(void) const;

	Renderer const &getRenderer(void) const
	{ return _renderer; }
	Renderer &getRenderer(void)
	{ return _renderer; }

private :

	std::string _file_path;

	std::vector<Tracking> _tracking;

	std::vector<Program> _programs;

	Server _server;

	Node _master;
	
	Renderer _renderer;

	std::vector<Node> _slaves;

	// @todo walls can be removed they are now stored in Window
	std::vector<Wall> _walls;

	// Is this structure for a slave or a master
	bool _slave;

	vl::config::LogLevel _level;

	std::string _log_dir;

};	// class EnvSettings


/**	@class EnvSerializer
 */
class HYDRA_API EnvSerializer
{
public :
	/// Constructor
	EnvSerializer(void);

	/// Destructor
	~EnvSerializer(void);

	/// Read data from string buffer. Buffer is not modified.
	/// Will completely over-ride the provided EnvSettings
	/// when XML data is processed.
	/// Any error in processing will result to defaulting EnvSettings.
	bool readString(vl::config::EnvSettings &env, std::string const &xml_data);

protected :
	bool readXML( void );

	void processConfig( rapidxml::xml_node<>* XMLNode );

	void processPlugins( rapidxml::xml_node<>* XMLNode );

	void processTracking( rapidxml::xml_node<>* XMLNode );

	void processWalls( rapidxml::xml_node<>* XMLNode );

	void processServer( rapidxml::xml_node<>* XMLNode );

	void processNode( rapidxml::xml_node<>* XMLNode, vl::config::Node &node );

	void processWindows( rapidxml::xml_node<>* XMLNode, vl::config::Node &node );

	void processChannel( rapidxml::xml_node<>* XMLNode, vl::config::Window &window );

	void processPrograms(rapidxml::xml_node<> *xml_node);

	void processProgram(rapidxml::xml_node<> *xml_node);

	void processRenderer(rapidxml::xml_node<> *xml_node, Renderer &renderer);

	void processProjection(rapidxml::xml_node<> *xml_node, Projection &projection, std::string const &channel_name);

	/// Do everything that needs data from multiple sections of the file
	/// because sections can be in any order.
	void finalise(void);

	void _checkUniqueNode(rapidxml::xml_node<> *xml_node);

	std::vector<double> getVector( rapidxml::xml_node<>* xml_node );

	EnvSettings *_env;

	/// Temp map for getting proper wall into channel
	/// We need this because Channels contains wall name info only
	/// channel name -> wall name
	/// Needs to be this way because Channel is guaranteed to be unique
	/// wall is not.
	std::map<std::string, std::string> _wall_map;

	/// file content needed for rapidxml
	char *_xml_data;

};	// class EnvSettingsSerializer


}	// namespace config


}	// namespace vl

#endif	// HYDRA_ENVSETTINGS_HPP
