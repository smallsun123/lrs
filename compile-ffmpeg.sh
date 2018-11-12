#!/bin/sh

EXTRA_DIR=/opt/contrib
INSTALL_DIR=$EXTRA_DIR/ffmpeg
export PATH="$INSTALL_DIR/bin:$PATH" PKG_CONFIG_PATH="$INSTALL_DIR/lib/pkgconfig" 
./configure \
  --prefix="$INSTALL_DIR" \
  --pkg-config-flags="--static" \
  --extra-cflags="-I$EXTRA_DIR/x264/include" \
  --extra-cflags="-I$EXTRA_DIR/fdk_aac/include" \
  --extra-cflags="-I$EXTRA_DIR/freetype/include" \
  --extra-ldflags="-L$EXTRA_DIR/x264/lib" \
  --extra-ldflags="-L$EXTRA_DIR/fdk_aac/lib" \
  --extra-ldflags="-L$EXTRA_DIR/freetype/lib" \
  --enable-static \
  --disable-shared \
  --enable-version3 \
  --enable-nonfree \
  --enable-runtime-cpudetect \
  --enable-gpl \
  --enable-libfdk-aac \
  --enable-libfreetype \
  --enable-libx264 \
  --enable-libass \
  --disable-ffserver \
  --disable-doc \
  --disable-htmlpages \
  --disable-manpages \
  --disable-podpages \
  --disable-txtpages \
#  && make && make install
