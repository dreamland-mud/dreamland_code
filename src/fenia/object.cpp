/* $Id: object.cpp,v 1.1.2.9.6.6 2009/11/02 15:09:01 rufina Exp $
 *
 * ruffina, 2004
 */
/* $Id: object.cpp,v 1.1.2.9.6.6 2009/11/02 15:09:01 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#include <sstream>
#include <fstream>
#include <sys/time.h>

#include <logstream.h>
#include <integer.h>
#include <xmldocument.h>

#include "register-impl.h"
#include "object.h"
#include "context.h"
#include "profiler.h"

namespace Scripting {

Object::Manager *Object::manager = 0;

Object::Object() : refcnt(0), id(0), dynamicHandler(false)
{
    next = prev = this;
}

Object::Object(id_t i) : refcnt(0), id(i), dynamicHandler(true) 
{
    next = prev = 0;
    justCreated = false;
}

void
Object::toList(Object &list)
{
    next = &list;
    prev = list.prev;
    next->prev = this;
    prev->next = this;
}

void
Object::fromList()
{
    if(!next)
        return;

    next->prev = prev;
    prev->next = next;
    next = prev = 0;
}

void
Object::changed()
{
    if(!next)
        toList(manager->changed);
}

void 
Object::fromXML( const XMLNode::Pointer& node )  
{
    backupNode = node;
}

bool
Object::toXML( XMLNode::Pointer& node ) const 
{
    node = backupNode;
    return true;
}

void 
Object::backup()
{
    if(handler) {
        backupNode = XMLNode::Pointer(NEW);
    
        handler.toXML( backupNode );
        
        if(dynamicHandler) {
            handler->backup();
            handler.clear();
        }
    } else
        backupNode.clear();
}

void
Object::recover()
{
    if(handler)
        return;

    if(!backupNode)
        return;

    handler.fromXML( backupNode );
    setHandler(handler);
}

void
Object::save()
{
    XMLDocument::Pointer doc( NEW );
    XMLNode::Pointer node;
    ostringstream of;

    if(handler) {
        node.construct( );
        handler.toXML( node );
    } else
        node = backupNode;

    node->setName( "object" );
    doc->appendChild( node );
    doc->save( of );

    manager->put(id, of.str( ));

    justCreated = false;
    fromList();
}

void
Object::finalize()
{
    handler->setSelf( 0 );
    
//    Handler::Pointer dummy(handler);
    handler.clear( );
    backupNode.clear( );
    dynamicHandler = true;
    refcnt = 0;
    
    /*if not in changed list, fromList will do nothing*/
    fromList( );
    
    if(justCreated)
        manager->erase( id );
    else
        toList( manager->deleted );
}

Object::NotRecoveredException::~NotRecoveredException( ) throw()
{
}

/*-----------------------------------------------------------------
 * Manager members
 *-----------------------------------------------------------------*/
Object::Manager::Manager()
{
    manager = this;
    Context::root = Register(&at(OBJID_ROOT));
}

Object::Manager::~Manager() 
{
    // Most handlers were backed up during plugin unload.
    // But remaining handlers (e.g. FeniaProcess) may contain references to other objects.
    // Clearing handlers will cause unlink operation on referenced objects.
    // Clearing the manager's map will destroy objects in random order.
    // We explicitely destroy all hanlders here first, while all objects are still intact, to prevent
    // the memory corruption.
    for(iterator i=begin();i!=end();i++) {
        if(i->hasHandler()) {
            i->handler->setSelf(0);
            i->handler.clear( );
        }
    }

    Context::root = Register( );
    manager = 0;
}

void
Object::Manager::open()
{
    DbContext::open( "fenia", "objects" );
}

void
Object::Manager::close()
{
    sync( 0 );
    DbContext::close( );
}

void
Object::Manager::seq(id_t id, Data &val)
{
    string s((char *)val.get_data( ), val.get_size( ));
    istringstream istr(s);
    XMLDocument::Pointer doc(NEW);

    doc->load( istr );
    
    XMLNode::Pointer node = doc->getFirstNode( );
    if(!node)
        throw Exception("empty document");

    at( id ).fromXML( node );

    if(id > lastId)
        lastId = id;
}

void
Object::Manager::put( id_t id, const string &s )
{
    char buf[s.size()];
    copy(s.begin( ), s.end( ), buf);
    Data val(buf, s.size());

    DbContext::put( id, val );
}

bool
Object::Manager::tlim(struct timeval * finishAt)
{
    if(!finishAt)
        return true;

    struct timeval now;
    gettimeofday(&now, NULL);

    return timercmp(&now, finishAt, <);
}

bool
Object::Manager::syncPut(struct timeval * finishAt)
{
    Profiler prof;
    int cnt;
    
    prof.start( );
    
    for(cnt = 0; changed.next != &changed && tlim(finishAt); cnt++)
        changed.next->save( );

    prof.stop();

    if(maxPut < prof.msec( ))
        maxPut = prof.msec( );

    return changed.next == &changed;
}

DLString
Object::Manager::stats()
{
    ostringstream os;

    os  << "Max objects times (ms):" << endl
        << "  sync:   " << maxPut << endl
        << "  unlink: " << maxDel << endl
        << "  commit: " << maxCommit << endl
        << endl
        << "Total objects: " << size() << endl;

    return os.str();
}


bool
Object::Manager::syncDel(struct timeval * finishAt)
{
    Profiler prof;
    int cnt;
    
    prof.start( );
    
    for(cnt = 0; deleted.next != &deleted && tlim(finishAt); cnt++) {
        id_t id = deleted.next->getId( );
        deleted.next->fromList( );

        del(id);
        erase(id);
    }
    
    prof.stop();

    if(maxDel < prof.msec( ))
        maxDel = prof.msec( );

    return deleted.next == &deleted;
}

bool
Object::Manager::sync(struct timeval *tickEnd)
{
    struct timeval withGap, gap = { 0, 40*1000 };
    if(tickEnd) {
        timersub(tickEnd, &gap, &withGap); // 40ms gap for sure
        tickEnd = &withGap;
    }

    bool consistent = syncPut(tickEnd) && syncDel(tickEnd);

    if(consistent && txnRunning( )) {
        Profiler prof;
        prof.start( );
        commit( );
        prof.stop();

        if(maxCommit < prof.msec( ))
            maxCommit = prof.msec( );
    }

    return consistent;
}

void 
Object::Manager::backup()
{
    iterator i;
    
    for(i = begin(); i != end(); i++) 
        i->backup();
}

void
Object::Manager::recover()
{
    iterator i;
    
    for(i = begin(); i != end(); i++) 
        i->recover();
}

Object &
Object::Manager::allocate()
{
    Object *rc;
    if(deleted.next != &deleted) {
        rc = deleted.next;
        rc->fromList( );
    } else {
        rc = &BaseManager<Object>::allocate();
        rc->justCreated = true;
    }

    rc->toList(changed);
    return *rc;
}


}
