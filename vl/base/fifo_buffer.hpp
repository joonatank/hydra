// Pointer fifo buffer
//
// Design
// This data structure is mainly used to transfer data structures from one
// thread to another either with messages or just plain read-write
// thread.
// Stores pointers that are dynamically allocated and whose ownership
// is transfered to this buffer and then passed along when popped.
// Important because in the event that user destroys the buffer without
// emptying it first the buffer will destroy all remaining elements.
//
// Implementation
// Completely working with using two threads one as a reader one as a writer
// transmiting cstrings and chars over the buffer from one thread to another.
// See tests/fifo_ptr_test.cpp
// Tested both fifo_buffer<T> and fifo_buffer<T *>
//
// TODO 
// Some considerations if fifo_buffer<T>::NONE should be replaced with 
// operator!(T). It's more intuitive works well for both naked pointers and
// smart_ptrs.
//
// TODO
// Not sure if this is such a good idea...
//
// Add spinlocks to read and write (separate).
// Add buffer resize method
// (need to aquire both read and write lock at some point).
//
// About spinlocks
// Add global variable to enable/disable spinlocks
// (this might be also in the definition of the lock).
// --- Not sure if this variable is such a good idea,
// 	most operations protected by the spinlocks are few instructions.
// 	Might provide an alternative for single core machines to use mutexes
// 	instead. Shouldn't be necessary, just disable interrupts for single
// 	core machines.
#ifndef EQ_OGRE_FIFO_BUFFER_HPP
#define EQ_OGRE_FIFO_BUFFER_HPP

#include <cstring>

#include "exceptions.hpp"

namespace vl 
{

namespace base
{
	// Fifo buffer for smart pointers
	template <class T>
	class fifo_buffer
	{
		public:
			fifo_buffer( size_t size = 8 );

			~fifo_buffer( void );

			T pop( void );

			void push( T const &d );

			size_t const &size (void) const
			{ return m_size; }

			static T NONE;

		private:
			// Non copyable
			fifo_buffer( fifo_buffer<T> const & );
			fifo_buffer<T> &operator=( fifo_buffer<T> const & );

			T *m_buf;
			size_t m_size;
			size_t read_index;
			size_t write_index;
	};	// class fifo_buffer

	template <class T>
	class fifo_buffer<T *>
	{
		public:
			fifo_buffer( size_t size = 8 );

			~fifo_buffer( void );

			T* pop( void );

			void push( T *d );

			size_t const &size (void) const
			{ return m_size; }

			static T *NONE;
		private:
			// Non copyable
			fifo_buffer( fifo_buffer<T *> const & );
			fifo_buffer<T> &operator=( fifo_buffer<T *> const & );

			T **m_buf;
			size_t m_size;
			size_t read_index;
			size_t write_index;
	};	// class fifo_buffer

}	// namespace base

}	// namespace vl

template< class T >
T vl::base::fifo_buffer< T >::NONE;

template< class T >
T *vl::base::fifo_buffer< T *>::NONE = 0;

// Speciallization for pointers.
// For pointers we need to allocate an array of pointers
template<class T> inline
vl::base::fifo_buffer<T *>::fifo_buffer( size_t size )
	: m_buf (0),
	  m_size (size),
	  read_index (0),
	  write_index (0)
{
//	char const *where = "fifo_buffer_p<T>::fifo_buffer_p";

	m_buf = new T*[m_size];

	for( size_t i = 0; i < m_size; i++ )
	{ m_buf[i] = NONE; }
}


// Speciallization for pointers.
// For pointers we will deallocate all elements contained in the buffer.
// As the purpose of pointer fifo buffer is to transfer ownership among
// threds any remaining pointer is owned by the container.
template< class T > inline
vl::base::fifo_buffer<T *>::~fifo_buffer( void )
{
	for (size_t i = 0; i < m_size; i++)
	{ delete m_buf[i]; }
	delete [] m_buf;
}

// Probably useless for now, version without specialization should work also.
template< class T > inline T*
vl::base::fifo_buffer<T *>::pop( void )
{ 
	T *tmp(m_buf[read_index]);

	// Only if buffer is not empty modify the index
	if( tmp != NONE )
	{
		// We need to NULL every element that has been read
		m_buf[read_index] = NONE;

		read_index++;
		if( read_index == m_size )
		{ read_index = 0; }
	}

	return tmp;
}

// Probably useless for now, version without specialization should work also.
template< class T > inline void
vl::base::fifo_buffer<T *>::push( T *d )
{
	char const *where = "fifo_buffer<T>::push";

	if( m_buf[write_index] != NONE )
	{ throw vl::fifo_full( where ); }
	m_buf[write_index] = d;

	write_index++;
	if( write_index == m_size )
	{ write_index = 0; }
}


template< class T > inline
vl::base::fifo_buffer<T>::fifo_buffer( size_t size )
	: m_buf (0),
	  m_size (size),
	  read_index (0),
	  write_index (0)
{
	char const *where = "fifo_buffer<T>::fifo_buffer";

	m_buf = new T[m_size];

	for( size_t i = 0; i < m_size; i++)
	{ m_buf[i] = NONE; }
}

template <class T> inline
vl::base::fifo_buffer<T>::~fifo_buffer( void )
{
	/*
	for (size_t i = 0; i < m_size; i++)
		delete m_buf[i];
	*/
	delete [] m_buf;
}

template <class T> inline T
vl::base::fifo_buffer<T>::pop( void )
{ 
	T tmp(m_buf[read_index]);

	// Only if buffer is not empty modify the index
	if ( tmp != NONE )
	{
		// We need to NULL every used element
		m_buf[read_index] = NONE;

		read_index++;
		if (read_index == m_size)
		{ read_index = 0; }
	}

	return tmp;
}

template <class T> inline void
vl::base::fifo_buffer<T>::push (T const &d)
{
	char const *where = "fifo_buffer<T>::push";

	if ( m_buf[write_index] != NONE )
	{ throw vl::fifo_full( where ); }
	m_buf[write_index] = d;

	write_index++;
	if (write_index == m_size)
	{ write_index = 0; }
}

#endif
