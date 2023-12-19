echo off

mkdir data
mkdir logs
mkdir images

set flow1=t01_create_pseudo_data.flow
set flow2=t02_create_image.flow
set flow3=t03_create_image_spec.flow
set flow4=t04_create_synthetic_data.flow

echo ...
echo  Run processing flow %flow1%
echo     seaseis -f %flow1% -d logs
seaseis.exe -f %flow1% -d logs

echo ...
echo  Run processing flow %flow2%
echo     seaseis -f %flow2% -d logs
seaseis.exe -f %flow2% -d logs

echo ...
echo  Run processing flow %flow3%
echo     seaseis -f %flow3% -d logs
seaseis.exe -f %flow3% -d logs

echo ...
echo  Run processing flow %flow4%
echo     seaseis -f %flow4% -d logs
seaseis.exe -f %flow4% -d logs

echo ...
echo Start SeaView (seaview.bat)...
START /B CMD /C CALL seaview.bat "data/t01_pseudo_data.cseis data/t01_pseudo_data_spectrum.cseis data/t04_syn_data.cseis data/t04_syn_data_spectrum.cseis"

