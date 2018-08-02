/* $Id: schedulerwrapperext.cpp,v 1.1.2.2 2005/04/27 18:46:13 rufina Exp $
 *
 * ruffina, 2004
 */
#include <sstream>

#include "register-impl.h"
#include "nativeext.h"
#include "schedulerwrapper.h"

using namespace Scripting;

NMI_INVOKE(FeniaProcess, api, "")
{
    ostringstream buf;
    
    traitsAPI<FeniaProcess>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(SchedulerWrapper, api, "")
{
    ostringstream buf;
    
    traitsAPI<SchedulerWrapper>( buf );
    return Register( buf.str( ) );
}



