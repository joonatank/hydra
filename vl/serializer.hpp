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

#ifndef HYDRA_SERIALIZER_HPP
#define HYDRA_SERIALIZER_HPP

#include "resource.hpp"

#include <OGRE/OgreVector2.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreQuaternion.h>

//#include <OGRE/OgreMatrix4.h>

#include <stdint.h>

namespace vl {

/** Generic class for serialising data to / from binary stream-based files.
@remarks
    This class provides a number of useful methods for exporting / importing data
    from stream-oriented binary files (e.g. .mesh and .skeleton).
*/
class Serializer 
{
public:
    Serializer();
    virtual ~Serializer();

	/// The endianness of written files
	enum Endian
	{
		/// Use the platform native endian
		ENDIAN_NATIVE,
		/// Use big endian (0x1000 is serialised as 0x10 0x00)
		ENDIAN_BIG,
		/// Use little endian (0x1000 is serialised as 0x00 0x10)
		ENDIAN_LITTLE
	};


protected:

    uint32_t mCurrentstreamLen;
    vl::ResourceStream *mStream;
    std::string mVersion;
	bool mFlipEndian; // default to native endian, derive from header

	// Internal methods
	virtual void writeFileHeader(void);
	virtual void writeChunkHeader(uint16_t id, size_t size);

	void writeFloats(const float * const pfloat, size_t count);
	void writeFloats(const double * const pfloat, size_t count);
	void writeShorts(const uint16_t * const pShort, size_t count);
	void writeInts(const uint32_t * const pInt, size_t count); 
	void writeBools(const bool * const pLong, size_t count);
	void writeObject(Ogre::Vector3 const &vec);
	void writeObject(Ogre::Quaternion const &q);
	// @warning added for passing viewmatrix to mouse events
	//Generally do not send or receive anything which is a 4x4 matrix,
	//instead if possible use quaternion and vector pairs!
	//void writeObject(Ogre::Matrix4 const &m);

    void writeString(std::string const &string);
    void writeData(const void* const buf, size_t size, size_t count);

	virtual void readFileHeader(vl::ResourceStream &stream);
	virtual unsigned short readChunk(vl::ResourceStream &stream);

	void readBools(vl::ResourceStream &stream, bool * pDest, size_t count);
	void readFloats(vl::ResourceStream &stream, float * pDest, size_t count);
	void readFloats(vl::ResourceStream &stream, double * pDest, size_t count);
	void readShorts(vl::ResourceStream &stream, uint16_t *pDest, size_t count);
	void readInts(vl::ResourceStream &stream, uint32_t *pDest, size_t count);
	void readObject(vl::ResourceStream &stream, Ogre::Vector3 &pDest);
	void readObject(vl::ResourceStream &stream, Ogre::Vector2 &pDest);
	void readObject(vl::ResourceStream &stream, Ogre::Quaternion &pDest);
	// @warning added for passing viewmatrix to mouse events
	//Generally do not send or receive anything which is a 4x4 matrix,
	//instead if possible use quaternion and vector pairs!
	
	//void readObject(vl::ResourceStream &stream, Ogre::Matrix4 &pDest);

	std::string readString(vl::ResourceStream &stream);
	std::string readString(vl::ResourceStream &stream, size_t numChars);
        
    virtual void flipToLittleEndian(void* pData, size_t size, size_t count = 1);
    virtual void flipFromLittleEndian(void* pData, size_t size, size_t count = 1);
        
    virtual void flipEndian(void * pData, size_t size, size_t count);
    virtual void flipEndian(void * pData, size_t size);

	/// Determine the endianness of the incoming stream compared to native
	virtual void determineEndianness(vl::ResourceStream &stream);
	/// Determine the endianness to write with based on option
	virtual void determineEndianness(Endian requestedEndian);
};

}	// namespace vl

#endif	// HYDRA_SERIALIZER_HPP
