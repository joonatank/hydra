/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file logger.hpp
 *
 */

#ifndef VL_BASE_THREAD_HPP
#define VL_BASE_THREAD_HPP

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/thread.hpp>

namespace vl
{

using boost::interprocess::scoped_lock;

typedef boost::interprocess::interprocess_mutex mutex;
typedef boost::thread thread;

}	// namespace vl

#endif	// VL_BASE_THREAD_HPP
