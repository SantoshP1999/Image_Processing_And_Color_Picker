cls

del *.obj

del *.dll

del *.exp

del *.lib

cl.exe /c /EHsc ImageEffectOne.cpp

link.exe ImageEffectOne.obj /DLL /DEF:ImageEffectOne.def user32.lib /SUBSYSTEM:WINDOWS
