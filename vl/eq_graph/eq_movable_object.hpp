
#ifndef VL_EQ_GRAPH_MOVABLE_OBJECT_HPP
#define VL_EQ_GRAPH_MOVABLE_OBJECT_HPP

#include <eq/client/object.h>

#include "base/typedefs.hpp"
#include "eq_cluster/utility.hpp"

#include "interface/movable_object.hpp"

namespace vl
{

namespace cl
{
	class MovableObject : public eq::Object
	{
		public :
			MovableObject( std::string const &name,
					vl::NamedValuePairList const &params )
				: eq::Object(), _params(params)
			{
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
