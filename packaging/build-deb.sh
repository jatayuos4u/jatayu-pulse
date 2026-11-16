#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
for tool in cmake ninja dpkg-deb dpkg-shlibdeps dpkg-architecture; do
  command -v "$tool" >/dev/null || { echo "Missing $tool" >&2; exit 1; }
done
# Provide a valid project contact via env, not an invented address.
: "${JATAYU_MAINTAINER_EMAIL:?Set JATAYU_MAINTAINER_EMAIL to your public project contact email}"
if [[ ! "$JATAYU_MAINTAINER_EMAIL" =~ ^[^[:space:]@]+@[^[:space:]@]+\.[^[:space:]@]+$ ]]; then
 echo 'Invalid JATAYU_MAINTAINER_EMAIL' >&2; exit 1
fi
arch="$(dpkg-architecture -qDEB_HOST_ARCH)"
version=1.0.1-1
builddir="$(mktemp -d)"
trap 'rm -rf "$builddir"' EXIT
cmake -S . -B "$builddir/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DCMAKE_INSTALL_PREFIX=/usr
cmake --build "$builddir/build" --parallel 2
ctest --test-dir "$builddir/build" --output-on-failure
DESTDIR="$builddir/root" cmake --install "$builddir/build"
# Standard Debian package permissions: directories 0755, ordinary files 0644.
# Keep the executable and maintainer scripts executable.
find "$builddir/root" -type d -exec chmod 0755 {} +
find "$builddir/root" -type f -exec chmod 0644 {} +
chmod 0755 "$builddir/root/usr/bin/jatayu-system-monitor"
mkdir -p "$builddir/root/DEBIAN"
# dpkg-shlibdeps expects a debian/control in the build context; generate minimal metadata.
mkdir -p "$builddir/debian"
cat > "$builddir/debian/control" <<EOF
Source: jatayu-system-monitor
Section: utils
Priority: optional
Maintainer: Anup Kumar Yadav <$JATAYU_MAINTAINER_EMAIL>
Standards-Version: 4.7.0

Package: jatayu-system-monitor
Architecture: any
Description: Jatayu Pulse system monitor
EOF
(
 cd "$builddir"
 dpkg-shlibdeps -O -e"root/usr/bin/jatayu-system-monitor" > substvars
)
deps="$(sed -n 's/^shlibs:Depends=//p' "$builddir/substvars")"
[[ -n "$deps" ]] || { echo 'Unable to determine runtime dependencies' >&2; exit 1; }
cat > "$builddir/root/DEBIAN/control" <<EOF
Package: jatayu-system-monitor
Version: $version
Section: utils
Priority: optional
Architecture: $arch
Maintainer: Anup Kumar Yadav <$JATAYU_MAINTAINER_EMAIL>
Depends: $deps, util-linux
Homepage: https://jatayuos.in
Description: Jatayu Pulse native system monitor for JatayuOS
 Read-only dashboard for CPU, RAM, thermal sensors, storage,
 processes, and network interfaces on Debian 13 and KDE Plasma 6.
EOF
# Ensure control directory is not group-writable either.
chmod 0755 "$builddir/root/DEBIAN"
out="$(pwd)/jatayu-system-monitor_${version}_${arch}.deb"
dpkg-deb --build --root-owner-group "$builddir/root" "$out"
dpkg-deb --info "$out"
dpkg-deb --contents "$out"
echo "Package ready: $out"
