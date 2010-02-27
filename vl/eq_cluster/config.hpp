#ifndef EQ_OGRE_CONFIG_H
#define EQ_OGRE_CONFIG_H

#include <eq/eq.h>

// members
//#include "initData.hpp"
#include "base/message.hpp"
//#include "server_command.hpp"
#include "base/fifo_buffer.hpp"
//#include "frameData.h"
//#include "tracker.h"
//#include "cameraAnimation.h"

//#include "ogreApplication.hpp"

namespace eqOgre
{
//    typedef std::vector< Model* >     ModelVector;
 //   typedef std::vector< ModelDist* > ModelDistVector;

//	class Root;
    /**
     * The configuration, run be the EqPly application. 
     *
     * A configuration instance manages configuration-specific data: it
     * distributes the initialization and model data, updates frame-specific
     * data and manages frame generation based on event handling. 
     */
    class Config : public eq::Config
    {
    public:
        Config( eq::base::RefPtr< eq::Server > parent );

        /** @sa eq::Config::init. */
        virtual bool init( uint32_t initID );

        /** @sa eq::Config::exit. */
        virtual bool exit (void);

        /** @sa eq::Config::startFrame. */
 //       virtual uint32_t startFrame (void);

		/*
		void setClientFifo( vl::base::fifo_buffer<vl::server::Command *> *buf )
		{ _client_fifo = buf; }

		vl::base::fifo_buffer<vl::server::Command *> *getClientFifo( void )
		{ return _client_fifo; }
		*/

		void setNodeFifo( vl::base::fifo_buffer<vl::base::Message *> *buf )
		{ _node_fifo = buf; }

		vl::base::fifo_buffer<vl::base::Message *> *getNodeFifo( void )
		{ return _node_fifo; }

        /** Map per-config data to the local node process */
   //     void mapData( const uint32_t initDataID );

        /** Unmap per-config data to the local node process */
    //    void unmapData( void );

        /** @return the requested, default model or 0. */
 //       const Model* getModel( const uint32_t id );

        /** @sa eq::Config::handleEvent */
		virtual bool handleEvent( const eq::ConfigEvent* event );

        /** @return true if an event required a redraw. */
   //     bool needsRedraw (void);

		// Functions used by 
		/*
		void setRoot( eqOgre::Root *root )
		{ _root = root; }

		eqOgre::Root *getRoot( void )
		{ return _root; }
		*/

    protected:
        virtual ~Config (void);

       // eq::Canvas* _currentCanvas;

//		vl::base::fifo_buffer<vl::server::Command *> *_client_fifo;
		vl::base::fifo_buffer<vl::base::Message *> *_node_fifo;

    private:
        void _deregisterData();

    };	// class Config

}	// namespace eqOgre

#endif // EQ_OGRE_CONFIG_H
