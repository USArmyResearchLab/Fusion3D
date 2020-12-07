#ifndef _draw_manager_class_h
#define _draw_manager_class_h
#ifdef __cplusplus

/**
Manages drawing overlays on the map.

Manages drawing overlays on the map.\n
Handles multiple lines and multiple points.\n

Files are written and read to KML only.  The class kml_class is used rether than the general OGR classes because it is envisioned
that one will draw very complex structures that are too complex for OGR.\n\n

Altitude:  By default, as each point is digitized, an absolute altitude is calculated from the map and assigned to the point.
Features are saved with these absolute altitudes.
The user can enter an altitude.  If it is absolute, this value is substituted for the map altitude.
If it is relative, then that value is added as an offset to the map altitude.\n\n
*/
class draw_manager_class:public atrlab_manager_class{
   protected:
	  int action_current;		///< current editing action (action on middle-click)
								///<  -99	= manager not initialized
								///< 	0  = no action
								///< 	1  = digitizing -- add point to line
								///< 	2  = digitizing -- move point
								///< 	3  = digitizing -- del point
								///< 	4  = selected feature -- mv feature
								///< 	5  = selected feature -- del feature
								///< 	6  = digitizing -- add circle
								///<	7  = Select a line/point (current, highlighted)
	  int initTreeFlag;			///< 1=tree initialized, 0=not init
	  int digDashLineFlag;		///< 0=digitize line as solid, 1=dashed
	  int pointDrawPointFlag;	///< temp storage for current point -- 0 dont draw point, 1 if draw point;
	  string pointNameString;	///< temp storage for current point -- string for name ("" if no name)
	  string pointSymbolString;	///< temp storage for current point -- string for symbol ("" if no symbol)

	  int currentFeatureType;	///< Current feature -- -99 for nothing, 0 for line, 1 for polygon, 2 for circle, 3 for point
	  int currentFeature;		///< Current feature -- line no. for lines, point no. for points

	  int altAbsoluteFlag;		///< Altitude -- 1 for absolute altitudes/elevations of each digitized point (default), 0 for relative to ground
	  int altUserFlag;			///< Altitude -- 1 iff user specifies altitude (other than default where points are put at ground level)
	  float altAdjust;			///< Altitude -- if absolute mode, absolute altitude; if relative to ground, offset from ground

	  std::vector<int>     moverILine;		///< MoverSymbol -- line no.
	  std::vector<string>  moverSymbolName;	///< MoverSymbol -- Symbol name

	  float d_above_ground;		///< Distance above local ground elevation to draw track (default 1.0)
      float d_thresh;			///< When clicking on a point, must be within this threshold
	  float LODThresholdDist;	///< LOD -- threshold distance of camera from scene to switch from most-detailed to second most
	  int outputSymbolWidth;	///< Desired size of drawn symbols in pixels

	  float red_normal, grn_normal, blu_normal;		///< Draw colors -- normal lines/points
	  float red_hilite, grn_hilite, blu_hilite;		///< Draw colors -- hilited (selected) line/point	

	  draw_data_inv_class *draw_data;

	  // Protected methods
      int reset_all();
	  int get_closest_point(float xMouse, float yMouse, float d_thresh, int &iline, int &ipt);
      
   public:
      draw_manager_class();
      ~draw_manager_class();
      
      int read_tagged(const char* filename);
      int read_file(string sfilename) override;
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _draw_manager_class_ */
