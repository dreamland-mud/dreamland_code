/* $Id: customsocial.h,v 1.1.2.1.6.4 2009/01/01 14:05:01 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef CUSTOMSOCIAL_H
#define CUSTOMSOCIAL_H

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlmap.h"
#include "xmlpointer.h"

#include "xmltableelement.h"
#include "xmlattribute.h"
#include "playerattributes.h"
#include "interpretlayer.h"
#include "socialbase.h"


class CustomSocialManager : public InterpretLayer {
public:
        typedef ::Pointer<CustomSocialManager> Pointer;

        CustomSocialManager( );
        virtual ~CustomSocialManager( );

        virtual bool process( InterpretArguments & );

protected:
        virtual void putInto( );
};

class CustomSocial : public SocialBase, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<CustomSocial> Pointer;

    CustomSocial( );
    virtual ~CustomSocial( );

    virtual const DLString &getName( ) const;
    virtual const DLString &getRussianName( ) const;
    virtual const DLString & getNoargOther( ) const;
    virtual const DLString & getNoargMe( ) const;
    virtual const DLString & getAutoOther( ) const;
    virtual const DLString & getAutoMe( ) const;
    virtual const DLString & getArgOther( ) const;
    virtual const DLString & getArgMe( ) const;
    virtual const DLString & getArgVictim( ) const;
    virtual const DLString & getErrorMsg( ) const;
    
    inline void setName( const DLString & );
    inline void setRussianName( const DLString & );
    inline void setNoargOther( const DLString & );
    inline void setNoargMe( const DLString & );
    inline void setAutoOther( const DLString & );
    inline void setAutoMe( const DLString & );
    inline void setArgOther( const DLString & );
    inline void setArgMe( const DLString & );
    inline void setArgVictim( const DLString & );
   
protected:
    virtual void reaction( Character *, Character *, const DLString & );
    virtual int getPosition( ) const;

private:
    XML_VARIABLE XMLStringNoEmpty name, rusName;
    XML_VARIABLE XMLStringNoEmpty noargOther, noargMe;
    XML_VARIABLE XMLStringNoEmpty autoMe, autoOther;
    XML_VARIABLE XMLStringNoEmpty argVictim, argMe, argOther;
};

inline void CustomSocial::setName( const DLString &arg ) 
{
    name.setValue( arg );
}
inline void CustomSocial::setRussianName( const DLString &arg ) 
{
    rusName.setValue( arg );
}
inline void CustomSocial::setNoargOther( const DLString &arg )
{
    noargOther.setValue( arg );
}
inline void CustomSocial::setNoargMe( const DLString &arg )
{
    noargMe.setValue( arg );
}
inline void CustomSocial::setArgMe( const DLString &arg )
{
    argMe.setValue( arg );
}
inline void CustomSocial::setArgOther( const DLString &arg )
{
    argOther.setValue( arg );
}
inline void CustomSocial::setArgVictim( const DLString &arg )
{
    argVictim.setValue( arg );
}
inline void CustomSocial::setAutoMe( const DLString &arg )
{
    autoMe.setValue( arg );
}
inline void CustomSocial::setAutoOther( const DLString &arg )
{
    autoOther.setValue( arg );
}

typedef XMLPointer<CustomSocial> XMLCustomSocial;

class XMLAttributeCustomSocials : public XMLMapBase<XMLCustomSocial>,
                                  public RemortAttribute
{
public:
        typedef ::Pointer<XMLAttributeCustomSocials> Pointer;

        XMLAttributeCustomSocials( );

        virtual const DLString & getType( ) const
        {
            return TYPE;
        }

        static const DLString TYPE;

        CustomSocial::Pointer chooseSocial( const DLString & );
        CustomSocial::Pointer getSocial( const DLString & );
};


#endif

