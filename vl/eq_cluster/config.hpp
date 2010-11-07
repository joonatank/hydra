#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

#include "frame_data.hpp"
#include "settings.hpp"
#include "eq_settings.hpp"
#include "base/exceptions.hpp"
#include "transform_event.hpp"
#include "event.hpp"

// python
#include "python.hpp"
#include "event_manager.hpp"

namespace eqOgre
{

    class Config : public eq::Config
    {
    public:
		Config( eq::base::RefPtr< eq::Server > parent );

		/** @sa eq::Config::init. */
		virtual bool init( uint32_t const initID );

		/** @sa eq::Config::exit. */
//		virtual bool exit (void);

        /** @sa eq::Config::handleEvent */
		virtual bool handleEvent( const eq::ConfigEvent* event );

		virtual uint32_t startFrame (const uint32_t frameID);

		void mapData( uint32_t const initDataID );

		/// These are mostly called from other eqOgre classes
		/// which need the settings to be distributed so it does not make sense
		/// to return the base class at this point.
		eqOgre::SettingsRefPtr getSettings( void )
		{ return _settings; }

		eqOgre::SettingsRefPtr const getSettings( void ) const
		{ return _settings; }

		// TODO this could be done with vl::SettingsRefPtr or vl::Settings
		// if we have a valid copy constructor
		void setSettings( eqOgre::SettingsRefPtr settings );

		bool addEvent( TransformationEvent const &event );

		bool removeEvent( TransformationEvent const &event );

		bool hasEvent( TransformationEvent const &event );

		void addSceneNode( SceneNode *node );

		void removeSceneNode( SceneNode *node );

		void updateSceneVersion( void )
		{ _frame_data.updateSceneVersion(); }

	protected :
		virtual ~Config (void);

		void _createTracker( vl::SettingsRefPtr settings );
		void _setHeadMatrix( Ogre::Matrix4 const &m );

		void _initPython( void );
		void _runPythonScript( std::string const &scriptFile );

		bool _handleKeyPressEvent( const eq::KeyEvent& event );
		bool _handleKeyReleaseEvent( const eq::KeyEvent& event );
		bool _handleMousePressEvent( const eq::PointerEvent& event );
		bool _handleMouseReleaseEvent( const eq::PointerEvent& event );
		bool _handleMouseMotionEvent( const eq::PointerEvent& event );
		// TODO replace the MagellanEvent with a real JoystickEvent
		bool _handleJoystickEvent( const eq::MagellanEvent& event );

		vl::TrackerRefPtr _tracker;

		eqOgre::SettingsRefPtr _settings;

		FrameData _frame_data;

		// NOTE we need to use Event pointer because Events can be inherited
		std::vector<Event *> _events;
		std::vector<TransformationEvent> _trans_events;

		EventManager *_event_manager;

		// Python related
		python::object _global;

	};	// class Config



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

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

};	// class QuitOperation


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

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
};




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

		// FIXME this needs to access EventHandler not config
//		_config->addEvent( _transform );
	}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;

private :
	TransformationEvent *_transform;

};


class RemoveTransformOperation : public EventManagerOperation
{
public :
	// FIXME setting parameters when using factory
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

		// FIXME this needs to access EventHandler not config
//		_config->removeEvent( _transform );
	}

	virtual std::string const &getTypeName( void ) const
	{ return TYPENAME; }

	static const std::string TYPENAME;
private :

	TransformationEvent *_transform;
};

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
