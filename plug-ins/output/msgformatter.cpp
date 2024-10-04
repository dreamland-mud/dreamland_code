/* $Id$
 *
 * ruffina, 2004
 */
#include "msgformatter.h"
#include "logstream.h"
#include "dl_ctype.h"
#include "noun.h"
#include "grammar_entities_impl.h"
#include "character.h"
#include "skill.h"
#include "merc.h"

#include "def.h"

MsgFormatter::MsgFormatter(Character *to)
                : to(to)
{
}

MsgFormatter::~MsgFormatter()
{
}

DLString MsgFormatter::myupcase(const DLString &str, int upcase)
{
    if(upcase == 0)
        return str;
    
    DLString msg = str;
    
    unsigned int i;

    for(i = 0; i < msg.size();i++) {
        if(msg[i] == '{') {
            i++;
            continue;
        }

        if(msg[i] != ' ' && msg[i] != '\t') {
            if(dl_isalpha(msg[i])) {
                if(upcase > 0)
                    msg[i] = dl_toupper(msg[i]);
                else
                    msg[i] = dl_tolower(msg[i]);
            }
            
            if(upcase == 1 || upcase == -1)
                break;
        }
    }

    return msg;
}

DLString MsgFormatter::pad(bool left, int width, int limit, int upcase, const DLString &str)
{
    DLString rc, tmp;
    int i, len;

    tmp = myupcase(str, upcase);
    
    if(limit) {
        bool c = false;
        i = 0;

        for(DLString::iterator it=tmp.begin();it != tmp.end();it++) {
            if(i == limit) {
                tmp.erase(it, tmp.end());
                break;
            }

            if(*it == '{') {
                if(c) 
                    i++;
                else {
                    c = true;
                    continue;
                }
            } else if(!c)
                i++;

            c = false;
        }
    }

    len = tmp.colourStrip().size();
    
    if(!width)
        width = len;
    
    if(!left)
        for(i = 0; i < width - len; i++)
            rc += ' ';

    rc += tmp;
    
    if(left)
        for(i = 0; i < width - len; i++)
            rc += ' ';

    return rc;
}

DLString MsgFormatter::run()
{
    static const char * const pronoun[][Grammar::MultiGender::MAX] = {
        { "оно",  "он",   "она", "они"  }, /* 1 */
        { "его",  "его",  "ее",  "их"   },
        { "ему",  "ему",  "ей",  "им"   },
        { "него", "него", "нее", "них"  },
        { "ним",  "ним",  "ней", "ними" },
        { "нем",  "нем",  "ней", "них"  },
        { "нему", "нему", "ней", "ним"  }  /*  7 */
    };
    static const char * const poss_pronoun[][Grammar::MultiGender::MAX] = {
        { "твое",   "твой",   "твоя",  "твои"  },   /*  1 */
        { "твоего", "твоего", "твоей", "твоих"  },   
        { "твоему", "твоему", "твоей", "твоим"  },  
        { "твоего", "твоего", "твою",  "твоих"  },   
        { "твоим",  "твоим",  "твоей", "твоим"  },  
        { "твоем",  "твоем",  "твоей", "твоих"  },  /* 6 */
        { "свое",   "свой",   "своя",  "свои"  },   /* 7 */
        { "своему", "своему", "своей", "своим"  }   /* 8 */
    };
    const char *f;
    int state, i;
    int width, limit;
    int upcase;
    bool left;
    bool alternative;
    char cvt;
    DLString rc, s;
    
    /*keep compiller happy*/
    i = 0;
    left = false;
    alternative = false;
    width = 0;
    limit = 0;
    cvt = 0;
    upcase = 0;
    s = "";

    state = 0;
    for(f = format; ;f++) {
again:
        ostringstream floatBuf;

        switch(state) {
        case 0:            /*blind copy*/
            if(*f == '%') {
                i = 0;
                left = false;
                alternative = false;
                width = 0;
                limit = 0;
                upcase = 0;
                state = 1;
                s = "";
            } else if(*f) {
                rc += *f;
            } else
                goto done;
            break;
        case 1:
            if(*f == '%') {
                rc += '%';
                state = 0;
                break;
            }
            state = 2;
            /* FALL THROUGH */
        case 2:            /*positional argunent specs*/
            if(isdigit(*f)) {
                i *= 10;
                i += *f - '0';
                break;
                
            } else if(*f == '$') {
                shiftArg(i);
                i = 0;
                state = 3;
                break;
            }

            nextArg();
            state = 3;
            /* FALL THROUGH */
        case 3:
            if(*f == '#') {
                alternative = true;
                break;
            } else if(*f == '-') {
                left = true;
                break;
            }
            state = 4;
            /* FALL THROUGH */
        case 4:
            if(isdigit(*f)) {
                i *= 10;
                i += *f - '0';
                break;
            }
            width = i;
            i = 0;
            state = 5;
            /* FALL THROUGH */
        case 5:
            if(isdigit(*f)) {
                i *= 10;
                i += *f - '0';
                break;
            } else if(*f == '.') {
                i = 0;
                break;
            }
            limit = i;
            i = 0;
            state = 55;
            /* FALL THROUGH */
        case 55:
            if(*f == '^') {
                upcase++;
                break;
            } else if(*f == '_') {
                upcase--;
                break;
            }
            state = 6;
            /* FALL THROUGH */
        case 6:
            switch(*f) {
            case 'd':
            case 'i':
            case 'l':
                s += argInt();
                state = 0;
                break;
            case 'u':
                s += argUInt();
                state = 0;
                break;
            case 'f':
                floatBuf.str().clear();
                floatBuf << argFloat();
                s += floatBuf.str();
                state = 0;
                break;
            case 's':
                s += argStr();
                state = 0;
                break;
            case 'S':
                s += DLString(argStr()).getOneArgument();
                state = 0;
                break;
            case 'K':
                s += argSkill()->getNameFor(to);
                state = 0;
                break;
            case 'c': 
                s += argChar();
                state = 0;
                break;
                
            case 'C':
            case 'P':
            case 'T':
            case 'O':
            case 'N':
            case 'p':
                cvt = *f;
                state = 7;
                break;
            case 'G':
            case 'I':
            case 'g':
            case 'n':
                state = 8;
                cvt = *f;
                break;
            default:
                /*invalid conversion*/
                state = 0;
                goto again;
            }
            if(state == 0) {
                rc += pad(left, width, limit, upcase, s);
            }
            break;
        case 7:
            switch(cvt) {
                int cs, sx, nounFlags;
            case 'C':
                nounFlags = (!alternative && to) ? FMT_INVIS|FMT_DOPPEL : 0;
                s += argNoun(nounFlags)->decline(*f);
                break;
            case 'O':
                nounFlags = (alternative ? 0 : FMT_INVIS);
                s += argNoun(nounFlags)->decline(*f);
                break;
            case 'N':
                s += russian_case( argStr(), *f );
                break;
            case 'P':
                cs = URANGE(0, *f - '1', (int)(sizeof(pronoun)/sizeof(*pronoun) - 1));
                nounFlags = (alternative ? 0 : FMT_DOPPEL);
                sx = argNoun(nounFlags)->getMultiGender();
                s += pronoun[cs][sx];
                break;
            case 'p':
                cs = URANGE(0, *f - '1', (int)(sizeof(pronoun)/sizeof(*pronoun) - 1));
                sx = URANGE(0, argInt(), 2);
                s += pronoun[cs][sx];
                break;
            case 'T':
                cs = URANGE(0, *f - '1', (int)(sizeof(poss_pronoun)/sizeof(*poss_pronoun) - 1));
                nounFlags = (alternative ? 0 : FMT_DOPPEL);
                sx = argNoun(nounFlags)->getMultiGender();
                s += poss_pronoun[cs][sx];
                break;
            }
            state = 0;
            rc += pad(left, width, limit, upcase, s);
            break;
        case 8:
        case 9:
        case 10:
        case 11:
            switch(*f) {
            case '|':
                if(state < 11) {
                    state++;
                    break;
                }
                /*FALL THROUGH*/
            case '\0':
            case ' ':
            case '-':
            case '?':
            case '!':
            case ',':
            case '.':
            case ':':
            case '{':
            case '(': case ')':
                state = 0;
                rc += pad(left, width, limit, upcase, s);
                goto again;
            default:
                switch (cvt) {
                    int nounFlags;
                case 'G':
                    nounFlags = (alternative ? 0 : FMT_INVIS|FMT_DOPPEL);
                    if (argNoun(nounFlags)->getMultiGender() == (state - 8))
                        s += *f;
                    break;
                case 'I':
                    if (state == GET_COUNT(argInt(), 8, 9, 10))
                        s += *f;
                    break;
                case 'g':
                    if (URANGE(0, argInt(), 2) == (state - 8))
                        s += *f;
                    break;
                case 'n':
                    nounFlags = (alternative ? 0 : FMT_INVIS);
                    if (argNoun(nounFlags)->getNumber() == (state - 8))
                        s += *f;
                    break;
                }
            }
            break;
        }
    }
done:
    return rc;
}

