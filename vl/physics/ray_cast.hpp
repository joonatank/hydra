#ifndef RAY_CAST_HPP
#define RAY_CAST_HPP

#include "math/transform.hpp"
// Necessary for HYDRA_API
#include "defines.hpp"
#include "typedefs.hpp"

namespace vl {
namespace physics {

enum RAY_COLLISION_TYPES {
	STATIC = (1 << 0),
	KINEMATIC = (1 << 1),
	DYNAMIC = (1 << 2)
};

typedef std::vector< boost::shared_ptr<Ogre::Vector3> > VectorPtrList; 

struct RayResult
{
	VectorPtrList m_hitpoints_world;
	VectorPtrList m_hitpoints_local;
	VectorPtrList m_hitnormals_world;
	VectorPtrList m_hitnormals_local;
	std::vector<vl::scalar> m_hitdistance;
	GameObjectList m_ordered_colliders;
};

class RayCast
{
public:
	//Miten luodaan? Parametreina vain maailma johon luodaan?
	RayCast(WorldRefPtr world);
	virtual ~RayCast(void);
	RayResult update(void);
	//RayResult update(Ogre::Vector3 pos, Ogre::Quaternion &dir);
	//RayResult update(Ogre::Vector3 &rayfromworld, Ogre::Vector3 &raytoworld);
	//CollisionObject getClosestCollider();
	//CollisionObject getFarthestCollider();
	//vl::scalar getClosestDistance();
	void setStartPosition(Ogre::Vector3 &pos);
	void setDirection(Ogre::Quaternion &dir);
	vl::scalar getLength(void);

	Ogre::Vector3 &getStartPosition(void)
	{ 
		return m_start_position;
	};
	
	Ogre::Vector3 &getEndPosition(void)
	{
		return m_end_position;
	};
	
	short int getMask(void);
	void setMask(short int mask);
	RayResult *getResults(void);

private:
	
	short int m_mask;
	vl::scalar m_length;

	Ogre::Vector3 m_start_position;
	Ogre::Vector3 m_end_position;
	
	
	WorldRefPtr world;
	
	RayResult *m_last_result;
	
	//short int *m_filter_group;
	
	Ogre::Vector3 _calculateEndPoint(Ogre::Quaternion &dir, vl::scalar &length);

}; //class RayCast

}; //NAMESPACE physics
}; //NAMESPACE vl
#endif	// RAY_CAST_HPP