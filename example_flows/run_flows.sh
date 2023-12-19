#!/bin/bash

if [ ! -d data ]; then
  mkdir data
fi
if [ ! -d logs ]; then
  mkdir logs
fi
if [ ! -d images ]; then
  mkdir images
fi

flow1=t01_create_pseudo_data.flow
flow2=t02_create_image.flow
flow3=t03_create_image_spec.flow
flow4=t04_create_synthetic_data.flow

counter=$(echo "1+0" | bc -l)
for flow in `echo "$flow1 $flow2 $flow3 $flow4"`
do
  echo ""
  echo "($counter) Run processing flow $flow"
  echo "    seaseis -f $flow -d logs"
  seaseis -f $flow -d logs
  ret=$?

  if [ $ret -ne 0 ]; then
    echo "    ..flow $flow terminated with ERRORS."
    exit -1
  else
     echo "    SUCCESSFUL COMPLETION"
  fi
  counter=$(echo "$counter+1" | bc -l)
done

#----------------------------------------------------------

echo ""
echo "Start SeaView (seaview.sh)..."
seaview.sh data/t01_pseudo_data.cseis data/t01_pseudo_data_spectrum.cseis data/t04_syn_data.cseis data/t04_syn_data_spectrum.cseis &

# Display images
# eog images/*.png > /dev/null 2>&1 &
