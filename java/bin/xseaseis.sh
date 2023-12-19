#!/bin/bash
# Launch Job Builder
#

DIR=$(dirname $0)
# Capture the case when script is executed from current directory
LETTER=$(echo $DIR | cut -c 1)
if [ $LETTER == '.' ]; then
   DIR=$(pwd)/$DIR
fi
export LD_LIBRARY_PATH=$DIR/../lib:$LD_LIBRARY_PATH

java -cp .:${DIR}/../lib/CSeisLib.jar:${DIR}/../lib/XCSeis.jar:${DIR}/../lib/SeaView.jar -Xmx1524m -Djava.library.path=${DIR}/../lib cseis/xcseis/XCSeis  $*
