## Краш 1
[Feb 09 02:24:37]:N: log stream closed
*** Error in `./bin/dreamland': double free or corruption (!prev): 0x000000000597f4a0 ***
Aborted (core dumped)
```bash
(gdb) bt
#0  0x00007f24de753c37 in __GI_raise (sig=sig@entry=6) at ../nptl/sysdeps/unix/sysv/linux/raise.c:56
#1  0x00007f24de757028 in __GI_abort () at abort.c:89
#2  0x00007f24de7902a4 in __libc_message (do_abort=do_abort@entry=1, fmt=fmt@entry=0x7f24de8a1ef0 "*** Error in `%s': %s: 0x%s ***\n") at ../sysdeps/posix/libc_fatal.c:175
#3  0x00007f24de79c56e in malloc_printerr (ptr=<optimized out>, str=0x7f24de8a1fd8 "double free or corruption (!prev)", action=1) at malloc.c:4996
#4  _int_free (av=<optimized out>, p=<optimized out>, have_lock=0) at malloc.c:3840
#5  0x00007f24dbc025c5 in DefaultBufferHandler::write (this=<optimized out>, d=0x4ade2c0, txt=0x7f24dbc13650 "\r[Нажмите Return для продолжения]\n\r")
    at ../../../dreamland_code/plug-ins/iomanager/defaultbufferhandler.cpp:270
#6  0x00007f24dbbf2468 in process_output (d=d@entry=0x4ade2c0, fPrompt=fPrompt@entry=true) at ../../../dreamland_code/plug-ins/iomanager/comm.cpp:88
```


## Краш 2
[Feb 20 13:46:35]:N: log stream closed
*** Error in `./bin/dreamland': double free or corruption (!prev): 0x00000000051ea500 ***
Aborted (core dumped)
```bash
#0  0x00007f508a2bdc37 in __GI_raise (sig=sig@entry=6) at ../nptl/sysdeps/unix/sysv/linux/raise.c:56
#1  0x00007f508a2c1028 in __GI_abort () at abort.c:89
#2  0x00007f508a2fa2a4 in __libc_message (do_abort=do_abort@entry=1, fmt=fmt@entry=0x7f508a40bef0 "*** Error in `%s': %s: 0x%s ***\n") at ../sysdeps/posix/libc_fatal.c:175
#3  0x00007f508a30656e in malloc_printerr (ptr=<optimized out>, str=0x7f508a40bfd8 "double free or corruption (!prev)", action=1) at malloc.c:4996
#4  _int_free (av=<optimized out>, p=<optimized out>, have_lock=0) at malloc.c:3840
#5  0x00007f508776c5c5 in DefaultBufferHandler::write (this=<optimized out>, d=0x551ce30, txt=0x7f508777d650 "\r[Нажмите Return для продолжения]\n\r")
    at ../../../dreamland_code/plug-ins/iomanager/defaultbufferhandler.cpp:270
#6  0x00007f508775c468 in process_output (d=d@entry=0x551ce30, fPrompt=fPrompt@entry=true) at ../../../dreamland_code/plug-ins/iomanager/comm.cpp:88
```

## Краш 3
[Feb 20 13:50:41]:N: channel [say] Stranger: ling
*** Error in `./bin/dreamland': munmap_chunk(): invalid pointer: 0x0000000004af0960 ***
Aborted (core dumped)
```bash
#0  0x00007fca72c14c37 in __GI_raise (sig=sig@entry=6) at ../nptl/sysdeps/unix/sysv/linux/raise.c:56
#1  0x00007fca72c18028 in __GI_abort () at abort.c:89
#2  0x00007fca72c512a4 in __libc_message (do_abort=do_abort@entry=1, fmt=fmt@entry=0x7fca72d62ef0 "*** Error in `%s': %s: 0x%s ***\n") at ../sysdeps/posix/libc_fatal.c:175
#3  0x00007fca72c5c007 in malloc_printerr (action=<optimized out>, str=0x7fca72d63270 "munmap_chunk(): invalid pointer", ptr=<optimized out>) at malloc.c:4996
#4  0x00007fca735a1533 in deallocate (this=0x7fff8b971c20, __p=0x4af0960) at /usr/include/c++/4.8/ext/new_allocator.h:110
#5  _M_put_node (this=0x7fff8b971c20, __p=0x4af0960) at /usr/include/c++/4.8/bits/stl_list.h:338
#6  std::_List_base<DLString, std::allocator<DLString> >::_M_clear (this=0x7fff8b971c20) at /usr/include/c++/4.8/bits/list.tcc:79
#7  0x00007fca6dac9792 in ~_List_base (this=0x7fff8b971c20, __in_chrg=<optimized out>) at /usr/include/c++/4.8/bits/stl_list.h:378
#8  ~list (this=0x7fff8b971c20, __in_chrg=<optimized out>) at /usr/include/c++/4.8/bits/stl_list.h:438
#9  show_list_to_char (list=<optimized out>, ch=ch@entry=0x7fca5c001940, fShort=fShort@entry=true, fShowNothing=fShowNothing@entry=true, pocket=..., 
    container=container@entry=0xad5d4f0) at ../../../dreamland_code/plug-ins/anatolia/act_look.cpp:291
#10 0x00007fca6dacb533 in oprog_examine_container (pocket=..., ch=0x7fca5c001940, obj=0xad5d4f0) at ../../../dreamland_code/plug-ins/anatolia/act_look.cpp:1603
#11 oprog_examine (obj=0xad5d4f0, ch=ch@entry=0x7fca5c001940, arg=...) at ../../../dreamland_code/plug-ins/anatolia/act_look.cpp:1638
#12 0x00007fca6dacc3c8 in do_look_into (arg2=0x7fff8b973580 "pit", ch=0x7fca5c001940) at ../../../dreamland_code/plug-ins/anatolia/act_look.cpp:1205
#13 CommandTemplate<dummyCmd_look_TypeName>::run (this=<optimized out>, ch=0x7fca5c001940, argument=<optimized out>) at ../../../dreamland_code/plug-ins/anatolia/act_look.cpp:1413
```
## Команды
> printf "%s", ch->desc->outbuf
> x/gx 0x0000000004af0960 -0x40 - содержимое outbuf

outtop 4009
outsize 4000
noiac=false notelnet=false

Local crash after outputting 10000 "я" to cp1251 player with notelnet=no, noiac=no.
```bash
*** Error in `./bin/dreamland': free(): invalid next size (normal): 0x0000000002cbbef0 ***
======= Backtrace: =========
/lib/x86_64-linux-gnu/libc.so.6(+0x777e5)[0x7f26181697e5]
/lib/x86_64-linux-gnu/libc.so.6(+0x8037a)[0x7f261817237a]
/lib/x86_64-linux-gnu/libc.so.6(cfree+0x4c)[0x7f261817653c]
./libexec/plugins/libiomanager.so(_ZN20DefaultBufferHandler5writeEP10DescriptorPKc+0x116)[0x7f2615650f26]
./libexec/plugins/libiomanager.so(_ZN16InterpretHandler6promptEP10Descriptor+0x9f)[0x7f261564954f]
./libexec/plugins/libiomanager.so(_Z14process_outputP10Descriptorb+0x44)[0x7f2615640e94]
./libexec/plugins/libiomanager.so(_ZN9IOManager7ioWriteEv+0x79)[0x7f261563fe59]
/home/margo/cpp/dreamland/tmp/DL/lib/libdreamland.so.0(_ZN13SchedulerList4tickEv+0x42)[0x7f2618b32cd2]
/home/margo/cpp/dreamland/tmp/DL/lib/libdreamland.so.0(_ZN20SchedulerPriorityMap4tickEv+0x27)[0x7f2618b332e7]
/home/margo/cpp/dreamland/tmp/DL/lib/libdreamland.so.0(_ZN9Scheduler4tickEv+0x8e)[0x7f2618b3269e]
/home/margo/cpp/dreamland/tmp/DL/lib/libdreamland_impl.so.0(_ZN9DreamLand3runEv+0x15a)[0x7f2618e1464a]
./bin/dreamland[0x400f27]
/lib/x86_64-linux-gnu/libc.so.6(__libc_start_main+0xf0)[0x7f2618112830]
./bin/dreamland[0x401059]
```


