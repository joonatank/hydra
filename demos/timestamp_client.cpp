/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-03
 */

#include <boost/asio.hpp>

#include <boost/program_options.hpp>

#include <iostream>

#include <deque>

#include "timer.hpp"

namespace po = boost::program_options;
namespace asio = boost::asio;

const uint32_t MSG_REG_TIMESTAMP = 102;
const uint32_t MSG_TIMESTAMP = 103;
const uint32_t MSG_TIMESTAMP_BOUNCE = 104;

using asio::ip::udp;

bool g_verbose;

#define DLOG(str) if(g_verbose) std::cout << str << std::endl;

class TimestampClient
{
public :
	TimestampClient(std::string const &hostname, uint16_t port)
		: _io_service()
		, _socket( _io_service )
		, _master()
	{
		std::cout << "vl::cluster::Client::Client : Connecting to host "
			<< hostname << " at port " << port << "." << std::endl;

		std::stringstream ss;
		ss << port;
		udp::resolver resolver( _io_service );
		udp::resolver::query query( udp::v4(), hostname.c_str(), ss.str().c_str());
		_master = *resolver.resolve(query);

		_socket.open( udp::v4() );
	}

	/// Request time from server,
	/// Will block till the request is successful
	void request_time(std::ostream &os, bool absolute)
	{
		DLOG("Client::requesting time");
		{
			std::vector<char> buf(sizeof(MSG_REG_TIMESTAMP)+sizeof(char));
			::memcpy(&buf[0], &MSG_REG_TIMESTAMP, sizeof(MSG_REG_TIMESTAMP));
			size_t pos = sizeof(MSG_REG_TIMESTAMP);
			buf[pos] = (char)(absolute);
			_send_time = vl::get_system_time();
			_socket.send_to( boost::asio::buffer(buf), _master );
		}

		vl::timer send_timer;

		bool ready = false;
		while( !ready )
		{
			receive_msgs();
			while( !_messages.empty() )
			{
				DLOG("Client has new messages");
				std::vector<char> msg = _messages.front();

				_messages.pop_front();
				uint32_t typ = 0;
				char absolute;
				vl::time time;
				size_t pos = 0;
				::memcpy(&typ, &msg[0]+pos, sizeof(typ));
				pos += sizeof(typ);
				if( MSG_TIMESTAMP == typ )
				{
					::memcpy(&absolute, &msg[0]+pos, sizeof(char));
					pos += sizeof(char);
					::memcpy(&time, &msg[0]+pos, sizeof(time));
					pos += sizeof(time);

					os << "Message TIMESTAMP : ";
					if( (bool)(absolute) )
					{
						vl::time diff = time - _send_time;
						os << "absolute time = " << time << " waited for time from server for "
							<< send_timer.getTime() << "s." << std::endl
							<< "Difference between client send and server timestamp = "
							<< diff << std::endl;
					}
					else
					{ os << "simulation time = " << time << std::endl; }

					ready = true;
				}
				else
				{
					std::cout << "Received UNKNOW MSG : type = " << typ << std::endl;
				}
			}
		}
	}

	void receive_msgs(void)
	{
		while( _socket.available() > 0 )
		{
			std::vector<char> recv_buf( _socket.available() );
			boost::system::error_code error;
			_socket.receive_from( boost::asio::buffer(recv_buf),
				_master, 0, error );

			_messages.push_back(recv_buf);
		}
	}

private :
	vl::time _send_time;
	std::deque< std::vector<char> > _messages;
	asio::io_service _io_service;
	udp::socket _socket;
	udp::endpoint _master;

};

struct Options
{
	Options(void)
		: port(0), verbose(false)
	{}

	uint16_t port;
	bool verbose;
	std::string hostname;

	bool parse_options(int argc, char **argv)
	{
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce a help message")
			("verbose,v", "print the output to system console")
			("port,p", po::value<int>(), "port to connect to")
			("address,a", po::value<std::string>(), "hostname to connect to")
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

		if( vm.count("address") )
		{
			hostname = vm["address"].as<std::string>();
		}
		else
		{
			std::cout << "Hostname is required." << std::endl;
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

	TimestampClient client(options.hostname, options.port);

	// Test local time
	client.request_time(std::cout, false);
	// Run a few tests on time difference
	for( size_t i = 0; i < 20; ++i )
	{
		client.request_time(std::cout, true);
	}

	return 0;
}
