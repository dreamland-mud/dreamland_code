Prompt example with comments. This prompt is accessible from the "cog" (aka `settings.js`) as `mudprompt` variable.

```javascript
{
  "area": "midgaard.are", // Filename of the current area (for map highlight).
  "cln": {                // Clan affects
    "a": "e",             // current ('evolve')
    "z": ""               // will disappear soon (none)
  },
  "date": {               // Current date/month/year
    "d": 14,
    "m": "Тени",
    "y": 670
  },
  "det": {                // Detection affects
    "a": "riw"            // current ('detect invis', 'infravision', 'imp detect')
  },
  "enh": {                // Enhancement affects
    "a": "b"              // current ('bless')
  },
  "exits": {              // List of room exits
    "e": "neswu",         // visible
    "h": "",              // hidden (need 'perception')
    "l": "e"              // 'config ruexits': e or r
  }, 
  "group": {              // Player group.
    "leader": {           // leader fields (same for all other pc/npc below)
      "health": 100,      // this group member's health percentage
      "hit": 59058,       // their current health
      "hit_clr": "6",     // health color
      "level": 100,       // their level
      "max_hit": 59058,   // their max health
      "sees": "Ruffina",  // their name as you see it
      "tnl": 129062       // their exp to next level
    },
    "ln": "Ruffina",      // Leader name
    "pc": [               // list of other players in the group
      { "health": 100, "hit": 3000, "hit_clr": "6", "level": 100, 
        "max_hit": 3000, "sees": "Tahi", "tnl": 999 }
    ],
    "npc": [              // list of mobs in the group
      { "health": 100, "hit": 739, "hit_clr": "6", "level": 28, 
        "max_hit": 739, "sees": "лошадь", "tnl": "" }
    ]
  },
  "hit": 59058,           // current health
  "mal": {                // Negative affects
    "a": "",              // current (none)
    "z": "s"              // will disappear soon ('slow')
  },
  "mana": 65969,          // current mana
  "max_hit": 59058,       // max health
  "max_mana": 65969,      // max mana
  "max_move": 18080,      // max move
  "move": 18080,          // current move
  "p1": {                 // parameters
    "cs": [               // current stats (str, int, wis, dex, con, cha)
      "19",      "23",      "17",      "22",      "20",      "10"
    ],
    "ps": [               // permanent stats (str, int, wis, dex, con, cha)
      "19",      "20",      "12",      "23",      "19",      "10"
    ]
  },
  "p2": {            // more parameters
    "a": "-315",     // armor class
    "d": "78",       // damroll
    "h": "65",       // hitroll
    "s": "-23",      // saves vs spell
    "pos": "stand",  // position (stand, sit, rest, sleep, fight, stun, incap, mort, dead)
    "posf": "",      // position flags ('fly_down')
  },
  "pro": {           // Protective affects
    "a": "SAsa"      // current ('sanctuary', 'armor', 'enh armor', 'shield')
  },
  "q": {             // Questor quest
    "i": "Вылечить гигантскую саранчу из Бездна (Армагеддон).",  // short 'quest info'
    "t": 19                                                      // time remaining
  },  
  "room": "Храмовая площадь",     // Current room name
  "time": {                       // Current time
    "h": 7,                       // hour of the day
    "l": "сумерки",               // sunlight: night, dusk etc
    "tod": "вечера"               // time of day: evening, morning
  },
  "trv": {                        // Transport affects
    "a": "pi"                     // current ('pass door', 'invisibility')
  },
  "vnum": 3005,                   // Current room vnum (for map hightlight)
  "w": {                          // Weather
    "i": "night-alt-cloudy",      // icon to display
    "m": "облачно, теплый ветер"  // weather description
  },
  "who": {                        // List of online players
    "p": [
      {
        "cc": "dg",               // clan color (dark green)
        "cn": "l",                // clan name (Lion)
        "n": "Ruffina",           // player name
        "r": "sa",                // race acronym (satyr)
        "s": "f"                  // sex (female)
      },
      {
        "cc": "dg", "cn": "l", "n": "Tahi", "r": "sa", "s": "f"
      }
    ],
    "t": "2"                     // Total
  },
  "zone": "Мидгаард"             // Current zone name
}
```