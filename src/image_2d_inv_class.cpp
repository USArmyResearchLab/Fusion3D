#include "internals.h"

// ********************************************************************************
/// Constructor.
// ********************************************************************************
image_2d_inv_class::image_2d_inv_class()
	:image_2d_class()
{
   label  = new char[50];
   numchr = new char[50];
   label_def_flag = 0;

#if defined(LIBS_COIN) 
   GL_amp_max_2d = NULL;
   timer = NULL;
#endif
}

// ********************************************************************************
/// Destructor.
// ********************************************************************************
image_2d_inv_class::~image_2d_inv_class()
{
	delete label;
	delete numchr;
}


// ********************************************************************************
/// Schedule the timer -- turn it on.
// ********************************************************************************
int image_2d_inv_class::timer_schedule()
{
#if defined(LIBS_COIN) 
   if (timer == NULL) {
      cerr << "image_2d_inv_class::timer_schedule:  Timer not defined " << std::endl;
      exit_safe(1, "image_2d_inv_class::timer_schedule:  Timer not defined ");
   }
   timer->schedule();
#endif
   return(1);
}

// ********************************************************************************
/// Unschedule the timer -- turn it off.
// ********************************************************************************
int image_2d_inv_class::timer_unschedule()
{
#if defined(LIBS_COIN)  
   if (timer == NULL) {
      cerr << "image_2d_inv_class::timer_unschedule:  Timer not defined " << std::endl;
      exit_safe(1, "image_2d_inv_class::timer_unschedule:  Timer not defined ");
   }
   timer->unschedule();
#endif
   return(1);
}

// ********************************************************************************
/// Set the timer interval.
// ********************************************************************************
int image_2d_inv_class::timer_interval(float interval)
{
#if defined(LIBS_COIN)  
   if (timer == NULL) {
      cerr << "image_2d_inv_class::timer_interval:  Timer not defined " << std::endl;
      exit_safe(1, "image_2d_inv_class::timer_unschedule:  Timer not defined ");
   }
   timer->setInterval(interval);
#endif
   return(1);
}

// ********************************************************************************
/// Set -- define label for image.
// ********************************************************************************
int image_2d_inv_class::set_label(char *label_in)
{
#if defined(LIBS_COIN)  
   strcpy(label, label_in);
   label_def_flag = 1;
#endif
   return(1);
}

// ********************************************************************************
/// Initialize Inventor stuff.
// ********************************************************************************
#if defined(LIBS_COIN)  
int image_2d_inv_class::register_inv(SoSeparator *base_in)
{
   base = base_in;
   textSep = new SoSeparator;
   textSep->ref();
   frameSep = new SoSeparator;
   frameSep->ref();
   
   SoBaseColor 		*textColor 	= new SoBaseColor;
   textColor->rgb.set1Value(0, 1., 1., 0.);
   SoFont*		textFont		= new SoFont;
   textFont->size.setValue(20.0);
   SoTranslation 	*tran1   	= new SoTranslation;
   tran1->translation.setValue(-1.+0.03, 1.1, 20.);		// Raise -- partially occluded by ground
   SoTranslation 	*tran2   	= new SoTranslation;
   tran2->translation.setValue(         0.0, 0.1, 0.0);
   frameTexture 		= new SoTexture2;
   frameTexture->ref();
   text1		= new SoText2;
   text2		= new SoText2;
   textSep->addChild(textColor);
   textSep->addChild(textFont);
   textSep->addChild(tran1);
   textSep->addChild(text1);
   //textSep->addChild(tran2);
   //textSep->addChild(text2);
   
   if (nframes > 1) {
      timer = new SoTimerSensor(timer_cbx, this);
      timer->setInterval(0.1); 			// Interval between events in s
   }
   
   GL_amp_max_2d = (SoSFFloat*) SoDB::getGlobalField("Amp-Max-2d");
   return(1);
}
#endif

// **********************************************
/// Make text for images -- Private
// **********************************************
int image_2d_inv_class::set_texts()
{
#if defined(LIBS_COIN)  
   if (bad_data_flag == -3) {
      strcpy(label, "> data interval");
      text1->string = label;
   }
   else if (bad_data_flag == -2) {
      strcpy(label, "< data interval");
      text1->string = label;
   }
 
   else if (nframes > 1) {
      if (!label_def_flag) {
         strcpy(label, "frame ");
         sprintf(numchr, "%d", i_frame);
         strcat(label, numchr);
      }
      text1->string = label;
   }
   strcpy(label, "tbd");
   text2->string = label;
#endif
   
   return(1);
}

// **********************************************
/// Draw image for the current frame.
// **********************************************
int image_2d_inv_class::make_frame()
{
#if defined(LIBS_COIN)  
   float (*coords_lod)[3] = new float[4][3];
   float (*coords_lod_tex)[2] = new float[4][2];

   coords_lod[0][0] =  -1;
   coords_lod[0][1] =  -1.;
   coords_lod[0][2] =   0.;
   coords_lod[1][0] =   1.;
   coords_lod[1][1] =  -1.;
   coords_lod[1][2] =   0.;
   coords_lod[2][0] =   1.;
   coords_lod[2][1] =   1.;
   coords_lod[2][2] =   0.;
   coords_lod[3][0] =  -1.;
   coords_lod[3][1] =   1.;
   coords_lod[3][2] =   0.;
   
   // Texture scans top-to-bottom like 2-images are ordered   
   coords_lod_tex[0][0] = 0.;
   coords_lod_tex[0][1] = 1.;
   coords_lod_tex[1][0] = 1.;
   coords_lod_tex[1][1] = 1.;
   coords_lod_tex[2][0] = 1.;
   coords_lod_tex[2][1] = 0.;
   coords_lod_tex[3][0] = 0.;
   coords_lod_tex[3][1] = 0.;
      

   // monColor->rgb.set1Value(0, 1., 0., 0.);

   SoCoordinate3 *frameCoord = new SoCoordinate3;
   frameCoord->point.setValues(0, 4, coords_lod);
   
   SoFaceSet *frameFaceSet = new SoFaceSet;
   frameFaceSet->numVertices.set1Value(0, 4);

   SoTextureCoordinate2	*frameTextureCoord	= new SoTextureCoordinate2;
   frameTextureCoord->point.setValues(0, 4, coords_lod_tex);
      
   SoComplexity *frameComplexity		= new SoComplexity;
   frameComplexity->textureQuality = 0.4;
   
   set_texts();
   
   SoDB::writelock();
   base->removeAllChildren();
   base->addChild(textSep);
   base->addChild(frameSep);

   frameSep->removeAllChildren();
   if (color_base_flag) {
      SoBaseColor 	*monColor 	= new SoBaseColor;
      monColor->rgb.set1Value(0, red_base, grn_base, blu_base);
      frameSep->addChild(monColor);
   }
   frameSep->addChild(frameComplexity);
   frameSep->addChild(frameTextureCoord);
   
   make_texture(frameTexture);
   frameSep->addChild(frameTexture);
   
   frameSep->addChild(frameCoord);
   frameSep->addChild(frameFaceSet);
   SoDB::writeunlock();
#endif
   return (1);
}


// **********************************************
/// Make the texture for the current frame.
// **********************************************
#if defined(LIBS_COIN) 
int image_2d_inv_class::make_texture(SoTexture2 *locTexture)
{
   unsigned char* data_ptr = data[i_frame];
   
   // **********************************
   // Bad data -- blank image
   // **********************************
   if (bad_data_flag != 0) {
      unsigned char *data_bad = new unsigned char[ncols*nrows];
      memset(data_bad, 127, ncols*nrows);
      locTexture->image.setValue(SbVec2s(ncols,nrows), 1, data_bad);
      delete[] data_bad;
   }
   
   // **********************************
   // Grayscale, no transparency
   // **********************************
   else if      ((data_type == 5 || data_type == 7) && !transparency_flag) {
      if (hist_eq_flag) {
         calc_hist_eq(0);
	 data_ptr = get_data_histeq();
      }
      locTexture->image.setValue(SbVec2s(ncols,nrows), 1, data_ptr);
   }

   // **********************************
   // Grayscale, transparency
   // **********************************
   else if ((data_type == 5 || data_type == 7) && transparency_flag) {
      if (hist_eq_flag) {
         calc_hist_eq(0);
	 data_ptr = get_data_histeq();
      }
      unsigned char *data_tex = new unsigned char[2*ncols*nrows];
      for (int i=0; i<ncols*nrows; i++) {
            data_tex[2*i  ] = data_ptr[i];
            data_tex[2*i+1] = 255. * transparency_alpha;	// Transparency [0,255]
      }
      locTexture->image.setValue(SbVec2s(ncols,nrows), 2, data_tex);
      delete[] data_tex;
   }
   
   // **********************************
   // Color, no transparency
   // **********************************
   else if (data_type == 6 && !transparency_flag) {
      locTexture->image.setValue(SbVec2s(ncols,nrows), 3, data_ptr);
   }
   // **********************************
   // Color, transparency
   // **********************************
   else if (data_type == 6 && transparency_flag) {
      unsigned char *data_tex = new unsigned char[4*ncols*nrows];
      for (int i=0; i<ncols*nrows; i++) {
            data_tex[4*i  ] = data_ptr[3*i];
            data_tex[4*i+1] = data_ptr[3*i+1];
            data_tex[4*i+2] = data_ptr[3*i+2];
            data_tex[4*i+3] = 255. * transparency_alpha;
      }
      locTexture->image.setValue(SbVec2s(ncols,nrows), 4, data_tex);
      delete[] data_tex;
   }
   
   // **********************************
   // Not yet implemented
   // **********************************
   else {
      cerr <<"image_2d_inv_class::make_texture:  unknown type/transparency " << data_type << "/" << transparency_flag << std::endl;
      exit_safe(1, "image_2d_inv_class::make_texture:  unknown type/transparency ");
   }
   return (1);
}
#endif

// ********************************************************************************
/// Callback wrapper so that can be called from within class.
/// If callback in main program, you can put anything into clientData, not just 'this'.
// ********************************************************************************
#if defined(LIBS_COIN)  
void image_2d_inv_class::timer_cbx(void *userData, SoSensor *timer)
{
   image_2d_inv_class* framet = (image_2d_inv_class*)  userData;
   framet->timer_cb();
}
#endif

// ********************************************************************************
/// Advance one frame and redraw the image -- Private.
// ********************************************************************************
void image_2d_inv_class::timer_cb()
{
   i_frame++;
   if (i_frame >= nframes) i_frame = 0;
   //update_frame();
   make_frame();
}
   
