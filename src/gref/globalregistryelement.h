/* $Id: globalregistryelement.h,v 1.1.2.4 2009/11/08 17:34:01 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#ifndef __GLOBALREGISTRYELEMENT_H__
#define __GLOBALREGISTRYELEMENT_H__

#include "dlobject.h"
#include "dlstring.h"
#include "lang.h"

class GlobalRegistryElement : public virtual DLObject {
friend class GlobalRegistryBase;
public:
    typedef ::Pointer<GlobalRegistryElement> Pointer;
    
    GlobalRegistryElement( );
    virtual ~GlobalRegistryElement( );
    
    virtual const DLString &getName( ) const = 0;
    virtual const DLString &getRussianName( ) const;
    virtual const DLString &getUkrainianName( ) const;

    // Name in the viewer's display language. Default composes it from the three
    // getters above (EN name / UA-or-RU / RU); elements with a richer per-language
    // store (Skill, SkillGroup) override this. Callers that render registry names
    // for a viewer -- GlobalArray::toStringList, GlobalBitvector::toString -- go
    // through here so UA stops collapsing into RU.
    virtual const DLString &getNameFor( lang_t lang ) const;
    
    virtual bool matchesStrict( const DLString &str ) const;
    virtual bool matchesUnstrict( const DLString &str ) const;
    virtual bool matchesSubstring( const DLString &str ) const;

    inline int getIndex( ) const
    {
        return index;
    }
    
    virtual bool isValid( ) const 
    {
        return true;
    }

protected:
    inline void setIndex( int i )
    {
        index = i;
    }

private:
    int index;
};

#endif
