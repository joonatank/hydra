/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file base/report.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**
 *	Class for saving strings and associated performance numbers,
 *	performance numbers can be anything number of clock cycles, time, number of
 *	meshes, number of vertices, number of bytes.
 *
 *	Performance numbers can either be one time or multiple times
 *	For multiple times they can either be avaraged or summed before printing
 *	the report.
 *
 *	Using templates so we avoid heap allocation as much as possible
 *	Constraint it creates is that all numbers in a single report needs to be same type
 *	i.e. time, double, integer
 */

#ifndef HYDRA_REPORT_HPP
#define HYDRA_REPORT_HPP

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "base/exceptions.hpp"

namespace vl
{

template<typename T>
T
calculate_sum( std::vector<T> const &v )
{
	T sum = 0;
	for( size_t i = 0; i < v.size(); ++i )
	{
		sum += v.at(i);
	}
	return sum;
}

template<typename T>
T
calculate_avarage( std::vector<T> const &v )
{
	if( v.size() == 0 )
	{ return T(); }
	else
	{ return calculate_sum(v)/v.size(); }
}


/// @todo change to flags
/// How the number should be calculated
enum CALC_PROTO
{
	CALC_AVARAGE,
	CALC_SUM,
};

template<typename T>
class Number
{
public :
	/// @param priority for this number
	/// The highest priority is always printed first
	/// if two numbers have the same priority their print order is undefined
	Number(int priority = -1, CALC_PROTO = CALC_AVARAGE);

	/// @brief calculates a new result based on the real values
	/// if the proto is avarage does not touch the real values
	/// if the proto is sum removes all the summed values
	/// because the sum would go horribly wrong if same values would be summed
	/// even once (user forgotting to call clear after calculating).
	void calculate(void);

	void push(T const &val);

	/// @brief Clears the array of real values, does not touch the result
	void clear(void);

	bool empty(void) const;

	T const &result(void) const;

	int priority(void) const
	{ return _priority; }

private:
	CALC_PROTO _calc_proto;

	T _result;
	std::vector<T> _values;
	int _priority;
};

template<typename T>
std::ostream &operator<<(std::ostream &os, Number<T> const &n);

template<typename T> class Report;
template<typename T>
std::ostream &operator<<(std::ostream &os, Report<T> const &r);

template<typename T>
class Report
{
public :
	Report(void);

	/// @param name is the name of the number, if it already exists this will throw
	/// @return number which is just added, reference so data can be added
	Number<T> &add_number(std::string const &name);

	/// @brief get already existing number or create a new if not present
	/// @param name is the name of the number
	/// @return reference to a number with correct name
	Number<T> &get_number(std::string const &name);

	/// @brief has or doesn't have a number with this name
	/// @param name is the name of the number
	/// @return true if number with name exists in report, false otherwise
	bool has_number(std::string const &name) const;

	Number<T> &operator[](std::string const &name)
	{ return _numbers[name]; }

	/// @brief removes all numbers
	void clear(void);

	/// @brief is container empty
	/// @return true if size() == 0, false otherwise
	bool empty(void) const;

	/// @brief how many numbers
	/// @return number of numbers in the container
	size_t size(void) const;

	/// @brief close the current report
	/// post condition
	/// this report is finished and any further modifications don't modify the
	/// result of printing the report
	/// only next call to this function will modify the printing.
	void finish(void);

	friend std::ostream &operator<< <>(std::ostream &os, Report<T> const &r);

private :
	/// Can not do a typedef on a template when using C++98 standard
	/// Can be updated when C++0x is realeased (and if compilers are up to date)
	std::map< std::string, Number<T> > _numbers;

};	// class Report

}	// namespace vl

/// Template definitions

/// -------------------------------- Number ------------------------------------
template<typename T>
vl::Number<T>::Number(int priority, vl::CALC_PROTO cp)
	: _calc_proto(cp)
	, _result(0)
	, _priority(priority)
{}

template<typename T>
void
vl::Number<T>::calculate(void)
{
	if( _calc_proto == CALC_SUM )
	{
		_result += calculate_sum(_values);
		_values.clear();
	}
	else
	{
		_result = calculate_avarage(_values);
		_values.clear();
	}
}

template<typename T>
void
vl::Number<T>::push(T const &val)
{
	_values.push_back(val);
}

template<typename T>
void
vl::Number<T>::clear(void)
{
	_values.clear();
}

template<typename T>
bool
vl::Number<T>::empty(void) const
{
	return _values.empty();
}

template<typename T>
T const &
vl::Number<T>::result(void) const
{
	return _result;
}

template<typename T>
std::ostream &
vl::operator<<(std::ostream &os, vl::Number<T> const &n)
{
	os << "avarage : " << n.result();

	return os;
}

/// -------------------------------- Report ------------------------------------
template<typename T>
vl::Report<T>::Report(void)
{
}

/// @param name is the name of the number, if it already exists this will throw
/// @return number which is just added, reference so data can be added
template<typename T>
vl::Number<T> &
vl::Report<T>::add_number(std::string const &name)
{
	if( has_number(name) )
	{ BOOST_THROW_EXCEPTION( vl::duplicate() ); }

	return _numbers[name];
}

/// @brief get already existing number or create a new if not present
/// @param name is the name of the number
/// @return reference to a number with correct name
template<typename T>
vl::Number<T> &
vl::Report<T>::get_number(std::string const &name)
{
	return _numbers[name];
}

/// @brief has or doesn't have a number with this name
/// @param name is the name of the number
/// @return true if number with name exists in report, false otherwise
template<typename T>
bool
vl::Report<T>::has_number(std::string const &name) const
{
	typename std::map< std::string, vl::Number<T> >::const_iterator iter = _numbers.find(name);
	if( iter == _numbers.end() )
	{ return false; }

	return true;
}

template<typename T>
void
vl::Report<T>::clear(void)
{
}

/// @brief is container empty
/// @return true if size() == 0, false otherwise
template<typename T>
bool
vl::Report<T>::empty(void) const
{
	return _numbers.empty();
}

/// @brief how many numbers
/// @return number of numbers in the container
template<typename T>
size_t
vl::Report<T>::size(void) const
{
	return _numbers.size();
}

template<typename T>
void
vl::Report<T>::finish(void)
{
	typename std::map< std::string, vl::Number<T> >::iterator iter;
	for( iter = _numbers.begin(); iter != _numbers.end(); ++iter )
	{
		iter->second.calculate();
	}
}

template<typename T>
std::ostream &
vl::operator<<(std::ostream &os, vl::Report<T> const &r)
{
	// @todo should rearrange the array using priorities

	// Using '\n' rather than endl so that the buffer is not flushed
	// When using the Hydra logging system the message is kept in one piece
	// till the buffer is flushed.
	os << "Report: \n";
	typename std::map< std::string, vl::Number<T> >::const_iterator iter;
	for( iter = r._numbers.begin(); iter != r._numbers.end(); ++iter )
	{
		// TODO should this remove the last '\n'
		os << iter->first << " : " << iter->second << ".\n";
	}

	return os;
}

#endif	// HYDRA_REPORT_HPP
