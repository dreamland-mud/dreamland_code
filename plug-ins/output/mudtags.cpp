/* $Id$
 *
 * ruffina, 2004
 */
#include "colour.h"
#include "dl_ctype.h"
#include "logstream.h"
#include "descriptor.h"
#include "pcharacter.h"
#include "race.h"
#include "merc.h"
#include "def.h"

LANG(common);   LANG(human);   LANG(dwarvish);
LANG(elvish);   LANG(gnomish); LANG(giant);
LANG(trollish); LANG(cat);

static bool is_websock( Character *ch )
{
    return ch && ch->desc && ch->desc->websock.state == WS_ESTABLISHED;
}

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
    PlayerConfig::Pointer cfg = ch ? ch->getConfig( ) : PlayerConfig::Pointer( );
    this->text = text;
    this->ch = ch;

    // Is colour enabled for this player?
    my_color = ch ? cfg->color : true;
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

static const char valid_colors[16] = { 'x', 'b', 'c', 'g', 'm', 'r', 'w', 'y', 'B', 'C', 'G', 'M', 'R', 'W', 'Y', 'D' };
static bool is_valid_color( char c )
{
    for (int i = 0; i < 16; i++)
        if (valid_colors[i] == c)
            return true;
    return false;
}


void ColorTags::parse_color_web( ostringstream &out )
{        
    desired_class = "fgdw";
    actual_class = "";

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
        case 'x':
            reset( );
            /* FALLTHROUGH */;

        // colors
        default:
            if (is_valid_color( *p )) {
                DLString clr;
                clr += *p;
                // Handle bright and dark colour variants.
                if (dl_isupper( *p )) 
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
        return "";

    case CLR_POP: case '2':
        if (color_stack.empty( )) 
            return "";

        t = color_stack.back( );
        color_stack.pop_back( );
        /* FALLTROUGH */

    default:
        color_last = t;
        break;
    }

    switch (t) {
    default:
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

    void rlang_tag_parse( );
    bool rlang_tag_work( );
    RaceLanguage *st_rlang;

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
        INVIS_CODER = (B),
        INVIS_RECRUITER = (C),
        INVIS_LEADER = (D),
        INVIS_WEB = (E)
    };
    void invis_tag_parse( ostringstream & );
    bool invis_tag_work( );
    long long st_invis, my_invis;

    void hyper_tag_start( ostringstream & );
    void hyper_tag_end( ostringstream & );
    const char *my_hyper_tag;

    void html_escape( ostringstream &buf );
    bool need_escape( );
    
    Character *ch;
    const char *text;
    const char *p;
    char c;
};

VisibilityTags::VisibilityTags( const char *text, Character *ch )
{
    PlayerConfig::Pointer cfg = ch ? ch->getConfig( ) : PlayerConfig::Pointer( );
    this->text = text;
    this->ch = ch;

    my_clang = (cfg && cfg->rucommands) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_slang = (cfg && cfg->ruskills) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_nlang = (cfg && cfg->runames) ? LANG_RUSSIAN : LANG_ENGLISH;

    my_sex = ch ? ch->getSex( ) : SEX_MALE;

    my_time = (weather_info.sunlight == SUN_DARK || weather_info.sunlight == SUN_SET) ? SUN_DARK : SUN_LIGHT;

    my_align = ch ? ALIGNMENT(ch) : N_ALIGN_NEUTRAL;

    my_invis = INVIS_NONE;

    my_web = is_websock(ch);

    if (ch) {
        if (ch->is_immortal( ))
            SET_BIT(my_invis, INVIS_IMMORTAL);
        if (ch->isCoder( ))
            SET_BIT(my_invis, INVIS_CODER);
        if (!ch->is_npc( ) && ch->getClan( )->isRecruiter( ch->getPC( ) ))
            SET_BIT(my_invis, INVIS_RECRUITER);
        if (!ch->is_npc( ) && ch->getClan( )->isLeader( ch->getPC( ) ))
            SET_BIT(my_invis, INVIS_LEADER);
        if (my_web)
            SET_BIT(my_invis, INVIS_WEB);
    }
   
    reset( );
}

void VisibilityTags::setWeb(bool web) 
{
    my_web = web;
    if (my_web)
        SET_BIT(my_invis, INVIS_WEB);
    else
        REMOVE_BIT(my_invis, INVIS_WEB);
}

void VisibilityTags::reset( )
{
    st_rlang = &*lang_common;
    st_clang = LANG_NONE;
    st_nlang = LANG_NONE;
    st_slang = LANG_NONE;
    st_sex = SEX_EITHER;
    st_time = SUN_NONE;
    st_align = N_ALIGN_NULL;
    invert_align = false;
    st_invis = INVIS_NONE;
    my_hyper_tag = 0;
}

// See if some characters need to be replaced with HTML entities.
bool VisibilityTags::need_escape( )
{
    // Don't escape for telnet clients.
    if (!my_web) {
        return false;
    }
    // Don't escape strings inside "{Iw" tags.
    if (IS_SET(st_invis, INVIS_WEB)) {
        return false;
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

            if (c != '\n' && c != '\r')
                if (!rlang_tag_work( )
                     || !clang_tag_work( )
                     || !nlang_tag_work( )
                     || !slang_tag_work( )
                     || !align_tag_work( )
                     || !sex_tag_work( )
                     || !time_tag_work( )
                     || !invis_tag_work( ))
            continue;
           
            html_escape( out );
            continue;
        }
        
        switch (*++p) {
        // composite two-letter tags
        case 'L':
            rlang_tag_parse( );
            break;
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
        case 'h':
            hyper_tag_start( out );
            break;

        // Total reset, but leave {x there for color parser to pick up.
        // Ideally each tag should reset individually, but historically many
        // texts rely on this 'total reset' sequence.
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
            
// {h
// close hyper link: x
// supported hyper link types: c (<hc>command</hc>), l (<hl>hyper link</hl>), h (<hh>help article</hh>),
// g (<hg>skill group names</hg>)
void VisibilityTags::hyper_tag_start( ostringstream &out )
{
    switch (*++p) {
    case 'c': 
        my_hyper_tag = "hc";
        break;

    case 'l': 
        my_hyper_tag = "hl";
        break;

    case 'h': 
        my_hyper_tag = "hh";
        break;

    case 'g': 
        my_hyper_tag = "hg";
        break;

    default:
    case 'x':
    case 'X': 
        hyper_tag_end( out );
        return;
    }

    if (IS_SET(my_invis, INVIS_WEB)) 
        out << "\036" << "<" << my_hyper_tag << ">" << "\037";
}    

void VisibilityTags::hyper_tag_end( ostringstream &out )
{
    if (my_hyper_tag) {
        if (IS_SET(my_invis, INVIS_WEB)) 
            out << "\036" << "</" << my_hyper_tag << ">" << "\037";
        
        my_hyper_tag = 0;
    }
}

// {L 
// close race lang: x
// race lang: x(common), h(uman), e(lvish), d(warvish), n(gnomish), g(iant), t(rollish), c(at)
bool VisibilityTags::rlang_tag_work( )
{
    if (st_rlang != &*lang_common) {
        DLString cstr;

        cstr.assign( c );
        cstr = st_rlang->translate( cstr, NULL, ch );
        
        if (!cstr.empty( ))
            c = cstr.at( 0 );
    }

    return true;
}

void VisibilityTags::rlang_tag_parse( )
{
    switch (*++p) {
    case 'h': st_rlang = &*lang_human;            break;
    case 'e': st_rlang = &*lang_elvish;            break;
    case 'd': st_rlang = &*lang_dwarvish;   break;
    case 'n': st_rlang = &*lang_gnomish;    break;
    case 'g': st_rlang = &*lang_giant;            break;
    case 't': st_rlang = &*lang_trollish;   break;
    case 'c': st_rlang = &*lang_cat;            break;
    default:  --p; /* FALLTHROUGH */
    case 'X':
    case 'x': st_rlang = &*lang_common;            break;
    }
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
// invis type: i (imm-only), c (coder-only), r (clanrecruiter-only), l (clanleader-only)
bool VisibilityTags::invis_tag_work( )
{
    return st_invis == INVIS_NONE || IS_SET(my_invis, st_invis);
}

void VisibilityTags::invis_tag_parse( ostringstream &out )
{
    switch (*++p) {
    case 'i': st_invis = INVIS_IMMORTAL; break;
    case 'c': st_invis = INVIS_CODER; break;
    case 'r': st_invis = INVIS_RECRUITER; break;
    case 'l': st_invis = INVIS_LEADER; break;
    case 'w': 
              st_invis = INVIS_WEB; 
              if (IS_SET(my_invis, INVIS_WEB)) 
                  out << "\036";
              break;
    default:  --p; /* FALLTHROUGH */ 
    case 'x':
    case 'X': 
              st_invis = INVIS_NONE; 
              if (IS_SET(my_invis, INVIS_WEB)) 
                  out << "\037";
              break;
    }
}

/*------------------------------------------------------------------------------------
 * utils 
 *------------------------------------------------------------------------------------*/
void mudtags_convert( const char *text, ostringstream &out, Character *ch )
{
     ostringstream vbuf;
     VisibilityTags( text, ch ).run( vbuf );

     DLString vbufStr = vbuf.str( );
     ColorTags( vbufStr.c_str( ), ch ).run( out );
}

void mudtags_convert_nocolor( const char *text, ostringstream &out, Character *ch )
{
     ostringstream vbuf;
     VisibilityTags( text, ch ).run( vbuf );

     DLString vbufStr = vbuf.str( );
     ColorTags ct( vbufStr.c_str( ), ch );
     ct.setColor( false );
     ct.run( out );
}

void mudtags_raw( const char *text, ostringstream &out )
{
      ColorTags ct( text );
      ct.setColor( false );
      ct.run( out );
}

void vistags_convert( const char *text, ostringstream &out, Character *ch )
{
    VisibilityTags( text, ch ).run( out );
}

void mudtags_convert_web( const char *text, ostringstream &out, Character *ch )
{
     ostringstream vbuf;
     VisibilityTags vtags( text, ch );
     vtags.setWeb(true);
     vtags.run( vbuf );

     DLString vbufStr = vbuf.str( );
     ColorTags ctags( vbufStr.c_str( ), ch );
     ctags.setWeb(true);
     ctags.run( out );
}
