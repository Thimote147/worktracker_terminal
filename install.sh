#!/bin/bash

# Work Tracker Installation Script
# This script will install the work tracker to ~/.local/bin and add it to your PATH

set -e  # Exit on any error

echo "╔════════════════════════════════════════╗"
echo "║   Work Tracker Installation            ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to install build tools
install_build_tools() {
    local os_type="$1"

    echo -e "${BLUE}Installing build tools...${NC}"

    case "$os_type" in
        "debian"|"ubuntu")
            echo -e "${YELLOW}Detected Debian/Ubuntu. Installing build-essential...${NC}"
            sudo apt-get update
            sudo apt-get install -y build-essential
            ;;
        "fedora"|"rhel"|"centos")
            echo -e "${YELLOW}Detected RHEL/Fedora/CentOS. Installing Development Tools...${NC}"
            sudo yum groupinstall -y "Development Tools"
            ;;
        "arch")
            echo -e "${YELLOW}Detected Arch Linux. Installing base-devel...${NC}"
            sudo pacman -S --noconfirm base-devel
            ;;
        "alpine")
            echo -e "${YELLOW}Detected Alpine Linux. Installing build-base...${NC}"
            sudo apk add --no-cache build-base
            ;;
        "macos")
            echo -e "${YELLOW}Detected macOS. Installing Xcode Command Line Tools...${NC}"
            xcode-select --install
            echo -e "${YELLOW}Please complete the Xcode installation dialog, then run this script again.${NC}"
            exit 0
            ;;
        *)
            echo -e "${RED}Unable to auto-install for this OS.${NC}"
            echo "Please install gcc and make manually, then run this script again."
            exit 1
            ;;
    esac

    echo -e "${GREEN}✓ Build tools installed${NC}"
}

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ -f /etc/os-release ]]; then
        . /etc/os-release
        case "$ID" in
            debian|ubuntu|linuxmint|pop)
                echo "debian"
                ;;
            fedora|rhel|centos|rocky|almalinux)
                echo "fedora"
                ;;
            arch|manjaro)
                echo "arch"
                ;;
            alpine)
                echo "alpine"
                ;;
            *)
                echo "unknown"
                ;;
        esac
    else
        echo "unknown"
    fi
}

# Check if gcc is installed
if ! command -v gcc &> /dev/null; then
    echo -e "${YELLOW}gcc is not installed${NC}"
    OS_TYPE=$(detect_os)

    if [[ "$OS_TYPE" == "unknown" ]]; then
        echo -e "${RED}Error: Unable to detect your operating system${NC}"
        echo "Please install build tools manually:"
        echo "  Ubuntu/Debian: sudo apt install build-essential"
        echo "  Fedora/RHEL:   sudo yum groupinstall 'Development Tools'"
        echo "  Arch Linux:    sudo pacman -S base-devel"
        echo "  macOS:         xcode-select --install"
        exit 1
    fi

    echo -e "${BLUE}Would you like to install build tools now? (y/n)${NC}"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        install_build_tools "$OS_TYPE"
    else
        echo -e "${RED}Installation aborted. Please install gcc and make first.${NC}"
        exit 1
    fi
fi

# Check if make is installed
if ! command -v make &> /dev/null; then
    echo -e "${YELLOW}make is not installed${NC}"
    OS_TYPE=$(detect_os)

    if [[ "$OS_TYPE" == "unknown" ]]; then
        echo -e "${RED}Error: Unable to detect your operating system${NC}"
        echo "Please install build tools manually:"
        echo "  Ubuntu/Debian: sudo apt install build-essential"
        echo "  Fedora/RHEL:   sudo yum groupinstall 'Development Tools'"
        echo "  Arch Linux:    sudo pacman -S base-devel"
        echo "  macOS:         xcode-select --install"
        exit 1
    fi

    echo -e "${BLUE}Would you like to install build tools now? (y/n)${NC}"
    read -r response
    if [[ "$response" =~ ^[Yy]$ ]]; then
        install_build_tools "$OS_TYPE"
    else
        echo -e "${RED}Installation aborted. Please install gcc and make first.${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}✓ Build tools are installed${NC}"

# Create ~/.local/bin if it doesn't exist
BIN_DIR="$HOME/.local/bin"
if [ ! -d "$BIN_DIR" ]; then
    echo -e "${YELLOW}Creating $BIN_DIR...${NC}"
    mkdir -p "$BIN_DIR"
fi

# Get the script's directory (where the source code is)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo -e "${YELLOW}Compiling worktracker...${NC}"
cd "$SCRIPT_DIR"

# Clean any previous builds
make clean 2>/dev/null || true

# Compile the program
if make; then
    echo -e "${GREEN}✓ Compilation successful${NC}"
else
    echo -e "${RED}✗ Compilation failed${NC}"
    exit 1
fi

# Copy the compiled binary to ~/.local/bin
echo -e "${YELLOW}Installing to $BIN_DIR...${NC}"
cp worktracker "$BIN_DIR/"
chmod +x "$BIN_DIR/worktracker"
echo -e "${GREEN}✓ Installed to $BIN_DIR/worktracker${NC}"

# Function to add PATH to a shell config file
add_to_path() {
    local config_file="$1"
    local shell_name="$2"

    if [ -f "$config_file" ]; then
        # Check if PATH is already configured
        if grep -q "export PATH=\"\$HOME/.local/bin:\$PATH\"" "$config_file" || grep -q 'export PATH="$HOME/.local/bin:$PATH"' "$config_file"; then
            echo -e "${GREEN}✓ PATH already configured in $config_file${NC}"
            return 0
        fi

        # Add PATH configuration
        echo -e "${YELLOW}Adding $BIN_DIR to PATH in $config_file...${NC}"
        echo "" >> "$config_file"
        echo "# Added by worktracker installer" >> "$config_file"
        echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" >> "$config_file"
        echo -e "${GREEN}✓ Updated $config_file${NC}"
        return 1
    fi
    return 2
}

# Detect shell and update appropriate config file
SHELL_UPDATED=false
SHELL_CONFIG=""

# Determine which shell is ACTUALLY running (not just $SHELL variable)
# Check parent process to see what shell launched this script
ACTUAL_SHELL="unknown"
if [ -n "$ZSH_VERSION" ]; then
    ACTUAL_SHELL="zsh"
elif [ -n "$BASH_VERSION" ]; then
    # Could be bash, but user might have launched from zsh
    # Check the parent process
    PARENT_CMD=$(ps -p $PPID -o comm= 2>/dev/null)
    if [[ "$PARENT_CMD" == *"zsh"* ]]; then
        ACTUAL_SHELL="zsh"
    else
        ACTUAL_SHELL="bash"
    fi
else
    # Not running in a versioned shell, check parent
    PARENT_CMD=$(ps -p $PPID -o comm= 2>/dev/null)
    if [[ "$PARENT_CMD" == *"zsh"* ]]; then
        ACTUAL_SHELL="zsh"
    elif [[ "$PARENT_CMD" == *"bash"* ]]; then
        ACTUAL_SHELL="bash"
    else
        # Fallback to $SHELL variable
        ACTUAL_SHELL=$(basename "$SHELL" 2>/dev/null || echo "unknown")
    fi
fi

echo -e "${YELLOW}Detected shell: $ACTUAL_SHELL${NC}"

# Only update the PRIMARY shell config (user's actual shell)
if [[ "$ACTUAL_SHELL" == "zsh" ]]; then
    # User's primary shell is zsh
    if [ -f "$HOME/.zshrc" ]; then
        if add_to_path "$HOME/.zshrc" "zsh"; then
            :  # Already configured
        else
            SHELL_UPDATED=true
            SHELL_CONFIG="$HOME/.zshrc"
        fi
    else
        # Create .zshrc if it doesn't exist
        touch "$HOME/.zshrc"
        add_to_path "$HOME/.zshrc" "zsh"
        SHELL_UPDATED=true
        SHELL_CONFIG="$HOME/.zshrc"
    fi
elif [[ "$ACTUAL_SHELL" == "bash" ]]; then
    # User's primary shell is bash
    if [ -f "$HOME/.bashrc" ]; then
        if add_to_path "$HOME/.bashrc" "bash"; then
            :  # Already configured
        else
            SHELL_UPDATED=true
            SHELL_CONFIG="$HOME/.bashrc"
        fi
    else
        # Create .bashrc if it doesn't exist
        touch "$HOME/.bashrc"
        add_to_path "$HOME/.bashrc" "bash"
        SHELL_UPDATED=true
        SHELL_CONFIG="$HOME/.bashrc"
    fi
else
    # Unknown shell - try to detect from config files
    if [ -f "$HOME/.zshrc" ]; then
        add_to_path "$HOME/.zshrc" "zsh"
        SHELL_UPDATED=true
        SHELL_CONFIG="$HOME/.zshrc"
    elif [ -f "$HOME/.bashrc" ]; then
        add_to_path "$HOME/.bashrc" "bash"
        SHELL_UPDATED=true
        SHELL_CONFIG="$HOME/.bashrc"
    fi
fi

# Update current session PATH
export PATH="$HOME/.local/bin:$PATH"

echo ""
echo "╔════════════════════════════════════════╗"
echo "║   Installation Complete! ✓             ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo -e "${GREEN}✓ worktracker installed to: $BIN_DIR/worktracker${NC}"

if [ "$SHELL_UPDATED" = true ]; then
    echo -e "${GREEN}✓ PATH configured in: $SHELL_CONFIG${NC}"
    echo ""
    echo -e "${YELLOW}To activate worktracker in THIS terminal, run:${NC}"
    echo -e "${GREEN}source $SHELL_CONFIG${NC}"
else
    echo -e "${GREEN}✓ PATH already configured${NC}"
fi

echo ""
