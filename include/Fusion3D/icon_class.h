#ifndef _icon_class_h_
#define _icon_class_h_	
#ifdef __cplusplus

/**
Stores all relevent information about an icon -- a point location on a map.

Stored information includes the icon location (northing, easting, elevation) in UTM m, a sequence number, and information about associated target, SAR image and camera.
Target information is the target ID and an azimuth angle for the target.
SAR image information is the image ID number, the number of rows and columns, and the elevation, azimuth and squint angles.
Camera information is the location and the azimuth and elevation angles and a flag.
Setting this flag to 1 causes the user viewpoint to switch to this position and angle when the icon is picked.


*/
class icon_class{
   private:
      
   public:
      int seqno;			/// Sequence no of icon
      float east;			/// Location East  of ref point in m
      float north;			/// Location North of ref point in m
      float height;			/// Location height above mean ground plane in m
      float target_az;		/// Azimuth angle of target in deg ccw? relative to East??
      int tarid;			/// Target id no -- 0 for unknown, -99 for not set
      int sar_image;		/// SAR -- associated image no (-99 for no SAR image)
      int sar_row;			/// SAR -- 
      int sar_col;			/// SAR -- 
      int sar_alg;			/// SAR -- 
      int sar_el;			/// SAR -- Elev angle in deg
      int sar_squint;		/// SAR -- Squint angle in deg (broadside is 90)
      float sar_range_angle;/// SAR -- angle of SAR image rel to world coordinates  
      						///        for vector from radar to scene center (anti-range) 
							///        in deg from East toward North
      int camera_adjust;	/// Camera -- 1 iff adjust viewer loc, azimuth, and elevation angles
      float camera_deast;	/// Camera -- Offset, Put viewer location at east + deast
      float camera_dnorth;	/// Camera -- Offset 
      float camera_dheight;	/// Camera -- Offset 
      float camera_az;		/// Camera -- Viewer azimuth angle in deg
      float camera_el;		/// Camera -- Viewer elevation angle in deg
   
      icon_class();
      ~icon_class();
      icon_class& operator=(icon_class&);
      
};

#endif /* __cplusplus */
#endif /* _icon_class_h_ */
