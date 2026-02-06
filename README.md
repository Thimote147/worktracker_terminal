# Work Hours Tracker

A simple terminal-based application to track your daily work hours, lunch breaks, and calculate overtime or time deficit.

## What is this?

This is a **command-line program** written in the **C programming language**. It helps you:
- Record your daily arrival and departure times
- Track lunch breaks
- Calculate total worked hours
- See if you're working more or less than required (default: 7 hours 48 minutes)
- View your work history
- Track overtime/deficit across multiple days

The program saves your data automatically, so you won't lose your work even if you close the terminal!

## Prerequisites (What You Need)

**Note:** If you use the [Quick Install](#quick-install-recommended) method below, the installer will automatically install these tools for you! You can skip this section and go straight to installation.

If you prefer to install manually, you need these tools on your computer:

### On Linux (Ubuntu, Debian, Raspberry Pi OS, etc.)

Open a terminal and run:
```bash
sudo apt update
sudo apt install build-essential
```

This installs:
- **gcc**: The C compiler (converts C code to an executable program)
- **make**: A build tool that automates the compilation process

### On macOS

Open Terminal and run:
```bash
xcode-select --install
```

This installs Apple's developer tools including gcc and make.

### On Windows

You have two options:

**Option 1: WSL (Windows Subsystem for Linux) - Recommended**
1. Install WSL from the Microsoft Store
2. Install Ubuntu from the Microsoft Store
3. Follow the Linux instructions above

**Option 2: MinGW**
1. Download MinGW from [mingw-w64.org](https://www.mingw-w64.org/)
2. Install it and add it to your PATH
3. Use MinGW terminal for all commands

## Installation & Setup

### Quick Install (Recommended)

The easiest way to install is using the automated installer. **No prerequisites needed** - the installer will automatically install build tools if they're missing!

1. **Download or clone the repository:**
   ```bash
   git clone https://github.com/Thimote147/worktracker_terminal.git/
   cd worktracker_terminal
   ```

2. **Run the installer:**
   ```bash
   ./install.sh
   ```

   Then activate in your current terminal:
   ```bash
   source ~/.zshrc    # or ~/.bashrc
   ```

3. **Start using it:**
   ```bash
   worktracker
   ```

**What the installer does:**
- Auto-detects your OS and shell (Ubuntu, Fedora, Arch, macOS, zsh/bash)
- Installs build tools if needed (gcc/make)
- Compiles the program
- Installs to `~/.local/bin/` and configures PATH

**Note:** New terminal windows will have `worktracker` available automatically.

### Manual Installation (Alternative)

If you prefer to do it manually or want to understand each step:

#### Step 1: Get the Code

If you have git installed:
```bash
git clone https://github.com/Thimote147/worktracker_terminal.git/
cd worktracker_terminal
```

Or simply download the files and navigate to the folder in your terminal:
```bash
cd /path/to/worktracker_terminal
```

#### Step 2: Compile the Program

This converts the human-readable C code into a program your computer can run.

```bash
make
```

**What just happened?**
- The `make` command read the `Makefile` (a recipe file)
- It ran the C compiler (`gcc`) with specific settings
- It created an executable file called `worktracker`

If you see no errors, you're ready to go!

#### Step 3: Run the Program

```bash
./worktracker
```

The `./` means "run the program in the current folder"

#### Step 4 (Optional): Install System-Wide

To run `worktracker` from anywhere:

```bash
# Copy to a directory in your PATH
cp worktracker ~/.local/bin/

# Add to PATH if not already there (add to ~/.zshrc or ~/.bashrc)
echo 'export PATH="$HOME/.local/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

## How to Use

Run `worktracker` to start. Choose option **1** to record today's hours, then enter your times when prompted (arrival, lunch times, departure). That's it!

**Other options:**
- **2** - Add past day
- **3** - Modify/delete entries
- **6** - View history and totals

**Tip:** Press Ctrl+C anytime to pause. Your progress is saved automatically.

## Where is My Data Stored?

Your data is saved in your home directory:
```
~/.local/bin/worktracker.dat
```

The `~` symbol means your home folder:
- Linux/Mac: `/home/yourusername/`
- Windows WSL: `/home/yourusername/`

This is a **binary file** (not human-readable), which makes it efficient and secure.

Temporary in-progress data is saved to:
```
~/.local/bin/temp_day.tmp
```

## Understanding Time Formats

All times are entered in 24-hour format:
- Morning: `09:00` (9 AM)
- Noon: `12:00` (12 PM)
- Afternoon: `14:30` (2:30 PM)
- Evening: `17:00` (5 PM)

## Customizing Required Hours

The program is set to require **7 hours and 48 minutes** of work per day.

To change this, edit [worktracker.c](worktracker.c) lines 13-14:
```c
#define REQUIRED_HOURS 7
#define REQUIRED_MINUTES 48
```

For example, for 8 hours:
```c
#define REQUIRED_HOURS 8
#define REQUIRED_MINUTES 0
```

After changing, recompile:
```bash
make clean
make
```

## Uninstalling

To remove worktracker from your system, use the uninstall script:

```bash
./uninstall.sh
```

The uninstaller will:
1. Remove the `worktracker` binary from `~/.local/bin/`
2. **Ask** if you want to delete your work data (worktracker.dat)
   - Choose **No** if you want to keep your work history
   - Choose **Yes** to completely remove all data
3. **Ask** if you want to remove PATH configuration from your shell config files
   - Creates backups before making changes
4. Provide clear feedback on what was removed

**Important Notes:**
- Your data files are preserved by default - you must explicitly choose to delete them
- Shell config backups are created automatically (`.zshrc.backup.*`, `.bashrc.backup.*`)
- You can reinstall later without losing data if you choose to keep it

**Manual Uninstall:**
If you prefer to remove manually:
```bash
# Remove the binary
rm ~/.local/bin/worktracker

# (Optional) Remove data
rm ~/.local/bin/worktracker.dat
rm ~/.local/bin/temp_day.tmp

# (Optional) Remove PATH from your shell config
# Edit ~/.zshrc or ~/.bashrc and remove the line:
# export PATH="$HOME/.local/bin:$PATH"
```

## Common Issues & Solutions

### "bash: ./install.sh: Permission denied"
Make the installer executable:
```bash
chmod +x install.sh
./install.sh
```

### "worktracker: command not found" after installation
Your PATH hasn't been updated in the current terminal. Choose one solution:

**Reload your shell config:**
```bash
source ~/.zshrc    # or ~/.bashrc
```

**Or simply:**
Close and reopen your terminal.

### "make: command not found"
You need to install build tools. See [Prerequisites](#prerequisites-what-you-need).

### "gcc: command not found"
You need to install the C compiler. See [Prerequisites](#prerequisites-what-you-need).

### "Permission denied" when running ./worktracker
Make the file executable:
```bash
chmod +x worktracker
```

### "No such file or directory" when running ./worktracker
Make sure you're in the correct folder:
```bash
ls
```
You should see `worktracker` in the list.

### I want to start fresh
Delete the data files:
```bash
rm ~/.local/bin/worktracker.dat
rm ~/.local/bin/temp_day.tmp
```
Or use option 7 in the program menu.

## Building Blocks Explained

For those curious about the C code:

### What are Libraries?
At the top of [worktracker.c](worktracker.c#L1-L8), you see:
```c
#include <stdio.h>
#include <stdlib.h>
```

These are **libraries** - pre-written code that provides useful functions:
- `stdio.h` - Standard Input/Output (printing to screen, reading user input)
- `stdlib.h` - Standard Library (memory management, conversions)
- `string.h` - String manipulation
- `time.h` - Date and time functions
- `signal.h` - Handle Ctrl+C and other signals
- `sys/stat.h` - File and directory operations
- `unistd.h` - UNIX standard functions
- `pwd.h` - User/password information

### What are Functions?
A function is a reusable block of code. For example, in [worktracker.c](worktracker.c#L70-L74):
```c
void get_current_date(char *date) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date, 11, "%Y-%m-%d", t);
}
```

This function gets today's date in YYYY-MM-DD format. Instead of writing this code repeatedly, we just call `get_current_date()` whenever needed.

### What are Structs?
A struct groups related data together. From [worktracker.c](worktracker.c#L24-L37):
```c
typedef struct {
    char date[11];
    DayState state;
    int start_hour;
    int start_min;
    // ...
} WorkDay;
```

This creates a `WorkDay` type that holds all information about one workday - like a container with multiple compartments.

### What is Compilation?
When you run `make`:
1. The compiler reads [worktracker.c](worktracker.c) (human-readable code)
2. It checks for errors
3. It converts the code to **machine code** (0s and 1s your CPU understands)
4. It creates the `worktracker` executable

This is different from Python/JavaScript where code runs directly through an interpreter.

## Project Structure

```
worktracker_terminal/
├── worktracker.c      # Main source code
├── Makefile           # Build instructions
├── install.sh         # Automated installation script
├── uninstall.sh       # Automated uninstallation script
└── README.md          # This file
```

## Development

Want to modify the code? Here's the workflow:

1. Edit [worktracker.c](worktracker.c)
2. Recompile:
   ```bash
   make clean   # Remove old compiled version
   make         # Compile new version
   ```
3. Test:
   ```bash
   ./worktracker
   ```

## Technical Details

- **Language**: C11 standard
- **Compiler**: GCC with `-Wall -Wextra` (enables all warnings)
- **Data Storage**: Binary format using `fwrite`/`fread`
- **State Management**: Progress saved to temporary file
- **Signal Handling**: Graceful Ctrl+C handling

## License

This project is open source. Feel free to modify and use it as you wish.

## Contributing

Found a bug? Want to add a feature? Contributions are welcome!

## Support

If you encounter issues:
1. Check [Common Issues & Solutions](#common-issues--solutions)
2. Make sure you have the latest version
3. Try deleting and recompiling: `make clean && make`

---

**Made with C** - Teaching the fundamentals of system programming, one timesheet at a time.
