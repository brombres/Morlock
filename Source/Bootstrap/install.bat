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

@if exist "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\success.txt" goto DOWNLOADED_EXE
@echo Downloading Morlock bootstrap executable...
@curl -fsSL https://github.com/AbePralle/Morlock/releases/download/v0.0.4/morlock.exe -o "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock.exe"
@echo Success > "%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\success.txt"
:DOWNLOADED_EXE

"%HOMEDRIVE%%HOMEPATH%\AppData\Local\Morlock\build\abepralle\morlock\morlock.exe" bootstrap --installer="%0"

