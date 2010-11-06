#ifndef EQ_OGRE_EVENT_MANAGER_HPP
#define EQ_OGRE_EVENT_MANAGER_HPP

#include "event.hpp"

#include <vector>

namespace eqOgre
{

class EventManager
{
public :
	Event *createEvent( std::string const &type );

	Trigger *createTrigger( std::string const &type );

	Operation *createOperation( std::string const &type );

	// Manipulate the event stack
	bool addEvent( Event *event );

	bool removeEvent( Event *event );

	bool hasEvent( Event *event );

	void processEvents( Trigger *trig );

	Event *findEvent( std::string const &name );

private :
	std::vector<Event *> _events;

};	// class EventManager

}	// namespace eqOgre

#endif	// EQ_OGRE_EVENT_MANAGER_HPP