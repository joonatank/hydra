
#ifndef VL_GRAPH_MOVABLE_OBJECT_HPP
#define VL_GRAPH_MOVABLE_OBJECT_HPP

#include <eq/client/object.h>

#include "typedefs.hpp"
#include "utility.hpp"

namespace vl
{

namespace graph
{
	class MovableObject : public eq::Object
	{
		public :
			MovableObject( std::string const &name,
					vl::NamedValuePairList const &params )
				: eq::Object(), _params(params)
			{
				eq::Object::setName( name );
			}

			// Equalizer overrides
			
			/* Object::serialize() */
			virtual void serialize( eq::net::DataOStream& os,
									const uint64_t dirtyBits )
			{
				if( DIRTY_PARAMS & dirtyBits )
				{
					os << _params;
				}
			}

			/* Object::deserialize() */
			virtual void deserialize( eq::net::DataIStream& is,
									  const uint64_t dirtyBits )
			{
				if( DIRTY_PARAMS & dirtyBits )
				{
					// TODO we should have a callback for reloading
					// if the params have been changed.
					is >> _params;
				}
			}

			/* Object::ChangeType */
			virtual ChangeType getChangeType() const
			{ return DELTA; }

			enum DirtyBits
			{
				// Dirty for the object container
				DIRTY_PARAMS = eq::Object::DIRTY_CUSTOM << 0,
				DIRTY_CUSTOM = eq::Object::DIRTY_CUSTOM << 1
			};

		protected :
			vl::NamedValuePairList _params;

	};	// class MovableObject

}	// namespace graph

}	// namespace vl

#endif
