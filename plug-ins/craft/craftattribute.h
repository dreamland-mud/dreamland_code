#ifndef CRAFTATTRIBUTE_H
#define CRAFTATTRIBUTE_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlmap.h"
#include "xmlattribute.h"
#include "xmlattributeplugin.h"
#include "playerattributes.h"

class CraftProfession;

/*
 Example attribute value:

<node name="craft">
        <proficiency>
                <node name="tattooist">
                        <level>3</level>
                        <exp>3300</exp>
                </node>
                <node name="hairdresser">
                        <level>1</level>
                        <exp>200</exp>
                </node>
        </proficiency>
        ...
</node>
*/

class CraftProficiency: public XMLVariableContainer {
XML_OBJECT
public:
        typedef ::Pointer<CraftProficiency> Pointer;
        XML_VARIABLE XMLInteger level;
        XML_VARIABLE XMLInteger exp;
};

class XMLAttributeCraft: public XMLVariableContainer, 
                         public virtual EventHandler<ScoreArguments>,
                         public virtual EventHandler<WhoisArguments>
{
XML_OBJECT
public:
        typedef ::Pointer<XMLAttributeCraft> Pointer;
        typedef XMLMapBase<CraftProficiency> Proficiency;

        XMLAttributeCraft( );
        virtual ~XMLAttributeCraft( );
        
        virtual bool handle( const ScoreArguments &args ); 
        virtual bool handle( const WhoisArguments &args ); 

        int proficiencyLevel(const DLString &profName) const;
        int exp(const DLString &profName) const;
        int gainExp(const DLString &profName, int xp);
        bool learned(const DLString &profName) const;
        inline const Proficiency &getProficiency() const;
        void setProficiencyLevel(const DLString &profName, int level);
protected:
        XML_VARIABLE Proficiency proficiency;
};

inline const XMLAttributeCraft::Proficiency &XMLAttributeCraft::getProficiency() const
{
        return proficiency;
}

#endif
