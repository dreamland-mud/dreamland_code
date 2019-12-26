
#include "servlet.h"
#include "codesource.h"
#include "register-impl.h"

using namespace Scripting;

SERVLET_HANDLE(fenia_eval, "/eval")
{

    try {
        if(request.method != "POST")
            throw ::Exception("Expecting POST method");
#if 0
        CodeSource &cs = CodeSource::manager->allocate();
        
        cs.author = "Filths";
        cs.name = "<eval servlet>";

        cs.content = request.body;
        Register rc = cs.eval(Register());
#endif
        response.status = 200;
        response.message = "OK";
#if 0
        response.body = rc.toString();
#endif
    
    } catch (const ::Exception &e) {
        response.status = 500;
        response.message = "Eval failed";
        response.body = e.what( );
    }
}
