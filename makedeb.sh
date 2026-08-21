#!/usr/bin/env bash
set -euo pipefail

project_root=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
version=$(tr -d '[:space:]' < "$project_root/VERSION")
output_dir="$project_root/dist"
output="$output_dir/livetikz_${version}_amd64.deb"

if [[ ! "$version" =~ ^[0-9]+(\.[0-9]+)*([+-][0-9A-Za-z.-]+)?$ ]]; then
    printf 'Invalid Debian package version: %s\n' "$version" >&2
    exit 1
fi

# Use fakeroot automatically for normal users so the archive has conventional
# root ownership without requiring the caller to run the script as root.
if [[ "${LIVETIKZ_DEB_FAKEROOT:-0}" != 1 && "$(id -u)" != 0 ]]; then
    if ! command -v fakeroot >/dev/null 2>&1; then
        printf 'fakeroot is required when building as a non-root user.\n' >&2
        exit 1
    fi
    exec env LIVETIKZ_DEB_FAKEROOT=1 fakeroot -- "$0" "$@"
fi

binary="${LIVETIKZ_BINARY:-}"
if [[ -z "$binary" ]]; then
    for candidate in "$project_root/build/livetikz" "$project_root/livetikz"; do
        if [[ -x "$candidate" ]]; then
            binary="$candidate"
            break
        fi
    done
fi
if [[ -z "$binary" || ! -x "$binary" ]]; then
    printf 'Could not find the built executable. Build LiveTikZ first, or set LIVETIKZ_BINARY.\n' >&2
    exit 1
fi

command -v dpkg-deb >/dev/null 2>&1 || {
    printf 'dpkg-deb is required to build the package.\n' >&2
    exit 1
}

mkdir -p "$output_dir"
staging=$(mktemp -d "${TMPDIR:-/tmp}/livetikz-deb.XXXXXX")
trap 'rm -rf "$staging"' EXIT
package_root="$staging/livetikz"

mkdir -p \
    "$package_root/DEBIAN" \
    "$package_root/usr/bin" \
    "$package_root/usr/share/applications" \
    "$package_root/usr/share/doc/livetikz" \
    "$package_root/usr/share/icons/hicolor/256x256/apps" \
    "$package_root/usr/share/livetikz/templates" \
    "$package_root/usr/share/man/man1"

install -m 0755 "$binary" "$package_root/usr/bin/livetikz"
strip "$package_root/usr/bin/livetikz" 2>/dev/null || true
sed "s/%VERSION%/$version/" "$project_root/docs/debian-control" > "$package_root/DEBIAN/control"

cat > "$package_root/usr/share/doc/livetikz/changelog" <<EOF
livetikz ($version) unstable; urgency=low

  * LiveTikZ release $version.

 -- Michael Schwarz <michael.schwarz91@gmail.com>  $(date -R)
EOF
printf 'Copyright 2026, Michael Schwarz\n' > "$package_root/usr/share/doc/livetikz/copyright"
gzip -c -9 -n "$package_root/usr/share/doc/livetikz/changelog" > "$package_root/usr/share/doc/livetikz/changelog.gz"
rm "$package_root/usr/share/doc/livetikz/changelog"
gzip -c -9 -n "$project_root/docs/livetikz.1" > "$package_root/usr/share/man/man1/livetikz.1.gz"

install -m 0644 "$project_root/data/livetikz.desktop" "$package_root/usr/share/applications/"
install -m 0644 "$project_root/data/livetikz.png" "$package_root/usr/share/icons/hicolor/256x256/apps/"
install -m 0644 "$project_root/data/templates/"* "$package_root/usr/share/livetikz/templates/"
install -m 0755 "$project_root/data/latexrun" "$package_root/usr/share/livetikz/latexrun"
install -m 0644 "$project_root/data/latexrun.LICENSE" "$package_root/usr/share/doc/livetikz/latexrun.LICENSE"

# Keep archive permissions deterministic regardless of the caller's umask.
find "$package_root" -type d -exec chmod 0755 {} +
find "$package_root" -type f -exec chmod 0644 {} +
chmod 0755 "$package_root/usr/bin/livetikz"
chown -R root:root "$package_root"
dpkg-deb --build "$package_root" "$output" >/dev/null

printf 'Created %s\n' "$output"
if command -v lintian >/dev/null 2>&1; then
    lintian "$output" || true
fi
