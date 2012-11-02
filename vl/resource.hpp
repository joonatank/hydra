/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-12
  *	@file resource.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

/**	Class to hold memory block where data from resource is stored
 *	@date 2011-05 Added ResourceStream for reading and writing to Resources
 */

#ifndef HYDRA_RESOURCE_HPP
#define HYDRA_RESOURCE_HPP

#include <string>
#include <vector>
#include <iterator>

#include "defines.hpp"

namespace vl
{

// Forward declaration for ResourceStream
class Resource;

class HYDRA_API ResourceStream
{
public :
	ResourceStream(Resource *resource);

	void skip(int bytes);

	/// @brief did we hit end of file
	bool eof(void);

	/// @brief Where in the stream are we
	size_t tell(void) const
	{ return _index; }

	void seek(size_t index);

	template<typename T>
	size_t read(T &t)
	{ return read( (char *)&t, sizeof(T) ); }

	template<typename T>
	size_t write(T const &t)
	{ return write( (char *)&t, sizeof(T) ); }

	size_t read(void *mem, size_t bytes)
	{ return read( (char *)mem, bytes ); }

	size_t write(void const *mem, size_t bytes)
	{ return write( (char *)mem, bytes ); }

	/// @brief read some data
	/// @param mem, the memory field where read data is stored
	/// @param bytes, the number of bytes to read
	/// Does not modify the resource, but increases the index
	size_t read(char *mem, size_t bytes);

	/// @brief write some data
	/// @param mem, memory field from where data is copied
	/// @param bytes, number of bytes to write
	size_t write(char const *mem, size_t bytes);

	/** Returns a String containing the next line of data, optionally 
		trimmed for whitespace. 
	@remarks
		This is a convenience method for text streams only, allowing you to 
		retrieve a String object containing the next line of data. The data
		is read up to the next newline character and the result trimmed if
		required.
    @note
        If you used this function, you <b>must</b> open the stream in <b>binary mode</b>,
        otherwise, it'll produce unexpected results.
	@param 
		trimAfter If true, the line is trimmed for whitespace (as in 
		String.trim(true,true))
	*/
	virtual std::string getLine(bool trimAfter = true);

	std::string getName(void) const;

	bool isWriteable(void) const;

	size_t left(void) const;

private :
	vl::Resource *_resource;
	size_t _index;

};

class HYDRA_API Resource
{
public :
	typedef std::vector<char>::iterator iterator;

	Resource( char *mem = 0, size_t size = 0 )
	{
		set( mem, size );
	}

	Resource( std::string const &name, std::vector<char> const &mem )
		: _name(name), _memory(mem)
	{}

	Resource( std::string const &name )
		: _name(name)
	{}

	virtual std::string const &getName( void ) const
	{ return _name; }

	virtual void setName( std::string const &name )
	{ _name = name; }

	size_t size( void ) const
	{ return _memory.size(); }

	void resize( size_t size )
	{ _memory.resize(size); }

	char &operator[]( size_t i )
	{ return _memory[i]; }

	char const &operator[]( size_t i ) const
	{ return _memory[i]; }


	/// Virtual so that the inherited classes can overload these and modify
	/// the binary data when it's set.
	virtual void set( char const *mem, size_t size );

	virtual void set( std::vector<char> const &mem );

	char *get( void )
	{ return (char *)&_memory[0]; }

	char const *get( void ) const
	{ return (char *)&_memory[0]; }

	/// for file input
	void insert( iterator pos, std::istreambuf_iterator<char> first, std::istreambuf_iterator<char> last )
	{ _memory.insert( pos, first, last ); }

	iterator begin( void )
	{ return _memory.begin(); }

	iterator end( void )
	{ return _memory.end(); }

	vl::ResourceStream getStream(void)
	{ return ResourceStream(this); }

protected :
	std::string _name;
	std::vector<char> _memory;

};	// class Resource

/**	TextResource
 *	Stores a text into memory.
 *	The resource has UNIX line-endings and is NULL ended.
 */
class HYDRA_API TextResource : public Resource
{
public :
	virtual void set( char const *mem, size_t size );

	virtual void set( std::vector<char> const &mem );

	/// Implicit conversion to the base class, as we have the same data
	operator Resource() const
	{
		return Resource( _name, _memory );
	}

	// TODO add conversion to const char *
	// TODO add conversion to std::string
};

void
loadResource( vl::Resource &res, std::string const &path );

}	// namespace vl

#endif // HYDRA_RESOURCE_HPP
