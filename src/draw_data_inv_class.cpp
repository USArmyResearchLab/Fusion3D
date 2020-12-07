#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
draw_data_inv_class::draw_data_inv_class()
	:draw_data_class()
{
	nSymbolsMade = 0;
	outputSymbolWidth = 45;
	symbolBase = NULL;

	fillDrawStyle = new SoDrawStyle;
	fillDrawStyle->style = SoDrawStyle::FILLED;
	fillDrawStyle->ref();

	solidLineDrawStyle = new SoDrawStyle;
	solidLineDrawStyle->style = SoDrawStyle::LINES;
	solidLineDrawStyle->lineWidth.setValue(2);
	solidLineDrawStyle->ref();

	dashLineDrawStyle = new SoDrawStyle;
	dashLineDrawStyle->style = SoDrawStyle::LINES;
	dashLineDrawStyle->lineWidth.setValue(2);
	dashLineDrawStyle->linePattern = 0x00ff;
	dashLineDrawStyle->ref();

	ptsDrawStyle = new SoDrawStyle;
	ptsDrawStyle->style = SoDrawStyle::POINTS;
	ptsDrawStyle->pointSize.setValue(6);
	ptsDrawStyle->ref();
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
draw_data_inv_class::~draw_data_inv_class()
{
}

// **********************************************
/// Set output symbol width in pixels.
// **********************************************
int draw_data_inv_class::set_symbol_width(int widthInPixels)
{
	outputSymbolWidth = widthInPixels;
	return (1);
}

// **********************************************
/// Set the base SoSeparator for all symbols.
/// Also define other OIV.
// **********************************************
int draw_data_inv_class::set_symbol_base(SoSeparator** symbolBaseIn)
{
	symbolBase = symbolBaseIn;
	return (1);
}

// **********************************************
/// Draw a single point with accompanying symbol or name.
/// By default, if a symbol exists for the point draw the symbol only;
/// If no symbol exists but a name does then draw the name only.
/// If neither symbol or name exists, draw the point itself.
/// @param	ipt					Input point number
/// @param	externalColorFlag	1 iff use color from call sequence, 0 if use internal color
/// @param	pointSize			input float variable point size (0 for default size, which may be set outside this class)
/// @param	noShowNameFlag		input int 0=default, 1=never show names
/// @param	color				Input line color
/// @param	inSep				Input base of tree for the line
// **********************************************
int draw_data_inv_class::draw_point(int ipt, int externalColorFlag, int noShowNameFlag, SoBaseColor *color, SoSeparator* gSep, SoSeparator* tSep, SoSeparator* sSep)
{
	int symbolFlag = 0, nameFlag = 0;
	float xpt, ypt, zpt;

	// **********************
	// Get location
	// **********************
	xpt = ppx[ipt] + xOffset;
	ypt = ppy[ipt] + yOffset;
	calc_elev(xpt, ypt, ppz[ipt], ppAltMode[ipt], zpt);

	// **********************
	// Add symbol
	// **********************
	if (ptSymbolFlag && ppISymbol[ipt] >= 0) {
		SoSeparator *ssSep = new SoSeparator;
		sSep->addChild(ssSep);
		SoTranslation 	*atrans = new SoTranslation;
		atrans->translation.setValue(xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
		ssSep->addChild(atrans);
		ssSep->addChild(symbolBase[ppISymbol[ipt]]);
		symbolFlag = 1;
	}

	// **********************
	// Add name
	// **********************
	if (!symbolFlag && noShowNameFlag==0 && entityNameFlag && ppname[ipt].compare("") != 0) {
		SoSeparator *tsSep = new SoSeparator;
		tSep->addChild(tsSep);
		if (externalColorFlag) tsSep->addChild(color);

		SoTranslation *translateToPlace = new SoTranslation;
		translateToPlace->translation.setValue(xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
		tsSep->addChild(translateToPlace);

		SoText2* nameText = new SoText2;
		if (ptDrawPtFlag > 1 || (ptDrawPtFlag == 0 && drawPtFlag[ipt])) {	// If drawing point, insert blank char before name so name not directly on point
			string ttname = " ";
			ttname.append(ppname[ipt]);
			nameText->string = ttname.c_str();
		}
		else {
			nameText->string = ppname[ipt].c_str();
		}
		tsSep->addChild(nameText);
		nameFlag = 1;
	}

	// **********************
	// Draw point if requested 
	// **********************
	if (ptDrawPtFlag==4 || (ptDrawPtFlag==3 && !symbolFlag) || (ptDrawPtFlag==2 && !symbolFlag && !nameFlag)  || (ptDrawPtFlag==0 && drawPtFlag[ipt])) {
		SoSeparator *gsSep = new SoSeparator;
		gSep->addChild(gsSep);
		SoCoordinate3 *oneCoord = new SoCoordinate3;
		oneCoord->point.set1Value(0, xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);// Offset up so track shows
		SoPointSet* onePoint = new SoPointSet;
		onePoint->numPoints.setValue(1);

		if (externalColorFlag) gsSep->addChild(color);
		gsSep->addChild(oneCoord);
		gsSep->addChild(onePoint);
	}
	return (1);
}

// **********************************************
/// Draw the mover associated with the given line.
/// @param	iline				Input line number
/// @param	time				Input time at which to place mover
/// @param	inSep				Input base of tree for the line
// **********************************************
int draw_data_inv_class::draw_line_mover(int iline, float time, SoSeparator* inSep)
{
	int index;
	float xpt, ypt, zpt, findex, del;
	if (plTimeIndex[iline] < 0) return(0);									// No time info for this line
	if (pltimeISymbol[iline] < 0 && pltimeISymbol[iline] < 0) return(0);	// No mover symbol/model defined for this line

	// Find location of mover
	interval_calc->calc_index_external(time, pltime[iline], plTimeIndex[iline], findex);
	index = (int)findex;
	del = findex - index;
	if (index >= plx[iline].size() - 1) {
		xpt = plx[iline][plx[iline].size() - 1] + xOffset;
		ypt = ply[iline][ply[iline].size() - 1] + yOffset;
		zpt = plz[iline][plz[iline].size() - 1];
	}
	else {
		xpt = plx[iline][index] + del * (plx[iline][index + 1] - plx[iline][index]) + xOffset;
		ypt = ply[iline][index] + del * (ply[iline][index + 1] - ply[iline][index]) + yOffset;
		zpt = plz[iline][index] + del * (plz[iline][index + 1] - plz[iline][index]);
	}

	// Adjust for altitude modes
	calc_elev(xpt, ypt, zpt, plAltMode[iline], zpt);

	// Add mover to tree
	SoSeparator *placeSep = new SoSeparator;
	inSep->addChild(placeSep);
	SoTranslation *translateToPlace = new SoTranslation;
	translateToPlace->translation.setValue(xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
	placeSep->addChild(translateToPlace);
	placeSep->addChild(symbolBase[pltimeISymbol[pltimeISymbol[iline]]]);
	return (1);
}

// **********************************************
/// Draw a line.
/// @param	iline				Input line number
/// @param	externalColorFlag	1 iff use color from call sequence, 0 if use internal color
/// @param	color				Input line color
/// @param	inSep				Input base of tree for the line
// **********************************************
int draw_data_inv_class::draw_line(int iline, int externalColorFlag, int showPointsFlag, SoBaseColor *color, SoSeparator* inSep)
{
	float xpt, ypt, zpt;
	SoPointSet* showPoints;
	float ref_utm_elevation = gps_calc->get_ref_elevation();
	int npt = plx[iline].size();

	SoLineSet*		lPoint = new SoLineSet;
	SoCoordinate3*	lCoord = new SoCoordinate3;
	if (showPointsFlag) {
		showPoints = new SoPointSet;
		showPoints->numPoints.setValue(npt);
	}


	for (int ipt = 0; ipt < npt; ipt++) {
		xpt = plx[iline][ipt] + xOffset;
		ypt = ply[iline][ipt] + yOffset;
		zpt = plz[iline][ipt];
		calc_elev(xpt, ypt, zpt, plAltMode[iline], zpt);
		lCoord->point.set1Value(ipt, xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
	}
	lPoint->numVertices.setValue(npt);

	if (lineDashFlag && pldash[iline] > 0) {
		inSep->addChild(dashLineDrawStyle);
	}
	else {
		inSep->addChild(solidLineDrawStyle);
	}
	if (externalColorFlag) inSep->addChild(color);
	inSep->addChild(lCoord);
	inSep->addChild(lPoint);
	if (showPointsFlag) inSep->addChild(ptsDrawStyle);
	if (showPointsFlag) inSep->addChild(showPoints);
	return (1);
}

// **********************************************
/// Draw a polygon.
/// @param	ipol				Input polygon number
/// @param	externalColorFlag	1 iff use color from call sequence, 0 if use internal color
/// @param	color				Input line color
/// @param	inSep				Input base of tree for the line
// **********************************************
int draw_data_inv_class::draw_polygon(int ipol, int externalColorFlag, int showPointsFlag, SoBaseColor *color, SoSeparator* inSep)
{
	float xpt, ypt, zpt;
	float ref_utm_elevation = gps_calc->get_ref_elevation();

	SoLineSet*		lPoint = new SoLineSet;
	SoCoordinate3*	lCoord = new SoCoordinate3;

	int npt = pbx[ipol].size();
	for (int ipt = 0; ipt < npt; ipt++) {
		xpt = pbx[ipol][ipt] + xOffset;
		ypt = pby[ipol][ipt] + yOffset;
		zpt = pbz[ipol][ipt];
		calc_elev(xpt, ypt, zpt, pbAltMode[ipol], zpt);
		lCoord->point.set1Value(ipt, xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
	}
	lPoint->numVertices.setValue(npt);

	inSep->addChild(solidLineDrawStyle);
	if (externalColorFlag) inSep->addChild(color);
	inSep->addChild(lCoord);
	inSep->addChild(lPoint);
	return (1);
}

// **********************************************
/// Draw all objects in memory.
/// @param	color				Input line color
/// @param	inSep				Input base of tree for the line
// **********************************************
int draw_data_inv_class::draw_all(SoBaseColor *color, SoSeparator* inSep)
{
	int iline, ipol, npt, ipt, drawType=0;		// 0=undefined, 1=points, 2=solid lines, 3=dashed lines, 4=filled polygons
	float xpt, ypt, zpt;
	float ref_utm_elevation = gps_calc->get_ref_elevation();

	// **********************************
	// Special processing for CAD formats handled by Assimp library
	// **********************************
	if (assimpFilename.compare("") != 0) {
		int ireturn = draw_assimp(assimpFilename, inSep);
		return(ireturn);
	}

	// **********************************
	// Other formats
	// **********************************
	// Need to adjust lighting -- make 2-sided -- since default has pols black
	if (pbx.size() > 0) {		// Only if pols present
		SoShapeHints *shapeHints = new SoShapeHints;					// Need this to turn on two-sided lighting (default seems to be off, so may look black)
		shapeHints->faceType.setValue(SoShapeHints::UNKNOWN_FACE_TYPE);
		//shapeHints->shapeType.setValue(SoShapeHints::SOLID);
		shapeHints->shapeType.setValue(SoShapeHints::UNKNOWN_SHAPE_TYPE);
		shapeHints->vertexOrdering.setValue(SoShapeHints::CLOCKWISE);
		inSep->addChild(shapeHints);
	}

	inSep->addChild(color);		

	// **********************************
	// Points
	// **********************************
	for (ipt = 0; ipt < ppx.size(); ipt++) {
		draw_point(ipt, 0, 0, NULL, inSep, inSep, inSep);
	}

	// **********************************
	// Lines
	// **********************************
	for (iline = 0; iline < plx.size(); iline++) {
		if (drawType != 2 && pldash[iline] == 0) {
		inSep->addChild(solidLineDrawStyle);
		drawType = 2;
		}
		else if (drawType != 3 && pldash[iline] == 1) {
		inSep->addChild(dashLineDrawStyle);
		drawType = 3;
		}

		SoLineSet*		lPoint = new SoLineSet;
		SoCoordinate3*	lCoord = new SoCoordinate3;
		npt = plx[iline].size();
		for (ipt = 0; ipt < npt; ipt++) {
			xpt = plx[iline][ipt] + xOffset;
			ypt = ply[iline][ipt] + yOffset;
			zpt = plz[iline][ipt];
			calc_elev(xpt, ypt, zpt, plAltMode[iline], zpt);
			lCoord->point.set1Value(ipt, xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
		}
		lPoint->numVertices.setValue(npt);

		inSep->addChild(lCoord);
		inSep->addChild(lPoint);
	}

	// **********************************
	// Polygons
	// **********************************
	for (ipol = 0; ipol < pbx.size(); ipol++) {
		SoCoordinate3*	lCoord = new SoCoordinate3;
		int npt = pbx[ipol].size();
		for (int ipt = 0; ipt < npt; ipt++) {
			xpt = pbx[ipol][ipt] + xOffset;
			ypt = pby[ipol][ipt] + yOffset;
			zpt = pbz[ipol][ipt];
			calc_elev(xpt, ypt, zpt, pbAltMode[ipol], zpt);
			lCoord->point.set1Value(ipt, xpt, ypt, zpt - gps_calc->get_ref_elevation() + zOffset);
		}
		inSep->addChild(lCoord);

		// Filled
		if ((fillUnkPolyFlag && pbfill[ipol] == -1) || pbfill[ipol] == 1) {
			if (drawType != 4) {
				inSep->addChild(fillDrawStyle);
				drawType = 4;
			}

			SoFaceSet *face = new SoFaceSet;
			face->numVertices.set1Value(0, npt);
			inSep->addChild(face);
		}

		// Wireframe
		else {
			if (drawType != 2) {
				inSep->addChild(solidLineDrawStyle);
				drawType = 2;
			}

			SoLineSet*		plines = new SoLineSet;
			plines->numVertices.setValue(npt);
			inSep->addChild(plines);
		}
	}
	return (1);
}

// **********************************************
/// Update symbols by making subtrees for any symbol icons that have not yet been made.
/// .
// **********************************************
int draw_data_inv_class::update_symbols()
{
	for (int iSym = nSymbolsMade; iSym < symbolName.size(); iSym++) {
		make_symbol(iSym);
	}
	nSymbolsMade = symbolName.size();
	return (1);
}

// **********************************************
/// Private -- Read the symbol image file and build the subtree that contains the image.
/// Define a new separator to put the subtree under.
// **********************************************
int draw_data_inv_class::make_symbol(int iSymbolMake)
{
	image_2d_class *image_2d;

	symbolBase[iSymbolMake] = new SoSeparator;
	symbolBase[iSymbolMake]->ref();

	if (symbolName[iSymbolMake].find(".bmp") != string::npos) {
		image_2d = new image_gdal_class();
	}
	else if (symbolName[iSymbolMake].find(".png") != string::npos) {
		image_2d = new image_gdal_class();
	}

	if (!image_2d->read_file(symbolName[iSymbolMake])) {
		cerr << "Cant read symbol icon file " << symbolName[iSymbolMake] << endl;			// Just print out since there may be many
		return(0);
	}

	int ncols = image_2d->get_n_cols();
	int nrows = image_2d->get_n_rows();
	int nbands = image_2d->get_nbands();
	unsigned char * data = image_2d->get_data();

	SoImage *img = new SoImage;
	img->vertAlignment = SoImage::CENTER;
	img->horAlignment = SoImage::CENTER;
	//img->width = ncols;
	//img->height = nrows;
	img->width = outputSymbolWidth;
	img->height = int(float(outputSymbolWidth) * float(nrows) / float (ncols));
	if (nbands > 4) {
		warning(1, "draw_data_inv_class not implemented for this number of bands");
		return(0);
	}
	img->image.setValue(SbVec2s(ncols, nrows), nbands, data);

	symbolBase[iSymbolMake]->addChild(img);
	delete image_2d;
	return (1);
}

// *************************************************************
/// Clear all saved data -- flags are not modified and symbol images are not modified.
/// Most data are stored internally and copied into graphics trees -- so can be deleted.
/// Symbols, however, are stored externally so may disappear if deleted.
// *************************************************************
int draw_data_inv_class::clear()
{
	// Dont modify symbol 
	//for (int i = 0; i < nSymbolsMade; i++) {
	//	symbolBase[i]->removeAllChildren();
	//	symbolBase[i]->unref();
	//}
	//nSymbolsMade = 0;

	// Call parent class to clear its data arrays
	draw_data_class::clear();
	return(1);
}

// *************************************************************
/// Clear all saved data -- flags are not modified and symbol images are not modified.
/// Most data are stored internally and copied into graphics trees -- so can be deleted.
/// Symbols, however, are stored externally so may disappear if deleted.
/// @param filename		Input name of (base) file
/// @param inSep		SoSeparator into which the OpenInventor output tree is inserted
/// @return				0 if Assimp cant read, 1 if successful read
// *************************************************************
int draw_data_inv_class::draw_assimp(string filename, SoSeparator* inSep)
{
#if defined(LIBS_ASSIMP) 
	int i, iMesh, nMesh, iVert, nVert, iFace, nFace, nIndex, iIndex, iIndexOut, iMaterial, nMaterial, iTex;
	unsigned int uvindex;
	string texNameString, path, name, suffix;
	vector <int> texFileIndexPerMaterial;
	int *texUseIndex;
	unsigned int* aiIndex;
	aiMesh* mesh;
	aiVector3D *verts, *norms;
	//aiVector3D(*texCoords)[8];
	aiVector3D **meshTexCoords;
	aiFace* faces;
	aiMaterial *material;
	aiTextureMapping aiTexMap;
	aiString aiStr;
	aiReturn aiStatus;
	aiColor4D aColor;
	i = filename.find_last_of("/\\");
	path = filename.substr(0, i+1);

	// *********************************
	// Assimp import scene -- scene automatically deleted when importer goes out of scope
	// *********************************
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filename,
		aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);
	if (!scene) {
		const char* err = importer.GetErrorString();
		cout << err << endl;
		return(0);
	}


	// *********************************
	// Init OIV primitives
	// *********************************
	SoNormalBinding *nBind = new SoNormalBinding;
	nBind->value = SoNormalBinding::PER_VERTEX;
	nBind->value = SoNormalBinding::PER_VERTEX_INDEXED;
	inSep->addChild(nBind);
	SoTextureCoordinateBinding *texBind = new SoTextureCoordinateBinding;
	texBind->value.setValue(SoTextureCoordinateBinding::PER_VERTEX_INDEXED);
	inSep->addChild(texBind);

	// *********************************
	// Materials -- Filenames -- some filenames may be used by multiple meshes
	// *********************************
	nMaterial = scene->mNumMaterials;
	for (iMaterial = 0; iMaterial < nMaterial; iMaterial++) {
		material = scene->mMaterials[iMaterial];
		int nTex = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (nTex == 0) {
			iTex = -1;
		}
		else {
			aiStatus = material->GetTexture(aiTextureType_DIFFUSE, 0, &aiStr, &aiTexMap, &uvindex);
			if (aiStatus != aiReturn_SUCCESS) {
				warning(1, "draw_data_inv_class::draw_assimp:  cant get required material info");
			}
			texNameString = aiStr.C_Str();
			i = texNameString.find_last_of("/\\");
			name = path;
			name = name.append(texNameString.substr(i+1));
			iTex = assign_tex_file_index(iMaterial, name);
		}
		texFileIndexPerMaterial.push_back(iTex);
	}

	// *********************************
	// Loop over assimp meshes
	// *********************************
	texUseIndex = new int[assimpUniqueTexNames.size()];					// Counts how many times texture file used -- after first time, ref by name rather than define a new tex
	memset(texUseIndex, 0, assimpUniqueTexNames.size() * sizeof(int));
	nMesh = scene->mNumMeshes;
	for (iMesh = 0; iMesh < nMesh; iMesh++) {
		mesh = scene->mMeshes[iMesh];
		iMaterial = mesh->mMaterialIndex;

		SoMaterial *myMat = new SoMaterial;
		SoSeparator *perMeshSep = new SoSeparator;
		SoIndexedFaceSet *meshFaces = new SoIndexedFaceSet;
		SoCoordinate3 *meshCoords = new SoCoordinate3;
		SoNormal *meshNormals = NULL;
		SoTexture2 *tex2 = NULL;
		SoTextureCoordinate2 *texCoords = NULL;

		// *********************************
		// Vertices
		// *********************************
		nVert = mesh->mNumVertices;
		verts = mesh->mVertices;
		for (iVert = 0; iVert < nVert; iVert++) {												// Can cast to set all values at one time?? so must do singly
			meshCoords->point.set1Value(iVert, verts[iVert].x, verts[iVert].y, verts[iVert].z);
		}

		// *********************************
		// Normals
		// *********************************
		if (mesh->HasNormals()) {
			meshNormals = new SoNormal;
			norms = mesh->mNormals;
			for (iVert = 0; iVert < nVert; iVert++) {												// Can cast to set all values at one time?? so must do singly
				meshNormals->vector.set1Value(iVert, norms[iVert].x, norms[iVert].y, norms[iVert].z);
			}
		}
		//meshNormals->vector.setValues(0, nVperMesh[iMesh], &meshNorms[iV1perMesh[iMesh]]);		// First parm is origin rel to OIV entity NOT to loc in input array

		// *********************************
		// Texture coordinates
		// *********************************
		material = scene->mMaterials[iMaterial];
		int nTex = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (texFileIndexPerMaterial[iMaterial] >= 0 && nTex > 0) {
			material->GetTexture(aiTextureType_DIFFUSE, 0, &aiStr, NULL, &uvindex);
			int tt;
			unsigned int pout;
			material->Get(AI_MATKEY_UVWSRC(aiTextureType_DIFFUSE, 0), &tt, &pout);
			texCoords = new SoTextureCoordinate2;
			meshTexCoords = mesh->mTextureCoords;
			aiVector3D *xt = meshTexCoords[0];
			for (iVert = 0; iVert < nVert; iVert++) {												// Can cast to set all values at one time?? so must do singly
				texCoords->point.set1Value(iVert, xt[iVert].x, xt[iVert].y);
			}
		}

		// *********************************
		// Faces
		// *********************************
		nFace = mesh->mNumFaces;
		faces = mesh->mFaces;
		iIndexOut = 0;
		for (iFace = 0; iFace < nFace; iFace++) {
			nIndex = faces[iFace].mNumIndices;
			aiIndex = faces[iFace].mIndices;
			for (iIndex = 0; iIndex < nIndex; iIndex++, iIndexOut++) {
				meshFaces->coordIndex.set1Value(iIndexOut, aiIndex[iIndex]);
				meshFaces->normalIndex.set1Value(iIndexOut, aiIndex[iIndex]);
				if (texCoords != NULL) meshFaces->textureCoordIndex.set1Value(iIndexOut, aiIndex[iIndex]);
			}
			meshFaces->coordIndex.set1Value(iIndexOut, SO_END_FACE_INDEX);
			meshFaces->normalIndex.set1Value(iIndexOut, SO_END_FACE_INDEX);
			if (texCoords != NULL) meshFaces->textureCoordIndex.set1Value(iIndexOut, SO_END_FACE_INDEX);
			iIndexOut++;
		}

		// *********************************
		// Ambient and Diffuse Color
		// *********************************
		aiStatus = aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &aColor);
		if (aiStatus == aiReturn_SUCCESS) {
			myMat->diffuseColor.setValue(aColor.r, aColor.g, aColor.b);
		}
		else {
			myMat->diffuseColor.setValue(1.0, 1.0, 1.0);
		}
		aiStatus = aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &aColor);
		if (aiStatus == aiReturn_SUCCESS) {
			myMat->ambientColor.setValue(aColor.r, aColor.g, aColor.b);
		}
		else {
			myMat->diffuseColor.setValue(1.0, 1.0, 1.0);
		}

		// *********************************
		// First diffuse texture
		// *********************************
		if (texFileIndexPerMaterial[iMaterial] >= 0) {
			//tex2->wrapS = SoTexture2::CLAMP;						// Default repeat looks to be correct
			//tex2->wrapT = SoTexture2::CLAMP;
			//tex2->filename.set(texFilename[iMaterial].c_str());	// Not implemented in Coin3d
			iTex = texFileIndexPerMaterial[iMaterial];
			i = assimpUniqueTexNames[iTex].find_last_of("/\\");
			texNameString = assimpUniqueTexNames[iTex].substr(i + 1);		// OIV doesnt like pathnames, so strip path
			i = texNameString.find_last_of(".");
			texNameString.erase(i, 1);

			if (texUseIndex[iTex] == 0) {
				tex2 = new SoTexture2;
				tex2->setName(texNameString.c_str());				// This will be passed by name to any other meshes using this texture
				read_tex_image(assimpUniqueTexNames[iTex], tex2);	// My method to implement OpenInventor utility not implemented in Coin3d
			}
			else {
				tex2 = (SoTexture2*)SoNode::getByName(texNameString.c_str());
				//read_tex_image(assimpUniqueTexNames[iTex], tex2);	// This is very inefficient -- so reuse texture already defined
			}
			texUseIndex[iTex]++;
		}

		// *********************************
		// Add elements to subtree for this mesh
		// *********************************
		perMeshSep->addChild(myMat);
		if (meshNormals != NULL) perMeshSep->addChild(meshNormals);
		if (tex2        != NULL) perMeshSep->addChild(tex2);
		if (texCoords   != NULL) perMeshSep->addChild(texCoords);
		perMeshSep->addChild(meshCoords);
		perMeshSep->addChild(meshFaces);
		inSep->addChild(perMeshSep);
	}
#endif
	return(1);
}

// *************************************************************
/// Read an image into a texture.
/// Coin3d does not implement reading files into textures using the filename
/// This can be added using the wrapper library simage but this uses in turn libs that I dont have and that are obsolete anyway.
/// So implement using GDAL.
// *************************************************************
int draw_data_inv_class::read_tex_image(string filename, SoTexture2 *tex2)
{
#if defined(LIBS_GDAL)
	GDALRasterBand *poBand;
	int iband, isamp, iy, ix;

	GDALAllRegister();
	GDALDataset *poDataset = (GDALDataset *)GDALOpen(filename.c_str(), GA_ReadOnly);
	if (poDataset == NULL) {
		warning_s("draw_data_inv_class::read_tex_image: cant open texture image file", filename);
		return(0);
	}

	int nrows = poDataset->GetRasterYSize();
	int ncols = poDataset->GetRasterXSize();
	int nbands = poDataset->GetRasterCount();

	unsigned char *buf = new unsigned char[nrows*ncols];
	unsigned char *data = new unsigned char[nrows*ncols*nbands];

	for (iband = 0; iband<nbands; iband++) {
		poBand = poDataset->GetRasterBand(iband + 1);
		poBand->RasterIO(GF_Read, 0, 0, ncols, nrows, buf, ncols, nrows, GDT_Byte, 0, 0);
		for (iy = 0; iy<nrows; iy++) {
			isamp = (nrows - iy - 1) * ncols;		// Flip in y -- looks like OpenInventor utility this replaces expects to be flipped
			//isamp = iy * ncols;					// No flip
			for (ix = 0; ix<ncols; ix++, isamp++) {
				data[nbands*isamp + iband] = buf[iy*ncols + ix];
			}
		}
	}

	tex2->image.setValue(SbVec2s(ncols, nrows), 3, data, SoSFImage::COPY);		// Must copy so I can modify array

	GDALClose(poDataset);
	delete[] buf;
	delete[] data;
#else
	warning(1, "image_png_class not implemented -- GDAL not loaded");
#endif
	return(1);

}