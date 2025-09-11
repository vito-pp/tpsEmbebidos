/* main_test_fsm.c
compile with:  gcc -Wall -Wextra -std=c11 -o fsm_test main_test_fsm.c ../app/
fsm.c */

#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#include "../app/fsm.h"

/* ---------- non blocking input ----------
source: https://stackoverflow.com/questions/448944/c-non-blocking-keyboard-input/448982#448982 */

static struct termios orig_termios;

static void reset_terminal_mode(void) 
{
    tcsetattr(0, TCSANOW, &orig_termios);
}

static void set_conio_terminal_mode(void) 
{
    struct termios new_termios;
    tcgetattr(0, &orig_termios);
    memcpy(&new_termios, &orig_termios, sizeof(new_termios));
    atexit(reset_terminal_mode);
    cfmakeraw(&new_termios);
    tcsetattr(0, TCSANOW, &new_termios);
}

static int kbhit(void) 
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0) {
        return r;
    } else {
        return c;
    }
}

// ---------- key -> event mapping ----------
static FSM_event_t key_to_event(int ch) 
{
    switch (ch) {
        case 'e': return EV_ENTER;
        case 'E': return EV_DOUBLE_ENTER;
        case 'f': return EV_FORWARD;
        case 'b': return EV_BACKWARD;
        case 'r': return EV_RESET;
        case 't': return EV_TIMEOUT;
        // uncomment if you use these:
        // case 'o': return EV_OK;
        // case 'm': return EV_MAG_DATA;
        default:  return EV_NONE;
    }
}

static void print_help(void) 
{
    puts("\r=== FSM CLI Test ===");
    puts("\r keys:");
    puts("\r   e -> EV_ENTER");
    puts("\r   E -> EV_DOUBLE_ENTER");
    puts("\r   f -> EV_FORWARD");
    puts("\r   b -> EV_BACKWARD");
    puts("\r   r -> EV_RESET");
    puts("\r   t -> EV_TIMEOUT");
    // puts("   o -> EV_OK");
    // puts("   m -> EV_MAG_DATA");
    puts("\r   q -> quit");
}

// Optional: single-step event fetch using your kbhit/getch
static FSM_event_t getEventTest(void) 
{
    if (!kbhit()) return EV_NONE;
    int ch = getch();
    if (ch == 'q' || ch == 27) 
    {
        puts("\r[quit]");
        exit(0); // terminate program
    }
    printf("\rYou've entered: %c\n", ch);
    // int ch;
    // ch = getchar();
    return key_to_event(ch);
}

int main(void)
{
    set_conio_terminal_mode();
    print_help();

    const FSM_State_t *current = getInitState();

    for (;;) 
    {
        // poll keyboard without blocking
        FSM_event_t event = getEventTest();
        current = fsmStep(current, event);

        // If you have software timers that should trigger events,
        // call your timerUpdate() here so their callbacks can run.
        // timerUpdate();

        // tiny idle pause
        usleep(200);
    }
    //reset_terminal_mode() is called automatically via atexit()

    // set_conio_terminal_mode();
    // char ch;
    // printf("Enter key ESC to exit \n");
    // // define infinite loop for taking keys
    // while (1) 
    // {
    //     if (kbhit) 
    //     {
    //         // fetch typed character into ch
    //         ch = getch();
    //         if ((int)ch == 27)
    //             // when esc button is pressed, then it will exit from loop
    //             break;
    //         printf("\rYou have entered : %c\n", ch);
    //     }
    // }

    return 0;
}
