
#include "mesh_writer.hpp"

int
vl::SubMesh::findFaceIndex(int vertex_index, size_t start_face) const
{
	for(size_t i = start_face; i < _faces.size(); ++i)
	{
		boost::tuple<int, int, int> const &face = _faces.at(i);
		if(vertex_index == face.get<0>() || vertex_index == face.get<1>()
			|| vertex_index == face.get<2>())
		{
			return i;
		}
	}

	return -1;
}

vl::Mesh::~Mesh(void)
{
	for( size_t i = 0; i < _sub_meshes.size(); ++i )
	{
		delete _sub_meshes.at(i);
	}
}

void
vl::Mesh::smoothNormals(Ogre::Radian const &angle)
{
	double cos_angle = std::cos(angle.valueRadians());

	/* calculate the average normal for each vertex */
	for(uint32_t vert_index = 0; vert_index < _vertices.size(); vert_index++)
	{
		std::vector<Ogre::Vector3> face_normals;
		for(size_t sm = 0; sm < _sub_meshes.size(); ++sm)
		{
			if(!_sub_meshes.at(sm)->hasFaceNormals())
			{ continue; }

			int start_face = -1;
			while(true)
			{
				start_face = _sub_meshes.at(sm)->findFaceIndex(vert_index, start_face+1);
				if(start_face < 0 )
				{ break; }
				face_normals.push_back(_sub_meshes.at(sm)->getFaceNormal(start_face));
			}
		}
			
		if(face_normals.empty())
		{
			std::clog << "No such vertex index : " << vert_index << " in the mesh face data." << std::endl;
			continue;
		}
		else if(face_normals.size() == 1)
		{
		//	std::clog << "Single vertex normal for " << vert_index << "." << std::endl;
		}

		Ogre::Vector3 average(0, 0, 0);
		size_t f_n;
		size_t f_n_used_for_average = 0;
		for(f_n = 0; f_n < face_normals.size(); ++f_n)
		{
			if(average != Ogre::Vector3::ZERO)
			{ break; }
			average = face_normals.at(f_n);
			f_n_used_for_average++;
		}
		// Increment so we avoid the reusing the one we already selected for avarage
		f_n++;

		for(; f_n < face_normals.size(); ++f_n)
		{
			/* only average if the dot product of the angle between the two
			facet normals is greater than the cosine of the threshold
			angle -- or, said another way, the angle between the two
			facet normals is less than (or equal to) the threshold angle */
			// @todo this should avarage all the face normals to avarage
			Ogre::Vector3 const &facet_n = face_normals.at(f_n);
			//Ogre::Vector3 const &facet2_n = face_normals.at(f_n+1);

			double dot = average.dotProduct(facet_n);
			if(dot > cos_angle)
			{
				f_n_used_for_average++;
				average += facet_n;
			}
		}

		if(average == Ogre::Vector3::ZERO)
		{ continue; }

		average.normalise();

		if(f_n_used_for_average > 1)
		{
			std::clog << "Used " << f_n_used_for_average << " face normals for creating average normal." << std::endl; 
			std::clog << "Original normal = " << _vertices.at(vert_index).normal << " and Avaraged normal = " << average << std::endl;
		}

		_vertices.at(vert_index).normal = average;
	}
}

vl::SubMesh *
vl::Mesh::createSubMesh(void)
{
	SubMesh *sm = new SubMesh;
	_sub_meshes.push_back(sm);
	return sm;
}

vl::MeshWriter::MeshWriter( void )
	: _logMgr(0)
	, _resourcegm(0)
	, _math(0)
	, _lodMgr(0)
	, _meshMgr(0)
	, _matMgr(0)
	, _meshSerializer(0)
	, _bufferMgr(0)
	, _skelMgr(0)
	, _skeletonSerializer(0)
{
	_logMgr = new Ogre::LogManager();
	_resourcegm = new Ogre::ResourceGroupManager();
	_math = new Ogre::Math();
	_lodMgr = new Ogre::LodStrategyManager();
	_meshMgr = new Ogre::MeshManager();
	_matMgr = new Ogre::MaterialManager();
	_matMgr->initialise();
	_skelMgr = new Ogre::SkeletonManager();

	_meshSerializer = new Ogre::MeshSerializer();
	_skeletonSerializer = new Ogre::SkeletonSerializer();
	_bufferMgr = new Ogre::DefaultHardwareBufferManager(); // needed because we don't have a rendersystem
}

vl::MeshWriter::~MeshWriter( void )
{
	delete _skeletonSerializer;
	delete _meshSerializer;
	delete _skelMgr;
	delete _matMgr;
	delete _meshMgr;
	delete _bufferMgr;
	delete _lodMgr;
	delete _math;
	delete _resourcegm;
	delete _logMgr;
}

vl::Mesh *
vl::MeshWriter::createMesh( void )
{
	return new Mesh;
}

void 
vl::MeshWriter::writeMesh(vl::Mesh *mesh, std::string const &filename)
{
	Ogre::MeshPtr newMesh = Ogre::MeshManager::getSingleton().createManual("conversion", 
            Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
	// TODO add the mesh to an array, because otherwise it can get destroyed
	// also we want to have the refptr at hand.

	// Create shared VertexData for ease of use
	newMesh->sharedVertexData = new Ogre::VertexData();

	// TODO copy from mesh to newMesh
	writeGeometry(mesh, newMesh->sharedVertexData, newMesh.get());
	writeSubMeshes(mesh, newMesh.get());

	Ogre::MeshSerializer ser;
	ser.exportMesh( newMesh.get(), filename );

	delete mesh;
}

/// The MeshWriter should be enough for creation and saving meshes
/// Now just export MeshWriter, Ogre::Mesh, Ogre::SubMesh to python module.

void 
vl::MeshWriter::writeGeometry(vl::Mesh *mesh, Ogre::VertexData *vertexData, Ogre::Mesh *og_mesh)
{
	assert(mesh);
	assert(vertexData);

	unsigned char *pVert;
	float *pFloat;
	Ogre::ARGB *pCol;

    // Skip empty 
    if( mesh->getNumVertices() == 0 ) return;

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
//	std::cout << "Writing " << mesh->getNumVertices() << " vertices." << std::endl;

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
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_FLOAT3", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_FLOAT4:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_FLOAT4", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_SHORT1:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_SHORT1", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_SHORT2:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_SHORT2", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_SHORT3:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_SHORT3", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_SHORT4:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_SHORT4", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_UBYTE4:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_UBYTE4", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_COLOUR:
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_COLOUR", "MeshWriter::writeGeometry");
					break;

				case Ogre::VET_COLOUR_ARGB:
				case Ogre::VET_COLOUR_ABGR: 
					OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "VET_COLOUR_ABGR", "MeshWriter::writeGeometry");
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
    Ogre::AxisAlignedBox const &currBox = og_mesh->getBounds();
    Ogre::Real currRadius = og_mesh->getBoundingSphereRadius();
    if (currBox.isNull())
    {
		// do not pad the bounding box
        og_mesh->_setBounds(Ogre::AxisAlignedBox(min, max), false);
        og_mesh->_setBoundingSphereRadius(Ogre::Math::Sqrt(maxSquaredRadius));
    }
    else
    {
        Ogre::AxisAlignedBox newBox(min, max);
        newBox.merge(currBox);
		// do not pad the bounding box
        og_mesh->_setBounds(newBox, false);
        og_mesh->_setBoundingSphereRadius(std::max(Ogre::Math::Sqrt(maxSquaredRadius), currRadius));
    }
}

void 
vl::MeshWriter::writeSubMeshes(vl::Mesh *mesh, Ogre::Mesh *og_mesh)
{
	for(size_t i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
		// All children should be submeshes 
		Ogre::SubMesh *og_sm = og_mesh->createSubMesh();

		vl::SubMesh *sm = mesh->getSubMesh(i); 
		writeSubMesh(sm, og_sm);

		og_mesh->nameSubMesh(sm->getName(), i);
    }
}

void 
vl::MeshWriter::writeSubMesh(vl::SubMesh *mesh, Ogre::SubMesh *og_sm)
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
			OGRE_EXCEPT(Ogre::Exception::ERR_NOT_IMPLEMENTED, "operationType not implemented", 
				__FUNCTION__);
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
