/* $Id: wrappertarget.h,v 1.1.2.2.18.6 2009/11/04 16:06:37 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __WRAPPERTARGET_H__
#define __WRAPPERTARGET_H__

#include "fenia/register-decl.h"

class WrapperBase;

class WrapperTarget {
public:

    WrapperTarget( );

    WrapperBase * getWrapper( );

    void extractWrapper(bool forever);

    Scripting::Object *wrapper;
};

#define BASE_VOID_CALL( base, id, fmt...) \
        if (base) { \
            static Scripting::IdRef onId( DLString("on")+id ); \
            base->call( onId, fmt ); \
            \
            static Scripting::IdRef postId( DLString("post")+id ); \
            base->postpone( postId, fmt ); \
        }

#define BASE_BOOL_CALL( base, id, fmt...) \
        if (base) { \
            bool rc; \
            static Scripting::IdRef onId( "on" id ); \
            rc = base->call( onId, fmt ); \
            \
            static Scripting::IdRef postId( "post" id ); \
            base->postpone( postId, fmt ); \
            \
            if (rc) return rc; \
        }

#define BASE_STR_CALL( base, id, fmt...) \
        if (base) { \
            DLString rc; \
            static Scripting::IdRef onId( "on" id ); \
            rc = base->stringCall( onId, fmt ); \
            \
            static Scripting::IdRef postId( "post" id ); \
            base->postpone( postId, fmt ); \
            \
            if (!rc.empty()) return rc; \
        }

#define FENIA_CALL( var, id, fmt...) \
        if (var) { \
            WrapperBase *base = var->getWrapper( ); \
            BASE_BOOL_CALL(base, id, fmt); \
        }

#define FENIA_NDX_CALL( var, id, fmt...) \
        if (var && var->pIndexData) { \
            WrapperBase *base = get_wrapper( var->pIndexData->wrapper ); \
            BASE_BOOL_CALL(base, id, fmt); \
        }

#define FENIA_VOID_CALL( var, id, fmt...) \
        if (var) { \
            WrapperBase *base = var->getWrapper( ); \
            BASE_VOID_CALL(base, id, fmt); \
        }

#define FENIA_NDX_VOID_CALL( var, id, fmt...) \
        if (var && var->pIndexData) { \
            WrapperBase *base = get_wrapper( var->pIndexData->wrapper ); \
            BASE_VOID_CALL(base, id, fmt); \
        }

#define FENIA_STR_CALL( var, id, fmt...) \
        if (var) { \
            WrapperBase *base = var->getWrapper( ); \
            BASE_STR_CALL(base, id, fmt); \
        }

#define FENIA_NDX_STR_CALL( var, id, fmt...) \
        if (var && var->pIndexData) { \
            WrapperBase *base = get_wrapper( var->pIndexData->wrapper ); \
            BASE_STR_CALL(base, id, fmt); \
        }

#define FENIA_NDX_HAS_TRIGGER( var, id ) \
        if (var && var->pIndexData) { \
            WrapperBase *base = get_wrapper( var->pIndexData->wrapper ); \
            bool rc = base ? base->hasTrigger(id) : false; \
            if (rc) return rc; \
        }

#define FENIA_HAS_TRIGGER( var, id ) \
        if (var) { \
            WrapperBase *base = var->getWrapper( ); \
            bool rc = base ? base->hasTrigger(id) : false; \
            if (rc) return rc; \
        }

#endif
