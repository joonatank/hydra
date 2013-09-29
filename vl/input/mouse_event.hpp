#ifndef HYDRA_INPUT_MOUSE_EVENT_HPP
#define HYDRA_INPUT_MOUSE_EVENT_HPP

// Necessary for ostream implementation
#include <iostream>

// Necessary for vl::scalar and uintX_t types
#include "math/math.hpp"
#include "cluster/message.hpp"


namespace vl
{

struct MouseEvent
{
	MouseEvent(void) :
			X(),
			Y(),
			buttons(0)
			//head_position(vl::Vector3()),
			//head_orientation(vl::Quaternion())
	{}

	~MouseEvent(void) {}
	
	//This has to be exactly like OIS::MouseButtonID
	enum BUTTON
	{
		// @todo: create a converter from OIS to this, otherwise it's very error prone and impossible to debug.
		MB_L = 0,
		MB_R,
		MB_M,
		MB_3,
		MB_4,
		MB_5,
		MB_6,
		MB_7
	};
	

	struct Axis
	{
		Axis(void) :abs(0), rel(0) {}
		int abs, rel;
		void clear(void)
		{
			abs = rel = 0;
		}
		
	};

	bool isButtonDown(BUTTON bt) const
	{
		return( buttons & (1 << bt) );
	}
	
	
	Axis			X;
	Axis			Y;
	Axis			Z;
	uint32_t		buttons;	//Bit position is button!

	//We can get these from python through player object:
	//vl::Vector3		head_position;
	//vl::Quaternion	head_orientation;
	//This isn't needed anymore because Joonatan exposed renderer getter in python:
	//Ogre::Matrix4	view_projection;
	//int					window_height;
	//int					window_width;
};


//For printing the event (debugging purposes)
inline std::ostream &
operator<<(std::ostream &os, MouseEvent const &evt)
{
	//Absolute and relative (~speed) position:
	os << "Absolute = ( " << evt.X.abs << ", " << evt.Y.abs << ", " << evt.Z.abs << " )" << std::endl;
	os << "Relative = ( " << evt.X.rel << ", " << evt.Y.rel << ", " << evt.Z.rel << " )" << std::endl;

	//Buttons:
	for(size_t i = 0; i < 8; ++i)
	{
		os << "Button " << i+1 << ": " << evt.isButtonDown( vl::MouseEvent::BUTTON(i) ) << std::endl;
	}

	//Head position, orientation and viewmatrix:
	//os << "Head_Position = ( " << evt.head_position << " )" << std::endl;
	//os << "Head_Orientation = ( " << evt.head_orientation << " )" << std::endl;
	
	return os;
}

namespace cluster {
/*
template<>
inline ByteStream &
operator<< <Ogre::Matrix4 const>(ByteStream &msg, Ogre::Matrix4 const &mat)
{
	// @todo: We should optimize this
	for(size_t row = 0; row < 4; ++row)
	{
		for(size_t col = 0; col < 4; ++col)
		{
			msg << mat[row][col];
		}
	}
	return msg;
}


template<>
inline ByteStream &
operator>> <Ogre::Matrix4>(ByteStream &msg, Ogre::Matrix4 &mat)
{
	// @todo: optimize this, could be copied as a whole, no need for loop.
	
	for(size_t row = 0; row < 4; ++row)
	{
		for(size_t col = 0; col < 4; ++col)
		{
			msg >> mat[row][col];
		}
	}
	return msg;
}

*/

template<>
inline ByteStream &
operator<< <vl::MouseEvent>(ByteStream &msg, vl::MouseEvent const &evt)
{
	msg << evt.X.abs << evt.X.rel << evt.Y.abs << evt.Y.rel << evt.Z.abs << evt.Z.rel;
	msg	<< evt.buttons;
	
	return msg;
}


template<>
inline ByteStream &
operator>> <vl::MouseEvent>(ByteStream &msg, vl::MouseEvent &evt)
{
	
	msg >> evt.X.abs >> evt.X.rel >> evt.Y.abs >> evt.Y.rel >> evt.Z.abs >> evt.Z.rel;
	msg	>> evt.buttons;
	
	return msg;
}


} //namespace cluster
} //namespace vl


#endif 