#!/bin/bash
# Plot seismic image
#

DIR=$(dirname $0)
# Capture the case when script is executed from current directory
LETTER=$(echo $DIR | cut -c 1)
if [ $LETTER == '.' ]; then
   DIR=$(pwd)/$DIR
fi

java -cp .:${DIR}/../lib/CSeisLib.jar:${DIR}/../lib/SeaView.jar -Xmx1524m -Djava.library.path=${DIR}/../lib cseis/seaview/PlotImage $*

