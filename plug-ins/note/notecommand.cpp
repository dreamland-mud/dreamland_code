/* $Id: notecommand.cpp,v 1.1.2.17.6.14 2009/08/16 02:50:31 rufina Exp $
 *
 * ruffina, 2004
 */

#include "locale.h"
#include "logstream.h"
#include "notemanager.h"
#include "notethread.h"
#include "note.h"
#include "noteattrs.h"
#include "listfilter_val.h"    
#include "notehooks.h"
#include "noteflags.h"

#include "class.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "xmlpcstringeditor.h"

#include "dreamland.h"
#include "merc.h"
#include "mercdb.h"
#include "arg_utils.h"
#include "comm.h"
#include "ban.h"
#include "descriptor.h"
#include "act.h"
#include "def.h"

const DLString & NoteThread::getRussianName( ) const
{
    return rusCommand;
}

short NoteThread::getLevel( ) const
{
    return min( readLevel.getValue( ), writeLevel.getValue( ) );
}

bool NoteThread::properOrder( Character *ch )
{
    return false;
}    

const DLString& NoteThread::getHint( ) const
{
    return hint;
}

void NoteThread::run( Character* cch, const DLString& constArguments ) 
{
    XMLAttributeNoteData::Pointer attr;
    PCharacter *ch;
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    
    if (cch->is_npc( ))
        return;

    ch = cch->getPC( );

    if (cmd.empty( )) {
        if (canRead( ch ))
            cmd = "read";
        else {
            ch->pecho( "Ты не можешь читать {W%N4{x.", rusNameMlt.c_str( ) );
            return;
        }
    }
    
    if (cmd.strPrefix( "dump" ) && ch->isCoder( )) {
        dump( );
        ch->println( "Ok." );
        return;
    }

    if (arg_oneof_strict(cmd, "flag", "флаг") && ch->isCoder()) {
        doFlag(ch, arguments);
        return;
    }

    if (arg_oneof_strict(cmd, "save", "сохранить") && ch->isCoder()) {
        saveAllBuckets();
        ch->println("Ok.");
        return;
    }

    if (canRead( ch )) {
        if (arg_oneof( cmd, "read", "читать" )) {
            doRead( ch, arguments );
            return;
        }
        else if (arg_oneof( cmd, "copy", "копировать" ) && !arg_oneof_strict( cmd, "к" )) {
            doCopy( ch, arguments );
            return;
        }
        else if (arg_is_list( cmd )) {
            doList( ch, arguments );
            return;
        }
        else if (arg_oneof( cmd, "catchup", "прочитаны" )) {
            doCatchup( ch );
            return;
        }
        else if (arg_oneof( cmd, "uncatchup", "непрочитаны" )) {
            doUncatchup( ch, arguments );
            return;
        }
    }

    if (!canWrite( ch )) {
        ch->pecho( "У тебя недостаточно привилегий для написания {W%N2{x.", rusNameMlt.c_str( ) );
        return;
    }

    if (!IS_SET(ch->act, PLR_CONFIRMED) && ch->getRemorts( ).size( ) == 0) {
        ch->println("Ты не можешь ничего написать, пока тебя не подтвердили Боги." );
        return;
    }

    if (ch->getRealLevel( ) < 3 && ch->getRemorts( ).size( ) == 0) {
        ch->println("Тебе нужно достичь 3 уровня, чтобы отправлять письма.");
        return;
    }

    if (canRead( ch )) {
        if (arg_oneof( cmd, "remove", "delete", "удалить" )) {
            doRemove( ch, arguments );
            return;
        }
    }

    static const DLString nopost( "nopost" );
    if (ch->getAttributes( ).isAvailable( nopost ) || banManager->check( ch->desc, BAN_COMMUNICATE )) {
        ch->println( "Боги лишили тебя возможности писать сообщения." );
        return;
    }

    attr = ch->getAttributes( ).getAttr<XMLAttributeNoteData>( "notedata" );
    
    if (canRead( ch ) && arg_oneof( cmd, "forward", "перенаправить" )) {
        doForward( ch, attr, arguments );
        return;
    }
    else if (cmd == "+") {
        doLinePlus( ch, attr, arguments );
        return;
    }
    else if (arg_oneof( cmd, "paste", "вставить" )) {
        doPaste( ch, attr );
        return;
    }
    else if (cmd == "-") {
        doLineMinus( ch, attr );
        return;
    }
    else if (arg_oneof( cmd, "subject", "тема" )) {
        doSubject( ch, attr, arguments );
        return;
    }
    else if (arg_oneof_strict( cmd, "to", "к" ) || arg_oneof( cmd, "кому" )) {
        doTo( ch, attr, arguments );
        return;
    }
    else if (arg_oneof_strict( cmd, "from", "от" )) {
        if (doFrom( ch, attr, arguments ))
            return;
    }
    else if (arg_oneof( cmd, "clear", "cancel", "очистить", "отменить" )) {
        if (doClear( ch, attr ))
            return;
    }
    else if (arg_oneof( cmd, "show", "показать" )) {
        if (doShow( ch, attr ))
            return;
    }
    else if (arg_oneof( cmd, "post", "send", "послать", "отправить" )) {
        if (doPost( ch, attr ))
            return;
    }
    else {
        ch->println("Неверная команда, смотри {W? {lRписьмо синтаксис{lEnote syntax{x.");
        return;
    }
    
    echo( ch, msgNoCurrent, "Ты не пишешь никакого письма." );
}

/**
 * Coder command:
 * '<note> flag <id> <flags>' - sets specified flag on a note with given number.
 * Note bucket is not saved, run '<note> save' to do so.
 */
void NoteThread::doFlag(PCharacter *ch, DLString &arguments )
{
    Integer noteId;
    DLString arg = arguments.getOneArgument( );
    
    if (!Integer::tryParse(noteId, arg)) {
        if (arg.empty())
            ch->println("Укажите номер письма.");
        else
            ch->println("Неправильный номер письма.");
        ch->printf( "Использование: %s flag <number> <flags to set|none>\r\n", getName( ).c_str( ) );
        return;
    }

    const Note *cnote = getNoteAtPosition( ch, noteId );
    if (!cnote) {
        ch->pecho( "Так много %N2 еще не написали.", rusNameMlt.c_str( ) );
        return;
    }
        
    Note *note = findNote(cnote->getID());
    if (!note) {
        ch->println("Что-то пошло не так, письмо не найдено.");
        return;
    }

    bitstring_t flags = (arguments == "none") ? 0 : note_flags.bitstring(arguments, false); 
    if (flags == NO_FLAG) {
        ch->pecho("Флаг не найден.");
        return;
    }

    note->setFlags(flags);
    ch->pecho("%^N3 номер %d установлены флаги %s.", 
            rusName.c_str(), noteId.getValue(), note->getFlags().names().c_str());
    ch->pecho("Используйте команду '%s save' для сохранения изменений на диск.", getName().c_str());
}

bool NoteThread::doShow( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note;

    if (( note = attr->findNote( this ) )) {
        ostringstream buf;
        note->toStream( buf );
        page_to_char( buf.str( ).c_str( ), ch );
        return true;
    }

    return false;
}

bool NoteThread::doClear( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note;

    if (( note = attr->findNote( this ) )) {
        attr->clearNote( this );
        ch->println( "Ok." );
        return true;
    }

    return false;
}

void NoteThread::doSubject( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );
    note->setSubject( arguments );
    ch->println( "Ok." );
}

void NoteThread::doTo( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    ostringstream buf;

    XMLNoteData *note = attr->makeNote( ch, this );
    note->setRecipient( arguments );
    
    if (!Note::parseRecipient( ch, arguments, buf ))
        echo( ch, msgToError, "{RТвое сообщение никто не получит!{x" );
    else
        echo( ch, msgToOk, "Твое сообщение получат:\r\n", buf.str( ) );
}

void NoteThread::doLinePlus( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );

    if (!ch->isCoder( ) && note->getBodySize( ) > 4096) 
        echo( ch, msgTooLong, "Слишком длинное сообщение." );
    else {
        note->addLine( arguments );
        ch->println( "Ok." );
    }
}

void NoteThread::doLineMinus( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );

    if (note->isBodyEmpty( )) 
        ch->println( "Больше нечего удалять." );
    else {
        note->delLine( );
        ch->println( "Ok." );
    }
}

void NoteThread::doPaste( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note = attr->makeNote( ch, this );

    const Editor::reg_t &reg = ch->getAttributes()
        .getAttr<XMLAttributeEditorState>("edstate")->regs[0];

    note->clearBody( );
    
    for(Editor::reg_t::const_iterator i = reg.begin(); i != reg.end(); i++)
        note->addLine( *i );

    ch->println( "Ok." );
}

void NoteThread::doCatchup( PCharacter *ch ) const
{
    ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" )->updateStamp( this );
    ch->println( "Ok." );
}

void NoteThread::doCopy( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
   
    if (arg.empty( )) {
        XMLAttributeNoteData::Pointer attr = ch->getAttributes( ).getAttr<XMLAttributeNoteData>( "notedata" );
        XMLNoteData *note = attr->findNote( this );
            if (!note) {
                ch->println("Ты не редактируешь сообщение, копировать нечего.");
                return;
        }

        ostringstream buf;
        note->linesToStream( buf );
        ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
            ->regs[0].split(buf.str( ));
        ch->println( "Текущее сообщение скопировано в буфер." );
        return;
    }
 
    if (arg.isNumber( )) {
        try {
            note = getNoteAtPosition( ch, arg.toInt( ) );

            if (note) {
                ostringstream ostr;
                note->toStream(arg.toInt( ), ostr);
                ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
                    ->regs[0].split(ostr.str( ));
                ch->println( "Ok." );
            } else
                ch->pecho( "Так много %N2 еще не написали.", rusNameMlt.c_str( ) );
                
        } catch (const ExceptionBadType& e) {
            ch->println( "Неправильный номер письма." );
        }
    }
    else {
        ch->println( "Скопировать какой номер?" );
    }
}

void NoteThread::doRead( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (arg.empty( ) || arg_oneof( arg, "next", "следующий", "дальше" )) {
        note = getNextUnreadNote( ch );
        
        if (note) 
            showNoteToChar( ch, note );
        else
            ch->pecho( "У тебя нет непрочитанных %N2.", rusNameMlt.c_str( ) );
    }
    else if (arg.isNumber( )) {
        try {
            note = getNoteAtPosition( ch, arg.toInt( ) );

            if (note)
                showNoteToChar( ch, note );
            else
                ch->pecho( "Так много %N2 еще не написали.", rusNameMlt.c_str( ) );
                
        } catch (const ExceptionBadType& e) {
            ch->println( "Неправильный номер письма." );
        }
    }
    else {
        ch->println( "Прочесть какой номер?" );
    }
}

void NoteThread::doList( PCharacter *ch, DLString &argument ) const
{
    ostringstream buf;
    NoteList mynotes;
    NoteList::const_iterator i;
    time_t stamp = getStamp( ch );
    
    for (i = xnotes.begin( ); i != xnotes.end( ); i++)
        if ((*i)->isNoteTo( ch ) || (*i)->isNoteFrom( ch ))
            mynotes.push_back( *i );

    try {
        int cnt, last = mynotes.size( ) - 1;
        
        for (cnt = 0, i = mynotes.begin( ); i != mynotes.end( ); i++, cnt++) {
            bool hidden = isNoteHidden( *i, ch, stamp );

            if (listfilter_parse( ch, cnt, last, *i, hidden, argument.c_str( ) )) {
                buf << "[" << cnt 
                    << (hidden ? " " : "N") << "] " 
                    << (*i)->getFrom( ) << ": " << (*i)->getSubject( ) << "{x";
                if (ch->isCoder() && (*i)->getFlags().getValue() != 0)
                    buf << "  {D[" << (*i)->getFlags().names() << "]{x";
                buf << endl;
            }
        }
    }
    catch (const Exception &e) {
        ch->println("Неверный формат, смотри {y{hc{lRсправка письмо список{lEhelp note list{x." );
        return;
    }
    
    if (buf.str( ).empty( ))
        ch->println( "Не найдено ни одного сообщения." );
    else
        page_to_char( buf.str( ).c_str( ), ch );
}

void NoteThread::doRemove( PCharacter *ch, DLString &arguments )
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (!arg.isNumber( )) {
        ch->println( "Удалить какой номер?" );
        return;
    }

    try {
        note = getNoteAtPosition( ch, arg.toInt( ) );

        if (note) {
            if (ch->get_trust( ) < CREATOR && note->getAuthor( ) != ch->getName( ))
                ch->println( "Ты не можешь удалить это сообщение, т.к. не являешься его автором." );
            else {
                LogStream::sendNotice( ) 
                    << getName( ) << " remove: " 
                    << ch->getName( ) << ", id " << note->getID( ) << endl;
                remove( note );
                ch->println( "Ok." );
            }
        }
        else
            ch->pecho( "Так много %N2 еще не написали.", rusNameMlt.c_str( ) );
    } 
    catch (const ExceptionBadType& e) {
        ch->println( "Неправильный номер письма." );
    }
}

void NoteThread::doUncatchup( PCharacter *ch, DLString &arguments ) const
{
    XMLAttributeLastRead::Pointer attr;
    DLString arg = arguments.getOneArgument( );
    
    attr = ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" );

    if (!arg.isNumber( )) {
        attr->setStamp( this, 0 );
        ch->printf( "Все %s помечены как непрочитанные.\r\n", nameMlt.getValue( ).c_str( ) );
        return;
    }

    try {
        int vnum = arg.toInt( );
        const Note *note = getNoteAtPosition( ch, vnum );
        
        if (note) {
            attr->setStamp( this, note->getID( ) );
            ch->printf( "Все %s, начиная с номера %d, помечены как непрочитанные.\r\n", 
                        nameMlt.getValue( ).c_str( ), vnum );
        }
        else
            ch->pecho( "Так много %N2 еще не написали.", rusNameMlt.c_str( ) );
    }
    catch (const ExceptionBadType& e) {
        ch->println( "Неправильный номер письма." );
    }
}

bool NoteThread::doPost( PCharacter *ch, XMLAttributeNoteData::Pointer attr )
{
    XMLNoteData *notedata;

    if (!( notedata = attr->findNote( this ) ))
        return false;

    if (notedata->getRecipient( ).empty( )) {
        echo( ch, msgNoRecepient, "Укажите адресата письма." );
        return false;
    }

    Note note;
    notedata->commit( &note );
    note.godsSeeAlways = godsSeeAlways;
    attach( &note );

    echo( ch, msgSent, "Письмо отправлено." );
    LogStream::sendNotice( ) 
        << getName( ) << " post: " 
        << ch->getName( ) << ", id " << note.getID( ) << endl;

    NoteHooks::processNoteMessage( *this, note );
    attr->clearNote( this );
    return true;
}

bool NoteThread::doFrom( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    if (ch->get_trust( ) >= GOD) {
        XMLNoteData *note = attr->makeNote( ch, this );
        DLString arg = arguments;
        
        arg = arg.getOneArgument( );
        arg.colourstrip( );

        if (PCharacterManager::find( arg )) {
            ch->println( "Подделка документов запрещена!" );
        }
        else {
            note->setFrom( arguments );
            ch->println( "Ok." );
        }

        return true;
    }

    return false;
}

void NoteThread::doForward( PCharacter *ch, XMLAttributeNoteData::Pointer attr, DLString &arguments ) const
{
    const Note *orig;
    XMLNoteData *note;
    ostringstream buf;
    DLString arg = arguments.getOneArgument( );
    
    if (!arg.isNumber( )) {
        ch->println( "Перенаправить какой номер?" );
        return;
    }

    try {
        orig = getNoteAtPosition( ch, arg.toInt( ) );
    } 
    catch (const ExceptionBadType& e) {
        ch->println( "Неправильный номер письма." );
        return;
    }

    if (!orig) {
        ch->pecho( "Так много %N2 еще не написали.", rusNameMlt.c_str( ) );
        return;
    }

    note = attr->makeNote( ch, this );
    orig->toForwardStream( buf );
    note->addLine( buf.str( ) );
    ch->println( "Ok." );
}

void NoteThread::echo( PCharacter *ch, const DLString &msg, const DLString &defaultMsg, const DLString &arg ) const
{
    ostringstream buf;
    buf << (msg.empty( ) ? defaultMsg : msg) << arg << endl;
    ch->send_to( buf );
}

const Flags & NoteThread::getExtra( ) const
{
    return extra;
}
