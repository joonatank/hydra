
#include "mesh.hpp"

#include "base/exceptions.hpp"

#include <OGRE/OgreMesh.h>
#include <OGRE/OgreSubMesh.h>
#include <OGRE/OgreDefaultHardwareBufferManager.h>
#include <OGRE/OgreMeshManager.h>

/// ------------------------------- Global -----------------------------------

Ogre::MeshPtr
vl::create_ogre_mesh(std::string const &name, vl::MeshRefPtr mesh)
{
	std::clog << "vl::create_ogre_mesh : " << name << std::endl;
	/*
Ogre::Mesh* mMesh = Ogre::MeshManager::getSingleton().createManual(yourMeshName, "General");
// @todo add support for named submeshes
Ogre::SubMesh* mSubMesh = mMesh->createSubMesh(yourSubMeshName);

// We first create a VertexData
 Ogre::VertexData* data = new Ogre::VertexData();
 // Then, we link it to our Mesh/SubMesh :
 #ifdef SHARED_GEOMETRY
     mMesh->sharedVertexData = data;
 #else
     mSubMesh->useSharedVertices = false; // This value is 'true' by default
     mSubMesh->vertexData = data;
 #endif
 // We have to provide the number of verteices we'll put into this Mesh/SubMesh
 data->vertexCount = iVertexCount;
 // Then we can create our VertexDeclaration
 Ogre::VertexDeclaration* decl = data->vertexDeclaration;

 size_t offset = 0;
 decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
 offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
 decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);

Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
     decl->getVertexSize(0),                     // This value is the size of a vertex in memory
     iVertexNbr,                                 // The number of vertices you'll put into this buffer
     Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY // Properties
 );

vbuf->writeData(0, vbuf->getSizeInBytes(), array, true);

array[0] = vertices[1].x
 array[1] = vertices[1].y
 array[2] = vertices[1].z
 array[3] = vertices[1].normal.x
 array[4] = vertices[1].normal.y
 array[5] = vertices[1].normal.z
 
 array[6] = vertices[2].x

 // "data" is the Ogre::VertexData* we created before
 Ogre::VertexBufferBinding* bind = data->vertexBufferBinding;
 bind->setBinding(0, vbuf);
 */
	Ogre::MeshPtr og_mesh = Ogre::MeshManager::getSingleton().createManual(name, 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

	// Create shared VertexData for ease of use
	og_mesh->sharedVertexData = new Ogre::VertexData();

	// TODO copy from mesh to newMesh
	convert_ogre_geometry(mesh.get(), og_mesh->sharedVertexData, og_mesh.get());
	convert_ogre_submeshes(mesh.get(), og_mesh.get());

	return og_mesh;
}

void 
vl::convert_ogre_geometry(vl::Mesh *mesh, Ogre::VertexData *vertexData, Ogre::Mesh *og_mesh)
{
	std::clog << "vl::convert_ogre_geometry : " << og_mesh->getName() << std::endl;
	
	unsigned char *pVert = 0;
    float *pFloat = 0;
    Ogre::uint16 *pShort = 0;
    Ogre::uint8 *pChar = 0;
    Ogre::ARGB *pCol = 0;

    // Skip empty 
    if( mesh->getNumVertices() == 0 ) 
	{
		std::cout << "Empty mesh " << og_mesh->getName() << std::endl;
		return;
	}

    Ogre::VertexDeclaration *decl = vertexData->vertexDeclaration;
    Ogre::VertexBufferBinding *bind = vertexData->vertexBufferBinding;
    unsigned short bufCount = 0;
    unsigned short totalTexCoords = 0; // across all buffers

    // Information for calculating bounds
    Ogre::Vector3 min = Ogre::Vector3::ZERO;
	Ogre::Vector3 max = Ogre::Vector3::UNIT_SCALE;
    Ogre::Real maxSquaredRadius = -1;
    bool first = true;

    // Assume single vertexbuffer

	size_t offset = 0;

	// Add element
	decl->addElement(bufCount, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

	// Add element
	decl->addElement(bufCount, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);
	offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);

	/*	No tangents
	Ogre::VertexElementType tangentType = Ogre::VET_FLOAT3;
	attrib = vbElem->Attribute("tangent_dimensions");
	if (attrib)
	{
		unsigned int dims = StringConverter::parseUnsignedInt(attrib);
		if (dims == 4)
			tangentType = VET_FLOAT4;
	}

	// Add element
	decl->addElement(bufCount, offset, tangentType, VES_TANGENT);
	offset += VertexElement::getTypeSize(tangentType);
	*/

	/*	No binormals
	attrib = vbElem->Attribute("binormals");
	if (attrib && StringConverter::parseBool(attrib))
	{
		// Add element
		decl->addElement(bufCount, offset, VET_FLOAT3, VES_BINORMAL);
		offset += VertexElement::getTypeSize(VET_FLOAT3);
	}
	*/

	Ogre::VertexElementType colourElementType = Ogre::VET_FLOAT2;

    // Process colour diffuse
//	decl->addElement(bufCount, offset, colourElementType, Ogre::VES_DIFFUSE);
//	offset += Ogre::VertexElement::getTypeSize(colourElementType);

	// Process colour specular
//	decl->addElement(bufCount, offset, colourElementType, Ogre::VES_SPECULAR);
//	offset += Ogre::VertexElement::getTypeSize(colourElementType);

	// Process texture coords
    // TODO add support for multiple texture coordinates
	unsigned short numTexCoords = 1;
    for (unsigned short tx = 0; tx < numTexCoords; ++tx)
	{
		// NB set is local to this buffer, but will be translated into a 
		// global set number across all vertex buffers
		// Default
		Ogre::VertexElementType vtype = Ogre::VET_FLOAT2; 

		// Add element
		decl->addElement(bufCount, offset, vtype,
			Ogre::VES_TEXTURE_COORDINATES, totalTexCoords++);
		offset += Ogre::VertexElement::getTypeSize(vtype);
	}

	vertexData->vertexCount = mesh->getNumVertices();

	// Now create the vertex buffer
	Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().
		createVertexBuffer(offset, vertexData->vertexCount, 
			Ogre::HardwareBuffer::HBU_STATIC_WRITE_ONLY, false);
	// Bind it
	bind->setBinding(bufCount, vbuf);
	// Lock it
	pVert = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

	// Get the element list for this buffer alone
	Ogre::VertexDeclaration::VertexElementList elems = decl->findElementsBySource(bufCount);

	// Now the buffer is set up, parse all the vertices
	for(size_t i = 0; i < mesh->getNumVertices(); ++i)
	{
		// Now parse the elements, ensure they are all matched
		Ogre::VertexDeclaration::VertexElementList::const_iterator ielem, ielemend;

		ielemend = elems.end();
		for (ielem = elems.begin(); ielem != ielemend; ++ielem)
		{
			const Ogre::VertexElement& elem = *ielem;
			// Find child for this element
			switch(elem.getSemantic())
			{
				case Ogre::VES_POSITION:
				{
					elem.baseVertexPointerToElement(pVert, &pFloat);

					Ogre::Vector3 const &pos = mesh->getVertex(i).position;
					*pFloat++ = pos.x;
					*pFloat++ = pos.y;
					*pFloat++ = pos.z;
				
					if(first)
					{
						min = max = pos;
						maxSquaredRadius = pos.squaredLength();
						first = false;
					}
					else
					{
						min.makeFloor(pos);
						max.makeCeil(pos);
						maxSquaredRadius = std::max(pos.squaredLength(), maxSquaredRadius);
					}
				}
				break;

			case Ogre::VES_NORMAL:
				elem.baseVertexPointerToElement(pVert, &pFloat);

				*pFloat++ = mesh->getVertex(i).normal.x;
				*pFloat++ = mesh->getVertex(i).normal.y;
				*pFloat++ = mesh->getVertex(i).normal.z;
				break;

			case Ogre::VES_TANGENT:
				/*	Tangents are not supported
				elem.baseVertexPointerToElement(pVert, &pFloat);

				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("x"));
				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("y"));
				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("z"));
				if (elem.getType() == VET_FLOAT4)
				{
					*pFloat++ = StringConverter::parseReal(
						xmlElem->Attribute("w"));
				}
				*/
				break;
			case Ogre::VES_BINORMAL:
				/*	Binormals are not supported
				elem.baseVertexPointerToElement(pVert, &pFloat);

				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("x"));
				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("y"));
				*pFloat++ = StringConverter::parseReal(xmlElem->Attribute("z"));
				*/
				break;
			case Ogre::VES_DIFFUSE:
//				std::cout << "Writing Vertex diffuse data." << std::endl;
				elem.baseVertexPointerToElement(pVert, &pCol);
				{
					Ogre::ColourValue cv = mesh->getVertex(i).diffuse;
					*pCol++ = Ogre::VertexElement::convertColourValue(cv, colourElementType);
				}
				break;
			case Ogre::VES_SPECULAR:
//				std::cout << "Writing Vertex specular data." << std::endl;
				elem.baseVertexPointerToElement(pVert, &pCol);
				{
					Ogre::ColourValue cv = mesh->getVertex(i).specular;
					*pCol++ = Ogre::VertexElement::convertColourValue(cv, colourElementType);
				}
				break;
			case Ogre::VES_TEXTURE_COORDINATES:
//				std::cout << "Writing Vertex texture coordinates." << std::endl;
				switch (elem.getType()) 
				{
				case Ogre::VET_FLOAT1:
					elem.baseVertexPointerToElement(pVert, &pFloat);
					*pFloat++ = mesh->getVertex(i).uv.x;
					break;

				case Ogre::VET_FLOAT2:
					elem.baseVertexPointerToElement(pVert, &pFloat);
					*pFloat++ = mesh->getVertex(i).uv.x;
					*pFloat++ = mesh->getVertex(i).uv.y;
					break;

				case Ogre::VET_FLOAT3:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_FLOAT3"));
					break;

				case Ogre::VET_FLOAT4:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_FLOAT4"));
					break;

				case Ogre::VET_SHORT1:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT1"));
					break;

				case Ogre::VET_SHORT2:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT2"));
					break;

				case Ogre::VET_SHORT3:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT3"));
					break;

				case Ogre::VET_SHORT4:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_SHORT4"));
					break;

				case Ogre::VET_UBYTE4:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_UBYTE4"));
					break;

				case Ogre::VET_COLOUR:
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_COLOUR"));
					break;

				case Ogre::VET_COLOUR_ARGB:
				case Ogre::VET_COLOUR_ABGR: 
					BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("VET_COLOUR_ABGR"));
					break;
				}

				break;
			default:
				break;
			}
		}	// semantic
		pVert += vbuf->getVertexSize();
	}	// vertices

	bufCount++;
    vbuf->unlock();
	// Vertexbuffer done

    // Set bounds
	/// @todo this should be separate function in vl::Mesh
	/// remove it from here when possible
    Ogre::AxisAlignedBox const &currBox = og_mesh->getBounds();
    Ogre::Real currRadius = og_mesh->getBoundingSphereRadius();
    if (currBox.isNull())
    {
		// do not pad the bounding box
        og_mesh->_setBounds(Ogre::AxisAlignedBox(min, max), false);
        og_mesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(maxSquaredRadius));
		std::clog << "Setting new bounding box for " << og_mesh->getName() 
			<< " box = " << og_mesh->getBounds() << std::endl;
    }
    else
    {
        Ogre::AxisAlignedBox newBox(min, max);
        newBox.merge(currBox);
		// do not pad the bounding box
        og_mesh->_setBounds(newBox, false);
        og_mesh->_setBoundingSphereRadius(std::max(Ogre::Math::Sqrt(maxSquaredRadius), currRadius));
		std::clog << "Appending to bounding box for " << og_mesh->getName()
			<< " box = " << og_mesh->getBounds() << std::endl;
    }
}

void 
vl::convert_ogre_submeshes(vl::Mesh *mesh, Ogre::Mesh *og_mesh)
{
	std::clog << "vl::convert_ogre_submeshes : " << og_mesh->getName() << std::endl;
	for(size_t i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
		// All children should be submeshes 
		Ogre::SubMesh *og_sm = og_mesh->createSubMesh();

		vl::SubMesh *sm = mesh->getSubMesh(i); 
		convert_ogre_submesh(sm, og_sm);

		og_mesh->nameSubMesh(sm->getName(), i);
    }
}

void 
vl::convert_ogre_submesh(vl::SubMesh *mesh, Ogre::SubMesh *og_sm)
{
	assert(mesh);
	assert(og_sm);

	og_sm->setMaterialName(mesh->getMaterial());

	if( mesh->getNumFaces() > 0 )
	{
		// Faces
		switch(og_sm->operationType)
		{
		case Ogre::RenderOperation::OT_TRIANGLE_LIST:
			// tri list
			og_sm->indexData->indexCount = mesh->getNumFaces() * 3;

			break;
		case Ogre::RenderOperation::OT_LINE_LIST:
			og_sm->indexData->indexCount = mesh->getNumFaces() * 2;

			break;
		case Ogre::RenderOperation::OT_TRIANGLE_FAN:
		case Ogre::RenderOperation::OT_TRIANGLE_STRIP:
			// triangle fan or triangle strip
			og_sm->indexData->indexCount = mesh->getNumFaces() + 2;

			break;
		default:
			BOOST_THROW_EXCEPTION(vl::not_implemented() << vl::desc("operationType not implemented"));
		}

		// Allocate space
		Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().
			createIndexBuffer(Ogre::HardwareIndexBuffer::IT_32BIT, 
				og_sm->indexData->indexCount,
				Ogre::HardwareBuffer::HBU_DYNAMIC,
				false);

		og_sm->indexData->indexBuffer = ibuf;
		unsigned int *pInt = 0;
		unsigned short *pShort = 0;
		
		pInt = static_cast<unsigned int*>(ibuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));

		bool firstTri = true;
		for(size_t i = 0; i < mesh->getNumFaces(); ++i)
		{
			*pInt++ = mesh->getFace(i).get<0>();
			if(og_sm->operationType == Ogre::RenderOperation::OT_LINE_LIST)
			{
				*pInt++ = mesh->getFace(i).get<1>();
			}
			// only need all 3 vertices if it's a trilist or first tri
			else if(og_sm->operationType == Ogre::RenderOperation::OT_TRIANGLE_LIST || firstTri)
			{
				*pInt++ = mesh->getFace(i).get<1>();
				*pInt++ = mesh->getFace(i).get<2>();
			}
			firstTri = false;
		}
		ibuf->unlock();
	}


	// Geometry
	// Independent geometry not supported
	/*
	if(!og_sm->useSharedVertices)
	{
		TiXmlElement* geomNode = smElem->FirstChildElement("geometry");
		if (geomNode)
		{
			og_sm->vertexData = new Ogre::VertexData();
			readGeometry(geomNode, og_sm->vertexData);
		}
	}
	*/

	// texture aliases, NOT supported
	/*
	TiXmlElement* textureAliasesNode = smElem->FirstChildElement("textures");
	if(textureAliasesNode)
	{ readTextureAliases(textureAliasesNode, sm); }
	*/

	// Bone assignments, NOT supported
	/*
	TiXmlElement* boneAssigns = smElem->FirstChildElement("boneassignments");
	if(boneAssigns)
	{ readBoneAssignments(boneAssigns, sm); }
	*/
}


std::ostream &
vl::operator<<(std::ostream &os, vl::Vertex const &v)
{
	os << "Vertex : position " << v.position << " : normal " << v.normal;
	return os;
}

std::ostream &
vl::operator<<( std::ostream &os, vl::Mesh const &m )
{
	os << "Mesh : vertices = ";
	for( size_t i = 0; i < m.getNumVertices(); ++i )
	{
		os << m.getVertex(i).position << ", ";
	}
	os << std::endl;
	return os;
}

std::ostream &
vl::operator<<( std::ostream &os, vl::SubMesh const &m )
{
	return os;
}

/// -------------------------------- Mesh ------------------------------------
vl::Mesh::~Mesh(void)
{
	for( size_t i = 0; i < _sub_meshes.size(); ++i )
	{
		delete _sub_meshes.at(i);
	}
}

vl::SubMesh *
vl::Mesh::createSubMesh(void)
{
	SubMesh *sm = new SubMesh;
	_sub_meshes.push_back(sm);
	return sm;
}
