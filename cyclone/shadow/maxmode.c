/* Copyright (c) 2004 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "unstable/loader.h"

void maxmode_setup(void)
{
    shared_setmaxcompatibility();
    if (zgetfn(&pd_objectmaker, gensym("cyclone")))
	loud_warning(0, "maxmode", "cyclone is already loaded");
    else
    {
	if (unstable_load_lib("", "cyclone") == LOADER_NOFILE)
	    loud_error(0, "cyclone library is missing");
	else if (!zgetfn(&pd_objectmaker, gensym("cyclone")))
	    loud_error(0, "miXed/Pd version mismatch");
    }
}
