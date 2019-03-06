/* $Id$
 *
 * ruffina, 2004
 */
#include "pagerhandler.h"
#include "dl_ctype.h"
#include "descriptor.h"
#include "character.h"

const char *MSG_HIT_RET       = "\r[Нажмите Return для продолжения]\n\r";

PagerHandler::PagerHandler()
{
}

PagerHandler::PagerHandler(const char *s)
{
    str.setValue(s);
}

int
PagerHandler::handle(Descriptor *d, char *input)
{
    ostringstream out;
    int lines = 0, show_lines;

    while (dl_isspace(*input))
        input++;

    if (*input) {
        if(!d->handle_input.empty( ))
            d->handle_input.pop_front( );

        return true;
    }

    if (d->character)
        show_lines = d->character->lines;
    else
        show_lines = 0;

    const char *p = str.getValue().c_str();

    for(; ; ptr++) {
        char c = p[ptr];
        
        if (c)
            out << c;
        
        switch(c) {
            case '\r':
                continue;
            case '\n':
                lines++;
                /* FALLTHROUGH */
            default:
                if(c == 0 || (show_lines > 0 && lines >= show_lines)) {
                    const string &b = out.str();
                    
                    d->send(b.c_str());

                    int i;
                    for (i = ptr; dl_isspace(p[i]); i++)
                        ;
                    
                    if (!p[i])
                        if(!d->handle_input.empty( ))
                            d->handle_input.pop_front( );
                    
                    return true;
                }
                break;
        }
    }

    return true;
}

void
PagerHandler::prompt(Descriptor *d)
{
    d->send(MSG_HIT_RET);
}


