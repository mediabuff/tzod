// Macros.h

#pragma once

///////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#define SAFE_KILL(p)         { if(p) { (p)->Kill();    (p)=NULL; } }

///////////////////////////////////////////////////////////////////////////////

#define FOREACH(ls, type, var)                                          \
	if( type *var = (type *) (0xffffffff) )                             \
	for( OBJECT_LIST::iterator var##iterator = (ls).begin();            \
	         (var = static_cast<type *>(*var##iterator)),               \
	         (var##iterator != (ls).end());                             \
	    var##iterator++)

#define FOREACH_R(ls, type, var)                                        \
	if( type *var = (type *) (0xffffffff) )                             \
	for( OBJECT_LIST::reverse_iterator var##iterator = (ls).rbegin();   \
	      (var = static_cast<type *>(*var##iterator)),                  \
	      (var##iterator != (ls).rend());                               \
	    var##iterator++ )


///////////////////////////////////////////////////////////////////////////////

#define WIDTH(rect) (rect.right - rect.left)
#define HEIGHT(rect) (rect.bottom - rect.top)

///////////////////////////////////////////////////////////////////////////////
// end of file
