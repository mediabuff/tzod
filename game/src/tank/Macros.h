// Macros.h

#pragma once

///////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(p)       { if(p) { delete (p); (p)=NULL; } }
#define SAFE_KILL(w, p)      { if(p) (p)->Kill(w, w.GetId(p)); (p)=NULL; }

///////////////////////////////////////////////////////////////////////////////

#define FOREACH(ls, type, var)                                              \
	if( type *var = (type *) (0xffffffff) )                                 \
	for( PtrList<GC_Object>::id_type var##iterator = (ls).begin();          \
	         var##iterator != (ls).end() &&                                 \
             ((var = static_cast<type *>((ls).at(var##iterator))), true);   \
	    var##iterator = (ls).next(var##iterator))


///////////////////////////////////////////////////////////////////////////////
// end of file
