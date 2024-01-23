/* $Id: socialbase.h,v 1.1.2.1.6.3 2008/03/23 02:26:02 rufina Exp $
 *
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */
#ifndef SOCIALBASE_H
#define SOCIALBASE_H

#include "commandbase.h"

class SocialBase : public CommandBase {
public:        
    typedef ::Pointer<SocialBase> Pointer;

    SocialBase( );
    virtual ~SocialBase( );
    
    virtual const DLString &getRussianName( ) const = 0;
    virtual short getLog( ) const;

    virtual bool matches( const DLString & ) const;
    virtual int properOrder( Character * ) const;
    virtual int dispatch( const InterpretArguments & );
    virtual int dispatchOrder( const InterpretArguments & );
    virtual void entryPoint( Character *, const DLString & );
    virtual void run( Character *, const DLString & );
    
    virtual const DLString & getNoargOther( ) const = 0;
    virtual const DLString & getNoargMe( ) const = 0;
    virtual const DLString & getAutoOther( ) const = 0;
    virtual const DLString & getAutoMe( ) const = 0;
    virtual const DLString & getArgOther( ) const = 0;
    virtual const DLString & getArgMe( ) const = 0;
    virtual const DLString & getArgVictim( ) const = 0;
    virtual const DLString & getErrorMsg( ) const = 0;
    inline virtual const DLString & getArgOther2( ) const { return DLString::emptyString; }
    inline virtual const DLString & getArgMe2( ) const { return DLString::emptyString; }
    inline virtual const DLString & getArgVictim2( ) const { return DLString::emptyString; }
    inline virtual const DLString & getObjVictim() const { return DLString::emptyString; }
    inline virtual const DLString & getObjChar() const { return DLString::emptyString; }
    inline virtual const DLString & getObjOthers() const { return DLString::emptyString; }
    inline virtual const DLString & getObjNoVictimSelf() const { return DLString::emptyString; }
    inline virtual const DLString & getObjNoVictimOthers() const { return DLString::emptyString; }


protected:    
    virtual bool reaction( Character *, Character *, const DLString & ) = 0;
    virtual int getPosition( ) const = 0;
    void visualize( Character * );
    bool checkPosition( Character * );
};

#endif

