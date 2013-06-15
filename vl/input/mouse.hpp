/* Do we need this device abstraction and handlers?

#include "input.hpp"
#include "mouse_event.hpp"
#include <boost/signal.hpp>
#include "OIS/OISMouse.h"
#include "math/math.hpp"

class Mouse : public vl::InputDevice
{
public:
	Mouse(void);
	virtual ~Mouse(void);

	void addHandler(MouseHandlerRefPtr handler);
	void removeHandler(MouseHandlerRefPtr handler);
	int addListener(MouseListenerRefPtr listener);
protected:
private:
	std::vector<MouseListenerRefPtr> _listeners;
	std::vector<MouseHandlerRefPtr> _handlers;

}; //Mickey mouse

struct MouseHandler
{
	MouseHandler(void) {}

	virtual ~MouseHandler(void) {}

	virtual void execute(MouseEvent const &evt) = 0;

};	// struct MouseHandler

*/