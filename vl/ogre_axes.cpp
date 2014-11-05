/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-02
 *	@file: ogre_axes.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "ogre_axes.hpp"

#include <OGRE/Ogre.h>

vl::ogre::Axes::Axes(vl::scalar length)
	: mNeedUpdate(true)
	, _length(length)
{
	// Create material, has to be first because _setupGeometry needs it
	mpMaterial = Ogre::MaterialManager::getSingletonPtr()->getByName("AxesMaterial");

	if(mpMaterial.isNull())
	{
		mpMaterial = Ogre::MaterialManager::getSingletonPtr()
			->create("AxesMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		mpMaterial->load();

		mpMaterial->getTechnique(0)->getPass(0)->setVertexColourTracking(Ogre::TVC_EMISSIVE);
	}

    mRenderOp.vertexData = NULL;
	this->_setupGeometry();
}

vl::ogre::Axes::~Axes(void)
{
	if(mRenderOp.vertexData)
	{ delete mRenderOp.vertexData; }
	// Do not remove material as all the axes objects use it
}

void
vl::ogre::Axes::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
{
	// Empty by design
}

void
vl::ogre::Axes::getWorldTransforms(Ogre::Matrix4 *xform) const
{
	if(this->isVisible())
	{
		if(mParentNode)
		{ *xform = mParentNode->_getFullTransform(); }
	}
}

void
vl::ogre::Axes::getRenderOperation(Ogre::RenderOperation &op)
{
	if (this->isVisible())
	{
		if (mNeedUpdate)
			this->_setupGeometry();
		op = mRenderOp;
	}
}

void
vl::ogre::Axes::setLength(vl::scalar len)
{
	if(_length != len)
	{
		_length = len;
		_setDirty();
	}
}

void
vl::ogre::Axes::_setupGeometry(void)
{
	assert(!mpMaterial.isNull());

	// Three lines, every line with two vertices
	unsigned int vertexCount = 3*2;
 
	if (mRenderOp.vertexData)
	{
		delete mRenderOp.vertexData;
		mRenderOp.vertexData = NULL;
	}
 
	if (!mRenderOp.vertexData)
		mRenderOp.vertexData = new Ogre::VertexData();
 
	mRenderOp.indexData = 0;
	mRenderOp.vertexData->vertexStart = 0;
	mRenderOp.vertexData->vertexCount = vertexCount;
	mRenderOp.operationType = Ogre::RenderOperation::OT_LINE_LIST; 
	mRenderOp.useIndexes = false; 
 
	Ogre::VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
	Ogre::VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;
	size_t offset = 0;
 
	// create/bind positions/tex.ccord. buffer
	if (!decl->findElementBySemantic(Ogre::VES_POSITION))
		decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton()
		.createVertexBuffer(offset, mRenderOp.vertexData->vertexCount,
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);

	bind->setBinding(0, vbuf);

	float *buf_i = static_cast<float*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	// x-axis
	*buf_i++ = 0;
	*buf_i++ = 0;
	*buf_i++ = 0;

	*buf_i++ = _length;
	*buf_i++ = 0;
	*buf_i++ = 0;

	// y-axis
	*buf_i++ = 0;
	*buf_i++ = 0;
	*buf_i++ = 0;

	*buf_i++ = 0;
	*buf_i++ = _length;
	*buf_i++ = 0;

	// z-axis
	*buf_i++ = 0;
	*buf_i++ = 0;
	*buf_i++ = 0;

	*buf_i++ = 0;
	*buf_i++ = 0;
	*buf_i++ = _length;

	vbuf->unlock();

	/// Colour buffer
	Ogre::RenderSystem* rs = Ogre::Root::getSingleton().getRenderSystem();
    Ogre::RGBA green;
    Ogre::RGBA blue;
	Ogre::RGBA red;
    // Use render system to convert colour value since colour packing varies
    rs->convertColourValue(Ogre::ColourValue::Green, &green);
	rs->convertColourValue(Ogre::ColourValue::Blue, &blue);
    rs->convertColourValue(Ogre::ColourValue::Red, &red);

    // 2nd buffer
    offset = 0;
	if (!decl->findElementBySemantic(Ogre::VES_DIFFUSE))
		decl->addElement(1, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_COLOUR);
    /// Allocate vertex buffer of the requested number of vertices (vertexCount) 
    /// and bytes per vertex (offset)
    vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
        offset, mRenderOp.vertexData->vertexCount, Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY);
	bind->setBinding(1, vbuf);
	
	Ogre::uint32 *cbuf_i = static_cast<Ogre::uint32 *>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	*cbuf_i++ = red;
	*cbuf_i++ = red;

	*cbuf_i++ = green;
	*cbuf_i++ = green;

	*cbuf_i++ = blue;
	*cbuf_i++ = blue;
	
	vbuf->unlock();

	mNeedUpdate = false;
}

void 
vl::ogre::Axes::_updateRenderQueue(Ogre::RenderQueue *queue)
{
	if (this->isVisible())
	{
		if (mNeedUpdate)
			this->_setupGeometry();
 
		queue->addRenderable(this, mRenderQueueID, OGRE_RENDERABLE_DEFAULT_PRIORITY);
	}
}
