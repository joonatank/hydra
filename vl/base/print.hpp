#ifndef VL_PRINT_HPP
#define VL_PRINT_HPP

#include <iostream>
#include <vector>

template< typename T> inline
std::ostream & operator<<( std::ostream &os, std::vector<T> vec )
{
	typename std::vector<T>::iterator iter;
	for( iter = vec.begin(); iter != vec.end(); ++iter )
	{
		os << *iter << " ";
	}
	os << std::endl;

	return os;
}

#endif