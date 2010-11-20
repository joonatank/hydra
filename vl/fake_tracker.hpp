/**	Joonatan Kuosa
 *	2010-05 initial implementation
 *	2010-11 changed to an empty class
 *
 */

#ifndef VL_FAKE_TRACKER_HPP
#define VL_FAKE_TRACKER_HPP

#include "tracker.hpp"

namespace vl
{

// Empty class
class FakeTracker : public Tracker
{
public :
	// Empty function
	FakeTracker( void ) {}

	// Empty function
	~FakeTracker( void ) {}

	// Empty function
	void init( void ) {}

	// Empty function
	void mainloop( void ) {}

};

}

#endif