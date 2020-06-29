/* $Id: material.cpp,v 1.1.2.6 2009/01/18 20:12:41 rufina Exp $
 *
 * ruffina, 2005
 */
#include "object.h"
#include "character.h"
#include "mercdb.h"
#include "merc.h"

#include "immunity.h"
#include "material.h"

static void 
material_parse( Object *obj, const material_t **argv, int size )
{
    const material_t **ap = argv;
    
    if (obj->getMaterial( )) {
        char *token, name[MAX_STRING_LENGTH];

        strcpy( name, obj->getMaterial( ) );
        
        token = strtok( name, ", " );
        
        for(token=strtok(name, ", "); token; token=strtok(NULL, ", "))
            if (*token != '\0') {
                const material_t *mat;

                for (mat = &material_table[0]; mat->name; mat++)
                    if (!str_cmp( token, mat->name )) {
                        *ap++ = mat;
                        break;
                    }                

                if (ap >= argv + size) 
                    return;
            }
    }

    if (ap < argv + size)
        *ap = NULL;
}

static const int msize = 10;

bool material_is_typed( Object *obj, int type )
{
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (ap = argv; ap < argv + msize && *ap != NULL; ap++) 
        if (IS_SET( (*ap)->type, type ))
            return true;

    return false;
}

bool material_is_flagged( Object *obj, int flags )
{
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (ap = argv; ap < argv + msize && *ap != NULL; ap++) 
        if (IS_SET( (*ap)->flags, flags ))
            return true;

    return false;
}

int material_swims( Object *obj )
{
    int swim;
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (swim = 0, ap = argv; ap < argv + msize && *ap != NULL; ap++) 
        swim += (*ap)->floats;
    
    if (swim > 0)
        return SWIM_ALWAYS;
    else if (swim < 0)
        return SWIM_NEVER;
    else
        return SWIM_UNDEF;
}

int material_burns( Object *obj )
{
    int max_burn;
    const material_t *argv[msize], **ap;

    material_parse( obj, argv, msize );
    
    for (max_burn = 0, ap = argv; ap < argv + msize && *ap != NULL; ap++) 
        if ((*ap)->burns < 0)
            return (*ap)->burns;
        else if ((*ap)->burns > max_burn)
            max_burn = (*ap)->burns;

    return max_burn;
}

int material_immune( Object *obj, Character *ch )
{
    const material_t *argv[msize], **ap;
    bitstring_t bits = 0;
    int res = RESIST_NORMAL;

    material_parse( obj, argv, msize );
    
    for (ap = argv; ap < argv + msize && *ap != NULL; ap++) 
        SET_BIT( bits, (*ap)->vuln );
    
    immune_from_flags( ch, bits, res );

    return immune_resolve( res );
}

const material_t material_table [] = {
 { "unknown",   0,  0, MAT_NONE,   0, 0, "неизвестный материал" },
 { "none",      0,  0, MAT_NONE,   0, 0, "неизвестный материал" },
 { "oldstyle",  0,  0, MAT_NONE,   0, 0, "неизвестный материал" },

 // metals
 { "adamantite",0, -1, MAT_METAL, 0, 0, "адамантит" }, // "dull, heavy and dense" (c) adnd
 { "aluminum",  0, -1, MAT_METAL, 0, 0, "алюминий" },
 { "brass",     0, -1, MAT_METAL, 0, 0, "латунь" },
 { "bronze",    0, -1, MAT_METAL, 0, 0, "бронза" },
 { "copper",    0, -1, MAT_METAL, 0, 0, "медь" },
 { "gold",      0, -1, MAT_METAL, 0, 0, "золото" },
 { "iron",      0, -1, MAT_METAL, 0, VULN_IRON, "железо" },
 { "lead",      0, -1, MAT_METAL, 0, 0, "свинец" },
 { "metal",     0, -1, MAT_METAL, 0, 0, "металл" },
 { "mithril",   0, -1, MAT_METAL, 0, VULN_MITHRIL, "мифрил" }, // "reflective, light and pure" (c) adnd
 { "platinum",  0, -1, MAT_METAL, MAT_INDESTR, 0, "платина" }, 
 { "silver",    0, -1, MAT_METAL, 0, VULN_SILVER, "серебро" },
 { "steel",     0, -1, MAT_METAL, 0, VULN_IRON, "сталь" },
 { "tin",       0, -1, MAT_METAL, 0, 0, "олово" },
 { "titanium",  0, -1, MAT_METAL, MAT_TOUGH, 0, "титан" },

 // gems
 { "amethyst",  0, -1, MAT_GEM,   0, 0, "аметист" },
 { "diamond",   0, -1, MAT_GEM,   0, 0, "алмаз" },
 { "emerald",   0, -1, MAT_GEM,   0, 0, "изумруд" },
 { "gem",       0, -1, MAT_GEM,   0, 0, "драгоценный камень" },
 { "jade",      0, -1, MAT_GEM,   0, 0, "нефрит" },
 { "lapis",     0, -1, MAT_GEM,   0, 0, "лазурит" },
 { "malachite", 0, -1, MAT_GEM,   0, 0, "малахит" },
 { "moonstone", 0, -1, MAT_GEM,   0, 0, "лунный камень" },
 { "onyx",      0, -1, MAT_GEM,   0, 0, "оникс" },
 { "opal",      0, -1, MAT_GEM,   0, 0, "опал" },
 { "ruby",      0, -1, MAT_GEM,   0, 0, "рубин" },
 { "sandstone", 0, -1, MAT_GEM,   0, 0, "песчаник" },
 { "sapphire",  0, -1, MAT_GEM,   0, 0, "сапфир" },
 { "topaz",     0, -1, MAT_GEM,   0, 0, "топаз" },

 // wood
 { "ash",       7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "ясень" },
 { "aspen",     5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "осина" },
 { "bamboo",    6,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "бамбук" },
 { "ebony",     7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "черное дерево" },
 { "hardwood",  7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "твердая древесина" },
 { "oak",       7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "дуб" },
 { "redwood",   7,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "красное дерево" },
 { "softwood",  5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "мягкое дерево" },
 { "wood",      5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "древесина" },
 { "yew",       5,  1, MAT_ORGANIC|MAT_WOOD, 0, VULN_WOOD, "тис" },

 // cloth
 { "canvas",    5,  1, MAT_CLOTH, 0, 0, "брезент||а|у||ом|е" },
 { "cashmere",  5,  1, MAT_CLOTH, 0, 0, "кашемир||а|у||ом|е" },
 { "cloth",     3,  1, MAT_CLOTH, 0, 0, "ткан|ь|и|и|ь|ью|и" },
 { "cotton",    3,  1, MAT_CLOTH, 0, 0, "хлоп|ок|ка|ку|ок|ком|ке" },
 { "felt",      3,  1, MAT_CLOTH, 0, 0, "войл|ок|ка|ку|ок|ком|ке" },
 { "fur",       4,  1, MAT_CLOTH, 0, 0, "мех||а|у||ом|е" },
 { "hard leather",5,1, MAT_CLOTH, 0, 0, "дублен|ая|ой|ой|ую|ой|ой кож|а|и|е|у|ей|е" },
 { "leather",   4,  1, MAT_CLOTH, 0, 0, "кож|а|и|е|у|ей|е" },
 { "linen",     3,  1, MAT_CLOTH, 0, 0, "полотн|о|а|у|о|ом|е" },
 { "satin",     3,  1, MAT_CLOTH, 0, 0, "атлас||а|у||ом|е" },
 { "silk",      3,  1, MAT_CLOTH, 0, 0, "шелк||а|у||ом|е" },
 { "soft leather",4,1, MAT_CLOTH, 0, 0, "мягк|ая|ой|ой|ую|ой|ой кож|а|и|е|у|ей|е" },
 { "tweed",     4,  1, MAT_CLOTH, 0, 0, "твид||а|у||ом|е" },
 { "velvet",    3,  1, MAT_CLOTH, 0, 0, "бархат||а|у||ом|е" },
 { "wool",      4,  1, MAT_CLOTH, 0, 0, "шерст|ь|и|и|ь|ью|и" },
                   
 // elements       
 { "air",       0,  1, MAT_ELEMENT,   0, 0, "воздух" },
 { "wind",      0,  1, MAT_ELEMENT,   0, 0, "ветер" },
 { "fire",      0,  1, MAT_ELEMENT,   0, VULN_FIRE, "огонь" },
 { "flame",     0,  1, MAT_ELEMENT,   0, VULN_FIRE, "пламя" },
 { "ice",      -1,  1, MAT_ELEMENT,   MAT_MELTING, VULN_COLD, "лед" },
 { "light",     0,  1, MAT_ELEMENT,   0, VULN_LIGHT, "свет" },
 { "water",    -1,  1, MAT_ELEMENT,   0, VULN_DROWNING, "вода" },
 { "snow",     -1,  1, MAT_ELEMENT,   MAT_MELTING, VULN_COLD|VULN_DROWNING, "снег" },
                   
 // minerals       
 { "brick",     0, -1, MAT_MINERAL,   0, 0, "кирпич" },
 { "ceramics",  0,  0, MAT_MINERAL,   0, 0, "керамика" },
 { "clay",      0,  0, MAT_MINERAL,   0, 0, "глина" },       
 { "corund",    0,  0, MAT_MINERAL,   0, 0, "корунд" },
 { "crystal",   0,  0, MAT_MINERAL,   0, 0, "хрусталь" },
 { "glass",     0,  0, MAT_MINERAL,   MAT_FRAGILE, 0, "стекло" },
 { "granite",   0, -1, MAT_MINERAL,   0, 0, "гранит" },
 { "ground",   -1,  0, MAT_MINERAL,   0, 0, "грунт" },
 { "marble",    0, -1, MAT_MINERAL,   0, 0, "мрамор" },
 { "obsidian",  0, -1, MAT_MINERAL,   0, 0, "обсидиан, вулк. стекло" },
 { "parafin",   0,  1, MAT_MINERAL,   0, 0, "парафин" },
 { "plastic",   2,  0, MAT_MINERAL,   0, 0, "пластик" },
 { "porcelain", 0,  0, MAT_MINERAL,   0, 0, "фарфор" },
 { "quartz",    0,  0, MAT_MINERAL,   0, 0, "кварц" },
 { "stone",     0,  0, MAT_MINERAL,   0, 0, "камень" },
 { "sulfur",    2,  1, MAT_MINERAL,   0, 0, "сера" },
                   
 // organics       
 { "amber",     0,  0, MAT_ORGANIC,   0, 0, "янтарь" },
 { "chalk",     0,  0, MAT_ORGANIC,   0, 0, "мел" },
 { "coal",      8,  0, MAT_ORGANIC,   0, 0, "уголь" },
 { "coral",     0,  0, MAT_ORGANIC,   0, 0, "коралл" },
 { "fiber",     4,  0, MAT_ORGANIC,   0, 0, "волокно" },
 { "pearl",     0, -1, MAT_ORGANIC,   0, 0, "жемчуг" },
 { "rubber",    3,  0, MAT_ORGANIC,   0, 0, "каучук" },
 { "shell",     0,  0, MAT_ORGANIC,   0, 0, "скорлупа" },
 { "wax",       0,  0, MAT_ORGANIC,   MAT_MELTING, 0, "воск" },
                   
 // papers
 { "paper",     1,  1, MAT_ORGANIC,   0, 0, "бумага" },
 { "parchment", 1,  1, MAT_ORGANIC,   0, 0, "пергамент" },
 { "vellum",    1,  1, MAT_ORGANIC,   0, 0, "тонкий пергамент" },

 // bones          
 { "bone",      0,  0, MAT_ORGANIC,   0, 0, "кость" },
 { "human femur",0, 0, MAT_ORGANIC,   0, 0, "человеческое бедро" },
 { "ivory",     0, -1, MAT_ORGANIC,   0, 0, "слоновая кость" },
                   
 // flesh          
 { "bearskin",  6,  1, MAT_ORGANIC,   0, 0, "медвежья шкура" },
 { "blood",    -1,  1, MAT_ORGANIC,   0, 0, "кровь" },
 { "dragonskin",8,  1, MAT_ORGANIC,   0, 0, "драконья шкура" },
 { "feathers",  1,  1, MAT_ORGANIC,   0, 0, "перья" },
 { "fish",      1,  1, MAT_ORGANIC,   0, 0, "рыба" },
 { "flesh",     1,  1, MAT_ORGANIC,   0, 0, "плоть" },
 { "gut",       1,  1, MAT_ORGANIC,   0, 0, "кишка" },
 { "hair",      1,  1, MAT_ORGANIC,   0, 0, "волосы" },
 { "human flesh",1, 1, MAT_ORGANIC,   0, 0, "человеческая плоть" },
 { "lion-fell", 4,  1, MAT_ORGANIC,   0, 0, "львиная шкура" },
 { "skin",      2,  1, MAT_ORGANIC,   0, 0, "кожа, шкура" },
 { "snakeskin", 2,  1, MAT_ORGANIC,   0, 0, "змеиная кожа" },
                   
 // plant          
 { "flower",    1,  1, MAT_ORGANIC,   0, 0, "цветок" },
 { "gourd",     1,  1, MAT_ORGANIC,   0, 0, "тыква" },
 { "grain",     1,  1, MAT_ORGANIC,   0, 0, "зерно" },
 { "grass",     1,  1, MAT_ORGANIC,   0, 0, "трава" },
 { "hay",       1,  1, MAT_ORGANIC,   0, 0, "сено" },
 { "hemp",      1,  1, MAT_ORGANIC,   0, 0, "конопля" },
 { "moss",      1,  1, MAT_ORGANIC,   0, 0, "мох" },
 { "plant",     1,  1, MAT_ORGANIC,   0, 0, "растение" },
 { "plant_organism",1,1, MAT_ORGANIC,   0, 0, "растительный организм" },
 { "pollen",    1,  1, MAT_ORGANIC,   0, 0, "пыльца" },
 { "root",      1,  1, MAT_ORGANIC,   0, 0, "корень" },
 { "straw",     1,  1, MAT_ORGANIC,   0, 0, "солома" },
                   
 // meal           
 { "bread",     1,  0, MAT_ORGANIC,   0, 0, "хлеб" },
 { "cake",      1,  0, MAT_ORGANIC,   0, 0, "печеное тесто" },
 { "pastry",    1,  0, MAT_ORGANIC,   0, 0, "печеное тесто" },
 { "drink",    -1,  0, MAT_ORGANIC,   0, 0, "питье" },
 { "flour",     1,  0, MAT_ORGANIC,   0, 0, "мука" },
 { "food",      2,  0, MAT_ORGANIC,   0, 0, "пища" },
 { "meat",      1,  0, MAT_ORGANIC,   0, 0, "мясо" },
                   
 // abstract       
 { "darkness",   0, 1, MAT_ABSTRACT,   0, 0, "тьма" },
 { "energy",     0, 1, MAT_ABSTRACT,   0, VULN_ENERGY, "энергия" },
 { "entropia",   0, 1, MAT_ABSTRACT,   0, 0, "энтропия" },
 { "ethereal",   0, 1, MAT_ABSTRACT,   0, 0,   },
 { "etherealness",0,1, MAT_ABSTRACT,   0, 0,   },
 { "evil",       0, 1, MAT_ABSTRACT,   0, 0, "зло" },
 { "magic",      0, 1, MAT_ABSTRACT,   0, VULN_MAGIC, "магия" },
 { "nothingness",0, 1, MAT_ABSTRACT,   0, 0, "ничего" },
 { "shadow",     0, 1, MAT_ABSTRACT,   0, 0, "тень" },
 { "vacuum",     0, 1, MAT_ABSTRACT,   0, 0, "вакуум" },

 { NULL,         0, 0, MAT_NONE,   0, 0, }, 
};

