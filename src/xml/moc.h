/* $Id: moc.h,v 1.1.2.1.8.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef __MOC_H__
#define __MOC_H__

#define MOC_GET_TYPE(Class)  \
const DLString & Class::mocGetType( ) const \
{ \
    return MOC_TYPE; \
}

#define MOC_CLASS_DECLS   \
    struct NodeType {  \
        typedef void fromXML_t( ContainerType *, const XMLNode::Pointer& );  \
        typedef bool toXML_t( const ContainerType *, XMLNode::Pointer& );  \
  \
        void set(fromXML_t *f, toXML_t *t) {  \
            fromXML = f;  \
            toXML = t;  \
        }  \
        fromXML_t *fromXML;  \
        toXML_t *toXML;  \
    };  \
      \
    typedef std::map<DLString, NodeType> NodesType;  \
    NodesType nodes;

#define MOC_NODES_INIT_VAR(name) \
    nodes[#name].set( &var_ ## name ## _fromXML, var_ ## name ## _toXML );
    
#define MOC_NODE_FROM_XML(Class) \
bool Class::mocNodeFromXML( const XMLNode::Pointer &parent ) \
{ \
    __MetaInfo__::NodesType::const_iterator i; \
     \
    i = __MetaInfo__::instance.nodes.find( parent->getName( ) ); \
 \
    if (i == __MetaInfo__::instance.nodes.end( )) \
        return false; \
 \
    i->second.fromXML( this, parent ); \
    return true; \
}

#define MOC_TO_XML(Class)  \
bool Class::mocToXML( XMLNode::Pointer &parent ) const \
{ \
    __MetaInfo__::NodesType::const_iterator i; \
     \
    for (i = __MetaInfo__::instance.nodes.begin( );  \
         i != __MetaInfo__::instance.nodes.end( );  \
         i++)  \
    { \
        XMLNode::Pointer child( NEW ); \
         \
        try { \
            if (i->second.toXML( this, child )) { \
                child->setName( i->first ); \
                parent->appendChild( child ); \
            } \
        }  \
        catch (const ExceptionSkipVariable &) { \
        } \
    } \
    return true; \
}

#define MOC_VARIABLE(name)  \
    static void var_ ## name ## _fromXML( ContainerType *container, const XMLNode::Pointer& parent ) throw( ExceptionBadType ) \
    { \
        container->name.fromXML( parent ); \
    } \
    static bool var_ ## name ## _toXML( const ContainerType *container, XMLNode::Pointer& parent ) \
    { \
        return container->name.toXML( parent ); \
    }

#endif
