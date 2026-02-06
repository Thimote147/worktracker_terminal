#!/bin/bash

# Work Tracker Uninstallation Script
# This script will remove worktracker from your system

echo "╔════════════════════════════════════════╗"
echo "║   Work Tracker Uninstallation         ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

BIN_DIR="$HOME/.local/bin"
DATA_DIR="$HOME/.local/bin"
WORKTRACKER_BIN="$BIN_DIR/worktracker"
DATA_FILE="$DATA_DIR/worktracker.dat"
TEMP_FILE="$DATA_DIR/temp_day.tmp"

echo -e "${YELLOW}This will remove worktracker from your system.${NC}"
echo ""

# Check if worktracker is installed
if [ ! -f "$WORKTRACKER_BIN" ]; then
    echo -e "${RED}worktracker is not installed at $WORKTRACKER_BIN${NC}"
    echo "Nothing to uninstall."
    exit 0
fi

echo "The following will be removed:"
echo -e "  ${RED}✗${NC} $WORKTRACKER_BIN"

# Check for data files
HAS_DATA=false
if [ -f "$DATA_FILE" ]; then
    HAS_DATA=true
    echo -e "  ${YELLOW}?${NC} $DATA_FILE (your work history data)"
fi

if [ -f "$TEMP_FILE" ]; then
    HAS_DATA=true
    echo -e "  ${YELLOW}?${NC} $TEMP_FILE (temporary day data)"
fi

# Check for PATH configuration
ZSHRC_HAS_PATH=false
BASHRC_HAS_PATH=false

if [ -f "$HOME/.zshrc" ]; then
    if grep -q "# Added by worktracker installer" "$HOME/.zshrc" || \
       grep -q 'export PATH="$HOME/.local/bin:$PATH"' "$HOME/.zshrc"; then
        ZSHRC_HAS_PATH=true
        echo -e "  ${YELLOW}?${NC} PATH configuration in ~/.zshrc"
    fi
fi

if [ -f "$HOME/.bashrc" ]; then
    if grep -q "# Added by worktracker installer" "$HOME/.bashrc" || \
       grep -q 'export PATH="$HOME/.local/bin:$PATH"' "$HOME/.bashrc"; then
        BASHRC_HAS_PATH=true
        echo -e "  ${YELLOW}?${NC} PATH configuration in ~/.bashrc"
    fi
fi

echo ""
echo -e "${BLUE}Do you want to continue? (y/n)${NC}"
read -r response

if [[ ! "$response" =~ ^[Yy]$ ]]; then
    echo -e "${YELLOW}Uninstallation cancelled.${NC}"
    exit 0
fi

# Remove the binary
echo ""
echo -e "${YELLOW}Removing worktracker binary...${NC}"
if rm -f "$WORKTRACKER_BIN"; then
    echo -e "${GREEN}✓ Removed $WORKTRACKER_BIN${NC}"
else
    echo -e "${RED}✗ Failed to remove $WORKTRACKER_BIN${NC}"
fi

# Ask about data files
if [ "$HAS_DATA" = true ]; then
    echo ""
    echo -e "${YELLOW}Do you want to remove your work tracking data?${NC}"
    echo -e "${RED}WARNING: This will delete all your recorded work hours!${NC}"
    echo -e "${BLUE}Remove data files? (y/n)${NC}"
    read -r data_response

    if [[ "$data_response" =~ ^[Yy]$ ]]; then
        if [ -f "$DATA_FILE" ]; then
            if rm -f "$DATA_FILE"; then
                echo -e "${GREEN}✓ Removed $DATA_FILE${NC}"
            else
                echo -e "${RED}✗ Failed to remove $DATA_FILE${NC}"
            fi
        fi

        if [ -f "$TEMP_FILE" ]; then
            if rm -f "$TEMP_FILE"; then
                echo -e "${GREEN}✓ Removed $TEMP_FILE${NC}"
            else
                echo -e "${RED}✗ Failed to remove $TEMP_FILE${NC}"
            fi
        fi
    else
        echo -e "${GREEN}✓ Data files preserved${NC}"
        echo -e "${YELLOW}Your data is still at:${NC}"
        [ -f "$DATA_FILE" ] && echo "  - $DATA_FILE"
        [ -f "$TEMP_FILE" ] && echo "  - $TEMP_FILE"
    fi
fi

# Remove PATH configuration
if [ "$ZSHRC_HAS_PATH" = true ] || [ "$BASHRC_HAS_PATH" = true ]; then
    echo ""
    echo -e "${YELLOW}Do you want to remove PATH configuration from shell config files?${NC}"
    echo -e "${BLUE}Remove PATH entries? (y/n)${NC}"
    read -r path_response

    if [[ "$path_response" =~ ^[Yy]$ ]]; then
        if [ "$ZSHRC_HAS_PATH" = true ]; then
            # Remove the worktracker installer lines from .zshrc
            if grep -q "# Added by worktracker installer" "$HOME/.zshrc"; then
                # Create a backup
                cp "$HOME/.zshrc" "$HOME/.zshrc.backup.$(date +%s)"
                # Remove the comment line and the next line (export PATH)
                sed -i.tmp '/# Added by worktracker installer/,+1d' "$HOME/.zshrc"
                rm -f "$HOME/.zshrc.tmp"
                echo -e "${GREEN}✓ Removed PATH configuration from ~/.zshrc${NC}"
                echo -e "${YELLOW}  Backup saved to ~/.zshrc.backup.*${NC}"
            fi
        fi

        if [ "$BASHRC_HAS_PATH" = true ]; then
            # Remove the worktracker installer lines from .bashrc
            if grep -q "# Added by worktracker installer" "$HOME/.bashrc"; then
                # Create a backup
                cp "$HOME/.bashrc" "$HOME/.bashrc.backup.$(date +%s)"
                # Remove the comment line and the next line (export PATH)
                sed -i.tmp '/# Added by worktracker installer/,+1d' "$HOME/.bashrc"
                rm -f "$HOME/.bashrc.tmp"
                echo -e "${GREEN}✓ Removed PATH configuration from ~/.bashrc${NC}"
                echo -e "${YELLOW}  Backup saved to ~/.bashrc.backup.*${NC}"
            fi
        fi

        echo ""
        echo -e "${YELLOW}To apply changes to current terminal, run:${NC}"
        if [ "$ZSHRC_HAS_PATH" = true ]; then
            echo -e "  ${GREEN}source ~/.zshrc${NC}"
        fi
        if [ "$BASHRC_HAS_PATH" = true ]; then
            echo -e "  ${GREEN}source ~/.bashrc${NC}"
        fi
    else
        echo -e "${GREEN}✓ PATH configuration preserved${NC}"
    fi
fi

echo ""
echo "╔════════════════════════════════════════╗"
echo "║   Uninstallation Complete! ✓           ║"
echo "╚════════════════════════════════════════╝"
echo ""
echo -e "${GREEN}worktracker has been uninstalled.${NC}"
echo ""
echo "Thank you for using Work Tracker!"
echo ""
