#ifndef _draw_data_inv_class_h
#define _draw_data_inv_class_h
#ifdef __cplusplus

/**
Adds OpenInventor functionality to draw_data_class.
Adds capability to draw the objects that the class imports.
Currently implemented to draw points, lines and polygons and the symbols associated with points.

*/
class draw_data_inv_class:public draw_data_class{
  private:
	  SoSeparator**		symbolBase;				///< Per military symbol, base
	  SoDrawStyle*		fillDrawStyle;			///< For filled polygons
	  SoDrawStyle*		solidLineDrawStyle;		///< For normal solid lines
	  SoDrawStyle*		dashLineDrawStyle;		///< For dashed lines
	  SoDrawStyle*		ptsDrawStyle;			///< For showing points that make up lines or polygons

	  int nSymbolsMade;							///< No. of symbols that have been read and made into subtrees
	  //float d_above_ground;						///< Draw objects this distance above ground
	  int outputSymbolWidth;					///< Desired size of drawn symbols in pixels


    // Private methods
	int make_symbol(int iSymbolMake);
	int draw_assimp(string filename, SoSeparator* inSep);
	int read_tex_image(string filename, SoTexture2 *tex2);
	
   public:
    draw_data_inv_class();
    ~draw_data_inv_class();
	//int set_d_above_ground(float distance);
	int set_symbol_width(int widthInPixels);
	int set_symbol_base(SoSeparator** symbolBaseIn);

	int draw_all(SoBaseColor *color, SoSeparator* inSep);
	int draw_point   (int ipt, int externalColorFlag, int noShowNameFlag, SoBaseColor *color, SoSeparator* gSep, SoSeparator* tSep, SoSeparator* sSep);
	int draw_line  (int iline, int externalColorFlag, int showPointsFlag, SoBaseColor *color, SoSeparator* inSep);
	int draw_polygon(int ipol, int externalColorFlag, int showPointsFlag, SoBaseColor *color, SoSeparator* inSep);
	int draw_line_mover(int iline, float time, SoSeparator* inSep);
	int update_symbols();

	int clear();
};

#endif /* __cplusplus */
#endif /* _draw_data_inv_class_ */
