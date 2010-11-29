
#include <OGRE/OgreVector3.h>

#include "math/math.hpp"

int main( int argc, char **argv )
{
	if( argc < 2 )
	{
		std::cout << "Usage : " << argv[0] << " x,y,z " << std::endl
			<< " where x is the rotation around x axis in degrees" << std::endl
			<< " where y is the rotation around y axis in degrees" << std::endl
			<< " where z is the rotation around z axis in degrees" << std::endl;
		return 0;
	}

	std::stringstream ss( argv[1] );
	Ogre::Real x, y, z;
	char tmp;
	ss >> x >> tmp >> y >> tmp >> z;

	Ogre::Quaternion q;
	vl::fromEulerAngles(q, Ogre::Radian( Ogre::Degree(x) ),
						Ogre::Radian( Ogre::Degree(y) ),
						Ogre::Radian( Ogre::Degree(z) ) );
	std::cout << "(" << x << ", " << y << ", " << z << ")" << " = "
		<< q << std::endl;

	return 0;
}