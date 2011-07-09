/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-06
 *	@file nv_swap_sync.cpp
 *
 *	This file is part of Hydra a VR game engine tests.
 *
 *	Test the usage of serial port.
 *
 *	Works only on Windows.
 *
 */

#ifdef _WIN32
#include <Windows.h>
#endif

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

const char *DEFAULT_COM_PORT = "COM1";
const int BAUD_RATE = CBR_9600;

/**	Specifications for communications
 *	TODO this is out dated
 *	Used baud rate 9600 in the Arduino
 * 
 *	Messages
 *	[MSG_READ_JOYSTICK]
 *	Request the joystick data from Arduino
 *
 *	[MSG_READ_JOYSTICK | N [ANALOG_ID CHANNEL DATA_HIGH DATA_LOW] | M [DIGITAL_ID CHANNEL DATA]]
 *	Response from Arduino
 *	Contains N analog channels and M digital channels every
 *	The order of the analog and digital channels is not significant
 *	The data for analog channel is 8 bits for id, 8 bits for channel, 
 *	8 bits for data high and 8 bits for data low,
 *	data is 10-bit integer (unsigned).
 *	The data for digital channel is 8 bits for id, 8 bits for channel and 8 bits for data,
 *	data is boolean (1 or 0).
 */

// Serial message ids
const int MSG_READ_JOYSTICK = 75;

const uint8_t ANALOG_ID = 0;
const uint8_t DIGITAL_ID = 1;

namespace po 
{
	using namespace boost::program_options;
}

namespace fs
{
	using namespace boost::filesystem;
}

struct options
{
	options(void)
		: com_port(DEFAULT_COM_PORT)
	{}

	bool parse(int argc, char **argv)
	{
		// Declare the supported options.
		po::options_description desc("Allowed options");
		desc.add_options()
			("help", "produce help message")
			("input-file,I", po::value<std::string>(), "Input file for the values to send.")
			("com", po::value<std::string>(), "Serial port to use.")
		;

		po::positional_options_description p;
		p.add("com", -1);

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				  options(desc).positional(p).run(), vm);
		po::notify(vm);

		if( vm.count("help") )
		{
			std::cout << desc << std::endl;
			return false;
		}

		if( vm.count("com") )
		{
			com_port = vm["com"].as<std::string>();
			std::cout << "COM port was set to " << com_port << "." << std::endl;
		}
		if( vm.count("input-file") )
		{
			input_file = vm["input-file"].as<std::string>();
			std::cout << "Input file was set to " << input_file << "." << std::endl;
			if( !fs::exists(input_file) )
			{
				std::cout << "No " << input_file << " found." << std::endl;
				return false;
			}
		}

		return true;
	}

	std::string com_port;
	// Not yet supported
	std::string input_file;
};

class Serial
{
public :
	Serial(std::string const &dev_name = std::string())
		: _hSerial(0)
	{
		try {
			if(!dev_name.empty())
			{ open(dev_name); }
		}
		catch(...)
		{
			close();
		}
	}

	void open(std::string const &dev_name)
	{
		_name = dev_name;
		_hSerial = CreateFile(dev_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if(_hSerial==INVALID_HANDLE_VALUE)
		{
			if(GetLastError()==ERROR_FILE_NOT_FOUND)
			{
				//serial port does not exist. Inform user.
				std::cerr << "ERROR : No such COM port." << std::endl;
			}
			else
			{
				//some other error occurred. Inform user.
				std::cerr << "ERROR : Unknow error type." << std::endl;
			}
			throw std::string("error");
		}

		// Set options
		DCB dcbSerialParams = {0};
		dcbSerialParams.DCBlength=sizeof(dcbSerialParams);
		if (!GetCommState(_hSerial, &dcbSerialParams)) 
		{
			//error getting state
			std::cerr << "Serial port : " << _name
				<< " couldn't get the state." << std::endl;
			throw std::string("error");
		}

		dcbSerialParams.BaudRate = BAUD_RATE;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.StopBits = ONESTOPBIT;
		dcbSerialParams.Parity = NOPARITY;
		if(!SetCommState(_hSerial, &dcbSerialParams))
		{
			//error setting serial port state
			std::cerr << "Serial port : " << _name
				<< " couldn't set the state." << std::endl;
			throw std::string("error");
		}

		COMMTIMEOUTS timeouts={0};
		timeouts.ReadIntervalTimeout=100;
		timeouts.ReadTotalTimeoutConstant=50;
		timeouts.ReadTotalTimeoutMultiplier=10;
		timeouts.WriteTotalTimeoutConstant=100;
		timeouts.WriteTotalTimeoutMultiplier=10;
		if(!SetCommTimeouts(_hSerial, &timeouts))
		{
			//error occureed. Inform user
			std::cerr << "Serial port : " << _name
				<< " couldn't set Timeouts." << std::endl;
			throw std::string("error");
		}
	}

	void close(void)
	{
		if(_hSerial)
		{
			CloseHandle(_hSerial);
			_hSerial = 0;
		}
	}

	/// @brief fill the vector with the data in the serial port
	/// blocking
	size_t read(std::vector<char> &buf, size_t n_bytes)
	{
		if(buf.size() == 0)
		{ throw(std::string("ERROR : in Serial::read buffer can't be zero length.")); }

		if(n_bytes == 0)
		{ n_bytes = buf.size(); }

		DWORD dwBytesRead = 0;
		if(!ReadFile(_hSerial, &buf[0], n_bytes, &dwBytesRead, NULL))
		{
			//error occurred. Report to user.
			throw(std::string("ERROR : in Serial::read"));
		}

		return dwBytesRead;
	}

	/// @brief write the vector of bytes to the serial port
	/// blocking
	size_t write(std::vector<char> const &buf, size_t n_bytes)
	{
		if(buf.size() == 0)
		{ throw(std::string("ERROR : in Serial::write buffer can't be zero length.")); }

		if(n_bytes == 0)
		{ n_bytes = buf.size(); }

		DWORD dwBytesWrite = 0;
		if(!WriteFile(_hSerial, &buf[0], n_bytes, &dwBytesWrite, NULL))
		{
			//error occurred. Report to user.
			throw(std::string("ERROR : in Serial::write"));
		}

		return dwBytesWrite;
	}

	~Serial(void)
	{
		close();
	}

private :
	HANDLE _hSerial;
	std::string _name;
};

// @todo change to vl::scalar
double convert_analog(uint16_t data)
{
	return ((double)data)/(1024.0/2) - 1.0;
}

struct JoystickEvent
{
	JoystickEvent(void)
		: axis_x(0)
		, axis_y(0)
		, axis_z(0)
		, buttons(0)
	{}

	~JoystickEvent(void) {}

	enum BUTTON
	{
		BUTTON0 = (1 << 0),
		BUTTON1 = (1 << 1),
		BUTTON2 = (1 << 2),
		BUTTON3 = (1 << 3),
		BUTTON4 = (1 << 4),
		BUTTON5 = (1 << 5),
		BUTTON6 = (1 << 6),
		BUTTON7 = (1 << 7),
	};

	// @brief get the button state based on the button index, from 0 to 8
	// This is quite dangerous function as the number could be casted to
	// BUTTON also, which would be terrible idea as BUTTON is the hex code
	// in the button structure.
	bool isButtonDown(size_t index) const
	{
		return(buttons & (1 << index));
	}

	bool isButtonDown(BUTTON bt) const
	{
		return(buttons & bt);
	}

	void setButtonDown(BUTTON index, bool down)
	{
		setButtonDown((uint8_t)index, down);
	}

	void setButtonDown(uint8_t index, bool down)
	{
		if(down)
		{ buttons |= (1 << index); }
		else
		{ buttons &= ~(1 << index); }
	}

	// @todo change to vl::scalar
	double axis_x;
	double axis_y;
	double axis_z;

	uint32_t buttons;
};

std::ostream &operator<<(std::ostream &os, JoystickEvent const &evt)
{
	os << "Axis = (" << evt.axis_x << ", " << evt.axis_y << ", " << evt.axis_z << ")" << std::endl;
	// @todo add buttons;
	for(size_t i = 0; i < 8; ++i)
	{
		os << "Button " << i << " : " << evt.isButtonDown(i) << std::endl;
	}

	return os;
}

struct JoystickReaderCallback 
{
	virtual void valueChanged(JoystickEvent const &evt) = 0;
};

struct PrintJoystickCallback : public JoystickReaderCallback
{
	PrintJoystickCallback(std::ostream &os)
		: _stream(os)
	{}

	virtual void valueChanged(JoystickEvent const &evt)
	{
		std::cout << "New joystick values : " << evt << std::endl;
	}

	std::ostream &_stream;
};

class SerialJoystickReader
{
public :
	SerialJoystickReader(std::string const &device)
		: _serial(device)
		, _cb(0)
	{
	}

	// Old parse function used for testing that it works
	void _parse_print(std::vector<char> msg, size_t bytes)
	{
		/// Parse the message
		assert(msg.at(0) == MSG_READ_JOYSTICK);
		int b = 1;
		while(b < bytes)
		{
			uint8_t id(msg.at(b));
			b += 1;
			if(ANALOG_ID == id)
			{
				uint16_t channel(uint8_t(msg.at(b)));
				uint8_t high_byte(msg.at(b+1));
				uint8_t low_byte(msg.at(b+2));
				uint16_t data = (high_byte << 8) | low_byte;
				double analog = convert_analog(data);
				std::cout << "Got response from analog channel " << channel
					<< " converted analog value : " << analog << std::endl;
				b += 3;
			}
			else if(DIGITAL_ID == id)
			{
				uint16_t channel(uint8_t(msg.at(b)));
				uint16_t data(uint8_t(msg.at(b+1)));
				std::cout << "Digital channel " << channel << " value " 
					<< data << std::endl;
				b += 2;
			}
			else
			{
				std::cout << "Not analog or digital channel." << std::endl;
			}
		}
	}

	// New parse function that creates the event structure
	JoystickEvent _parse(std::vector<char> msg, size_t bytes)
	{
		/// Parse the message
		assert(msg.at(0) == MSG_READ_JOYSTICK);
		
		JoystickEvent evt;
		int b = 1;
		while(b < bytes)
		{
			uint8_t id(msg.at(b));
			b += 1;
			if(ANALOG_ID == id)
			{
				uint16_t channel(uint8_t(msg.at(b)));
				uint8_t high_byte(msg.at(b+1));
				uint8_t low_byte(msg.at(b+2));
				uint16_t data = (high_byte << 8) | low_byte;
				b += 3;
				double analog = convert_analog(data);
				
				if(channel == 0)
				{
					evt.axis_x = analog;
				}
				else if(channel == 1)
				{
					evt.axis_y = analog;
				}
				else if(channel == 2)
				{
					evt.axis_z = analog;
				}
				else
				{
					std::cerr << "Unkown analog axis : " << channel << std::endl;
				}
			}
			else if(DIGITAL_ID == id)
			{
				uint8_t channel(uint8_t(msg.at(b)));
				uint8_t data(uint8_t(msg.at(b+1)));
				b += 2;
//				std::cout << "Digital channel " << channel << " value " 
//					<< data << std::endl;
				evt.setButtonDown(channel, (bool)data);
			}
			else
			{
				// @todo this should throw as the message is incorrect
				std::cerr << "Not analog or digital channel." << std::endl;
			}
		}

		return evt;
	}

	void mainloop(void)
	{
		// Get new data
		// Write a request to the serial port for reading analog channel 0
		std::vector<char> buf(128);
		buf.at(0) = char(MSG_READ_JOYSTICK);
		size_t bytes = _serial.write(buf, 1);
		// @todo replace by throwing
		if(bytes != 1)
		{
			std::cerr << "Something fishy : wrote " << bytes << " instead of 1." << std::endl;
		}

		bytes = _serial.read(buf, 128);

		JoystickEvent evt = _parse(buf, bytes);

		// Callback
		if(_cb)
		{
			_cb->valueChanged(evt);
		}
	}

	void setCallback(JoystickReaderCallback *cb)
	{ _cb = cb; }

	JoystickReaderCallback *getCallback(void) const
	{ return _cb; }

private :
	JoystickReaderCallback *_cb;
	Serial _serial;
};

int main(int argc, char **argv)
{
	try {
		options opt;
		opt.parse(argc, argv);

		PrintJoystickCallback cb(std::cout);
		SerialJoystickReader joy(opt.com_port);
		joy.setCallback(&cb);
		// Read the response
		while(true)
		{
			joy.mainloop();

			Sleep(100);
		}
	}
	catch(std::string const &e)
	{
		std::cerr << "Exception : " << e << " thrown." << std::endl;
	}
	catch(...)
	{
		std::cerr << "Exception thrown." << std::endl;
	}

	return 0;
}

