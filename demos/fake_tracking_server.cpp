/**	Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-11
 *
 *	Dummy tracking server that sends constant position and orientation data
 *	Used for testing.
 */

#include "base/sleep.hpp"
#include "vrpn_Tracker.h"

#include <iostream>

void getHeadData( vrpn_float64 *pos, vrpn_float64 *quat )
{
	pos[0] = 0;
	pos[1] = 1.5;
	pos[2] = 0;

	quat[0] = 0;
	quat[1] = 0;
	quat[2] = 0.707;
	quat[3] = 0.707;
}

void getMeveaData( size_t sensor, vrpn_float64 *pos, vrpn_float64 *quat )
{
	// TODO this should have different positions for different sensors
	int i = 0, j = 0, k = 0;
	// 5 minor elements, we start from -4,4 coordinate and go to 4,4
	i = ( sensor%5 - 2)*2;
	// 4 major elements, we start from -4,4 coordinate and go to -4,4
	j = ( sensor/5 - 2)*(-2);

	pos[0] = i;
	pos[1] = j;
	pos[2] = 0;

	quat[0] = 0;
	quat[1] = 0;
	quat[2] = 0.707;
	quat[3] = 0.707;
}

int main (int argc, char **argv)
{
	vrpn_Connection *connection = vrpn_create_server_connection( "localhost" );
	vrpn_Tracker_Server *tracker = new vrpn_Tracker_Server("glasses", connection );
	vrpn_Tracker_Server *mevea = new vrpn_Tracker_Server("Mevea", connection, 20 );

	if( tracker == NULL || mevea == NULL )
	{
		fprintf(stderr,"Can not create NULL tracker.");
		return -1;
	}

	// Loop forever calling the mainloop()s for all devices and the connection
	while (1)
	{
		const int msecs = 8;
		struct timeval t;
		t.tv_sec = 0;
		t.tv_usec = msecs*1e3;

		vrpn_float64 pos[3];
		vrpn_float64 quat[4];

		getHeadData(pos, quat);
		tracker->report_pose( 0, t, pos, quat );
		for( size_t i = 0; i < 20; ++i )
		{
			getMeveaData(i, pos, quat);
			mevea->report_pose( i, t, pos, quat );
		}

		// Send and receive all messages
		tracker->mainloop();
		mevea->mainloop();
		connection->mainloop();
		vl::msleep(msecs);
	}

	delete tracker;
	connection->removeReference();
}