// Digital Clock with Date, Day & Month Name (12-hour format)

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

int main() 
{

    while (1)
     {

        time_t now = time(NULL);
        struct tm *t = localtime(&now);

        int hour = t->tm_hour;
        char *period;

        // 12-hour conversion
        if (hour == 0) 
        {
            hour = 12;
            period = "AM";
        }
        else if (hour < 12)
        {
            period = "AM";
        }
        else if (hour == 12) 
        {
            period = "PM";
        }
        else {
            hour = hour - 12;
            period = "PM";
        }

        // Day names
        char *days[] = 
        {
            "Sunday", "Monday", "Tuesday",
            "Wednesday", "Thursday", "Friday", "Saturday"
        };

        // Month names
        char *months[] = 
        {
            "January", "February", "March",
            "April", "May", "June",
            "July", "August", "September",
            "October", "November", "December"
        };

        // Clear screen
        #ifdef _WIN32
        system("cls");
        #else
        system("clear");
        #endif

        printf("Digital Clock with Full Date\n");
        printf("-------------------------------\n");

        printf("Time : %02d:%02d:%02d %s\n",
               hour,
               t->tm_min,
               t->tm_sec,
               period);

        printf("Date : %02d %s %04d\n",
               t->tm_mday,
               months[t->tm_mon],   // month name
               t->tm_year + 1900);

        printf("Day  : %s\n",
               days[t->tm_wday]);

        #ifdef _WIN32
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }

    return 0;
}