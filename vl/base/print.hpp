/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 */
#ifndef VL_BASE_PRINT_HPP
#define VL_BASE_PRINT_HPP

#include <iostream>
#include <vector>

template< typename T> inline
std::ostream & operator<<( std::ostream &os, std::vector<T> const &vec )
{
	typename std::vector<T>::const_iterator iter;
	for( iter = vec.begin(); iter != vec.end(); ++iter )
	{
		os << *iter << " ";
	}

	return os;
}

#endif