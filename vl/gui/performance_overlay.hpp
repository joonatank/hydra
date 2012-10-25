/**
 *	Copyright (c) 2012 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2012-05
 *	@file gui/performance_overlay.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_GUI_PERFORMANCE_OVERLAY_HPP
#define HYDRA_GUI_PERFORMANCE_OVERLAY_HPP

#include "gui_window.hpp"

// Base data stucture
#include "base/report.hpp"

#include "math/types.hpp"

#include "profiler_report.hpp"

namespace vl
{

namespace gui
{

/**	@class PerformanceOverlay
 *	@brief Graphical overlay for displaying the comprehensive performance statistics
 *	Distributed so the statistics are gathered by the master and distributed 
 *	to slaves for displaying.
 *	@todo change from general purpose Report to something more specialised for profiling
 *	special case ProfilerReport would be useful.
 *	We can still keep Report distribution for displaying init report for example.
 */
class HYDRA_API PerformanceOverlay : public Window
{
public :
	PerformanceOverlay(vl::gui::GUI *creator);

	~PerformanceOverlay(void);

	void shutdown(void);

	void setInitReport(Report<vl::time> *report)
	{
		setDirty(DIRTY_INIT_REPORT);
		_init_report = report;
	}

	Report<vl::time> *getInitReport(void)
	{ return _init_report; }

	void setRenderingReport(ProfilerReport *report)
	{
		setDirty(DIRTY_RENDERING_REPORT);
		_rendering_report = report;
	}

	ProfilerReport *getRenderingReport(void)
	{ return _rendering_report; }

	void setShowAdvanced(bool show)
	{
		if(_show_advanced != show)
		{
			setDirty(DIRTY_SHOW_ADVANCED);
			_show_advanced = show;
		}
	}

	bool isShowAdvanced(void) const
	{ return _show_advanced; }

	enum DirtyBits
	{
		DIRTY_SHOW_ADVANCED = Window::DIRTY_CUSTOM << 0,
		DIRTY_RENDERING_REPORT = Window::DIRTY_CUSTOM << 1,
		DIRTY_INIT_REPORT = Window::DIRTY_CUSTOM << 2,
	};

	// Renderer specific stats (updated by the Renderer)
	// This is mostly useless because we can easily calculate this on server side
	// though might be nice to see if they differ
	void setFrameFrame(vl::scalar const fps)
	{ _fps = fps; }

	void setLastBatchCount(size_t batches)
	{ _batches = batches; }

	void setLastTriangleCount(size_t tris)
	{ _tris = tris; }

// Private virtual overrides
private :
	virtual void _window_resetted(void);

	virtual void _update(void);

	virtual void recaluclateDirties(void);

	// Serializing the report
	virtual void doSerialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits) const;

	virtual void doDeserialize(vl::cluster::ByteStream &msg, const uint64_t dirtyBits);

	// Dirty update
	void _dirty_update(void);

private :
	// From master
	Report<vl::time> *_init_report;
	ProfilerReport *_rendering_report;

	// From local renderer
	vl::scalar _fps;
	size_t _batches;
	size_t _tris;

	bool _show_advanced;

	/// Gorilla

	/// Advanced layer
	Gorilla::Layer *_advanced_layer;
	Gorilla::Caption *_advanced_caption;
	Gorilla::MarkupText *_advance_text;
	// Total frame time
	Gorilla::LineList *_frame_line;
	Gorilla::MarkupText *_frame_text;
	// Kinematics
	Gorilla::LineList *_kinematics_line;
	Gorilla::MarkupText *_kinematics_text;
	// Physics
	Gorilla::LineList *_physics_line;
	Gorilla::MarkupText *_physics_text;
	// Collisions
	Gorilla::LineList *_collisions_line;
	Gorilla::MarkupText *_collisions_text;
	// Rendering
	Gorilla::LineList *_rendering_line;
	Gorilla::MarkupText *_rendering_text;

	// CPU usage
	Gorilla::MarkupText *_cpu_text;
	// GPU usage
	Gorilla::MarkupText *_gpu_text;

	/// Basic layer
	Gorilla::Layer *_basic_layer;
	Gorilla::Rectangle*  _basic_decoration;
	Gorilla::MarkupText* _basic_text;

	bool mIsInitialised;

	bool _dirty;

};	// class PerformanceOverlay

}	// namespace gui

namespace cluster
{

template<>
ByteStream &operator<<(ByteStream &msg, vl::Report<vl::time> const &report);

template<>
ByteStream &operator>>(ByteStream &msg, vl::Report<vl::time> &report);

template<>
ByteStream &operator<<(ByteStream &msg, vl::Number<vl::time> const &num);

template<>
ByteStream &operator>>(ByteStream &msg, vl::Number<vl::time> &num);

template<>
ByteStream &operator<<(ByteStream &msg, vl::ProfilerReport const &report);

template<>
ByteStream &operator>>(ByteStream &msg, vl::ProfilerReport &report);


}

}	// namespace vl

#endif	// HYDRA_GUI_PERFORMANCE_OVERLAY_HPP
