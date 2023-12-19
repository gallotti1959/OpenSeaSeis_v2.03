echo on
set YOUR_PATH=%~dp$PATH:0
set MY_PATH=%~dp0

cd /d %~dp0

IF NOT "%YOUR_PATH%" == "" GOTO startWithYourPath
IF NOT "%MY_PATH%" == "" GOTO startWithMyPath

:startWithoutPath
java -cp SeaView.jar;CSeisLib.jar -Xmx1024m cseis/seaview/SeaView "%~1"
goto end

:startWithYourPath
java -cp "%YOUR_PATH%\SeaView.jar";"%YOUR_PATH%\CSeisLib.jar" -Xmx1024m -Djava.library.path=%YOUR_PATH% cseis/seaview/SeaView "%~1"
goto end

:startWithMyPath
java -cp "%MY_PATH%\SeaView.jar";"%MY_PATH%\CSeisLib.jar" -Xmx1024m -Djava.library.path=%MY_PATH% cseis/seaview/SeaView "%~1"


:end
