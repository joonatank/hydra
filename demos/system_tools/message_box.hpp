/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@file message_box.hpp
 *	@date 2011-05
 *
 *	Class to display system erros
 *	Multi platform wrapper around native versions
 *	Minimal implementation that does not depend on heavy GUI libraries
 *
 *	@todo add Linux version using Xlib
 */

#ifndef HYDRA_SYSTEM_MESSAGE_BOX_HPP
#define HYDRA_SYSTEM_MESSAGE_BOX_HPP

#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace vl
{

class MessageDialog
{
public :
	MessageDialog(std::string const &title, std::string const &message)
	{
#ifdef _WIN32
		int msgboxID = MessageBox(
			NULL, message.c_str(), title.c_str(),
			MB_ICONERROR | MB_OK
		);

#else
		/// @todo add Linux support
		std::cerr << message << std::endl;
#endif
	}

	~MessageDialog(void)
	{
	}

};	// class MessageBox

}	// namespace vl

#endif	// HYDRA_SYSTEM_MESSAGE_BOX_HPP