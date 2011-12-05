/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-03
 *	@file base/thread.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

#ifndef HYDRA_BASE_THREAD_HPP
#define HYDRA_BASE_THREAD_HPP

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/thread.hpp>

namespace vl
{

using boost::interprocess::scoped_lock;

typedef boost::interprocess::interprocess_mutex mutex;
typedef boost::thread thread;

}	// namespace vl

#endif	// HYDRA_BASE_THREAD_HPP
