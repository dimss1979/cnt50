#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

static struct gpiod_line *line = NULL;

void timer_callback(int signum)
{
    static unsigned int val = 0;
    int ret;

    ret = gpiod_line_set_value(line, val);
    if (ret < 0) {
        perror("Set line output failed\n");
        exit(1);
    }
    val = !val;
}

int main(int argc, char **argv)
{
    int chip_num, line_num;
    struct gpiod_chip *chip;
    int ret;
    struct sigevent sev = {0};
    timer_t timerid;
    struct itimerspec its;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s <chip_num> <pin_num>\n", argv[0]);
        exit(1);
    }

    chip_num = atoi(argv[1]);
    line_num = atoi(argv[2]);

    chip = gpiod_chip_open_by_number(chip_num);
    if (!chip) {
        perror("Open chip failed\n");
        exit(1);
    }

    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("Get line failed\n");
        exit(1);
    }

    ret = gpiod_line_request_output(line, "50 Hz generator", 0);
    if (ret < 0) {
        perror("Request line as output failed\n");
        exit(1);
    }

    signal(SIGRTMIN, timer_callback);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &timerid;
    if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1) {
        perror("Timer creation failed\n");
        exit(1);
    }

    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 1;

    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 10000 /* usec */ * 1000;

    if (timer_settime(timerid, 0, &its, NULL) == -1) {
        perror("Timer setup failed\n");
        exit(1);
    }

    while(1) {
        sleep(3600);
    }

    return 0;
}
