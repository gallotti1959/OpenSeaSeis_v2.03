@echo off
set YOUR_PATH=%~dp$PATH:0
java -cp %YOUR_PATH%\SeaView.jar;%YOUR_PATH%\CSeisLib.jar -Xmx1524m -Djava.library.path=%YOUR_PATH% cseis/seaview/PlotImage %~1
