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

@echo Downloading Morlock bootstrap source...
@curl -fsSL https://raw.githubusercontent.com/AbePralle/Morlock/main/Source/Bootstrap/Morlock.h -o "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\Morlock.h"
@if errorlevel 1 goto EXIT
@curl -fsSL https://raw.githubusercontent.com/AbePralle/Morlock/main/Source/Bootstrap/Morlock.c -o "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\Morlock.c"
@if errorlevel 1 goto EXIT

@cl /EHsc /nologo "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\Morlock.c" /Fo"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.obj" /Fe"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.exe"

"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock-v2.exe" bootstrap --installer="%0"

:EXIT
