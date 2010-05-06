#ifndef VL_BASE_FILESYSTEM_HPP
#define VL_BASE_FILESYSTEM_HPP

#include <boost/filesystem.hpp>

#include <fstream>

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

// Data structure to hold file in memory
// can be used for other strings also, but is mainly useful
// for holding char string and making sure that it's destroyed.
struct FileString
{
	// Read the file in to memory
	FileString( std::ifstream &in )
		: data(0)
	{
		readStream( in );
	}

	FileString( void )
		: data(0)
	{}

	~FileString( void )
	{
		delete [] data;
	}

	void readStream( std::ifstream &in )
	{
		in.seekg( 0, std::ios::end );
		int length = in.tellg();
		in.seekg( 0, std::ios::beg );

		delete [] data;
		data = new char[length];
		in.read( data, length-1 );
		in.close();
		data[length-1] = '\0';
	}

	char *data;

private :
	/** Disallow copy
	 * This struct is used for reading and manipulating files in memory
	 * copy would probably be a modified version of the file and not what the
	 * user wants.
	 *
	 * If necessary please provide an alternative class with const data or 
	 * another class which this class can be copyed from.
	 **/
	FileString( FileString const &);
	FileString &operator=( FileString const &);
};

}

#endif
