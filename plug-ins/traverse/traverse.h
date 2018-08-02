/* $Id: traverse.h,v 1.1.2.1 2005/09/10 21:13:06 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __TRAVERSE_H__
#define __TRAVERSE_H__

template <typename TraverseTraits>
struct NodesEntry {
    typedef typename TraverseTraits::NodeType  NodeType;
    typedef typename TraverseTraits::HookType  HookType;
    
    NodeType * node;
    HookType hook;
    NodesEntry * prev;
    int generation;
};

template <typename TraverseTraits, typename HooksIterator, typename TraverseComplete>
struct BroadTraverse {     
    typedef typename TraverseTraits::NodeType  NodeType;
    typedef typename TraverseTraits::HookType  HookType;

    typedef ::NodesEntry<TraverseTraits> NodesEntry;

    struct AddHook {
	inline AddHook( BroadTraverse *bt ) : traverse( bt )
	{
	}
	
	inline void operator () ( HookType &hook ) const
	{
	    traverse->addHook( hook );
	}

	BroadTraverse *traverse;
    };

    HooksIterator foreach_hook;
    TraverseComplete complete;

    BroadTraverse(  HooksIterator fh = HooksIterator(), 
		    TraverseComplete c = TraverseComplete())
	: foreach_hook(fh), complete(c)
    {
    }
    
    inline void operator () ( NodeType *start, int top)
    {
	NodesEntry *nodes = new NodesEntry[top];
	
	nodes[0].node = start;
	nodes[0].prev = 0;
	nodes[0].generation = 0;

	begin = nodes;
	end = nodes + top;
	tail = begin + 1;
	
	AddHook ah( this );

	for (head = begin; head != tail; head++) {

	    if (complete( head, false ))
		break;
	     
	    foreach_hook( head->node, ah );
	}
    
	if (head == tail)
	    complete( head - 1, true );

	register NodesEntry *i;
	for (i = begin; i != tail; i++) 
	    TraverseTraits::unmark( i->node );

	delete nodes;
    }
   
    inline void addHook( HookType &hook ) {
	if (tail == end)
	    return;

	register NodeType * node = hook.target( head->node );

	if (TraverseTraits::marked( node ))
	    return;
	
	TraverseTraits::mark( node );

	tail->node = node;
	tail->prev = head;
	tail->hook = hook;
	tail->generation = head->generation + 1;
	tail++;
    }

    NodesEntry *head, *tail, *begin, *end;
};

#endif
