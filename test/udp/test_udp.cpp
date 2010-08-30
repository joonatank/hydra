/*	Joonatan Kuosa
 *	2010-08
 *	Test program that contains both udp::Server and udp::Client
 */
#ifdef VL_UNIX
#define BOOST_TEST_DYN_LINK
#endif

#define BOOST_TEST_MODULE test_udp

#include <boost/test/unit_test.hpp>

// Standard headers
#include <iostream>

// UNIX headers
#include <time.h>

#include "udp/server.hpp"
#include "udp/client.hpp"
#include "udp/print_command.hpp"
#include "base/sleep.hpp"
#include "base/print.hpp"
#include "base/exceptions.hpp"

// Test includes
#include "udp_fixtures.hpp"

// Tolerance used in floating point comparison in percents 
double const TOLERANCE = 1e-3;

/// Test command which checks that the sended and received values are equal
class AssertCommand : public vl::udp::Command
{
public :
	AssertCommand( vl::udp::CMD_TYPE type, std::vector<double> check_arr )
		: _type(type), _check_values(check_arr)
	{
		if( _type == vl::udp::CMD_POS )
		{
			BOOST_REQUIRE_EQUAL( _check_values.size(), 3 );
		}
		else if( _type == vl::udp::CMD_ROT_QUAT )
		{
			BOOST_REQUIRE_EQUAL( _check_values.size(), 4 );
		}
		else if( _type == vl::udp::CMD_ROT_AA )
		{
			BOOST_REQUIRE_EQUAL( _check_values.size(), 4 );
		}
		else
		{
			BOOST_FAIL( "Incorrect Message type" );
		}
		_data.resize( _check_values.size() );
	}

	virtual void operator()( void )
	{
		BOOST_REQUIRE_EQUAL( _data.size(), _check_values.size() );

		for( size_t i = 0; i < _data.size(); ++i )
		{
			BOOST_CHECK_CLOSE( _data.at(i), _check_values.at(i), TOLERANCE );
		}
	}

	/// Returns the number of elements in the array for this command
	/// All elements are type double for now.
	virtual uint16_t getSize( void ) const
	{
		return _data.size();
	}

	virtual vl::udp::CMD_TYPE getType( void ) const
	{
		return _type;
	}

	virtual double &at( size_t i )
	{
		return _data.at(i);
	}

	virtual double const &at( size_t i ) const
	{
		return _data.at(i);
	}

	vl::udp::CMD_TYPE _type;
	std::vector<double> _data;
	std::vector<double> _check_values;
};

/// Test command used for incorrect packets
class EmptyCommand : public vl::udp::Command
{
public :
	EmptyCommand( vl::udp::CMD_TYPE type, std::vector<double> check_arr )
		: _type(type)
	{
	}

	virtual void operator()( void )
	{
	}

	virtual uint16_t getSize( void ) const
	{
		if( _type == vl::udp::CMD_POS )
		{ return 3; }
		else if( _type == vl::udp::CMD_ROT_QUAT )
		{ return 4; }
		else if( _type == vl::udp::CMD_ROT_AA )
		{ return 4; }
		else
		{ return 0; }
	}

	virtual vl::udp::CMD_TYPE getType( void ) const
	{
		return _type;
	}

	virtual double &at( size_t i )
	{
		return tmp;
	}

	virtual double const &at( size_t i ) const
	{
		return tmp;
	}

	vl::udp::CMD_TYPE _type;
	double tmp;
};

BOOST_FIXTURE_TEST_SUITE( TestUDP, TestUdpFixture )

BOOST_AUTO_TEST_CASE( sending )
{
	boost::shared_ptr<vl::udp::Command> cmd( new AssertCommand( vl::udp::CMD_POS, msg_pos ) );
	server.addCommand( cmd );
	cmd.reset( new AssertCommand( vl::udp::CMD_ROT_QUAT, msg_rot_quat ) );
	server.addCommand( cmd );
	cmd.reset( new AssertCommand( vl::udp::CMD_ROT_AA, msg_rot_aa ) );
	server.addCommand( cmd );

	std::vector<double> msg;
	add_vec( msg, msg_pos );
	add_vec( msg, msg_rot_quat );
	add_vec( msg, msg_rot_aa );

	BOOST_CHECK_NO_THROW( send( msg ) );
}

// Test throwing when too short packet is sent from client
BOOST_AUTO_TEST_CASE( too_short_packet )
{
	boost::shared_ptr<vl::udp::Command> cmd( new EmptyCommand( vl::udp::CMD_POS, msg_pos ) );
	server.addCommand( cmd );
	
	// Create too short a message
	std::vector<double> msg( msg_pos );
	msg.resize( msg.size()-1 );

	// TODO we should check that the number of bytes missing is correct
	BOOST_CHECK_THROW( send( msg ), vl::short_message );
}

// Test throwing when too long packet is sent from client
BOOST_AUTO_TEST_CASE( too_long_packet )
{
	boost::shared_ptr<vl::udp::Command> cmd( new EmptyCommand( vl::udp::CMD_POS, msg_pos ) );
	server.addCommand( cmd );
	
	// Create too short a message
	std::vector<double> msg( msg_pos );
	msg.resize( msg.size()+1 );

	// TODO we should check that the number of bytes too many is correct
	BOOST_CHECK_THROW( send( msg ), vl::long_message );
}

BOOST_AUTO_TEST_SUITE_END()