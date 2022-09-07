@if exist "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock" goto CREATED_HOME
@echo --------------------------------------------------------------------------------
@echo Installing Morlock
@echo --------------------------------------------------------------------------------
@echo Creating home folder: %HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock
@mkdir "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock"
:CREATED_HOME

@if exist "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock" goto CREATED_BUILD
@mkdir "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock"
:CREATED_BUILD

@if exist "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\download-v2.success" goto DOWNLOADED_EXE
@echo Downloading Morlock bootstrap source...
@curl -fsSL https://github.com/AbePralle/Morlock/main/Source/Bootstrap/Morlock.h -o "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\Morlock.h"
@curl -fsSL https://github.com/AbePralle/Morlock/main/Source/Bootstrap/Morlock.c -o "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\Morlock.c"
@echo Success > "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\download-v2.success"
:DOWNLOADED_EXE

@if exist "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.exe" goto COMPILED
@cl /EHsc /nologo Build\RogueC-Windows.c /Fo"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.obj" /Fe"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.exe"
:COMPILED

"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.exe" bootstrap --installer="%0"

