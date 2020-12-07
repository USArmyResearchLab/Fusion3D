#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
sun_pos_class::sun_pos_class()
	:base_jfd_class()
{

   // I moved these from #define statements (global in scope) because they might conflict   
   RAD 		= 0.017453292519943296;
   DEG 		= 57.29577951308232089;
   DEBUG_FLAG	= 0;
   
   latlonD = new float[2];
   date = new float[3];
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
sun_pos_class::~sun_pos_class()
{
   delete[] latlonD;
   delete[] date;
}

// ********************************************************************************
/// Set the date in format yyyy-mm-dd.
// ********************************************************************************
int sun_pos_class::set_date(char *tchar)
{
   int nread = sscanf(tchar, "%d-%d-%d", &year, &month, &day);
   if (nread != 3) {
      cerr << "sun_pos_class::set_date:  Illegal date format" << tchar << endl;
      exit_safe(1, "sun_pos_class::set_date:  Illegal date format");
   }
   return(1);
}
   
// ********************************************************************************
/// Set the time as seconds from midnight UTC.
// ********************************************************************************
int sun_pos_class::set_time(int time)
{
   sec_utc = time;
   return(1);
}
   
// ********************************************************************************
// Set the time and data in format yyyy-mm-dd T hh:mm:ss.sssZ or format yyyy-mm-dd hh:mm:s where seconds are float.
// ********************************************************************************
int sun_pos_class::set_date_time(char *tchar)
{
   int hours, minutes, nread;
   float seconds;
   
   if (strstr(tchar, "T") != NULL) {				// Format yyyy-mm-dd T hh:mm:ss.sssZ
      nread = sscanf(tchar, "%d-%d-%d T %d:%d:%f", &year, &month, &day, &hours, &minutes, &seconds);
   }
   else {
      nread = sscanf(tchar, "%d-%d-%d %d:%d:%f", &year, &month, &day, &hours, &minutes, &seconds);
   }
   if (nread != 6) {
      cerr << "sun_pos_class::set_date_time:  Illegal time format" << tchar << endl;
      exit_safe(1, "sun_pos_class::set_date_time:  Illegal time format");
   }
   sec_utc = 3600 * hours + 60 * minutes + seconds;
   return(1);
}

// ********************************************************************************
/// Set latitude and longitude in deg.
// ********************************************************************************
int sun_pos_class::set_latlon(double lat_in, double lon_in)
{
   lat = lat_in;
   lon = lon_in;
   return(1);
}
   
// ********************************************************************************
/// Get the azimuth angle of the sun in deg from due North, positive CW.
// ********************************************************************************
float sun_pos_class::get_sun_az()
{
   return az_sun;
}
   
// ********************************************************************************
/// Get the elevation (measured from the groundplane) of the sun in deg.
// ********************************************************************************
float sun_pos_class::get_sun_el()
{
   return el_sun;
}
   
// ********************************************************************************
/// Calculate the angles of the sun.
// ********************************************************************************
int sun_pos_class::calc_sunpos()
{
   latlonD[0] = lat;
   latlonD[1] = lon;
   date[0] = year;
   date[1] = month;
   date[2] = day;
   
   az_sun = get_sun_azimuth(latlonD,    /* OBSERVER LAT-LONG IN DEGREES */
                            date,       /* YEAR-MONTH-DAY DATA ARRAY    */
                            sec_utc);
   el_sun = get_sun_elevation(latlonD,  /* OBSERVER LAT-LONG IN DEGREES */
                            date,       /* YEAR-MONTH-DAY DATA ARRAY    */
                            sec_utc);
   return(1);
}
   
// ********************************************************************************
// C-code from Joe Penn -- moded by me only to make private to this class
// ********************************************************************************

/**************************** INCLUDES AND DEFINES *************************/

// #include "./solar.h"

/***************************************************************************/

/***************************************************************************
 *                                                                         *
 *      ASTRONOMICAL ALGORITHMS AND FORMULAS OBTAINED ON THE INTERNET FROM *
 * THE SOUTH AFRICAN ASTRONOMICAL OBSERVATORY (SAAO). THE CONSTANTS EXCEPT *
 * UNIT CONVERSIONS ARE BASED ON THE 01 JANUARY, 1990 (12:00:00 UTC) EPOCH *
 * AND THUS ALL CALCULATIONS WITH TIME ARE REFERENCED TO THAT EPOCH.       *
 *                                                                         *
 *      OTHER ALGORITHMS FOR JULIAN DAY AND REFRACTION WERE OBTAINED FROM  *
 * THE BOOK "ASTRONOMICAL ALGORITHMS" BY JEAN MEEUS WHICH CONTAINS SIMILAR *
 * ALGORITHMS FOR CALCULATING POSITIONS OF CELESTIAL BODIES, ET AL WITH    *
 * MUCH GREATER PRECISION, BUT WHICH WOULD BE OVERKILL FOR THIS PROBLEM.   *
 *                                                                         *
 * Joseph A. Penn       U.S. Army Research Lab          29 October 2008    *    
 ***************************************************************************/
  
/*************************** BEGIN SUBROUTINES *****************************/

/***************************************************************************/
float sun_pos_class::
      get_sun_azimuth(float latlonD[2],    /* OBSERVER LAT-LONG IN DEGREES */
                      float date[3],       /* YEAR-MONTH-DAY DATA ARRAY    */
                      float time_in_secs)  /* TIME IN SECONDS UTC          */
                                           /*                              */
/***************************************************************************
 * get_sun_azimuth:                                                        *
 *                                                                         *
 *        COMPUTES THE SOLAR AZIMUTH ANGLE FROM DUE NORTH ON LOCAL GROUND  *
 * GIVEN LAT-LONG, CALENDAR DATE OF YEAR AND UNIVERSAL COORDINATED TIME IN *
 * SECONDS. CALLS solar_noon TO LOCATE THE SUN AT SOLAR NOON (WHERE THE    *
 * SUN IS AT THE ZENITH) AT THE SAME INSTANT.                              *
 ***************************************************************************/

{                             /*********************************************/
float azimuth;                /* HEADING ANGLE RELATIVE TO NORTH           */
                              /*********************************************/
float latlon[2];              /* LATITUDE-LONGITUDE  OF LOCAL POINT        */
float lat,lon;                /* LATITUDE, LONGITUDE OF LOCAL POINT        */
                              /*********************************************/
float hA;         
float dec;
float termA,termB;

lat=latlonD[0]*RAD;
lon=latlonD[1]*RAD;

latlon[0]=lat;
latlon[1]=lon;

hA=get_hour_angle(latlon,date,time_in_secs);
dec=get_declination(date,time_in_secs);

termA=sin(hA);
termB=cos(hA)*sin(lat)-tan(dec)*cos(lat);

if (termA>0.0 && termB>0.0) {
   azimuth=180.0+atan2(termA,termB)*DEG;
   }
else if (termA>0.0 && termB<0.0) {
   azimuth=180.0+atan2(termA,termB)*DEG;
   }
else if (termA<0.0 && termB>0.0) {
   azimuth=180.0+atan2(termA,termB)*DEG;
   }
else if (termA<0.0 && termB<0.0) {
   azimuth=180+atan2(termA,termB)*DEG;
   }

return(azimuth);
}



/***************************************************************************/
float sun_pos_class::
      get_sun_elevation(float latlonD[2],     /* LOCAL LAT-LONG IN DEGREES */
                        float date[3],        /* YEAR-MONTH-DAY DATA ARRAY */
                        float time_in_secs)   /* TIME IN SECONDS UTC       */
                                              /*                           */
/***************************************************************************
 * get_sun_elevation:                                                      *
 *                                                                         *
 *        COMPUTE THE ELEVATION ANGLE FROM THE GROUND TO THE SUN GIVEN THE *
 * LAT-LONG IN DEGREES OF GROUND POINT, CALENDAR DATE, AND TIME IN SECONDS *
 * UNIVERSAL COORDINATED TIME (UTC). CALLS solar_noon TO LOCATE THE SUN AT *
 * SOLAR NOON (WHERE THE SUN IS AT THE ZENITH) AT THE SAME INSTANT.        *
 ***************************************************************************/

{                          /************************************************/
float elevation;           /* ANGLE FROM GROUND PLANE TO SOLAR DISK CENTER */
                           /************************************************/
float latlon[2];
float lat,lon;
float hA;
float dec;

lat=latlonD[0]*RAD;
lon=latlonD[1]*RAD;

latlon[0]=lat;
latlon[1]=lon;

hA=get_hour_angle(latlon,date,time_in_secs);
dec=get_declination(date,time_in_secs);

elevation=asin(sin(lat)*sin(dec)+cos(lat)*cos(dec)*cos(hA));

return(elevation*DEG);
}


/***************************************************************************/
float sun_pos_class::
      get_sun_intensity(float elev_RealD,  /* REAL SUN ELEVATION (DEGREES) */
                        float pAir,        /* AIR PRESSURE IN MILLIBARS    */
                        float tAir)        /* CELSIUS AIR TEMPERATURE      */
                                           /*                              */
/***************************************************************************
 * get_sun_intensity:                                                      *
 *                                                                         *
 *      COMPUTE THE NORMALIZED SOLAR INTENSITY (1.000 AT SOLAR NOON) FOR   *
 * A KNOWN ELEVATION ANGLE WITH REFRACTION DEPENDENT ON AIR TEMPERATURE    *
 * AND PRESSURE.  NO ATMOSPHERIC TRANSMISSION ATTENUATION IS APPLIED!      *
 ***************************************************************************/

{                             /*********************************************/
float intensity;              /* NORMALIZED SOLAR INTENSITY                */
float elev_ApparentD;         /* APPARENT (OBSERVED) ELEVATION WRT HORIZON */
                              /*********************************************/

elev_ApparentD=refract(elev_RealD,pAir,tAir);

intensity=sin(elev_ApparentD*RAD);

                       /****************************************************/
return(intensity);     /* RETURN NORMALIZED NON-ATTENUATED SOLAR INTENSITY */
}                      /****************************************************/



/***************************************************************************/
float sun_pos_class::get_hour_angle(float latlon[2],      /* OBSERVER LAT-LONG IN RADIANS */
                     float date[3],        /* YEAR-MONTH-DAY DATA ARRAY    */
                     float time_in_secs)   /* TIME IN SECONDS UTC          */
                                           /*                              */
/***************************************************************************
 * get_hour_angle:                                                         *
 *                                                                         *
 *     GIVEN GEODETIC LATITUDE-LONGITUDE OF THE OBSERVED POINT, DATE, AND  *
 * THE UNIVERSAL COORDINATED TIME IN SECONDS FIND THE HOUR ANGLE OR THE    *
 * ANGULAR ROTATION OF THE SKY VERSUS NOON OF VERNAL EQUINOX.              *
 ***************************************************************************/

{                             /*********************************************/
float hrAng;                  /* ANGLE OF ROTATION OF SKY                  */
                              /*********************************************/
float lon;                    /* LATITUDE, LONGITUDE OF LOCAL POINT        */
                              /*********************************************/
float utcHrs;                 /* UTC TIME IN HOURS                         */
                              /*********************************************/
float meanSL;                 /* MEAN SOLAR LONGITUDE FOR GIVEN YEAR       */
float meanSA;                 /* MEAN SOLAR ANOMALY FOR GIVEN YEAR         */
                              /*********************************************/
float epsilon;                /* EPSILON: OBLIQUITY OF ECLIPTIC            */
float lambda;                 /* SOLAR ECLIPTIC LONGITUDE AT DATE AND TIME */
float alpha;                  /* RIGHT ASCENSION OF SUN AT DATE AND TIME   */
                              /*********************************************/
float jDE1900;                /* JULIAN DAY FROM JULIAN DAY EPOCH          */
float jD100;                  /* JULIAN DAY DIVIDED BY JULIAN CENTURY      */
                              /*********************************************/
float hms[3];                 /* HOURS:MINUTES:SECONDS (UTC)               */
                              /*********************************************/
float tmp;                    /* TEMPORARY VARIABLE FOR INTERMEDIATE USE   */
                              /*********************************************/
int n360;                     /* NUMBER OF 360 DEGREE MULTIPLES WITHIN tmp */
                              /*********************************************/
lon=latlon[1];

/***************************************************************************
 * COMPUTE TIMES IN SECONDS, DATES AND TIME IN JULIAN DAYS FROM EPOCH 1900 * 
 ***************************************************************************/
                                          /*                               */
hms[3]=cnvrt_sec2hms(time_in_secs,hms);   /* TIME IN HRS, MINUTES, SECONDS */
utcHrs=time_in_secs/HOUR;                 /* EXPRESSED AS HOURS            */
                                          /*********************************/
jDE1900=julian_from_E1900(date,hms);      /* JULIAN DAY - EPOCH 1900-01-01 */
jD100=jDE1900/JYR100;                     /* JULIAN CENTURIES SINCE EPOCH  */
                                          /*********************************/
meanSL=279.697+(36000.769*jD100);         /* IN DEGREES...                 */
n360=(int) (meanSL/360.0);                /* 360D DEGREE INTEGER MULTIPLE   */
meanSL-=((float) n360*360.0);             /* REMOVE 360 DEGREE MULTIPLES   */
meanSL*=RAD;                              /* MEAN SOLAR LONGITUDE          */
                                          /*********************************/
meanSA=358.476+(35999.050*jD100);         /* IN DEGREES...                 */
n360=(int) (meanSA/360.0);                /* INTEGER MULTIPLE OF 360 DEG   */
meanSA-=((float) n360*360.0);             /* REMOVE 360 DEGREE MULTIPLES   */
meanSA*=RAD;                              /* MEAN SOLAR ANOMALY            */ 
                                          /*********************************/

lambda=(meanSL*DEG)+(1.919-(0.005*jD100))*sin(meanSA)+0.02*sin(2.0*meanSA);
n360=(int) (lambda/360.0);
lambda-=(float) n360*360.0;

epsilon=(ECLIPTIC*DEG)-0.013*jD100;    
 
          /***************************************************
           * ALPHA IS IN THE SAME QUADRANT AS LAMBDA...      * 
           ***************************************************/

tmp=tan(lambda*RAD)*cos(epsilon*RAD);

if (lambda>=0.0 && lambda<90.0) {            /* FIRST QUADRANT:  (+,+)     */
   alpha=atan2(tmp,1.0f)*DEG;
   }
else if (lambda>=90.0 && lambda<180.0) {     /* SECOND QUADRANT: (+,-)     */
   alpha=180.0-atan2(-tmp,1.0f)*DEG;
   }
else if (lambda>=180.0 && lambda<270.0) {    /* THIRD QUADRANT:  (-,-)     */
   alpha=360.0+atan2(-tmp,-1.0f)*DEG;
   }
else if (lambda>=270.0 && lambda<360.0) {    /* FOURTH QUADRANT: (-,+)     */
   alpha=360.0-atan2(-tmp,1.0f)*DEG;
   }
          /*************************************************** 
           * REMOVE 360 DEGREE MULTIPLES FROM THE HOUR ANGLE * 
           ***************************************************/

hrAng=(meanSL+lon)*DEG-alpha+180.0+(15.0*utcHrs);
n360=(int) (hrAng/360.0);
hrAng-=(float) (n360*360.0);

if (DEBUG_FLAG) {
   printf("\n           COMPUTING HOUR ANGLE...\n");
   printf("JULIAN CENTURIES   :   %13.9f\n", jD100);
   printf("MEAN SOLAR LATITUDE:   %13.9f-deg\n",meanSL*DEG);
   printf("MEAN SOLAR ANOMALY :   %13.9f-deg\n",meanSA*DEG);
   printf("EPSILON            :   %13.9f-deg\n",epsilon);
   printf("LAMBDA             :   %13.9f-deg\n",lambda);
   printf("TMP                :   %13.9f\n",tmp);
   printf("RIGHT ASCENSION    :   %13.9f-deg\n",alpha);
   printf("HOUR ANGLE         :   %13.9f-deg\n",hrAng);
   }
                                           /********************************/
return(hrAng*RAD);                         /* RETURN HOUR ANGLE IN RADIANS */
}                                          /********************************/
 


/***************************************************************************/
float sun_pos_class::get_declination(float date[3],          /* YEAR-MONTH-DAY DATA ARRAY */
                      float time_in_secs)     /* TIME IN SECONDS UTC       */
                                              /*                           */
/***************************************************************************
 * get_declination:                                                        *
 *                                                                         *
 *        COMPUTES THE DECLINATION ANGLE GIVEN THE DATE, AND THE UNIVERSAL *
 * COORDINATED TIME IN SECONDS.  NUMERICAL CONSTANTS IN THE EQUATIONS ARE  *
 * DEFINED RELATIVE TO THE 1900 EPOCH.  ALL ANGLES IN DEGREES IN "D".  THE *
 * ANGLES USED IN TRIGONOMETRIC CALCULATIONS ARE IN RADIANS.               *
 ***************************************************************************/

{                             /*********************************************/
float declination;            /* SOLAR DECLINATION AT DATE AND TIME        */
                              /*********************************************/
float utcHrs;                 /* UTC TIME IN HOURS                         */
                              /*********************************************/
float meanSL;                 /* MEAN SOLAR LONGITUDE FOR GIVEN YEAR       */
float meanSA;                 /* MEAN SOLAR ANOMALY FOR GIVEN YEAR         */
                              /*********************************************/
float epsilon;                /* OBLIQUITY OF ELLIPSE                      */
float lambda;                 /* SOLAR ECLIPTIC LONGITUDE AT DATE          */
                              /*********************************************/
float jDE1900;                /* JULIAN DAY FROM JULIAN DAY EPOCH          */
float jD100;                  /* JULIAN DAY DIVIDED BY JULIAN CENTURY      */
                              /*********************************************/
float hms[3];                 /* HOURS:MINUTES:SECONDS (UTC)               */
                              /*********************************************/
int n360;                     /* NUMBER OF 360 DEGREE MULTIPLES WITHIN ANG */
                              /*********************************************/
                                          /*                               */
hms[3]=cnvrt_sec2hms(time_in_secs,hms);   /* TIME IN SECONDS TO HH:MM:SS   */
utcHrs=time_in_secs/HOUR;                 /* TIME IN SECONDS TO HOURS      */
                                          /*********************************/
jDE1900=julian_from_E1900(date,hms);      /* JULIAN DAY - EPOCH 1900-01-01 */
jD100=jDE1900/JYR100;                     /* JULIAN CENTURIES SINCE EPOCH  */
                                          /*********************************/
meanSL=279.697+(36000.769*jD100);         /* IN DEGREES...                 */
n360=(int) (meanSL/360.0);                /* INTEGER MULTIPLE OF 360 DEG   */
meanSL-=((float) n360*360.0);             /* REMOVE 360 DEGREE MULTIPLES   */
meanSL*=RAD;                              /* MEAN SOLAR LONGITUDE          */
                                          /*********************************/
meanSA=358.476+(35999.050*jD100);         /* IN DEGREES...                 */
n360=(int) (meanSA/360.0);                /* INTEGER MULTIPLE OF 360 DEG   */
meanSA-=((float) n360*360.0);             /* REMOVE 360 DEGREE MULTIPLES   */
meanSA*=RAD;                              /* MEAN SOLAR ANOMALY            */ 
                                          /*********************************/

lambda=(meanSL*DEG)+(1.919-(0.005*jD100))*sin(meanSA)+0.02*sin(2.0*meanSA);
n360=(int) (lambda/360.0);
lambda-=(float) n360*360.0;

epsilon=ECLIPTIC*DEG-(0.013*jD100);    
declination=asin(sin(lambda*RAD)*sin(epsilon*RAD));

if (DEBUG_FLAG) {
   printf("\n           COMPUTING DECLINATION ANGLE...\n");
   printf("JULIAN CENTURIES   :   %13.9f\n",jD100);
   printf("MEAN SOLAR LATITUDE:   %13.9f-deg\n",meanSL*DEG);
   printf("MEAN SOLAR ANOMALY :   %13.9f-deg\n",meanSA*DEG);
   printf("EPSILON            :   %13.9f-deg\n",epsilon);
   printf("LAMBDA             :   %13.9f-deg\n",lambda);
   printf("DECLINATION ANGLE  :   %13.9f\n",declination*DEG);
   }
                           /************************************************/
return(declination);       /* RETURN THE SOLAR DECLINATION ANGLE (RADIANS) */
}                          /************************************************/
 


/***************************************************************************/
float sun_pos_class::
      refract(float elev_RealD,         /* ACTUAL ELEVATION ANGLE OF SUN   */
              float pAir,               /* AIR PRESSURE IN MILLIBARS       */
              float tAir)               /* AIR TEMPERATURE DEGREES CELSIUS */
                                        /*                                 */
/***************************************************************************
 * refract:                                                                *
 *                                                                         *
 *       CALCULATES THE APPARENT ELEVATION OF SUN AFTER AIR REFRACTION IS  *
 * APPLIED.   REFRACTION IS TEMPERATURE AND PRESSURE DEPENDENT AND CAN     *
 * ACCOUNT FOR UP TO 0.61 DEGREES OF DIFFERENCE BETWEEN THE APPARENT AND   *
 * THE ACTUAL POSITION OF THE SUN IN THE VERTICAL DIRECTION AT ANGLES      *
 * CLOSE TO THE HORIZON.  THE ALGORITHM ASSUMES THAT ANGLES ARE MEASURED   *
 * WITH RESPECT TO THE HORIZONTAL PLANE SO THAT ANGLES DO NOT EXCEED 90.0  *
 * DEGREES (WITH RESPECT TO THE HORIZON.)                                  *
 *                                                                         *
 *        THIS ALGORITHM WAS FOUND ON PAGES 106-107 OF "ASTRONOMICAL       *
 * ALGORITHMS" BY JEAN MEEUS (SEE julian BELOW.)  ACCURACY IS SAID TO BE   *
 * APPROXIMATELY 0.0011 DEGREES OVER THE RANGE FROM 0 TO 90 DEGREES WITH   *
 * RESPECT TO THE HORIZON.                                                 *
 ***************************************************************************/

{                            /**********************************************/
float refraction;            /* REFRACTION ANGLE AFTER CORRECTIONS APPLIED */
float elev_apparent,h;       /* APPARENT (HIGHER THAN ACTUAL) ELEVATION    */
                             /**********************************************/
float correction_for_PT;     /* CORRECTION TERM FOR TEMPERATURE & PRESSURE */
float standard_ratio;        /* ALGORITHM CONSTS VALID AT 1010 MB & 10 C   */
float adjusted_ratio;        /* ADJUSTMENT FACTOR FOR OTHER T AND P        */
                             /**********************************************/
h=elev_RealD;                /* SET TO ACTUAL ELEVATION ABOVE HORIZON      */
                             /**********************************************/
if (h<90.0) {                /* NEAR HORIZON IN SAME DIRECTION AS VIEWER   */
                             /**********************************************/
   refraction=1.02*ARC_MIN/tan((h+(10.3/(h+5.11)))*RAD);
   }
                             /**********************************************/
else if (h>90.0) {           /* DEFINED HORIZON POINT OTHER SIDE OF ZENITH */
                             /**********************************************/
   h=180.0-elev_RealD;       /* RE-DEFINE WITH RESPECT TO NEAREST HORIZON  */
                             /**********************************************/
   refraction=-1.02*ARC_MIN/tan((h+(10.3/(h+5.11)))*RAD);
   }
                             /**********************************************/
else {                       /* SUN IS AT THE ZENITH POINT (SOLAR NOON)    */
                             /**********************************************/
   refraction=0.0;           /* REFRACTION IS ZERO AT ZENITH IN STABLE AIR */
   }                         /**********************************************/

/***************************************************************************
 * APPLY PRESSURE & TEMPERATURE FOR AIR DENSITY TO ACCOUNT FOR REFRACTION  *
 * DECREASE WITH HIGHER TEMPERATURE AND INCREASE WITH HIGHER PRESSURE AND  *
 * VICE VERSA.                                                             *
 ***************************************************************************/

standard_ratio=(10.0+KELVIN)/1010.0; /* AT 10.0 CELSIUS & 1010 MB PRESSURE */
adjusted_ratio=pAir/(tAir+KELVIN);
correction_for_PT=standard_ratio*adjusted_ratio;

                                /*******************************************/
refraction*=correction_for_PT;  /* SCALE REFRACTION FOR EFFECT OF P AND T  */
                                /*******************************************/

elev_apparent=elev_RealD+refraction;

if (DEBUG_FLAG) {
   printf("STANDARD RATIO: %13.9f\n",standard_ratio);
   printf("ADJUSTED RATIO: %13.9f\n",adjusted_ratio);
   printf("P-T CORRECTION: %13.9f\n",correction_for_PT);
   printf("REFRACTION    : %13.9f\n",refraction);
   }
                                /*******************************************/
return(elev_apparent);          /* RETURN THE APPARENT SUN ELEVATION ANGLE */
}                               /*******************************************/
 


/***************************************************************************/
float sun_pos_class::
      julian(float date[3])                   /* YEAR-MONTH-DAY DATA ARRAY */
                                              /*                           */
/***************************************************************************
 * julian:                                                                 *
 *                                                                         *
 *      GIVEN THE YEAR, MONTH, AND DAY COMPUTE THE INTEGER JULIAN DAY AT   *
 * NOON.                                                                   *
 *                                                                         *
 *      THIS ALGORITHM WAS FOUND ON PAGES 60-61 OF "ASTRONOMIC ALGORITHMS, *
 * 2ND EDITION," BY JEAN MEEUS PUBLISHED BY WILLMAN-BELL, INC. MARCH 2000. *
 ***************************************************************************/

{
float jDay;
float year,month,day;

int a,b,jDtmp;
  
year=date[0];
month=date[1];
day=date[2];

a=(int) (year/100);
b=2-a+(int) (a/4);
                       /****************************************************/
if (month<=2) {        /* JAN, FEB SET 13TH OR 14TH MONTH OF PREVIOUS YEAR */
                       /****************************************************/
   month+=12.0;
   year-=1.0;
   }

jDtmp=(int) (JYEAR*(year+4716))+(int) (30.6001*(month+1));
jDay=(float) jDtmp+b+day-1524.5;

                       /****************************************************/
return(jDay);          /* RETURNS JULIAN DATE IN GREGORIAN CALENDAR (NOON) */
}                      /****************************************************/


/***************************************************************************/
float sun_pos_class::
      julian_from_E1900(float date[3],     /* YEAR-MONTH-DAY ARRAY         */
                        float time[3])     /* HR-MIN-SEC PAST 00:00:00 UTC */
                                           /*                              */
/***************************************************************************
 * julian_from_E1900:                                                      *
 *                                                                         *
 *       COMPUTE JULIAN YEAR WITH RESPECT TO AN ARBITRARY DATE AT AN       *
 * ARBITRARY TIME OF DAY WITH SUCCESSIVE CALLS TO julian FUNCTION ABOVE.  *
 ***************************************************************************/

{                      /****************************************************/
float jDate;           /* JULIAN DAY AT INPUT DATE AND TIME                */ 
float jDE;             /* JULIAN DAYS FROM BEGINNING EPOCH 1900-01-01      */
                       /****************************************************/
float time_in_days;    /* TIME IN HOURS, MINUTES, SECONDS AS PART OF A DAY */
                       /****************************************************/

jDate=julian(date);
jDE=jDate-EPOCH1900;

time_in_days=(time[0]*HOUR+time[1]*MINUTE+time[0])/SOLAR_DAY;
jDE+=time_in_days;
                           /************************************************/
return(jDE);               /* RETURN JULIAN DAY WITH RESPECT TO EPOCH DATE */
}                          /************************************************/



/***************************************************************************/
float sun_pos_class::
      cnvrt_UTC2loc(float utcTime,      /* LOCAL TIME (SECS PAST 00:00:00) */
                    float latlonD[2],   /* LOCAL LATITUDE-LONGITUDE        */
                    float timeOffset)   /* OFFSET FROM STANDARD TIME ZONE  */
                                        /*                                 */
/*************************************************************************** 
 * cnvrt_UTC2loc:                                                          *
 *                                                                         *
 *       USE IF LOCAL TIME IN SECONDS IS INPUT INSTEAD OF UTC TO CORRECT   *
 * TIME ZONE AND LOCAL DEPARTURES FROM UTC USED TO COMPUTE SOLAR AZIMUTH,  *
 * AND SOLAR ELEVATION ANGLES.                                             *
 ***************************************************************************
 * TIME OFFSET:                                                            *
 *                                                                         *
 *       INCLUDES FRACTIONAL HOURS, E.G., NEWFOUNDLAND (+0.5), AS WELL AS, *
 * THE DAYLIGHT SAVINGS TIME OFFSET FROM THE STANDARD TIME ZONE BASED ON   *
 * THE LONGITUDINAL DIFFERENCE FROM THE PRIME MERIDIAN (POSITIVE EAST,     *
 * NEGATIVE WEST).  STANDARD TIMEZONES ARE 15 DEGREES WIDE WITH THE ZEROTH *
 * TIME ZONE CENTERED AT ZERO EXTENDING FROM LONGITUDE -7.5 (WEST) THRU    *
 * +7.5 (EAST).  TIMEZONE BOUNDARIES ARE NOT UNIFORMLY 15-DEGREE LONGITUDE *
 * WIDE SECTIONS ALTHOUGH IN GENERAL THEY ARE CENTERED ON THE 15-DEGREE    *
 * INCREMENTS OF LONGITUDE.                                                *
 ***************************************************************************/

{                                     /*************************************/
float localTime;                      /* LOCAL TIME SINCE 00:00:00 IN SECS */
float timeZone;                       /* STANDARD TIME ZONE PLUS OFFSETS   */
                                      /*************************************/
float lon;                            /* LOCAL LONGITUDE IN DEGREES        */
                                      /*************************************/
int timezone;                         /* STANDARD TIMEZONE FROM LONGITUDE  */
                                      /*************************************/
lon=latlonD[1];
                                      /*************************************/
if (lon>0.0) {                        /* IF (+) EAST OF PRIME MERIDIAN     */
   timezone=(int) (lon+7.5)/15.0;     /* CHECK EASTERN EDGE OF TIMEZONE    */
   }                                  /*************************************/
else if (lon<0.0) {                   /* ELSE (-) WEST OF PRIME MERIDIAN   */
   timezone=(int) (lon-7.5)/15.0;     /* CHECK WESTERN EDGE OF TIMEZONE    */
   }                                  /*************************************/
else {                                /* AT THE PRIME MERIDIAN!            */
   timezone=0;                        /* ZEROTH TIMEZONE                   */
   }                                  /*************************************/

timeZone=((float) timezone+timeOffset)*HOUR;
localTime=utcTime+timeZone;

if (localTime<0.0) {
   localTime+=SOLAR_DAY;
   }

if (localTime>SOLAR_DAY) {
   localTime-=NOON;
   }
                            /***********************************************/
return(localTime);          /* RETURN LOCAL TIME IN SECONDS AFTER 00:00:00 */
}                           /***********************************************/

 

/***************************************************************************/
float sun_pos_class::
      cnvrt_loc2UTC(float clktime,      /* LOCAL TIME (SECS PAST 00:00:00) */
                    float latlon[2],    /* LOCAL LATITUDE-LONGITUDE        */
                    float timeOffset)   /* OFFSET FROM STANDARD TIME ZONE  */
                                        /*                                 */
/*************************************************************************** 
 * cnvrt_loc2UTC:                                                          *
 *                                                                         *
 *       USE IF LOCAL TIME IN SECONDS IS INPUT INSTEAD OF UTC TO CORRECT   *
 * TIME ZONE AND LOCAL DEPARTURES FROM UTC USED TO COMPUTE SOLAR AZIMUTH,  *
 * AND SOLAR ELEVATION ANGLES.                                             *
 ***************************************************************************
 * TIME OFFSET:                                                            *
 *                                                                         *
 *       INCLUDES FRACTIONAL HOURS, E.G., NEWFOUNDLAND (+0.5), AS WELL AS, *
 * THE DAYLIGHT SAVINGS TIME OFFSET FROM THE STANDARD TIME ZONE BASED ON   *
 * THE LONGITUDINAL DIFFERENCE FROM THE PRIME MERIDIAN (POSITIVE EAST,     *
 * NEGATIVE WEST).  STANDARD TIMEZONES ARE 15 DEGREES WIDE WITH THE ZEROTH *
 * TIME ZONE CENTERED AT ZERO EXTENDING FROM LONGITUDE -7.5 (WEST) THRU    *
 * +7.5 (EAST).  TIMEZONE BOUNDARIES ARE NOT UNIFORMLY 15-DEGREE LONGITUDE *
 * WIDE SECTIONS ALTHOUGH IN GENERAL THEY ARE CENTERED ON THE 15-DEGREE    *
 * INCREMENTS OF LONGITUDE.                                                *
 ***************************************************************************/

{                                     /*************************************/
float utcTime;                        /* UNIVERSAL COORDINATED TIME (UTC)  */
float timeZone;                       /* STANDARD TIME ZONE PLUS OFFSETS   */
                                      /*************************************/
float lon;                            /* LOCAL LONGITUDE IN DEGREES        */
                                      /*************************************/
int timezone;                         /* STANDARD TIMEZONE FROM LONGITUDE  */
                                      /*************************************/
lon=latlon[1]*DEG;
                                      /*************************************/
if (lon>0.0) {                        /* IF (+) EAST OF PRIME MERIDIAN     */
   timezone=(int) (lon+7.5)/15.0;     /* CHECK EASTERN EDGE OF TIMEZONE    */
   }                                  /*************************************/
else if (lon<0.0) {                   /* ELSE (-) WEST OF PRIME MERIDIAN   */
   timezone=(int) (lon-7.5)/15.0;     /* CHECK WESTERN EDGE OF TIMEZONE    */
   }                                  /*************************************/
else {                                /* AT THE PRIME MERIDIAN!            */
   timezone=0;                        /* ZEROTH TIMEZONE                   */
   }                                  /*************************************/

timeZone=((float) timezone+timeOffset)*HOUR;
utcTime=clktime+timeZone;
                             /**********************************************/
return(utcTime);             /* RETURN TIME IN SECONDS WITH RESPECT TO UTC */
}                            /**********************************************/
 


/***************************************************************************/
float sun_pos_class::
      cnvrt_gcl2gdl(float gclatlon[2],       /* GEOCENTRIC LATITUDE        */
                    float height,            /* HEIGHT ABOVE MSL ELLIPSOID */
                    float gdlatlon[2])       /* GEODETIC LATITUDE          */
                                             /*                            */
/***************************************************************************
 * cnvrt_gcl2gdl:                                                          *
 *                                                                         *
 *      USE TO CONVERT GEOCENTRIC LATITUDE TO GEODETIC LATITUDE            *
 ***************************************************************************/

{                             /*********************************************/
float gclat,gdlat;            /* GEOCENTRIC AND GEODETIC LATITUDES         */
float gdlon;                  /* LONGITUDE GEODETIC IS EQUAL TO GEOCENTRIC */
                              /*********************************************/
float sinLatGC,cosLatGC;      /* SINE AND COSINE OF GEOCENTRIC LATITUDE    */
float sinLatGC2,cosLatGC2;    /* SQUARES OF SINE AND COSINE OF GC LATITUDE */
float h;                      /* CORRECTION TERM WHEN ALTITUDE IS NON-ZERO */
                              /*********************************************/
gclat=gclatlon[0];
gdlon=gclatlon[1];

if (height==0.0) {
   h=0.0;
   }
else {
   sinLatGC=sin(gclat);
   cosLatGC=cos(gclat);

   sinLatGC2=sinLatGC*sinLatGC;
   cosLatGC2=cosLatGC*cosLatGC;

   /************************************************************************
    * ADJUST HEIGHT DIFFERENCE BETWEEN GEOCENTRIC AND GEODETIC COORDINATES *
    * NEGLIGIBLE DIFFERENCE EXCEPT FOR VERY HIGH ALTITUDES                 * 
    ************************************************************************/

   h=height*sqrt(RADIUS_POL2*cosLatGC2+RADIUS_EQ2*sinLatGC2);
   }

gdlat=atan2(tan(gclat)*(h+RADIUS_EQ2),h+RADIUS_POL2);

gdlatlon[0]=gdlat;
gdlatlon[1]=gclatlon[1];               /* LONGITUDES ARE THE SAME FOR BOTH */

if (DEBUG_FLAG) {
   printf("\n                    CONVERT GCL TO GDL\n");
   printf("------------------------------------------------------------\n");
   printf("Geocentric latitude: %13.9f-dg\n",gclat*DEG);
   printf("Geodetic   latitude: %13.9f-deg\n",gdlat*DEG);
   printf("------------------------------------------------------------\n");
   }
                                     /**************************************/
return(gdlatlon[2]);                 /* RETURN GEODETIC LATITUDE-LONGITUDE */
}                                    /**************************************/

 
/***************************************************************************/
float sun_pos_class::
      cnvrt_hms2sec(float hhmmss[3])    /* ARRAY OF HRS-MINS-SECS          */
                                        /*                                 */
/***************************************************************************
 * cnvrt_hms2sec:                                                          *
 *                                                                         *
 *      CONVERT UNIVERSAL COORDINATED TIME (UTC) IN HR:MIN:SEC OF 24-HOUR  *
 *  CLOCKTIME TO SECONDS ELAPSED SINCE 00:00:00                            *
 ***************************************************************************/

{
float hr,min,sec;
float seconds;

hr=hhmmss[0]*HOUR;
min=hhmmss[1]*MINUTE;
sec=hhmmss[2];

seconds=hr+min+sec;

if (DEBUG_FLAG) {         /* CALCULATE SECONDS FROM HRS,MINUTES,SECS AND COMPARE */ 
   printf("\nUTC CLOCK TIME     : %.2f:%.2f:%.5f ",hr,min,sec);
   printf("\nTIME IN SECONDS    : %10.5f-sec\n",seconds);
   }
                                  /*****************************************/
return(seconds);                  /* RETURN SECONDS ELAPSED SINCE MIDNIGHT */
}                                 /*****************************************/


/***************************************************************************/
float sun_pos_class::
      cnvrt_sec2hms(float time_in_secs, /* TOTAL TIME IN SECONDS           */
                    float hhmmss[3])    /* ARRAY OF HRS-MINS-SECS          */
                                        /*                                 */
/***************************************************************************
 *  cnvrt_sec2hms:                                                         *
 *                                                                         *
 *      CONVERT UNIVERSAL COORDINATED TIME (UTC) IN SECONDS PAST MIDNIGHT  *
 * TO HOURS, MINUTES, AND SECONDS OF 24-HOUR CLOCKTIME.                    *
 ***************************************************************************/

{
int hr,min;

float sec;

hr=(int) (time_in_secs/HOUR); 
min=(int) ((time_in_secs-(float) hr*HOUR)/MINUTE);
sec=time_in_secs-((float) hr*HOUR+(float) min*MINUTE);

hhmmss[0]=(float) hr;
hhmmss[1]=(float) min;
hhmmss[2]=sec;

if (DEBUG_FLAG) {        /* CALCULATE SECONDS FROM HRS,MINUTES,SECS AND COMPARE */ 
   printf("\nTIME IN SECONDS    : %10.5f sec\n",time_in_secs);
   printf("\nUTC CLOCK TIME     : %.2d:%.2d:%.5f\n",hr,min,sec);
   }
                          /*************************************************/
return(hhmmss[3]);        /* RETURN POINTER TO HOURS-MINUTES-SECONDS ARRAY */
}                         /*************************************************/
 
 
/*************************** END OF SUB-ROUTINES ***************************/
