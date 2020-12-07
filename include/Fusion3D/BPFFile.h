#ifndef BPFFILE_H
#define BPFFILE_H

#include <set>


#if defined(MFC_QUICKLZ) 
	#include "quicklz.h"
#endif
#if defined(LIBS_FASTLZ) 
	#include "fastlz.h"
#endif

#if defined(LIBS_BOOST) 
	#include "boost/math/quaternion.hpp"

	typedef boost::math::quaternion<double> Quat;
#endif

using namespace std;

/// @cond		Exclude from Doxygen
class BPFFileException
{
	public:
	string e;
	BPFFileException(const string& error) : e(error) {}
	string& what() throw() {return e;}
};
/// @endcond	End Doxygen exclusion

/**
This class reads, filters and writes lidar point cloud files in .bpf format.
It was provided by M. Brandon Cox - Booz Allen Hamilton - cox_michael@bah.com.

*/
class BPFFile
{
	private:
	
	const int STATIC_HEADER_SIZE_IN_BYTES;
	
	bool dirtyMinsAndMaxs;
	bool dirtyFieldMinsAndMaxs;
	bool headerDataOnly;
	
	
	//Static header
	int headerLength;			//header length in bytes (including variable length section)
	int version;				//file format version number, currently 1
	int numberOfPoints;			//number of XYZ points in file
	int numberOfExtraFields;		//number of extra fields per XYZ point
	int coordinateSystem;		//1= UTM; no other coordinate systems yet used
	int zone;					// UTM zone, if applicable
	float pointSpacing;			//average point spacing, not required to be populated
	double xOffset;			//offset to be added to all x values
	double yOffset;			//offset to be added to all y values
	double zOffset;			//offset to be added to all z values
	double xMin;				//minimum x value in entire file
	double xMax;				//maximum x value in entire file
	double yMin;				//minimum y value in entire file
	double yMax;				//maximum y value in entire file
	double zMin;				//minimum z value in entire file
	double zMax;				//maximum z value in entire file
	unsigned char interleaved;  //indicates whether point data is interleaved (BPF1 = 0, BPF2 = 1, BPF3 = 0 or 1)
	unsigned char isCompressed;
	double transformMatrix[4][4];
	double startTime;
	double endTime;

	int *suppressExtraFieldFlags;	// JFD fix memory overflows -- 1 iff suppress reading of extra field
	int nStride;					// JFD fix memory overflows -- stride thru all data fields (ratio of pts in file to pts stored)
	
	
	//variable length header section
	vector<double> fieldOffsets;	//offsets to be added to extra field values (size = numberOfExtraFields)
	vector<double> minFieldValues;	//minimum of extra field value in file (size = numberOfExtraFields)
	vector<double> maxFieldValues;	//maximum of extra field value in file (size = numberOfExtraFields)
	vector<string> fieldLabels;	//max 32 chars, descriptive text labels for extra fields
	
	//data section
	vector<float> xValues;			//x values
	vector<float> yValues;			//y values
	vector<float> zValues;			//z values
	vector<vector<float>*> extraFields;  //extra field values
	
	//compression related
	char* decompressedBytesBuffer;
	unsigned int decompressedBytesBufferPos;
	unsigned int decompressedBytesBufferSize;
	
	//calculates the length of the header, which is the size of the static and variable length header section in bytes.
	//if the variable length section is modified after calling this function, one would need to call it again to update
	//this field.
	int calculateHeaderLength();
	void updateHeaderLength();
	void updateNumberOfPoints();
	void updateNumberOfExtraFields();
	void updateXYZMinsAndMaxs();
	void updateFieldMinsAndMaxs();
	void readBPFV3Header(ifstream& fin);
	void readBPFV1Header(ifstream& fin);
	
	void readBytesFromFile(ifstream& fin, char* buf, unsigned int bytesToRead);
	
	public:

	static const int FIELD_LABEL_LENGTH;
	static const int HEADER_SIZE_V1;
	static const int HEADER_SIZE_V2;
	static const int HEADER_SIZE_V3;
	static const int HEADER_LENGTH_POS_V3;
	static const int NUMBER_OF_DIMS_POS_V3;
	static const int POINT_COUNT_POS_V3;
	static const char* ASCII_MAGIC_NUMBER_STRING;
	static const unsigned INTERLEAVED_POINTS_PER_COMPRESSED_BLOCK;
	
/// @cond		Exclude from Doxygen
	class IOFailure : public BPFFileException
	{
		public:
		IOFailure(const string& error) : BPFFileException(error) {}
	};
	
	class InvalidBPFData : public BPFFileException
	{
		public:
		InvalidBPFData(const string& error) : BPFFileException(error) {}	
	};
	
	class ParameterOutOfRange : public BPFFileException
	{
		public:
		ParameterOutOfRange(const string& error) : BPFFileException(error) {}
	};
/// @endcond	End Doxygen exclusion
	
	BPFFile();
	~BPFFile();
	
	//accessors
	int getHeaderLength();
	int getVersion() const {return version;}
	int getNumberOfPoints();
	int getNumberOfExtraFields();
	int getCoordinateSystem() const {return coordinateSystem;}
	int getZone() const {return zone;}
	double getXValue(int n) const {return ((double)xValues[n])+xOffset;}
	double getYValue(int n) const {return ((double)yValues[n])+yOffset;}
	double getZValue(int n) const {return ((double)zValues[n])+zOffset;}
	double getXMean() const;
	double getYMean() const;
	double getZMean() const;
	double getStartTime() const {return startTime;}
	double getEndTime() const {return endTime;}
	float getXValueNoOffset(int n) const {return (xValues[n]);}
	float getYValueNoOffset(int n) const {return (yValues[n]);}
	float getZValueNoOffset(int n) const {return (zValues[n]);}
	string getExtraFieldLabel(unsigned int n) const {if(n < fieldLabels.size()) return fieldLabels[n]; else return "";}
	bool getExtraFieldValueByName(const int n, const string& name, double& valueOut);
	double getExtraFieldValueByNumber(const int fieldNum, const int pointNum) const;
	float getExtraFieldValueByNumberNoOffset(const int fieldNum, const int pointNum) const;
	double getExtraFieldOffsetByNumber(const int n) const {return fieldOffsets[n];}
	string getExtraFieldName(const int fieldNum) const; //zero-based indexing
	unsigned int getExtraFieldNumberByName(const string& fieldName) const;
	double getExtraFieldMean(const string& name) const;
	double getExtraFieldMean(const int fieldNum) const;
	float getPointSpacing() const {return this->pointSpacing;}
	unsigned char getIsCompressed() const {return this->isCompressed;}
	unsigned char getInterleaved() const {return this->interleaved;}
	double getXOffset() const {return xOffset;}
	double getYOffset() const {return yOffset;}
	double getZOffset() const {return zOffset;}
	double getXMin();
	double getXMax();
	double getYMin();
	double getYMax();
	double getZMin();
	double getZMax();
	double getFieldMin(int n);
	double getFieldMax(int n);
	void writeBPFFile(ofstream& fout);
	void writeBPFFileV3(ofstream& fout);
	bool writeBPFFileV3(const string& filename);
	void writeBytesToFile(ofstream& fout, const char* buf, unsigned int bytesToWrite);
	
	bool extractPointsFilterByExtraField(const unsigned int fieldNumber, const double& lowerBound, const double& upperBound,
	                                     vector<double>& x, vector<double>& y, vector<double>& z) const;
	bool extractPointsFilterByExtraField(const unsigned int fieldNumber, const double& lowerBound, const double& upperBound,
	                                     set<unsigned int>& indices) const;
	bool extractExtraFieldFilterByExtraField(const unsigned int keyFieldNumber, const double& lowerBound, const double& upperBound,
	                                     vector<double>& ef, const unsigned int valueFieldNumber) const;
	multimap<double, unsigned int>* partitionPointsByExtraField(const unsigned int fieldIndex) const;
	multimap<double, unsigned int>* partitionPointsByExtraField(const string& fieldName) const;
	multimap<float, unsigned int>* partitionPointsByExtraFieldOffset(const unsigned int fieldIndex) const;
	multimap<float, unsigned int>* partitionPointsByExtraFieldOffset(const string& fieldName) const;
	void getPointsInRange(double& xMin, double& xMax, double& yMin, double& yMax, double& zMin, double& zMax, set<unsigned int>& indices) const;
	                                     
	//returns a sorted-ascending, unique-valued list in "vals" extracted from the extra field with "fieldNumber"
	//returns true if successful.
	//throws a ParameterOutOfRange exception if fieldNumber is not valid.
	bool extractUniqueValuesFromExtraField(list<double>& vals, unsigned int fieldNumber) const;

	int determineFileVersion(ifstream& fin);
	bool isValidBPFV2File(ifstream& fin);
	bool isValidBPFV1File(ifstream& fin);
	bool isValidBPFV3File(ifstream& fin);
	bool xyzMinMaxMinusAircraftTrack(double& xMin, double& xMax, double& yMin, double& yMax, double& zMin, double& zMax) const;

	bool mergeWithBPFFile(BPFFile& newData);
	
	//modifiers
	bool readBPFFile(const string& filename, bool readHeaderOnly = false);
	void readBPFFile(ifstream& fin, bool readHeaderOnly = false);
	void readBPFV1File(ifstream& fin);
	void readBPFV3File(ifstream& fin);
	void setCompression(const unsigned char val);
	void setVersion(int ver) {this->version = ver;}
	void setInterleaved(unsigned char val) {this->interleaved = val;} //0 = non-interleaved, 1=interleaved
	void setCoordinateSystem(int coordSys) {this->coordinateSystem = coordSys;}
	void setZone(int zone) {this->zone = zone;}
	void setStartTime(const double& val) {this->startTime = val;}
	void setPointSpacing(const float val) {this->pointSpacing = val;}
	void setEndTime(const double& val) {this->endTime = val;}
	void setXYZCoordinates(const vector<float>& X, const vector<float>& Y, const vector<float>& Z);
	void addExtraFieldData(const string& label, vector<float>& extraField, const double& offset=0);
	//NOTE!!  After removing an extra field, your extra field indices might become invalid.  Look them up again by name!
	void removeExtraField(const string& label);
	void renameExtraField(const string& curName, const string& newName);
	void setExtraFieldValue(unsigned int fieldIndex, unsigned int pointIndex, const double& val);
	void readBPFHeader(ifstream& fin);
	void setXYZOffset(const double& xoff, const double& yoff, const double& zoff);
	void changeOffsetX(const double& off);
	void changeOffsetY(const double& off);
	void changeOffsetZ(const double& off);
	void changeOffsetEF(const double& off, const string& fieldName);
	//rotates the points in the BPFFile object by theta RADIANS around the axis defined
	//by the vector <axisX, axisY, axisZ>
	//uses quaternions to perform the rotation
	void rotatePoints(const double& theta, const double& axisX, const double& axisY, const double& axisZ);
	void clipPoints(const double& xMinClip, const double& xMaxClip, const double& yMinClip,
					 const double& yMaxClip, const double& zMinClip, const double& zMaxClip);
	//clip (keep) the points specified (by index) in keepers
	void clipPoints(set<unsigned int>& keepers);
	//removes all points with extra field (chosen by extraFieldNum) with values between lower and upper bound
	void clipPointsInRangeByExtraFieldValue(const unsigned int& extraFieldNum, const double& lowerBound, const double& upperBound);
	void clipPointsInRangeByExtraFieldValue(const string& extraFieldName, const double& lowerBound, const double& upperBound);
	
	
	void printPoint(unsigned int n) const;

	int setNStride(int n);
	int suppressExtraField(int ifield);
	
	friend ostream& operator<<(ostream& out, BPFFile& bpf);

};

#endif
