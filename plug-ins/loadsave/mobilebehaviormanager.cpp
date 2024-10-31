/* $Id: mobilebehaviormanager.cpp,v 1.1.2.1 2009/09/19 00:53:18 rufina Exp $
 *
 * ruffina, 2003
 */
#include "mobilebehaviormanager.h"
#include "mobilebehavior.h"
#include "xmldocument.h"
#include "logstream.h"
#include "plugininitializer.h"
#include "dlxmlloader.h"

#include "npcharacter.h"
#include "fread_utils.h"

#include "def.h"

/**
 * Class responsible for reading XML behavior description from a folder;
 * useful for specifying big default behaviors such as healer or questor.
 */
class MobileBehaviorLoader : public DLXMLLoader, public Plugin {
public:
    
    MobileBehaviorLoader() {
        thisClass = this;
    }
    virtual ~MobileBehaviorLoader() {
        thisClass = 0;
    }

    virtual void initialization( ) {}
    virtual void destruction( ) {}

    virtual DLString getTableName( ) const {
        return "legacy-behaviors";
    }
    virtual DLString getNodeName( ) const {
        return MobileBehavior::NODE_NAME;
    }
    
    inline static MobileBehaviorLoader *getThis( ) {
        return thisClass;
    }

private:
    static MobileBehaviorLoader *thisClass;
};

MobileBehaviorLoader * MobileBehaviorLoader::thisClass = 0;

PluginInitializer<MobileBehaviorLoader> initMobileBehaviorLoader(INITPRIO_NORMAL);

void MobileBehaviorManager::assign( NPCharacter *mob ) {
    if (!mob->pIndexData->behavior)
        return;
    
    try {
        if (mob->behavior) {
            mob->behavior->unsetChar( );
            mob->behavior.clear( );
        }

        const XMLNode::Pointer &rootNode = mob->pIndexData->behavior->getFirstNode();                
        const DLString & type = rootNode->getAttribute( XMLNode::ATTRIBUTE_TYPE );

        // First load behavior from its XML definition in the area file.
        mob->behavior.fromXML( rootNode );

        // Try to override behavior definition from a file in 'share/DL/behaviors' folder.
        // Careful here to pass the original pointer, so that fromXML on the underlying class is called,
        // rather than the one on the XMLPolymorphPointer.
        MobileBehaviorLoader::getThis()->loadXML(
            mob->behavior.getPointer(), type, true);

        mob->behavior->setChar( mob );

    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void MobileBehaviorManager::assignBasic( NPCharacter *mob ) 
{
    static const DLString basicName( "BasicMobileBehavior" );

    if (mob->behavior) {
        mob->behavior->unsetChar( );
        mob->behavior.clear( );
    }

    try {
        AllocateClass::Pointer pointer = Class::allocateClass( basicName );
        MobileBehavior::Pointer behavior = pointer.getDynamicPointer<MobileBehavior>( );
        
        if (!behavior)
            throw Exception( "BasicMobileBehavior is not derived from MobileBehavior" );
        
        mob->behavior.setPointer( *behavior );
        mob->behavior->setChar( mob );
    }
    catch (const ExceptionClassNotFound &e ) {
    }
}

void MobileBehaviorManager::parse( MOB_INDEX_DATA * pMobIndex, FILE *fp ) {
    char letter;
    DLString word;
    std::basic_istringstream<char> istr;
    XMLDocument::Pointer doc( NEW );
   
    if (feof( fp ))
        return;
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
        
    word = fread_dlstring( fp );

    try {
        istr.str( word );
        
        doc->load( istr );
        pMobIndex->behavior = new XMLDocument( **doc );

    } catch (Exception e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void MobileBehaviorManager::parse( NPCharacter * mob, FILE *fp ) {
    char letter;
    DLString word;
    
    if (feof( fp ))
        return;
    
    letter = fread_letter( fp );
    ungetc( letter, fp );

    if (letter != '<') 
        return;
    
    word = fread_dlstring( fp );

    try {
        std::basic_istringstream<char> istr( word );
        
        if (mob->behavior) {
            mob->behavior->unsetChar( );
            mob->behavior.clear( );
        }
        
        mob->behavior.fromStream( istr );
        mob->behavior->setChar( mob );

    } catch (Exception e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void MobileBehaviorManager::save( const MOB_INDEX_DATA *pMobIndex, FILE *fp ) {
    std::basic_ostringstream<char> ostr;
     
    if (!pMobIndex->behavior)
        return;
    
    try {
        pMobIndex->behavior->save( ostr );
        fprintf( fp, "%s~\n", ostr.str( ).c_str( ) );

    } catch (ExceptionXMLError e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

void MobileBehaviorManager::save( const NPCharacter *mob, FILE *fp ) {
    std::basic_ostringstream<char> ostr;
     
    if (!mob->behavior)
        return;

    if (!mob->behavior->isSaved( ))
        return;
    
    try {
        mob->behavior.toStream( ostr );
        fprintf( fp, "%s~\n", ostr.str( ).c_str( ) );

    } catch (ExceptionXMLError e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }
}

