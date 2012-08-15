/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-05
 *	@file profiler_report.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#include "profiler_report.hpp"

std::ostream &
vl::operator<<(std::ostream &os, vl::ProfilerReport const &report)
{
	os << "KINEMATICS : " << report._profiling.at(PT_KINEMATICS).result() << "\n"
		<< "PHYSICS : " << report._profiling.at(PT_PHYSICS).result() << "\n"
		<< "COLLISIONS" << report._profiling.at(PT_COLLISIONS).result() << "\n"
		<< "RENDERING : " << report._profiling.at(PT_RENDERING).result() << "\n"
		<< "FRAME TOTAL : " << report._profiling.at(PT_FRAME).result() << "\n";

	return os;
}

vl::ProfilerReport::ProfilerReport(void)
	: _dirty(false)
{
	// @todo resize the storage vector
	_profiling.resize(PT_SIZE);
}

vl::ProfilerReport::~ProfilerReport(void)
{
}

void
vl::ProfilerReport::finish(void)
{
	_setDirty();
	std::vector< vl::Number<vl::time> >::iterator iter;
	for( iter = _profiling.begin(); iter != _profiling.end(); ++iter )
	{
		iter->calculate();
	}
}
