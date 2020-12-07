#ifndef _osus_image_store_class_h_
#define _osus_image_store_class_h_	
#ifdef __cplusplus

/**
   Given a monotonically increasing array of times, calculate the indices of the earliest and latest times within a time window.

*/
class osus_image_store_class{
   
   private:
	   int n_images;
	   int n_images_max;
 
	   SoSeparator**		cameraImagesBase;	///< Per image stored, base of tree 


   public:
      osus_image_store_class();
      ~osus_image_store_class();
      
	  int make_sensor_image_screen(float scaleFactor, int image_nx, int image_ny, int image_np, unsigned char *image_data);
	  SoSeparator* get_image_subtree(int index);
};

#endif /* __cplusplus */
#endif /* _osus_image_store_class_h_ */
