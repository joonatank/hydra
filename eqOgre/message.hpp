#ifndef VL_BASE_MESSAGE_HPP
#define VL_BASE_MESSAGE_HPP

#include "command.hpp"

// Needed for pointer transfer commands
#include <OgreRenderTarget.h>
//#include <OgreRoot.h>
#include <OgreCamera.h>
#include "graph/root.hpp"

namespace vl
{

namespace base
{
	// Dummy class where to derive different types of Messages used to
	// transmit dynamically allocated commands so that they can be
	// put to the same container (buffer).
	//
	// TODO create a MessageBuffer class that derives from fifo_buffer
	// specializes in Messages and provides a memory pool where to get
	// messages.
	// TODO also needs a reference holder (movable, not copyable) similar to
	// std::auto_ptr, when it goes out of scope Message is returned to the
	// pool. This has to be done in a way it's not intrussive to threads, well
	// we might assume that a message allocated from the pool is passed to
	// specific thread (reader of the buffer) and writer never releases reference,
	// then fifo_buffer would work in saving non-used references.
	// Other possibility would be to use, array to keep the unused references
	// and have one fifo_buffer per thread where they save the released
	// references as only one thread is able to allocate new Messages we can
	// clear the release fifos when new Message is allocated.
	// - Memory consumption worst case message_fifo + 2*release_fifo + array
	// 	 would equal N*sizeof(Message)+N*8*3 (as the message fifo and release
	// 	 fifo would have to be at least N at length to hold all the references,
	// 	 in worst case one thread has released all threads without the other
	// 	 allocating any new ones, or pushed all allocated to message_fifo
	// 	 without other releasing them).
	// 	  -> Example assuming N=256, sizeof(Message) = 80bytes (argued below)
	// 	  	 then MessageBuffer would use
	// 	  	 256*80bytes + 256*24bytes = 256*104bytes = 26 kbytes,
	// 	  	 where 6kbytes would be overhead (that is 23% overhead).
	class Message
	{
		public :
			Message( vl::base::CMD_TYPE type )
				: cmdType( type )
			{}

			CMD_TYPE cmdType;
	};


	// Class to Transfer the ownership of an object across threads
	// Transmits pointer to allocated object from one thread (responsible for
	// creating objects). Creation and deletion of objects without locking
	// is difficult if we use factories (which we do) so we need one
	// thread to be responsible for this.
	//
	// TODO we should derive both message types ownership transfer and
	// commands from same master class (so they can be put on the same queue
	// and so on).
	class TransferObject : public vl::base::Message
	{
		public :
			TransferObject( CMD_TYPE cmd )
				: Message( cmd )
			{}

	};

	// Command to transmit Root pointer from one thread to another
	// Used to create objects in Node thread and pass them to Pipe thread
	class RootTransfer: public vl::base::TransferObject
	{
		public :
			RootTransfer( vl::graph::Root *ptr )
				: TransferObject( CMD_ROOT_TRANS ),
				  root( ptr )
			{}

			// Function operator
			// Implicit casting to Ogre::Root
			vl::graph::Root *operator()( void )
			{ return root; }

			vl::graph::Root *root;
	};

	// Command to transmit a camera created in another thread to another
	// Used to create objects in Node thread and pass them to Pipe thread
	class CameraTransfer: public vl::base::TransferObject
	{
		public :
			CameraTransfer( Ogre::Camera *cam )
				: TransferObject( CMD_CAMERA_TRANS ),
				  camera( cam )
			{}

			// Function operator
			Ogre::Camera *operator()( void )
			{ return camera; }

			Ogre::Camera *camera;
	};

	// Command to transmit RenderTarget (Window) from one thread to another
	// Used to create objects in Node thread and pass them to Pipe thread
	class WindowTransfer : public vl::base::TransferObject
	{
		public :
			WindowTransfer( Ogre::RenderTarget *t )
				: TransferObject( CMD_WINDOW_TRANS ),
				  target( t )
			{}

			// Function operator
			Ogre::RenderTarget *operator()( void )
			{ return target; }

			Ogre::RenderTarget *target;
	};

}	// namespace base

}	// namespace vl

#endif
