/**	Joonatan Kuosa
 *	2010-11 initial implementation
 *
 */

#ifndef EQ_OGRE_CONFIG_EVENTS_HPP
#define EQ_OGRE_CONFIG_EVENTS_HPP

#include "event.hpp"
#include "config.hpp"
#include "tracker.hpp"

namespace eqOgre
{

// ConfigEvents
class ConfigOperation : public vl::BasicAction
{
public :
	ConfigOperation( void )
		: _config(0)
	{}

	void setConfig( Config *conf )
	{ _config = conf; }

	Config *getConfig( void )
	{ return _config; }

	virtual std::ostream & print( std::ostream & os ) const;

protected :
	Config *_config;

};	// class ConfigOperation

/// Sets the Head matrix in eqOgre::Config
class HeadTrackerAction : public vl::TransformAction
{
public :
	HeadTrackerAction( void )
		: _config(0)
	{}

	void setConfig( Config *conf )
	{ _config = conf; }

	Config *getConfig( void )
	{ return _config; }

	/// Callback function for TrackerTrigger
	/// Called when new data is received from the tracker
	/// Sets the head matrix in Config
	virtual void execute( vl::Transform const &data );

	virtual std::string const &getTypeName( void ) const;

protected :
	Config *_config;
};

class HeadTrackerActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new HeadTrackerAction; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


class QuitOperation : public ConfigOperation
{
public :
	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

};	// class QuitOperation

class QuitOperationFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new QuitOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

class ActivateCamera : public ConfigOperation
{
public :
	void setCamera( std::string const &name )
	{ _camera_name = name; }

	std::string const &getCamera( void ) const
	{ return _camera_name; }

	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

protected :
	std::string _camera_name;

};	// ActivateCamera

class ActivateCameraFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new ActivateCamera; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

};	// ActivateCameraFactory


class ReloadScene : public ConfigOperation
{
public :
	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

};

class ReloadSceneFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new ReloadScene; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


class ToggleMusic : public ConfigOperation
{
public :
	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

};

class ToggleMusicFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new ToggleMusic; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};







class EventManagerOperation : public vl::BasicAction
{
public :
	EventManagerOperation( void )
		: _event_man(0)
	{}

	void setManager( vl::EventManager *event_man )
	{ _event_man = event_man; }

	vl::EventManager *getManager( void )
	{ return _event_man; }

	virtual std::ostream & print( std::ostream & os ) const
	{
		os << *this << " event manager = " << _event_man << std::endl;
		return os;
	}

protected :
	vl::EventManager *_event_man;
};



class AddTransformOperation : public EventManagerOperation
{
public :
	AddTransformOperation( void )
		: _transform(0)
	{}

	void setTransformEvent( TransformationEvent *transform )
	{ _transform = transform; }

	TransformationEvent *getTransformEvent( void )
	{ return _transform; }

	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

private :
	TransformationEvent *_transform;

};

class AddTransformOperationFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new AddTransformOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};



class RemoveTransformOperation : public EventManagerOperation
{
public :
	RemoveTransformOperation( void )
		: _transform(0)
	{}

	void setTransformEvent( TransformationEvent *transform )
	{ _transform = transform; }

	TransformationEvent *getTransformEvent( void )
	{ return _transform; }


	virtual void execute( void );

	virtual std::string const &getTypeName( void ) const;

private :
	TransformationEvent *_transform;

};

class RemoveTransformOperationFactory : public vl::ActionFactory
{
public :
	virtual vl::ActionPtr create( void )
	{ return new RemoveTransformOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};


}	// namespace eqOgre

#endif	// EQ_OGRE_CONFIG_EVENTS_HPP