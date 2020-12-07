#include "internals.h"


// ********************************************************************************
/// Constructor.
// ********************************************************************************
icon_class::icon_class()
{
   seqno = -99;
   east = 0.;
   north = 0.;
   height = 0.;
   target_az = 0.;
   tarid = -99;
   sar_image = -99;
   sar_range_angle = -99.;
   camera_adjust = 0;
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
icon_class::~icon_class()
{
}

// ********************************************************************************
/// Operator =.
// ********************************************************************************
icon_class& icon_class::operator=(icon_class& old)
{
   seqno     		= old.seqno;
   east       		= old.east;
   north     		= old.north;
   height     		= old.height;
   target_az  		= old.target_az;
   tarid      		= old.tarid;
   sar_image  		= old.sar_image;
   sar_row    		= old.sar_row;
   sar_col    		= old.sar_col;
   sar_alg    		= old.sar_alg;
   sar_el     		= old.sar_el;
   sar_squint 		= old.sar_squint;
   sar_range_angle 	= old.sar_range_angle;
   camera_adjust 	= old.camera_adjust;
   camera_deast 	= old.camera_deast;
   camera_dnorth 	= old.camera_dnorth;
   camera_dheight 	= old.camera_dheight;
   camera_az 		= old.camera_az;
   camera_el 		= old.camera_el;
   
   return *this;
}
