cls

cl /EHsc /c VK.c

rc VK.rc

link VK.obj VK.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

VK.exe
