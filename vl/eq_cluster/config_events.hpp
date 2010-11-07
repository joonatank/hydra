#ifndef EQ_OGRE_CONFIG_EVENTS_HPP
#define EQ_OGRE_CONFIG_EVENTS_HPP

#include "event.hpp"
#include "config.hpp"

namespace eqOgre
{
	
// ConfigEvents
class ConfigOperation : public Operation
{
public :
	ConfigOperation( void )
		: _config(0)
	{}

	void setConfig( Config *conf )
	{ _config = conf; }

	Config *getConfig( void )
	{ return _config; }

	virtual std::ostream & print( std::ostream & os ) const
	{
		Operation::print(os) << " config = " << _config << std::endl;
		return os;
	}

protected :
	Config *_config;

};	// class ConfigOperation


class QuitOperation : public ConfigOperation
{
public :
	// FIXME setting parameters when using factory
	QuitOperation( void )
	{}

	virtual void execute( void )
	{
		if( !_config )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_config->stopRunning();
	}

	virtual std::string const &getTypeName( void ) const;

};	// class QuitOperation

class QuitOperationFactory : public OperationFactory
{
public :
	virtual Operation *create( void )
	{ return new QuitOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

inline std::string const &
QuitOperation::getTypeName( void ) const
{ return QuitOperationFactory::TYPENAME; }

class ReloadScene : public ConfigOperation
{
public :
	ReloadScene( void )
	{}

	virtual void execute( void )
	{
		if( !_config )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_config->updateSceneVersion();
	}

	virtual std::string const &getTypeName( void ) const;

};

class ReloadSceneFactory : public OperationFactory
{
public :
	virtual Operation *create( void )
	{ return new ReloadScene; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

inline std::string const &
ReloadScene::getTypeName( void ) const
{ return ReloadSceneFactory::TYPENAME; }

class EventManagerOperation : public Operation
{
public :
	EventManagerOperation( void )
		: _event_man(0)
	{}

	void setManager( EventManager *event_man )
	{ _event_man = event_man; }

	EventManager *getManager( void )
	{ return _event_man; }

	virtual std::ostream & print( std::ostream & os ) const
	{
		Operation::print(os) << " event manager = " << _event_man << std::endl;
		return os;
	}

protected :
	EventManager *_event_man;
};

class AddTransformOperation : public EventManagerOperation
{
public :
	// FIXME setting parameters when using factory
	AddTransformOperation( void )
		: _transform(0)
	{}

	void setTransformEvent( TransformationEvent *transform )
	{ _transform = transform; }

	TransformationEvent *getTransformEvent( void )
	{ return _transform; }

	virtual void execute( void )
	{
		if( !_event_man )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_event_man->addEvent( _transform );
	}

	virtual std::string const &getTypeName( void ) const;

private :
	TransformationEvent *_transform;

};

class AddTransformOperationFactory : public OperationFactory
{
public :
	virtual Operation *create( void )
	{ return new AddTransformOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

inline std::string const &
AddTransformOperation::getTypeName( void ) const
{ return AddTransformOperationFactory::TYPENAME; }

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


	virtual void execute( void )
	{
		if( !_event_man )
		{ BOOST_THROW_EXCEPTION( vl::null_pointer() ); }

		_event_man->removeEvent( _transform );
	}

	virtual std::string const &getTypeName( void ) const;

private :
	TransformationEvent *_transform;

};

class RemoveTransformOperationFactory : public OperationFactory
{
public :
	virtual Operation *create( void )
	{ return new RemoveTransformOperation; }

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};

inline std::string const &
RemoveTransformOperation::getTypeName( void ) const
{ return RemoveTransformOperationFactory::TYPENAME; }

}	// namespace eqOgre

#endif	// EQ_OGRE_CONFIG_EVENTS_HPP