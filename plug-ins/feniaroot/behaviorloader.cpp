#include "behaviorloader.h"
#include "dreamland.h"
#include "feniamanager.h"
#include "logstream.h"
#include "configurable.h"
#include "l10n.h"

CONFIGURABLE_LOADED(behaviors, services)
{
}


const DLString BehaviorHelp::TYPE = "BehaviorHelp";

void BehaviorHelp::save() const
{
    if (bhv)
        bhv->save();
}

DLString BehaviorHelp::getTitle(const DLString &label) const
{
    ostringstream buf;

    // Website: right-hand side table of contents
    if (label == "toc") {
        buf << "Поведение '" << bhv->getRussianName().ruscase('1') << "'";
        return buf.str();
    }

    // Website: article title
    if (label == "title") {
        return DLString::emptyString;
    }

    if (!title.get(RU).empty() || !bhv)
        return MarkupHelpArticle::getTitle(label);

    // In-game header: just the behavior's name, capitalized. No "Поведение"
    // prefix -- players don't know what "behaviors" are. Per-viewer name is
    // handled by the lang-aware override below; this path is the RU default.
    buf << "{c" << bhv->getRussianName().ruscase('1').upperFirstCharacter() << "{x";
    return buf.str();
}

static DLString behavior_name_for(DefaultBehavior::Pointer bhv, lang_t lang)
{
    if (lang == LANG_EN)
        return bhv->getName().ruscase('1');       // latin keyword, e.g. "cuisinart"
    if (lang == LANG_UA)
        return bhv->getUkrainianName().ruscase('1');  // falls back to RU
    return bhv->getRussianName().ruscase('1');
}

DLString BehaviorHelp::getTitle(const DLString &label, lang_t lang) const
{
    // An authored <title> already resolves per language in the base.
    if (label == "title" || !title.get(RU).empty() || !bhv)
        return HelpArticle::getTitle(label, lang);

    // Website: right-hand side table of contents. Keeps the "Поведение '...'"
    // grouping -- it is what sorts these together in the rail -- but the frame
    // word and the name now follow the viewer.
    if (label == "toc")
        return help_title_fmt(lang, _("Поведение '%1$s'"), behavior_name_for(bhv, lang));

    // In-game player help header: the behavior's name in the viewer's language,
    // capitalized, with NO "Поведение" prefix -- players don't know what
    // "behaviors" are.
    ostringstream buf;
    buf << "{c" << behavior_name_for(bhv, lang).upperFirstCharacter() << "{x";
    return buf.str();
}

const DLString & BehaviorHelp::getType( ) const
{
    return TYPE;
}

void BehaviorHelp::setBehavior( DefaultBehavior::Pointer bhv )
{
    StringSet::const_iterator r;

    this->bhv = bhv;
    
    DLString name = bhv->getName();
    addAutoKeyword(name.replaces("_", " "));
    addAutoKeyword(bhv->getRussianName().ruscase('1'));    

    labels.addTransient("item");
   
    helpManager->registrate( Pointer( this ) );
}


void BehaviorHelp::unsetBehavior( )
{
    helpManager->unregistrate( Pointer( this ) );
    
    bhv.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}


DefaultBehavior::DefaultBehavior()
                    : Behavior()
{

}

void DefaultBehavior::loaded()
{
    // Once loaded from disk, register this behavior with the manager and resolve Fenia wrappers.
    behaviorManager->registrate(Pointer(this));

    if (help)
        help->setBehavior(Pointer(this));

    if (FeniaManager::wrapperManager) {
        FeniaManager::wrapperManager->linkWrapper(this);
        if (wrapper)
            LogStream::sendNotice() << "Behavior: linked wrapper for " << getName() << endl;
    }    
}

void DefaultBehavior::unloaded()
{
    // Notify Fenia that the wrapper is extracted before unloading the behavior. De-register from the manager.
    if (FeniaManager::wrapperManager)
        if (wrapper)
            extractWrapper(false);    
    
    if (help)
        help->unsetBehavior();

    behaviorManager->unregistrate(Pointer(this));
}

const DLString & DefaultBehavior::getName() const
{
    return Behavior::getName();
}
     
void DefaultBehavior::setName(const DLString &name)
{
    this->name = name;
}



TABLE_LOADER_IMPL(BehaviorLoader, "behaviors", "behavior");
