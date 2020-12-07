#include "internals.h"

// *******************************************
/// Constructor.
// *******************************************
draw_data_class::draw_data_class()
	:base_jfd_class()
{
	fillUnkPolyFlag = 0;
	strcpy(class_type, "draw_data");
	ptDrawPtFlag = 3;	// Use this flag for all pts -- always draw dot except when pt has symbol
	ptSymbolFlag = 0;
	lineDashFlag = 0;
	entityNameFlag = 0;
	entityTimeFlag = 0;
	avgFlag = 0;
	useLowresElevFlag = 0;

	xOffset = 0.;
	yOffset = 0.;
	zOffset = 0.;

	pxmin = -99.;
	pxmax = -99.;
	pymin = -99.;
	pymax = -99.;
	assimpFilename = "";

	time_conversion = new time_conversion_class();
	interval_calc = new interval_calc_class();
	map3d_index = NULL;
	map3d_lowres = NULL;
}

// *************************************************************
/// Destructor.
// *************************************************************

draw_data_class::~draw_data_class()
{
	delete time_conversion;
}

// ********************************************************************************
/// Register map3d_lowres_class to class.
/// Register a pointer to the map3d_lowres_class that does lowres DEM calculations.
// ********************************************************************************
int draw_data_class::register_map3d_lowres(map3d_lowres_class*	map3d_lowres_in)
{
	map3d_lowres = map3d_lowres_in;
	return(1);
}

// ********************************************************************************
/// Register the map3d_index_class used to convert elevation relative to ground to absolute elevation.
// ********************************************************************************
int draw_data_class::register_map3d_index(map3d_index_class *map3d_index_in)
{
	map3d_index = map3d_index_in;
	return TRUE;
}

// *************************************************************
/// Clear all saved data -- flags are not modified, symbols are not modified.
// *************************************************************
int draw_data_class::clear()
{
	// Polygons
	for (int i = 0; i < get_n_polygons(); i++) {
		pbx[i].clear();
		pby[i].clear();
		pbz[i].clear();
	}
	pbx.clear();
	pby.clear();
	pbz.clear();

	pbfill.clear();
	pbAltMode.clear();
	pbname.clear();
	pbtime.clear();

	// Lines
	for (int i = 0; i < get_n_lines(); i++) {
		plx[i].clear();
		ply[i].clear();
		plz[i].clear();
		if (entityTimeFlag) pltime[i].clear();
	}
	plx.clear();
	ply.clear();
	plz.clear();
	pltime.clear();

	plAltMode.clear();
	plname.clear();
	pldash.clear();
	plTimeIndex.clear();
	pltimeISymbol.clear();
	pltimeIModel.clear();

	// Points
	ppx.clear();
	ppy.clear();
	ppz.clear();
	ppAltMode.clear();
	drawPtFlag.clear();
	ppISymbol.clear();
	ppname.clear();
	pptime.clear();

	// Symbols -- stored in expernal trees, so dont delete them
	//symbolName.clear();
	return (1);
}

// *************************************************************
/// Define a symbol icon (reference to a .bmp or .png file for the symbol image) for a point.
/// @param		ipt		point number
/// @param		iconName	String containing a pathname to a .bmp or .png file for the symbol image
// *************************************************************

int draw_data_class::set_symbol_icon(int ipt, string iconName)
{
	int nSymbols = symbolName.size();
	int iSymbol = nSymbols;
	for (int i = 0; i < nSymbols; i++) {
		if (symbolName[i].compare(iconName) == 0) {
			iSymbol = i;
		}
	}
	if (iSymbol == nSymbols) symbolName.push_back(iconName);

	// Set symbol number for the point
	if (ipt < ppISymbol.size()) {
		ppISymbol[ipt] = iSymbol;
	}
	else if (ipt == ppISymbol.size()) {
		ppISymbol.push_back(iSymbol);
	}
	else {
		return(0);
	}
	return(1);
}

// ********************************************************************************
/// Define a symbol icon (reference to a .bmp or .png file for the symbol image) for a mover along a line.
/// @param		iline		line number
/// @param		iconName	String containing a pathname to a .bmp or .png file for the symbol image
// ********************************************************************************
int draw_data_class::set_mover(int iline, string iconName)
{
	if (iline < 0 || iline >= pltimeISymbol.size()) return(0);

	int nSymbols = symbolName.size();
	int iSymbol = nSymbols;
	for (int i = 0; i < nSymbols; i++) {
		if (symbolName[i].compare(iconName) == 0) {
			iSymbol = i;
		}
	}
	if (iSymbol == nSymbols) symbolName.push_back(iconName);

	// Set symbol number for the line
	pltimeISymbol[iline] = iSymbol;
	return(1);
}

// ********************************************************************************
/// Set flag that determines whether to fill polygons whose fill-flags are undefined.
/// @param		flag		FALSE not to fill (default), TRUE to fill
// ********************************************************************************
int draw_data_class::set_unk_polygon_fill_flag(bool flag)
{
	fillUnkPolyFlag = flag;
	return(1);
}

// ********************************************************************************
/// Set a smoothing interval (m) over which to average track parameters. 
// ********************************************************************************
int draw_data_class::set_distance_average_interval(float radialDist)
{
	avgRadialDist = radialDist;
	if (radialDist >= 0.) {
		avgFlag = 1;
	}
	else {
		avgFlag = 0;
	}
	return(1);
}

// *************************************************************
/// Get number of polygons currently defined
// *************************************************************

int draw_data_class::get_n_polygons()
{
	return pbx.size();
}

// *************************************************************
/// Get number of lines currently defined
// *************************************************************

int draw_data_class::get_n_lines()
{
	return plx.size();
}

// *************************************************************
/// Get number of points currently defined
// *************************************************************

int draw_data_class::get_n_points()
{
	return ppx.size();
}

// *************************************************************
/// Get number of symbols currently defined
// *************************************************************

int draw_data_class::get_n_symbols()
{
	return symbolName.size();
}

// *************************************************************
/// Get closest point over all points.
// *************************************************************

int draw_data_class::get_closest_point_from_points(float xin, float yin, int &iPointMin, float &distMin)
{
	int ipt, npt;
	float d, dmin = FLT_MAX;
	iPointMin = -99;
	npt = get_n_points();
	for (ipt = 0; ipt < npt; ipt++) {
		d = (xin - ppx[ipt])*(yin - ppx[ipt]) + (xin - ppy[ipt])*(yin - ppy[ipt]);
		if (dmin > d) {
			iPointMin = ipt;
			dmin = d;
		}
	}
	distMin = sqrt(dmin);
	return(1);
}

// *************************************************************
/// Get closest point (and associated line) over all lines to an input location.
/// @param	Selected input location
/// @param	Selected input location
/// @param	Output line of the closest point
/// @param	Output point
/// @param	Output distance from input location to closest point
// *************************************************************

int draw_data_class::get_closest_point_from_lines(float xin, float yin, int &iLineMin, int &iPointMin, float &distMin)
{
	int ipt, npt, iline;
	float d, dmin = FLT_MAX;
	iPointMin = -99;
	iLineMin = -99;

	// Loop over all lines
	for (iline = 0; iline < plx.size(); iline++) {
		npt = plx[iline].size();
		for (ipt = 0; ipt < npt; ipt++) {
			d = (xin - plx[iline][ipt])*(xin - plx[iline][ipt]) + (yin - ply[iline][ipt])*(yin - ply[iline][ipt]);
			if (dmin > d) {
				iPointMin = ipt;
				iLineMin = iline;
				dmin = d;
			}
		}
	}

	distMin = sqrt(dmin);
	return(1);
}

// ******************************************
/// Calculate distances along a line between control points, from ic1 to ic2.
/// Distances are point-to-point and will increase as point density increases.
// ******************************************
int draw_data_class::get_dist_along_line(int iline, int ic1, int ic2, float &dTotal, float &dHoriz, float &dVert, float &dVGain)
/// @param	iline		Line number
/// @param	ic1			First control point
/// @param	ic2			Second control point
/// @param	dTotal		Output distance in m -- the sum of all distances, from point to point along the path
/// @param	dHoriz		Output horizontal distance in m -- the sum of all horizontal distances from point to point along the path
/// @param	dVert		Output vertical distance in m -- the sum of all vertical distances, either up or down, from point to point along the path
/// @param	dVGain		Output net elevation gain from ic1 to ic2 in m
{
	int i1, i2, i;
	float z1, z2, dh, dv;
	if (ic1 < 0 || ic2 < 0 || ic1 >= plx[iline].size() || ic2 >= plx[iline].size()) return(0);

	// Calc net elevation gain from ic1 to ic2 in m
	calc_elev(plx[iline][ic1], ply[iline][ic1], plz[iline][ic1], plAltMode[iline], z1);
	calc_elev(plx[iline][ic2], ply[iline][ic2], plz[iline][ic2], plAltMode[iline], z2);
	dVGain = z2 - z1;

	// Sum distances from point to point along the path
	if (ic1 < ic2) {
		i1 = ic1;
		i2 = ic2;
	}
	else {
		i1 = ic2;
		i2 = ic1;
	}
	dTotal = 0;
	dHoriz = 0.;
	dVert = 0.;
	for (i = i1; i<i2; i++) {
		dh = sqrt((plx[iline][i + 1] - plx[iline][i])*(plx[iline][i + 1] - plx[iline][i]) + (ply[iline][i + 1] - ply[iline][i])*(ply[iline][i + 1] - ply[iline][i]));
		calc_elev(plx[iline][i    ], ply[iline][i    ], plz[iline][i    ], plAltMode[iline], z1);
		calc_elev(plx[iline][i + 1], ply[iline][i + 1], plz[iline][i + 1], plAltMode[iline], z2);
		dv = fabs(z2 - z1);
		dHoriz = dHoriz + dh;
		dVert = dVert + dv;
		dTotal = dTotal + sqrt(dh * dh + dv * dv);
	}

	return(1);
}

// ********************************************************************************
/// Calculates a smoothed path and look direction az.
/// First calculates the az at each point (average of forward and backward az).
/// Then averages all track points within radius avgRadialDist.
// ********************************************************************************
int draw_data_class::distance_average(int iline)
{
	int i;
	float azr, azr_left, azr_right, winding_offset = 0.;
	int npts = plx[iline].size();
	if (npts == 0) return(0);

	az_pt.clear();
	az_avg.clear();
	plx_avg.clear();
	ply_avg.clear();
	plz_avg.clear();

	// *********************************************
	// First calc az at each point -- avg of forward az and backward az
	// *********************************************
	az_pt.push_back(atan2(ply[iline][1] - ply[iline][0], plx[iline][1] - plx[iline][0]));
	for (i = 1; i<npts - 1; i++) {
		azr_left = atan2(ply[iline][i] - ply[iline][i - 1], plx[iline][i] - plx[iline][i - 1]);
		azr_right = atan2(ply[iline][i + 1] - ply[iline][i], plx[iline][i + 1] - plx[iline][i]);
		if (azr_left - azr_right > 3.1415927) azr_right = azr_right + 2. * 3.1415927;
		if (azr_right - azr_left > 3.1415927) azr_right = azr_right - 2. * 3.1415927;
		azr = 0.5 * (azr_left + azr_right) + winding_offset;
		if (azr - az_pt[i - 1] > 3.1415927) {
			azr = azr - 2. * 3.1415927;
			winding_offset = winding_offset - 2. * 3.1415927;
		}
		if (az_pt[i - 1] - azr > 3.1415927) {
			azr = azr + 2. * 3.1415927;
			winding_offset = winding_offset + 2. * 3.1415927;
		}
		az_pt.push_back(azr);
	}
	azr = atan2(ply[iline][npts - 1] - ply[iline][npts - 2], plx[iline][npts - 1] - plx[iline][npts - 2]);
	if (azr - az_pt[npts - 2] > 3.1415927) azr = azr - 2. * 3.1415927;
	if (az_pt[npts - 2] - azr > 3.1415927) azr = azr + 2. * 3.1415927;
	az_pt.push_back(azr);

	if (0) {		// Diagnostics
		for (i = 0; i<npts; i++) {
			fprintf(stdout, "%5.0f ", (180. / 3.14159)* az_pt[i]);
			if (i % 8 == 7) fprintf(stdout, "\n");
		}
		fprintf(stdout, " ********************************************************\n");
	}

	// *********************************************
	// Next calc distances from point to point -- dist_a[j] is distance j-1 to j
	// *********************************************
	float *dist_a = new float[npts];	// Temp storage
	dist_a[0] = 0.;
	for (i = 1; i<npts; i++) {
		dist_a[i] = sqrt((plx[iline][i] - plx[iline][i - 1])*(plx[iline][i] - plx[iline][i - 1]) + (ply[iline][i] - ply[iline][i - 1])*(ply[iline][i] - ply[iline][i - 1])
			+ (plz[iline][i] - plz[iline][i - 1])*(plz[iline][i] - plz[iline][i - 1]));
	}

	// *********************************************
	// Next fill averaged arrays
	// *********************************************
	for (i = 0; i<npts; i++) {
		float sumAz = az_pt[i];		// Count the center point
		float sumDN = ply[iline][i];	// Count the center point
		float sumDE = plx[iline][i];	// Count the center point
		float sumDX = plz[iline][i];	// Count the center point
		int iavg, navg = 1;

		// Shorten averaging interval near ends of the route so maintain balanced avg
		float threshDist = avgRadialDist;
		float sumDist = 0.;
		for (iavg = i; iavg >= 0; iavg--) {
			sumDist = sumDist + dist_a[iavg];
			if (iavg == 0 && sumDist < threshDist) threshDist = sumDist;
			if (sumDist > threshDist) break;
		}
		sumDist = 0.;
		if (i == npts - 1) threshDist = 0.;	// Never goes thru following loop
		for (iavg = i + 1; iavg<npts; iavg++) {
			sumDist = sumDist + dist_a[iavg];
			if (iavg == npts - 1 && sumDist < threshDist) {
				threshDist = sumDist;
			}
			if (sumDist > threshDist) break;
		}

		// Sum over points within range
		sumDist = 0.;
		for (iavg = i; iavg >= 1; iavg--) {
			sumDist = sumDist + dist_a[iavg];
			if (sumDist > threshDist) break;
			sumAz = sumAz + az_pt[iavg - 1];
			sumDN = sumDN + ply[iline][iavg - 1];
			sumDE = sumDE + plx[iline][iavg - 1];
			sumDX = sumDX + plz[iline][iavg - 1];
			navg++;
		}
		sumDist = 0.;
		for (iavg = i + 1; iavg<npts; iavg++) {
			sumDist = sumDist + dist_a[iavg];
			if (sumDist > threshDist) break;
			sumAz = sumAz + az_pt[iavg];
			sumDN = sumDN + ply[iline][iavg];
			sumDE = sumDE + plx[iline][iavg];
			sumDX = sumDX + plz[iline][iavg];
			navg++;
		}

		az_avg.push_back(sumAz / float(navg));
		ply_avg.push_back(sumDN / float(navg));
		plx_avg.push_back(sumDE / float(navg));
		plz_avg.push_back(sumDX / float(navg));
	}
	if (0) {		// Diagnostics
		for (i = 0; i<npts; i++) {
			fprintf(stdout, "%5.0f ", (180. / 3.14159)* az_avg[i]);
			if (i % 8 == 7) fprintf(stdout, "\n");
		}
		fprintf(stdout, " ********************************************************");
	}
	delete[] dist_a;

	return(1);
}

// *************************************************************
/// Add a point.
// *************************************************************
int draw_data_class::add_point(float x, float y, float z, int altMode, int drawPtFlagIn, string iconName, string name, float time)
{
	ppx.push_back(x);
	ppy.push_back(y);
	ppz.push_back(z);
	ppAltMode.push_back(altMode);
	if (!ptDrawPtFlag)   drawPtFlag.push_back(drawPtFlagIn);
	if (entityTimeFlag) pptime.push_back(time);
	if (entityNameFlag) ppname.push_back(name);
	if (ptSymbolFlag) {
		ppISymbol.push_back(-1);		// Default to no symbol
		if (iconName != "") set_symbol_icon(ppx.size() - 1, iconName);
	}
	return(1);
}

// *************************************************************
/// Add an empty line.
/// Defined with no symbol link and no model link and defaults to contains no meaningful times.
/// @param altMode	KML altitude mode: 0=clamp-to-ground (default), 1=relative-to-ground, 2=absolute
/// @param dashFlag	0=solid line, 1=dashed line (only used if lineDashFlag is set)
/// @param name		name (only used if entityNameFlag is set)
// *************************************************************
int draw_data_class::add_empty_line(int altMode, int dashFlag, string name)
{
	plx.push_back(vector<float>());
	ply.push_back(vector<float>());
	plz.push_back(vector<float>());
	if (entityTimeFlag) pltime.push_back(vector<float>());

	plAltMode.push_back(altMode);
	if (entityNameFlag) plname.push_back(name);
	if (lineDashFlag)   pldash.push_back(dashFlag);
	plTimeIndex.push_back(-99);
	pltimeISymbol.push_back(-99);
	pltimeIModel.push_back(-99);
	return(1);
}

// *************************************************************
/// Merge two lines -- append line 2 to the end of line 1.
/// @param iline1	First line to merge
/// @param iline2	Second line to merge
/// @param canFlipFlag	0=append second line to first, 1=flip lines as necessary to get min distance from end of line1 to beginning of line 2
// *************************************************************
int draw_data_class::merge_two_lines(int iline1, int iline2, int canFlipFlag)
{
	int i, np2;
	float xpt1, xpt2, ypt1, ypt2, dist1, dist2, dist3, dist4;
	if (iline1 < 0 || iline2 < 0 || iline1 >= plx.size() || iline2 >= plx.size()) return(0);

	// *************************************************
	// Calculate distances between ends of lines
	// *************************************************
	if (canFlipFlag) {
		// Distance between first index of new file and first index of old files
		xpt1 = plx[iline2][0];
		ypt1 = ply[iline2][0];
		xpt2 = plx[iline1][0];
		ypt2 = ply[iline1][0];
		dist1 = sqrt((xpt1 - xpt2) * (xpt1 - xpt2) + (ypt1 - ypt2) * (ypt1 - ypt2));

		// Distance between last index of new file and first index of old files
		xpt1 = plx[iline2][plx[iline2].size() - 1];
		ypt1 = ply[iline2][ply[iline2].size() - 1];
		xpt2 = plx[iline1][0];
		ypt2 = ply[iline1][0];
		dist2 = sqrt((xpt1 - xpt2) * (xpt1 - xpt2) + (ypt1 - ypt2) * (ypt1 - ypt2));

		// Distance between first index of new file and last index of old files
		xpt1 = plx[iline2][0];
		ypt1 = ply[iline2][0];
		xpt2 = plx[iline1][plx[iline1].size() - 1];
		ypt2 = ply[iline1][ply[iline1].size() - 1];
		dist3 = sqrt((xpt1 - xpt2) * (xpt1 - xpt2) + (ypt1 - ypt2) * (ypt1 - ypt2));

		// Distance between last index of new file and last index of old files
		xpt1 = plx[iline2][plx[iline2].size() - 1];
		ypt1 = ply[iline2][ply[iline2].size() - 1];
		xpt2 = plx[iline1][plx[iline1].size() - 1];
		ypt2 = ply[iline1][ply[iline1].size() - 1];
		dist4 = sqrt((xpt1 - xpt2) * (xpt1 - xpt2) + (ypt1 - ypt2) * (ypt1 - ypt2));

		// *************************************************
		// Flip if necessary
		// *************************************************
		if (dist1 < dist2 && dist1 < dist3 && dist1 < dist4) {	// Min distance is first index of new file and first index of old files -- flip first line
			warning(1, "Flipping order of first line to merge into single track");
			flip_line(iline1);
		}
		else if (dist2 < dist1 && dist2 < dist3 && dist2 < dist4) {	// Min distance is last index of new file and first index of old files -- flip both
			warning(1, "Flipping order of both lines to merge into single track");
			flip_line(iline1);
			flip_line(iline2);
		}
		else if (dist3 < dist1 && dist3 < dist2 && dist3 < dist4) {	// Min distance is first index of new file and last index of old files -- no flip
		}
		else if (dist4 < dist1 && dist4 < dist2 && dist4 < dist3) {	// Min distance is last index of new file and last index of old files -- flip new file
			warning(1, "Flipping order of second line to merge into single track");
			flip_line(iline2);
		}

	}

	// Append line2 to line1
	np2 = plx[iline2].size();
	for (i = 0; i < np2; i++) {
		plx[iline1].push_back(plx[iline2][i]);
		ply[iline1].push_back(ply[iline2][i]);
		plz[iline1].push_back(plz[iline2][i]);
		if (entityTimeFlag) pltime[iline1].push_back(pltime[iline2][i]);
	}

	// Delete the second line
	delete_line(iline2);
	return(1);
}

// *************************************************************
/// Flip a line.
// *************************************************************
int draw_data_class::flip_line(int iline)
{
	int i, nmid, npt;
	float xt, yt, zt, tt;

	npt = plx[iline].size();
	nmid = plx[iline].size() / 2;
	for (i = 0; i < nmid; i++) {
		xt = plx[iline][i];
		yt = ply[iline][i];
		zt = plz[iline][i];
		if (entityTimeFlag) tt = pltime[iline][i];

		plx[iline][i] = plx[iline][npt - 1 - i];
		ply[iline][i] = ply[iline][npt - 1 - i];
		plz[iline][i] = plz[iline][npt - 1 - i];
		if (entityTimeFlag) pltime[iline][i] = pltime[iline][npt - 1 - i];

		plx[iline][npt - 1 - i] = xt;
		ply[iline][npt - 1 - i] = yt;
		plz[iline][npt - 1 - i] = zt;
		if (entityTimeFlag) pltime[iline][npt - 1 - i] = tt;
	}
	return(1);
}

// *************************************************************
/// Move a point.
/// @param	ipt		Point number
/// @param	delx	Translation
/// @param	dely	Translation
/// @param	followEarthFlag	If set and elevation relative to ground, keep same distance from new ground location at each point
// *************************************************************
int draw_data_class::move_point(int ipt, float delx, float dely, int followEarthFlag)
{
	float zold=0., znew=0.;
	if (ipt < 0 || ipt >= ppx.size()) return 0;
	if (followEarthFlag) calc_elev(ppx[ipt], ppy[ipt], zold, ppAltMode[ipt], zold);
	ppx[ipt] = ppx[ipt] + delx;
	ppy[ipt] = ppy[ipt] + dely;
	if (followEarthFlag) calc_elev(ppx[ipt], ppy[ipt], znew, ppAltMode[ipt], znew);
	ppz[ipt] = ppz[ipt] + znew - zold;
	return(1);
}

// *************************************************************
/// Move a line.
/// @param	iline	Line number
/// @param	delx	Translation
/// @param	dely	Translation
/// @param	followEarthFlag	If set and elevation relative to ground, keep same distance from new ground location at each point
// *************************************************************
int draw_data_class::move_line(int iline, float delx, float dely, int followEarthFlag)
{
	float zold = 0., znew = 0.;
	if (iline < 0 || iline >= plx.size()) return 0;
	for (int ipt = 0; ipt < plx[iline].size(); ipt++) {
		if (followEarthFlag) calc_elev(plx[iline][ipt], ply[iline][ipt], zold, plAltMode[iline], zold);
		plx[iline][ipt] = plx[iline][ipt] + delx;
		ply[iline][ipt] = ply[iline][ipt] + dely;
		if (followEarthFlag) calc_elev(plx[iline][ipt], ply[iline][ipt], znew, plAltMode[iline], znew);
		plz[iline][ipt] = plz[iline][ipt] + znew - zold;
	}
	return(1);
}

// *************************************************************
/// Delete a point.
// *************************************************************
int draw_data_class::delete_point(int ipt)
{
	if (ipt < 0 || ipt >= ppx.size()) return 0;
	ppx.erase(ppx.begin() + ipt);
	ppy.erase(ppy.begin() + ipt);
	ppz.erase(ppz.begin() + ipt);
	ppAltMode.erase(ppAltMode.begin() + ipt);
	if (entityNameFlag) ppname.erase(ppname.begin() + ipt);
	if (entityTimeFlag) pptime.erase(pptime.begin() + ipt);
	if (!ptDrawPtFlag)   drawPtFlag.erase(drawPtFlag.begin() + ipt);
	if (ptSymbolFlag)   ppISymbol.erase(ppISymbol.begin() + ipt);
	return(1);
}

// *************************************************************
/// Delete a line.
// *************************************************************
int draw_data_class::delete_line(int iline)
{
	if (iline < 0 || iline >= plx.size()) return 0;
	plx.erase(plx.begin() + iline);
	ply.erase(ply.begin() + iline);
	plz.erase(plz.begin() + iline);
	plAltMode.erase(plAltMode.begin() + iline);
	if (entityNameFlag) plname.erase(plname.begin() + iline);
	if (entityTimeFlag) pltime.erase(pltime.begin() + iline);
	if (lineDashFlag)   pldash.erase(pldash.begin() + iline);
	return(1);
}

// *************************************************************
/// Delete a point in a line.
// *************************************************************
int draw_data_class::delete_point_from_line(int iline, int ipt)
{
	if (iline < 0 || iline >= plx.size()) return 0;
	if (ipt   < 0 || ipt   >= plx[iline].size()) return 0;
	plx[iline].erase(plx[iline].begin() + ipt);
	ply[iline].erase(ply[iline].begin() + ipt);
	plz[iline].erase(plz[iline].begin() + ipt);
	if (entityTimeFlag) pltime[iline].erase(pltime[iline].begin() + ipt);
	return(1);
}

// *************************************************************
/// Delete a polygon.
// *************************************************************
int draw_data_class::delete_polygon(int ipol)
{
	if (ipol < 0 || ipol >= pbx.size()) return 0;
	pbx.erase(pbx.begin() + ipol);
	pby.erase(pby.begin() + ipol);
	pbz.erase(pbz.begin() + ipol);
	pbfill.erase(pbfill.begin() + ipol);
	pbAltMode.erase(pbAltMode.begin() + ipol);
	if (entityNameFlag) pbname.erase(pbname.begin() + ipol);
	if (entityTimeFlag) pbtime.erase(pbtime.begin() + ipol);
	return(1);
}

// *******************************************
/// Given the line number and index into that line, get the location and direction of travel.
/// @param iline					Input line number
/// @param time_interval_show		Input float index into line
/// @param x						Output location
/// @param y						Output location
/// @param z						Output location
/// @param az						Output heading angle
// *******************************************
int draw_data_class::get_loc_along_line_at_index(int iline, float index, float &x, float &y, float &z, float &az)
{
	int j;
	float azr;
	int npts = plx[iline].size();

	// **********************************************
	// No track defined yet -- just go to origin
	// **********************************************
	if (npts == 0) {
		x = 0.;
		y = 0.;
		z = 0.;
		az = 0.;
		return(0);
	}

	// **********************************************
	// Time less than first point
	// **********************************************
	if (index == 0.) {
		x = plx[iline][0];
		y = ply[iline][0];
		z = plz[iline][0];
		if (npts >= 2) {
			azr = atan2(ply[iline][1] - ply[iline][0], plx[iline][1] - plx[iline][0]);
		}
		else {
			azr = 0.;
		}
		az = (180. / 3.14159) * azr;
		return(0);
	}

	// **********************************************
	// Time greater than last point
	// **********************************************
	if (index == npts - 1) {
		x = plx[iline][npts - 1];
		y = ply[iline][npts - 1];
		z = plz[iline][npts - 1];
		if (npts >= 2) {
			azr = atan2(ply[iline][npts - 1] - ply[iline][npts - 2], plx[iline][npts - 1] - plx[iline][npts - 2]);
		}
		else {
			azr = 0.;
		}
		az = (180. / 3.14159) * azr;
		return(0);
	}

	// **********************************************
	// Interpolate to get output location
	// **********************************************
	j = int(index);
	float del = index - int(index);
	if (avgFlag) {
		x = plx_avg[j] + (plx_avg[j + 1] - plx_avg[j]) * del;
		y = ply_avg[j] + (ply_avg[j + 1] - ply_avg[j]) * del;
		z = plz_avg[j] + (plz_avg[j + 1] - plz_avg[j]) * del;
		azr = az_avg[j] +  (az_avg[j + 1] -  az_avg[j]) * del;
		az = (180. / 3.14159) * azr;
	}
	else {
		x = plx[iline][j] + (plx[iline][j + 1] - plx[iline][j]) * del;
		y = ply[iline][j] + (ply[iline][j + 1] - ply[iline][j]) * del;
		z = plz[iline][j] + (plz[iline][j + 1] - plz[iline][j]) * del;
		azr = atan2(ply[iline][j + 1] - ply[iline][j], plx[iline][j + 1] - plx[iline][j]);
		az = (180. / 3.14159) * azr;
	}
	return(1);
}

// *************************************************************
/// Calculate the true elevation of a point where the altitude-mode determines whether the point is absolute, clamped to ground or relative to ground -- private.
/// Either hi-res or low-res map elevations can be used to determine ground elevations in the clamp-to-ground or relative-to-ground modes.
/// @param xin		input location relative to map reference point
/// @param yin		input location relative to map reference point
/// @param zin		input location relative to map reference point
/// @param altMode	altitude mode -- 0=clamp to ground, 1=rel to ground, 2=absolute, 3=absolute but with my later adjustments
// *************************************************************
int draw_data_class::calc_elev(float xin, float yin, float zin, int altMode, float &zout)
{
	double north, east;
	float zground;

	if (altMode == 2 || altMode == 3) {											// Absolute elevation (3=adjusted from relative-to-feature to absolute in vector_layer_class)
		zout = zin;
	}
	else {
		north = gps_calc->get_ref_utm_north() + yin;
		east = gps_calc->get_ref_utm_east() + xin;
		if (map3d_index->is_map_defined() && useLowresElevFlag == 0) {
			if (!map3d_index->get_elev_at_pt(north, east, zground)) {
				zground = map3d_lowres->get_lowres_elev_at_loc(north, east);	// Point not on map, so cant find precise elevation -- use lowres
			}
		}
		else  {
			zground = map3d_lowres->get_lowres_elev_at_loc(north, east);
		}
		if (altMode == 0) {														// clampToGround
			zout = zground;
		}
		else {																	// relativeToGround
			zout = zground + zin;
		}

	}
	return(1);
}

// *************************************************************
/// Calculate the extent of the polygons in x,y.
// *************************************************************
int draw_data_class::get_bb(float &bb_xmin, float &bb_xmax, float &bb_ymin, float &bb_ymax)
{
	int ip, i;
	if (pbx.size() == 0) return(1);

	// If bb not yet calculated, do it
	if (pxmin == -99. && pxmax == -99.) {
		pxmin = pbx[0][0];
		pxmax = pbx[0][0];
		pymin = pby[0][0];
		pymax = pby[0][0];
		for (ip = 0; ip < pbx.size(); ip++) {
			for (i = 0; i < pbx[ip].size(); i++) {
				if (pxmin > pbx[ip][i]) pxmin = pbx[ip][i];
				if (pxmax < pbx[ip][i]) pxmax = pbx[ip][i];
				if (pymin > pby[ip][i]) pymin = pby[ip][i];
				if (pymax < pby[ip][i]) pymax = pby[ip][i];
			}
		}
	}

	bb_xmin = pxmin;
	bb_xmax = pxmax;
	bb_ymin = pymin;
	bb_ymax = pymax;
	return(1);
}

// *************************************************************
/// Set location and size of elevation deconflict mask.
/// @param	xmin	edge of mask
/// @param	xmax	edge of mask
/// @param	ymin	edge of mask
/// @param	ymax	edge of mask
/// @param	res		pixel size in m
// *************************************************************
int draw_data_class::set_mask_loc(float xmin, float xmax, float ymin, float ymax, float res)
{
	mask_xmin = xmin;
	mask_xmax = xmax;
	mask_ymin = ymin;
	mask_ymax = ymax;
	mask_res = res;
	mask_nx = (mask_xmax - mask_xmin) / res;
	mask_ny = (mask_ymax - mask_ymin) / res;
	return(1);
}

// *************************************************************
/// Set value of elevation offset used for drawing and also for deconflict.
/// Value used for drawing and deconflict.
// *************************************************************
int draw_data_class::set_elev_offset(float zoff)
{
	zOffset = zoff;
	return(1);
}

// *************************************************************
/// Set value of xy offsets (easting and northing in projection coordinates).
/// Values used for drawing and deconflict.
// *************************************************************
int draw_data_class::set_xy_offsets(double xoff, double yoff)
{
	xOffset = xoff;
	yOffset = yoff;
	return(1);
}

// *************************************************************
/// Calculate the deconflict elevation mask and return it.
/// This mask is used to push DEM pixels just below the level of a CAD model so they dont conflict with it.
/// Mask value zero indicates no pol intersects.
/// @param mask_deconflict	input/output float mask containing min z-value of all pols that intersect each pixel (zero indicates no pol intersects).
// *************************************************************
int draw_data_class::get_deconflict_mask(float* mask_deconflict)
{
	int ipol, iv, iv2, ix, iy, ix1, ix2, iy1, iy2, ixp, iyp, imask, ninter, nmaskset=0;
	float xm, ym, xinter, frac;
	float pol_xmin, pol_xmax, pol_ymin, pol_ymax, pol_zmin;
	if (pbx.size() == 0) return(0);

	// ****************************************************
	// Loop over all pols -- Use bounding box to limit search region
	// ****************************************************
	for (ipol = 0; ipol < pbx.size(); ipol++) {
	//for (ipol = 0; ipol < 11; ipol++) {
		if (pbx[ipol].size() < 3) {
			continue;			// There shouldnt be any, but just in case
		}
		pol_xmin = pbx[ipol][0];
		pol_xmax = pbx[ipol][0];
		pol_ymin = pby[ipol][0];
		pol_ymax = pby[ipol][0];
		pol_zmin = pbz[ipol][0];
		for (iv = 1; iv < pbx[ipol].size(); iv++) {
			if (pol_xmin > pbx[ipol][iv]) pol_xmin = pbx[ipol][iv];
			if (pol_xmax < pbx[ipol][iv]) pol_xmax = pbx[ipol][iv];
			if (pol_ymin > pby[ipol][iv]) pol_ymin = pby[ipol][iv];
			if (pol_ymax < pby[ipol][iv]) pol_ymax = pby[ipol][iv];
			if (pol_zmin > pbz[ipol][iv]) pol_zmin = pbz[ipol][iv];
		}

		if (pol_zmin < -1000.) {
			cout << "pol_zmin=" << pol_zmin << endl;
		}

		ix1 = int((pol_xmin - mask_xmin) / mask_res);
		ix2 = int((pol_xmax - mask_xmin) / mask_res);
		iy1 = int((mask_ymax - pol_ymax) / mask_res);
		iy2 = int((mask_ymax - pol_ymin) / mask_res);
		if (ix1 < 0 || iy1 < 0 || ix2 >= mask_nx || iy2 >= mask_ny) {
			cout << "CAD deconflict error1" << endl;
		}

		// ****************************************************
		// Loop over all 4 corners of mask pixels within bounding box
		// ****************************************************
		for (iy = iy1+1; iy <= iy2; iy++) {
			ym = mask_ymax - iy * mask_res;
			for (ix = ix1+1; ix <= ix2; ix++) {
				xm = mask_xmin + ix * mask_res;


				// Shoot ray from pixel center parallel to positive x-axis and count intersections
				ninter = 0;
				for (iv = 0; iv < pbx[ipol].size(); iv++) {
					iv2 = iv + 1;
					if (iv2 == pbx[ipol].size()) iv2 = 0;
					if (pby[ipol][iv] >= ym && pby[ipol][iv2] < ym || pby[ipol][iv] <= ym && pby[ipol][iv2] > ym) {
						frac = (pby[ipol][iv] - ym) / (pby[ipol][iv] - pby[ipol][iv2]);
						xinter = (1.0 - frac) * pbx[ipol][iv] + frac * pbx[ipol][iv2];
						if (xinter > xm) ninter++;
					}
				}

				// Odd number of intersections -- point interior -- set mask to min z of any pol it intersects
				if (ninter % 2 == 1) {
					for (iyp = iy - 1; iyp <= iy; iyp++) {
						for (ixp = ix - 1; ixp <= ix; ixp++) {
							imask = iyp * mask_nx + ixp;
							if (mask_deconflict[imask] == 0) {
								mask_deconflict[imask] = pol_zmin + zOffset;
							}
							else {
								if (mask_deconflict[imask] > pol_zmin + zOffset) mask_deconflict[imask] = pol_zmin + zOffset;
							}
							nmaskset++;
						}
					}
				}
			}				// End loop over x
		}					// End loop over y
		if (ipol % 10 == 0) cout << ipol << " deconflict -- no. of mask pixels=" << nmaskset << endl;
	}						// End loop over pols


	return(1);
}

// *************************************************************
/// Builds a list of unique texture image filenames and returns the index into that list for the given texture filename.
/// @param iMaterial		Material index (used only for diagnostic printout)
/// @param filename			Input texture image filename
/// @return					Returns index into list of filenames, -1 if input filename does not exist
// *************************************************************
int draw_data_class::assign_tex_file_index(int iMaterial, string filename)
{
	int i;

	// **********************************************
	// Return -1 if file does not exist
	// **********************************************
	if (!check_file_exists(filename)) {
		cout << "Mat " << iMaterial << " No texture file " << endl;
		return(-1);
	}

	// **********************************************
	// Return index if filename already stored
	// **********************************************
	for (i=0; i<assimpUniqueTexNames.size(); i++) {
		if (filename.compare(assimpUniqueTexNames[i]) == 0) {
			cout << "Mat " << iMaterial << " tex Reused " << assimpUniqueTexNames[i] << endl;
			return(i);
		}
	}

	// **********************************************
	// New filename -- store and return new index
	// **********************************************
	assimpUniqueTexNames.push_back(filename);
	int iRet = assimpUniqueTexNames.size() - 1;
	cout << "Mat " << iMaterial << " tex New    " << assimpUniqueTexNames[iRet] << endl;
	return(iRet);
}

