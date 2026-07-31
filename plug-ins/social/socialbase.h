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
#include "multimessage.h"

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

    /* Trilinguality (Trello 2594): what a recipient actually sees, resolved in
     * their display language by the act/pecho MultiMessage overloads. Socials
     * keep their translations as DATA (XMLMultiString per message) rather than
     * in the catalog, so Social overrides these with its per-language values.
     * The default wraps the single stored string -- which is all a
     * player-authored CustomSocial has, and all it will ever have. */
    virtual MultiMessage getNoargOtherMsg( ) const;
    virtual MultiMessage getNoargMeMsg( ) const;
    virtual MultiMessage getAutoOtherMsg( ) const;
    virtual MultiMessage getAutoMeMsg( ) const;
    virtual MultiMessage getArgOtherMsg( ) const;
    virtual MultiMessage getArgMeMsg( ) const;
    virtual MultiMessage getArgVictimMsg( ) const;
    virtual MultiMessage getErrorMsgMsg( ) const;
    virtual MultiMessage getArgOther2Msg( ) const;
    virtual MultiMessage getArgMe2Msg( ) const;
    virtual MultiMessage getArgVictim2Msg( ) const;
    virtual MultiMessage getObjVictimMsg( ) const;
    virtual MultiMessage getObjCharMsg( ) const;
    virtual MultiMessage getObjOthersMsg( ) const;
    virtual MultiMessage getObjNoVictimSelfMsg( ) const;
    virtual MultiMessage getObjNoVictimOthersMsg( ) const;

protected:
    virtual bool reaction( Character *, Character *, const DLString & ) = 0;
    virtual int getPosition( ) const = 0;
    void visualize( Character * );
    bool checkPosition( Character * );

    /** One string standing in for all three languages: the same text whatever
     *  the viewer, with no catalog lookup. */
    static MultiMessage plain( const DLString & );
};

#endif

