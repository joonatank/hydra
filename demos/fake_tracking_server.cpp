/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Dummy tracking server that sends constant position and orientation data
 *	Used for testing.
 */

#include "base/sleep.hpp"
#include "vrpn_Tracker.h"

int main (int argc, char **argv)
{
	vrpn_Connection *connection = vrpn_create_server_connection( ":3883" );
	vrpn_Tracker_Server *tracker = new vrpn_Tracker_Server("glasses", connection );
	if( tracker == NULL)
	{
		fprintf(stderr,"Can not create NULL tracker.");
		return -1;
	}
	vrpn_float64 pos[3];
	vrpn_float64 quat[4];

	pos[0] = 0;
	pos[1] = 1.5;
	pos[2] = 0;

	quat[0] = 0;
	quat[1] = 0;
	quat[2] = 0.707;
	quat[3] = 0.707;
	// Loop forever calling the mainloop()s for all devices and the connection
	while (1)
	{
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = 1e3;

		// Send and receive all messages
		tracker->report_pose( 0, t, pos, quat );
		tracker->mainloop();

		vl::msleep(1);
	}

	delete tracker;
	connection->removeReference();
}