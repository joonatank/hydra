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
			buttons(0),
			head_position(vl::Vector3()),
			head_orientation(vl::Quaternion()),
			view_projection(Ogre::Matrix4())
	{}

	~MouseEvent(void) {}
	
	//This has to be exactly like OIS::MouseButtonID
	enum BUTTON
	{
		// @todo tripla tsekkaa mätsääkö OISSIIN!
		MB_L = (1 << 0),
		MB_R = (1 << 1),
		MB_M = (1 << 2),
		MB_3 = (1 << 3),
		MB_4 = (1 << 4),
		MB_5 = (1 << 5),
		MB_6 = (1 << 6),
		MB_7 = (1 << 7)
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
		return(buttons & bt);
	}
	
	
	Axis			X;
	Axis			Y;
	Axis			Z;
	uint32_t		buttons;
	vl::Vector3		head_position;
	vl::Quaternion	head_orientation;
	Ogre::Matrix4	view_projection;
	//int					window_height;
	//int					window_width;
};


inline std::ostream &
operator<<(std::ostream &os, MouseEvent const &evt)
{
	//Absolute and relative (~speed) position:
	os << "Absolute = (" << evt.X.abs << ", " << evt.Y.abs << evt.Z.abs << ")" << std::endl;
	os << "Relative = (" << evt.X.rel << ", " << evt.Y.rel << evt.Z.rel << ")" << std::endl;

	//Buttons:
	for(size_t i = 0; i < 8; ++i)
	{
		os << "Button " << i << " : " << evt.isButtonDown( vl::MouseEvent::BUTTON(1 << i) ) << std::endl;
	}

	//Head position, orientation and viewmatrix:
	os << "Head_Position = (" << evt.head_position << ")" << std::endl;
	os << "Head_Orientation = (" << evt.head_orientation << ")" << std::endl;
	os << "ViewMatrix = {" << evt.view_projection << "}" << std::endl;

	return os;
}


namespace cluster {

template<>
inline ByteStream &
operator<< <vl::MouseEvent const>(ByteStream &msg, vl::MouseEvent const &evt)
{
	msg << evt.X.abs << evt.X.rel; 
	//<< evt.Y.abs << evt.Y.rel << evt.Z.abs << evt.Z.rel << evt.buttons << evt.head_position << evt.head_orientation;
	
	return msg;
}


template<>
inline ByteStream &
operator>> <vl::MouseEvent>(ByteStream &msg, vl::MouseEvent &evt)
{
	
	msg >> evt.X.abs >> evt.X.rel;
		//>> evt.Y.abs >> evt.Y.rel >> evt.Z.abs >> evt.Z.rel;
	//msg >> evt.buttons;
	//msg >> evt.head_position;
	//msg >> evt.head_orientation;

	return msg;
}


template<>
inline ByteStream &
vl::cluster::operator<< <Ogre::Matrix4 const>(ByteStream &msg, Ogre::Matrix4 const &mat)
{
	// @todo: optimize this, could be copied as a whole, no need for loop.
	for( size_t i = 0; i < 16; ++i)
	{
		msg << *mat[i];	
	}
	
	return msg;
};


template<>
inline ByteStream &
operator>> <Ogre::Matrix4>(ByteStream &msg, Ogre::Matrix4 &mat)
{
	// @todo: optimize this, could be copied as a whole, no need for loop.
	//for( size_t i = 0; i < 16; ++i)
	//{
	//	msg >> *mat[i];
	//}
	
	return msg;
}


} //namespace cluster
} //namespace vl


#endif 