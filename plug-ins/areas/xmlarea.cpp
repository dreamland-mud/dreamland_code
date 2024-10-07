/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "dlfileop.h"
#include "xmlarea.h"
#include "areahelp.h"
#include "feniamanager.h"
#include "fileformatexception.h"
#include "room.h"
#include "merc.h"

#include "dreamland.h"
#include "def.h"

#include <fstream>

using namespace std;

bool dup_room_vnum( int vnum );
bool dup_obj_vnum( int vnum );
bool dup_mob_vnum( int vnum );

XMLAreaHeader::XMLAreaHeader() : flags(0, &area_flags), loaded(false)
{
}

bool
XMLAreaHeader::toXML(XMLNode::Pointer &parent) const
{
    if(!loaded)
        return false;

    return XMLVariableContainer::toXML(parent);
}

void 
XMLAreaHeader::fromXML(const XMLNode::Pointer &parent) 
{
    XMLVariableContainer::fromXML(parent);
    loaded = true;
}

void
XMLAreaHeader::init(AreaIndexData *a)
{
    if(a->name) name.setValue(a->name);
    if(a->credits) credits.setValue(a->credits);
    if(a->authors) authors.setValue(a->authors);
    if(a->altname) altname.setValue(a->altname);
    if(a->translator) translator.setValue(a->translator);
    if(a->speedwalk) speedwalk.setValue(a->speedwalk);

    security.setValue(a->security);
    
    vnumLow.setValue(a->min_vnum);
    vnumHigh.setValue(a->max_vnum);
    
    levelLow.setValue(a->low_range);
    levelHigh.setValue(a->high_range);
    
    flags.setValue(a->area_flag);
    
    if(a->resetmsg) {
        resetMessage.setValue(a->resetmsg);
        resetMessage.set(true);
    } else {
        resetMessage.set(false);
    }
    
    if(a->behavior) {
        XMLNode::Pointer node(NEW);
        a->behavior.toXML(node);
        node->setName("behavior");
        behavior.setNode(node);
    }
    loaded = true;
}

AreaIndexData *
XMLAreaHeader::compat(area_file *areaFile)
{
    if(!loaded)
        return 0;

    AreaIndexData *a = new AreaIndexData;
    
    a->vnum                = top_area++;

    areaFile->area = a;
    a->area_file = areaFile;

    a->name = str_dup(name.getValue( ).c_str( ));
    a->credits = str_dup(credits.getValue( ).c_str( ));
    a->authors = str_dup(authors.getValue( ).c_str( ));
    a->altname = str_dup(altname.getValue( ).c_str( ));
    a->translator = str_dup(translator.getValue( ).c_str( ));
    a->speedwalk= str_dup(speedwalk.getValue( ).c_str( ));

    a->security = security.getValue( );
    
    a->min_vnum = vnumLow.getValue( );
    a->max_vnum = vnumHigh.getValue( );
    
    a->low_range = levelLow.getValue( );
    a->high_range = levelHigh.getValue( );
    
    a->area_flag = flags.getValue( );
    
    if(resetMessage.set( ))
        a->resetmsg = str_dup(resetMessage.getValue( ).c_str( ));
    else
        a->resetmsg = 0;
        
    if(behavior.getNode( )) {
        a->behavior.fromXML(behavior.getNode( ));
        if(a->behavior)
            a->behavior->setArea(a);
    } else
        a->behavior.clear( );

    if (FeniaManager::wrapperManager)
        FeniaManager::wrapperManager->linkWrapper(a);

    return a;
}

XMLArea::XMLArea( ) : mobiles(false), objects(false), rooms(false)
{
}

void
XMLArea::init(area_file *af)
{
    AreaIndexData *area = af->area;

    if(!area)
        return;
    
    HelpArticles::iterator h;
    for (h = area->helps.begin( ); h != area->helps.end( ); h++) {
        AreaHelp *ahelp = h->getDynamicPointer<AreaHelp>();
        if (ahelp && ahelp->persistent) {
            XMLAreaHelp help;

            help.level = ahelp->getLevel();
            help.id = ahelp->getID();
            help.labels = ahelp->labels.persistent.toString();

            help.title = ahelp->title;
            help.extra = ahelp->extra;
            help.keyword = ahelp->keyword;
            help.text = ahelp->text;

            helps.push_back(help); 
        }
    }

    areadata.init(area);

    for (auto &q: area->quests)
        quests.push_back(*q);

    MOB_INDEX_DATA *pMobIndex;
    for (int v = area->min_vnum; v <= area->max_vnum; v++) {
        pMobIndex = get_mob_index(v);
        if(pMobIndex)
            mobiles[pMobIndex->vnum].init(pMobIndex);
    }
    
    OBJ_INDEX_DATA *pObjIndex;
    for (int v = area->min_vnum; v <= area->max_vnum; v++) {
        pObjIndex = get_obj_index(v);
        if(pObjIndex) 
            objects[pObjIndex->vnum].init(pObjIndex);
    }
    
    RoomIndexData *pRoom;
    for (int v = area->min_vnum; v <= area->max_vnum; v++) {
        pRoom = get_room_index(v);
        if(pRoom)
            rooms[pRoom->vnum].init(pRoom);
    }
}

void
XMLArea::load_helps(AreaIndexData *a)
{
    XMLListBase<XMLAreaHelp>::const_iterator h;
    bool selfHelpExists = false;

    for (h = helps.begin( ); h != helps.end( ); h++) {
        AreaHelp::Pointer help(NEW);
        DLString aname(a->getName());
        aname.colourstrip();

        help->areafile = a->area_file;
        help->selfHelp = is_name(aname.c_str(), h->keyword.get(RU).c_str()) || is_name(aname.c_str(), h->keyword.get(EN).c_str());
        help->persistent = true;
        help->keyword = h->keyword;
        help->title = h->title;
        help->extra = h->extra;
        help->text = h->text;
        help->refreshKeywords();

        help->setLevel(h->level);
        help->setID(h->id);
        help->labels.addPersistent(h->labels);
        helpManager->registrate(help);
        if (help->selfHelp) {
            selfHelpExists = true;
            help->labels.addTransient("area");
        }
        XMLPersistent<HelpArticle> phelp(help.getPointer());
        a->helps.push_back(phelp);
    }

    if (!selfHelpExists) {
        AreaHelp::Pointer help(NEW);
        help->areafile = a->area_file;
        help->selfHelp = true;
        help->persistent = false;
        help->addAutoKeyword(a->getName().colourStrip().quote());
        help->addAutoKeyword(DLString(a->credits).colourStrip().quote());
        help->text[RU] = "    ";
        help->labels.addTransient("area");
        helpManager->registrate(help);

        XMLPersistent<HelpArticle> phelp(help.getPointer());
        a->helps.push_back(phelp);
    }
}

void
XMLArea::load_rooms(AreaIndexData *a)
{
    XMLMapBase<XMLRoom>::iterator rit;
    for(rit = rooms.begin( ); rit != rooms.end( ); rit++) {
        int vnum = rit->first.toInt( );
        
        if (dup_room_vnum( vnum ))
            throw FileFormatException("Load_rooms: vnum %d duplicated", vnum);

        RoomIndexData *room = rit->second.compat(vnum);
        room->areaIndex = a;
        
        if(3000 <= vnum && vnum < 3400)
            SET_BIT(room->room_flags, ROOM_LAW);

        roomIndexMap[vnum] = room;
        room->areaIndex->roomIndexes[vnum] = room;

        // Create new single room instance for this index (FIXME)
        room->create();        
    }
}

void
XMLArea::load_mobiles(AreaIndexData *a)
{
    XMLMapBase<XMLMobileFactory>::iterator mit;
    int i = 0;
    for(mit = mobiles.begin( ); mit != mobiles.end( ); mit++, i++) {
        int iHash, vnum = mit->first.toInt( );
        
        if (dup_mob_vnum( vnum ))
            throw FileFormatException("Load_mobiles: vnum %d duplicated", vnum);
        
        MOB_INDEX_DATA *pMobIndex = mit->second.compat( );
        pMobIndex->vnum = vnum;
        pMobIndex->area = a;

        iHash = vnum % MAX_KEY_HASH;
        pMobIndex->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = pMobIndex;

        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper( pMobIndex );
    }
}

void
XMLArea::load_objects(AreaIndexData *a)
{
    XMLMapBase<XMLObjectFactory>::iterator oit;
    for(oit = objects.begin( ); oit != objects.end( ); oit++) {
        int iHash, vnum = oit->first.toInt( );
        
        if (dup_obj_vnum( vnum ))
            throw FileFormatException("Load_objects: vnum %d duplicated", vnum);
        
        OBJ_INDEX_DATA *pObjIndex = oit->second.compat( );
        pObjIndex->vnum = vnum;
        pObjIndex->area        = a;

        iHash = vnum % MAX_KEY_HASH;
        pObjIndex->next = obj_index_hash[iHash];
        obj_index_hash[iHash] = pObjIndex;
        
        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper( pObjIndex );
    }
}

void XMLArea::load_quests(AreaIndexData *a)
{
    for (auto &q: quests) {
        a->quests.push_back(*q);
        a->questMap[q->vnum.getValue()] = *q;
        q->pAreaIndex = a;

        areaQuests[q->vnum] = *q;

        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper(*q);
    }
}

void
XMLArea::load(const DLString &fname)
{
    LogStream::sendNotice( ) << "loading `" << fname << "'..." << endl;
    XMLDocument::Pointer doc(NEW);
    
    DLFileRead areaFile( dreamland->getAreaDir( ), fname, ".xml" );
    ifstream is(areaFile.getPath( ).c_str( ));

    if(!is)
        throw Exception("failed to open " + fname);

    doc->load(is);

    fromXML(doc->getFirstNode( ));
    
    area_file *af = new_area_file(fname.c_str( ));
    AreaIndexData *a = areadata.compat(af);
    areaIndexes.push_back(a);

    try {
        if (a) {
            // Create new single area instance for this index (FIXME)
            a->create();

            load_rooms(a);
            load_mobiles(a);
            load_objects(a);
            load_helps(a);
            load_quests(a);
        }
    } catch (const Exception &ex) {
        LogStream::sendFatal() << ex.what() << endl;
        throw ex;
    }
}

void
XMLArea::save(area_file *af)
{
    LogStream::sendNotice( ) << "saving `" << af->file_name << "'..." << endl;

    XMLNode::Pointer node(NEW);

    init(af);
    toXML(node);
    node->setName("area");
    
    XMLDocument::Pointer doc(NEW);
    doc->appendChild(node);
    
    DLFileRead areaFile( dreamland->getAreaDir( ), af->file_name, ".xml" );
    ofstream os( areaFile.getPath( ).c_str( ) );
    doc->save(os);

    if (!os) {
        DLString msg = "Error saving area " + areaFile.getPath();
        LogStream::sendSystem() << msg << endl;
        throw ExceptionDBIO(msg);
    }
}
