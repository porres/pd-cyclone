#!/bin/sh
#

NEWDIR=../newHelp
SEDFILE=../shed/updateVersionHelpPatch.sed

test -e $SEDFILE || exit

pushd help

for FILE in `ls *-help.pd`
do
    echo sed -f $SEDFILE $FILE $NEWDIR/$FILE
    sed -f $SEDFILE $FILE > $NEWDIR/$FILE
done

popd