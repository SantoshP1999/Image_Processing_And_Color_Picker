cls

del *.obj

del *.dll

del *.exp

del *.lib

cl.exe /c /EHsc ImageEffectTwo.cpp

link.exe ImageEffectTwo.obj /DLL /DEF:ImageEffectTwo.def user32.lib ole32.lib /SUBSYSTEM:WINDOWS
