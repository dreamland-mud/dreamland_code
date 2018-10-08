/* $Id: socialmanager.h,v 1.1.2.2.6.1 2007/06/26 07:21:13 rufina Exp $
 *
 * ruffina, 2004
 */
/* 
 *
 * sturm, 2003
 */

#ifndef SOCIALMANAGER_H
#define SOCIALMANAGER_H

#include "dlxmlloader.h"
#include "interpretlayer.h"

class SocialManager : public InterpretLayer, public DLXMLTableLoader {
friend class Social;    
public:
        typedef ::Pointer<SocialManager> Pointer;

        SocialManager( );
        virtual ~SocialManager( );
        
        virtual DLString getTableName( ) const;
        virtual DLString getNodeName( ) const;

        inline static SocialManager * getThis( );

        virtual bool process( InterpretArguments & );
protected:
        virtual void putInto( );

        virtual void initialization( );
        virtual void destruction( );

private:
        static SocialManager *thisClass;

        static const DLString NODE_NAME;
        static const DLString TABLE_NAME;
};

inline SocialManager * SocialManager::getThis( )
{
    return thisClass;
}

#endif 
