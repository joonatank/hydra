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

#include "serializer.hpp"

#include "base/exceptions.hpp"

/// stream overhead = ID + size
const size_t STREAM_OVERHEAD_SIZE = sizeof(uint16_t) + sizeof(uint32_t);
const uint16_t HEADER_STREAM_ID = 0x1000;
const uint16_t OTHER_ENDIAN_HEADER_STREAM_ID = 0x0010;
//---------------------------------------------------------------------
vl::Serializer::Serializer()
	: mStream(0)
{}

//---------------------------------------------------------------------
vl::Serializer::~Serializer()
{}

//---------------------------------------------------------------------
void 
vl::Serializer::determineEndianness(vl::ResourceStream &stream)
{
	if(stream.tell() != 0)
	{
		std::string msg("Can only determine the endianness of the input stream if it is at the start");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
	}

	uint16_t dest;
	// read header id manually (no conversion)
    size_t actually_read = stream.read(dest);
	// skip back
    stream.skip(0 - (long)actually_read);
    if (actually_read != sizeof(uint16_t))
    {
        // end of file?
		std::string msg("Couldn't read 16 bit header value from input stream.");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
    }
	if (dest == HEADER_STREAM_ID)
	{
		mFlipEndian = false;
	}
	else if (dest == OTHER_ENDIAN_HEADER_STREAM_ID)
	{
		mFlipEndian = true;
	}
	else
	{
		std::string msg("Header chunk didn't match either endian: Corrupted stream?");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
	}
}
//---------------------------------------------------------------------
void 
vl::Serializer::determineEndianness(vl::Serializer::Endian requestedEndian)
{
	/*	Endian flipping not supported for now
	switch(requestedEndian)
	{
	case ENDIAN_NATIVE:
		mFlipEndian = false;
		break;
	case ENDIAN_BIG:
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		mFlipEndian = false;
#else
		mFlipEndian = true;
#endif
		break;
	case ENDIAN_LITTLE:
#if OGRE_ENDIAN == OGRE_ENDIAN_BIG
		mFlipEndian = true;
#else
		mFlipEndian = false;
#endif
		break;
	}
	*/
}
//---------------------------------------------------------------------
void 
vl::Serializer::writeFileHeader(void)
{
	uint16_t val = HEADER_STREAM_ID;
	writeShorts(&val, 1);

	writeString(mVersion);

}
//---------------------------------------------------------------------
void 
vl::Serializer::writeChunkHeader(uint16_t id, size_t size)
{
    writeShorts(&id, 1);
	uint32_t uint32size = static_cast<uint32_t>(size);
    writeInts(&uint32size, 1);
}
//---------------------------------------------------------------------
void 
vl::Serializer::writeFloats(const float* const pFloat, size_t count)
{
	if (mFlipEndian)
	{
        float * pFloatToWrite = (float *)malloc(sizeof(float) * count);
        memcpy(pFloatToWrite, pFloat, sizeof(float) * count);
            
        flipToLittleEndian(pFloatToWrite, sizeof(float), count);
        writeData(pFloatToWrite, sizeof(float), count);
            
        free(pFloatToWrite);
	}
	else
	{
        writeData(pFloat, sizeof(float), count);
	}
}
//---------------------------------------------------------------------
void 
vl::Serializer::writeFloats(const double* const pDouble, size_t count)
{
	// Convert to float, then write
	float* tmp = new float[count];
	for (unsigned int i = 0; i < count; ++i)
	{
		tmp[i] = static_cast<float>(pDouble[i]);
	}
	if(mFlipEndian)
	{
        flipToLittleEndian(tmp, sizeof(float), count);
        writeData(tmp, sizeof(float), count);
	}
	else
	{
        writeData(tmp, sizeof(float), count);
	}
	delete [] tmp;
}
//---------------------------------------------------------------------
void 
vl::Serializer::writeShorts(const uint16_t* const pShort, size_t count = 1)
{
	if(mFlipEndian)
	{
        unsigned short * pShortToWrite = (unsigned short *)malloc(sizeof(unsigned short) * count);
        memcpy(pShortToWrite, pShort, sizeof(unsigned short) * count);
            
        flipToLittleEndian(pShortToWrite, sizeof(unsigned short), count);
        writeData(pShortToWrite, sizeof(unsigned short), count);
            
        free(pShortToWrite);
	}
	else
	{
        writeData(pShort, sizeof(unsigned short), count);
	}
}
//---------------------------------------------------------------------
void 
vl::Serializer::writeInts(const uint32_t *const pInt, size_t count = 1)
{
	if(mFlipEndian)
	{
        unsigned int * pIntToWrite = (unsigned int *)malloc(sizeof(unsigned int) * count);
        memcpy(pIntToWrite, pInt, sizeof(unsigned int) * count);
            
        flipToLittleEndian(pIntToWrite, sizeof(unsigned int), count);
        writeData(pIntToWrite, sizeof(unsigned int), count);
            
        free(pIntToWrite);
	}
	else
	{
        writeData(pInt, sizeof(unsigned int), count);
	}
}
//---------------------------------------------------------------------
//---------------------------------------------------------------------
void
vl::Serializer::writeBools(const bool* const pBool, size_t count = 1)
{
//no endian flipping for 1-byte bools
//XXX Nasty Hack to convert to 1-byte bools
#	if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    char * pCharToWrite = (char *)malloc(sizeof(char) * count);
    for(unsigned int i = 0; i < count; i++)
    {
        *(char *)(pCharToWrite + i) = *(bool *)(pBool + i);
    }
        
    writeData(pCharToWrite, sizeof(char), count);
        
    free(pCharToWrite);
#	else
    writeData(pBool, sizeof(bool), count);
#	endif

}
    
//---------------------------------------------------------------------
void 
vl::Serializer::writeData(const void* const buf, size_t size, size_t count)
{
	mStream->write(buf, size * count);
}
//---------------------------------------------------------------------
void 
vl::Serializer::writeString(std::string const &string)
{
	// Old, backwards compatible way - \n terminated
	mStream->write(string.c_str(), string.length());
	// Write terminating newline char
	char terminator = '\n';
	mStream->write(&terminator, 1);
}
//---------------------------------------------------------------------
void
vl::Serializer::readFileHeader(vl::ResourceStream &stream)
{
    unsigned short headerID;
        
    // Read header ID
    readShorts(stream, &headerID, 1);
        
    if (headerID == HEADER_STREAM_ID)
    {
		// Read version
		std::string ver = readString(stream);
		/// @todo add checking that we support the version
		/// for now does not matter as the basic stuff is identical in all the
		/// versions
		/*
		if(ver != mVersion)
		{
			std::string msg = "Invalid file: version incompatible, file reports " 
				+ std::string(ver) + " Serializer is version " + mVersion;
			std::clog << msg << std::endl;
			BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
		}
		*/
    }
    else
    {
		std::string msg("Invalid file: no header");
		std::clog << msg << std::endl;
		BOOST_THROW_EXCEPTION(vl::exception() << vl::desc(msg));
    }
}
//---------------------------------------------------------------------
unsigned short
vl::Serializer::readChunk(vl::ResourceStream &stream)
{
    unsigned short id;
    readShorts(stream, &id, 1);
        
    readInts(stream, &mCurrentstreamLen, 1);
    return id;
}
//---------------------------------------------------------------------
void
vl::Serializer::readBools(vl::ResourceStream &stream, bool* pDest, size_t count)
{
    //XXX Nasty Hack to convert 1 byte bools to 4 byte bools
#	if OGRE_PLATFORM == OGRE_PLATFORM_APPLE || OGRE_PLATFORM == OGRE_PLATFORM_APPLE_IOS
    char * pTemp = (char *)malloc(1*count); // to hold 1-byte bools
    stream.read(pTemp, 1 * count);
    for(unsigned int i = 0; i < count; i++)
        *(bool *)(pDest + i) = *(char *)(pTemp + i);
            
    free (pTemp);
#	else
    stream.read(pDest, sizeof(bool) * count);
#	endif
    //no flipping on 1-byte datatypes
}
//---------------------------------------------------------------------
void 
vl::Serializer::readFloats(vl::ResourceStream &stream, float* pDest, size_t count)
{
    stream.read(pDest, sizeof(float) * count);
    flipFromLittleEndian(pDest, sizeof(float), count);
}
//---------------------------------------------------------------------
void
vl::Serializer::readFloats(vl::ResourceStream &stream, double* pDest, size_t count)
{
	// Read from float, convert to double
	float* tmp = new float[count];
	float* ptmp = tmp;
    stream.read(tmp, sizeof(float) * count);
    flipFromLittleEndian(tmp, sizeof(float), count);
	// Convert to doubles (no cast required)
	while(count--)
	{
		*pDest++ = *ptmp++;
	}
	delete [] tmp;
}
//---------------------------------------------------------------------
void
vl::Serializer::readShorts(vl::ResourceStream &stream, unsigned short* pDest, size_t count)
{
    stream.read(pDest, sizeof(unsigned short) * count);
    flipFromLittleEndian(pDest, sizeof(unsigned short), count);
}
//---------------------------------------------------------------------
void
vl::Serializer::readInts(vl::ResourceStream &stream, unsigned int* pDest, size_t count)
{
    stream.read(pDest, sizeof(unsigned int) * count);
    flipFromLittleEndian(pDest, sizeof(unsigned int), count);
}
//---------------------------------------------------------------------
std::string
vl::Serializer::readString(vl::ResourceStream &stream, size_t numChars)
{
    assert (numChars <= 255);
    char str[255];
    stream.read(str, numChars);
    str[numChars] = '\0';
    return str;
}
//---------------------------------------------------------------------
std::string
vl::Serializer::readString(vl::ResourceStream &stream)
{
    return stream.getLine(false);
}
//---------------------------------------------------------------------
void
vl::Serializer::writeObject(Ogre::Vector3 const &vec)
{
    writeFloats(vec.ptr(), 3);
}
//---------------------------------------------------------------------
void
vl::Serializer::writeObject(Ogre::Quaternion const &q)
{
    float tmp[4] = { q.x, q.y, q.z, q.w };
    writeFloats(tmp, 4);
}
//---------------------------------------------------------------------
void
vl::Serializer::readObject(vl::ResourceStream &stream, Ogre::Vector3 &pDest)
{
    readFloats(stream, pDest.ptr(), 3);
}

void 
vl::Serializer::readObject(vl::ResourceStream &stream, Ogre::Vector2 &pDest)
{
	readFloats(stream, pDest.ptr(), 2);
}
//---------------------------------------------------------------------
void
vl::Serializer::readObject(vl::ResourceStream &stream, Ogre::Quaternion &pDest)
{
    float tmp[4];
    readFloats(stream, tmp, 4);
    pDest.x = tmp[0];
    pDest.y = tmp[1];
    pDest.z = tmp[2];
    pDest.w = tmp[3];
}
//---------------------------------------------------------------------


void
vl::Serializer::flipToLittleEndian(void* pData, size_t size, size_t count)
{
	if(mFlipEndian)
	{
	    flipEndian(pData, size, count);
	}
}
    
void
vl::Serializer::flipFromLittleEndian(void* pData, size_t size, size_t count)
{
	if(mFlipEndian)
	{
	    flipEndian(pData, size, count);
	}
}
    
void 
vl::Serializer::flipEndian(void * pData, size_t size, size_t count)
{
    for(unsigned int index = 0; index < count; index++)
    {
        flipEndian((void *)((size_t)pData + (index * size)), size);
    }
}
    
void 
vl::Serializer::flipEndian(void * pData, size_t size)
{
    char swapByte;
    for(unsigned int byteIndex = 0; byteIndex < size/2; byteIndex++)
    {
        swapByte = *(char *)((size_t)pData + byteIndex);
        *(char *)((size_t)pData + byteIndex) = *(char *)((size_t)pData + size - byteIndex - 1);
        *(char *)((size_t)pData + size - byteIndex - 1) = swapByte;
    }
}
