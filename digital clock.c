//"This code is designed to run exclusively on macOS and Linux."
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define RESET   "\x1b[0m"
#define BOLD    "\033[1m"

typedef struct 
{
    int    alarmH;
    int    alarmM;
    int    alarmSet;
    int    isSnoozed;
    int    isRunning;
    int    alarmTriggered;
    int    isRinging;
    time_t snoozeUntil;
} ClockState;

ClockState state = {0, 0, 0, 0, 1, 0, 0, 0};
pthread_mutex_t alarm_mutex;

static const char *months[] = 
{
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

static const char *days[] = 
{
    "Sunday","Monday","Tuesday","Wednesday",
    "Thursday","Friday","Saturday"
};

void startup_animation() 
{
    const char *title = " TALUKDER MAC PRO DIGITAL CLOCK";
    system("clear");
    printf(CYAN "=======================================\n" RESET);
    printf(BOLD MAGENTA);
    for (int i = 0; title[i] != '\0'; i++) 
    {
        printf("%c", title[i]);
        fflush(stdout);
        usleep(40000);
    }
    printf(RESET "\n");
    printf(CYAN "=======================================\n" RESET);
    printf(BOLD GREEN "\n        Initializing system...\n" RESET);
    usleep(500000);
    printf(BOLD GREEN "        Loading threads...\n" RESET);
    usleep(500000);
    printf(BOLD GREEN "        Clock ready!\n\n" RESET);
    usleep(600000);
}

void* sound_thread(void* arg)
{
    while (1)
    {
        pthread_mutex_lock(&alarm_mutex);
        int running = state.isRunning;
        int ringing = state.isRinging;
        pthread_mutex_unlock(&alarm_mutex);

        if (!running) break;

        if (ringing)
        {
            system("afplay /System/Library/Sounds/Funk.aiff");
        }
        else
        {
            usleep(200000);
        }
    }
    return NULL;
}

void* clock_thread(void* arg) 
{
    int ring_frame = 0;

    while (1) 
    {
        pthread_mutex_lock(&alarm_mutex);
        int running = state.isRunning;
        pthread_mutex_unlock(&alarm_mutex);
        if (!running) break;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        pthread_mutex_lock(&alarm_mutex);
        int    current_snooze_status  = state.isSnoozed;
        time_t current_snooze_until   = state.snoozeUntil;
        int    current_alarm_h        = state.alarmH;
        int    current_alarm_m        = state.alarmM;
        int    alarm_set              = state.alarmSet;
        pthread_mutex_unlock(&alarm_mutex);

        int displayHour = t->tm_hour;
        char *period = (displayHour >= 12) ? "PM" : "AM";
        if (displayHour == 0) displayHour = 12;
        else if (displayHour > 12) displayHour -= 12;

        system("clear");
        printf(CYAN "=======================================\n" RESET);
        printf(BOLD MAGENTA "   TALUKDER MAC PRO DIGITAL CLOCK\n" RESET);
        printf(CYAN "=======================================\n" RESET);

        if (alarm_set)
        {
            printf(YELLOW " [Alarm: %02d:%02d]  [Snooze: %s]\n" RESET,
                   current_alarm_h, current_alarm_m,
                   current_snooze_status ? GREEN "ACTIVE" RESET : RED "OFF" RESET);
        }
        else
        {
            printf(YELLOW " [Alarm: --:--]  [No alarm set]\n" RESET);
        }

        printf(CYAN "---------------------------------------\n" RESET);

        printf(BOLD GREEN "    TIME : %02d:%02d:%02d %s\n" RESET,
               displayHour, t->tm_min, t->tm_sec, period);

        printf(BLUE "    DATE : %s, %02d %s %04d\n" RESET,
               days[t->tm_wday], t->tm_mday,
               months[t->tm_mon], t->tm_year + 1900);

        printf(CYAN "---------------------------------------\n" RESET);

        if (current_snooze_status) 
        {
            int remaining = (int)(current_snooze_until - now);
            if (remaining < 0) remaining = 0;
            printf(YELLOW " >> Snooze: ringing again in %02d sec\n" RESET, remaining);
        } 
        else 
        {
            printf(YELLOW " >> Press 's' + Enter to Snooze\n" RESET);
        }

        printf(YELLOW " >> Press 'a' + Enter to Set New Alarm\n" RESET);
        printf(YELLOW " >> Press 'q' + Enter to Stop Alarm\n" RESET);
        printf(YELLOW " >> Press 'x' + Enter to Exit Clock\n" RESET);
        printf(CYAN "=======================================\n" RESET);

        int should_ring = 0;

        if (current_snooze_status && now >= current_snooze_until) 
        {
            pthread_mutex_lock(&alarm_mutex);
            state.isSnoozed      = 0;
            state.alarmTriggered = 0;
            pthread_mutex_unlock(&alarm_mutex);
            should_ring = 1;
        } 
        else if (alarm_set              &&
                 !current_snooze_status &&
                 t->tm_hour == current_alarm_h &&
                 t->tm_min  == current_alarm_m) 
        {
            should_ring = 1;
        }

        if (should_ring) 
        {
            const char *frames[] = 
            {
                BOLD RED "  !!! HEY BIKASH ALARM IS RINGING !!!  " RESET,
                BOLD YELLOW "  *** HEY BIKASH ALARM IS RINGING ***  " RESET
            };
            printf("\n%s\n", frames[ring_frame % 2]);
            fflush(stdout);
            ring_frame++;

            pthread_mutex_lock(&alarm_mutex);
            state.isRinging = 1;
            pthread_mutex_unlock(&alarm_mutex);
        } 
        else 
        {
            ring_frame = 0;
            pthread_mutex_lock(&alarm_mutex);
            state.isRinging = 0;
            pthread_mutex_unlock(&alarm_mutex);
        }

        sleep(1);
    }
    return NULL;
}

void* input_thread(void* arg) 
{
    char ch;
    while (1) 
    {
        if (scanf(" %c", &ch) == 1) 
        {
            if (ch == 's' || ch == 'S') 
            {
                pthread_mutex_lock(&alarm_mutex);
                state.isSnoozed      = 1;
                state.isRinging      = 0;
                state.alarmTriggered = 0;
                state.snoozeUntil    = time(NULL) + 10;
                pthread_mutex_unlock(&alarm_mutex);
            } 
            else if (ch == 'q' || ch == 'Q') 
            {
                pthread_mutex_lock(&alarm_mutex);
                state.isRinging      = 0;
                state.isSnoozed      = 0;
                state.alarmTriggered = 0;
                state.alarmSet       = 0;
                pthread_mutex_unlock(&alarm_mutex);
                printf(BOLD GREEN "\n  Alarm stopped. Clock is still running.\n" RESET);
            }
            else if (ch == 'a' || ch == 'A')
            {
                int newH, newM;
                printf(BOLD YELLOW "\n Hey bikash Set New Alarm (HH MM): " RESET);
                if (scanf("%d %d", &newH, &newM) == 2)
                {
                    if (newH >= 0 && newH <= 23 && newM >= 0 && newM <= 59)
                    {
                        pthread_mutex_lock(&alarm_mutex);
                        state.alarmH         = newH;
                        state.alarmM         = newM;
                        state.alarmSet       = 1;
                        state.isSnoozed      = 0;
                        state.isRinging      = 0;
                        state.alarmTriggered = 0;
                        pthread_mutex_unlock(&alarm_mutex);
                        printf(BOLD GREEN "  New alarm set for %02d:%02d!\n" RESET, newH, newM);
                    }
                    else
                    {
                        printf(RED "  Invalid time. Use HH (0-23) and MM (0-59).\n" RESET);
                    }
                }
            }
            else if (ch == 'x' || ch == 'X') 
            {
                pthread_mutex_lock(&alarm_mutex);
                state.isRunning = 0;
                state.isRinging = 0;
                pthread_mutex_unlock(&alarm_mutex);
                break;
            }
        }
    }
    return NULL;
}

int main() 
{
    pthread_mutex_init(&alarm_mutex, NULL);

    startup_animation();

    printf(BOLD GREEN "Welcome to TALUKDER Pro-Clock Setup\n" RESET);
    printf("Set Alarm (HH MM) or press Enter to skip: ");

    int newH, newM;
    if (scanf("%d %d", &newH, &newM) == 2)
    {
        if (newH >= 0 && newH <= 23 && newM >= 0 && newM <= 59)
        {
            state.alarmH   = newH;
            state.alarmM   = newM;
            state.alarmSet = 1;
            printf(BOLD GREEN "\nAlarm set for %02d:%02d. Clock starting...\n" RESET,
                   state.alarmH, state.alarmM);
        }
        else
        {
            printf(RED "Invalid time. Clock starting without alarm.\n" RESET);
        }
    }
    else
    {
        printf(BOLD CYAN "\nNo alarm set. Clock starting...\n" RESET);
    }

    sleep(1);

    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, clock_thread, NULL);
    pthread_create(&t2, NULL, input_thread, NULL);
    pthread_create(&t3, NULL, sound_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    pthread_mutex_destroy(&alarm_mutex);

    system("clear");
    printf(BOLD CYAN "\n  Thank you for using TALUKDER MAC PRO DIGITAL CLOCK.\n\n" RESET);
    return 0;
}
