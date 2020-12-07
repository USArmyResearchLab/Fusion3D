#ifndef _mensuration_manager_class_h_
#define _mensuration_manager_class_h_	
#ifdef __cplusplus

/**
Manages all mensuration calculations.

This class has 2 functions depending on which is chosen from the menu.\n
\n
The first is to measure the distance in meters between two clicked points.
Both points are defined by clicking on them with the middle mouse.
(This functionality is duplicated on CNTRL-left mouse and the lefthand button on the SpaceNavigator.)
When the first point is clicked, the elevation of the point is displayed.
When the second point is clicked, the horizontal distance 'dl' and the vertical distance 'dh' are displayed.\n
\n
The class also gives location stats for a single point, picked with the middle mouse.
Northing, Easting, Lat, Lon and Elevation are given for each point clicked.

*/
class mensuration_manager_class:public atrlab_manager_class{
   protected:
      int action_current;	///< -99=off, 0=dist-pend 1st dig, 1=dist-pend 1st draw, 2=dist-pend 2nd dig, 3=dist-pend 2nd draw, 4=pt-pend dig, 5=pt-pend draw
      float xpt1, ypt1;		///< Coordinates of first point
      float xpt2, ypt2;		///< Coordinates of second point
      
   public:
      mensuration_manager_class();
      virtual ~mensuration_manager_class();
      
      int read_tagged(const char* filename);
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _mensuration_manager_class_h_ */
