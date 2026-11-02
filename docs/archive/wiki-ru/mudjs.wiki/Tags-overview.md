## Current tags and commands

### HTML-like tags

#### Color tags: {W, {r, {x
Source: `{Iw<c c='fgbg'>{Ixярко-зеленое сообщение{Iw</c>{Ix`

Result: `<span class='fgbg'>ярко-зеленое сообщение</span>`


#### Execute command: {hc ... {hx
Source: `<hc>look plaque</hc>`

Result: `<span class="manip-cmd" data-action="look plaque" data-echo="look plaque">look plaque</span>`

#### Show help article: {hh ... {hx
Source: `<hh>квестор</hh>`

Result: `<span class="manip-cmd" data-action="help квестор" data-echo="справка квестор">квестор</span>`


#### HTML hyper-link: {hl ... {hx
Source: `<hl>https://ya.ru</hl>`

Result: `<a target="_blank" class="manip-link" href="https://ya.ru">https://ya.ru</a>`

#### List skill group: {hg ... {hx
Source: `<hg>defensive</hg>`

Result: `<span class="manip-cmd" data-action="glist defensive" data-echo="группаумен defensive">defensive</span>`

#### Show a speedwalk: {hs ... {hx
Source: `<hs>6e10nd</hs>`

Result: `<span class="manip-cmd" data-action="run 6e10nd" data-echo="бежать 6e10nd">6e10nd</span>`

#### Item drop-down menu
Source: `{Iw<m c='бросить $, надеть $' l='чинить $' id='5573732900'>{Ixхитрость лаеркаи{Iw</m>{Ix`

Result: 
```html
<span class="dropdown-norelative">
    <span class="dropdown-toggle" data-toggle="dropdown">хитрость лаеркаи</span>
    <span class="dropdown-menu">
        <a class="dropdown-item manip-item" data-action="бросить 5573732900" href="#">бросить </a>      
        <a class="dropdown-item manip-item" data-action="надеть 5573732900" href="#">надеть </a>      
        <div class="dropdown-divider"></div>
        <a class="dropdown-item manip-item" data-action="чинить 5573732900" href="#">чинить </a>      
   </span>
</span>
```

### Square-bracketed commands

#### Map button
Source: `[map=hitower.are]`

Result: `<a class="btn btn-sm btn-outline-info btn-orange" href="https://dreamland.rocks/maps/hitower.html" target=_blank>открыть карту</a>`

#### Read extra-description: знак (sign)
Source: `{Iw[read=sign знак,see=sign]{Ix{IW(sign){Ix`

Result: `(<span class="manip-cmd manip-ed" data-action="read 'sign знак'">sign</span>)`

#### Command
Source: `[cmd=quest $1,see=13,nonce=r4U78eAa]`

Result: `<span class="manip-cmd manip-ed" data-action="quest 13" data-echo="квест 13">13</span>`
