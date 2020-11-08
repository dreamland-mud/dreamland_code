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
#include "mercdb.h"
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
XMLAreaHeader::compat( )
{
    if(!loaded)
        return 0;

    AreaIndexData *a = new AreaIndexData;
    
    a->age                = 15;
    a->nplayer        = 0;
    a->empty        = false;
    a->count        = 0;
    a->next        = 0;
    a->vnum                = top_area++;


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
            help.setValue((DLString)(*ahelp));
            help.keywordAttribute = ahelp->getKeywordAttribute();
	    help.titleAttribute = ahelp->getTitleAttribute();
            help.level = ahelp->getLevel();
            help.id = ahelp->getID();
            help.labels = ahelp->labels.persistent.toString();
            helps.push_back(help); 
        }
    }

    areadata.init(area);

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
    
    Room *pRoom;
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
        DLString aname(a->name);
        aname.colourstrip();

        help->areafile = a->area_file;
        help->selfHelp = is_name(aname.c_str(), h->keywordAttribute.c_str());
        help->persistent = true;
        help->setKeywordAttribute(h->keywordAttribute);
	help->setTitleAttribute(h->titleAttribute);
        help->setLevel(h->level);
        help->setID(h->id);
        help->setText(h->getValue());
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
        help->addAutoKeyword(DLString(a->name).colourStrip().quote());
        help->addAutoKeyword(DLString(a->credits).colourStrip().quote());
        help->setText("     ");
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
        int iHash, vnum = rit->first.toInt( );
        
        if (dup_room_vnum( vnum ))
            throw FileFormatException("Load_rooms: vnum %d duplicated", vnum);

        Room *room = rit->second.compat(vnum);
        room->area = a;

        if(3000 <= vnum && vnum < 3400)
            SET_BIT(room->room_flags, ROOM_LAW);

        iHash = vnum % MAX_KEY_HASH;
        room->next = room_index_hash[iHash];
        room_index_hash[iHash] = room;
        top_room++;
        top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;    /* OLC */

        room->rnext = room_list;
        room_list = room;

        room->area->rooms[vnum] = room;

        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper(room);
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
        pMobIndex->area        = a;

        iHash = vnum % MAX_KEY_HASH;
        pMobIndex->next = mob_index_hash[iHash];
        mob_index_hash[iHash] = pMobIndex;

        if (FeniaManager::wrapperManager)
            FeniaManager::wrapperManager->linkWrapper( pMobIndex );

        newmobs++;
        top_mob_index++;
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
        kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
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

        newobjs++;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;       /* OLC */
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
    AreaIndexData *a = areadata.compat( );

    if(a) {
        af->area = a;
        a->area_file = af;

        load_rooms(a);
        load_mobiles(a);
        load_objects(a);
        load_helps(a);
        
        if ( area_first == 0 )
            area_first = a;
        
        if ( area_last  != 0 )
            area_last->next = a;
        
        area_last = a;
        a->next = 0;
    }
}

void
XMLArea::save(area_file *af)
{
    LogStream::sendNotice( ) << "saving `" << af->file_name << "'..." << endl;

    XMLArea area;
    XMLNode::Pointer node(NEW);

    init(af);
    toXML(node);
    node->setName("area");
    
    XMLDocument::Pointer doc(NEW);
    doc->appendChild(node);
    
    DLFileRead areaFile( dreamland->getAreaDir( ), af->file_name, ".xml" );
    ofstream os( areaFile.getPath( ).c_str( ) );
    doc->save(os);
}
