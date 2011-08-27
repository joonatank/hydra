/**	@author Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	@date 2011-08
 *	@file base/system_utils.cpp
 *
 *	This file is part of Hydra VR game engine.
 */

#include "system_util.hpp"

#ifdef VL_WIN32
// Necessary for GetCurrentProcessId
#include <Windows.h>
#else
// Necessary for getpid and fork
#include <unistd.h>
#endif

uint32_t
vl::getPid(void)
{
#ifdef _WIN32
	return GetCurrentProcessId();
#else
	return ::getpid();
#endif
}

uint32_t
vl::create_process(std::string const &path, std::vector<std::string> const &params, bool create_new_console)
{
	uint32_t id = 0;
	size_t size = 0;
	size += (3+path.size());

	for(size_t i = 0; i < params.size(); ++i)
	{
		// Extra space one for the space delimeter and two for the quetation marks
		size += (params.at(i).size() + 3);
	}
	// Null termination
	size += 1;

	// Convert params to C compatible string with space delimeter and quetation marks for params
	char *argv = new char[size];
	size_t offset = 0;
	
	argv[offset] = '\"';
	offset++;
	for(size_t i = 0; i < path.size(); ++i)
	{
		argv[offset] = path.at(i);
		offset++;
	}
	argv[offset++] = '\"';
	argv[offset++] = ' ';
	//offset++;

	for(size_t i = 0; i < params.size(); ++i)
	{
		argv[offset] = '\"';
		++offset;
		
		std::string const &arg = params.at(i);
		for(size_t j = 0; j < arg.size(); ++j)
		{
			argv[offset] = arg.at(j);
			++offset;
		}

		argv[offset] = '\"';
		++offset;
		argv[offset] = ' ';
		++offset;
	}

	// Null terminate
	argv[size-1] = '\0';

#ifdef _WIN32
	
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	DWORD flags = 0;
	if(create_new_console)
	{ flags |= CREATE_NEW_CONSOLE; }

    // Start the child process. 
	if( !::CreateProcess(NULL,
		argv,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		flags,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // STARTUPINFO, necessary
		&pi)			// PROCESS_INFORMATION, necessary
	)
	{
		// @todo add the use of GetLastError for error msgs, or use boost::system
		std::string err("CreateProcess failed ");
		BOOST_THROW_EXCEPTION( vl::exception() << vl::desc(err));
	}
	id = pi.dwProcessId;
#else
	// Not implemented
#error "create_process not implemented for Linux"
#endif

	// Cleanup
	delete [] argv;

	return id;
}

void
vl::kill_process(uint32_t pid)
{
	// Hard coded exit code
	uint32_t exit_code = 0;
#ifdef _WIN32
	// @tood get handle from pid
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

	::TerminateProcess(hProcess, exit_code);

	::CloseHandle(hProcess);
#else
#error "kill_process not implemented for Linux"
#endif
}
