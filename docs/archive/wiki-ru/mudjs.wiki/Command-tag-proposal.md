Two approaches to implementing custom command, where displayed part is different from the action (and echoed command):

**1. [cmd] tag**

Source: `[cmd=quest $1,see=13,nonce=r4U78eAa]`

Result: 
```html
<span class="manip-cmd manip-ed" data-action="quest 13" data-echo="квест 13">13</span>
```

C++ usage example:
```cpp
string lineFormat = "[" + web_cmd(ch, "cs web $1", "%5d") + "] ... \r\n";
buf << dlprintf(lineFormat.c_str(), id, ...) << endl;
```
Fenia usage example:
```cpp
header = "[{w" + .webcmd(ch, "quest $1", "%3d") + "{w]     %-40s %-20s\n";

ch.act(header, 13, "Охота на ведьм", "Шабаш");
```


**2. <cmd> </cmd> tags**

Source: 
```html
<cmd c='quest $1' l='13' nonce='r4U78eAa'>13</cmd>
```

Result: 
```html
<span class="manip-cmd manip-ed" data-action="quest 13" data-echo="квест 13">13</span>
```

C++ usage example: same as above. 
```cpp
string lineFormat = "[" + web_cmd(ch, "cs web $1", "%5d") + "] ... \r\n";
buf << dlprintf(lineFormat.c_str(), id, ...) << endl;
```