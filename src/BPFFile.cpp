//written by M. Brandon Cox - Booz Allen Hamilton - cox_michael@bah.com

#include "internals.h"

BPFFile::BPFFile()
: STATIC_HEADER_SIZE_IN_BYTES(100), dirtyMinsAndMaxs(false),
  dirtyFieldMinsAndMaxs(false), pointSpacing(0.0), xMin(0.0), xMax(0.0), yMin(0.0), yMax(0.0),
  zMin(0.0), zMax(0.0), decompressedBytesBuffer(NULL), decompressedBytesBufferPos(0), decompressedBytesBufferSize(0),
  numberOfExtraFields(0), numberOfPoints(0), xOffset(0.0), yOffset(0.0), zOffset(0.0), headerDataOnly(false), isCompressed(2), interleaved(0)
{
	extraFields.clear();
	transformMatrix[0][0] = 1.0;
	transformMatrix[0][1] = 0.0;
	transformMatrix[0][2] = 0.0;
	transformMatrix[0][3] = 0.0;
	transformMatrix[1][0] = 0.0;
	transformMatrix[1][1] = 1.0;
	transformMatrix[1][2] = 0.0;
	transformMatrix[1][3] = 0.0;
	transformMatrix[2][0] = 0.0;
	transformMatrix[2][1] = 0.0;
	transformMatrix[2][2] = 1.0;
	transformMatrix[2][3] = 0.0;
	transformMatrix[3][0] = 0.0;
	transformMatrix[3][1] = 0.0;
	transformMatrix[3][2] = 0.0;
	transformMatrix[3][3] = 1.0;
	updateHeaderLength();
	nStride = 1;
	suppressExtraFieldFlags = NULL;
}

const int BPFFile::FIELD_LABEL_LENGTH = 32;
const int BPFFile::HEADER_SIZE_V1 = 100;
const int BPFFile::HEADER_SIZE_V2 = 100;
const int BPFFile::HEADER_SIZE_V3 = 176;
const int BPFFile::HEADER_LENGTH_POS_V3 = 8;
const int BPFFile::NUMBER_OF_DIMS_POS_V3 = 12;
const int BPFFile::POINT_COUNT_POS_V3 = 16;
const char* BPFFile::ASCII_MAGIC_NUMBER_STRING = "BPF!";
const unsigned BPFFile::INTERLEAVED_POINTS_PER_COMPRESSED_BLOCK = 100000;


BPFFile::~BPFFile()
{
	for(unsigned int i = 0; i < extraFields.size(); i++)
		delete extraFields[i];

	delete [] decompressedBytesBuffer;	
	if (suppressExtraFieldFlags == NULL) delete[] suppressExtraFieldFlags ;


}

int BPFFile::setNStride(int n)
{
	nStride = n;
	return(1);
}

int BPFFile::suppressExtraField(int ifield)
{
	if (ifield < 0 || ifield >= numberOfExtraFields) {
		return(0);
	}
	else {
		suppressExtraFieldFlags[ifield] = 1;
		return(1);
	}
}

int BPFFile::getHeaderLength()
{
	updateHeaderLength();
	return headerLength;
}

int BPFFile::getNumberOfPoints()
{
	updateNumberOfPoints();
	return numberOfPoints;
}

int BPFFile::getNumberOfExtraFields()
{
	//only update if this object doesn't represent only header data
	//otherwise, the number of extra field data will get zeroed in the update
	if(!this->headerDataOnly)
		updateNumberOfExtraFields();
	return numberOfExtraFields;
}

double BPFFile::getXMin()
{
	updateXYZMinsAndMaxs();
	return xMin;
}

double BPFFile::getXMax()
{
	updateXYZMinsAndMaxs();
	return xMax;
}

double BPFFile::getYMin()
{
	updateXYZMinsAndMaxs();
	return yMin;
}

double BPFFile::getYMax()
{
	updateXYZMinsAndMaxs();
	return yMax;
}

double BPFFile::getZMin()
{
	updateXYZMinsAndMaxs();
	return zMin;
}

double BPFFile::getZMax()
{
	updateXYZMinsAndMaxs();
	return zMax;
}

bool BPFFile::mergeWithBPFFile(BPFFile& newData)
{
	//prerequisites for merge
	//same number of extra fields
	//same coordinate system
	//same zone
	if(this->zone != newData.getZone() || this->coordinateSystem != newData.getCoordinateSystem()
		|| this->numberOfExtraFields != newData.getNumberOfExtraFields())
		return false;
	else
	{
		int numNewPoints = newData.getNumberOfPoints();
		//copy x's
		for(int i = 0; i < numNewPoints; i++)
			this->xValues.push_back((float)(newData.getXValue(i) - this->xOffset));
		//copy y's
		for(int i = 0; i < numNewPoints; i++)
			this->yValues.push_back((float)(newData.getYValue(i) - this->yOffset));
		//copy z's 
		for(int i = 0; i < numNewPoints; i++)
			this->zValues.push_back((float)(newData.getZValue(i) - this->zOffset));
		//copy extra fields
		for(int i = 0; i < this->numberOfExtraFields; i++)
		{
			for(int n = 0; n < numNewPoints; n++)
			{
				this->extraFields[i]->push_back(newData.getExtraFieldValueByNumber(i, n)
					- this->fieldOffsets[i]);
			}
		}
		//recalculate all of the min and max values
		this->updateNumberOfPoints();
		this->xMin = (this->xMin < newData.getXMin() ? this->xMin : newData.getXMin());
		this->yMin = (this->yMin < newData.getYMin() ? this->yMin : newData.getYMin());
		this->zMin = (this->zMin < newData.getZMin() ? this->zMin : newData.getZMin());
		this->xMax = (this->xMax > newData.getXMax() ? this->xMax : newData.getXMax());
		this->yMax = (this->yMax > newData.getYMax() ? this->yMax : newData.getYMax());
		this->zMax = (this->zMax > newData.getZMax() ? this->zMax : newData.getZMax());

		for(int i = 0; i < this->numberOfExtraFields; i++)
		{
			this->minFieldValues[i] = (this->minFieldValues[i] < newData.getFieldMin(i) ?
				this->minFieldValues[i] : newData.getFieldMin(i));
			this->maxFieldValues[i] = (this->maxFieldValues[i] < newData.getFieldMax(i) ?
				this->maxFieldValues[i] : newData.getFieldMax(i));
		}

		return true;
	}
}

double BPFFile::getFieldMin(int n)
{
	updateFieldMinsAndMaxs();
	return this->minFieldValues[n];
}

double BPFFile::getFieldMax(int n)
{
	updateFieldMinsAndMaxs();
	return this->maxFieldValues[n];
}

void BPFFile::setXYZCoordinates(const vector<float>& X, const vector<float>& Y, const vector<float>& Z)
{
	if(X.size() != Y.size() || Y.size() != Z.size())
		throw BPFFile::InvalidBPFData("In BPFFile::setXYZCoordinates:  X, Y, and Z vectors not equal size");
	
	for(unsigned int i = 0; i < X.size(); i++)
	{
		xValues.push_back(X[i]);
		yValues.push_back(Y[i]);
		zValues.push_back(Z[i]);
	}
	updateNumberOfPoints();
	dirtyMinsAndMaxs = true;
	updateXYZMinsAndMaxs();
}

void BPFFile::addExtraFieldData(const string& label, vector<float>& extraField, const double& offset)
{
	if(extraField.size() != this->numberOfPoints)
		throw BPFFile::InvalidBPFData("In BPFFile::addExtraFieldData:  extraField length does not match number of points");
		
	if(label.length() > FIELD_LABEL_LENGTH-1)
		throw BPFFile::InvalidBPFData("In BPFFile::addExtraFieldData:  extra field label length is greater than allowed");
		
	fieldLabels.push_back(label);
	fieldOffsets.push_back(offset);
	extraFields.push_back(new vector<float>);
	
	float tempFloat;
	//mins and maxs should be reported as the unoffset values
	tempFloat = *min_element(extraField.begin(), extraField.end());
	minFieldValues.push_back(tempFloat+offset);
	tempFloat = *max_element(extraField.begin(), extraField.end());
	maxFieldValues.push_back(tempFloat+offset);
	
	for(int i = 0; i < this->numberOfPoints; i++)
	{
		extraFields.back()->push_back(extraField[i]);
	}
	updateNumberOfExtraFields();
	updateHeaderLength();
}

void BPFFile::removeExtraField(const string& label)
{
	//first determine if the specified extra field exists
	unsigned int labelEFI = this->getExtraFieldNumberByName(label);
	
	//erase all data associated with this extra field.
	this->fieldOffsets.erase(this->fieldOffsets.begin()+labelEFI);
	this->minFieldValues.erase(this->minFieldValues.begin()+labelEFI);
	this->maxFieldValues.erase(this->maxFieldValues.begin()+labelEFI);
	this->fieldLabels.erase(this->fieldLabels.begin()+labelEFI);
	
	delete this->extraFields[labelEFI];
	this->extraFields.erase(this->extraFields.begin()+labelEFI);
	
	this->numberOfExtraFields--;
}

int BPFFile::calculateHeaderLength()
{
	return  STATIC_HEADER_SIZE_IN_BYTES
		+ fieldOffsets.size() * sizeof(double)
		+ minFieldValues.size() * sizeof(double)
		+ maxFieldValues.size() * sizeof(double)
		+ fieldLabels.size() * FIELD_LABEL_LENGTH;
					
}

void BPFFile::updateHeaderLength()
{
	this->headerLength = calculateHeaderLength();
}

void BPFFile::updateXYZMinsAndMaxs()
{
	if(dirtyMinsAndMaxs == true)
	{
		xMin = *min_element(xValues.begin(), xValues.end()) + this->xOffset;
		xMax = *max_element(xValues.begin(), xValues.end()) + this->xOffset;
		yMin = *min_element(yValues.begin(), yValues.end()) + this->yOffset;
		yMax = *max_element(yValues.begin(), yValues.end()) + this->yOffset;
		zMin = *min_element(zValues.begin(), zValues.end()) + this->zOffset;
		zMax = *max_element(zValues.begin(), zValues.end()) + this->zOffset;		
		
		dirtyMinsAndMaxs = false;
	}
}

void BPFFile::updateFieldMinsAndMaxs()
{
	if(dirtyFieldMinsAndMaxs == true)
	{
		for(unsigned int i = 0; i < extraFields.size(); i++)
		{
			minFieldValues[i] = *min_element(extraFields[i]->begin(), extraFields[i]->end())
				+ this->fieldOffsets[i];
			maxFieldValues[i] = *max_element(extraFields[i]->begin(), extraFields[i]->end())
				+ this->fieldOffsets[i];
		}
		dirtyFieldMinsAndMaxs = false;		
	}
}

void BPFFile::updateNumberOfPoints()
{
	//doesn't make sense to update if only header data has been read
	if(this->headerDataOnly)
		return;
	else
		this->numberOfPoints = xValues.size();
		
	return;
}

void BPFFile::updateNumberOfExtraFields()
{
	//don't update if only header data has been loaded
	if(!this->headerDataOnly)
		this->numberOfExtraFields = extraFields.size();
}

bool BPFFile::writeBPFFileV3(const string& filename)
{
	ofstream fout;
	fout.open(filename.c_str(), ios_base::binary);
	if(!fout.is_open())
		return false;
	
	writeBPFFileV3(fout);
	fout.close();
	
	return true;
}

void BPFFile::writeBPFFileV3(ofstream& fout)
{
	try
	{
		updateNumberOfPoints();
	 	updateNumberOfExtraFields();
		//POS 0: Write ASCII Magic Number Identifier ("BPF!")
		fout.write(ASCII_MAGIC_NUMBER_STRING, 4);
		//POS 4: write version
		const char* ASCII_FORMAT_VERSION = "0003";
		fout.write(ASCII_FORMAT_VERSION, 4);
		//POS 8: write header length
		int v3HeaderSize = HEADER_SIZE_V3 + 56*(this->numberOfExtraFields+3);
		fout.write((char*)&v3HeaderSize, 4);
		//POS 12: number of dimensions (uchar)
		//3 added because this class stores x, y, and z seperately from the extra fields
		unsigned char numberOfDims = this->numberOfExtraFields + 3;
		fout.write((char*)&numberOfDims, 1);
		unsigned char tempChar = 0;
		//POS 13: interleaved (uchar)
		fout.write((char*)&tempChar, 1);
		//POS 14: compressed file (uchar)
		fout.write((char*)&this->isCompressed, 1);
		//POS 15: unused (char)
		char unusedChar = 'X';
		fout.write((char*)&unusedChar, 1);
		//POS 16: point count (int 32)
		updateNumberOfPoints(); //must be called in case points have been added
		fout.write((char*)&this->numberOfPoints, 4);
		//POS 20: coordinate space type (int 32)
		fout.write((char*)&this->coordinateSystem, 4);
		//POS 24: coordinate space ID (utm zone) (int 32)
		fout.write((char*)&this->zone, 4);
		//POS 28: point spacing
		fout.write((char*)&this->pointSpacing, 4);
		//POS 32: transformation matrix (16 x float64)
		fout.write((char*)this->transformMatrix, 128);
		//POS 160: start time (float64)
		fout.write((char*)&this->startTime, 8);
		//POS 168: end time (float 64)
		fout.write((char*)&this->endTime, 8);
		
		//Metadata subheader
		//write xOffset
		fout.write((char*)&this->xOffset, sizeof(double));
		//write yOffset
		fout.write((char*)&this->yOffset, sizeof(double));
		//write zOffset
		fout.write((char*)&this->zOffset, sizeof(double));
		//write offsets for other fields
		for(unsigned int i = 0; i < fieldOffsets.size(); i++)
			fout.write((char*)&fieldOffsets[i], sizeof(double));
			
		//minimums
		updateXYZMinsAndMaxs();
		fout.write((char*)&this->xMin, sizeof(double));
		fout.write((char*)&this->yMin, sizeof(double));
		fout.write((char*)&this->zMin, sizeof(double));
		updateFieldMinsAndMaxs();
		for(unsigned int i = 0; i< minFieldValues.size(); i++)
			fout.write((char*)&minFieldValues[i], sizeof(double));
			
		//maximums
		fout.write((char*)&this->xMax, sizeof(double));
		fout.write((char*)&this->yMax, sizeof(double));
		fout.write((char*)&this->zMax, sizeof(double));
		for(unsigned int i = 0; i < maxFieldValues.size(); i++)
			fout.write((char*)&maxFieldValues[i], sizeof(double));
			
		//field labels
		char tempStr[FIELD_LABEL_LENGTH];
		//zero tempStr
		for(int j=0; j<FIELD_LABEL_LENGTH; j++)
			tempStr[j] = (char)0;
		strcpy(tempStr, "X");
		fout.write(tempStr, FIELD_LABEL_LENGTH);
		strcpy(tempStr, "Y");
		fout.write(tempStr, FIELD_LABEL_LENGTH);
		strcpy(tempStr, "Z");
		fout.write(tempStr, FIELD_LABEL_LENGTH);		
		for(unsigned int i = 0; i < fieldLabels.size(); i++)
		{
			//zero tempStr
			for(int j=0; j<FIELD_LABEL_LENGTH; j++)
				tempStr[j] = (char)0;
			strcpy(tempStr, fieldLabels[i].c_str());
			//write
			fout.write(tempStr, FIELD_LABEL_LENGTH);
		}
		
		if(true)
		{
			//write data compressed
			float* writeBuf = new float[this->numberOfPoints];
			//write X values
			for(unsigned int i = 0; i < this->numberOfPoints; i++)
				writeBuf[i] = xValues[i];
			writeBytesToFile(fout, (char*)writeBuf, sizeof(float) * this->numberOfPoints);
			//write Y values
			for(unsigned int i = 0; i < this->numberOfPoints; i++)
				writeBuf[i] = yValues[i];
			writeBytesToFile(fout, (char*)writeBuf, sizeof(float) * this->numberOfPoints);
			//write Z values
			for(unsigned int i = 0; i < this->numberOfPoints; i++)
				writeBuf[i] = zValues[i];
			writeBytesToFile(fout, (char*)writeBuf, sizeof(float) * this->numberOfPoints);
			//write extra fields values
			for(unsigned int i = 0; i < extraFields.size(); i++)
			{
				for(int j = 0; j < this->numberOfPoints; j++)	
					writeBuf[j] = (*extraFields[i])[j];
				writeBytesToFile(fout, (char*)writeBuf, sizeof(float) * this->numberOfPoints);
			
			}
			delete [] writeBuf;
		}	
		else
		{
			//fill output buffer with number of points equal to
			//INTERLEAVED_POINTS_PER_COMPRESSED_BLOCK
			unsigned long int pointRecordSize = (3 + extraFields.size());
			unsigned long int writeBufSize = pointRecordSize * INTERLEAVED_POINTS_PER_COMPRESSED_BLOCK;
			float* writeBuf = new float[writeBufSize];
			
			unsigned int ptsWritten = 0;
			while(ptsWritten < this->numberOfPoints)
			{
				//fill the buffer or fill with as many points as possible
				unsigned int ptsInBuf = 0;
				unsigned int bufferPos = 0;
				while(ptsInBuf < INTERLEAVED_POINTS_PER_COMPRESSED_BLOCK && ptsWritten < this->numberOfPoints)
				{
					writeBuf[bufferPos++] = xValues[ptsWritten];
					writeBuf[bufferPos++] = yValues[ptsWritten];
					writeBuf[bufferPos++] = zValues[ptsWritten];
					for(unsigned int i = 0; i < extraFields.size(); i++)
					{
						writeBuf[bufferPos++] = (*extraFields[i])[ptsWritten];
					}
					ptsInBuf++;
					ptsWritten++;
				}
				writeBytesToFile(fout, (char*)writeBuf, sizeof(float) * pointRecordSize * ptsInBuf);
			}
			delete [] writeBuf;
		}	
		
		
		
	}
	catch(ios_base::failure&)
	{
		cerr << "Error writing to file in BPFFile::writeBPFFileV3" << endl;
		return;
	}
}

void BPFFile::writeBPFFile(ofstream& fout)
{
	try
	{
		//write static header section
		//update header length
		updateHeaderLength();
		//write header length
		fout.write((char*)&this->headerLength, sizeof(int));
		//write version
		int outVersion = 1;
		fout.write((char*)&outVersion, sizeof(int));
		//write number of Points
		updateNumberOfPoints();
		fout.write((char*)&this->numberOfPoints, sizeof(int));
		//write number of extra fields
		updateNumberOfExtraFields();
		fout.write((char*)&this->numberOfExtraFields, sizeof(int));
		//write coordinate system
		fout.write((char*)&this->coordinateSystem, sizeof(int));
		//write zone
		fout.write((char*)&this->zone, sizeof(int));
		//write pointSpacing
		fout.write((char*)&this->pointSpacing, sizeof(float));
		//write xOffset
		fout.write((char*)&this->xOffset, sizeof(double));
		//write yOffset
		fout.write((char*)&this->yOffset, sizeof(double));
		//write zOffset
		fout.write((char*)&this->zOffset, sizeof(double));
		//update the minimum values and maximum values
		updateXYZMinsAndMaxs();
		fout.write((char*)&this->xMin, sizeof(double));
		fout.write((char*)&this->xMax, sizeof(double));
		fout.write((char*)&this->yMin, sizeof(double));
		fout.write((char*)&this->yMax, sizeof(double));
		fout.write((char*)&this->zMin, sizeof(double));
		fout.write((char*)&this->zMax, sizeof(double));
		
		//write variable length header section
		//write field offsets
		for(unsigned int i = 0; i < fieldOffsets.size(); i++)
			fout.write((char*)&fieldOffsets[i], sizeof(double));
		//write field minimum values
		updateFieldMinsAndMaxs();
		for(unsigned int i = 0; i< minFieldValues.size(); i++)
			fout.write((char*)&minFieldValues[i], sizeof(double));
		for(unsigned int i = 0; i < maxFieldValues.size(); i++)
			fout.write((char*)&maxFieldValues[i], sizeof(double));
		//write field labels
		char tempStr[FIELD_LABEL_LENGTH];
		for(unsigned int i = 0; i < fieldLabels.size(); i++)
		{
			//zero tempStr
			for(int j=0; j<FIELD_LABEL_LENGTH; j++)
				tempStr[j] = (char)0;
			strcpy(tempStr, fieldLabels[i].c_str());
			//write
			fout.write(tempStr, FIELD_LABEL_LENGTH);
		}
		//write X, Y, and Z values
		for(unsigned int i = 0; i < xValues.size(); i++)
		{
			fout.write((char*)&xValues[i], sizeof(float));
		}
		for(unsigned int i = 0; i < yValues.size(); i++)
		{
			fout.write((char*)&yValues[i], sizeof(float));
		}
		for(unsigned int i = 0; i < zValues.size(); i++)
		{
			fout.write((char*)&zValues[i], sizeof(float));
		}
		//write extra field values
		for(unsigned int i = 0; i < extraFields.size(); i++)
		{
			for(unsigned int j = 0; j < extraFields[i]->size(); j++)
			{
				fout.write((char*)&(*extraFields[i])[j], sizeof(float));
			}
		}
		
		
	}
	catch(ios_base::failure&)
	{
		cerr << "Error writing to file in BPFFile::writeToBPFFile" << endl;
		return;
	}
}

bool BPFFile::getExtraFieldValueByName(const int n, const string& name, double& valueOut)
{
	//find field with name "name"
	for(int i = 0; i < fieldLabels.size(); i++)
	{
		if(fieldLabels[i] == name)
		{
			if(n >= extraFields[i]->size())
				return false;
			valueOut = fieldOffsets[i] + (*extraFields[i])[n];
			return true;
		}
	}
	return false;
}

string BPFFile::getExtraFieldName(const int fieldNum) const
{
	if(fieldNum >= 0 && fieldNum < fieldLabels.size())
		return fieldLabels[fieldNum];
	else
		return "";
}

double BPFFile::getExtraFieldValueByNumber(const int fieldNum, const int pointNum) const
{
	if(fieldNum < this->numberOfExtraFields)
	{
		return fieldOffsets[fieldNum] + (*extraFields[fieldNum])[pointNum];
	}
	return 0.0;
}

float BPFFile::getExtraFieldValueByNumberNoOffset(const int fieldNum, const int pointNum) const
{
	if(fieldNum < this->numberOfExtraFields)
	{
		return (*extraFields[fieldNum])[pointNum];
	}
	return 0.0;
}

unsigned int BPFFile::getExtraFieldNumberByName(const string& fieldName) const
{
	for(int i = 0; i < fieldLabels.size(); i++)
	{
		if(fieldLabels[i] == fieldName)
		{
			return i;
		}
	}
	throw ParameterOutOfRange("Field name <" + fieldName + "> does not exist");
	return 0;
}

bool BPFFile::readBPFFile(const string& filename, bool readHeaderOnly)
{
	ifstream fin;
	fin.open(filename.c_str(), ios_base::binary);
	
	if(!fin.is_open())
		return false;
	else
	{
		readBPFFile(fin, readHeaderOnly);
		fin.close();
		return true;
	}
}

void BPFFile::readBPFFile(ifstream& fin, bool readHeaderOnly)
{
	this->version = this->determineFileVersion(fin);
	this->headerDataOnly = readHeaderOnly;

	if(this->version == 1)
		this->readBPFV1File(fin);
	else if(this->version == 2)
		cerr << "Can't read BPF2 just yet" << endl;
	else if(this->version == 3)
		this->readBPFV3File(fin);
	else
		cerr << "Invalid BPF file" << endl;

	return;
}

void BPFFile::readBPFHeader(ifstream& fin)
{
	this->version = this->determineFileVersion(fin);

	if(this->version == 1)
		this->readBPFV1Header(fin);
	else if(this->version == 2)
		cerr << "Can't read BPF2 just yet" << endl;
	else if(this->version == 3)
		this->readBPFV3Header(fin);
	else
		cerr << "Invalid BPF file" << endl;

	return;
}

void BPFFile::readBPFV3Header(ifstream& fin)
{
	if(fin.is_open())
	{
		unsigned int calculatedFileSize;
		unsigned int actualFileSize;
		fin.seekg(0, ios::end);
		actualFileSize = fin.tellg();
		fin.seekg(0, ios::beg);
		this->headerDataOnly = true;
		
		try
		{
			//read header length
			fin.seekg(this->HEADER_LENGTH_POS_V3, ios::beg);
			fin.read((char*)&this->headerLength, sizeof(int));
			//read number of dimensions
			unsigned char numberOfDims;
			fin.read((char*)&numberOfDims, sizeof(unsigned char));
			//BPF3 includes x, y, and z in this number, store in BPF1/2 format
			this->numberOfExtraFields = numberOfDims - 3;
			//read interleaved flag
			fin.read((char*)&this->interleaved, sizeof(unsigned char));
			//read compressed flag
			fin.read((char*)&this->isCompressed, sizeof(unsigned char));
			//read point count
			fin.seekg(this->POINT_COUNT_POS_V3, ios::beg);
			fin.read((char*)&this->numberOfPoints, sizeof(int));
			//read coordinate system
			fin.read((char*)&this->coordinateSystem, sizeof(int));
			//read utm zone
			fin.read((char*)&this->zone, sizeof(int));
			//read point spacing
			fin.read((char*)&this->pointSpacing, sizeof(float));
			//read transform matrix
			for(int row = 0; row < 4; row++)
				for(int col = 0; col < 4; col++)
					fin.read((char*)&this->transformMatrix[row][col], sizeof(double));

			//read start time
			fin.read((char*)&this->startTime, sizeof(double));
			//read end time
			fin.read((char*)&this->endTime, sizeof(double));


			//READ METADATA SUBHEADER

			fin.seekg(this->HEADER_SIZE_V3, ios::beg);
			//read x offset
			int sizeOfDouble = sizeof(double);
			fin.read((char*)&this->xOffset, sizeof(double));
			//read y offset
			fin.read((char*)&this->yOffset, sizeof(double));
			//read z offset
			fin.read((char*)&this->zOffset, sizeof(double));
			//read additional field offsets
			double* tempDoubleArray = new double[this->numberOfExtraFields];
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				fieldOffsets.push_back(tempDoubleArray[i]);


			//read x min
			fin.read((char*)&this->xMin, sizeof(double));
			//read y min
			fin.read((char*)&this->yMin, sizeof(double));
			//read z min
			fin.read((char*)&this->zMin, sizeof(double));
			//read additional field minimums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				minFieldValues.push_back(tempDoubleArray[i]);


			//read x max
			fin.read((char*)&this->xMax, sizeof(double));			
			//read y max
			fin.read((char*)&this->yMax, sizeof(double));			
			//read z max
			fin.read((char*)&this->zMax, sizeof(double));
			//read additional field maximums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				maxFieldValues.push_back(tempDoubleArray[i]);
			delete [] tempDoubleArray;


			
			//skip reading x, y, and z field labels
			fin.clear();
			unsigned int fieldLabelPos = this->HEADER_SIZE_V3 + 24*numberOfDims + 32*3;
			fin.seekg(fieldLabelPos);
			//read field labels
			char* tempCharArray = new char[FIELD_LABEL_LENGTH];
			for(int i = 0; i < numberOfExtraFields; i++)
			{
				fin.read(tempCharArray, FIELD_LABEL_LENGTH);
				fieldLabels.push_back(string(tempCharArray));
			}			
			delete [] tempCharArray;
		}
		catch(std::exception& e)
		{
			throw BPFFile::IOFailure("Attempt to read BPF failed when calling ifstream.read");
		}
	}
}

void BPFFile::readBPFV3File(ifstream& fin)
{
	if(fin.is_open())
	{
		unsigned int calculatedFileSize;
		unsigned int actualFileSize;
		fin.seekg(0, ios::end);
		actualFileSize = fin.tellg();
		fin.seekg(0, ios::beg);
		
		try
		{
			//read header length
			fin.seekg(this->HEADER_LENGTH_POS_V3, ios::beg);
			fin.read((char*)&this->headerLength, sizeof(int));
			//read number of dimensions
			unsigned char numberOfDims;
			fin.read((char*)&numberOfDims, sizeof(unsigned char));
			//BPF3 includes x, y, and z in this number, store in BPF1/2 format
			this->numberOfExtraFields = numberOfDims - 3;
			//read interleaved flag
			fin.read((char*)&this->interleaved, sizeof(unsigned char));
			//read compressed flag
			fin.read((char*)&this->isCompressed, sizeof(unsigned char));
			//read point count
			fin.seekg(this->POINT_COUNT_POS_V3, ios::beg);
			fin.read((char*)&this->numberOfPoints, sizeof(int));
			//read coordinate system
			fin.read((char*)&this->coordinateSystem, sizeof(int));
			//read utm zone
			fin.read((char*)&this->zone, sizeof(int));
			//read point spacing
			fin.read((char*)&this->pointSpacing, sizeof(float));
			//read transform matrix
			for(int row = 0; row < 4; row++)
				for(int col = 0; col < 4; col++)
					fin.read((char*)&this->transformMatrix[row][col], sizeof(double));

			//read start time
			fin.read((char*)&this->startTime, sizeof(double));
			//read end time
			fin.read((char*)&this->endTime, sizeof(double));


			//READ METADATA SUBHEADER

			fin.seekg(this->HEADER_SIZE_V3, ios::beg);
			//read x offset
			int sizeOfDouble = sizeof(double);
			fin.read((char*)&this->xOffset, sizeof(double));
			//read y offset
			fin.read((char*)&this->yOffset, sizeof(double));
			//read z offset
			fin.read((char*)&this->zOffset, sizeof(double));
			//read additional field offsets
			double* tempDoubleArray = new double[this->numberOfExtraFields];
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				fieldOffsets.push_back(tempDoubleArray[i]);


			//read x min
			fin.read((char*)&this->xMin, sizeof(double));
			//read y min
			fin.read((char*)&this->yMin, sizeof(double));
			//read z min
			fin.read((char*)&this->zMin, sizeof(double));
			//read additional field minimums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				minFieldValues.push_back(tempDoubleArray[i]);


			//read x max
			fin.read((char*)&this->xMax, sizeof(double));			
			//read y max
			fin.read((char*)&this->yMax, sizeof(double));			
			//read z max
			fin.read((char*)&this->zMax, sizeof(double));
			//read additional field maximums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				maxFieldValues.push_back(tempDoubleArray[i]);
			delete [] tempDoubleArray;


			
			//skip reading x, y, and z field labels
			fin.clear();
			unsigned int fieldLabelPos = this->HEADER_SIZE_V3 + 24*numberOfDims + 32*3;
			fin.seekg(fieldLabelPos);
			//read field labels
			char* tempCharArray = new char[FIELD_LABEL_LENGTH];
			for(int i = 0; i < numberOfExtraFields; i++)
			{
				fin.read(tempCharArray, FIELD_LABEL_LENGTH);
				fieldLabels.push_back(string(tempCharArray));
			}			
			delete [] tempCharArray;
			
			// JFD added
			if (suppressExtraFieldFlags == NULL) {
				suppressExtraFieldFlags = new int[numberOfExtraFields];
				memset(suppressExtraFieldFlags, 0, numberOfExtraFields * sizeof(int));
			}
			if(this->headerDataOnly)
				return;
			
			if(this->interleaved == 0)
			{
				//seek to beginning of points
				fin.seekg(this->headerLength, ios::beg);
				//read point data non-interleaved
				float* tempFloatArray = new float[this->numberOfPoints];
				//read x
				this->readBytesFromFile(fin, (char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
				for(int i = 0; i < this->numberOfPoints; i=i+nStride)
					xValues.push_back(tempFloatArray[i]);
				//read y
				this->readBytesFromFile(fin, (char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
				for(int i = 0; i < this->numberOfPoints; i=i+nStride)
					yValues.push_back(tempFloatArray[i]);
				//read z
				this->readBytesFromFile(fin, (char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
				for(int i = 0; i < this->numberOfPoints; i=i+nStride)
					zValues.push_back(tempFloatArray[i]);
					
				
				//read extra field values
				for(int i = 0; i < this->numberOfExtraFields; i++)
				{
					extraFields.push_back(new vector<float>);
					this->readBytesFromFile(fin, (char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
					if (suppressExtraFieldFlags[i]) continue;

					for(int j = 0; j < this->numberOfPoints; j=j+nStride)
						extraFields[i]->push_back(tempFloatArray[j]);
				}
				delete [] tempFloatArray;
			}
			else //is interleaved
			{
				//seek to beginning of points
				fin.seekg(this->headerLength, ios::beg);

				float* floatTemp = new float[numberOfDims];

				for(int i = 0; i < this->numberOfExtraFields; i++)
					this->extraFields.push_back(new vector<float>);

				for(int point = 0; point < this->numberOfPoints; point++)
				{
					this->readBytesFromFile(fin, (char*)floatTemp, 4 * numberOfDims);
					// Transfer only every nStride'th point
					if (point % nStride == 0) {
						this->xValues.push_back(floatTemp[0]);
						this->yValues.push_back(floatTemp[1]);
						this->zValues.push_back(floatTemp[2]);
						for(int extraDimNum = 3; extraDimNum < numberOfDims; extraDimNum++)
						{						
							if (!suppressExtraFieldFlags[extraDimNum-3]) this->extraFields[extraDimNum-3]->push_back(floatTemp[extraDimNum]);
						}
					}
				}
				delete [] floatTemp;
			}
			
			
		}
		catch(std::exception& e)
		{
			throw BPFFile::IOFailure("Attempt to read BPF failed when calling ifstream.read");
		}
		
	}
	else
	{
		throw BPFFile::IOFailure("Attempt to read failed because file not open.");
	}
}

void BPFFile::readBPFV1Header(ifstream& fin)
{
	if(fin.is_open())
	{
		unsigned int calculatedFileSize;
		unsigned int actualFileSize;
		fin.seekg(0, ios::end);
		actualFileSize = fin.tellg();
		fin.seekg(0, ios::beg);
		this->headerDataOnly = true;
		
		try
		{
			//all of these values are considered suspect until the
			//actual values are calculated from the data and compared
			//read header size
			fin.read((char*)&this->headerLength, sizeof(int));
			//read version
			fin.read((char*)&this->version, sizeof(int));
			//read number of points
			fin.read((char*)&this->numberOfPoints, sizeof(int));
			//read numberOfExtraFields
			fin.read((char*)&this->numberOfExtraFields, sizeof(int));
			//read coordinate system
			fin.read((char*)&this->coordinateSystem, sizeof(int));
			//read utm zone
			fin.read((char*)&this->zone, sizeof(int));
			//read point spacing
			fin.read((char*)&this->pointSpacing, sizeof(float));
			//read x offset
			fin.read((char*)&this->xOffset, sizeof(double));
			//read y offset
			fin.read((char*)&this->yOffset, sizeof(double));
			//read z offset
			fin.read((char*)&this->zOffset, sizeof(double));
			//read x min
			fin.read((char*)&this->xMin, sizeof(double));
			//read x max
			fin.read((char*)&this->xMax, sizeof(double));
			//read y min
			fin.read((char*)&this->yMin, sizeof(double));
			//read y max
			fin.read((char*)&this->yMax, sizeof(double));
			//read z min
			fin.read((char*)&this->zMin, sizeof(double));
			//read z max
			fin.read((char*)&this->zMax, sizeof(double));
			
			//before going any further, let's validate the claimed number of points and number of extra fields
			calculatedFileSize = this->headerLength + 12*this->numberOfPoints
								+ 4*this->numberOfExtraFields*this->numberOfPoints;
			char msg[150];
			if(calculatedFileSize != actualFileSize)
			{
				sprintf(msg, "Actual filesize = %d, calculated file size = %d", actualFileSize, calculatedFileSize);
				fin.close();
				throw BPFFile::InvalidBPFData(msg);
			}
										
			//if we're here, then we know "numberOfPoints" and "numberOfExtraFields" are valid
			//read field offsets
			double* tempDoubleArray = new double[this->numberOfExtraFields];
			
			//read field offsets
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				fieldOffsets.push_back(tempDoubleArray[i]);
			//read field minimums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				minFieldValues.push_back(tempDoubleArray[i]);
			//read field maximums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				maxFieldValues.push_back(tempDoubleArray[i]);
				
			delete [] tempDoubleArray;
			
			//read field labels
			char* tempCharArray = new char[FIELD_LABEL_LENGTH];
			for(int i = 0; i < numberOfExtraFields; i++)
			{
				fin.read(tempCharArray, FIELD_LABEL_LENGTH);
				fieldLabels.push_back(string(tempCharArray));
			}			
			delete [] tempCharArray;
			
			fin.close();
			
		}
		catch(std::exception& e)
		{
			throw BPFFile::IOFailure("Attempt to read BPF failed when calling ifstream.read");
		}
		
	}
	else
	{
		throw BPFFile::IOFailure("Attempt to read failed because file not open.");
	}	
}

void BPFFile::readBPFV1File(ifstream& fin)
{
	if(fin.is_open())
	{
		unsigned int calculatedFileSize;
		unsigned int actualFileSize;
		fin.seekg(0, ios::end);
		actualFileSize = fin.tellg();
		fin.seekg(0, ios::beg);
		
		this->isCompressed = 0;  //can't be since its V1
		this->interleaved = 0; //again, can't be in V1
		
		try
		{
			//all of these values are considered suspect until the
			//actual values are calculated from the data and compared
			//read header size
			fin.read((char*)&this->headerLength, sizeof(int));
			//read version
			fin.read((char*)&this->version, sizeof(int));
			//read number of points
			fin.read((char*)&this->numberOfPoints, sizeof(int));
			//read numberOfExtraFields
			fin.read((char*)&this->numberOfExtraFields, sizeof(int));
			//read coordinate system
			fin.read((char*)&this->coordinateSystem, sizeof(int));
			//read utm zone
			fin.read((char*)&this->zone, sizeof(int));
			//read point spacing
			fin.read((char*)&this->pointSpacing, sizeof(float));
			//read x offset
			fin.read((char*)&this->xOffset, sizeof(double));
			//read y offset
			fin.read((char*)&this->yOffset, sizeof(double));
			//read z offset
			fin.read((char*)&this->zOffset, sizeof(double));
			//read x min
			fin.read((char*)&this->xMin, sizeof(double));
			//read x max
			fin.read((char*)&this->xMax, sizeof(double));
			//read y min
			fin.read((char*)&this->yMin, sizeof(double));
			//read y max
			fin.read((char*)&this->yMax, sizeof(double));
			//read z min
			fin.read((char*)&this->zMin, sizeof(double));
			//read z max
			fin.read((char*)&this->zMax, sizeof(double));
			
			//before going any further, let's validate the claimed number of points and number of extra fields
			calculatedFileSize = this->headerLength + 12*this->numberOfPoints
								+ 4*this->numberOfExtraFields*this->numberOfPoints;
			char msg[150];
			if(calculatedFileSize != actualFileSize)
			{
				sprintf(msg, "Actual filesize = %d, calculated file size = %d", actualFileSize, calculatedFileSize);
				fin.close();
				throw BPFFile::InvalidBPFData(msg);
			}
										
			//if we're here, then we know "numberOfPoints" and "numberOfExtraFields" are valid
			//read field offsets
			double* tempDoubleArray = new double[this->numberOfExtraFields];
			
			//read field offsets
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				fieldOffsets.push_back(tempDoubleArray[i]);
			//read field minimums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				minFieldValues.push_back(tempDoubleArray[i]);
			//read field maximums
			fin.read((char*)tempDoubleArray, sizeof(double) * numberOfExtraFields);
			for(int i = 0; i < numberOfExtraFields; i++)
				maxFieldValues.push_back(tempDoubleArray[i]);
				
			delete [] tempDoubleArray;
			
			//read field labels
			char* tempCharArray = new char[FIELD_LABEL_LENGTH];
			for(int i = 0; i < numberOfExtraFields; i++)
			{
				fin.read(tempCharArray, FIELD_LABEL_LENGTH);
				fieldLabels.push_back(string(tempCharArray));
			}			
			delete [] tempCharArray;

			// JFD added
			if (suppressExtraFieldFlags == NULL) {
				suppressExtraFieldFlags = new int[numberOfExtraFields];
				memset(suppressExtraFieldFlags, 0, numberOfExtraFields * sizeof(int));
			}
			
			if(this->headerDataOnly)
				return;
				
			//have to put this here because some groups jam non-standard stuff in the header
			fin.seekg(this->headerLength, ios::beg);
			
			//read point data
			float* tempFloatArray = new float[this->numberOfPoints];
			//read x
			fin.read((char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
			for(int i = 0; i < this->numberOfPoints; i=i+nStride)
				xValues.push_back(tempFloatArray[i]);
			//read y
			fin.read((char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
			for(int i = 0; i < this->numberOfPoints; i=i+nStride)
				yValues.push_back(tempFloatArray[i]);
			//read z
			fin.read((char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
			for(int i = 0; i < this->numberOfPoints; i=i+nStride)
				zValues.push_back(tempFloatArray[i]);
				
			
			//read extra field values
			for(int i = 0; i < this->numberOfExtraFields; i++)
			{
				extraFields.push_back(new vector<float>);
				fin.read((char*)tempFloatArray, sizeof(float) * this->numberOfPoints);
				if (suppressExtraFieldFlags[i]) continue;

				for(int j = 0; j < this->numberOfPoints; j=j+nStride)
					extraFields[i]->push_back(tempFloatArray[j]);
			}
			delete [] tempFloatArray;
			
			fin.close();
			
		}
		catch(std::exception& e)
		{
			throw BPFFile::IOFailure("Attempt to read BPF failed when calling ifstream.read");
		}
		
	}
	else
	{
		throw BPFFile::IOFailure("Attempt to read failed because file not open.");
	}
}

bool BPFFile::xyzMinMaxMinusAircraftTrack(double& xMin, double& xMax, double& yMin, double& yMax, double& zMin, double& zMax) const
{
	xMin = yMin = zMin = DBL_MAX;
	xMax = yMax = zMax = DBL_MIN;
	unsigned int pixelFieldNum;
	try
	{
		pixelFieldNum = this->getExtraFieldNumberByName("Pixel Number");
	}
	catch(ParameterOutOfRange e)
	{
		return false;
	}
	
	for(unsigned int pt = 0; pt < this->numberOfPoints; pt++)
	{
		if((*extraFields[pixelFieldNum])[pt] != -5.0)
		{
			xMin = min(xMin, (double)xValues[pt]);
			xMax = max(xMax, (double)xValues[pt]);
			yMin = min(yMin, (double)yValues[pt]);
			yMax = max(yMax, (double)yValues[pt]);
			zMin = min(zMin, (double)zValues[pt]);
			zMax = max(zMax, (double)zValues[pt]);
		}
	}
	
	xMin += xOffset;
	xMax += xOffset;
	yMin += yOffset;
	yMax += yOffset;
	zMin += zOffset;
	zMax += zOffset;
	
	return true;	
}


#define C1WIDTH 30
#define C2WIDTH 15
#define C3WIDTH 15
#define C4WIDTH 15

ostream& operator<<(ostream& out, BPFFile& bpf)
{
	double minMaxNoAT[3][2];
	bool isAnAT = bpf.xyzMinMaxMinusAircraftTrack(minMaxNoAT[0][0], minMaxNoAT[0][1], minMaxNoAT[1][0], minMaxNoAT[1][1], minMaxNoAT[2][0], minMaxNoAT[2][1]);
	bpf.updateXYZMinsAndMaxs();
	bpf.updateFieldMinsAndMaxs();

	out << "--------------------------------------------------------------------------" << endl;
	out << "Header Length               :  " << bpf.headerLength << endl;
	out << "Version                     :  " << bpf.version << endl;
	out << "Number of Points            :  " << bpf.numberOfPoints << endl;
	out << "Number of Extra Fields      :  " << bpf.numberOfExtraFields << endl;
	out << "Interleaved                 :  " << (unsigned int)bpf.interleaved << endl;
	out << "Compressed                  :  " << (bpf.isCompressed == 0 ? "No" : "Yes") << endl;
	out << "Coordinate System           :  " << bpf.coordinateSystem << endl;
	out << "Zone                        :  " << bpf.zone << endl;
	out << "Point Spacing               :  " << bpf.pointSpacing << endl;
	out << "X Offset                    :  " << bpf.xOffset << endl;
	out << "Y Offset                    :  " << bpf.yOffset << endl;
	out << "Z Offset                    :  " << bpf.zOffset << endl;
	out.precision(20);
	out << "X Min                       :  " << (isAnAT ? minMaxNoAT[0][0] : bpf.xMin) << endl;
	out << "X Max                       :  " << (isAnAT ? minMaxNoAT[0][1] : bpf.xMax) << endl;
	out << "Y Min                       :  " << (isAnAT ? minMaxNoAT[1][0] : bpf.yMin) << endl;
	out << "Y Max                       :  " << (isAnAT ? minMaxNoAT[1][1] : bpf.yMax) << endl;
	out << "Z Min                       :  " << (isAnAT ? minMaxNoAT[2][0] : bpf.zMin) << endl;
	out << "Z Max                       :  " << (isAnAT ? minMaxNoAT[2][1] : bpf.zMax) << endl;
	out << "Aircraft Track in BPF?      :  " << (isAnAT ? "Yes" : "No") << endl;
	out << "--------------------------------------------------------------------------" << endl;
	out.width(C1WIDTH);
	out.precision(3);
	out.setf(ios::left, ios::adjustfield);
	out << "Field Label"; 
	out.width(C2WIDTH);
	out.setf(ios::left, ios::adjustfield);
	out << "Min"; 
	out.width(C3WIDTH);
	out.setf(ios::left, ios::adjustfield);
	out << "Max";
	out.width(C4WIDTH);
	out.setf(ios::left, ios::adjustfield);
	out << "Offset" << endl;
	for(int i = 0; i < bpf.numberOfExtraFields; i++)
	{
		out.width(C1WIDTH);
		out.precision(3);
		out.setf(ios::left, ios::adjustfield);
		out << bpf.fieldLabels[i];
		out.width(C2WIDTH);
		out.setf(ios::left, ios::adjustfield);
		out << bpf.minFieldValues[i];
		out.width(C3WIDTH);
		out.setf(ios::left, ios::adjustfield);
		out << bpf.maxFieldValues[i];
		out.width(C4WIDTH);
		out.setf(ios::left, ios::adjustfield);
		out << bpf.fieldOffsets[i] << endl;
	}
	
	return out;
}

bool BPFFile::isValidBPFV3File(ifstream& fin)
{
	unsigned long long actualFileSize;
	unsigned long long calculatedFileSize;
	int headerLength;
	int pointCount;
	unsigned char metaDataDims;

	fin.seekg(0, ios::end);
	actualFileSize = fin.tellg();
	
	if(actualFileSize < HEADER_SIZE_V3)
		return false;

	fin.seekg(BPFFile::HEADER_LENGTH_POS_V3, ios::beg);
	fin.read((char*)&headerLength, sizeof(int));
	fin.seekg(BPFFile::NUMBER_OF_DIMS_POS_V3, ios::beg);
	fin.read((char*)&metaDataDims, sizeof(unsigned char));
	fin.seekg(BPFFile::POINT_COUNT_POS_V3, ios::beg);
	fin.read((char*)&pointCount, 4);

	calculatedFileSize = (unsigned long long)headerLength + (unsigned long long)metaDataDims*(unsigned long long)pointCount*4;

	if(actualFileSize == calculatedFileSize)
		return true;
	else
		return false;

}

bool BPFFile::isValidBPFV1File(ifstream& fin)
{
	return this->isValidBPFV2File(fin);
}

bool BPFFile::isValidBPFV2File(ifstream& fin)
{
	unsigned long long actualFileSize;
	unsigned long long calculatedFileSize;
	int headerLength;
	int pointCount;
	int metaDataDims;
	
	fin.seekg(0, ios::end);
	actualFileSize = fin.tellg();

	if(actualFileSize < HEADER_SIZE_V2)
		return false;

	fin.seekg(0, ios::beg);
	
	//read header length
	fin.read((char*)&headerLength, sizeof(int));

	//read point count
	fin.seekg(8, ios::beg);
	fin.read((char*)&pointCount, sizeof(int));

	//read number metadata dims
	fin.seekg(12, ios::beg);
	fin.read((char*)&metaDataDims, sizeof(int));

	//calculate file size
	calculatedFileSize = (unsigned long long)headerLength + ((unsigned long long)pointCount)*12 + ((unsigned long long)metaDataDims*pointCount)*4;

	if(actualFileSize == calculatedFileSize)
		return true;
	else
		return false;
}

void BPFFile::renameExtraField(const string& curName, const string& newName)
{
	unsigned int fieldNum = this->getExtraFieldNumberByName(curName);
	fieldLabels[fieldNum] = newName;
	return;
}

int BPFFile::determineFileVersion(ifstream& fin)
{
	int versionNum = 0;
	char magicNumber[5];
	char versionStr[5];

	//reset file pointer
	fin.seekg(0, ios::beg);


	//read first 4 bytes
	fin.read((char*)magicNumber, sizeof(int));
	magicNumber[4] = '\0'; //if version 3

	//read second 4 bytes
	fin.read((char*)&versionNum, sizeof(int)); //if version 1 or 2
	memcpy((void*)versionStr, (void*)&versionNum, 4);
	versionStr[4] = '\0'; //if version 3

	if(versionNum == 1)
	{
		//validate as v1 file
		if(this->isValidBPFV1File(fin))
			return 1;
		else
			return 0;
	}
	else if (versionNum == 2)
	{
		//validate as v2 file
		if(this->isValidBPFV2File(fin))
			return 2;
		else
			return 0;
	}
	
	if(strncmp(magicNumber, "BPF!",4)==0 && strncmp(versionStr, "0003",4)==0)
	{
		return 3;
		//validate as v3 file
		if(this->isValidBPFV3File(fin))
			return 3;
		else
			return 0;
	}
	else
		return 0;
}

void BPFFile::printPoint(unsigned int n) const
{
	cout << "X			: " << this->getXValue(n) << endl;
	cout << "Y			: " << this->getYValue(n) << endl;
	cout << "Z			: " << this->getZValue(n) << endl;
	
	//print extra fields
	for(int i = 0; i < extraFields.size(); i++)
	{
		cout << this->getExtraFieldName(i) << "			: " << this->getExtraFieldValueByNumber(i, n) << endl;
	}
	return;
}

void BPFFile::writeBytesToFile(ofstream& fout, const char* buf, unsigned int bytesToWrite)
{
	//check if file is open
	if(!fout.is_open())
		return;
		
	if(this->isCompressed == 0) //no compression
	{
		fout.write(buf, bytesToWrite);
	}
	else if(this->isCompressed == 1) //quick lz
	{
		cerr << "QuickLZ writing not implemented" << endl;
		exit(1);
	}
	else if(this->isCompressed == 2) //fast lz
	{
#if defined(LIBS_FASTLZ) 
		unsigned int uncompressedSize = bytesToWrite;
		unsigned int compressedSize = bytesToWrite;
		
		//write the uncompressed size to the file
		fout.write((char*)&uncompressedSize, sizeof(unsigned int));
		
		//create compressed buffer
		char* compressedBuffer;
		int compressedBufferSize = max((int)ceil(uncompressedSize*1.05), 66); //see fast lz documentation
		compressedBuffer = new char[compressedBufferSize];
		unsigned int actualCompressedSize = 0;
		
		//compress the data if it is larger than 16 bytes (see fastlz docs)
		if(uncompressedSize > 16)
		{
			actualCompressedSize = fastlz_compress_level(2, (const void*)buf, uncompressedSize, 										(void*)compressedBuffer);
		}
		else
			actualCompressedSize = uncompressedSize;
		
		if(actualCompressedSize < uncompressedSize)
		{
			//write compressed size to file
			fout.write((char*)&actualCompressedSize, sizeof(unsigned int));
			//write compressed buffer to file
			fout.write(compressedBuffer, actualCompressedSize);
		}
		else //just write the original bytes to the file
		{
			//write compressed size to file (which in this case is the original size
			fout.write((char*)&uncompressedSize, sizeof(unsigned int));
			//write uncompressed buffer
			fout.write(buf, uncompressedSize);
		}
		
		delete [] compressedBuffer;
#else
#endif
	}
	return;
}

void BPFFile::readBytesFromFile(ifstream& fin, char* buf, unsigned int bytesToRead)
{
	//check if file is open
	if(!fin.is_open())
		return;
	
	//if not compressed, read as normal
	if(!this->isCompressed)
	{
		fin.read(buf, bytesToRead);
	}
	else if(this->isCompressed == 1) //is compressed, read with quicklz
	{
#if defined(MFC_QUICKLZ) 
		unsigned int decompressedBytesRead = 0;
		//read compressed bytes until decompressed bytes read >= bytes to Read
		while(decompressedBytesRead < bytesToRead)
		{		
			//if buffer has unread decompressed bytes left in it read those first
			if(decompressedBytesBufferPos < decompressedBytesBufferSize)
			{
				//read the lesser of the bytes in the buffer or the bytes left to read
				unsigned int numBytesLeftInBuffer = decompressedBytesBufferSize - decompressedBytesBufferPos;
				unsigned int curReadBytes = min(bytesToRead - decompressedBytesRead, numBytesLeftInBuffer);
				memcpy(buf + decompressedBytesRead,
					this->decompressedBytesBuffer + this->decompressedBytesBufferPos,
					curReadBytes);
				decompressedBytesBufferPos += curReadBytes;
				decompressedBytesRead += curReadBytes;
			}
			else //decompressed bytes buffer is empty, refill
			{
				qlz_state_decompress state;
				char* compressedBuffer = NULL;
				unsigned int compressedBufferSize = 0;
				//delete old buffer
				delete [] decompressedBytesBuffer;
				//read next byte from file, which has the size of the subsequent compressed chunk
				fin.read((char*)&compressedBufferSize, sizeof(unsigned int));
				//cout << "Compressed buffer size = " << compressedBufferSize << endl;
				//create new compressed buffer
				compressedBuffer = new char[compressedBufferSize];
				//read in compressed bytes from file
				fin.read(compressedBuffer, compressedBufferSize);
				//decompress the compressed chunk and copy it into the decompressed buffer
				//get the decompressed size, create new decompressed buffer
				decompressedBytesBufferPos = 0;
				decompressedBytesBufferSize = qlz_size_decompressed(compressedBuffer);
				decompressedBytesBuffer = new char[decompressedBytesBufferSize];
				//decompress
				qlz_decompress(compressedBuffer, (void*)decompressedBytesBuffer, &state);
				//delete compressed buffer
				delete [] compressedBuffer;
			}		
		}	
#else
#endif
	}
	else if(this->isCompressed == 2)
	{
#if defined(LIBS_FASTLZ) 
		unsigned int decompressedBytesRead = 0;
		//read compressed bytes until decompressed bytes read >= bytes to Read
		while(decompressedBytesRead < bytesToRead)
		{		
			//if buffer has unread decompressed bytes left in it read those first
			if(decompressedBytesBufferPos < decompressedBytesBufferSize)
			{
				//read the lesser of the bytes in the buffer or the bytes left to read
				unsigned int numBytesLeftInBuffer = decompressedBytesBufferSize - decompressedBytesBufferPos;
				unsigned int curReadBytes = min(bytesToRead - decompressedBytesRead, numBytesLeftInBuffer);
				memcpy(buf + decompressedBytesRead,
					this->decompressedBytesBuffer + this->decompressedBytesBufferPos,
					curReadBytes);
				decompressedBytesBufferPos += curReadBytes;
				decompressedBytesRead += curReadBytes;
			}
			else //decompressed bytes buffer is empty, refill
			{
				char* compressedBuffer = NULL;
				unsigned int compressedBufferSize = 0;
				unsigned int uncompressedBufferSize = 0;
				//delete old buffer
				delete [] decompressedBytesBuffer;
				//read next 4 bytes, is the uncompressed size of the subsequent compressed chunk
				fin.read((char*)&uncompressedBufferSize,sizeof(unsigned int));
				decompressedBytesBufferSize = uncompressedBufferSize;
				//read next 4 bytes from file, which has the size of the subsequent compressed chunk
				fin.read((char*)&compressedBufferSize, sizeof(unsigned int));
				//cout << "Uncompressed buffer size = " << uncompressedBufferSize << endl;
				//cout << "Compressed buffer size = " << compressedBufferSize << endl;
				//create new decompressed bytes buffer
				decompressedBytesBufferPos = 0;
				decompressedBytesBuffer = new char[uncompressedBufferSize];
				if(uncompressedBufferSize > compressedBufferSize) //data is indeed compressed
				{
					//create new compressed buffer
					compressedBuffer = new char[compressedBufferSize];
					//read in compressed bytes from file
					fin.read(compressedBuffer, compressedBufferSize);
					
					fastlz_decompress(compressedBuffer, compressedBufferSize,
								decompressedBytesBuffer, uncompressedBufferSize);
					//delete compressed buffer
					delete [] compressedBuffer;
					compressedBuffer = NULL;
				}
				else if(uncompressedBufferSize == compressedBufferSize) //data is not compressed
				{
					//just copy bytes directly from file into decompressed buffer
					fin.read(decompressedBytesBuffer, uncompressedBufferSize);			
				}
				else
				{
					//error, compressed size should have never been larger than uncompressed size
					cerr << "ERROR: decompresesd size less than compressed size" << endl;
					exit(1);
				}
			}		
		}	
#else
#endif
	}
}

void BPFFile::setXYZOffset(const double& xoff, const double& yoff, const double& zoff)
{
	this->xOffset = xoff;
	this->yOffset = yoff;
	this->zOffset = zoff;
	this->dirtyMinsAndMaxs = true;
	return;
}

void BPFFile::setCompression(const unsigned char val)
{
	this->isCompressed = val;
}

void BPFFile::setExtraFieldValue(unsigned int fieldIndex, unsigned int pointIndex, const double& val)
{
	(*this->extraFields[fieldIndex])[pointIndex] = val - fieldOffsets[fieldIndex];
	dirtyFieldMinsAndMaxs = true;
	return;
}

//changes the offset value and changes the point values to compensate
//the result is that the actual point values do not change at all
void BPFFile::changeOffsetX(const double& off)
{
	double offsetDiff = off - this->xOffset;
	
	//add the offset diff to each coordinate
	vector<float>::iterator itr = xValues.begin();
	vector<float>::iterator endItr = xValues.end();
	
	while(itr != endItr)
	{
		(*itr) = (float)((double)(*itr) - offsetDiff);
		itr++;
	}
	this->xOffset = off;
	return;
}

void BPFFile::changeOffsetY(const double& off)
{
	double offsetDiff = off - this->yOffset;
	
	//add the offset diff to each coordinate
	vector<float>::iterator itr = yValues.begin();
	vector<float>::iterator endItr = yValues.end();
	
	while(itr != endItr)
	{
		(*itr) = (float)((double)(*itr) - offsetDiff);
		itr++;
	}
	this->yOffset = off;
	return;
}
void BPFFile::changeOffsetZ(const double& off)
{
	double offsetDiff = off - this->zOffset;
	
	//add the offset diff to each coordinate
	vector<float>::iterator itr = zValues.begin();
	vector<float>::iterator endItr = zValues.end();
	
	while(itr != endItr)
	{
		(*itr) = (float)((double)(*itr) - offsetDiff);
		itr++;
	}
	this->zOffset = off;
	return;
}

void BPFFile::changeOffsetEF(const double& off, const string& fieldName)
{
	unsigned int efi = this->getExtraFieldNumberByName(fieldName);
	
	double offsetDiff = off - fieldOffsets[efi];
	
	//add the offset diff to each coordinate
	vector<float>::iterator itr = (*extraFields[efi]).begin();
	vector<float>::iterator endItr = (*extraFields[efi]).end();
	
	while(itr != endItr)
	{
		(*itr) = (float)((double)(*itr) - offsetDiff);
		itr++;
	}
	fieldOffsets[efi] = off;
	return;
}

bool BPFFile::extractPointsFilterByExtraField(const unsigned int fieldNumber, const double& lowerBound, const double& upperBound,
	                                     vector<double>& x, vector<double>& y, vector<double>& z) const
{
	
	if(upperBound < lowerBound)
	{
		throw(ParameterOutOfRange("upper bound less than lower bound"));
	}
	
	for(unsigned int pt = 0; pt < this->numberOfPoints; pt++)
	{
		double efv = this->getExtraFieldValueByNumber(fieldNumber, pt);
		if(efv >= lowerBound && efv <= upperBound)
		{
			x.push_back(this->getXValue(pt));
			y.push_back(this->getYValue(pt));
			z.push_back(this->getZValue(pt));
		}
	}
	
	return true;
}

bool BPFFile::extractPointsFilterByExtraField(const unsigned int fieldNumber, const double& lowerBound, const double& upperBound, set<unsigned int>& indices) const
{
	if(upperBound < lowerBound)
	{
		throw(ParameterOutOfRange("upper bound less than lower bound"));
	}
	
	for(unsigned int pt = 0; pt < this->numberOfPoints; pt++)
	{
		double efv = this->getExtraFieldValueByNumber(fieldNumber, pt);
		if(efv >= lowerBound && efv <= upperBound)
		{
			indices.insert(pt);
		}
	}
	
	return true;	        	
	
}

bool BPFFile::extractExtraFieldFilterByExtraField(const unsigned int keyFieldNumber, const double& lowerBound, const double& upperBound,
	                                     vector<double>& ef, const unsigned int valueFieldNumber) const
{
	
	if(upperBound < lowerBound)
	{
		throw(ParameterOutOfRange("upper bound less than lower bound"));
	}
	
	for(unsigned int pt = 0; pt < this->numberOfPoints; pt++)
	{
		double efv = this->getExtraFieldValueByNumber(keyFieldNumber, pt);
		if(efv >= lowerBound && efv <= upperBound)
		{
			ef.push_back(this->getExtraFieldValueByNumber(valueFieldNumber, pt));
		}
	}
	
	return true;
}

                                 

bool BPFFile::extractUniqueValuesFromExtraField(list<double>& vals, unsigned int fieldNumber) const
{
	if(fieldNumber >= extraFields.size())
		throw(ParameterOutOfRange("Field number not valid"));
		
	//copy extra field values to list
	double fieldOffsetVal = fieldOffsets[fieldNumber];
	vector<float>::const_iterator itr = (*extraFields[fieldNumber]).begin();
	while(itr != (*extraFields[fieldNumber]).end())
	{
		vals.push_back(*itr + fieldOffsetVal);		
		itr++;
	}
	
	//run unique on the list (running it first shortens the list some so the sort doesn't take so long)
	vals.unique();
	//sort the list
	vals.sort();	
	//run unique again
	vals.unique();
	return true;		
	
}

void BPFFile::rotatePoints(const double& theta, const double& axisX, const double& axisY, const double& axisZ)
{
#if defined(LIBS_BOOST) 
	//the axis of rotation
	Quat r(0, axisX, axisY, axisZ);
	//normalize r
	Quat rnorm = r / boost::math::abs(r);
	
	//create rotation quaternion
	// [cos(theta/2)  Rx*sin(theta/2)  Ry*sin(theta/2)  Rz*sin(theta/2)]
	Quat Qr = Quat(cos(theta/2.0), 0, 0, 0) + (rnorm * sin(theta/2.0));
	//Qrconj
	Quat QrConj = boost::math::conj(Qr);
	
	//rotate each point
	for(unsigned int pt = 0; pt < this->numberOfPoints; pt++)
	{
		Quat point(0, (double)xValues[pt], (double)yValues[pt], (double)zValues[pt]);
		Quat pointRotated = Qr * point * QrConj;
		//overwrite original point value with new rotated value
		xValues[pt] = pointRotated.R_component_2();
		yValues[pt] = pointRotated.R_component_3();
		zValues[pt] = pointRotated.R_component_4();
	}
	
	
	//rotate offset values
	Quat point(0, xOffset, yOffset, zOffset);
	Quat pointRotated = Qr * point * QrConj;
	//overwrite original point value with new rotated value
	xOffset = pointRotated.R_component_2();
	yOffset = pointRotated.R_component_3();
	zOffset = pointRotated.R_component_4();
	
	dirtyMinsAndMaxs = true;
	
#else
#endif
	return;
}

void BPFFile::clipPoints(set<unsigned int>& keepers)
{
	//clip all of the storage arrays
	vector<float> temp;
	set<unsigned int>::const_iterator itr, endItr = keepers.end();
	
	for(itr = keepers.begin(); itr != endItr; itr++)
	{
		temp.push_back(xValues[*itr]);				
	}
	xValues.clear();
	xValues = temp;
	temp.clear();
	
	for(itr = keepers.begin(); itr != endItr; itr++)
	{
		temp.push_back(yValues[*itr]);		
	}
	yValues.clear();
	yValues = temp;
	temp.clear();
	
	for(itr = keepers.begin(); itr != endItr; itr++)
	{
		temp.push_back(zValues[*itr]);		
	}
	zValues.clear();
	zValues = temp;
	temp.clear();
	
	for(unsigned int field = 0; field < extraFields.size(); field++)
	{
		for(itr = keepers.begin(); itr != endItr; itr++)
		{
			temp.push_back((*extraFields[field])[*itr]);		
		}
		(*extraFields[field]).clear();
		(*extraFields[field]) = temp;
		temp.clear();
	}
	
	updateNumberOfPoints();
	updateXYZMinsAndMaxs();
	updateFieldMinsAndMaxs();
	
	return;
}

void BPFFile::clipPoints(const double& xMinClip, const double& xMaxClip, const double& yMinClip,
					 const double& yMaxClip, const double& zMinClip, const double& zMaxClip)
{
	vector<unsigned int> keepers; //indices of points to keep
	
	//offset the clipping values
	float xMinClipOff = xMinClip - xOffset;
	float xMaxClipOff = xMaxClip - xOffset;
	float yMinClipOff = yMinClip - yOffset;
	float yMaxClipOff = yMaxClip - yOffset;
	float zMinClipOff = zMinClip - zOffset;
	float zMaxClipOff = zMaxClip - zOffset;
	
	//lets find those points to keep
	unsigned int numPoints = xValues.size();
	
	for(unsigned int i = 0; i < numPoints; i++)
	{
		if(xValues[i] >= xMinClipOff &&
			xValues[i] <= xMaxClipOff &&
			yValues[i] >= yMinClipOff &&
			yValues[i] <= yMaxClipOff &&
			zValues[i] >= zMinClipOff &&
			zValues[i] <= zMaxClipOff)
		keepers.push_back(i);
	}
	
	//clip all of the storage arrays
	vector<float> temp;
	
	for(unsigned int i = 0; i < keepers.size(); i++)
	{
		temp.push_back(xValues[keepers[i]]);				
	}
	xValues.clear();
	xValues = temp;
	temp.clear();
	
	for(unsigned int i = 0; i < keepers.size(); i++)
	{
		temp.push_back(yValues[keepers[i]]);		
	}
	yValues.clear();
	yValues = temp;
	temp.clear();
	
	for(unsigned int i = 0; i < keepers.size(); i++)
	{
		temp.push_back(zValues[keepers[i]]);		
	}
	zValues.clear();
	zValues = temp;
	temp.clear();
	
	for(unsigned int field = 0; field < extraFields.size(); field++)
	{
		for(unsigned int i = 0; i < keepers.size(); i++)
		{
			temp.push_back((*extraFields[field])[keepers[i]]);		
		}
		(*extraFields[field]).clear();
		(*extraFields[field]) = temp;
		temp.clear();
	}
	
	updateNumberOfPoints();
	updateXYZMinsAndMaxs();
	updateFieldMinsAndMaxs();
	
	return;
}

void BPFFile::getPointsInRange(double& xMin, double& xMax, double& yMin, double& yMax, double& zMin, double& zMax, set<unsigned int>& indices) const
{
	//offset the clipping values
	float xMinClipOff = xMin - xOffset;
	float xMaxClipOff = xMax - xOffset;
	float yMinClipOff = yMin - yOffset;
	float yMaxClipOff = yMax - yOffset;
	float zMinClipOff = zMin - zOffset;
	float zMaxClipOff = zMax - zOffset;
	
	//lets find those points to keep
	unsigned int numPoints = xValues.size();
	
	for(unsigned int i = 0; i < numPoints; i++)
	{
		if(xValues[i] >= xMinClipOff &&
			xValues[i] <= xMaxClipOff &&
			yValues[i] >= yMinClipOff &&
			yValues[i] <= yMaxClipOff &&
			zValues[i] >= zMinClipOff &&
			zValues[i] <= zMaxClipOff)
		indices.insert(i);
	}
}

void BPFFile::clipPointsInRangeByExtraFieldValue(const string& extraFieldName, const double& lowerBound, const double& upperBound)
{
	this->clipPointsInRangeByExtraFieldValue(this->getExtraFieldNumberByName(extraFieldName), lowerBound, upperBound);
}

void BPFFile::clipPointsInRangeByExtraFieldValue(const unsigned int& extraFieldNum, const double& lowerBound, const double& upperBound)
{
	vector<unsigned int> keepers; //indices of points to keep
	
	//offset the clipping values
	float lb = lowerBound - fieldOffsets[extraFieldNum];
	float ub = upperBound - fieldOffsets[extraFieldNum];
		
	//lets find those points to keep
	unsigned int numPoints = xValues.size();
	
	for(unsigned int i = 0; i < numPoints; i++)
	{
		float value = (*extraFields[extraFieldNum])[i];
		if( value < lb || value > ub)
		keepers.push_back(i);
	}
	
	//clip all of the storage arrays
	vector<float> temp;
	
	for(unsigned int i = 0; i < keepers.size(); i++)
	{
		temp.push_back(xValues[keepers[i]]);				
	}
	xValues.clear();
	xValues = temp;
	temp.clear();
	
	for(unsigned int i = 0; i < keepers.size(); i++)
	{
		temp.push_back(yValues[keepers[i]]);		
	}
	yValues.clear();
	yValues = temp;
	temp.clear();
	
	for(unsigned int i = 0; i < keepers.size(); i++)
	{
		temp.push_back(zValues[keepers[i]]);		
	}
	zValues.clear();
	zValues = temp;
	temp.clear();
	
	for(unsigned int field = 0; field < extraFields.size(); field++)
	{
		for(unsigned int i = 0; i < keepers.size(); i++)
		{
			temp.push_back((*extraFields[field])[keepers[i]]);		
		}
		(*extraFields[field]).clear();
		(*extraFields[field]) = temp;
		temp.clear();
	}
	
	updateNumberOfPoints();
	dirtyMinsAndMaxs = true;
	updateXYZMinsAndMaxs();
	dirtyFieldMinsAndMaxs = true;
	updateFieldMinsAndMaxs();
	
	return;
}

double BPFFile::getExtraFieldMean(const string& name) const
{
	int efi = this->getExtraFieldNumberByName(name);
	
	return this->getExtraFieldMean(efi);
}

double BPFFile::getExtraFieldMean(const int fieldNum) const
{
	//find mean of extra field fieldNum
	vector<float>* ef = extraFields[fieldNum];
	
	vector<float>::const_iterator itr = ef->begin(), end = ef->end();
	double sum = 0.0;
	while(itr != end)
	{
		sum += (double)(*itr);
		itr++;
	}
	
	double average = sum / ef->size();
	
	return average + fieldOffsets[fieldNum];
}

double BPFFile::getXMean() const
{
	double sum = 0.0;
	for(vector<float>::const_iterator itr = xValues.begin(); itr != xValues.end(); itr++)
	{
		sum += (double)*itr;
	}
	
	return (sum / xValues.size()) + xOffset;
}

double BPFFile::getYMean() const
{
	double sum = 0.0;
	for(vector<float>::const_iterator itr = yValues.begin(); itr != yValues.end(); itr++)
	{
		sum += (double)*itr;
	}
	
	return (sum / yValues.size()) + yOffset;
}

double BPFFile::getZMean() const
{
	double sum = 0.0;
	for(vector<float>::const_iterator itr = zValues.begin(); itr != zValues.end(); itr++)
	{
		sum += (double)*itr;
	}
	
	return (sum / zValues.size()) + zOffset;
}

multimap<float, unsigned int>* BPFFile::partitionPointsByExtraFieldOffset(const string& fieldName) const
{
	return partitionPointsByExtraFieldOffset(this->getExtraFieldNumberByName(fieldName));
}

multimap<float, unsigned int>* BPFFile::partitionPointsByExtraFieldOffset(const unsigned int fieldIndex) const
{
	multimap<float, unsigned int>* pMap = new multimap<float, unsigned int>;
	multimap<float, unsigned int>::iterator prev = pMap->begin();
	
	for(unsigned int i = 0; i < this->numberOfPoints; i++)
	{
		prev = pMap->insert(prev, pair<float,unsigned int>((*extraFields[fieldIndex])[i], i));
	}
	
	return pMap;
}

multimap<double, unsigned int>* BPFFile::partitionPointsByExtraField(const string& fieldName) const
{
	return partitionPointsByExtraField(this->getExtraFieldNumberByName(fieldName));
}

multimap<double, unsigned int>* BPFFile::partitionPointsByExtraField(const unsigned int fieldIndex) const
{
	multimap<double, unsigned int>* pMap = new multimap<double, unsigned int>;
	double offset = fieldOffsets[fieldIndex];
	multimap<double, unsigned int>::iterator prev = pMap->begin();
	
	for(unsigned int i = 0; i < this->numberOfPoints; i++)
	{
		prev = pMap->insert(prev, pair<double,unsigned int>((*extraFields[fieldIndex])[i] + offset, i));
	}
	
	return pMap;
}


