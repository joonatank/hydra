/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 *	@file timestamp_server.cpp
 *
 *	Puporse for this is to test UDP message performance
 *	Message round trip time should be less than one millisecond
 *	preferably less than 100 microseconds.
 *
 *	@todo fails miserably in Linux (round trip time ~20ms)
 *	Linux needs sleeping on the server side,
 *	adding a sleep of 0ms the round trip time is less than 100 microseconds.
 *	No difference between asynchronious and synchronious servers, which is not a
 *	big suprise with so simple servers.
 *	@todo Add results on Windows with sleeping
 *	If the results are bad we need a switch to enable sleeping in Linux and
 *	disable it on Windows (for example sleep 0ms or negative sleep time).
 *	On Windows 7 64-bit 
 *		with 0ms sleeping bit over 200 microseconds round trip time.
 *		with 1ms sleeping bit over 1ms round trip time.
 *		without sleeping around 150 microseconds round trip time, varied from 80-250.
 *	Seems like on Windows 7 sleeping 0ms is the same as not sleeping. 
 *	The server is using a complete CPU core.
 *
 *	Working implementation is copied to the Hydra communication system and used
 *	as a reference for the latency there.
 *
 *	Uses high performance timers implemented in Hydra
 *	Provides both async and sync servers to test the difference in performance.
 */

#include <boost/asio.hpp>

#include <boost/program_options.hpp>

#include <iostream>

// Necessary for binding callback functions
#include <boost/bind.hpp>

// Necessary for retrieving timestamp information
#include "base/timer.hpp"
// Necessary for sleeping
#include "base/sleep.hpp"

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

	virtual void poll(void) = 0;

	void handleMessage(udp::endpoint const &endpoint, std::vector<char> const &msg)
	{
		if( msg.size() >= sizeof(uint32_t) + sizeof(char) )
		{
			uint32_t type;
			char absolute;
			size_t pos = 0;
			::memcpy(&type, &msg[0]+pos, sizeof(type));
			pos = sizeof(type);
			absolute = msg[pos];
			pos++;
			if( type == MSG_REG_TIMESTAMP )
			{
				DLOG("Received MSG_REG_TIMESTAMP")
				sendTimestamp(endpoint, bool(absolute));
			}
			else if( type == MSG_TIMESTAMP_BOUNCE )
			{
				DLOG("Received MSG_TIMESTAMP_BOUNCE")
				uint64_t time;
				::memcpy(&time, &msg[0]+pos, sizeof(time));
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
			vl::time t = _start_timer.elapsed();
			::memcpy( &buf[0]+pos, &t, sizeof(t) );
			pos += sizeof(t);
		}

		std::stringstream ss;
		ss << "sending message = ";
		for(size_t i = 0; i < buf.size(); ++i )
			ss << std::hex << (short)(buf[i]);
		DLOG(ss.str());

		// Send timestamp back
		sendMessage(buf, endpoint);
	}

	virtual void sendMessage(std::vector<char> const &msg, udp::endpoint const &endpoint) = 0;

	void handleBounce(uint64_t time, bool absolute)
	{
		std::cerr << "Bounce not implemented" << std::endl;
	}

protected:
	asio::io_service _io_service;
	udp::socket _socket;

	vl::timer _start_timer;
};


class AsyncTimestampServer : public TimestampServer
{
public :
	AsyncTimestampServer(uint16_t port)
		: TimestampServer(port), _data(BUF_SIZE)
	{
		_socket.async_receive_from(
			boost::asio::buffer(_data, BUF_SIZE), _sender_endpoint,
			boost::bind(&AsyncTimestampServer::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	/// ------------------- Overrides from TimestampServer ---------------------
	virtual void poll(void)
	{
		_io_service.poll();
	}

	virtual void sendMessage(std::vector<char> const &msg, udp::endpoint const &endpoint)
	{
		/// @todo what are the placeholders for?
		_socket.async_send_to(
			boost::asio::buffer(msg), _sender_endpoint,
			boost::bind( &AsyncTimestampServer::handle_send_to, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred ) );
	}

	/// ----------------- Callback handlers ------------------------------------
	void handle_receive_from(const boost::system::error_code& error,
		size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
			/// @todo Resizes are not good
			_data.resize(bytes_recvd);
			handleMessage(_sender_endpoint, _data);
			/*
			_socket.async_send_to(
				boost::asio::buffer(_data, bytes_recvd), _sender_endpoint,
				boost::bind( &AsyncTimestampServer::handle_send_to, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred ) );
			*/
		}
		/// Problems try another receive
		else
		{
			_socket.async_receive_from(
				boost::asio::buffer(_data, BUF_SIZE), _sender_endpoint,
				boost::bind(&AsyncTimestampServer::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
		}
	}

	void handle_send_to(const boost::system::error_code& /*error*/,
		size_t /*bytes_sent*/)
	{
		_socket.async_receive_from(
			boost::asio::buffer(_data, BUF_SIZE), _sender_endpoint,
			boost::bind(&AsyncTimestampServer::handle_receive_from, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

private :
	udp::endpoint _sender_endpoint;
	std::vector<char> _data;
};

class SyncTimestampServer : public TimestampServer
{
public :
	SyncTimestampServer(uint16_t port)
		: TimestampServer(port)
	{}

	virtual void poll(void)
	{
		// Poll all the messages at once
		while( _socket.available() )
		{
			std::vector<char> data(BUF_SIZE);
			udp::endpoint sender_endpoint;
			size_t length = _socket.receive_from(
				boost::asio::buffer(data), sender_endpoint);

			data.resize(length);
			handleMessage(sender_endpoint, data);
		}
	}

	virtual void sendMessage(std::vector<char> const &msg, udp::endpoint const &endpoint)
	{
		_socket.send_to(boost::asio::buffer(msg), endpoint);
	}

};


struct Options
{
	Options(void)
		: port(0), sleep(0), verbose(false), async(false)
	{}

	uint16_t port;
	uint16_t sleep;
	bool verbose;
	bool async;

	bool parse_options(int argc, char **argv)
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce a help message")
			("verbose,v", "print the output to system console")
			("async,a", "use async server rather than sync")
			("sleep,s", po::value<int>(), "how long to sleep")
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
			if( vm["port"].as<int>() <= 0)
			{
				std::cout << "Port can not be zero or negative" << std::endl;
				return false;
			}
			port = vm["port"].as<int>();
		}
		else
		{
			std::cout << "Port is required." << std::endl;
			return false;
		}

		if( vm.count("async") )
		{
			async = true;
		}

		if( vm.count("sleep") )
		{
			if( vm["sleep"].as<int>() < 0)
			{
				std::cout << "Sleep time can not be negative" << std::endl;
				return false;
			}
			sleep = vm["sleep"].as<int>();
		}

		return true;
	}
};

int main(int argc, char **argv)
{
	using boost::asio::ip::udp;
	try {

		Options options;

		if( !options.parse_options(argc, argv) )
		{ return -1; }
		g_verbose = options.verbose;

		std::cout << "Time accuracy is " << vl::get_system_time_accuracy() << " microseconds." << std::endl;

		boost::scoped_ptr<TimestampServer> server;
		if( options.async )
		{
			std::cout << "Using asynchronious server." << std::endl;
			server.reset(new AsyncTimestampServer(options.port));
		}
		else
		{
			std::cout << "Using synchronious server." << std::endl;
			server.reset(new SyncTimestampServer(options.port));
		}

		while(true)
		{
			assert(server);
			server->poll();
			vl::msleep(options.sleep);
		}
	}
	catch( std::exception &e )
	{
		std::cerr << "Exception: " << e.what() << std::endl;
	}

	return 0;
}
