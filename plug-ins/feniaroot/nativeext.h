/* $Id: nativeext.h,v 1.1.2.4 2005/04/27 18:46:13 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef NATIVEEXT_H
#define NATIVEEXT_H

#include <sstream>

#include "native.h"

using namespace std;

// MOC_SKIP_BEGIN
namespace Scripting {

template <typename TT>
void traitsAPIAux( ostringstream &buf ) 
{
    typename TT::List *list = TT::List::begin();
    
    for ( ; list; list = list->getNext())
        buf << "{g" << list->getKey().name 
            << "{x: " << list->getVal().help << endl;
}

template <typename T>
void traitsAPI( ostringstream &buf ) 
{
    typedef NativeTraits<T> Traits;
    
    buf << "{Y" << Traits::NAME << "{x" << endl
        << Traits::HELP << endl;
    
    buf << endl << "{WПоля, доступные для записи: {x" << endl;
    traitsAPIAux<typename Traits::Set>( buf );
    buf << endl << "{WПоля, доступные для чтения: {x" << endl;
    traitsAPIAux<typename Traits::Get>( buf );
    buf << endl << "{WМетоды: {x" << endl;
    traitsAPIAux<typename Traits::Invoke>( buf );
}

}

// MOC_SKIP_END
#endif 
