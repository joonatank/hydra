/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file: ogre_axes.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#ifndef HYDRA_OGRE_AXES_HPP
#define HYDRA_OGRE_AXES_HPP

// Base class
#include <OGRE/OgreMovableObject.h>

#include "math/types.hpp"

namespace vl
{

namespace ogre
{

class Axes : public Ogre::MovableObject, public Ogre::Renderable
{
public :
	Axes(vl::scalar length);

	virtual ~Axes(void);

	// from MovableObject
	virtual void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

	void getWorldTransforms(Ogre::Matrix4 *xform) const;
	Ogre::Real getBoundingRadius(void) const {return 100*100;};
	Ogre::Real getSquaredViewDepth(const Ogre::Camera *cam) const {return 0;};
	Ogre::AxisAlignedBox const &getBoundingBox(void) const {return Ogre::AxisAlignedBox::BOX_INFINITE;}

	std::string const &getMovableType(void) const 
	{
		static Ogre::String movType = "OgreAxes";
		return movType;
	}

	// from renderable
	void getRenderOperation(Ogre::RenderOperation &op);
	Ogre::MaterialPtr const &getMaterial(void) const 
	{ return mpMaterial; }

	Ogre::LightList const &getLights(void) const 
	{ return mLList; }

	void setLength(vl::scalar len);

	vl::scalar getLength(void) const
	{ return _length; }

	/******************************** protected methods and overload **************/
protected:
	void _setDirty(void)
	{ mNeedUpdate = true; }

	void _setupGeometry(void);

	/// from MovableObject, automatic methods called from redender system

	void _updateRenderQueue(Ogre::RenderQueue* queue);

	/************************** protected data *******************************/
protected:
	Ogre::MaterialPtr mpMaterial;
	Ogre::RenderOperation mRenderOp;
	Ogre::LightList mLList;

	bool mNeedUpdate;

	vl::scalar _length;

};	// Axes

}	// namespace ogre

}	// namespace vl

#endif	// HYDRA_OGRE_AXES_HPP
