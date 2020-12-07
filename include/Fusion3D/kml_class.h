#ifndef _kml_class_h_
#define _kml_class_h_	
#ifdef __cplusplus

/**
Reads and writes KML files based on the libkml library.

This class is based on libkml and is designed to read and write more complex kml structures that cant be addressed by the more general GDAL library.
GDAL wont handle all KML constructs and also is awkward as it requires mapping KML structures into completely different GDAL/OGR ones.\n

The class uses draw_data_class to store the data for reading or writing.\n

I have tested with overlays I have drawn containing points, lines and symbols.
Also with sensor inputs from the India program consisting of points and sensor bounding boxes, both having times and names (not drawn but determining color).
Also with QT-produced (BuckEye) tile outlines using LatLonAltBox.
Also with outlines of Nangahar, which I am not sure where they were generated.
Also with vehicle tracks generated from GPS.\n


*/
class kml_class:public vector_layer_class{
   
   private:
	   std::vector<string> stylelName;					///< Styles -- Per style -- name
	   std::vector<int> StyleSymbolNameIndex;			///< Styles -- Per style -- index of mil symbol associated with it (negative for no symbol)

#if defined(LIBS_KML)
	   KmlFactory* factory;
	   FolderPtr folder;
	   DocumentPtr doc;

	   // Private methods
	   int parsePoint     (kmldom::PointPtr point,            float timePastMidnight, string placemarkName, int iSymbol, int extendeddataDrawPtFlag);
	   int parseLineString(kmldom::LineStringPtr lineStringn, float timePastMidnight, string placemarkName, int extendeddataDashlineFlag);
	   int parseLinearRing(kmldom::LinearRingPtr linearRing,  float timePastMidnight, string placemarkName);
	   int parsePolygon   (kmldom::PolygonPtr polygon,        float timePastMidnight, string placemarkName);
	   int parseGxTrack   (kmldom::GxTrackPtr gxTrack, string placemarkName, int extendeddataDashlineFlag);

	   int write_styles();
	   int find_substring_within(std::string stringIn, std::string &stringOut, std::string stringBeg, std::string stringEnd, int &n_offset);
#endif

   public:
      kml_class();
      ~kml_class();
      
	  int write_file(string sfilename) override;
	  int read_file(string sfilename) override;
	  int write_file_groundOverlay_LatLonAltBox(string sfilename, double north, double south, double east, double west);
	  int write_file_groundOverlay_LatLonBox(string sfilename, double north, double south, double east, double west);
};

#endif /* __cplusplus */
#endif /* _kml_class_h_ */
