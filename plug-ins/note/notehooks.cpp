/* $Id$
 *
 * ruffina, 2018
 */
#include "notehooks.h"
#include "notemanager.h"
#include "noteflags.h"
#include "webnote.h"
#include "dlfileop.h"
#include "dlfilestream.h"
#include "dldirectory.h"
#include "object.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "infonet.h"
#include "messengers.h"
#include "descriptor.h"
#include "arg_utils.h"
#include "dl_strings.h"
#include "act.h"
#include "def.h"

void NoteHooks::processNoteMessage( const NoteThread &thread, const Note &note )
{
    // Notify via crystal orb for all types of messages.
    notifyOrb(thread, note);

    if (dreamland->hasOption(DL_BUILDPLOT))
        return;

    if (!note.isNoteToAll( ))
        return;
  
    // Notify Discord and Telegram about all types of public messages.
    hookTelegram(thread, note);
    hookDiscord(thread, note);

    // Publish news and changes to the website.
    if (thread.getName() == "news" || thread.getName() == "change")
    {
        webDumpNews();
    }

    // Publish new stories to the website under 'Modern Stories'.
    if (thread.getName() == "story")
    {
        webDumpModernStories();
    }
}

void NoteHooks::notifyOrb( const NoteThread &thread, const Note &note )
{
    Descriptor *d;
    NoteManager::Threads::const_iterator i;
    NoteManager::Threads &threads = NoteManager::getThis( )->getThreads( );

    for (d = descriptor_list; d; d = d->next) {
        Object *pager;
        std::basic_ostringstream<char> buf0;
        PCharacter *victim;
        bool fFirst = true;
        
        if (d->connected != CON_PLAYING || !d->character)
            continue;

        victim = d->character->getPC( );
        
        if (victim->getName() == note.getAuthor())
            continue;
        
        if (!thread.canRead( victim ))
            continue;

        if (!note.isNoteTo( victim ))
            continue;
         
        if (!( pager = get_pager( victim ) ))
            continue;

        buf0 << "{CТихий голос из $o2: {WУ тебя:";
        
        for (i = threads.begin( ); i != threads.end( ); i++) {
            const char *c1, *c2, *c5;
            const NoteThread &th = **i->second;
            int count = th.countSpool( victim );
            int gender = th.getGender().getValue();
            
            if (count > 0) {
                c1 = (gender == SEX_FEMALE ? "ая" : gender == SEX_MALE ? "ый" : "ое");
                c2 = (gender == SEX_FEMALE ? "ые" : "ых");
                c5 = "ых";
                
                if (fFirst) 
                    fFirst = false;
                else
                    buf0 << "                                       ";

                buf0 << " {Y" << count << "{W непрочитанн"
                     << GET_COUNT(count, c1, c2, c5) << " " 
                     << GET_COUNT(count, 
                                    russian_case( th.getRussianThreadName(), '1' ),
                                    russian_case( th.getRussianThreadName(), '2' ),
                                    russian_case( th.getRussianMltName(), '2' ) )
                     << " (" << th.getName() << ").{x" << endl;
            }
        }
        
        act_p( buf0.str( ).c_str( ), victim, pager, 0, TO_CHAR, POS_DEAD );
    }
}

void NoteHooks::hookTelegram(const NoteThread &thread, const Note &note)
{
    ostringstream content;

    content 
        << "*Автор*: " << note.getFrom() << endl
        << "*Тема*: " << note.getSubject() << endl
        << endl
        << note.getText();
        
    send_telegram(content.str());
}

void NoteHooks::hookDiscord(const NoteThread &thread, const Note &note)
{
    if (thread.getName() == "news" || thread.getName() == "change")
        send_discord_news(thread.getRussianName().ruscase('1'), note.getFrom(), note.getSubject(), note.getText());
    else
        send_discord_note(thread.getRussianName().ruscase('1'), note.getFrom(), note.getSubject(), note.getText());
      
    send_discord_note_notify(thread.getRussianName().ruscase('1'), note.getFrom(), note.getSubject());
}

// Timestamp/ID of the first story written after 2018 DL rebirth.
static const long long STORY_MODERN_START = 1529205799;
// Where to dump various files.
static const DLString NEWS_EXPORT_PATH = "/tmp/news.xml";
static const DLString STORY_OLD_EXPORT_PATH = "/tmp/story1.xml";
static const DLString STORY_MODERN_EXPORT_PATH = "/tmp/story2.xml";

// Dump all news&changes readable by everyone.
static bool news_include(const Note *note)
{
    return note->isNoteToAll();
}

// Dump old stories explicitly marked with "publish" flag (via 'story flags' command).
static bool story_include_old(const Note *note)
{
    return note->getFlags().isSet(NOTE_PUBLISH)
            && note->getID() < STORY_MODERN_START;
}

// Dump all modern stories.  In the future may need to mark them too, to exclude
// some from being published. So far most of them are good.
static bool story_include_modern(const Note *note)
{
    return note->isNoteToAll() 
            && note->getID() > STORY_MODERN_START;
}

void NoteHooks::webDumpNews()
{
    WebNoteList webNotes;
    bool fColor = false;

    webNotes.importThread("news", news_include, fColor);
    webNotes.importThread("change", news_include, fColor);
    webNotes.saveTo(NEWS_EXPORT_PATH);
}

void NoteHooks::webDumpModernStories()
{
    WebNoteList webNotes;
    bool fColor = true;

    webNotes.importThread("story", story_include_modern, fColor);
    webNotes.saveTo(STORY_MODERN_EXPORT_PATH);
}

void NoteHooks::webDumpOldStories()
{
    WebNoteList webNotes;
    bool fColor = true;

    webNotes.importThread("story", story_include_old, fColor);
    webNotes.saveTo(STORY_OLD_EXPORT_PATH);
}

/**
 * Coder command webdump, for debugging.
 * webdump news  - dumps all public news and changes into a file.
 * webdump story - dumps old and modern stories into two separate files.
 */
COMMAND(NoteHooks, "webdump")
{
    DLString args = constArguments;

    if (!ch->isCoder()) {
        ch->println("Что?");
        return;
    }

    DLString name = args.getOneArgument();
    if (name.empty()) {
        ch->println("Какой именно вид сообщений нужно сохранить на диск?");
        return;
    }

    if (arg_oneof(name, "news", "changes")) {
        webDumpNews();
        ch->printf("Все новости и изменения сохранены в %s.\n", NEWS_EXPORT_PATH.c_str());
        return;
    }

    if (arg_oneof(name, "story")) {
        webDumpOldStories();
        ch->printf("Старые истории сохранены в %s.\n", STORY_OLD_EXPORT_PATH.c_str());

        webDumpModernStories();
        ch->printf("Современные истории сохранены в %s.\n", STORY_MODERN_EXPORT_PATH.c_str());
        return;
    }

    ch->println( "Команда 'webdump' не поддерживается для этого вида сообщений." );
}
