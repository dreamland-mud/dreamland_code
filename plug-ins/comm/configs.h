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
#include "xmlmultistring.h"
#include "descriptorstatelistener.h"
#include "jsoncpp/json/json.h"

class PCharacter;
class Descriptor;

class ConfigElement : public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<ConfigElement> Pointer;
    typedef ::XMLPointer<ConfigElement> XMLPointer;
    
    const DLString & getName() const;
    const DLString & getRussianName( ) const;
    const DLString & getUaName( ) const;
    bool handleArgument( PCharacter *, const DLString & ) const;
    bool available(PCharacter *) const;

    bool printText( PCharacter * ) const;
    void printLine( PCharacter * ) const;

    // Public so the account-config write-through can read an option's new value
    // after handleArgument toggles it.
    bool isSetBit( PCharacter * ) const;

protected:
    XML_VARIABLE XMLFlagsWithTable   bit;
    XML_VARIABLE XMLString  name, rname, uaname;
    XML_VARIABLE XMLMultiString  msgOn, msgOff;
    XML_VARIABLE XMLIntegerNoEmpty level;

private:
    Flags & getField( PCharacter * ) const;
};


class ConfigGroup : public XMLListContainer<ConfigElement::XMLPointer> 
{
XML_OBJECT
public:
    typedef ::Pointer<ConfigGroup> Pointer;

    void printHeader( PCharacter * ) const;

    XML_VARIABLE XMLMultiString name;
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

    /** Every option this player may see, by its canonical EN name, with the
     *  value it has right now: a switch as 1 or 0, a value option as its value.
     *  The web dialog opens on this and never guesses. */
    void webValues( PCharacter *, Json::Value &values );

    /** Change one option by its canonical EN name, with no command line to
     *  parse: no abbreviation pass, no localised on/off word, nothing that a
     *  renamed command could break. The player sees the same line the typed
     *  command prints. Returns false when there is no such option or this
     *  player may not have it; otherwise the value as it now stands -- read
     *  back from where the option keeps it, not the value that was asked for --
     *  is written to 'stored'. */
    bool webApply( PCharacter *, const DLString &key, const DLString &value, Json::Value &stored );

   
protected:
    virtual void initialization( );
    virtual void destruction( );

private:
    XML_VARIABLE Groups groups;

    static const DLString COMMAND_NAME;
    static ConfigCommand *thisClass;
};


/**
 * On every login/reconnect (transition to CON_PLAYING), push an account's
 * account-wide config (screenreader, colour, language, spam toggles) onto the
 * entering character, so alts inherit them without re-typing. Unlinked chars are
 * untouched. Per-char config stays on the pfile. See ACCOUNTS_NANNY_ROADMAP.md.
 */
class AccountConfigLoginListener : public DescriptorStateListener {
public:
    typedef ::Pointer<AccountConfigLoginListener> Pointer;

    virtual void run( int, int, Descriptor * );
};

#endif
