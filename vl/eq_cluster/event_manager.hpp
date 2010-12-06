#ifndef EQ_OGRE_EVENT_MANAGER_HPP
#define EQ_OGRE_EVENT_MANAGER_HPP

#include "event.hpp"

#include <vector>

namespace vl
{

class EventManager
{
public :
	Event *createEvent( std::string const &type );

	Trigger *createTrigger( std::string const &type );

	// TODO naming of these function
	// should they be registerOperationType or addOperationFactory or some
	// combination of the two themes
	// TODO factories can not be copied, either we need to provide
	// copy function to them (which returns a new allocated ptr) or we need to
	// get python to create objects with long life time
	// probably the later
	// NOTE the factories can infact be copied because they don't contain
	// any data, so they can be assigned to each other with ease

	/// Parameter : Pointer to the factory which user wants to add
	/// 	The factory is expected to be alive till the factory is removed
	/// If a factory with that object typeName is not found the factory
	/// is added to the list of factories.
	/// If a factory with the same object typeName is found this will throw
	void addEventFactory( EventFactory *fact );

	void addTriggerFactory( TriggerFactory *fact );

	// Manipulate the event stack
	bool addEvent( Event *event );

	bool removeEvent( Event *event );

	bool hasEvent( Event *event );

	bool processEvents( Trigger *trig );

	Event *findEvent( std::string const &name );

	void printEvents( std::ostream &os ) const;

private :
	std::vector<Event *> _events;

	std::vector<TriggerFactory *> _trigger_factories;
	std::vector<EventFactory *> _event_factories;

};	// class EventManager

inline std::ostream &operator<<( std::ostream &os, EventManager const &man )
{
	os << "Event Manager " << std::endl;
	man.printEvents(os);
	return os;
}

}	// namespace eqOgre



#endif	// EQ_OGRE_EVENT_MANAGER_HPP