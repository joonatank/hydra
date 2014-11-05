/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-05
 *	@file profiler_report.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PROFILER_REPORT_HPP
#define HYDRA_PROFILER_REPORT_HPP

// Using the storage type from report
#include "base/report.hpp"

#include "base/time.hpp"

namespace vl
{

/// Total times taken for certain parts of the program
/// The order is important at least on conceptual level because
/// We assume that the frame is splitted in the presented order.
/// @todo we might want to make these structs instead
/// even as basic enums they couldn't be implicitly casted
/// and the possibility of adding more data to them is quite powerful.
/// One basic struct with ID (corresponds to the current enum)
/// and overloaded comparison operators.
/// Added bonus data localisation, so we could have type strings and
/// overloaded printing in the struct itself.
enum PROFILER_TOTAL
{
	PT_KINEMATICS,
	PT_PHYSICS,
	PT_COLLISIONS,
	PT_RENDERING,
	PT_FRAME,
	PT_SIZE,	// Keep as a last element used to determine size
};

// @todo add more complex individual stats using
// CATEGORY and NAME system
// category so we can assing it under one of the above totals.

/** @class ProfilerReport
 */
class ProfilerReport
{
public :
	ProfilerReport(void);

	~ProfilerReport(void);

	Number<vl::time> &operator[](PROFILER_TOTAL index)
	{ return _profiling.at(index); }

	Number<vl::time> const &operator[](PROFILER_TOTAL index) const
	{ return _profiling.at(index); }

	Number<vl::time> &operator[](size_t index)
	{ return _profiling.at(index); }

	Number<vl::time> const &operator[](size_t index) const
	{ return _profiling.at(index); }

	bool isDirty(void)
	{ return _dirty; }

	void _clearDirty(void)
	{ _dirty = false; }

	void finish(void);

	size_t size(void) const
	{ return _profiling.size(); }

	friend std::ostream &operator<<(std::ostream &, ProfilerReport const &);

private :
	void _setDirty(void)
	{ _dirty = true; }

	std::vector< Number<vl::time> > _profiling;

	bool _dirty;

};	// class ProfilerReport

}	// namespace vl

#endif	// HYDRA_PROFILER_REPORT_HPP
