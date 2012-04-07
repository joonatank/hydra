/**
 *	Copyright (c) 2011 Tampere University of Technology
 *	Copyright (c) 2011/10 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file: movable_text.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.3
 *
 *	Licensed under the MIT Open Source License, 
 *	for details please see LICENSE file or the website
 *	http://www.opensource.org/licenses/mit-license.php
 *
 */

/**	@description: This create create a billboarding object that display a text.
 *
 *	Copied from Ogre Wiki, original created by
 *	2003 by cTh see gavocanov@rambler.ru
 *	2006 by barraq see nospam@barraquand.com
 */
 
#include <OGRE/Ogre.h>
#include <OGRE/OgreFontManager.h>
#include "movable_text.hpp"
 
#include "base/exceptions.hpp"

#define POS_TEX_BINDING    0
#define COLOUR_BINDING     1
 
vl::ogre::MovableText::MovableText(std::string const &name, 
	std::string const &caption, std::string const &fontName, 
	Ogre::Real charHeight, const Ogre::ColourValue &color)
	: MovableObject(name)
	, mpFont(NULL)
	, mCaption(caption)
	, mFontName(fontName)
	, mCharHeight(charHeight)
	, mColor(color)
	, mSpaceWidth(0)
	, mUpdateColors(true)
	, mOnTop(false)
	, mVerticalAlignment(V_CENTER)
	, mPosition(Ogre::Vector3::ZERO)
{
    if (name == "")
    {
		std::string str("Trying to create MovableText without name");
		BOOST_THROW_EXCEPTION(vl::empty_param() << vl::desc(str));
	}
    if (caption == "")
    {
		std::string str("Trying to create MovableText without caption");
		BOOST_THROW_EXCEPTION(vl::empty_param() << vl::desc(str));
	}
 
    mRenderOp.vertexData = NULL;
    this->setFontName(mFontName);
    this->_setupGeometry();
}
 
vl::ogre::MovableText::~MovableText()
{
	if(mRenderOp.vertexData)
	{ delete mRenderOp.vertexData; }
	// May cause crashing... check this and comment if it does
	if(!mpMaterial.isNull())
	{ Ogre::MaterialManager::getSingletonPtr()->remove(mpMaterial->getName()); }
}
 
void
vl::ogre::MovableText::setFontName(std::string const &fontName)
{
    if((Ogre::MaterialManager::getSingletonPtr()->resourceExists(mName + "Material"))) 
    { 
        Ogre::MaterialManager::getSingleton().remove(mName + "Material"); 
    }
 
    if (mFontName != fontName || mpMaterial.isNull() || !mpFont)
    {
        mFontName = fontName;
        mpFont = (Ogre::Font *)Ogre::FontManager::getSingleton().getByName(mFontName).getPointer();
        if (!mpFont)
		{
 			std::string str("Could not find font");
			BOOST_THROW_EXCEPTION(vl::item_not_found() << vl::desc(str) << vl::file_name(fontName));
		}
        mpFont->load();
        if (!mpMaterial.isNull())
        {
            Ogre::MaterialManager::getSingletonPtr()->remove(mpMaterial->getName());
            mpMaterial.setNull();
        }
 
        mpMaterial = mpFont->getMaterial()->clone(mName + "Material");
        if (!mpMaterial->isLoaded())
            mpMaterial->load();
 
        mpMaterial->setDepthCheckEnabled(!mOnTop);
        mpMaterial->setDepthBias(1.0,1.0);
        mpMaterial->setDepthWriteEnabled(mOnTop);
        mpMaterial->setLightingEnabled(false);
        mNeedUpdate = true;
    }
}
 
void
vl::ogre::MovableText::setCaption(std::string const &caption)
{
    if (caption != mCaption)
    {
        mCaption = caption;
        mNeedUpdate = true;
    }
}
 
void
vl::ogre::MovableText::setColor(Ogre::ColourValue const &color)
{
    if (color != mColor)
    {
        mColor = color;
        mUpdateColors = true;
    }
}
 
void
vl::ogre::MovableText::setCharacterHeight(Ogre::Real height)
{
    if (height != mCharHeight)
    {
        mCharHeight = height;
        mNeedUpdate = true;
    }
}
 
void
vl::ogre::MovableText::setSpaceWidth(Ogre::Real width)
{
    if (width != mSpaceWidth)
    {
        mSpaceWidth = width;
        mNeedUpdate = true;
    }
}
 
void
vl::ogre::MovableText::setTextAlignment(VerticalAlignment const &verticalAlignment)
{
    if(mVerticalAlignment != verticalAlignment)
    {
        mVerticalAlignment = verticalAlignment;
        mNeedUpdate = true;
    }
}
 
void
vl::ogre::MovableText::setPosition(Ogre::Vector3 const &pos)
{
    mPosition = pos;
}

void
vl::ogre::MovableText::showOnTop(bool show)
{
    if( mOnTop != show && !mpMaterial.isNull() )
    {
        mOnTop = show;
        mpMaterial->setDepthBias(1.0,1.0);
        mpMaterial->setDepthCheckEnabled(!mOnTop);
        mpMaterial->setDepthWriteEnabled(mOnTop);
    }
}

void 
vl::ogre::MovableText::visitRenderables(Ogre::Renderable::Visitor* visitor, 
        bool debugRenderables)
{}

void 
vl::ogre::MovableText::getWorldTransforms(Ogre::Matrix4 *xform) const 
{
	if(this->isVisible())
	{
		if(mParentNode)
		{ *xform = mParentNode->_getFullTransform(); }
	}
}

void
vl::ogre::MovableText::getRenderOperation(Ogre::RenderOperation &op)
{
	if (this->isVisible())
	{
		if (mNeedUpdate)
			this->_setupGeometry();
		if (mUpdateColors)
			this->_updateColors();
		op = mRenderOp;
	}
}

void
vl::ogre::MovableText::_setupGeometry()
{
    assert(mpFont);
    assert(!mpMaterial.isNull());
 
    unsigned int vertexCount = static_cast<unsigned int>(mCaption.size() * 6);
 
    if (mRenderOp.vertexData)
    {
        // Removed this test as it causes problems when replacing a caption
        // of the same size: replacing "Hello" with "hello"
        // as well as when changing the text alignment
        //if (mRenderOp.vertexData->vertexCount != vertexCount)
        {
            delete mRenderOp.vertexData;
            mRenderOp.vertexData = NULL;
            mUpdateColors = true;
        }
    }
 
    if (!mRenderOp.vertexData)
        mRenderOp.vertexData = new Ogre::VertexData();
 
    mRenderOp.indexData = 0;
    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.vertexData->vertexCount = vertexCount;
    mRenderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST; 
    mRenderOp.useIndexes = false; 
 
    Ogre::VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
    Ogre::VertexBufferBinding *bind = mRenderOp.vertexData->vertexBufferBinding;
    size_t offset = 0;
 
    // create/bind positions/tex.ccord. buffer
    if (!decl->findElementBySemantic(Ogre::VES_POSITION))
        decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
 
    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
 
    if (!decl->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES))
        decl->addElement(POS_TEX_BINDING, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
 
    Ogre::HardwareVertexBufferSharedPtr ptbuf = Ogre::HardwareBufferManager::getSingleton()
		.createVertexBuffer( decl->getVertexSize(POS_TEX_BINDING),
			mRenderOp.vertexData->vertexCount,
			Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY );
    bind->setBinding(POS_TEX_BINDING, ptbuf);
 
    // Colours - store these in a separate buffer because they change less often
    if (!decl->findElementBySemantic(Ogre::VES_DIFFUSE))
        decl->addElement(COLOUR_BINDING, 0, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
 
    Ogre::HardwareVertexBufferSharedPtr cbuf = Ogre::HardwareBufferManager::getSingleton()
		.createVertexBuffer( decl->getVertexSize(COLOUR_BINDING),
			mRenderOp.vertexData->vertexCount,
			Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY );
    bind->setBinding(COLOUR_BINDING, cbuf);
 
    size_t charlen = mCaption.size();
    float *pPCBuff = static_cast<float*>(ptbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
 
    float left = 0;
    float top = 0;
 
    Ogre::Real spaceWidth = mSpaceWidth;
    // Derive space width from a capital A
    if(spaceWidth == 0)
	{ spaceWidth = mpFont->getGlyphAspectRatio('A') * mCharHeight * 2.0; }
 
    // for calculation of AABB
    Ogre::Vector3 min, max, currPos;
    Ogre::Real maxSquaredRadius;
    bool first = true;
 
    // Use iterator
    std::string::iterator i, iend;
    iend = mCaption.end();
    bool newLine = true;
    Ogre::Real len = 0.0f;
 
    Ogre::Real verticalOffset = 0;
    switch (mVerticalAlignment)
    {
    case MovableText::V_ABOVE:
        verticalOffset = mCharHeight;
        break;
    case MovableText::V_CENTER:
        verticalOffset = 0.5*mCharHeight;
        break;
    case MovableText::V_BELOW:
        verticalOffset = 0;
        break;
    }
    // Raise the first line of the caption
    top += verticalOffset;
    for (i = mCaption.begin(); i != iend; ++i)
    {
        if (*i == '\n')
            top += verticalOffset * 2.0;
    }
 
    for (i = mCaption.begin(); i != iend; ++i)
    {
		Ogre::Real const z_coord = 0;
        if (newLine)
        {
            len = 0.0f;
            for (std::string::iterator j = i; j != iend && *j != '\n'; j++)
            {
                if (*j == ' ')
                    len += spaceWidth;
                else 
                    len += mpFont->getGlyphAspectRatio((unsigned char)*j) * mCharHeight * 2.0;
            }
            newLine = false;
        }
 
        if (*i == '\n')
        {
            left = 0;
            top -= mCharHeight * 2.0;
            newLine = true;
            continue;
        }
 
        if (*i == ' ')
        {
            // Just leave a gap, no tris
            left += spaceWidth;
            // Also reduce tri count
            mRenderOp.vertexData->vertexCount -= 6;
            continue;
        }
 
		Ogre::Real horiz_height = mpFont->getGlyphAspectRatio((unsigned char)*i);
		Ogre::Real u1, u2, v1, v2; 
		Ogre::Font::UVRect utmp;
		utmp = mpFont->getGlyphTexCoords((unsigned char)*i);
		u1 = utmp.left;
		u2 = utmp.right;
		v1 = utmp.top;
		v2 = utmp.bottom;
 
		// each vert is (x, y, z, u, v)
		//-------------------------------------------------------------------------------------
		// First tri
		//
		// Upper left
		*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = z_coord;
		*pPCBuff++ = u1;
		*pPCBuff++ = v1;
 
		// Deal with bounds
		currPos = Ogre::Vector3(left - (len / 2), top, z_coord);
		if (first)
		{
			min = max = currPos;
			maxSquaredRadius = currPos.squaredLength();
			first = false;
		}
		else
		{
			min.makeFloor(currPos);
			max.makeCeil(currPos);
			maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
		}
 
		top -= mCharHeight * 2.0;
 
		// Bottom left
		*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = z_coord;
		*pPCBuff++ = u1;
		*pPCBuff++ = v2;
 
		// Deal with bounds
		currPos = Ogre::Vector3(left - (len / 2), top, z_coord);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
 
		top += mCharHeight * 2.0;
		left += horiz_height * mCharHeight * 2.0;
 
		// Top right
		// We could create the currPos here and use it instead of separate left, top, z_coord
		*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = z_coord;
		*pPCBuff++ = u2;
		*pPCBuff++ = v1;
		//-------------------------------------------------------------------------------------
 
		// Deal with bounds
		currPos = Ogre::Vector3(left - (len / 2), top, z_coord);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
 
		//-------------------------------------------------------------------------------------
		// Second tri
		//
		// Top right (again)
		*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = z_coord;
		*pPCBuff++ = u2;
		*pPCBuff++ = v1;
 
		// Deal with bounds
		currPos = Ogre::Vector3(left - (len / 2), top, z_coord);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
 
		top -= mCharHeight * 2.0;
		left -= horiz_height  * mCharHeight * 2.0;
 
		// Bottom left (again)
		*pPCBuff++ = left - (len / 2);
		*pPCBuff++ = top;
		*pPCBuff++ = z_coord;
		*pPCBuff++ = u1;
		*pPCBuff++ = v2;
 
		// Deal with bounds
		currPos = Ogre::Vector3(left - (len / 2), top, z_coord);
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());
 
		left += horiz_height * mCharHeight * 2.0;
 
		// Bottom right
		currPos = Ogre::Vector3(left - (len / 2), top, z_coord);
		*pPCBuff++ = currPos.x;
		*pPCBuff++ = currPos.y;
		*pPCBuff++ = currPos.z;
		*pPCBuff++ = u2;
		*pPCBuff++ = v2;

		// Deal with bounds
		
		min.makeFloor(currPos);
		max.makeCeil(currPos);
		maxSquaredRadius = std::max(maxSquaredRadius, currPos.squaredLength());

		//-------------------------------------------------------------------------------------
 
		// Go back up with top
		top += mCharHeight * 2.0;
	}
 
	// Unlock vertex buffer
	ptbuf->unlock();
 
	// update AABB/Sphere radius
	mAABB = Ogre::AxisAlignedBox(min, max);
	mBoundingRadius = Ogre::Math::Sqrt(maxSquaredRadius);
 
	if (mUpdateColors)
		this->_updateColors();
 
	mNeedUpdate = false;
}
 
void
vl::ogre::MovableText::_updateColors(void)
{
	assert(mpFont);
	assert(!mpMaterial.isNull());
 
	// Convert to system-specific
	Ogre::RGBA color;
	Ogre::Root::getSingleton().convertColourValue(mColor, &color);
	Ogre::HardwareVertexBufferSharedPtr vbuf = mRenderOp.vertexData->vertexBufferBinding->getBuffer(COLOUR_BINDING);
	Ogre::RGBA *pDest = static_cast<Ogre::RGBA*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
	for (int i = 0; i < (int)mRenderOp.vertexData->vertexCount; ++i)
		*pDest++ = color;
	vbuf->unlock();
	mUpdateColors = false;
}

void 
vl::ogre::MovableText::_updateRenderQueue(Ogre::RenderQueue *queue)
{
	if (this->isVisible())
	{
		if (mNeedUpdate)
			this->_setupGeometry();
		if (mUpdateColors)
			this->_updateColors();
 
		queue->addRenderable(this, mRenderQueueID, OGRE_RENDERABLE_DEFAULT_PRIORITY);
	}
}


/// ---------------------------- vl::MovableText -----------------------------
/// ---------------------------- public -----------------------------------
vl::MovableText::MovableText(std::string const &name, vl::SceneManagerPtr creator)
	: vl::MovableObject(name, creator)
{
	_clear();
}

vl::MovableText::MovableText(vl::SceneManagerPtr creator)
	: vl::MovableObject(creator)
{
	_clear();
}

void
vl::MovableText::setCaption(const std::string &caption)
{
	if(_caption != caption)
	{
		setDirty(DIRTY_TEXT);
		_caption = caption;
	}
}

void
vl::MovableText::setFontName(const std::string &fontName)
{
	if(_font_name != fontName)
	{
		setDirty(DIRTY_TEXT_PARAMS);
		_font_name = fontName;
	}
}

void
vl::MovableText::setColour(Ogre::ColourValue const &colour)
{
	if(_colour != colour)
	{
		setDirty(DIRTY_TEXT_PARAMS);
		_colour = colour;
	}
}

void
vl::MovableText::setCharacterHeight(Ogre::Real height)
{
	if(_char_height != height)
	{
		setDirty(DIRTY_TEXT_PARAMS);
		_char_height = height;
	}
}

void
vl::MovableText::setSpaceWidth(Ogre::Real width)
{
	if(_space_width != width)
	{
		setDirty(DIRTY_TEXT_PARAMS);
		_space_width = width;
	}
}

void
vl::MovableText::setTextAlignment(std::string const &vertical)
{
	// @todo implement
}

void
vl::MovableText::showOnTop(bool show)
{
	if(_on_top != show)
	{
		setDirty(DIRTY_TEXT_PARAMS);
		_on_top = show;
	}
}

vl::MovableObjectPtr
vl::MovableText::clone(std::string const &append_to_name) const
{
	/// @todo implement
	return 0;
}

/// ---------------------------- protected -----------------------------------
bool
vl::MovableText::_doCreateNative(void)
{
	if(!_ogre_text)
	{
		_ogre_text = new vl::ogre::MovableText(getName(), _caption, _font_name, _char_height, _colour);
		_ogre_text->setTextAlignment(_vertical_alignment);
		_ogre_text->showOnTop(_on_top);
	}

	return true;
}

void
vl::MovableText::doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
	if(DIRTY_TEXT & dirtyBits)
	{
		msg << _caption;
	}

	if(DIRTY_TEXT_PARAMS & dirtyBits)
	{
		msg << _font_name << _colour << _char_height << _space_width << _on_top;
	}
}

void
vl::MovableText::doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	if(DIRTY_TEXT & dirtyBits)
	{
		msg >> _caption;
		if(_ogre_text)
		{ _ogre_text->setCaption(_caption); }
	}

	if(DIRTY_TEXT_PARAMS & dirtyBits)
	{
		msg >> _font_name >> _colour >> _char_height >> _space_width >> _on_top;
		if(_ogre_text)
		{
			_ogre_text->setFontName(_font_name);
			_ogre_text->setColor(_colour);
			_ogre_text->setCharacterHeight(_char_height);
			_ogre_text->setSpaceWidth(_space_width);
			_ogre_text->showOnTop(_on_top);
		}
	}
}

void
vl::MovableText::_clear(void)
{
	_caption = "";
	_font_name = "BlueHighway-12";
	_colour =  Ogre::ColourValue::White;
	_char_height =  1.0;
	_space_width = 0;
	_on_top = false;
	_vertical_alignment = vl::ogre::MovableText::V_CENTER;

	_ogre_text = 0;
}
