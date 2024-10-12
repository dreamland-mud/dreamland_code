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
#include "commandmanager.h"

#include "dreamland.h"
#include "merc.h"

#include "arg_utils.h"
#include "comm.h"
#include "ban.h"
#include "descriptor.h"
#include "act.h"
#include "def.h"

void NoteCommand::setThread(NoteThread::Pointer thread)
{
    this->thread = thread;

    commandManager->registrate( Pointer( this ) );
}

NoteThread::Pointer NoteCommand::getThread() const
{
    return thread;
}

void NoteCommand::unsetThread()
{
    commandManager->unregistrate( Pointer( this ) );
    
    thread.clear();
}

bool NoteCommand::saveCommand() const
{
    if (thread)
        return NoteManager::getThis()->saveXML(*thread, thread->getName());

    return false;
}


void NoteCommand::run( Character* cch, const DLString& constArguments ) 
{
    XMLAttributeNoteData::Pointer attr;
    PCharacter *ch;
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    
    if (!thread)
        throw Exception("Note command not connected to a thread");

    if (cch->is_npc( ))
        return;

    ch = cch->getPC( );

    if (cmd.empty( )) {
        if (thread->canRead( ch ))
            cmd = "read";
        else {
            ch->pecho( "Ты не можешь читать {W%N4{x.", thread->getRussianMltName().c_str( ) );
            return;
        }
    }
    
    if (cmd.strPrefix( "dump" ) && ch->isCoder( )) {
        thread->dump( );
        ch->pecho( "Ok." );
        return;
    }

    if (arg_oneof_strict(cmd, "flag", "флаг") && ch->isCoder()) {
        doFlag(ch, arguments);
        return;
    }

    if (arg_oneof_strict(cmd, "save", "сохранить") && ch->isCoder()) {
        thread->saveAllBuckets();
        ch->pecho("Ok.");
        return;
    }

    if (thread->canRead( ch )) {
        if (arg_oneof( cmd, "read", "читать" )) {
            doRead( ch, arguments );
            return;
        }
        else if (arg_oneof( cmd, "copy", "копировать" ) && !arg_oneof_strict( cmd, "note.to" )) {
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

    if (!thread->canWrite( ch )) {
        ch->pecho( "У тебя недостаточно привилегий для написания {W%N2{x.", thread->getRussianMltName().c_str( ) );
        return;
    }

    if (!IS_SET(ch->act, PLR_CONFIRMED) && ch->getRemorts( ).size( ) == 0) {
        ch->pecho("Ты не можешь ничего написать, пока тебя не подтвердили Боги." );
        return;
    }

    if (thread->canRead( ch )) {
        if (arg_oneof( cmd, "del", "delete", "удалить" )) {
            doRemove( ch, arguments );
            return;
        }
    }

    static const DLString nopost( "nopost" );
    if (ch->getAttributes( ).isAvailable( nopost ) || banManager->check( ch->desc, BAN_COMMUNICATE )) {
        ch->pecho( "Боги лишили тебя возможности писать сообщения." );
        return;
    }

    attr = ch->getAttributes( ).getAttr<XMLAttributeNoteData>( "notedata" );
    
    if (thread->canRead( ch ) && arg_oneof( cmd, "forward", "перенаправить" )) {
        doForward( ch, attr, arguments );
        return;
    }
    else if (cmd == "+") {
        doLinePlus( ch, attr, arguments );
        return;
    }
    else if (arg_is_paste(cmd)) {
        doPaste( ch, attr );
        return;
    }
    else if (cmd == "-") {
        doLineMinus( ch, attr );
        return;
    }
    else if (arg_oneof( cmd, "subj", "тема" )) {
        doSubject( ch, attr, arguments );
        return;
    }
    else if (arg_oneof_strict( cmd, "note.to", "к" )) {
        doTo( ch, attr, arguments );
        return;
    }
    else if (arg_oneof_strict( cmd, "note.from", "от" )) {
        if (doFrom( ch, attr, arguments ))
            return;
    }
    else if (arg_is_clear(cmd)) {
        if (doClear( ch, attr ))
            return;
    }
    else if (arg_is_show(cmd)) {
        if (doShow( ch, attr ))
            return;
    }
    else if (arg_oneof( cmd, "post", "send", "послать", "отправить" )) {
        if (doPost( ch, attr ))
            return;
    }
    else {
        ch->pecho("Неверная команда, смотри {W? письмо синтаксис{x.");
        return;
    }
    
    echo( ch, thread->msgNoCurrent, "Ты не пишешь никакого письма." );
}

/**
 * Coder command:
 * '<note> flag <id> <flags>' - sets specified flag on a note with given number.
 * Note bucket is not saved, run '<note> save' to do so.
 */
void NoteCommand::doFlag(PCharacter *ch, DLString &arguments )
{
    Integer noteId;
    DLString arg = arguments.getOneArgument( );
    
    if (!Integer::tryParse(noteId, arg)) {
        if (arg.empty())
            ch->pecho("Укажите номер письма.");
        else
            ch->pecho("Неправильный номер письма.");
        ch->pecho( "Использование: %s flag <number> <flags to set|none>", thread->getName( ).c_str( ) );
        return;
    }

    const Note *cnote = thread->getNoteAtPosition( ch, noteId );
    if (!cnote) {
        ch->pecho( "Так много %N2 еще не написали.", thread->getRussianMltName().c_str( ) );
        return;
    }
        
    Note *note = thread->findNote(cnote->getID());
    if (!note) {
        ch->pecho("Что-то пошло не так, письмо не найдено.");
        return;
    }

    bitstring_t flags = (arguments == "none") ? 0 : note_flags.bitstring(arguments, false); 
    if (flags == NO_FLAG) {
        ch->pecho("Флаг не найден.");
        return;
    }

    note->setFlags(flags);
    ch->pecho("%^N3 номер %d установлены флаги %s.", 
            thread->getRussianThreadName().c_str(), noteId.getValue(), note->getFlags().names().c_str());
    ch->pecho("Используйте команду '%s save' для сохранения изменений на диск.", getName().c_str());
}

bool NoteCommand::doShow( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note;

    if (( note = attr->findNote( *thread ) )) {
        ostringstream buf;
        note->toStream( buf );
        page_to_char( buf.str( ).c_str( ), ch );
        return true;
    }

    return false;
}

bool NoteCommand::doClear( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note;

    if (( note = attr->findNote( *thread ) )) {
        attr->clearNote( *thread );
        ch->pecho( "Ok." );
        return true;
    }

    return false;
}

void NoteCommand::doSubject( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    XMLNoteData *note = attr->makeNote( ch, *thread );
    note->setSubject( arguments );
    ch->pecho( "Ok." );
}

void NoteCommand::doTo( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    ostringstream buf;

    XMLNoteData *note = attr->makeNote( ch, *thread );
    note->setRecipient( arguments );
    
    if (!Note::parseRecipient( ch, arguments, buf ))
        echo( ch, thread->msgToError, "{RТвое сообщение никто не получит!{x" );
    else
        echo( ch, thread->msgToOk, "Твое сообщение получат:\r\n", buf.str( ) );
}

void NoteCommand::doLinePlus( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    XMLNoteData *note = attr->makeNote( ch, *thread );

    if (!ch->isCoder( ) && note->getBodySize( ) > 4096) 
        echo( ch, thread->msgTooLong, "Слишком длинное сообщение." );
    else {
        note->addLine( arguments );
        ch->pecho( "Ok." );
    }
}

void NoteCommand::doLineMinus( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note = attr->makeNote( ch, *thread );

    if (note->isBodyEmpty( )) 
        ch->pecho( "Больше нечего удалять." );
    else {
        note->delLine( );
        ch->pecho( "Ok." );
    }
}

void NoteCommand::doPaste( PCharacter *ch, XMLAttributeNoteData::Pointer attr ) const
{
    XMLNoteData *note = attr->makeNote( ch, *thread );

    const Editor::reg_t &reg = ch->getAttributes()
        .getAttr<XMLAttributeEditorState>("edstate")->regs[0];

    note->clearBody( );
    
    for(Editor::reg_t::const_iterator i = reg.begin(); i != reg.end(); i++)
        note->addLine( *i );

    ch->pecho( "Ok." );
}

void NoteCommand::doCatchup( PCharacter *ch ) const
{
    ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" )->updateStamp( *thread );
    ch->pecho( "Ok." );
}

void NoteCommand::doCopy( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
   
    if (arg.empty( )) {
        XMLAttributeNoteData::Pointer attr = ch->getAttributes( ).getAttr<XMLAttributeNoteData>( "notedata" );
        XMLNoteData *note = attr->findNote( *thread );
            if (!note) {
                ch->pecho("Ты не редактируешь сообщение, копировать нечего.");
                return;
        }

        ostringstream buf;
        note->linesToStream( buf );
        ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
            ->regs[0].split(buf.str( ));
        ch->pecho( "Текущее сообщение скопировано в буфер." );
        return;
    }
 
    if (arg.isNumber( )) {
        try {
            note = thread->getNoteAtPosition( ch, arg.toInt( ) );

            if (note) {
                ostringstream ostr;
                note->toStream(arg.toInt( ), ostr);
                ch->getAttributes().getAttr<XMLAttributeEditorState>("edstate")
                    ->regs[0].split(ostr.str( ));
                ch->pecho( "Ok." );
            } else
                ch->pecho( "Так много %N2 еще не написали.", thread->getRussianMltName().c_str( ) );
                
        } catch (const ExceptionBadType& e) {
            ch->pecho( "Неправильный номер письма." );
        }
    }
    else {
        ch->pecho( "Скопировать какой номер?" );
    }
}

void NoteCommand::doRead( PCharacter *ch, DLString &arguments ) const
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (arg.empty( ) || arg_oneof( arg, "next", "следующий", "дальше" )) {
        note = thread->getNextUnreadNote( ch );
        
        if (note) 
            thread->showNoteToChar( ch, note );
        else
            ch->pecho( "У тебя нет непрочитанных %N2.", thread->getRussianMltName().c_str( ) );
    }
    else if (arg.isNumber( )) {
        try {
            note = thread->getNoteAtPosition( ch, arg.toInt( ) );

            if (note)
                thread->showNoteToChar( ch, note );
            else
                ch->pecho( "Так много %N2 еще не написали.", thread->getRussianMltName().c_str( ) );
                
        } catch (const ExceptionBadType& e) {
            ch->pecho( "Неправильный номер письма." );
        }
    }
    else {
        ch->pecho( "Прочесть какой номер?" );
    }
}

void NoteCommand::doList( PCharacter *ch, DLString &argument ) const
{
    ostringstream buf;
    NoteThread::NoteList mynotes;
    NoteThread::NoteList::const_iterator i;
    time_t stamp = thread->getStamp( ch );
    
    for (i = thread->xnotes.begin( ); i != thread->xnotes.end( ); i++)
        if ((*i)->isNoteTo( ch ) || (*i)->isNoteFrom( ch ))
            mynotes.push_back( *i );

    try {
        int cnt, last = mynotes.size( ) - 1;
        
        for (cnt = 0, i = mynotes.begin( ); i != mynotes.end( ); i++, cnt++) {
            bool hidden = thread->isNoteHidden( *i, ch, stamp );

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
        ch->pecho("Неверный формат, смотри {y{hcсправка письмо список{x." );
        return;
    }
    
    if (buf.str( ).empty( ))
        ch->pecho( "Не найдено ни одного сообщения." );
    else
        page_to_char( buf.str( ).c_str( ), ch );
}

void NoteCommand::doRemove( PCharacter *ch, DLString &arguments )
{
    const Note *note;
    DLString arg = arguments.getOneArgument( );
    
    if (!arg.isNumber( )) {
        ch->pecho( "Удалить какой номер?" );
        return;
    }

    try {
        note = thread->getNoteAtPosition( ch, arg.toInt( ) );

        if (note) {
            if (ch->get_trust( ) < CREATOR && note->getAuthor( ) != ch->getName( ))
                ch->pecho( "Ты не можешь удалить это сообщение, т.к. не являешься его автором." );
            else {
                LogStream::sendNotice( ) 
                    << getName( ) << " remove: " 
                    << ch->getName( ) << ", id " << note->getID( ) << endl;
                thread->remove( note );
                ch->pecho( "Ok." );
            }
        }
        else
            ch->pecho( "Так много %N2 еще не написали.", thread->getRussianMltName().c_str( ) );
    } 
    catch (const ExceptionBadType& e) {
        ch->pecho( "Неправильный номер письма." );
    }
}

void NoteCommand::doUncatchup( PCharacter *ch, DLString &arguments ) const
{
    XMLAttributeLastRead::Pointer attr;
    DLString arg = arguments.getOneArgument( );
    
    attr = ch->getAttributes( ).getAttr<XMLAttributeLastRead>( "lastread" );

    if (!arg.isNumber( )) {
        attr->setStamp( *thread, 0 );
        ch->pecho( "Все %N1 помечены как непрочитанные.", thread->getRussianMltName().c_str( ) );
        return;
    }

    try {
        int vnum = arg.toInt( );
        const Note *note = thread->getNoteAtPosition( ch, vnum );
        
        if (note) {
            attr->setStamp( *thread, note->getID( ) );
            ch->pecho( "Все %N2, начиная с номера %d, помечены как непрочитанные.", 
                        thread->getRussianMltName().c_str( ), vnum );
        }
        else
            ch->pecho( "Так много %N2 еще не написали.", thread->getRussianMltName().c_str( ) );
    }
    catch (const ExceptionBadType& e) {
        ch->pecho( "Неправильный номер письма." );
    }
}

bool NoteCommand::doPost( PCharacter *ch, XMLAttributeNoteData::Pointer attr )
{
    XMLNoteData *notedata;

    if (!( notedata = attr->findNote( *thread ) ))
        return false;

    if (notedata->getRecipient( ).empty( )) {
        echo( ch, thread->msgNoRecepient, "Укажите адресата письма." );
        return false;
    }

    Note note;
    notedata->commit( &note );
    note.godsSeeAlways = thread->godsSeeAlways;
    thread->attach( &note );

    echo( ch, thread->msgSent, "Письмо отправлено." );
    LogStream::sendNotice( ) 
        << getName( ) << " post: " 
        << ch->getName( ) << ", id " << note.getID( ) << endl;

    NoteHooks::processNoteMessage( **thread, note );
    attr->clearNote( *thread );
    return true;
}

bool NoteCommand::doFrom( PCharacter *ch, XMLAttributeNoteData::Pointer attr, const DLString &arguments ) const
{
    if (ch->get_trust( ) >= GOD) {
        XMLNoteData *note = attr->makeNote( ch, *thread );
        DLString arg = arguments;
        
        arg = arg.getOneArgument( );
        arg.colourstrip( );

        if (PCharacterManager::find( arg )) {
            ch->pecho( "Подделка документов запрещена!" );
        }
        else {
            note->setFrom( arguments );
            ch->pecho( "Ok." );
        }

        return true;
    }

    return false;
}

void NoteCommand::doForward( PCharacter *ch, XMLAttributeNoteData::Pointer attr, DLString &arguments ) const
{
    const Note *orig;
    XMLNoteData *note;
    ostringstream buf;
    DLString arg = arguments.getOneArgument( );
    
    if (!arg.isNumber( )) {
        ch->pecho( "Перенаправить какой номер?" );
        return;
    }

    try {
        orig = thread->getNoteAtPosition( ch, arg.toInt( ) );
    } 
    catch (const ExceptionBadType& e) {
        ch->pecho( "Неправильный номер письма." );
        return;
    }

    if (!orig) {
        ch->pecho( "Так много %N2 еще не написали.", thread->getRussianMltName().c_str( ) );
        return;
    }

    note = attr->makeNote( ch, *thread );
    orig->toForwardStream( buf );
    note->addLine( buf.str( ) );
    ch->pecho( "Ok." );
}

void NoteCommand::echo( PCharacter *ch, const DLString &msg, const DLString &defaultMsg, const DLString &arg ) const
{
    ostringstream buf;
    buf << (msg.empty( ) ? defaultMsg : msg) << arg << endl;
    ch->send_to( buf );
}

