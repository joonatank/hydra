#ifndef VL_TEST_TRACKING_FIXTURE_HPP
#define VL_TEST_TRACKING_FIXTURE_HPP

#include "vrpn_Tracker.h"
#include "vrpn_Connection.h"
#include <time.h>

#include "base/exceptions.hpp"

struct TrackerServerFixture
{
	TrackerServerFixture( void )
	{
		connection = vrpn_create_server_connection( ":3883" );
		tracker = new vrpn_Tracker_Server("glasses", connection );
		if( tracker == NULL)
		{
			fprintf(stderr,"Can not create NULL tracker.");
			throw vl::exception();
		}

		pos[0] = 0;
		pos[1] = 1.5;
		pos[2] = 0;

		quat[0] = 0;
		quat[1] = 0;
		quat[2] = 0.707;
		quat[3] = 0.707;
	}

	~TrackerServerFixture( void )
	{
		delete tracker;
		connection->removeReference();
	}

	void mainloop( void )
	{
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = 1e3;

		tracker->report_pose( 0, t, pos, quat );
		
		tracker->mainloop();

		// Send and receive all messages
		connection->mainloop();
	}

	vrpn_float64 pos[3];
	vrpn_float64 quat[4];

	vrpn_Connection *connection;
	vrpn_Tracker_Server *tracker;
};

#endif
