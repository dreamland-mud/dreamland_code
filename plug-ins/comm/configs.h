/* $Id: configs.h,v 1.1.2.3.6.6 2008/05/21 08:15:30 rufina Exp $
 *
 * ruffina, 2005
 */
#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#include "commandplugin.h"
#include "xmllist.h"
#include "xmlinteger.h"
#include "xmlpointer.h"

class PCharacter;

class ConfigElement : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ConfigElement> Pointer;
    typedef ::XMLPointer<ConfigElement> XMLPointer;
    
    const DLString & getName() const;
    const DLString & getRussianName( ) const;
    bool handleArgument( PCharacter *, const DLString & ) const;
    bool available(PCharacter *) const;

    bool printText( PCharacter * ) const;
    void printRow( PCharacter * ) const;
    void printLine( PCharacter * ) const;

protected:    
    XML_VARIABLE XMLFlagsWithTable   bit;
    XML_VARIABLE XMLString  name, rname;
    XML_VARIABLE XMLString  msgOn, msgOff;
    XML_VARIABLE XMLString  hint;
    XML_VARIABLE XMLIntegerNoEmpty level;

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

class ConfigCommand : public CommandPlugin {
XML_OBJECT
public:
    typedef ::Pointer<ConfigCommand> Pointer;
    typedef XMLListBase<ConfigGroup> Groups;

    ConfigCommand( );

    virtual void run( Character *, const DLString & );

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
