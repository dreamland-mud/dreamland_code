/* $Id$
 *
 * ruffina, 2004
 */
#include <sstream>
#include <string.h>
#include "mudtags.h"
#include "colour.h"
#include "dl_ctype.h"
#include "door_utils.h"
#include "logstream.h"
#include "descriptor.h"
#include "websocketrpc.h"
#include "screenreader.h"
#include "pcharacter.h"
#include "race.h"
#include "merc.h"
#include "def.h"

/*------------------------------------------------------------------------------------
 * ColorTags
 *------------------------------------------------------------------------------------*/
struct ColorTags {
    ColorTags( const char *, Character * = NULL );

    void run( ostringstream & );
    void setRaw( bool );
    void setColor( bool );
    void setWeb( bool );

protected:    
    void reset( );
    bool raw;

    void parse_color_ansi( ostringstream & );
    void parse_color_web( ostringstream & );
    const char *color_tag_ansi( );
    bool my_color;
    bool my_ansi;
    list<char> color_stack; 
    char color_last;
    DLString desired_class;
    DLString actual_class;

    Character *ch;
    const char *text;
    const char *p;
};

ColorTags::ColorTags( const char *text, Character *ch )
{
    PlayerConfig cfg = ch ? ch->getConfig( ) : PlayerConfig( );
    this->text = text;
    this->ch = ch;

    // Is colour enabled for this player?
    my_color = ch ? cfg.color : true;
    // Are we using ANSI escape sequences or HTML tags?
    my_ansi = !is_websock(ch);
    raw = false;

    reset( );
}

void ColorTags::setWeb( bool web )
{
    my_ansi = !web;
}

void ColorTags::setColor( bool color )
{
    my_color = color;
}

// raw mode: output non-color tags asis
void ColorTags::setRaw( bool raw )
{
    this->raw = raw;
}

void ColorTags::reset( )
{
    color_last = 0;
}

void ColorTags::run( ostringstream &out )
{
    reset( );
    if (my_ansi)
        parse_color_ansi( out );
    else
        parse_color_web( out );
}

static const char valid_colors[17] = { 'x', 'X', 'b', 'c', 'g', 'm', 'r', 'w', 'y', 'B', 'C', 'G', 'M', 'R', 'W', 'Y', 'D' };
static bool is_valid_color( char c )
{
    for (int i = 0; i < 17; i++)
        if (valid_colors[i] == c)
            return true;
    return false;
}


void ColorTags::parse_color_web( ostringstream &out )
{        
    desired_class = "fgdw";
    actual_class = "";
    char t;

    for (p = text; *p; ++p) {
        // Text output with current colours.
        if (*p != '{') {
            // Start of special tag (e.g. "<m") is encountered, close current color.
            if (*p == 036) {
                if (actual_class != "") 
                    if (my_color) out << "</c>";

            // End of special tag (i.e. "..>") is encountered, reopen current color.
            } else if (*p == 037) {
                if (my_color) out << "<c c='" << desired_class << "'>";
                actual_class = desired_class;
            // Output normal character.
            } else {
                out << *p;
            }
            continue;
        }

        switch (*++p) {
        // special characters
        case '*':
            break;
        case '+':
            break;
        case '/':        
            out << endl;             
            break;
        case '{':        
            out << *p;
            break;

        // total reset
        case 'X':
        case 'x':
            reset( );
            /* FALLTHROUGH */;

        // colors
        default:
            t = *p;

            // Handle {1 (remember color) and {2 (restore color) tags.
            switch (t) {
            case CLR_PUSH: case '1':
                if (color_last) 
                    color_stack.push_back( color_last );
                else
                    color_stack.push_back('x');
                break;

            case CLR_POP: case '2':
                if (color_stack.empty( )) {
                    t = 'x';
                } else {
                    t = color_stack.back( );
                    color_stack.pop_back( );
                }
                /* FALLTROUGH */
            }

            if (is_valid_color( t )) {
                DLString clr;
                clr += t;

                // Remember current color for push/pop functionality.
                color_last = t;

                // Handle bright and dark colour variants.
                if (dl_isupper( t )) 
                    desired_class = "fgb" + clr.toLower( );
                else
                    desired_class = "fgd" + clr;

                // Output current colour tag as soon as we encounter it,
                // because next input symbol might as well try to close it,
                // resulting in extra closed tags.
                if (desired_class != actual_class) {
                    if (actual_class != "")
                        if (my_color) out << "</c>";
                    if (my_color) out << "<c c='" << desired_class << "'>";
                    actual_class = desired_class;
                }
            }
            // Invalid colour symbols are ignored.
            break;
        }
    }

    if (actual_class != "")
        if (my_color) out << "</c>";
}

void ColorTags::parse_color_ansi( ostringstream &out )
{        
    for (p = text; *p; ++p) {
        if (*p != '{') {
            out << *p;
            continue;
        }

        switch (*++p) {
        // special characters
        case '*':
            if (!raw)
                out << "\007";             
            break;
        case '+':
            if (!raw) 
                out << C_BLINK;             
            break;
        case '/':        
            out << endl;             
            break;
        case '{':        
            out << *p;
            break;

        // total reset
        case 'X':
        case 'x':
            reset( );
            if (!raw)
                out << CLEAR << C_WHITE;
            break;

        // colors
        default: 
            if (my_color)
                out << color_tag_ansi( );
            break;
        }
    }
}





// color close: x (see above)        
// colors: bcgmrwyBCGMRWYD
// special: +*/ (see above)
// escape: {        (see above)
// stack push/pop: 1 2 
const char * ColorTags::color_tag_ansi( )
{
    char t = *p;

    switch (t) {
    case CLR_PUSH: case '1':
        if (color_last) 
            color_stack.push_back( color_last );
        else
            color_stack.push_back('x');
        return "";

    case CLR_POP: case '2':
        if (color_stack.empty( )) {
            t = 'x';
        } else {
            t = color_stack.back( );
            color_stack.pop_back( );
        }
        /* FALLTROUGH */

    default:
        color_last = t;
        break;
    }

    switch (t) {
    default:
    case 'X':
    case 'x':        return CLEAR C_WHITE;
    case 'b':        return C_BLUE;             
    case 'c':        return C_CYAN;             
    case 'g':        return C_GREEN;             
    case 'm':        return C_MAGENTA;    
    case 'r':        return C_RED;             
    case 'w':        return C_WHITE;             
    case 'y':        return C_YELLOW;     
    case 'B':        return C_B_BLUE;     
    case 'C':        return C_B_CYAN;     
    case 'G':        return C_B_GREEN;    
    case 'M':        return C_B_MAGENTA;  
    case 'R':        return C_B_RED;             
    case 'W':        return C_B_WHITE;    
    case 'Y':        return C_B_YELLOW;   
    case 'D':        return C_D_GREY;     
    }
}

/*------------------------------------------------------------------------------------
 * VisibilityTags
 *------------------------------------------------------------------------------------*/
struct VisibilityTags {
    VisibilityTags( const char *, Character * = NULL );
    
    void run( ostringstream & );
    void setWeb( bool );

protected:    
    void reset( );

    bool my_web;

    enum {
        LANG_NONE,
        LANG_ENGLISH,
        LANG_RUSSIAN,
    };
    void clang_tag_parse( );
    bool clang_tag_work( );
    int st_clang, my_clang;
    
    void nlang_tag_parse( );
    bool nlang_tag_work( );
    int st_nlang, my_nlang;

    void slang_tag_parse( );
    bool slang_tag_work( );
    int st_slang, my_slang;

    void sex_tag_parse( );
    bool sex_tag_work( );
    int st_sex, my_sex;

    enum {
        SUN_NONE = -1,
    };
    void time_tag_parse( );
    bool time_tag_work( );
    int st_time, my_time;

    void align_tag_parse( );
    bool align_tag_work( );
    int st_align, my_align;
    bool invert_align;

    enum {
        INVIS_NONE = 0,
        INVIS_IMMORTAL = (A),
        INVIS_WEB = (E),
        INVIS_TELNET = (F),
        INVIS_SCREENREADER_ON = (G),
        INVIS_SCREENREADER_OFF = (H)
    };
    void invis_tag_parse( ostringstream & );
    bool invis_tag_work( );
    long long st_invis, my_invis;

    enum {
        SKY_NONE = -1,
    };
    enum {
        SEASON_NONE = -1,
        SEASON_WINTER = 0,
        SEASON_SPRING = 1,
        SEASON_SUMMER = 2,
        SEASON_AUTUMN = 3,
    };
    void sky_season_tag_parse( ostringstream & );
    bool sky_season_tag_work( );
    int st_sky, my_sky;
    bool invert_sky;
    int st_season, my_season;
    bool invert_season;

    void hyper_tag_start( ostringstream & );
    void hyper_tag_end( ostringstream & );
    bool hyper_tag_work();
    const char *my_hyper_tag;
    int my_elang;

    void html_escape( ostringstream &buf );
    bool need_escape( );
    
    Character *ch;
    const char *text;
    const char *p;
    char c;
};

VisibilityTags::VisibilityTags( const char *text, Character *ch )
{
    PlayerConfig cfg = ch ? ch->getConfig( ) : PlayerConfig( );
    this->text = text;
    this->ch = ch;

    my_clang = (cfg.rucommands) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_slang = (cfg.ruskills) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_nlang = (cfg.runames) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_elang = (cfg.ruexits) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_sex = ch ? ch->getSex( ) : SEX_MALE;

    my_time = (weather_info.sunlight == SUN_DARK || weather_info.sunlight == SUN_SET) ? SUN_DARK : SUN_LIGHT;

    my_align = ch ? ALIGNMENT(ch) : N_ALIGN_NEUTRAL;

    my_invis = INVIS_NONE;

    my_web = is_websock(ch);

    my_sky = weather_info.sky;
    if (time_info.month < 4) {
        my_season = SEASON_WINTER;
    } else if (time_info.month < 8) {
        my_season = SEASON_SPRING;
    } else if (time_info.month < 12) {
        my_season = SEASON_SUMMER;
    } else if (time_info.month < 16) {
        my_season = SEASON_AUTUMN;
    } else {
        my_season = SEASON_WINTER;
    }

    if (ch) {
        if (ch->is_immortal( ))
            SET_BIT(my_invis, INVIS_IMMORTAL);
        if (my_web)
            SET_BIT(my_invis, INVIS_WEB);
        else
            SET_BIT(my_invis, INVIS_TELNET);
        
        if (uses_screenreader(ch))
            SET_BIT(my_invis, INVIS_SCREENREADER_ON);
        else
            SET_BIT(my_invis, INVIS_SCREENREADER_OFF);
    }
   
    reset( );
}

void VisibilityTags::setWeb(bool web) 
{
    my_web = web;
    if (my_web) {
        SET_BIT(my_invis, INVIS_WEB);
        REMOVE_BIT(my_invis, INVIS_TELNET);
    } else {
        REMOVE_BIT(my_invis, INVIS_WEB);
        SET_BIT(my_invis, INVIS_TELNET);
    }
}

void VisibilityTags::reset( )
{
    st_clang = LANG_NONE;
    st_nlang = LANG_NONE;
    st_slang = LANG_NONE;
    st_sex = SEX_EITHER;
    st_time = SUN_NONE;
    st_align = N_ALIGN_NULL;
    invert_align = false;
    st_invis = INVIS_NONE;
    my_hyper_tag = 0;
    st_sky = SKY_NONE;
    invert_sky = false;
    st_season = SEASON_NONE;
    invert_season = false;
}


// Return true if a character pointed to by p is followed by 
// all characters from msg.
static bool is_followed_by(const char *p, const char *msg) {
    if (!p || !msg || !*p)
        return false;

    size_t msg_len = strlen(msg);
    for (size_t m = 0; m < msg_len; m++) {
        if (*(p + m + 1) != msg[m])
            return false;
    }

    return true;
}

// Return true if a character in 'text' (pointed to by p) is preceded by
// all character from msg. 
static bool is_preceded_by(const char *p, const char *text, const char *msg)
{
    size_t text_position = p - text;
    size_t msg_len = strlen(msg);
    if (msg_len >= text_position)
        return false;

    for (size_t m = 0; m < msg_len; m++) {
        if (*(p - msg_len + m) != msg[m])
            return false;
    }

    return true;
}

// See if some characters need to be replaced with HTML entities.
bool VisibilityTags::need_escape( )
{
    // Don't escape for telnet clients.
    if (!my_web) {
        return false;
    }

    // Escape strings outside of "{Iw" tags for web clients.
    if (!IS_SET(st_invis, INVIS_WEB)) {
        return true;
    }

    // Don't escape custom tags inside '{Iw' tags.
    // Examples: {Iw<m ... >{Ix or {Iw</m>{Ix
    // This awaits better implementation of client-server proto for mudjs.

    // Only allow < if it's preceded by {Iw and followed by m|r space or /m|r>.
    if (c == '<') {
        if (!is_preceded_by(p, text, "{Iw")) 
            return true;        
        if (is_followed_by(p, "m ") || is_followed_by(p, "r "))
            return false;
        if (is_followed_by(p, "/m>") || is_followed_by(p, "/r>"))
            return false;
        return true;
    }

    // Only allow > if it's followed by {Ix, {IW.
    if (c == '>') {
        if (is_followed_by(p, "{Ix") || is_followed_by(p, "{IW"))
            return false;

        return true;
    }

    return true;
}

// Replace certain chars with HTML entities.
void VisibilityTags::html_escape( ostringstream &buf )
{
    if (need_escape( )) {
        switch (c) {
            case '<':
                buf << "&lt;";
                break;
            case '>':
                buf << "&gt;";
                break;
            default:
                buf << c;
        }
    }
    else
        buf << c;
}

void VisibilityTags::run( ostringstream &out )
{
    reset( );

    for (p = text; *p; ++p) {
        if (*p != '{') {
            c = *p;

            if (!clang_tag_work( )
                    || !nlang_tag_work( )
                    || !slang_tag_work( )
                    || !align_tag_work( )
                    || !sex_tag_work( )
                    || !time_tag_work( )
                    || !hyper_tag_work( )
                    || !invis_tag_work( )
                    || !sky_season_tag_work( ))
            continue;
           
            html_escape( out );
            continue;
        }
        
        switch (*++p) {
        // composite two-letter tags
        case 'l':
            clang_tag_parse( );
            break;
        case 'n':
            nlang_tag_parse( );
            break;
        case 's':
            slang_tag_parse( );
            break;
        case 'A':
            align_tag_parse( );
            break;
        case 'S':
            sex_tag_parse( );
            break;
        case 'T':
            time_tag_parse( );
            break;
        case 'I':
            invis_tag_parse( out );
            break;
        case 'F':
            sky_season_tag_parse( out );
            break;
        case 'h':
            hyper_tag_start( out );
            break;

        // Total reset, but leave {x there for color parser to pick up.
        // Ideally each tag should reset individually, but historically many
        // texts rely on this 'total reset' sequence.
        case 'X':
        case 'x':
            hyper_tag_end( out );
            reset( );
            out << "{" << *p;
            break;

        default: 
            out << "{" << *p;
            break;
        }
    }
}

// Read chars from 'p' while they are numbers. Rollback if number is followed by "."
// (help 2.create vs help 2).
static DLString collect_number(const char *&p) {
    const char *p_backup = p;
    DLString number;

    while (isdigit(*++p)) {
        number.append(*p);
    }

    if (*p == '.') {
        p = p_backup;
        return DLString::emptyString;
    }

    --p;
    return number;
}


// {h
// close hyper link: x
// supported hyper link types: c (<hc>command</hc>), l (<hl>hyper link</hl>), h (<hh>help article</hh>
// or <hh id='234'>article</hh>), g (<hg>skill group names</hg>), s (<hs>speedwalk</hs>)
void VisibilityTags::hyper_tag_start( ostringstream &out )
{
    DLString id;

    switch (*++p) {
    case 'c': 
        my_hyper_tag = "hc";
        break;

    case 'l': 
        my_hyper_tag = "hl";
        break;

    case 'h': 
        my_hyper_tag = "hh";
        id = collect_number(p);
        break;

    case 'g': 
        my_hyper_tag = "hg";
        break;

    case 's':
        my_hyper_tag = "hs";
        break;

    default:
    case 'x':
    case 'X': 
        hyper_tag_end( out );
        return;
    }

    if (IS_SET(my_invis, INVIS_WEB)) {
        out << "\036" << "<" << my_hyper_tag;
        if (!id.empty())
            out << " id='" << id << "'";
        out << ">" << "\037";
    }
}    

void VisibilityTags::hyper_tag_end( ostringstream &out )
{
    if (my_hyper_tag) {
        if (IS_SET(my_invis, INVIS_WEB)) 
            out << "\036" << "</" << my_hyper_tag << ">" << "\037";
        
        my_hyper_tag = 0;
    }
}

/** 
 * Additional behavior for some of the hyper-tags:
 * - {hs replaces exit names inside speedwalk based on player's ruexits config.
 *    Works for both web and telnet.
 */
bool VisibilityTags::hyper_tag_work()
{
    // Inside <hs> tag.
    if (my_hyper_tag && my_hyper_tag[1] == 's') {
        if (my_elang == LANG_RUSSIAN)
            c = door_translate_en_ru(c);
    }

    return true;
}


// {l
// close config lang: x
// config lang: e r
void VisibilityTags::clang_tag_parse( )
{
    switch (*++p) {
    case 'e':
    case 'E': st_clang = LANG_ENGLISH; break;
    case 'r':
    case 'R': st_clang = LANG_RUSSIAN; break;
    default:  --p; /* FALLTHROUGH */
    case 'X':
    case 'x': st_clang = LANG_NONE; break;
    }
}

bool VisibilityTags::clang_tag_work( )
{
    return st_clang == LANG_NONE || st_clang == my_clang;
}

// {n
// close config lang: x
// config lang: e r
void VisibilityTags::nlang_tag_parse( )
{
    switch (*++p) {
    case 'e':
    case 'E': st_nlang = LANG_ENGLISH; break;
    case 'r':
    case 'R': st_nlang = LANG_RUSSIAN; break;
    default:  --p; /* FALLTHROUGH */
    case 'X':
    case 'x': st_nlang = LANG_NONE; break;
    }
}

bool VisibilityTags::nlang_tag_work( )
{
    return st_nlang == LANG_NONE || st_nlang == my_nlang;
}

// {s
// close skills lang: x
// skills lang: e r
void VisibilityTags::slang_tag_parse( )
{
    switch (*++p) {
    case 'e':
    case 'E': st_slang = LANG_ENGLISH; break;
    case 'r':
    case 'R': st_slang = LANG_RUSSIAN; break;
    default:  --p; /* FALLTHROUGH */
    case 'X':
    case 'x': st_slang = LANG_NONE; break;
    }
}

bool VisibilityTags::slang_tag_work( )
{
    return st_slang == LANG_NONE || st_slang == my_slang;
}

// {A
// close align: x
// align: n(eutral) g(ood) e(vil) N(non-neutral) G(non-good) E(non-evil)
bool VisibilityTags::align_tag_work( ) 
{
    return st_align == N_ALIGN_NULL || invert_align != (st_align == my_align);
}

void VisibilityTags::align_tag_parse( )
{
    switch (*++p) {
    case 'n': st_align = N_ALIGN_NEUTRAL;   invert_align = false; break;
    case 'g': st_align = N_ALIGN_GOOD;            invert_align = false; break;
    case 'e': st_align = N_ALIGN_EVIL;            invert_align = false; break;
    case 'N': st_align = N_ALIGN_NEUTRAL;   invert_align = true; break;
    case 'G': st_align = N_ALIGN_GOOD;            invert_align = true; break;
    case 'E': st_align = N_ALIGN_EVIL;            invert_align = true; break;
    default:  --p; /* FALLTHROUGH */
    case 'x':
    case 'X': st_align = N_ALIGN_NULL;            invert_align = false; break;
    }
}

// {S
// close sex: x
// sexes: m(ale) f(emale) n(eutral)
bool VisibilityTags::sex_tag_work( )
{
    return st_sex == SEX_EITHER || st_sex == my_sex;
}

void VisibilityTags::sex_tag_parse( )
{
    switch (*++p) {
    case 'm': st_sex = SEX_MALE; break;
    case 'f': st_sex = SEX_FEMALE; break;
    case 'n': st_sex = SEX_NEUTRAL; break;
    default:  --p; /* FALLTHROUGH */ 
    case 'X':
    case 'x': st_sex = SEX_EITHER; break;
    }
}

// {T
// close day: x
// daytime: n(ight) d(ay)
bool VisibilityTags::time_tag_work( )
{
    return st_time == SUN_NONE || st_time == my_time;
}

void VisibilityTags::time_tag_parse( )
{
    switch (*++p) {
    case 'n': st_time = SUN_DARK; break;
    case 'd': st_time = SUN_LIGHT; break;
    default:  --p; /* FALLTHROUGH */ 
    case 'x':
    case 'X': st_time = SUN_NONE; break;
    }
}

// {I
// end invis: x
// invis type: i (imm-only), s/S (screenreader on/off), w/W (web client on/off)
bool VisibilityTags::invis_tag_work( )
{
    return st_invis == INVIS_NONE || IS_SET(my_invis, st_invis);
}

void VisibilityTags::invis_tag_parse( ostringstream &out )
{
    switch (*++p) {
    case 'i': st_invis = INVIS_IMMORTAL; break;
    case 's': st_invis = INVIS_SCREENREADER_ON; break;
    case 'S': st_invis = INVIS_SCREENREADER_OFF; break;
    case 'W': st_invis = INVIS_TELNET; break;
    case 'w': 
              st_invis = INVIS_WEB; 
              if (IS_SET(my_invis, INVIS_WEB)) 
                  out << "\036";
              break;
    default:  --p; /* FALLTHROUGH */ 
    case 'x':
    case 'X': 
              if (st_invis == INVIS_WEB && IS_SET(my_invis, INVIS_WEB)) 
                  out << "\037";
              st_invis = INVIS_NONE; 
              break;
    }
}

// {F
// end sky: x X
// sky type: t (thunderstorm), r (rain), c (cloudy), s (sunny)
//           T (not thunderstorm), R (not rain), C (not cloudy), S (not sunny)
// end season: y Y
// sky type: w (winter), p (spring), u (summer), a (autumn)
//           W (not winter), P (not spring), U (not summer), A (not autumn)
bool VisibilityTags::sky_season_tag_work( )
{
    return (
               st_sky == SKY_NONE ||
               (!invert_sky && st_sky == my_sky) ||
               (invert_sky && st_sky != my_sky)
           ) &&
           (
               st_season == SEASON_NONE ||
               (!invert_season && st_season == my_season) ||
               (invert_season && st_season != my_season)
           );
}

void VisibilityTags::sky_season_tag_parse( ostringstream &out )
{
    switch (*++p) {
    case 't': st_sky = SKY_LIGHTNING; invert_sky = false; break;
    case 'r': st_sky = SKY_RAINING; invert_sky = false; break;
    case 'c': st_sky = SKY_CLOUDY; invert_sky = false; break;
    case 's': st_sky = SKY_CLOUDLESS; invert_sky = false; break;
    case 'T': st_sky = SKY_LIGHTNING; invert_sky = true; break;
    case 'R': st_sky = SKY_RAINING; invert_sky = true; break;
    case 'C': st_sky = SKY_CLOUDY; invert_sky = true; break;
    case 'S': st_sky = SKY_CLOUDLESS; invert_sky = true; break;
    case 'w': st_season = SEASON_WINTER; invert_season = false; break;
    case 'p': st_season = SEASON_SPRING; invert_season = false; break;
    case 'u': st_season = SEASON_SUMMER; invert_season = false; break;
    case 'a': st_season = SEASON_AUTUMN; invert_season = false; break;
    case 'W': st_season = SEASON_WINTER; invert_season = true; break;
    case 'P': st_season = SEASON_SPRING; invert_season = true; break;
    case 'U': st_season = SEASON_SUMMER; invert_season = true; break;
    case 'A': st_season = SEASON_AUTUMN; invert_season = true; break;
    default:  --p; /* FALLTHROUGH */
    case 'x':
    case 'X': st_sky = SKY_NONE; invert_sky = false; break;
    case 'y':
    case 'Y': st_season = SEASON_NONE; invert_season = false; break;
    }
}

/*------------------------------------------------------------------------------------
 * utils 
 *------------------------------------------------------------------------------------*/
void mudtags_convert( const char *text, ostringstream &out, int flags, Character *ch )
{
    DLString result = text;

    // Convert tags like {I, {L if requested.
    if (IS_SET(flags, TAGS_CONVERT_VIS)) {
        ostringstream vbuf;
        VisibilityTags vtags(text, ch);

        if (IS_SET(flags, TAGS_ENFORCE_WEB)) 
            vtags.setWeb(true);
        if (IS_SET(flags, TAGS_ENFORCE_NOWEB)) 
            vtags.setWeb(false);

        vtags.run(vbuf);
        result = vbuf.str();
    }

    // Convert colors, either stripping them (NOCOLOR) or replacing with ANSI or HTML tags.
    if (IS_SET(flags, TAGS_CONVERT_COLOR)) {
        ColorTags ctags(result.c_str(), ch);

        if (IS_SET(flags, TAGS_ENFORCE_WEB)) 
            ctags.setWeb(true);
        if (IS_SET(flags, TAGS_ENFORCE_NOWEB)) 
            ctags.setWeb(false);
        if (IS_SET(flags, TAGS_ENFORCE_NOCOLOR)) 
            ctags.setColor(false);
        if (IS_SET(flags, TAGS_ENFORCE_RAW))
            ctags.setRaw(true);

        ctags.run(out);

    } else {
        out << result;
    }
}
