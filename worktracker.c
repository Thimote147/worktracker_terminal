// timetracker.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pwd.h>

#define DATA_DIR ".local/bin"
#define DATA_FILE_NAME "timetracker.dat"
#define TEMP_FILE_NAME "temp_day.tmp"
#define REQUIRED_HOURS 7
#define REQUIRED_MINUTES 48

typedef enum {
    STATE_NEW,
    STATE_STARTED,
    STATE_LUNCH_START,
    STATE_LUNCH_END,
    STATE_COMPLETED
} DayState;

typedef struct {
    char date[11];  // YYYY-MM-DD
    DayState state;
    int start_hour;
    int start_min;
    int lunch_start_hour;
    int lunch_start_min;
    int lunch_end_hour;
    int lunch_end_min;
    int end_hour;
    int end_min;
    int worked_minutes;
    int excess_minutes;
} WorkDay;

WorkDay current_day;
int is_editing = 0;
char data_file_path[512];
char temp_file_path[512];

void init_paths() {
    const char *home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        home = pw->pw_dir;
    }
    
    // Create full path
    snprintf(data_file_path, sizeof(data_file_path), "%s/%s/%s", home, DATA_DIR, DATA_FILE_NAME);
    snprintf(temp_file_path, sizeof(temp_file_path), "%s/%s/%s", home, DATA_DIR, TEMP_FILE_NAME);
    
    // Create directory if it doesn't exist
    char dir_path[512];
    snprintf(dir_path, sizeof(dir_path), "%s/%s", home, DATA_DIR);
    
    struct stat st = {0};
    if (stat(dir_path, &st) == -1) {
        mkdir(dir_path, 0700);
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void get_current_date(char *date) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date, 11, "%Y-%m-%d", t);
}

int time_to_minutes(int hour, int min) {
    return hour * 60 + min;
}

void minutes_to_time(int total_minutes, int *hour, int *min) {
    *hour = total_minutes / 60;
    *min = total_minutes % 60;
}

void print_time_diff(int minutes) {
    int hours = abs(minutes) / 60;
    int mins = abs(minutes) % 60;
    if (minutes >= 0) {
        printf("+%02d:%02d", hours, mins);
    } else {
        printf("-%02d:%02d", hours, mins);
    }
}

int load_temp_day(WorkDay *day) {
    FILE *file = fopen(temp_file_path, "rb");
    if (!file) {
        return 0;
    }
    
    int result = fread(day, sizeof(WorkDay), 1, file);
    fclose(file);
    
    // Check if it's the same day
    char today[11];
    get_current_date(today);
    if (strcmp(day->date, today) != 0) {
        remove(temp_file_path);
        return 0;
    }
    
    return result == 1;
}

void save_temp_day(WorkDay *day) {
    FILE *file = fopen(temp_file_path, "wb");
    if (file) {
        fwrite(day, sizeof(WorkDay), 1, file);
        fclose(file);
    }
}

void delete_temp_day() {
    remove(temp_file_path);
}

void signal_handler(int signum) {
    if (signum == SIGINT && is_editing) {
        printf("\n\n✓ Saving...\n");
        save_temp_day(&current_day);
        printf("✓ Data saved! You can continue later.\n");
        exit(0);
    }
}

void save_completed_day(WorkDay *day) {
    // Calculate worked minutes
    int start_minutes = time_to_minutes(day->start_hour, day->start_min);
    int lunch_start_minutes = time_to_minutes(day->lunch_start_hour, day->lunch_start_min);
    int lunch_end_minutes = time_to_minutes(day->lunch_end_hour, day->lunch_end_min);
    int end_minutes = time_to_minutes(day->end_hour, day->end_min);
    
    int morning_work = lunch_start_minutes - start_minutes;
    int afternoon_work = end_minutes - lunch_end_minutes;
    day->worked_minutes = morning_work + afternoon_work;
    
    int required_minutes = REQUIRED_HOURS * 60 + REQUIRED_MINUTES;
    day->excess_minutes = day->worked_minutes - required_minutes;
    
    day->state = STATE_COMPLETED;
    
    // Save to history
    FILE *file = fopen(data_file_path, "ab");
    if (file) {
        fwrite(day, sizeof(WorkDay), 1, file);
        fclose(file);
        
        printf("\n✓ Day saved!\n");
        printf("===========================================\n");
        printf("Arrival:         %02d:%02d\n", day->start_hour, day->start_min);
        printf("Lunch break:     %02d:%02d - %02d:%02d\n", 
               day->lunch_start_hour, day->lunch_start_min,
               day->lunch_end_hour, day->lunch_end_min);
        printf("Departure:       %02d:%02d\n", day->end_hour, day->end_min);
        printf("-------------------------------------------\n");
        printf("Time worked:     %02d:%02d\n", day->worked_minutes / 60, day->worked_minutes % 60);
        printf("Required:        %02d:%02d\n", REQUIRED_HOURS, REQUIRED_MINUTES);
        printf("Difference:      ");
        print_time_diff(day->excess_minutes);
        printf("\n===========================================\n");
        
        delete_temp_day();
    } else {
        printf("Error: Unable to save data\n");
    }
}

void calculate_end_time(WorkDay *day) {
    int required_minutes = REQUIRED_HOURS * 60 + REQUIRED_MINUTES;
    int lunch_duration = time_to_minutes(day->lunch_end_hour, day->lunch_end_min) - 
                         time_to_minutes(day->lunch_start_hour, day->lunch_start_min);
    
    int start_minutes = time_to_minutes(day->start_hour, day->start_min);
    int end_minutes = start_minutes + required_minutes + lunch_duration;
    
    minutes_to_time(end_minutes, &day->end_hour, &day->end_min);
    
    printf("\n===========================================\n");
    printf("You should finish at: %02d:%02d\n", day->end_hour, day->end_min);
    printf("===========================================\n");
}

void enter_day_data() {
    is_editing = 1;
    
    // Load temporary data if it exists
    int has_temp = load_temp_day(&current_day);
    
    if (has_temp) {
        printf("\n=== RESUMING DAY: %s ===\n", current_day.date);
        printf("\n📌 Already recorded:\n");
        
        if (current_day.state >= STATE_STARTED) {
            printf("✓ Arrival: %02d:%02d\n", current_day.start_hour, current_day.start_min);
        }
        if (current_day.state >= STATE_LUNCH_START) {
            printf("✓ Lunch start: %02d:%02d\n", current_day.lunch_start_hour, current_day.lunch_start_min);
        }
        if (current_day.state >= STATE_LUNCH_END) {
            printf("✓ Lunch end: %02d:%02d\n", current_day.lunch_end_hour, current_day.lunch_end_min);
        }
        printf("\n");
    } else {
        // Initialize a new day
        current_day = (WorkDay){0};
        get_current_date(current_day.date);
        current_day.state = STATE_NEW;
        printf("\n=== NEW DAY: %s ===\n", current_day.date);
    }
    
    printf("\n💡 You can press Ctrl+C at any time to save and quit\n\n");
    
    // Arrival time (only if not recorded yet)
    if (current_day.state == STATE_NEW) {
        printf("Arrival time (HH:MM): ");
        if (scanf("%d:%d", &current_day.start_hour, &current_day.start_min) != 2) {
            clear_input_buffer();
            printf("Invalid format\n");
            is_editing = 0;
            return;
        }
        clear_input_buffer();
        current_day.state = STATE_STARTED;
        save_temp_day(&current_day);
    }
    
    // Lunch start (only if not recorded yet)
    if (current_day.state == STATE_STARTED) {
        printf("Lunch break start (HH:MM): ");
        if (scanf("%d:%d", &current_day.lunch_start_hour, &current_day.lunch_start_min) != 2) {
            clear_input_buffer();
            printf("Invalid format\n");
            is_editing = 0;
            return;
        }
        clear_input_buffer();
        current_day.state = STATE_LUNCH_START;
        save_temp_day(&current_day);
    }
    
    // Lunch end (only if not recorded yet)
    if (current_day.state == STATE_LUNCH_START) {
        printf("Lunch break end (HH:MM): ");
        if (scanf("%d:%d", &current_day.lunch_end_hour, &current_day.lunch_end_min) != 2) {
            clear_input_buffer();
            printf("Invalid format\n");
            is_editing = 0;
            return;
        }
        clear_input_buffer();
        current_day.state = STATE_LUNCH_END;
        save_temp_day(&current_day);
        
        // Calculate expected end time
        calculate_end_time(&current_day);
    }
    
    // Departure time (always asked if we get here)
    if (current_day.state == STATE_LUNCH_END) {
        printf("\nDeparture time (HH:MM): ");
        if (scanf("%d:%d", &current_day.end_hour, &current_day.end_min) != 2) {
            clear_input_buffer();
            printf("Invalid format\n");
            is_editing = 0;
            return;
        }
        clear_input_buffer();
        
        // Save the completed day
        save_completed_day(&current_day);
    }
    
    is_editing = 0;
}

void add_past_day() {
    WorkDay day = {0};
    
    printf("\n=== ADD PAST DAY ===\n\n");
    
    // Enter date
    printf("Date (YYYY-MM-DD): ");
    if (scanf("%10s", day.date) != 1) {
        clear_input_buffer();
        printf("Invalid format\n");
        return;
    }
    clear_input_buffer();
    
    // Check date format (basic)
    if (strlen(day.date) != 10 || day.date[4] != '-' || day.date[7] != '-') {
        printf("Invalid date format. Use YYYY-MM-DD (e.g., 2026-02-03)\n");
        return;
    }
    
    // Check if this date already exists
    FILE *file = fopen(data_file_path, "rb");
    if (file) {
        WorkDay existing;
        while (fread(&existing, sizeof(WorkDay), 1, file) == 1) {
            if (strcmp(existing.date, day.date) == 0) {
                printf("\n⚠️  An entry already exists for this date!\n");
                printf("Arrival: %02d:%02d, Departure: %02d:%02d\n", 
                       existing.start_hour, existing.start_min,
                       existing.end_hour, existing.end_min);
                printf("\nDo you want to replace it? (y/n): ");
                char choice;
                scanf("%c", &choice);
                clear_input_buffer();
                if (choice != 'y' && choice != 'Y') {
                    fclose(file);
                    return;
                }
                // Continue to replace
                break;
            }
        }
        fclose(file);
    }
    
    printf("\nEntering times for %s:\n\n", day.date);
    
    // Arrival time
    printf("Arrival time (HH:MM): ");
    if (scanf("%d:%d", &day.start_hour, &day.start_min) != 2) {
        clear_input_buffer();
        printf("Invalid format\n");
        return;
    }
    clear_input_buffer();
    
    // Lunch start
    printf("Lunch break start (HH:MM): ");
    if (scanf("%d:%d", &day.lunch_start_hour, &day.lunch_start_min) != 2) {
        clear_input_buffer();
        printf("Invalid format\n");
        return;
    }
    clear_input_buffer();
    
    // Lunch end
    printf("Lunch break end (HH:MM): ");
    if (scanf("%d:%d", &day.lunch_end_hour, &day.lunch_end_min) != 2) {
        clear_input_buffer();
        printf("Invalid format\n");
        return;
    }
    clear_input_buffer();
    
    // Departure time
    printf("Departure time (HH:MM): ");
    if (scanf("%d:%d", &day.end_hour, &day.end_min) != 2) {
        clear_input_buffer();
        printf("Invalid format\n");
        return;
    }
    clear_input_buffer();
    
    // Calculate worked minutes
    int start_minutes = time_to_minutes(day.start_hour, day.start_min);
    int lunch_start_minutes = time_to_minutes(day.lunch_start_hour, day.lunch_start_min);
    int lunch_end_minutes = time_to_minutes(day.lunch_end_hour, day.lunch_end_min);
    int end_minutes = time_to_minutes(day.end_hour, day.end_min);
    
    int morning_work = lunch_start_minutes - start_minutes;
    int afternoon_work = end_minutes - lunch_end_minutes;
    day.worked_minutes = morning_work + afternoon_work;
    
    int required_minutes = REQUIRED_HOURS * 60 + REQUIRED_MINUTES;
    day.excess_minutes = day.worked_minutes - required_minutes;
    
    day.state = STATE_COMPLETED;
    
    // Save - insert at the right place to keep chronological order
    // Load all entries
    FILE *read_file = fopen(data_file_path, "rb");
    WorkDay *entries = NULL;
    int count = 0;
    int capacity = 10;
    
    if (read_file) {
        entries = malloc(capacity * sizeof(WorkDay));
        WorkDay temp;
        while (fread(&temp, sizeof(WorkDay), 1, read_file) == 1) {
            // Don't copy entry if we're replacing it
            if (strcmp(temp.date, day.date) != 0) {
                if (count >= capacity) {
                    capacity *= 2;
                    entries = realloc(entries, capacity * sizeof(WorkDay));
                }
                entries[count++] = temp;
            }
        }
        fclose(read_file);
    } else {
        entries = malloc(capacity * sizeof(WorkDay));
    }
    
    // Insert new entry at the right place (chronological order)
    int insert_pos = count;
    for (int i = 0; i < count; i++) {
        if (strcmp(day.date, entries[i].date) < 0) {
            insert_pos = i;
            break;
        }
    }
    
    // Shift entries if needed
    if (count >= capacity) {
        capacity++;
        entries = realloc(entries, capacity * sizeof(WorkDay));
    }
    
    for (int i = count; i > insert_pos; i--) {
        entries[i] = entries[i-1];
    }
    entries[insert_pos] = day;
    count++;
    
    // Rewrite file
    FILE *write_file = fopen(data_file_path, "wb");
    if (write_file) {
        fwrite(entries, sizeof(WorkDay), count, write_file);
        fclose(write_file);
        
        printf("\n✓ Day %s saved!\n", day.date);
        printf("===========================================\n");
        printf("Arrival:         %02d:%02d\n", day.start_hour, day.start_min);
        printf("Lunch break:     %02d:%02d - %02d:%02d\n", 
               day.lunch_start_hour, day.lunch_start_min,
               day.lunch_end_hour, day.lunch_end_min);
        printf("Departure:       %02d:%02d\n", day.end_hour, day.end_min);
        printf("-------------------------------------------\n");
        printf("Time worked:     %02d:%02d\n", day.worked_minutes / 60, day.worked_minutes % 60);
        printf("Required:        %02d:%02d\n", REQUIRED_HOURS, REQUIRED_MINUTES);
        printf("Difference:      ");
        print_time_diff(day.excess_minutes);
        printf("\n===========================================\n");
    } else {
        printf("Error: Unable to save data\n");
    }
    
    free(entries);
}

void show_current_status() {
    WorkDay day;
    
    if (!load_temp_day(&day)) {
        printf("\nNo day in progress.\n");
        return;
    }
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║        DAY IN PROGRESS                 ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\nDate: %s\n", day.date);
    
    if (day.state >= STATE_STARTED) {
        printf("✓ Arrival: %02d:%02d\n", day.start_hour, day.start_min);
    }
    
    if (day.state >= STATE_LUNCH_START) {
        printf("✓ Lunch start: %02d:%02d\n", day.lunch_start_hour, day.lunch_start_min);
    }
    
    if (day.state >= STATE_LUNCH_END) {
        printf("✓ Lunch end: %02d:%02d\n", day.lunch_end_hour, day.lunch_end_min);
        
        // Recalculate expected end time
        int required_minutes = REQUIRED_HOURS * 60 + REQUIRED_MINUTES;
        int lunch_duration = time_to_minutes(day.lunch_end_hour, day.lunch_end_min) - 
                             time_to_minutes(day.lunch_start_hour, day.lunch_start_min);
        int start_minutes = time_to_minutes(day.start_hour, day.start_min);
        int end_minutes = start_minutes + required_minutes + lunch_duration;
        int end_hour, end_min;
        minutes_to_time(end_minutes, &end_hour, &end_min);
        
        printf("\n→ Expected end time: %02d:%02d\n", end_hour, end_min);
    }
    
    printf("\nNext step: ");
    switch (day.state) {
        case STATE_NEW:
            printf("Record arrival time\n");
            break;
        case STATE_STARTED:
            printf("Record lunch break start\n");
            break;
        case STATE_LUNCH_START:
            printf("Record lunch break end\n");
            break;
        case STATE_LUNCH_END:
            printf("Record departure time\n");
            break;
        default:
            printf("Unknown\n");
    }
}

void show_history() {
    FILE *file = fopen(data_file_path, "rb");
    if (!file) {
        printf("\nNo history found.\n");
        return;
    }
    
    WorkDay day;
    int total_excess = 0;
    
    printf("\n========================================================================================================\n");
    printf("Date       | Start  | Lunch      | End    | Time worked     | Required | Difference\n");
    printf("========================================================================================================\n");
    
    while (fread(&day, sizeof(WorkDay), 1, file) == 1) {
        printf("%s | %02d:%02d  | %02d:%02d-%02d:%02d | %02d:%02d  | %02d:%02d           | %02d:%02d    | ",
               day.date,
               day.start_hour, day.start_min,
               day.lunch_start_hour, day.lunch_start_min,
               day.lunch_end_hour, day.lunch_end_min,
               day.end_hour, day.end_min,
               day.worked_minutes / 60, day.worked_minutes % 60,
               REQUIRED_HOURS, REQUIRED_MINUTES);
        
        print_time_diff(day.excess_minutes);
        printf("\n");
        
        total_excess += day.excess_minutes;
    }
    
    printf("========================================================================================================\n");
    printf("TOTAL MONTH EXCESS: ");
    print_time_diff(total_excess);
    printf("\n========================================================================================================\n");
    
    fclose(file);
}

void reset_data() {
    printf("\nAre you sure you want to delete all data? (y/n): ");
    char choice;
    scanf("%c", &choice);
    clear_input_buffer();
    
    if (choice == 'y' || choice == 'Y') {
        remove(data_file_path);
        remove(temp_file_path);
        printf("✓ Data deleted.\n");
    }
}

int main() {
    // Initialize file paths
    init_paths();
    
    // Install signal handler for Ctrl+C
    signal(SIGINT, signal_handler);
    
    int choice;
    WorkDay temp_day;
    int has_current_day;
    
    while (1) {
        has_current_day = load_temp_day(&temp_day);
        
        printf("\n╔════════════════════════════════════════╗\n");
        printf("║      WORK HOURS TRACKER                ║\n");
        printf("╚════════════════════════════════════════╝\n");
        printf("\n");
        
        if (has_current_day) {
            printf("⚠️  Day in progress detected!\n\n");
        }
        
        printf("1. %s\n", has_current_day ? "Continue/Complete current day" : "New day");
        printf("2. Add past day\n");
        printf("3. View current status\n");
        printf("4. Cancel current day\n");
        printf("5. View history\n");
        printf("6. Reset data\n");
        printf("7. Quit\n");
        
        printf("\nChoice: ");
        
        if (scanf("%d", &choice) != 1) {
            clear_input_buffer();
            printf("\nInvalid choice.\n");
            continue;
        }
        clear_input_buffer();
        
        switch (choice) {
            case 1:
                enter_day_data();
                break;
            case 2:
                add_past_day();
                break;
            case 3:
                show_current_status();
                break;
            case 4:
                if (has_current_day) {
                    delete_temp_day();
                    printf("\n✓ Current day canceled.\n");
                } else {
                    printf("\nNo day in progress.\n");
                }
                break;
            case 5:
                show_history();
                break;
            case 6:
                reset_data();
                break;
            case 7:
                printf("\nGoodbye!\n");
                return 0;
            default:
                printf("\nInvalid choice.\n");
        }
    }
    
    return 0;
}