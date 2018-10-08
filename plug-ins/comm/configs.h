/* $Id: configs.h,v 1.1.2.3.6.6 2008/05/21 08:15:30 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#include "commandplugin.h"
#include "defaultcommand.h"
#include "xmllist.h"
#include "xmlinteger.h"
#include "xmlpointer.h"

class PCharacter;

class ConfigElement : public DefaultCommand, public XMLCommand {
XML_OBJECT
friend class ConfigCommand;
public:
    typedef ::Pointer<ConfigElement> Pointer;
    typedef ::XMLPointer<ConfigElement> XMLPointer;
    
    virtual const DLString & getRussianName( ) const;
    virtual void run( Character *, const DLString & );

protected:    
    void init( );
    void destroy( );

    bool handleArgument( PCharacter *, const DLString & ) const;

    bool printText( PCharacter * ) const;
    void printRow( PCharacter * ) const;
    void printLine( PCharacter * ) const;

    XML_VARIABLE XMLBoolean autocmd, autolist, autotext;
    XML_VARIABLE XMLFlagsWithTable   bit;
    XML_VARIABLE XMLString  rname;
    XML_VARIABLE XMLString  msgOn, msgOff;

private:
    Flags & getField( PCharacter * ) const;
    bool isSetBit( PCharacter * ) const;
};


class ConfigGroup : public XMLListContainer<ConfigElement::XMLPointer> 
{
XML_OBJECT
public:
    typedef ::Pointer<ConfigGroup> Pointer;

    void printHeader( PCharacter * ) const;

    XML_VARIABLE XMLString name;
};

class ConfigCommand : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
    typedef ::Pointer<ConfigCommand> Pointer;
    typedef XMLListBase<ConfigGroup> Groups;

    ConfigCommand( );

    virtual void run( Character *, const DLString & );

    void printAllRows( PCharacter * ) const;
    void printAllTexts( PCharacter * ) const;

    inline static ConfigCommand * getThis( )
    {
        return thisClass;
    }
   
protected:
    virtual void initialization( );
    virtual void destruction( );

private:
    XML_VARIABLE Groups groups;

    static const DLString COMMAND_NAME;
    static ConfigCommand *thisClass;
};


#endif
