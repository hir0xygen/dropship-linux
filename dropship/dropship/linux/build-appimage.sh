#!/bin/bash
# AppImage build script for Dropship Linux

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_DIR/build-appimage"
APPDIR="$BUILD_DIR/AppDir"

echo "=== Dropship AppImage Builder ==="
echo "Project directory: $PROJECT_DIR"

# Clean and create build directory
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Build the project
echo "=== Building Dropship ==="
cmake "$PROJECT_DIR" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
make -j$(nproc)

# Create AppDir structure
echo "=== Creating AppDir ==="
mkdir -p "$APPDIR/usr/bin"
mkdir -p "$APPDIR/usr/share/dropship/fonts"
mkdir -p "$APPDIR/usr/share/applications"
mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
mkdir -p "$APPDIR/usr/share/polkit-1/actions"

# Copy binary
cp dropship "$APPDIR/usr/bin/"

# Copy desktop file
cp "$PROJECT_DIR/linux/dropship.desktop" "$APPDIR/usr/share/applications/"
cp "$PROJECT_DIR/linux/dropship.desktop" "$APPDIR/"

# Copy polkit policy (for reference - user needs to install separately)
cp "$PROJECT_DIR/linux/org.dropship.policy" "$APPDIR/usr/share/polkit-1/actions/"

# Copy icon (TODO: create proper icon)
# For now, create a placeholder
if [ ! -f "$PROJECT_DIR/linux/dropship.png" ]; then
    echo "Warning: No icon found at $PROJECT_DIR/linux/dropship.png"
    echo "Creating placeholder icon..."
    # Create a simple placeholder using ImageMagick if available
    if command -v convert &> /dev/null; then
        convert -size 256x256 xc:#2d5aa0 -fill white -gravity center \
            -pointsize 72 -annotate 0 "DS" "$APPDIR/usr/share/icons/hicolor/256x256/apps/dropship.png"
    fi
fi

if [ -f "$PROJECT_DIR/linux/dropship.png" ]; then
    cp "$PROJECT_DIR/linux/dropship.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/"
    cp "$PROJECT_DIR/linux/dropship.png" "$APPDIR/dropship.png"
fi

# Copy resources
if [ -f "$PROJECT_DIR/../servers.json" ]; then
    cp "$PROJECT_DIR/../servers.json" "$APPDIR/usr/share/dropship/"
fi

# Copy fonts (TODO: bundle actual fonts)
# For now, create a README
cat > "$APPDIR/usr/share/dropship/fonts/README.txt" << 'EOF'
Font files should be placed here:
- Roboto-Regular.ttf
- Industry-Bold.otf (optional)
- Industry-Medium.otf (optional)

The application will use system fonts if these are not found.
EOF

# Create AppRun
cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin/:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib/:${LD_LIBRARY_PATH}"
export XDG_DATA_DIRS="${HERE}/usr/share/:${XDG_DATA_DIRS}"

# Check if we need root privileges
if [ "$EUID" -ne 0 ]; then
    echo "Dropship requires root privileges for firewall management."
    echo "Attempting to restart with pkexec..."
    exec pkexec "${HERE}/usr/bin/dropship" "$@"
fi

exec "${HERE}/usr/bin/dropship" "$@"
EOF
chmod +x "$APPDIR/AppRun"

# Download linuxdeploy if not present
if [ ! -f "$BUILD_DIR/linuxdeploy-x86_64.AppImage" ]; then
    echo "=== Downloading linuxdeploy ==="
    wget -q "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage" \
        -O "$BUILD_DIR/linuxdeploy-x86_64.AppImage"
    chmod +x "$BUILD_DIR/linuxdeploy-x86_64.AppImage"
fi

# Build AppImage
echo "=== Building AppImage ==="
export VERSION="1.0.0"
"$BUILD_DIR/linuxdeploy-x86_64.AppImage" --appdir "$APPDIR" --output appimage

echo "=== Done ==="
echo "AppImage created: $BUILD_DIR/Dropship-*.AppImage"
