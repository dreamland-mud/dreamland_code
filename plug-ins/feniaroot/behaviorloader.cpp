#include "behaviorloader.h"
#include "dreamland.h"
#include "feniamanager.h"
#include "logstream.h"
#include "configurable.h"

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

    if (!titleAttribute.empty() || !bhv)
        return MarkupHelpArticle::getTitle(label);

    buf << "Поведение {c" << bhv->getRussianName().ruscase('1') << "{x";
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
