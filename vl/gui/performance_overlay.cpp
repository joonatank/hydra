/**
 *	Copyright (c) 2012 - 2014 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-05
 *	@file gui/performance_overlay.cpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.5
 *
 *	Licensed under commercial license.
 *
 */

#include "performance_overlay.hpp"

#include "gui_defines.hpp"

#include "gui.hpp"

vl::gui::PerformanceOverlay::PerformanceOverlay(vl::gui::GUI *creator)
	: vl::gui::Window(creator)
	, _init_report(0)
	, _rendering_report(0)
	, _show_advanced(false)
	, _font(CONSOLE_FONT_INDEX)
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

	/// @todo need to move the basic overlay up and left
	if(_creator->getScale() == GS_HMD)
	{ _font = CONSOLE_FONT_INDEX_HMD; }

	// Create gorilla things here.
	_advanced_layer = mScreen->createLayer(10);

	size_t y = mScreen->getHeight()/2;
	size_t x = 10;
	// More advanced performance metrics
	_advanced_caption = _advanced_layer->createCaption(_font,  x,y, "Performance Overlay");

	x = 10;
	y = mScreen->getHeight()/2;
	// Total frame time
	y += 24;
	_frame_text = _advanced_layer->createMarkupText(_font,  x, y, "FRAME time");
	_frame_line = _advanced_layer->createLineList();
	_frame_line->begin(1.5, Gorilla::Colours::BlueViolet);
	_frame_line->position(x, y + 22);
	_frame_line->position(mScreen->getWidth() - 20, y + 22);
	_frame_line->end();
	// Kinematics
	y += 24;
	size_t line_x_end = 60;
	size_t line_x_start = x;
	_kinematics_text = _advanced_layer->createMarkupText(_font,  x, y, "KINEMATICS time");
	_kinematics_line = _advanced_layer->createLineList();
	_kinematics_line->begin(1.5, Gorilla::Colours::BlueViolet);
	_kinematics_line->position(line_x_start, y + 22);
	_kinematics_line->position(line_x_end, y + 22);
	_kinematics_line->end();
	// Physics
	y += 24;
	line_x_start = line_x_end;
	line_x_end += 40;
	_physics_text = _advanced_layer->createMarkupText(_font,  x, y, "PHYSICS time");
	_physics_line = _advanced_layer->createLineList();
	_physics_line->begin(1.5, Gorilla::Colours::BlueViolet);
	_physics_line->position(line_x_start, y + 22);
	_physics_line->position(line_x_end, y + 22);
	_physics_line->end();
	line_x_start = line_x_end;
	// Collisions
	y += 24;
	line_x_start = line_x_end;
	line_x_end += 60;
	_collisions_text = _advanced_layer->createMarkupText(_font,  x, y, "COLLISIONS time");
	_collisions_line = _advanced_layer->createLineList();
	_collisions_line->begin(1.5, Gorilla::Colours::BlueViolet);
	_collisions_line->position(line_x_start, y + 22);
	_collisions_line->position(line_x_end, y + 22);
	_collisions_line->end();
	// Rendering 
	y += 24;
	line_x_start = line_x_end;
	line_x_end = mScreen->getWidth() - 20 - line_x_start;
	_rendering_text = _advanced_layer->createMarkupText(_font,  x, y, "RENDERING time");
	_rendering_line = _advanced_layer->createLineList();
	_rendering_line->begin(1.5, Gorilla::Colours::Red);
	_rendering_line->position(line_x_start, y + 22);
	_rendering_line->position(line_x_end, y + 22);
	_rendering_line->end();

	// Extra text displyu for extra ordinary stuff
	y = mScreen->getHeight()/4*3;
	_advance_text = _advanced_layer->createMarkupText(_font,  x,y + 20, "");
	_advance_text->width(mScreen->getWidth() - x - 10);

	// @todo add maximum height
	// @todo add lines between the elements

	// Basic text for some basic performance metrics
	// @todo move the basic field to the bottom left corner (like SdkTrays was)
	// Remove SdkTrays and use the overlay ini config for basic
	_basic_layer = mScreen->createLayer(10);

	size_t basic_w = 200;
	size_t basic_h = 3*23;
	size_t basic_y = mScreen->getHeight() - (basic_h + 10);
	size_t basic_x = mScreen->getWidth() - (basic_w + 10);
	if(_creator->getScale() == GS_HMD)
	{
		basic_w = 330;
		basic_h = 3*47;
		basic_y = mScreen->getHeight() - (basic_h + 10);
		basic_x = mScreen->getWidth() - (basic_w + 10);
	}
	_basic_text = _basic_layer->createMarkupText(_font,  basic_x, basic_y, "");
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
	if(_rendering_report)
	{
		if(_rendering_report->isDirty())
		{ setDirty(DIRTY_RENDERING_REPORT); }

		_rendering_report->_clearDirty();
	}

	if(_init_report)
	{
		if(_init_report->isDirty())
		{ setDirty(DIRTY_INIT_REPORT); }

		_init_report->_clearDirty();
	}
}

// Serializing the report
void
vl::gui::PerformanceOverlay::doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const
{

	if(DIRTY_SHOW_ADVANCED & dirtyBits)
	{
		msg << _show_advanced;
	}

	if(DIRTY_RENDERING_REPORT & dirtyBits)
	{
		if(_rendering_report)
		{ msg << *_rendering_report; }
		else
		{ msg << size_t(0); }
	}

	if(DIRTY_INIT_REPORT & dirtyBits)
	{
		if(_init_report)
		{ msg << *_init_report; }
		else
		{ msg << size_t(0); }
	}
}

void
vl::gui::PerformanceOverlay::doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits)
{
	if(DIRTY_SHOW_ADVANCED & dirtyBits)
	{
		msg >> _show_advanced;
		_dirty = true;
	}

	if(DIRTY_RENDERING_REPORT & dirtyBits)
	{
		if(!_rendering_report)
		{ _rendering_report = new ProfilerReport; }
		msg >> *_rendering_report;
		if(_advance_text)
		{
			std::stringstream ss(std::stringstream::in | std::stringstream::out);
			ss << *_rendering_report;
			//_advance_text->text(ss.str());
			
			// Frame time
			ss.str("");
			vl::time frame_time = (*_rendering_report)[PT_FRAME].result();
			ss << "Frame time " << frame_time;
			_frame_text->text(ss.str());

			vl::scalar p_collisions = 0.0;
			vl::scalar p_physics = 0.0;
			vl::scalar p_kinematics = 0.0;
			// Kinematics
			// @todo we should hide Kinematics, Physics and Collisions if they have zero time
			vl::time kinematics_time = (*_rendering_report)[PT_KINEMATICS].result();
			if(kinematics_time != vl::time())
			{
				ss.str("");
				p_kinematics = kinematics_time/frame_time;
				ss << "Kinematics time " << kinematics_time << "    %3" << 100*p_kinematics << "%%" << "%R";
				_kinematics_text->text(ss.str());
			}
			else
			{ _kinematics_text->text(""); }
			// Physics
			vl::time physics_time = (*_rendering_report)[PT_PHYSICS].result();
			if(physics_time != vl::time())
			{
				ss.str("");
				p_physics = physics_time/frame_time;
				ss << "Physics time " << physics_time << "    %3" << 100*p_physics << "%%" << "%R";
				_physics_text->text(ss.str());
			}
			else
			{ _physics_text->text(""); }
			// Collisions
			vl::time collisions_time = (*_rendering_report)[PT_COLLISIONS].result();
			if(collisions_time != vl::time())
			{
				ss.str("");
				p_collisions = collisions_time/frame_time;
				ss << "Collision time " << collisions_time << "    %3" << 100*p_collisions << "%%" << "%R";
				_collisions_text->text(ss.str());
			}
			else
			{ _collisions_text->text(""); }
			// Rendering
			ss.str("");
			vl::time rend_time = (*_rendering_report)[PT_RENDERING].result();
			vl::scalar p_rendering = rend_time/frame_time;
			ss << "Rendering time \t" << rend_time << "    %3" << 100*p_rendering << "%%" << "%R";
			_rendering_text->text(ss.str());
			
			// Draw the lines
			// Kinematics
			size_t total_width = mScreen->getWidth() - 20;
			size_t start = 10;
			size_t stop = start + total_width*p_kinematics;
			size_t y = _kinematics_text->top() + 24;
			_kinematics_line->begin(1.5, Gorilla::Colours::Red);
			_kinematics_line->position(start, y);
			_kinematics_line->position(stop, y);
			_kinematics_line->end();
			// Physics
			start = stop;
			stop = start + total_width*p_physics;
			y = _physics_text->top() + 24;
			_physics_line->begin(1.5, Gorilla::Colours::Red);
			_physics_line->position(start, y);
			_physics_line->position(stop, y);
			_physics_line->end();
			// Collisions			
			start = stop;
			stop = start + total_width*p_collisions;
			y = _collisions_text->top() + 24;
			_collisions_line->begin(1.5, Gorilla::Colours::Red);
			_collisions_line->position(start, y);
			_collisions_line->position(stop, y);
			_collisions_line->end();
			// Rendering
			_rendering_line->begin(1.5, Gorilla::Colours::Red);
			start = stop;
			stop = start + total_width*p_rendering;
			y = _rendering_text->top() + 24;
			_rendering_line->position(start, y);
			_rendering_line->position(stop, y);
			_rendering_line->end();
			// @todo we need undefined line for the rest
		}
	}

	if(DIRTY_INIT_REPORT & dirtyBits)
	{
		if(!_init_report)
		{ _init_report = new Report<vl::time>; }
		msg >> *_init_report;
		// @todo add widget for this
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

template<>
vl::cluster::ByteStream &
vl::cluster::operator<<(vl::cluster::ByteStream &msg, vl::ProfilerReport const &report)
{
	msg << report.size();
	
	for(size_t i = 0; i < report.size(); ++i)
	{
		msg << report[i];
	}

	return msg;
}

template<>
vl::cluster::ByteStream &
vl::cluster::operator>>(vl::cluster::ByteStream &msg, vl::ProfilerReport &report)
{
	size_t size;
	msg >> size;
	
	for(size_t i = 0; i < size; ++i)
	{
		Number<vl::time> &num = report[i];
		msg >> num;
	}

	return msg;
}
