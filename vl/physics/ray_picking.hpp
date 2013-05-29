#ifndef RAY_PICKING_HPP
#define RAY_PICKING_HPP

#include "math/transform.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"
#include "typedefs.hpp"
//necessary for inheriting the raycallback:
#include "BulletCollision\CollisionDispatch\btCollisionWorld.h"

namespace vl {
namespace physics {

	class RayPicker
	{
		void update(void);

	}




}; //NAMESPACE physics
}; //NAMESPACE vl
#endif	// RAY_PICKING_HPP