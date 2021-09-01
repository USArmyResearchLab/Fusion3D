#ifndef _bookmark_manager_class_h_
#define _bookmark_manager_class_h_	
#ifdef __cplusplus

/**
Manages all bookmarks.

When the make-bookmarks option is enabled, one can make a bookmark by middle-mouse clicking on a location.
On clicking, a menu allows the user to name the bookmark.
Bookmarks can be read from and written to a KML file.

*/

class bookmark_manager_class:public atrlab_manager_class{
   protected:
      float d_above_ground;						///< Draw bookmark this dist above ground (default 2m)
	  draw_data_inv_class *draw_data;			///< Helper class -- store and draw bookmarks

public:
      bookmark_manager_class();
      ~bookmark_manager_class();
      
      int read_tagged(const char* filename);
      int read_file(string sfilename) override;
      int write_file(string sfilename) override;
      int write_parms(FILE *out_fd) override;
};

#endif /* __cplusplus */
#endif /* _bookmark_manager_class_h_ */
