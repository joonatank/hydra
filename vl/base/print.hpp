/**
 *	Copyright (c) 2010 - 2011 Tampere University of Technology
 *	Copyright (c) 2011 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2010-11
 *	@file base/print.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_BASE_PRINT_HPP
#define HYDRA_BASE_PRINT_HPP

#include <iostream>
#include <vector>

namespace std
{

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

}	// namespace std

#endif	// HYDRA_BASE_PRINT_HPP
