#ifndef VL_DISTRIB_CONTAINER_HPP
#define VL_DISTRIB_CONTAINER_HPP

#include <vector>

#include <eq/client/object.h>

namespace vl 
{

	// For converting types we use specific functor that is parameterized
	// with two types, the input type and the output type.
	//
	// This class it self is abstract and have to be overloaded to provide
	// any functionality.
	// It's primary function though is to convert one type to another.
	// We use it to convert from ID to various types without casting
	// (where the template comes is to avoid casting).
	template<typename T, typename H>
	class ConversionFunctor
	{
		public :
			virtual T &operator()( H const &id ) = 0;
	};	// class ConversionFunctor

	template<typename T, typename H>
	class ConversionFunctor<T *, H>
	{
		public :
			virtual T *operator()( H const &id ) = 0;
	};

	template<typename T, typename H>
	class ConversionFunctor<T, H *>
	{
		public :
			virtual T &operator()( H const *id ) = 0;
	};

	template<typename T, typename H>
	class ConversionFunctor<T *, H *>
	{
		public :
			virtual T *operator()( H const *id ) = 0;
	};

	template< typename T >
	class DistributedContainer: public eq::Object
	{
		public :
			// Constructor which takes two Functors as parameters
			// add functor is called when an element is added to cont
			// del functor is called when an element is removed from cont
			DistributedContainer(
					ConversionFunctor<T, uint32_t> *add = 0,
					ConversionFunctor<T, uint32_t> *del = 0);

			~DistributedContainer( void ) {}

			void setAddCallback( ConversionFunctor<T, uint32_t> *add = 0 )
			{ _add_functor = add; }

			void setDelCallback( ConversionFunctor<T, uint32_t> *del = 0 )
			{ _del_functor = del; }

			void push( T const &obj )
			{
				// Add to new stack
				_add.push_back( obj.getID() );
				setDirty( DIRTY_ADD );

				// Add to the real array
				_array.push_back( obj );
				setDirty( DIRTY_SIZE );
			}

			T pop( void )
			{
				// Remove from the real array
				T &tmp = _array.top();
				_array.pop_back();
				setDirty( DIRTY_SIZE );

				// Add to deleted stack
				_del.push_back( tmp.getID() );
				setDirty( DIRTY_DEL );

				return tmp;
			}

			T &at( size_t index )
			{ return _array.at(index); }

			T const &at( size_t index ) const
			{ return _array.at(index); }

			void remove( size_t index )
			{
				typename std::vector<T>::iterator iter
					= _array.begin()+index;
				// Add to deleted stack
				_del.push_back( iter->getID() );
				setDirty( DIRTY_DEL );

				// Remove from the array
				_array.erase( iter );
				setDirty( DIRTY_SIZE );
			}

			void clear( void )
			{
				// Copy the current array to deleted array
				typename std::vector<T>::iterator iter = _array.begin();
				for( ; iter != _array.end(); ++iter )
				{ _del.push_back( iter->getID() ); }
				setDirty( DIRTY_DEL );

				// Clear added and deleted arrays
				_array.clear();
				_add.clear();

				setDirty( DIRTY_SIZE );
			}

			size_t size( void ) const
			{ return _array.size(); }

			// Equalizer overrides
			
			/* Object::serialize() */
			virtual void serialize( eq::net::DataOStream& os,
									const uint64_t dirtyBits )
			{
				std::cout << "Serializing container" << std::endl;
				if( DIRTY_SIZE & dirtyBits )
				{
					// This is more like a checksum for slaves
					os << _array.size();
				}
				// New array was modified so we have new elements added
				// between these versions.
				if( DIRTY_ADD )
				{
					os << _add;

					_add.clear();
				}
				// New delete elements have been added so we have destroyed
				// elements
				if( DIRTY_DEL )
				{
					os << _del;

					_del.clear();
				}
			}

			/* Object::deserialize() */
			virtual void deserialize( eq::net::DataIStream& is,
									  const uint64_t dirtyBits )
			{
				std::cout << "Deserializing container" << std::endl;
				size_t size = 0;
				if( DIRTY_SIZE & dirtyBits )
				{
					is >> size;
				}

				if( DIRTY_ADD & dirtyBits )
				{
					// Copy new add array
					is >> _add;

					// For now we disallow containers without callbacks
					EQASSERT( _add_functor );

					// Update the array using functor
					std::vector<uint32_t>::iterator iter = _add.begin();
					for( ; iter != _add.end(); ++iter )
					{ _array.push_back( (*_add_functor)( *iter ) ); }

					// Clear the add array as we don't need it anymore
					_add.clear();
				}
				EQASSERT( _add.empty() );
				
				if( DIRTY_DEL & dirtyBits )
				{
					// Copy new del array
					is >> _del;

					// Update the array using functor
					std::vector<uint32_t>::iterator iter = _del.begin();
					for( ; iter != _del.end(); ++iter )
					{
						// For now we disallow containers without callbacks
						EQASSERT( _del_functor );

						T tmp = (*_del_functor)( *iter );
						typename std::vector<T>::iterator del_iter =
							find( _array.begin(), _array.end(), tmp );
						if( del_iter != _array.end() )
						{ _array.erase( del_iter ); }
						else
						{
							// Should throw
						}
					}

					// Clear the del array, no longer needed
					_del.clear();
				}
				EQASSERT( _del.empty() );

				// We need to keep the master and slave containers consistent
				EQASSERT( size == _array.size() );
			}

			/* Object::ChangeType */
			virtual ChangeType getChangeType() const
			{ return DELTA; }

			enum DirtyBits
			{
				DIRTY_SIZE = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_ADD = eq::Object::DIRTY_CUSTOM << 1,
				DIRTY_DEL  = eq::Object::DIRTY_CUSTOM << 2,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 3
			};

		protected :
			std::vector<T> _array;

			// ID vectors of added and deleted elements
			std::vector<uint32_t> _add;
			std::vector<uint32_t> _del;

			// Functors to provide callbacks when a list of deleted and added
			// elements are received.
			ConversionFunctor<T, uint32_t> *_add_functor;
			ConversionFunctor<T, uint32_t> *_del_functor;
	};

	// Partial specialization for pointers
	template< typename T >
	class DistributedContainer<T *> : public eq::Object
	{
		public :
			// Constructor which takes two Functors as parameters
			// add functor is called when an element is added to cont
			// del functor is called when an element is removed from cont
			DistributedContainer(
					ConversionFunctor<T *, uint32_t> *add = 0,
					ConversionFunctor<T *, uint32_t> *del = 0 )
				: _add_functor(add), _del_functor(del)
			{}

			~DistributedContainer( void ) {}

			void setAddCallback( ConversionFunctor<T *, uint32_t> *add = 0 )
			{ _add_functor = add; }

			void setDelCallback( ConversionFunctor<T *, uint32_t> *del = 0 )
			{ _del_functor = del; }

			void push( T *obj )
			{
				// Add to new stack
				_add.push_back( obj->getID() );
				setDirty( DIRTY_ADD );

				// Add to the real array
				_array.push_back( obj );
				setDirty( DIRTY_SIZE );
			}

			T *pop( void )
			{
				// Remove from the real array
				T *tmp = _array->top();
				_array->pop_back();
				setDirty( DIRTY_SIZE );

				// Add to deleted stack
				_del.push_back( tmp->getID() );
				setDirty( DIRTY_DEL );

				return tmp;
			}

			T *at( size_t index )
			{ return _array.at(index); }

			T const *at( size_t index ) const
			{ return _array.at(index); }

			void remove( size_t index )
			{
				typename std::vector<T *>::iterator iter
					= _array.begin()+index;
				// Add to deleted stack
				_del.push_back( (*iter)->getID() );
				setDirty( DIRTY_DEL );

				// Remove from the array
				_array.erase( iter );
				setDirty( DIRTY_SIZE );
			}

			void clear( void )
			{
				// Copy the current array to deleted array
				typename std::vector<T *>::iterator iter = _array.begin();
				for( ; iter != _array.end(); ++iter )
				{ _del.push_back( (*iter)->getID() ); }
				setDirty( DIRTY_DEL );

				// Clear added and deleted arrays
				_array.clear();
				_add.clear();

				setDirty( DIRTY_SIZE );
			}

			size_t size( void ) const
			{ return _array.size(); }

			// Equalizer overrides
			
			/* Object::serialize() */
			virtual void serialize( eq::net::DataOStream& os,
									const uint64_t dirtyBits )
			{
				std::cout << "Serializing pointer container" << std::endl;
				if( DIRTY_SIZE & dirtyBits )
				{
					// This is more like a checksum for slaves
					os << _array.size();
				}
				// New array was modified so we have new elements added
				// between these versions.
				if( DIRTY_ADD )
				{
					os << _add;
					std::cout << "New elements n = " << _add.size() << std::endl;

					_add.clear();
				}
				// New delete elements have been added so we have destroyed
				// elements
				if( DIRTY_DEL )
				{
					os << _del;
					std::cout << "Deleted elements n = " << _del.size()
						<< std::endl;

					_del.clear();
				}
			}

			/* Object::deserialize() */
			virtual void deserialize( eq::net::DataIStream& is,
									  const uint64_t dirtyBits )
			{
				std::cout << "Deserializing pointer container" << std::endl;
				size_t size = 0;
				if( DIRTY_SIZE & dirtyBits )
				{ is >> size; }

				if( DIRTY_ADD & dirtyBits )
				{
					// Copy new add array
					is >> _add;
					std::cout << "New elements n = " << _add.size() << std::endl;

					// For now we disallow containers without callbacks
					if( _add_functor )
					{
						// Update the array using functor
						std::vector<uint32_t>::iterator iter = _add.begin();
						for( ; iter != _add.end(); ++iter )
						{ _array.push_back( (*_add_functor)( *iter ) ); }
					}
					else
					{ std::cerr << "No add functor" << std::endl; }

					// Clear the add array as we don't need it anymore
					_add.clear();
				}
				if( !_add.empty() )
				{ std::cerr << "Add container not empty" << std::endl; }
				
				if( DIRTY_DEL & dirtyBits )
				{
					// Copy new del array
					is >> _del;
					std::cout << "Deleted elements n = " << _del.size()
						<< std::endl;

					// Update the array using functor
					if( _del_functor )
					{
						std::vector<uint32_t>::iterator iter = _del.begin();
						for( ; iter != _del.end(); ++iter )
						{
							// For now we disallow containers without callbacks
							EQASSERT( _del_functor );

							T *tmp = (*_del_functor)( *iter );
							typename std::vector<T *>::iterator del_iter =
								find( _array.begin(), _array.end(), tmp );
							if( del_iter != _array.end() )
							{ _array.erase( del_iter ); }
							else
							{
								// Should throw
							}
						}
					}
					else
					{ std::cerr << "No del functor" << std::endl; }

					// Clear the del array, no longer needed
					_del.clear();
				}

				if( !_del.empty() )
				{ std::cerr << "Del container not empty" << std::endl; }

				// We need to keep the master and slave containers consistent
				if( size != _array.size() )
				{ std::cerr << "Incorrect size" << std::endl; }
			}

			/* Object::ChangeType */
			virtual ChangeType getChangeType() const
			{ return DELTA; }

			enum DirtyBits
			{
				DIRTY_SIZE = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_ADD = eq::Object::DIRTY_CUSTOM << 1,
				DIRTY_DEL  = eq::Object::DIRTY_CUSTOM << 2,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 3
			};

		protected :
			std::vector<T *> _array;

			// ID vectors of added and deleted elements
			std::vector<uint32_t> _add;
			std::vector<uint32_t> _del;

			// Functors to provide callbacks when a list of deleted and added
			// elements are received.
			ConversionFunctor<T *, uint32_t> *_add_functor;
			ConversionFunctor<T *, uint32_t> *_del_functor;

	};	// class 

}	// namespace vl

// Implementation of the basic template
template< typename T >
vl::DistributedContainer<T>::DistributedContainer(
		ConversionFunctor<T, uint32_t> *add,
		ConversionFunctor<T, uint32_t> *del )
	: _add_functor(add), _del_functor(del)
{}

// ENDOF Implementation of the basic template

// Implementation of the pointer specialization

// ENDOF Implementation of the pointer specialization

#endif
