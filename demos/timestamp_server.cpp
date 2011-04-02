/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file timestamp_server.cpp
 *
 *	@todo Change to use high performance timers at the moment timer accuracy is
 *	millisecond, which is not nearly enough for performance timings.
 */

#include <boost/asio.hpp>

#include <boost/program_options.hpp>

#include <iostream>

#include "base/timer.hpp"

namespace po = boost::program_options;
namespace asio = boost::asio;

bool g_verbose;
#define DLOG(str) if(g_verbose) std::cout << str << std::endl;

/** Timestamp messages
 *
 *	MSG_REG_TIMESTAMP
 *	Request a timestamp from server, timestamp is sent back to the client
 *	[MSG_REG_TIMESTAMP | ABSOLUTE]
 *	4 bytes (uint32_t), 1 byte (char)
 *	CHAR ABSOLUTE, use 1 for requesting absolute timestamp, i.e. current computer time
 *	use 0 for requesting relative timestamp, i.e. time since the server was started.
 *
 *	MSG_TIMESTAMP
 *	Timestamp message
 *	[MSG_TIMESTAMP | ABSOLUTE | TIME]
 *	4 bytes (uint32_t), 1 byte (char), 8 bytes (pair of uint32_t, timeval)
 *	ABSOLUTE is 1 if client requested it, 0 otherwise
 *	TIME is a timeval, measured from the CPU ticks so it will only change on reboot
 *	timeval struct contains two 32-bit? (long) integers, seconds and microseconds
 *
 *	If ABOSULTE is requested time is measured since the system was rebooted
 *	If relative is requested time is measured since simulation start
 *
 *	Reasons for abosolute and relative
 *	Absolute is used to measure performance on a single system, it might be
 *	possible to use it accross multiple systems if their difference is measured
 *	accurately.
 *	Relative is used for timestamping simulation messages and for saving them
 *	to disk.
 *	Absolute time has no meaning after a reboot, relative time has no meaning
 *	in the system context (only simulation).
 *
 *	MSG_TIMESTAMP_BOUNCE
 *	Used to bounce the message from a client
 *	[MSG_TIMESTAM_BOUNCE | ABSOLUTE | TIME]
 *	4 bytes (uint32_t), 1 byte, 8 bytes (pair of uint32_t, timeval)
 *	Client sends the same message as the one server sent
 *	Used to measure round trip time.
 */
const uint32_t MSG_REG_TIMESTAMP = 102;
const uint32_t MSG_TIMESTAMP = 103;
const uint32_t MSG_TIMESTAMP_BOUNCE = 104;

const size_t BUF_SIZE = 1024;

using asio::ip::udp;

class TimestampServer
{
public :
	TimestampServer(uint16_t port)
		: _io_service()
		, _socket(_io_service, udp::endpoint(udp::v4(), port))
	{}

	void poll(void)
	{
		// Poll all the messages at once
		while( _socket.available() )
		{
			char data[BUF_SIZE];
			udp::endpoint sender_endpoint;
			size_t length = _socket.receive_from(
				boost::asio::buffer(data, BUF_SIZE), sender_endpoint);

			if( length >= sizeof(uint32_t) + sizeof(char) )
			{
				uint32_t type;
				char absolute;
				size_t pos = 0;
				::memcpy(&type, data+pos, sizeof(type));
				pos = sizeof(type);
				absolute = data[pos];
				pos++;
				if( type == MSG_REG_TIMESTAMP )
				{
					DLOG("Received MSG_REG_TIMESTAMP")
					sendTimestamp(sender_endpoint, bool(absolute));
				}
				else if( type == MSG_TIMESTAMP_BOUNCE )
				{
					DLOG("Received MSG_TIMESTAMP_BOUNCE")
					uint64_t time;
					::memcpy(&time, data+pos, sizeof(time));
					pos += sizeof(time);
					handleBounce(time, bool(absolute));
				}
				else
				{
					std::cout << "Received UNKNOW MSG" << std::endl;
				}
			}
			else
			{
				std::cerr << "Client send too short a message." << std::endl;
			}
		}
	}

	void sendTimestamp(udp::endpoint const &endpoint, bool absolute)
	{
		DLOG("Sending TimeStamp")
		std::vector<char> buf(sizeof(MSG_TIMESTAMP)+1+sizeof(vl::time));
		::memcpy( &buf[0], &MSG_TIMESTAMP, sizeof(MSG_TIMESTAMP) );
		size_t pos = sizeof(MSG_TIMESTAMP);
		if( absolute )
		{
			buf[pos] = (char)(1);
			pos++;
			vl::time t = vl::get_system_time();
			::memcpy( &buf[0]+pos, &t, sizeof(t) );
			pos += sizeof(t);
		}
		else
		{
			buf[pos] = (char)0;
			pos++;
			vl::time t = _start_timer.getTime();
			::memcpy( &buf[0]+pos, &t, sizeof(t) );
			pos += sizeof(t);
		}

		std::stringstream ss;
		ss << "sending message = ";
		for(size_t i = 0; i < buf.size(); ++i )
			ss << std::hex << (short)(buf[i]);
		DLOG(ss.str());

		// Send timestamp back
		_socket.send_to(boost::asio::buffer(buf), endpoint);
	}

	void handleBounce(uint64_t time, bool absolute)
	{
		std::cerr << "Bounce not implemented" << std::endl;
	}

private:
	asio::io_service _io_service;
	udp::socket _socket;
	vl::timer _start_timer;
};

struct Options
{
	uint16_t port;
	bool verbose;

	bool parse_options(int argc, char **argv)
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce a help message")
			("verbose,v", "print the output to system console")
			("port,p", po::value<int>(), "port for the time stamp server")
		;

		// Parse command line
		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		// Print help
		if( vm.count("help") )
		{
			std::cout << "Help : " << desc << "\n";
			return false;
		}

		// Verbose
		if( vm.count("verbose") )
		{
			verbose = true;
		}

		if( vm.count("port") )
		{
			port = vm["port"].as<int>();
		}
		else
		{
			std::cout << "Port is required." << std::endl;
			return false;
		}

		return true;
	}
};

int main(int argc, char **argv)
{
	using boost::asio::ip::udp;

	Options options;

	if( !options.parse_options(argc, argv) )
	{ return -1; }
	g_verbose = options.verbose;

	std::cout << "Time accuracy is " << vl::get_system_time_accuracy() << " microseconds." << std::endl;

	TimestampServer server(options.port);
	while(true)
	{
		server.poll();
	}

	return 0;
}
