#ifndef __HEDIT_H__
#define __HEDIT_H__

#include "olcstate.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlstringlist.h"
#include "helpmanager.h"
#include "markuphelparticle.h"
#include "helpcontainer.h"
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
    XML_VARIABLE XMLString seeAlso, refBy;
    /** Set by the seealso/alias subcommands, so commit() never writes fields
     *  an editor state saved by an older binary did not carry. */
    XML_VARIABLE XMLBoolean seeAlsoEdited, refByEdited;
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

    if (arg_is(arg, "create")) {
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

    // An article written as a plain help file before the entity had its own:
    // move it over under the same id, so links to it keep working.
    if (arg.isNumber()) {
        if (help && help->getID() > 0) {
            ptc(ch, "Справка уже существует, используй команду {y{hchelp{x для редактирования.\r\n");
            return false;
        }

        int id = arg.toInt();
        HelpArticle::Pointer old = helpManager->getArticle(id);
        GenericHelp *generic = old ? dynamic_cast<GenericHelp *>(old.getPointer()) : 0;

        if (!generic) {
            ptc(ch, "Статьи %d нет среди общих справок из папки helps, присоединить можно только такую.\r\n", id);
            return false;
        }

        XMLNode::Pointer node(NEW);
        generic->toXML(node);
        generic->detach();

        if (!help)
            help.construct();

        help->fromXML(node);
        postCreateAction(help);
        help->save();

        ptc(ch, "Справка %d теперь принадлежит этой сущности.\r\n", id);
        OLCStateHelp::Pointer hedit(NEW, help.getPointer());
        hedit->attach(ch);
        hedit->show(ch);
        return true;
    }

    ptc(ch, "Использование: {y{hchelp{x, {y{hchelp create{x, {y{hchelp{x {Dномер_статьи{x\r\n");
    return false;
}

#endif
