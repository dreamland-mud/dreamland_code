#ifndef MESSENGERS_H
#define MESSENGERS_H

#include "dlstring.h"

class PCharacter;
class Character;

void send_discord(const DLString &content);
void send_to_discord_stream(const DLString &content);

void send_discord_note(const DLString &thread, const DLString &author, const DLString &title, const DLString &description);
void send_discord_news(const DLString &thread, const DLString &author, const DLString &title, const DLString &description);
void send_discord_level(PCharacter *ch);
void send_discord_orb(const DLString &msg);
void send_discord_clan(const DLString &msg);
void send_discord_note_notify(const DLString &thread, const DLString &from, const DLString &subj);
void send_discord_death(const DLString &msg);
void send_discord_gquest(const DLString &gqName, const DLString &msg);
void send_discord_bonus(const DLString &msg);
void send_discord_fenia(const DLString &header, const DLString &exception);
void send_discord_confirm(PCharacter *ch);
void send_discord_wiznet(const DLString &message);
void send_discord_ooc(Character *ch, const DLString &format, const DLString &msg);
void send_discord_ic(Character *ch, const DLString &format, const DLString &msg);

void send_telegram_note(const DLString &thread, const DLString &author, const DLString &title, const DLString &description);
void send_telegram_gquest(const DLString &gqName, const DLString &msg);
void send_telegram_level(PCharacter *ch);
void send_telegram(const DLString &content);
void send_telegram_no_escape(const DLString &content);

// Returns true if the mail was queued for the drainer, false if the write failed
// (so a caller can report "sent" honestly instead of assuming success).
bool send_email(const DLString &to, const DLString &subject, const DLString &body);

// Mail the gods' inbox (ADMIN_EMAIL) about something an immortal must act on
// (a pending referral, an unmatched payment, a confirm request). Plain text, no
// mudtags. Capped per hour so a hook that misfires cannot flood the inbox:
// past the cap the mail is dropped and logged. True if queued.
bool send_admin_email(const DLString &subject, const DLString &body);

// Something an immortal must act on: the subject to the private Discord scream
// channel (where the gods actually look) plus the full mail to ADMIN_EMAIL.
void send_admin_alert(const DLString &subject, const DLString &body);

#endif
