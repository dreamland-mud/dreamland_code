/* $Id: cwizlist.h,v 1.1.2.2.10.2 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    Zadvinsky Alexandr   {Kiddy}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#ifndef CWIZLIST_H
#define CWIZLIST_H

#include <list>

#include "commandplugin.h"
#include "defaultcommand.h"
#include "pcmemoryinterface.h"

class CWizlist : public CommandPlugin, public DefaultCommand {
XML_OBJECT
public:
    typedef ::Pointer<CWizlist> Pointer;

    CWizlist( );

    virtual void run( Character*, const DLString& constArguments );
    
private:
    static const DLString COMMAND_NAME;
                
    /* Элемент массива с именами уровней */
    struct GodLevelName {
        DLString name;
        int level;
        const char * color;
    };

    /* Уровни богов */
    static GodLevelName names[];
    
    /* Кусочки мечей, рисуемых по бокам списка */
    struct SwordLine {
        DLString name;
        int count;
    };

    /* Кусочки меча */
    static SwordLine swordLines[];
    
    int cSwordLine;
    int lineCounter;
    static const int textLine; /* Ширина текста между мечами */
    
    void initSwords( );
    void writeSwordLine( std::ostream &, char *, char * );
    void writeSwordPixels( std::ostream &, char, int );

    struct CompareGods : public std::binary_function<PCMemoryInterface *, PCMemoryInterface *, bool>
    {
        bool operator () ( PCMemoryInterface *a, PCMemoryInterface *b )
        {
            if (a->getLevel( ) == b->getLevel( ))
                return a->getName( ) < b->getName( );
            else
                return a->getLevel( ) > b->getLevel( );
        }
    };

    typedef std::list<PCMemoryInterface *> GodList;
};

#endif






