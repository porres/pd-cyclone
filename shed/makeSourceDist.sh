#!/bin/bash
#
CURRENTDIR=`pwd`
CURRENTPROJECT=`basename $CURRENTDIR`

pushd .. || exit

tar cvzf ${CURRENTPROJECT}/${CURRENTPROJECT}_sources.tgz \
 ${CURRENTPROJECT}/build_counter \
 ${CURRENTPROJECT}/abstractions/ \
 ${CURRENTPROJECT}/cyclone-meta.pd \
 ${CURRENTPROJECT}/hammer/ \
 ${CURRENTPROJECT}/help/ \
 ${CURRENTPROJECT}/LICENSE.txt \
 ${CURRENTPROJECT}/Makefile \
 ${CURRENTPROJECT}/Makefile.pdlibbuilder \
 ${CURRENTPROJECT}/notes.txt \
 ${CURRENTPROJECT}/original_README.txt \
 ${CURRENTPROJECT}/README.md \
 ${CURRENTPROJECT}/shadow/ \
 ${CURRENTPROJECT}/shared/ \
 ${CURRENTPROJECT}/sickle/ \
 ${CURRENTPROJECT}/test/ \
 ${CURRENTPROJECT}/shed

popd
