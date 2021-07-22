# PKGBUILD for AMSynth
# For test purpose, using CMake instead of Autotools
#
# NOTE: VST and LV2 only. DSSI and desktop configs are not implemented.

pkgname=amsynth-userbuild
pkgver=1.12.2
pkgrel=1
pkgdesc="Analogule Modeling SYNTHesizer (User build edition)"
arch=('i686' 'x86_64')
url="https://amsynth.github.io/"
license=('GPL2')
groups=('lv2-plugins' 'pro-audio' 'vst-plugins')
depends=('cairo' 'gcc-libs' 'glibc')
makedepends=('cmake' 'atk' 'fontconfig' 'freetype2'
'gdk-pixbuf2' 'glib2' 'gtk2' 'harfbuzz' 'intltool' 'jack' 'ladspa' 'liblo'
'lv2' 'pandoc')
#checkdepends=('lv2lint')
optdepends=('alsa-lib: for standalone application'
            'dssi-host: for DSSI plugin'
            'jack: for standalone application'
            'liblo: for standalone application and DSSI plugin'
            'new-session-manager: for session management with the standalone application'
            'lv2-host: for LV2 plugin'
            'vst-host: for VST plugin')
conflicts=('amsynth')


source_root="$PWD"

prepare() {
    cd $source_root
    mkdir -p tmp
    cmake -S . -B tmp/build-PKGBUILD -DCMAKE_INSTALL_PREFIX=/usr
}

build() {
    cd $source_root
    cd tmp/build-PKGBUILD
    make -j4
}

package() {
    cd $source_root/tmp/build-PKGBUILD
    make DESTDIR="$pkgdir/" install
    chmod +x "$pkgdir/usr/bin/amsynth"
}
