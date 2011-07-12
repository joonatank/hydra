/**	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2011-06
 *	@file: movable_text.hpp
 *
 *	This file is part of Hydra VR game engine.
 *
 *	description: This create a billboarding object that display a text.
 *
 *	Copied from Ogre Wiki, original created by
 *	2003 by cTh see gavocanov@rambler.ru
 *	2006 by barraq see nospam@barraquand.com
 */
 
#ifndef HYDRA_MOVABLE_TEXT_HPP
#define HYDRA_MOVABLE_TEXT_HPP

#include <OGRE/OgreMovableObject.h>

/// Necessary for the distributed 3d text class
#include "movable_object.hpp"

namespace vl {

namespace ogre {
 
/// @class MovableText
/// Core renderable object, not distributed
/// Extends Ogre renderables
class MovableText : public Ogre::MovableObject, public Ogre::Renderable
{
public:
	enum HorizontalAlignment    {H_LEFT, H_CENTER};
	enum VerticalAlignment      {V_BELOW, V_ABOVE, V_CENTER};
 
	/******************************** public methods ******************************/
public:
	MovableText( std::string const &name, std::string const &caption, 
		std::string const &fontName = "BlueHighway-12", Ogre::Real charHeight = 1.0, 
		Ogre::ColourValue const &color = Ogre::ColourValue::White );
	virtual ~MovableText();
 
	// Add to build on Shoggoth:
	virtual void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);
 
	// Set settings
	void    setFontName(const std::string &fontName);
	void    setCaption(const std::string &caption);
	void    setColor(const Ogre::ColourValue &color);
	void    setCharacterHeight(Ogre::Real height);
	void    setSpaceWidth(Ogre::Real width);
	void    setTextAlignment(const HorizontalAlignment& horizontalAlignment, const VerticalAlignment& verticalAlignment);
	void    setPosition(Ogre::Vector3 const &trans);
	void    showOnTop(bool show=true);
	void	setTrackCamera(bool track);
 
	// Get settings
	std::string const &getFontName() const {return mFontName;}
	std::string const &getCaption() const {return mCaption;}
	Ogre::ColourValue const &getColor() const {return mColor;}
 
	Ogre::Real getCharacterHeight() const {return mCharHeight;}
	Ogre::Real getSpaceWidth() const {return mSpaceWidth;}
	Ogre::Vector3 const &getPosition() const {return mPosition;}
	bool getShowOnTop() const {return mOnTop;}
	bool getTrackCamera(void) const { return mTrackCamera; }

	// from MovableObject
	void getWorldTransforms(Ogre::Matrix4 *xform) const;
	Ogre::Real getBoundingRadius(void) const {return mRadius;};
	Ogre::Real getSquaredViewDepth(const Ogre::Camera *cam) const {return 0;};
	Ogre::Quaternion const &getWorldOrientation(void) const;
	Ogre::Vector3 const &getWorldPosition(void) const;
	/// @todo there's a problem with the bounding box size, it does not correctly
	/// reflect the size and position of the font
	Ogre::AxisAlignedBox const &getBoundingBox(void) const {return mAABB;}

	std::string const &getMovableType(void) const 
	{
		static Ogre::String movType = "MovableText";
		return movType;
	}

	// from renderable
	void getRenderOperation(Ogre::RenderOperation &op);
	Ogre::MaterialPtr const &getMaterial(void) const 
	{ return mpMaterial; }

	Ogre::LightList const &getLights(void) const 
	{ return mLList; }

	/******************************** protected methods and overload **************/
protected:
 
	// from MovableText, create the object
	void _setupGeometry();
	void _updateColors();

	/// from MovableObject, automatic methods called from redender system

	/// change the camera used for orientating the text
	void    _notifyCurrentCamera(Ogre::Camera *cam);
	void    _updateRenderQueue(Ogre::RenderQueue* queue);
 

	/************************** protected data *******************************/
protected:
	std::string mFontName;
	std::string mType;
	std::string mCaption;
	HorizontalAlignment mHorizontalAlignment;
	VerticalAlignment mVerticalAlignment;
 
	Ogre::ColourValue mColor;
	Ogre::RenderOperation mRenderOp;
	Ogre::AxisAlignedBox mAABB;
	Ogre::LightList mLList;
 
	Ogre::Real mCharHeight;
	Ogre::Real mSpaceWidth;
 
	bool mNeedUpdate;
	bool mUpdateColors;
	
	bool mTrackCamera;
	bool mOnTop;
 
	Ogre::Real mTimeUntilNextToggle;
	Ogre::Real mRadius;
 
	Ogre::Vector3 mPosition;
 
	Ogre::Camera *mpCam;
	Ogre::RenderWindow *mpWin;
	Ogre::Font *mpFont;
	Ogre::MaterialPtr mpMaterial;
	Ogre::MaterialPtr mpBackgroundMaterial;

};	// end class MovableText
 
}	//end namespace ogre

/// @class MovableText
/// @brief Distributed 3d text class, used for interfacing the Rendering system
class MovableText : public vl::MovableObject
{
public :
	MovableText(std::string const &name, vl::SceneManagerPtr creator);

	/// Slave constructor
	MovableText(vl::SceneManagerPtr creator);

	virtual ~MovableText(void) {}

	// Set settings
	void    setFontName(const std::string &fontName);
	void    setCaption(const std::string &caption);
	void    setColour(const Ogre::ColourValue &colour);
	void    setCharacterHeight(Ogre::Real height);
	void    setSpaceWidth(Ogre::Real width);
	/// set the text aligment
	/// @param horizontal valid values H_LEFT, H_CENTER
	/// @param vertical valid values V_BELOW, V_ABOVE, V_CENTER
	void    setTextAlignment(std::string const &horizontal, std::string const &vertical);
	void    setPosition(Ogre::Vector3 const &trans);
	void    showOnTop(bool show);
	void    setTrackCamera(bool track);
	
	// Get settings
	std::string const &getFontName() const { return _font_name; }
	std::string const &getCaption() const { return _caption; }
	Ogre::ColourValue const &getColour() const { return _colour; }
	/// @todo add get aligment
	Ogre::Real getCharacterHeight() const { return _char_height; }
	Ogre::Real getSpaceWidth() const { return _space_width; }
	Ogre::Vector3 const &getPosition() const { return _position; }
	bool getShowOnTop() const { return _on_top; }
	bool getTrackCamera() const { return _track_camera; }

	virtual Ogre::MovableObject *getNative(void) const
	{ return _ogre_text; }

	virtual std::string getTypeName(void) const
	{ return "MovableText"; }

	virtual MovableObjectPtr clone(std::string const &append_to_name) const;

	enum DirtyBits
	{
		DIRTY_TEXT = vl::MovableObject::DIRTY_CUSTOM << 0,
		DIRTY_TEXT_PARAMS = vl::MovableObject::DIRTY_CUSTOM << 1,
		DIRTY_CUSTOM = vl::MovableObject::DIRTY_CUSTOM << 2,
	};

private :
	/// Virtual private methods

	virtual bool _doCreateNative(void);
	virtual void doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;
	virtual void doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	void _clear(void);

	/************************** private data *******************************/
private :

	std::string _caption;
	std::string _font_name;
	Ogre::ColourValue _colour;
	Ogre::Real _char_height;
	Ogre::Real _space_width;
	Ogre::Vector3 _position;
	bool _on_top;
	bool _track_camera;
	
	vl::ogre::MovableText::HorizontalAlignment _horizontal_alignment;
	vl::ogre::MovableText::VerticalAlignment _vertical_alignment;

	vl::ogre::MovableText *_ogre_text;

};	// end class MovableText

}	// end namespace vl
 
#endif	// HYDRA_MOVABLE_TEXT_HPP
