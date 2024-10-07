#ifndef __HEDIT_H__
#define __HEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "helpmanager.h"
#include "markuphelparticle.h"
#include "olc.h"

class OLCStateHelp : public OLCStateTemplate<OLCStateHelp>,
                     public virtual OLCState
{
XML_OBJECT
public:
    typedef ::Pointer<OLCStateHelp> Pointer;

    OLCStateHelp();
    OLCStateHelp(HelpArticle *h);
    virtual ~OLCStateHelp();
    
    virtual void commit();
    virtual void changed( PCharacter * );
    void show( PCharacter * ) const;

    XML_VARIABLE XMLInteger id, level;
    XML_VARIABLE XMLString labels, labelsAuto, autotitle;
    XML_VARIABLE XMLMultiString text, keyword, title, extra;

    template <typename T>
    bool cmd(PCharacter *ch, char *argument);
    
private:
    virtual void statePrompt( Descriptor * );
    StringSet allKeywords() const;

    XML_VARIABLE XMLBoolean isChanged;
};

#define HEDIT(C, rname, help) OLC_CMD(OLCStateHelp, C, rname, help)


template <typename Func, typename HelpPointer>
bool help_subcommand(PCharacter *ch, const DLString &argument, HelpPointer &help, Func &postCreateAction)
{
    DLString arg = argument;

    if (arg.empty()) {
        if (!help || help->getID() < 1) {
            ptc(ch, "Справка не задана, используй {y{hchelp create{x для создания новой.\r\n");
            return false;
        }

        OLCStateHelp::Pointer hedit(NEW, help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }

    if (arg_oneof(arg, "create", "создать")) {
        if (help && help->getID() > 0) {
            ptc(ch, "Справка уже существует, используй команду {y{hchelp{x для редактирования.\r\n");
            return false;
        }

        if (!help)
            help.construct();

        help->setID(
            help_next_free_id()
        );

        postCreateAction(help);

        OLCStateHelp::Pointer hedit(NEW, help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }   

    ptc(ch, "Использование: {y{hchelp{x, {y{hchelp create{x\r\n");
    return false;
}

#endif
