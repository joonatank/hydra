/**	Joonatan Kuosa
 *	2010-12
 *
 *	Class to hold memory block where data from resource is stored
 */

#ifndef VL_RESOURCE_HPP
#define VL_RESOURCE_HPP

#include <string>
#include <vector>
#include <iterator>

namespace vl
{

class Resource
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
	virtual void set( char *mem, size_t size );

	virtual void set( std::vector<char> mem );

	char *get( void )
	{ return (char *)&_memory[0]; }

	/// for file input
	void insert( iterator pos, std::istreambuf_iterator<char> first, std::istreambuf_iterator<char> last )
	{ _memory.insert( pos, first, last ); }

	iterator begin( void )
	{ return _memory.begin(); }

	iterator end( void )
	{ return _memory.end(); }

protected :
	std::string _name;
	std::vector<char> _memory;

};	// class Resource

class TextResource : public Resource
{
public :
	virtual void set( char *mem, size_t size );

	virtual void set( std::vector<char> mem );

	/// Implicit conversion to base class, as we have the same data
	operator Resource()
	{
		return Resource( _name, _memory );
	}
};

}	// namespace vl

#endif // VL_RESOURCE_HPP
