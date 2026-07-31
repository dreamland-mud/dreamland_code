/* $Id: social.h,v 1.1.2.1.6.3 2008/05/20 22:11:38 rufina Exp $
 *
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */
#ifndef SOCIAL_H
#define SOCIAL_H

#include "lang.h"
#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlmultistring.h"
#include "xmlstringlist.h"
#include "xmlenumeration.h"
#include "xmltableelement.h"
#include "socialbase.h"
#include "xmlpointer.h"
#include "xmlloader.h"
#include "markuphelparticle.h"

class Social;

class SocialHelp : public MarkupHelpArticle {
public:
    typedef ::Pointer<SocialHelp> Pointer;
    
    SocialHelp();
    virtual ~SocialHelp();

    void setSocial(::Pointer<Social> social);
    void unsetSocial();
    virtual void save() const;

    virtual DLString getTitle(const DLString &label) const;
    virtual DLString getTitle(const DLString &label, lang_t lang) const;
    inline virtual const DLString & getType( ) const;
    static const DLString TYPE;

protected:
    virtual void getRawText( Character *, ostringstream & ) const;
    ::Pointer<Social> social;
};

inline const DLString & SocialHelp::getType( ) const
{
    return TYPE;
}

class Social : public SocialBase, public XMLVariableContainer, 
               public XMLTableElement 
{
XML_OBJECT
public:        
    typedef ::Pointer<Social> Pointer;

    Social( );
    virtual ~Social( );
    
    virtual void loaded( );
    virtual void unloaded( );
    virtual bool matches( const DLString & ) const;
    inline virtual const DLString &getName( ) const;
    inline virtual void setName( const DLString & );
    inline virtual const DLString &getRussianName( ) const;
    inline virtual const DLString &getUaName( ) const;
    /** The name in a given language, falling back to RU then the Latin
     *  keyword. Socials carry three separate names rather than one
     *  XMLMultiString because the Latin keyword is also the registry key. */
    virtual const DLString &getNameFor( lang_t lang ) const;
    inline const DLString &getShortDesc( ) const;
    /** Short description in a given language, RU fallback. */
    inline const DLString &getShortDescFor( lang_t lang ) const;

    inline virtual int getPosition( ) const;
    inline virtual const DLString & getNoargOther( ) const;
    inline virtual const DLString & getNoargMe( ) const;
    inline virtual const DLString & getAutoOther( ) const;
    inline virtual const DLString & getAutoMe( ) const;
    inline virtual const DLString & getArgOther( ) const;
    inline virtual const DLString & getArgMe( ) const;
    inline virtual const DLString & getArgVictim( ) const;
    inline virtual const DLString & getArgOther2( ) const;
    inline virtual const DLString & getArgMe2( ) const;
    inline virtual const DLString & getArgVictim2( ) const;
    inline virtual const DLString & getErrorMsg( ) const;
    inline virtual const DLString & getObjVictim() const;
    inline virtual const DLString & getObjChar() const;
    inline virtual const DLString & getObjOthers() const;
    inline virtual const DLString & getObjNoVictimSelf() const;
    inline virtual const DLString & getObjNoVictimOthers() const;

    /* Trilinguality (Trello 2594): each message resolved in the recipient's own
     * language rather than handing the same Russian line to the whole room. */
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
    virtual bool reaction( Character *, Character *, const DLString & );

private:
    bool mprog( Character *, Character * );

    DLString name;

public:
    XML_VARIABLE XMLString  rusName;
    XML_VARIABLE XMLString  uaName;

    /* Every player-visible line is per-language data. Legacy nodes carry no 'l'
     * attribute and XMLMultiString reads them as Russian (Cyrillic content), so
     * an untranslated social renders exactly as it did before. */
    XML_VARIABLE XMLMultiString  shortDesc;
    XML_VARIABLE XMLMultiString  msgCharNoArgument;
    XML_VARIABLE XMLMultiString  msgOthersNoArgument;
    XML_VARIABLE XMLMultiString  msgCharFound;
    XML_VARIABLE XMLMultiString  msgOthersFound;
    XML_VARIABLE XMLMultiString  msgVictimFound;
    XML_VARIABLE XMLMultiString  msgCharNotFound;
    XML_VARIABLE XMLMultiString  msgCharAuto;
    XML_VARIABLE XMLMultiString  msgOthersAuto;

    XML_VARIABLE XMLMultiString  msgCharFound2;
    XML_VARIABLE XMLMultiString  msgOthersFound2;
    XML_VARIABLE XMLMultiString  msgVictimFound2;

    XML_VARIABLE XMLMultiString  msgVictimObj;
    XML_VARIABLE XMLMultiString  msgCharVictimObj;
    XML_VARIABLE XMLMultiString  msgOthersVictimObj;
    XML_VARIABLE XMLMultiString  msgCharObj;
    XML_VARIABLE XMLMultiString  msgOthersObj;

    XML_VARIABLE XMLStringList aliases;

    XML_VARIABLE XMLEnumeration position;

    XML_VARIABLE XMLPointer<SocialHelp> help;
};

inline const DLString& Social::getName( ) const 
{
    return name;
}
inline void Social::setName( const DLString &name ) 
{
    this->name = name;
}
inline const DLString& Social::getRussianName( ) const 
{
    return rusName.getValue( );
}

inline const DLString& Social::getUaName( ) const
{
    return uaName.getValue( );
}
inline const DLString & Social::getShortDesc( ) const
{
    return shortDesc.get(RU);
}
inline const DLString & Social::getShortDescFor( lang_t lang ) const
{
    return shortDesc.getForLang(lang);
}
inline int Social::getPosition( ) const 
{
    return position.getValue( );
}
inline const DLString & Social::getNoargOther( ) const
{
    return msgOthersNoArgument.get(RU);
}
inline const DLString & Social::getNoargMe( ) const
{
    return msgCharNoArgument.get(RU);
}
inline const DLString & Social::getArgMe( ) const
{
    return msgCharFound.get(RU);
}
inline const DLString & Social::getArgOther( ) const
{
    return msgOthersFound.get(RU);
}
inline const DLString & Social::getArgVictim( ) const
{
    return msgVictimFound.get(RU);
}
inline const DLString & Social::getAutoMe( ) const
{
    return msgCharAuto.get(RU);
}
inline const DLString & Social::getAutoOther( ) const
{
    return msgOthersAuto.get(RU);
}
inline const DLString & Social::getArgMe2( ) const
{
    return msgCharFound2.get(RU);
}
inline const DLString & Social::getArgOther2( ) const
{
    return msgOthersFound2.get(RU);
}
inline const DLString & Social::getArgVictim2( ) const
{
    return msgVictimFound2.get(RU);
}
inline const DLString & Social::getErrorMsg( ) const
{
    return msgCharNotFound.get(RU);
}

inline const DLString & Social::getObjVictim() const
{
    return msgVictimObj.get(RU);
}
inline const DLString & Social::getObjChar() const
{
    return msgCharVictimObj.get(RU);
}
inline const DLString & Social::getObjOthers() const
{
    return msgOthersVictimObj.get(RU);
}
inline const DLString & Social::getObjNoVictimSelf() const
{
    return msgCharObj.get(RU);
}
inline const DLString & Social::getObjNoVictimOthers() const
{
    return msgOthersObj.get(RU);
}

#endif

