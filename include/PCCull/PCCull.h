#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_PCCull.h"

class dir_class;

/**
Culls large point clouds in the vicinity of a DSM to reduce point cloud size and reduce visual clutter when DSM and point cloud are combined.

The code was originally written to cull very large point clouds where the viewer was decimating severely.
The hope was that if we culled out all the points near the corresponding DSMs that we could greatly reduce the size of the files
and thus show a much larger percentage of the points.
When we did this, we reduced the file size by about half-- not as good as we had hoped but still useful.
The resulting point clouds look significantly denser.\n
\n
When the files are culled, they have to be combined with the DSM since they lose almost all their smooth areas like grass and roof tops.
The combination looks better since the point clouds add clutter to the DSM and the point clouds are denser also.
So culling is useful if the 2 datasets are combined.\n
\n
The culling process takes some time, so we implemented it in a separate utility.
We made the ROI larger than default (1280x1280 rather than 128x128) and that sped up the computations greatly.
With default values it was spending most of its time rereading the elevation data for new ROIs.\n
\n
One specifies all all point cloud files within a directory that match a pattern (default *.las).
The search is recursive so finds files in subdirectories.
Likewise, one finds all DSMs within a directory that match a pattern (default *.tif).
One also specifies a culling threshold; all points are culled whose elevation is closer than that threshold to the corresponding DSM elevation.\n
\n
The output filenames are constructed from the input point cloud filenames by inserting "_cull".
For example the input file test.las would generate an output file test_cull.las.\n
\n
The process is to read a block of point cloud data from the input file, cull it, then write it out.
Therefore, it should be able to read arbitrarily large files without memory problems.

*/
class PCCull : public QMainWindow, public Ui::PCCullClass
{
	Q_OBJECT

	float elevThresh;
	std::vector<std::string> pcNames;
	std::string dirnamePC;
	std::string dirnameDem;
	std::string patternPC;
	std::string patternDem;
	dir_class *dirDSM;

	int findFiles();
	int doCull();

private slots:
	void doApply();
	void doCancel();
	void doPCDirBrowse();
	void doDemDirBrowse();
	void doPcPattern();
	void doDemPattern();

public:
	PCCull(QWidget *parent = Q_NULLPTR);
};
