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
 */

#ifndef HYDRA_GUI_PERFORMANCE_OVERLAY_HPP
#define HYDRA_GUI_PERFORMANCE_OVERLAY_HPP

#include "gui_window.hpp"

// Base data stucture
#include "base/report.hpp"

#include "Math/types.hpp"

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
class PerformanceOverlay : public Window
{
public :
	PerformanceOverlay(vl::gui::GUI *creator);

	~PerformanceOverlay(void);

	void shutdown(void);

	void setReport(Report<vl::time> *report)
	{ _report = report; }

	Report<vl::time> *getReport(void)
	{ return _report; }

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
		DIRTY_REPORT = Window::DIRTY_CUSTOM << 0,
		DIRTY_SHOW_ADVANCED = Window::DIRTY_CUSTOM << 1,
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
	Report<vl::time> *_report;

	// From local renderer
	vl::scalar _fps;
	size_t _batches;
	size_t _tris;

	bool _show_advanced;

	/// Gorilla
	Gorilla::Layer *_advanced_layer;
	Gorilla::Caption*    _advanced_caption;
	Gorilla::MarkupText* _advance_text;

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

}

}	// namespace vl

#endif	// HYDRA_GUI_PERFORMANCE_OVERLAY_HPP
