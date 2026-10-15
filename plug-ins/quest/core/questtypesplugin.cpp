/* Dream Land, 2026 */
#include <algorithm>

#include "questtypesplugin.h"
#include "questmanager.h"
#include "dldirectory.h"
#include "exceptiondbio.h"
#include "exceptiondbioeof.h"
#include "dreamland.h"
#include "logstream.h"

void QuestTypesPlugin::initialization( )
{
    QuestManager *manager = QuestManager::getThis( );
    std::vector<DLString> names;

    try {
        DLDirectory dir( dreamland->getTableDir( ), manager->getTableName( ) );
        dir.open( );

        for ( ;; ) {
            DLString name = dir.nextTypedEntry( ".xml" ).getFileName( );

            if (!name.empty( ))
                names.push_back( name );
        }
    }
    catch (const ExceptionDBIOEOF &) {
    }
    catch (const ExceptionDBIO &ex) {
        LogStream::sendError( ) << "Quest types: " << ex.what( ) << endl;
    }

    // readdir order is arbitrary; the questor menu breaks priority ties by
    // registration order, so keep it stable.
    std::sort( names.begin( ), names.end( ) );

    for (auto &name: names) {
        QuestRegistratorBase::Pointer type( NEW );

        type->setName( name );
        if (!manager->load( type.getPointer( ) ))
            continue;

        types.push_back( type );
        // After load(), because getID() reads feniaId out of the config file.
        type->linkFeniaWrapper( );

        // A type that relies on onApplicable to pick its heroes is withheld
        // from everyone while the hook is missing. Say so at boot, not when a
        // player notices the type is gone.
        if (type->needsFeniaApplicable( ) && !type->hasFeniaApplicable( ))
            LogStream::sendError( )
                << "Quest type " << name << " sets <needsApplicable> but has no Fenia"
                << " onApplicable -- nobody will be offered it" << endl;
    }

    LogStream::sendNotice( ) << "Quest types: " << types.size( ) << " registered" << endl;
}

void QuestTypesPlugin::destruction( )
{
    // Before unLoad(), so the wrapper is detached while the type is still
    // registered. Left attached, a `plug reload` would strand it. The Fenia
    // side may already be gone by now -- see unlinkFeniaWrapper.
    for (auto &type: types) {
        type->unlinkFeniaWrapper( );
        QuestManager::getThis( )->unLoad( type.getPointer( ) );
    }

    types.clear( );
}
