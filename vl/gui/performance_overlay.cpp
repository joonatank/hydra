/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-05
 *	@file gui/performance_overlay.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 */

#include "performance_overlay.hpp"

#define CONSOLE_FONT_INDEX 14

vl::gui::PerformanceOverlay::PerformanceOverlay(vl::gui::GUI *creator)
	: vl::gui::Window(creator)
	, _report(0)
	, _show_advanced(false)
	, _fps(0)
	, _batches(0)
	, _tris(0)
	, _advanced_layer(0)
	, _advanced_caption(0)
	, _advance_text(0)
	, _basic_layer(0)
	, _basic_decoration(0)
	, _basic_text(0)
	, mIsInitialised(false)
	, _dirty(true)
{
}

vl::gui::PerformanceOverlay::~PerformanceOverlay(void)
{
	shutdown();
}

void
vl::gui::PerformanceOverlay::shutdown(void)
{
	if(!mIsInitialised)
		return;

	mIsInitialised = false;

	mScreen->destroy(_basic_layer);
	mScreen->destroy(_advanced_layer);
}

void
vl::gui::PerformanceOverlay::_window_resetted(void)
{
	if(mIsInitialised)
		shutdown();

	assert(mScreen);

	// Create gorilla things here.
	_advanced_layer = mScreen->createLayer(10);

	std::stringstream ss;
	if(_report)
	{ ss << *_report; }
	size_t y = mScreen->getHeight()/2;
	size_t x = 10;
	// More advanced performance metrics
	_advanced_caption = _advanced_layer->createCaption(CONSOLE_FONT_INDEX,  x,y, "Performance Overlay");
	_advance_text = _advanced_layer->createMarkupText(CONSOLE_FONT_INDEX,  x,y + 20, ss.str());
	_advance_text->width(mScreen->getWidth() - x - 10);
	
	// Basic text for some basic performance metrics
	// @todo move the basic field to the bottom left corner (like SdkTrays was)
	// Remove SdkTrays and use the overlay ini config for basic
	_basic_layer = mScreen->createLayer(10);

	size_t basic_w = 200;
	size_t basic_h = 3*23;
	size_t basic_y = mScreen->getHeight() - (basic_h + 10);
	size_t basic_x = mScreen->getWidth() - (basic_w + 10);
	_basic_text = _basic_layer->createMarkupText(CONSOLE_FONT_INDEX,  basic_x, basic_y, "");
	_basic_text->width(basic_w);
	_basic_text->height(basic_h);
	// @todo this would look nicer if it had round edged (not easily achieved)
	_basic_decoration = _basic_layer->createRectangle(basic_x-2, basic_y-2, basic_w - 16, basic_h+2);
	_basic_decoration->background_gradient(Gorilla::Gradient_NorthSouth, Gorilla::rgb(0,128,0,32), Gorilla::rgb(64,64,64,32));
	_basic_decoration->border(2, Gorilla::rgb(256,0,0,64));

	mIsInitialised = true;
	_dirty = true;
}

void
vl::gui::PerformanceOverlay::_update(void)
{
	if(!mIsInitialised)
	{ return; }

	if(_basic_layer->isVisible() != _visible)
	{ _basic_layer->setVisible(_visible); }

	bool advanced_visible = _visible && _show_advanced;
	if(advanced_visible != _advanced_layer->isVisible())
	{ _advanced_layer->setVisible(advanced_visible); }

	if(_visible)
	{
		if(_basic_text)
		{
			// @todo this is stupid way to do the formating
			// we should just do six text boxes one for each lable
			// and one for each value. Easier to align and change
			// also looks much cleaner.
			std::stringstream ss;
			ss.unsetf(std::ios::floatfield);
			ss.setf(std::ios::fixed);
			ss.precision(1);
			
			ss << "%3" << "FPS : ";
			ss.width(18);
			ss << _fps << "%R" << "\n"
				<< "Batches : ";
			ss.width(11);
			ss << _batches << "\n"
				<< "Triangles : ";
			ss.width(8);
			ss << _tris;
			_basic_text->text(ss.str());
		}

		if(_dirty)
		{ _dirty_update(); }
	}
}

void
vl::gui::PerformanceOverlay::_dirty_update(void)
{
	// We should switch between showing advanced and basic
	_dirty = false;
}

void
vl::gui::PerformanceOverlay::recaluclateDirties(void)
{
	if(_report)
	{
		if(_report->isDirty())
		{ setDirty(DIRTY_REPORT); }

		_report->_clearDirty();
	}
}

// Serializing the report
void
vl::gui::PerformanceOverlay::doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{
	// @todo Report is a pointer to persistant objects so it needs either be
	// distributed by itself or we need to add dirty method to it also
	// so we now when to mark it dirty here.
	if(DIRTY_REPORT & dirtyBits)
	{
		if(_report)
		{ msg << *_report; }
		else
		{ msg << size_t(0); }
	}

	if(DIRTY_SHOW_ADVANCED & dirtyBits)
	{
		msg << _show_advanced;
	}
}

void
vl::gui::PerformanceOverlay::doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	if(DIRTY_REPORT & dirtyBits)
	{
		if(!_report)
		{ _report = new Report<vl::time>; }
		msg >> *_report;
		std::stringstream ss;
		if(_advance_text)
		{
			ss << *_report;
			_advance_text->text(ss.str());
		}
	}

	if(DIRTY_SHOW_ADVANCED & dirtyBits)
	{
		msg >> _show_advanced;
		_dirty = true;
	}
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::Report<vl::time> const &report)
{
	msg << report.size();
	std::map< std::string, Number<vl::time> >::const_iterator iter;
	for(iter = report.begin(); iter != report.end(); ++iter)
	{
		msg << iter->first << iter->second;
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::Report<vl::time> &report)
{
	size_t size;
	msg >> size;
	
	for(size_t i = 0; i < size; ++i)
	{
		std::string key;
		msg >> key;
		Number<vl::time> &num = report[key];
		msg >> num;
	}

	return msg;
}


template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::Number<vl::time> const &num)
{
	msg << num.result().sec << num.result().usec;

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::Number<vl::time> &num)
{
	vl::time res;
	msg >> res.sec >> res.usec;
	num.set_result(res);

	return msg;
}
