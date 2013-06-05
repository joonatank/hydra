/**
 *	Copyright (c) 2013 Savant Simulators
 *
 *	@author Joonatan Kuosa <joonatan.kuosa@savantsimulators.com>
 *	@date 2013-05
 *	@file input/pcan.hpp
 *
 *	This file is part of Hydra VR game engine.
 *	Version 0.4
 *
 *	Licensed under commercial license.
 *
 */

#ifndef HYDRA_PCAN_HPP
#define HYDRA_PCAN_HPP

// Base class
#include "input.hpp"

#include "typedefs.hpp"

#include <stdint.h>

#include <boost/signal.hpp>

/// Using the c interface for PCAN
/// @todo this is problematic we really don't want to include Windows.h
/// in a header file but we must for the PCANBasic types.
#include <Windows.h>
#include <PCANBasic.h>

namespace vl
{

struct CANMsg
{
	CANMsg(uint32_t id_, int8_t const data_[], uint8_t len_)
		: id(id_), length(len_), data(8, 0)
	{
		::memcpy(&data[0], data_, 8);
	}

	CANMsg(uint32_t id_, uint8_t const data_[], uint8_t len_)
		: id(id_), length(len_), data(8, 0)
	{
		::memcpy(&data[0], data_, 8);
	}

	CANMsg()
		: id(0), length(0), data(8, 0)
	{
		::memset(&data[0], 0, 8);
	}

	CANMsg(CANMsg const &other)
	{
		copy(other);
	}

	CANMsg &operator=(CANMsg const &other)
	{
		copy(other);
		return *this;
	}

	void copy(CANMsg const &other)
	{
		id = other.id;
		length = other.length;
		for(uint8_t i = 0; i < 8; ++i)
		{
			data[i] = other.data[i];
		}
	}

    uint32_t id;
//    TPCANMessageType  MSGTYPE;
    uint8_t length;
    std::vector<int8_t> data;
};

inline
std::ostream &operator<<(std::ostream &os, CANMsg const &msg)
{
	os << "CAN msg 0x" << std::hex << msg.id << " with data : ";
	for(uint8_t i = 0; i < msg.length; ++i)
	{
		os << std::dec << (short)msg.data[i] << " ";
	}
	return os;
}

/// Uses non threaded version of pcan by manually requesting messages
/// @todo PCAN library shouldn't be required in Hydra
/// @todo we could also make it dynamic with the dll (sample in the PCAN c++ builder)
class PCAN : public InputDevice
{
	typedef boost::signal<void (CANMsg const &)> NewMessageSignal;
public :
	/// Create a pcan listener
	PCAN(void);

	/// Release a pcan listener
	~PCAN(void);

	/// Process all the messages
	void mainloop(void);

	/// Add a boost signals slot
	/// signals forward the message as CANMsg structure
	int addListener(NewMessageSignal::slot_type const &slot) 
	{ _signal.connect(slot); return 1; }

private :
	/// Disallow copy
	PCAN(PCAN const &other);
	PCAN &operator=(PCAN const &other);

	void _initialise(void);
	void _disconnect(void);
	void _set_connected(bool connected);

	/// Hardware information
	bool _connected;
	/// Hardware channel, a hexadecimal id number for the channel
	TPCANHandle _channel;
	/// Connection baudrate
	TPCANBaudrate _baudrate; 

	/// Callbacks
	NewMessageSignal _signal;

};	// class PCAN

}	// namespace vl

#endif // HYDRA_PCAN_HPP
