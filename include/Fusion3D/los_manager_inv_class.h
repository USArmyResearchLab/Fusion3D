#ifndef _los_manager_inv_class_h
#define _los_manager_inv_class_h
#ifdef __cplusplus

/**
Adds OpenInventor functionality to los_manager_class.
Adds capability to draw shadowed areas and to output them to shapefiles and mask files.

*/
class los_manager_inv_class:public los_manager_class{
   private:
		SoSeparator*		classBase;
		SoSeparator*		lineBase;
		SoSeparator*		sensorBase;
		SoSeparator*		lookedAtBase;

	SoSFInt32*		GL_los_flag;		// OIV Global -- For Line-of-Sight calcs
        SoSFFloat*		GL_los_rmin;		// OIV Global -- For Line-of-Sight calcs -- Min range
        SoSFFloat*		GL_los_rmax;		// OIV Global -- For Line-of-Sight calcs -- Max range
        SoSFFloat*		GL_los_cenht;		// OIV Global -- For Line-of-Sight calcs -- Ht above ground at center
        SoSFFloat*		GL_los_perht;		// OIV Global -- For Line-of-Sight calcs -- Ht above ground at periphery
        SoSFFloat*		GL_los_amin;		// OIV Global -- For Line-of-Sight calcs -- Min angle in deg
        SoSFFloat*		GL_los_amax;		// OIV Global -- For Line-of-Sight calcs -- Max angle in deg
        SoSFInt32*		GL_los_ovis;		// OIV Global -- For Line-of-Sight calcs -- 1 iff outline vis regions
        SoSFInt32*		GL_los_oshad;		// OIV Global -- For Line-of-Sight calcs -- 1 iff outline shadow regions
        SoSFInt32*		GL_los_bound;		// OIV Global -- For Line-of-Sight calcs -- 1 iff outline regions that touch bound
        SoSFFloat*		GL_los_pshad;		// OIV Global -- For Line-of-Sight calcs -- Percent of area shadowed
        SoSFFloat*		GL_los_sensorEl;	// OIV Global -- For Line-of-Sight calcs -- Standoff Sensor -- elevation
        SoSFFloat*		GL_los_sensorN;		// OIV Global -- For Line-of-Sight calcs -- Standoff Sensor -- north
        SoSFFloat*		GL_los_sensorE;		// OIV Global -- For Line-of-Sight calcs -- Standoff Sensor -- east

        SoSFString*     GL_filename;        // OIV Global -- Open new file
        SoSFInt32*      GL_busy;            // OIV Global -- For multithreading
		SoSFFloat*		GL_mti_sensor_north;// OIV Global -- MTI sensor loc
        SoSFFloat*		GL_mti_sensor_east;	// OIV Global -- MTI sensor loc
        SoSFFloat*		GL_mti_sensor_elev;	// OIV Global -- MTI sensor loc

        SoSFFloat*		GL_clock_time;		// OIV Global -- Clock current time in s after midnight
        SoSFString*		GL_clock_date;		// OIV Global -- Clock date/time string from track

		SoSFFloat*		GL_mousem_north;	// OIV Global -- For mouse middle click
		SoSFFloat*		GL_mousem_east;		// OIV Global -- For mouse middle click
		SoSFInt32*		GL_mousem_new;		// OIV Global -- For mouse middle click

		SoFieldSensor*		mouseMSensor;	// Monitors mouse middle button
        SoFieldSensor*		losSensor;		// Monitors menu requests

		// Private methods
		int draw_los_sensor();
		int draw_los_line(float xptt1, float yptt1, float elevt1, float xptt2, float yptt2, float elevt2);
        static void los_cbx(void *userData, SoSensor *timer);
        void los_cb();
		static void mousem_cbx(void *userData, SoSensor *timer);
		void mousem_cb();
	
	
   public:
   	los_manager_inv_class();
	~los_manager_inv_class();
	int register_inv( SoSeparator* classBase_in );
    int make_scene_3d();
	int clear_all() override;
	int refresh();
};

#endif /* __cplusplus */
#endif /* _los_manager_inv_class_ */
