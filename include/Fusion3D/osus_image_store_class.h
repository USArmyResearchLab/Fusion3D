#ifndef _osus_image_store_class_h_
#define _osus_image_store_class_h_	
#ifdef __cplusplus

/**
   Provides storage for images that are defined in screen coordinates (always staying same size regardless of zoom).

   Images are created and stored internally.  They can then be retrieved by image index.

*/
class osus_image_store_class{
   
   private:
	   int n_images;					///< No. of images currently stored
	   int n_images_max;				///< Max no. of images that can be stored -- hardwired to 100
	   SoSeparator** cameraImagesBase;	///< Per image stored, base of tree 

   public:
      osus_image_store_class();
      ~osus_image_store_class();
      
	  int make_sensor_image_screen(float scaleFactor, int image_nx, int image_ny, int image_np, unsigned char *image_data);
	  SoSeparator* get_image_subtree(int index);
};

#endif /* __cplusplus */
#endif /* _osus_image_store_class_h_ */
