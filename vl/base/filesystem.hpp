#ifndef VL_BASE_FILESYSTEM_HPP
#define VL_BASE_FILESYSTEM_HPP

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace vl
{

inline fs::path find_parent_dir( std::string const &name, fs::path const &path )
{
	fs::path tmp( path );
	// Check that we don't end into infinite loop
	if( tmp.empty() )
	{ return tmp; }

	// Init existing directory where to start the search
	while( !fs::exists( tmp )  )
	{
		tmp = tmp.remove_filename();
	}

	while( tmp != tmp.root_path() )
	{
		// Already is the correct path
		if( path.filename() == name )
		{
			tmp = path;
			return tmp;
		}
		else
		{
			// Iterate through all the child directories
			fs::directory_iterator end_itr;
			for( fs::directory_iterator itr( tmp ); itr != end_itr; ++itr )
			{
				if( fs::is_directory( itr->status() ) )
				{
					if( itr->path().filename() == name )
					{
						tmp = itr->path();
						return tmp;
					}
				}
			}

			// Go one level up
			tmp.remove_filename();
		}
	}

	if( tmp == tmp.root_path() )
	{ tmp = fs::path(); }

	return tmp;
}

}

#endif
