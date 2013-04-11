/**
 *	Copyright (c) 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-03
 *	@file cad_importer.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_CAD_IMPORTER_HPP
#define HYDRA_CAD_IMPORTER_HPP

#include <string>
#include <vector>

#include "defines.hpp"

namespace vl
{

/// @class ImporterCallback
/// @brief callback that is called when file has been loaded
class ImporterCallback
{
public :
	/// Callback operator
	/// @param filename original name of the file that was loaded
	/// @param file_path path to the exported file that is ready for loading
	void operator()(std::string const &filename, std::string const &file_path)
	{
		_load(filename, file_path);
	}

private :
	virtual void _load(std::string const &filename, std::string const &file_path) = 0;
};

/**	@class CadImporter
 *	Keeps track of the launched importing processes
 *	Because all the importing is asynchronic and rather slow this class manages
 *	callbacks to the simulator and keeps track of all imports.
 *
 *	Can be used even when there is no external importer, naturally does not do
 *	anything but reports the error to the user. Doesn't however crash the software.
 *	
 *	Exiting: needs to destroy temp files and kill all forked processes.
 */
class HYDRA_API CadImporter
{
public :
	/// Constructor
	/// @todo we need to pass the executable path here or null string if it's disabled
	/// 
	CadImporter(std::string const &exe_path);
	
	/// Destructor
	~CadImporter(void);

	/// @brief
	/// If the Importing software is not found throws an error.
	/// @todo add option for overwriting the default recipe 
	/// (using the name of already uploaded recipe)
	void import(std::string const &filename, ImporterCallback *callback);

	/// loop function that is called to check if any of the imports have been finished
	void mainloop(void);

	/// @brief check if we are currently loading a file
	bool isLoading(void) const
	{
		return !_loading.empty();
	}

private :
	void _read_ini(void);

	bool _valid_config(void);

	std::string _exe_path;

	std::string _import_directory;

	std::string _export_directory;

	// path callback pairs
	// we check when there is a new file in path then use the callback
	// we also need a timestamp if we are using file overwrite
	// if we remove any existing files first we don't need timestamp
	std::vector< std::pair<std::string, ImporterCallback *> > _loading;

	// pid vector of processes we have launched
	// we need to manage these a bit better, we need to check when the process is
	// finished etc.
	// Not sure if actually need these as the batch_importer is really fast
	// and small program we should never need to kill it.
	//std::vector<uint32_t> _forked_processes;

};

}

#endif	// HYDRA_CAD_IMPORTER_HPP