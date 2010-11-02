
#include "eq_settings.hpp"

#include "serialize_helpers.hpp"

eqOgre::Settings::Settings( std::string const &name  )
	: vl::Settings(name),
	  _frame_data_id(EQ_ID_INVALID)
{}

void eqOgre::Settings::getInstanceData(eq::net::DataOStream& os)
{
	os << _frame_data_id << _project_name << _log_dir;

	os << _roots.size();
	for( size_t i = 0; i < _roots.size(); ++i )
	{
		os << _roots.at(i).name << _roots.at(i).path;
	}

	// Not necessary as these are only needed when launched
//	os << _exe_path << _file_path << _eq_config.file;

	os << _scenes.size();
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		os << _scenes.at(i).file << _scenes.at(i).name
			<< _scenes.at(i).attach_node << _scenes.at(i).type;
	}
	
	os << _plugins.getPath();

	os << _resources.size();
	for( size_t i = 0; i < _resources.size(); ++i )
	{
		os << _resources.at(i).getPath();
	}

	os << _tracking.size();
	for( size_t i = 0; i < _tracking.size(); ++i )
	{
		os << _tracking.at(i).getPath();
	}

	// No need to distribute eq args as they are only needed when launched

	// TODO
	// These probably don't need to be distributed also as the AppNode should
	// handle tracking
	os << _tracker_address << _tracker_default_pos << _tracker_default_orient;
}

void eqOgre::Settings::applyInstanceData(eq::net::DataIStream& is)
{
	is >> _frame_data_id >> _project_name >> _log_dir;

	size_t size;
	is >> size;
	_roots.resize(size);
	for( size_t i = 0; i < _roots.size(); ++i )
	{
		is >> _roots.at(i).name >> _roots.at(i).path;
	}

	// Not necessary as these are only needed when launched
//	is >> _exe_path >> _file_path >> _eq_config.file;

	is >> size;
	_scenes.resize(size);
	for( size_t i = 0; i < _scenes.size(); ++i )
	{
		is >> _scenes.at(i).file >> _scenes.at(i).name
			>> _scenes.at(i).attach_node >> _scenes.at(i).type;
	}

	is >> _plugins.file;

	is >> size;
	_resources.resize(size);
	for( size_t i = 0; i < _resources.size(); ++i )
	{
		is >> _resources.at(i).file;
	}

	is >> size;
	_tracking.resize(size);
	for( size_t i = 0; i < _tracking.size(); ++i )
	{
		is >> _tracking.at(i).file;
	}

	// No need to distribute eq args as they are only needed when launched

	// TODO
	// These probably don't need to be distributed also as the AppNode should
	// handle tracking
	is >> _tracker_address >> _tracker_default_pos >> _tracker_default_orient;
}
