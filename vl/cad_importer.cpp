/**
 *	Copyright (c) 2013 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-03
 *	@file cad_importer.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */


#include "cad_importer.hpp"

#include "base/filesystem.hpp"
#include "base/exceptions.hpp"
// Necessary for TRACE and CRITICAL
#include "logger.hpp"
// Necessary for create_process
#include "base/system_util.hpp"
// Necessary for APP data path
#include "base/string_utils.hpp"

// Used for ini file parsing
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

vl::CadImporter::CadImporter(std::string const &exe_path)
	: _exe_path(exe_path)
{
	// Check that we have a proper exe
	// @todo add better checking than just exists
	if(!fs::exists(_exe_path))
	{
		// @todo add error reporting (but do not throw)
		_exe_path.clear();
	}

	/// @todo we need to read cad_importer config here
	/// to get import_directory and export_directory

	_read_ini();

	/// @todo add reporting if the CAD importer is working or not
	/// check import, export and exe paths
	/// we might as well create a function for these
}

/* Why would we need this if the config already defines them?
 * Oh right we need to know the watch folder here
 */
void
vl::CadImporter::_read_ini(void)
{
	// Hard coded path for the moment
	fs::path ini_file_path = vl::get_global_path(vl::GP_APP_DATA) / fs::path("\\Hydra\\cad_importer.ini");
	if(!fs::exists(ini_file_path))
	{
		std::cout << vl::CRITICAL << "Hydra CAD importer config not existing at : " 
			<< ini_file_path.string() << std::endl;
		return;
	}

	boost::property_tree::ptree pt;

	// Load the INI file into the property tree. If reading fails
	// (cannot open file, parse error), an exception is thrown.
	boost::property_tree::read_ini(ini_file_path.string(), pt);

	// Parse ptree
	_import_directory = pt.get("importer.import_folder", "");
	_export_directory = pt.get("importer.export_folder", "");
}

bool
vl::CadImporter::_valid_config(void)
{
	/// @todo implement
	return true;
}

vl::CadImporter::~CadImporter(void)
{
}

void
vl::CadImporter::import(std::string const &filename, ImporterCallback *callback)
{
	std::clog << "vl::CadImporter::import" << std::endl;
	// @todo we can't check the file path because we only have filename here
	// the base path is in batch_importer's config.
	
	if(!callback)
	{
		// @todo add proper exception
		std::clog << "No callback function" << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No callback function"));
	}

	if(_exe_path.empty())
	{
		// @todo add proper exception
		std::clog << "No batch importer." << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc("No batch importer"));
	}

	if(_export_directory.empty())
	{
		std::string msg("We don't have export directory, cad_importer config was propably not processed.");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
	}

	if(_import_directory.empty())
	{
		std::string msg("We don't have import directory, cad_importer config was propably not processed.");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
	}

	/// Fork a batch_importer process
	fs::path import_file = fs::path(_import_directory) / fs::path(filename);
	std::clog << "Should load file : " << import_file.string() << std::endl;

	// check that the import file exists
	if(!fs::exists(import_file))
	{
		std::string msg("import file does not exists");
		std::clog << msg << " path : " << import_file.string() << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg) << vl::file_name(import_file.string()));
	}
	
	/// delete old copies
	/// @todo we are using funny filenames at the moment in the batch importer
	fs::path export_file = fs::path(_export_directory) / fs::path(filename + std::string(".dae"));
	if(fs::exists(export_file))
	{
		std::clog << "Deleting the old file at : " << export_file.string() << std::endl;
		fs::remove_all(export_file);
		assert(!fs::exists(export_file));
	}


	std::vector<std::string> params;
	params.push_back(std::string("--import_path"));
	params.push_back(import_file.string());
	uint32_t pid = vl::create_process(_exe_path, params);

	/// Add a callback filename pair
	_loading.push_back(std::make_pair(filename, callback));
}

void
vl::CadImporter::mainloop(void)
{
	// necessary for delayed destruction because we can not modify container
	// that is beign iterated at the moment
	// other option would be to use copies.
	std::vector< std::vector< std::pair<std::string, ImporterCallback *> >::iterator > to_delete;

	std::vector< std::pair<std::string, ImporterCallback *> >::iterator iter;
	for(iter = _loading.begin(); iter != _loading.end(); ++iter)
	{
		// we need to check if export_directory has any of the files we are waiting for
		// @todo we need to know the export directory here
		std::string filename = iter->first;
		fs::path import_file = fs::path(_export_directory) / fs::path(filename + ".dae");
		//std::clog << "checking for file : " << import_file.string() << std::endl;
		if(fs::exists(import_file))
		{
			std::clog << "File : " << filename << " loaded." << std::endl;
			(*iter->second)(filename, import_file.string());
			to_delete.push_back(iter);
		}
	}

	for(size_t i = 0; i < to_delete.size(); ++i)
	{
		_loading.erase(to_delete.at(i));
	}
}
