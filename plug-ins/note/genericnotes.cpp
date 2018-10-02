/* $Id: genericnotes.cpp,v 1.1.2.8.6.2 2008/04/10 00:06:02 rufina Exp $
 *
 * ruffina, 2005
 */
#include "notethread.h"
#include "note.h"
#include "dreams.h"

#include "xmlattributeplugin.h"
#include "pcharacter.h"
#include "clanreference.h"
#include "act.h"
#include "def.h"

CLAN(ruler);

NoteThreadRegistrator * NoteThreadRegistrator::first = 0;

NOTE_DECL(note);
VOID_NOTE(note)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: У тебя {W8{x непрочитанных писем ('{hc{y{lRписьмо{lEnote{x').
    buf << fmt( 0, "У тебя {W%1$d{x непрочитан%1$Iное|ных|ных пис%1$Iьмо|ьма|ем ('{hc{y{lRписьмо{lEnote{x').", count ) << endl;
}

NOTE_DECL(news);
VOID_NOTE(news)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: {W5{x новостей ожидают тебя ('{hc{y{lRновость{lEnews{x').
    buf << fmt( 0, "{W%1$d{x новост%1$Iь|и|ей ожида%1$Iет|ют|ют тебя ('{hc{y{lRновость{lEnews{x').", count ) << endl;
}

NOTE_DECL(change);
VOID_NOTE(change)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: За последнее время произошли {W2{x изменения ('{hc{y{lRизменение{lEchange{x').
    buf << fmt( 0, "За последнее время произошл%1$Iо|и|и {W%1$d{x изменен%1$Iие|ия|ий ('{hc{y{lRизменение{lEchange{x').", count ) << endl;
}

NOTE_DECL(story);
VOID_NOTE(story)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: {W10{x новых историй ожидают прочтения ('{hc{y{lRистория{lEstory{x').
    buf << fmt( 0, "{W%1$d{x нов%1$Iая|ые|ых истор%1$Iия|ии|ий ожида%1$Iет|ют|ют прочтения ('{hc{y{lRистория{lEstory{x').", count ) << endl;
}

NOTE_DECL(idea);
VOID_NOTE(idea)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: У тебя {W1{x непрочитанная идея ('{hc{y{lRидея{lEidea{x').
    buf << fmt( 0, "У тебя {W%1$d{x непрочитанн%1$Iая|ых|ых иде%1$Iя|и|й ('{hc{y{lRидея{lEidea{x').", count ) << endl;
}

NOTE_DECL(penalty);
VOID_NOTE(penalty)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: Опубликованы {W8{x сообщений о Каре Небесной ('{hc{y{lRнаказание{lEpenalty{x').
    buf << fmt( 0, "Опубликован%1$Iо|ы|ы {W%1$d{x сообщен%1$Iие|ия|ий о Каре Небесной ('{hc{y{lRнаказание{lEpenalty{x').", count ) << endl;
}


NOTE_DECL(crime);
VOID_NOTE(crime)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: Тебя ожидает {W1{x сообщение о преступлениях ('{hc{y{lRпреступление{lEcrime{x').
    buf << fmt( 0, "Тебя ожида%1$Iет|ют|ют {W%1$d{x сообщен%1$Iие|ия|ий о преступлени%1$Iи|ях|ях ('{hc{y{lRпреступление{lEcrime{x').", count ) << endl;
}
TYPE_NOTE(bool, crime)::canWrite( const PCharacter *ch ) const 
{
    if (!NoteThread::canWrite( ch ))
	return false;
	
    return (const_cast<PCharacter *>(ch))->getClan( ) == clan_ruler;
}

NOTE_DECL(qnote);
VOID_NOTE(qnote)::getUnreadMessage( int count, ostringstream &buf ) const 
{
    // Example: Тебя ожидает {W2{x сообщения о глобальных квестах ('{hc{y{lRквестнота{lEqnote{x').
    buf << fmt( 0, "Тебя ожида%1$Iет|ют|ют {W%1$d{x сообщен%1$Iие|ия|ий о глобальн%1$Iом|ых|ых квест%1$Iе|ах|ах ('{hc{y{lRквестнота{lEqnote{x').", count ) << endl;
}

extern "C"
{
    SO::PluginList initialize_generic_notes( ) 
    {
	SO::PluginList ppl;

	NoteThreadRegistrator::registrateAll( ppl );

	Plugin::registerPlugin<DreamThread>( ppl );
	Plugin::registerPlugin<DreamManager>( ppl );
	Plugin::registerPlugin<XMLAttributeRegistrator<XMLAttributeDream> >( ppl );

	return ppl;
    }
}
