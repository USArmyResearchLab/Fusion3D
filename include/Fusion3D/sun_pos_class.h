#ifndef _sun_pos_class_h_
#define _sun_pos_class_h_
#ifdef __cplusplus

#define RADIUS_EQ   6.378137E3           /* RADIUS AT EQUATOR             */
#define RADIUS_EQ2  4.0680631590769E7    /* SQUARE OF RADIUS AT EQUATOR   */
#define RADIUS_POL  6.3567523142E3       /* RADIUS AT POLE                */
#define RADIUS_POL2 4.0408299981544355E7 /* SQUARE OF RADIUS AT POLE      */
#define ECLIPTIC    23.452*RAD           /* SUN OVERHEAD VERNAL EQUINOX   */
 
 /*********************** CHRONOLOGICAL/HOROLOGICAL ************************/

#define HOUR        3600.0               /* HOUR IN SECONDS               */
#define MINUTE      60.0                 /* MINUTE IN SECONDS             */
                                         /*********************************/
#define SOLAR_DAY   86400.0              /* SOLAR DAY IN SECONDS          */
#define NOON        12.0*HOUR            /* NOON 12:00:00 UTC (43200 SEC) */
#define JYEAR       365.25               /* JULIAN YEAR IN 24-HOUR DAYS   */
#define JYR100      36525.0              /* NUMBER OF DAYS JULIAN CENTURY */
#define EPOCH1900   2415020.0            /* EPOCH 1900-01-01 (00:00:00)   */

/****************************** BOOLEANS **********************************/

/************************** MISCELLANEOUS *********************************/

                                         /*********************************/
#define KELVIN      273.15               /* FREEZING PT OF H2O IN KELVINS */
#define ARC_MIN     1.66666666667E-2     /* MINUTE OF ARC (1/60   DEGREE) */

/**************************************************************************/

/**
Calculates the sun angles given a position on the earth and a date and time.

intensity image.
*/
class sun_pos_class:public base_jfd_class{
 private:
   float az_sun;		// Sun angles in deg rel to North
   float el_sun;		// Sun angles in deg
   double lat, lon;		// LatLon in deg
   int year, month, day;	// Date
   float sec_utc;			// Seconds from midnight UTC
   
   float *latlonD, *date;	// Work -- 
   
   double RAD, DEG;
   int DEBUG_FLAG;
   
   // Private methods
   float get_sun_azimuth(float latlonD[2],       /* OBSERVER LAT-LONG IN DEGREES */
                      float date[3],             /* YEAR-MONTH-DAY DATA ARRAY    */
                      float time_in_secs);
   float get_sun_elevation(float latlonD[2],     /* LOCAL LAT-LONG IN DEGREES */
                        float date[3],           /* YEAR-MONTH-DAY DATA ARRAY */
                        float time_in_secs);     /* TIME IN SECONDS UTC       */
   float get_sun_intensity(float elev_RealD,     /* REAL SUN ELEVATION (DEGREES) */
                        float pAir,              /* AIR PRESSURE IN MILLIBARS    */
                        float tAir);             /* CELSIUS AIR TEMPERATURE      */
   float get_hour_angle(float latlon[2],         /* OBSERVER LAT-LONG IN RADIANS */
                     float date[3],              /* YEAR-MONTH-DAY DATA ARRAY    */
                     float time_in_secs);        /* TIME IN SECONDS UTC          */
   float get_declination(float date[3],          /* YEAR-MONTH-DAY DATA ARRAY */
                      float time_in_secs);       /* TIME IN SECONDS UTC       */
   float refract(float elev_RealD,               /* ACTUAL ELEVATION ANGLE OF SUN   */
              float pAir,                        /* AIR PRESSURE IN MILLIBARS       */
              float tAir);                       /* AIR TEMPERATURE DEGREES CELSIUS */
   float julian(float date[3]);                  /* YEAR-MONTH-DAY DATA ARRAY */
   float julian_from_E1900(float date[3],        /* YEAR-MONTH-DAY ARRAY         */
                        float time[3]);          /* HR-MIN-SEC PAST 00:00:00 UTC */
   float cnvrt_UTC2loc(float utcTime,            /* LOCAL TIME (SECS PAST 00:00:00) */
                    float latlonD[2],            /* LOCAL LATITUDE-LONGITUDE        */
                    float timeOffset);           /* OFFSET FROM STANDARD TIME ZONE  */
   float cnvrt_loc2UTC(float clktime,            /* LOCAL TIME (SECS PAST 00:00:00) */
                    float latlon[2],             /* LOCAL LATITUDE-LONGITUDE        */
                    float timeOffset);           /* OFFSET FROM STANDARD TIME ZONE  */
   float cnvrt_gcl2gdl(float gclatlon[2],        /* GEOCENTRIC LATITUDE        */
                    float height,                /* HEIGHT ABOVE MSL ELLIPSOID */
                    float gdlatlon[2]);          /* GEODETIC LATITUDE          */
   float cnvrt_hms2sec(float hhmmss[3]);         /* ARRAY OF HRS-MINS-SECS          */
   float cnvrt_sec2hms(float time_in_secs,       /* TOTAL TIME IN SECONDS           */
                    float hhmmss[3]);            /* ARRAY OF HRS-MINS-SECS          */
   
 public:
   sun_pos_class();
   ~sun_pos_class();
   
   int set_date_time(char *tchar);		// date-time string 'yyyy-mm-dd hh:mm:ss' OR 'yyyy-mm-dd T hh:mm:ss.sssZ'
   int set_date(char *tchar);			// date string yyyy-mm-dd
   int set_time(int time);			// time in s from midnight
   int set_latlon(double lat, double lon);	// lat, lon in deg (not deg,min,sec)
   
   int calc_sunpos();
   
   float get_sun_az();			// deg, in most standard, or "best" frame -- just define it for me
   float get_sun_el();			// deg, in most standard, or "best" frame -- just define it for me
};

#endif /* __cplusplus */
#endif /* _sun_pos_class_h_ */
