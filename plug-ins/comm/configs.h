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

class PCharacter;

namespace Json {
    class Value;
}

/** One 'this is what it looks like' sample for an option's extended help: a
 *  short caption ('on' / 'off') and a line of game output with colour codes,
 *  rendered for the web client as it would appear in the terminal. */
class ConfigExample : public XMLVariableContainer {
XML_OBJECT
public:
    const XMLMultiString & getLabel( ) const { return label; }
    const XMLMultiString & getText( ) const { return text; }

protected:
    XML_VARIABLE XMLMultiString label, text;
};

/** One choice of a multiple-choice option: the value the server understands,
 *  the word the player sees for it, and the sentence describing what the world
 *  is like while that choice is in force. */
class ConfigEnumValue : public XMLVariableContainer {
XML_OBJECT
public:
    const DLString & getValue( ) const { return value; }
    const XMLMultiString & getLabel( ) const { return label; }
    const XMLMultiString & getDescription( ) const { return desc; }

protected:
    XML_VARIABLE XMLString value;
    XML_VARIABLE XMLMultiString label, desc;
};

/** Everything an option has no matter how it is stored: its names, its level
 *  requirement and the texts the settings dialog shows -- a one-line hint, an
 *  extended help and a few examples. All of it optional except the names: an
 *  option with no help simply shows none. */
class ConfigOption : public XMLVariableContainer {
XML_OBJECT
public:
    typedef XMLListBase<ConfigExample> Examples;

    const DLString & getName( ) const { return name; }
    const DLString & getRussianName( ) const { return rname; }
    const DLString & getUaName( ) const { return uaname; }

    bool available(PCharacter *) const;

    const XMLMultiString & getHint( ) const { return hint; }
    const XMLMultiString & getHelp( ) const { return help; }
    const Examples & getExamples( ) const { return examples; }
    const DLString & getWebPage( ) const { return webPage; }
    int getWebOrder( ) const { return webOrder.getValue( ); }

protected:
    XML_VARIABLE XMLString name, rname, uaname;
    XML_VARIABLE XMLIntegerNoEmpty level;
    XML_VARIABLE XMLMultiString hint, help;
    XML_VARIABLE Examples examples;
    XML_VARIABLE XMLString webPage;
    XML_VARIABLE XMLIntegerNoEmpty webOrder;
};

/** An option stored as a single bit in one of the player's flag fields. */
class ConfigElement : public ConfigOption {
XML_OBJECT
public:
    typedef ::Pointer<ConfigElement> Pointer;
    typedef ::XMLPointer<ConfigElement> XMLPointer;

    bool handleArgument( PCharacter *, const DLString & ) const;

    bool printText( PCharacter * ) const;
    void printLine( PCharacter * ) const;

    bool isSetBit( PCharacter * ) const;

    /** What the game says the world is like with the flag set, and without it.
     *  The dialog shows one of the two under the option, so the player reads
     *  the state in the same words the terminal uses. */
    const XMLMultiString & getMsgOn( ) const { return msgOn; }
    const XMLMultiString & getMsgOff( ) const { return msgOff; }

protected:
    XML_VARIABLE XMLFlagsWithTable   bit;
    XML_VARIABLE XMLMultiString  msgOn, msgOff;

private:
    Flags & getField( PCharacter * ) const;
};

/** An option that holds a value rather than a flag: the colour scheme, the
 *  output buffer size, the language, the Telegram handle, the Discord link.
 *  Their behaviour lives in C++, keyed by 'name'; everything the player reads
 *  is described here, so it stays translatable without a rebuild.
 *
 *  webType tells the dialog how to draw it: 'enum' (values below), 'int'
 *  (minValue..maxValue, offValue switches it off), 'string' (free text,
 *  checked against pattern) or 'account' (a link to an outside service,
 *  changed by its actions rather than typed into). */
class ConfigValueElement : public ConfigOption {
XML_OBJECT
public:
    typedef XMLListBase<ConfigEnumValue> Values;

    const DLString & getWebType( ) const { return webType; }
    const Values & getValues( ) const { return values; }
    const XMLMultiString & getDescription( ) const { return desc; }
    const XMLMultiString & getEmptyDescription( ) const { return descEmpty; }
    const DLString & getPlaceholder( ) const { return placeholder; }
    const DLString & getPattern( ) const { return pattern; }
    int getMinValue( ) const { return minValue.getValue( ); }
    int getMaxValue( ) const { return maxValue.getValue( ); }
    int getStep( ) const { return step.getValue( ); }
    int getOffValue( ) const { return offValue.getValue( ); }

protected:
    XML_VARIABLE XMLString webType, placeholder, pattern;
    /** The state sentence, with %d or %s where the value goes, and the one for
     *  an option holding nothing at all. */
    XML_VARIABLE XMLMultiString desc, descEmpty;
    XML_VARIABLE Values values;
    XML_VARIABLE XMLIntegerNoEmpty minValue, maxValue, step, offValue;
};

class ConfigGroup : public XMLListContainer<ConfigElement::XMLPointer> 
{
XML_OBJECT
public:
    typedef ::Pointer<ConfigGroup> Pointer;

    void printHeader( PCharacter * ) const;

    XML_VARIABLE XMLMultiString name;
};

/** A branch of the settings dialog's tree: 'server settings', 'client
 *  settings' and so on. Pages hang off it. */
class ConfigWebSection : public XMLVariableContainer {
XML_OBJECT
public:
    const DLString & getKey( ) const { return key; }
    const XMLMultiString & getName( ) const { return name; }

protected:
    XML_VARIABLE XMLString key;
    XML_VARIABLE XMLMultiString name;
};

/** One page of the settings dialog: a heading, a line saying what the page is
 *  about, and the section of the tree it hangs from. An option names its page
 *  in webPage. */
class ConfigWebPage : public XMLVariableContainer {
XML_OBJECT
public:
    const DLString & getKey( ) const { return key; }
    const DLString & getSection( ) const { return section; }
    const XMLMultiString & getName( ) const { return name; }
    const XMLMultiString & getSubtitle( ) const { return subtitle; }

protected:
    XML_VARIABLE XMLString key, section;
    XML_VARIABLE XMLMultiString name, subtitle;
};

class ConfigCommand : public CommandPlugin {
XML_OBJECT
public:
    typedef ::Pointer<ConfigCommand> Pointer;
    typedef XMLListBase<ConfigGroup> Groups;
    typedef XMLListBase<ConfigValueElement> ValueElements;
    typedef XMLListBase<ConfigWebSection> WebSections;
    typedef XMLListBase<ConfigWebPage> WebPages;

    ConfigCommand( );

    virtual void run( Character *, const DLString & );

    const Groups & getGroups( ) const { return groups; }
    const ValueElements & getValueElements( ) const { return valueElements; }
    const WebSections & getWebSections( ) const { return webSections; }
    const WebPages & getWebPages( ) const { return webPages; }

    inline static ConfigCommand * getThis( )
    {
        return thisClass;
    }
   
protected:
    virtual void initialization( );
    virtual void destruction( );

private:
    XML_VARIABLE Groups groups;
    XML_VARIABLE ValueElements valueElements;
    XML_VARIABLE WebSections webSections;
    XML_VARIABLE WebPages webPages;

    static const DLString COMMAND_NAME;
    static ConfigCommand *thisClass;
};

/** The options that are not flags, shared by the 'config' command and by the
 *  web client: one place decides what a value means and what changing it
 *  prints, so both ways of changing it behave the same.
 *
 * 'key' is matched with the same synonym rules the command uses, so both
 * 'lines' and its translations get here. Returns false if the key names no
 * value option, leaving the caller to look among the flag options. */
bool config_value_run(PCharacter *ch, const DLString &key, const DLString &argument);

/** Current values of all the options above, as one JSON object. */
void config_value_json(PCharacter *ch, Json::Value &values);

#endif
