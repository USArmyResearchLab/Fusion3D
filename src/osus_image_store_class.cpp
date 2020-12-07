#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
osus_image_store_class::osus_image_store_class()
{
	n_images = 0;
	n_images_max = 100;
	cameraImagesBase = new SoSeparator*[n_images_max];
}

// *************************************************************
/// Destructor.
// *************************************************************

osus_image_store_class::~osus_image_store_class()
{
}

// **********************************************
/// Draw image for the current frame in screen coordinates (which does not scale with zoom).
// **********************************************
SoSeparator* osus_image_store_class::get_image_subtree(int index)
{
	if (index >= 0 && index < n_images_max) {
		return(cameraImagesBase[index]);
	}
	else {
		return NULL;
	}
}

// **********************************************
/// Draw image for the current frame in screen coordinates (which does not scale with zoom).
// **********************************************
int osus_image_store_class::make_sensor_image_screen(float scaleFactor, int image_nx, int image_ny, int image_np, unsigned char *image_data)
{
	// Need to flip image in y
	int i = 0, i_row, i_col, ip;
	unsigned char *flip_a;
	if (image_np == 3) {
		int n_chars = image_ny*image_nx*image_np;
		flip_a = new unsigned char[n_chars];
		for (i_row = 0; i_row<image_ny; i_row++) {
			for (i_col = 0; i_col<image_nx; i_col++, i++) {
				ip = (image_ny - i_row - 1) * image_nx + i_col;
				flip_a[3 * i] = image_data[3 * ip];
				flip_a[3 * i + 1] = image_data[3 * ip + 1];
				flip_a[3 * i + 2] = image_data[3 * ip + 2];
			}
		}
	}
	else {
		int n_chars = image_ny*image_nx;
		flip_a = new unsigned char[n_chars];
		for (i_row = 0; i_row<image_ny; i_row++) {
			for (i_col = 0; i_col<image_nx; i_col++, i++) {
				ip = (image_ny - i_row - 1) * image_nx + i_col;
				flip_a[i] = image_data[ip];
			}
		}
	}

	// Check *********************
	if (0) {
		image_pnm_class *image_pnm = new image_pnm_class();
		image_pnm->set_data(flip_a, image_ny, image_nx, 6);		// Works for gray-scale, 6 for color
		image_pnm->write_file("D:/Adelaide/testOSUSFiles/image.ppm");
	}
	// *******************************

	// Make an OIV SoImage containing the image and attach to base
	SoImage *img = new SoImage;
	img->vertAlignment = SoImage::TOP;
	img->horAlignment = SoImage::LEFT;
	img->width = (int)scaleFactor;
	img->height = (int)scaleFactor;
	if (image_np == 1) {			// Grayscale
		img->image.setValue(SbVec2s(image_nx, image_ny), 1, flip_a);
	}
	else if (image_np == 3) {			// Color
		img->image.setValue(SbVec2s(image_nx, image_ny), 3, flip_a);
	}

	cameraImagesBase[n_images] = new SoSeparator;
	cameraImagesBase[n_images]->ref();
	cameraImagesBase[n_images]->addChild(img);
	n_images++;

	delete[] flip_a;
	return (1);
}

