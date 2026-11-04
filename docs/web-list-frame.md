# The "list" frame

Some commands print a list: skills, skill groups, help matches, socials, commands, the vault, notes, areas, the questor's offers. For web clients the server also sends a structured copy of that list. A client can then build a menu from it without parsing the text, which differs in each language and changes over time. Players see the same text as before, and telnet players get nothing extra.

mudjs ignores frames it has no handler for, so the frame is safe to send to any web client.

## Shape

```json
{"command": "list", "args": [{
  "kind": "skills",
  "title": "",
  "rows": [{
    "label": "bash, 75% (80%)",
    "id": "bash",
    "section": "6",
    "data": {"level": 6, "available": true, "learned": 75, "real": 80, "moves": 13},
    "actions": [
      {"verb": "practice", "cmd": "practice bash"},
      {"verb": "help", "cmd": "help 1234"}
    ]
  }]
}]}
```

- `kind`: what the list is. Current kinds are skills, glist, group, help, socials, commands, vault, notes, areas and quest_request.
- `title`: optional, in the reader's language. It may be empty.
- `label`: the row as one line in the reader's language, without colour codes. A client can show it as it is.
- `id`: optional stable id, such as the English skill name, a help article number or a row number.
- `section`: optional group heading in the reader's language, such as a level or a help category.
- `data`: optional raw values for clients that want to sort or format. A client must work without them.
- `actions`: what the row can do. `verb` is an English word from the list below, and `cmd` is the command to send. Commands use English command names, so they work in every language.

Verbs: help, read, practice, open, do, social_at, get, filter, look, whois, tell, path, request, buy. A client shows its own translated label for a verb it knows and the plain command for one it doesn't. That lets the server add verbs without breaking clients.

- `open` and `filter` mean the reply is another list, such as a skill group or a vault filter.
- `social_at` and `tell` mean the command still needs a name or a message from the player.
- `request` closes the list, since the questor's reply ends the conversation.

## Sending it from C++

`WebList` in plug-ins/descriptor/websocketrpc.h builds the frame. It does nothing for players who are not on a web client, so the same code runs for everyone.

```cpp
WebList web(ch, "help");
for (...) {
    buf << ...;                                   // the text, unchanged
    int row = web.row(title, DLString(id));
    web.action(row, "read", "help " + DLString(id));
}
web.send();
page_to_char(buf.str().c_str(), ch);
```

Send the frame before or together with the text. The frame is written at once, while the text is buffered, so the client gets the frame first and can match it to the text that follows.

## Sending it from Fenia

Scripts can use `ch.writeWSCommand` (CharacterWrapper::writeWSCommand) with the same shape. Practice, train, service, the shop list, craft, and the questor's trade list and zone quests are Fenia scripts, so this is how they would join in. For example, a shop row could be:

```
{"label": "small wooden shield, level 3, 43 silver", "id": "5",
 "actions": [{"verb": "buy", "cmd": "buy 5"}, {"verb": "look", "cmd": "properties 5"}]}
```

## Lists that need no frame

Inventory, equipment, the room, who and where already carry `<m>` menus or command links on every line. A client can read those in any language, so they don't need this frame.
