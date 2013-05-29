/*
#include "input.hpp"
#include "mouse_event.hpp"
#include <boost/signal.hpp>
#include "OIS/OISMouse.h"
#include "math/math.hpp"

class Mickey : public OIS::Mouse, public vl::InputDevice
{
public:
	void addHandler(MouseHandlerRefPtr handler);
	void removeHandler(MouseHandlerRefPtr handler);
	void removeHandler(int idx);
protected:
private:
	std::vector<MouseHandlerRefPtr> _handlers;

}; //Mickey mouse

struct MouseHandler
{
	MouseHandler(void) {}

	virtual ~MouseHandler(void) {}

	virtual void execute(MouseEvent const &evt) = 0;

};	// struct MouseHandler

struct MouseEvent
{
	MouseEvent(void) : axis_x(0), axis_y(0), buttons(0) {}

	~MouseEvent(void) {}

	vl::scalar axis_x;
	vl::scalar axis_y;
	uint32_t buttons;

};

*/