

#include "base/sleep.hpp"

#include "../tracking_fixture.hpp"

int main (unsigned argc, char *argv[])
{
	TrackerServerFixture fixture;
	
	// Loop forever calling the mainloop()s for all devices and the connection
	while (1)
	{
		fixture.mainloop();
		vl::msleep(1);
	}
}