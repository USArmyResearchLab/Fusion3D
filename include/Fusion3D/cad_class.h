/** @file */
#ifndef _cad_class_h_
#define _cad_class_h_	
#ifdef __cplusplus

/**
Imports and exports files in CAD formats.
   
Implemented using the assimp library which is a uniform interface into a number of different CAD formats including .obj and collada.
   

*/
class cad_class:public vector_layer_class{
   
   private:
      
      // Private methods
      
   public:
	cad_class();
      ~cad_class();
      
	  bool canHandleThisFormat(string filename);
	  int read_file(string sfilename) override;
	  int write_file(string sfilename) override;
};

#endif /* __cplusplus */
#endif /* _cad_class_h_ */
