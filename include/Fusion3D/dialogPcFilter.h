#pragma once

#include <QDialog>
#include "ui_dialogPcFilter.h"

class SoSFInt32;
class SoSFFloat;

/**
Qt dialog for setting parameters for filtering point clouds.
\n
The menu sets parameters for filtering point clouds.
One can filter on the FINE algorithm quality metric.
Also, one can filter out points above a certain elevation --
either an absolute elevation of elevation above the local DSM --
so that one can more clearly see under foliage.
*/
class dialogPcFilter : public QDialog, public Ui::dialogPcFilter
{
	Q_OBJECT

	int filterTypeFlag;		// Filter type 0=none, 1=FINE, 2= elevation, 3 = elevation rel to DEM
	SoSFInt32*		GL_mobmap_mod;		// OIV Global -- Flag indicating type of modify PC parameters
	SoSFInt32*		GL_mobmap_filt_type;// OIV Global -- Current filtering -- 1=none, 2=TAU, 3=elevation
	SoSFFloat*		GL_mobmap_filt_rmin;// OIV Global -- Filter on relative elevation -- rmin
	SoSFFloat*		GL_mobmap_filt_rmax;// OIV Global -- Filter on relative elevation -- rmax
	SoSFFloat*		GL_mobmap_filt_amin;// OIV Global -- Filter on absolute elevation -- rmin
	SoSFFloat*		GL_mobmap_filt_amax;// OIV Global -- Filter on absolute elevation -- rmax
	SoSFInt32*		GL_mobmap_fine_min;	// OIV Global -- Min FINE TAU value for display
	SoSFInt32*		GL_mobmap_fine_max;	// OIV Global -- Max FINE TAU value for display

private slots:
	void doApply();
	void doNone();
	void doFine();
	void doElevAbs();
	void doElevRel();

public:
	dialogPcFilter(QWidget *parent = Q_NULLPTR);
	~dialogPcFilter();
};
