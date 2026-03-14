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

int alarmH, alarmM;
int isSnoozed = 0;
int alarmTriggered = 0;
time_t snoozeUntil = 0;
pthread_mutex_t alarm_mutex;

static const char *months[] = {
    "Jan","Feb","Mar","Apr","May","Jun",
    "Jul","Aug","Sep","Oct","Nov","Dec"
};

void* clock_thread(void* arg) 
{
    while (1) 
    {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        system("clear");

        pthread_mutex_lock(&alarm_mutex);
        int current_snooze_status = isSnoozed;
        time_t current_snooze_until = snoozeUntil;
        pthread_mutex_unlock(&alarm_mutex);

        int displayHour = t->tm_hour;
        char *period = (displayHour >= 12) ? "PM" : "AM";
        if (displayHour == 0) displayHour = 12;
        else if (displayHour > 12) displayHour -= 12;

        printf(CYAN "=======================================\n" RESET);
        printf(BOLD MAGENTA "       MAC PRO DIGITAL CLOCK\n" RESET);
        printf(CYAN "=======================================\n" RESET);

        printf(YELLOW " [Alarm: %02d:%02d]  [Snooze: %s]\n" RESET,
               alarmH, alarmM,
               current_snooze_status ? GREEN "ACTIVE" RESET : RED "OFF" RESET);

        printf(CYAN "---------------------------------------\n" RESET);

        printf(BOLD GREEN "    TIME : %02d:%02d:%02d %s\n" RESET,
               displayHour, t->tm_min, t->tm_sec, period);

        printf(BLUE "    DATE : %02d %s %04d\n" RESET,
               t->tm_mday, months[t->tm_mon], t->tm_year + 1900);

        printf(CYAN "---------------------------------------\n" RESET);
        printf(YELLOW " >> Press 's' + Enter to Snooze\n" RESET);
        printf(CYAN "=======================================\n" RESET);

        int should_ring = 0;

        if (current_snooze_status && now >= current_snooze_until) 
        {
            pthread_mutex_lock(&alarm_mutex);
            isSnoozed = 0;
            pthread_mutex_unlock(&alarm_mutex);
            should_ring = 1;
        } 
        else if (!current_snooze_status &&
                   t->tm_hour == alarmH &&
                   t->tm_min == alarmM &&
                   t->tm_sec == 0) 
        {
            should_ring = 1;
        }

        if (should_ring)
        {
            printf(BOLD RED "\n!!! ALARM IS RINGING !!!\a\n" RESET);
            fflush(stdout);
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
                isSnoozed = 1;
                snoozeUntil = time(NULL) + 10;
                pthread_mutex_unlock(&alarm_mutex);
            }
        }
    }
    return NULL;
}

int main() 
{
    pthread_mutex_init(&alarm_mutex, NULL);

    printf(BOLD GREEN "Welcome to Pro-Clock Setup\n" RESET);
    printf("Set Alarm (HH MM): ");
    if (scanf("%d %d", &alarmH, &alarmM) != 2) return 1;

    if (alarmH < 0 || alarmH > 23 || alarmM < 0 || alarmM > 59) 
    {
        printf(RED "Invalid alarm time. Use HH (0-23) and MM (0-59).\n" RESET);
        return 1;
    }

    pthread_t t1, t2;
    pthread_create(&t1, NULL, clock_thread, NULL);
    pthread_create(&t2, NULL, input_thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&alarm_mutex);
    return 0;
}
