#!/bin/sh

RELEASETYPE=$1

ls

if [ "x$RELEASETYPE" = "x" ]; then
    echo "Usage: $0 [linux|windows]"
    exit 1
fi

if [ "$RELEASETYPE" = "linux" ]; then
    RELEASE_BIN="cutecw"
elif [ "$RELEASETYPE" = "windows" ]; then
    RELEASE_BIN="cutecw.exe"
else
    RELEASE_BIN="cutecw"
fi

RELEASE_DIR="cutecw.${RELEASETYPE}.${DRONE_BUILD_NUMBER}"

mkdir -p $RELEASE_DIR
cp ${RELEASE_BIN} $RELEASE_DIR/
cp -r books $RELEASE_DIR/
cp -r icons $RELEASE_DIR/
cp LICENSE $RELEASE_DIR/LICENSE.txt
cp *.qm $RELEASE_DIR/
cp cutecw.cfg.sample $RELEASE_DIR/cutecw.cfg

echo "Build infos" >> $RELEASE_DIR/BUILD.txt
if [ "$RELEASETYPE" = "windows" ]; then
    echo "Built with MXE and Qt5" >> $RELEASE_DIR/BUILD.txt
fi
echo "CI Commit SHA: $DRONE_COMMIT_SHA" >> $RELEASE_DIR/BUILD.txt
echo "   Job number: $DRONE_BUILD_NUMBER" >> $RELEASE_DIR/BUILD.txt
echo "     Built on: `date` " >> $RELEASE_DIR/BUILD.txt

echo -n "Binary sha256: " >> $RELEASE_DIR/BUILD.txt
sha256sum $RELEASE_DIR/${RELEASE_BIN} >> $RELEASE_DIR/BUILD.txt

zip -r "${RELEASE_DIR}.zip" ${RELEASE_DIR}/

sha256sum "${RELEASE_DIR}.zip" >> "${RELEASE_DIR}.zip.sha256"

echo "Generated ${RELEASE_DIR}.zip"