/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#ifndef OGRE_GLSUPPORT_H
#define OGRE_GLSUPPORT_H

#include "OgreGLPrerequisites.h"
#include "OgreGLRenderSystem.h"

#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreConfigOptionMap.h>

namespace Ogre
{

/// @todo what is the purpose of this class?
/// Why is it separate from GLContext and RenderSystem?
class _OgreGLExport GLSupport
{
public:
    GLSupport() { }
    virtual ~GLSupport() { }

	/// Static create function for easier usage and choosing between
	/// different platform versions.
	static GLSupport *create(void);

    /**
    * Add any special config values to the system.
    * Must have a "Full Screen" value that is a bool and a "Video Mode" value
    * that is a string in the form of wxh
    */
    virtual void addConfig() = 0;

	virtual void setConfigOption(const String &name, const String &value);

    /**
    * Make sure all the extra options are valid
    * @return string with error message
    */
    virtual String validateConfig() = 0;

	virtual ConfigOptionMap& getConfigOptions(void);

    /**
    * Start anything special
    */
    virtual void start() = 0;
    /**
    * Stop anything special
    */
    virtual void stop() = 0;

    /**
    * Get vendor information
    */
    const String& getGLVendor(void) const
    {
        return mVendor;
    }

    /**
    * Get version information
    */
    const String& getGLVersionString(void) const
    {
        return mVersion;
    }

	Ogre::DriverVersion getGLVersion(void) const;

    /**
    * Compare GL version numbers
    */
    bool checkMinGLVersion(const String& v) const;

	/// @copydoc RenderSystem::getDisplayMonitorCount
	virtual unsigned int getDisplayMonitorCount() const
	{
		return 1;
	}

	/// @brief check for attributes we need the GPU to have
	/// for example only NVidia, ATI and Intel are supported
	/// FBOs are required 
	/// OpenGL 2.0 might became a requirement later.
	bool isValidGPU(void) const;

protected:
	/// @brief Initialises GL extensions.
	///	@preconditions Valid OpenGL Context is set (can be temporary)
	/// @todo replace with Template Method Pattern
	///  and call _initialiseExtensions from this Classes init method
	virtual void _initialiseExtensions(void);

protected:
	// Stored options
    ConfigOptionMap mOptions;

	// This contains the complete list of supported extensions
    set<String>::type extensionList;

private:
    String mVersion;
    String mVendor;

}; // class GLSupport

}; // namespace Ogre

#endif // OGRE_GLSUPPORT_H
