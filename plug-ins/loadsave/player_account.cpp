#include "player_account.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "commonattributes.h"
#include "interp.h"

void Player::quitAndDelete( PCharacter *victim ) 
{
    DLString name = victim->getName( );

    victim->getAttributes( ).getAttr<XMLStringAttribute>( "quit_flags" )->setValue( "quiet count forced" );
    interpret_raw( victim, "quit", "" );
    PCharacterManager::pfDelete( name );
}
