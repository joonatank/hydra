
/*	Joonatan Kuosa
 *	2010-05 initial
 *	2010-06 some meat
 *
 *	Tracker interface.
 *	Real trackers should derive from this.
 *	
 *	Has add callback method which user can use to add a callback functor for
 *	specific sensors.
 */

#ifndef VL_TRACKER_HPP
#define VL_TRACKER_HPP

namespace vl
{

class TrackerCallback
{
public :

};

class Tracker
{
public :
	

};	// class Tracker

class TrackerSerializer
{
	void read( char *xml_data );
};

}	// namespace vl

#endif