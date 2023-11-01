#include <stdio.h>
#include <stdlib.h>
#include <gpiod.h>
#include <unistd.h>

#define PERIOD_CNT 200 /* ~4 sec */
#define PERIOD_MIN 19231 /* 52 Hz */
#define PERIOD_MAX 20833 /* 48 Hz */

static void register_event(struct gpiod_line_event *event)
{
    unsigned long ts, period;
    double freq;

    static unsigned long ts_prev;
    static unsigned long ts_first;
    static unsigned int count = 0;

    ts = event->ts.tv_sec * 1000000 + event->ts.tv_nsec / 1000;
    if (count) {
        period = ts - ts_prev;
        if (period > PERIOD_MAX || period < PERIOD_MIN) {
            count = 0;
            return;
        }
    } else {
        ts_first = ts;
    }
    ts_prev = ts;

    if (count == PERIOD_CNT) {
        freq = 1000000.0 * PERIOD_CNT / (ts - ts_first);
        printf("Freq: %0.03f\n", freq);
        count = 0;
        return;
    }
    count++;
}

int main(int argc, char **argv)
{
    int chip_num, line_num;
    struct gpiod_chip *chip;
    static struct gpiod_line *line = NULL;
    int ret;
    struct timespec timeout = { 1, 0 };
    struct gpiod_line_event event;
    unsigned int skip_cnt = 10;

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

    ret = gpiod_line_request_rising_edge_events(line, "50 Hz counter");
    if (ret < 0) {
        perror("Request line as input failed\n");
        exit(1);
    }

    while(1) {
        ret = gpiod_line_event_wait(line, &timeout);
        if (ret < 0) {
            perror("Wait event notification failed\n");
            exit(1);
        } else if (ret == 0) {
            printf("Wait event notification timeout\n");
            continue;
        }

        ret = gpiod_line_event_read(line, &event);
        if (ret < 0) {
            perror("Read last event notification failed\n");
            exit(1);
        }

        if (skip_cnt)
            skip_cnt--;
        else
            register_event(&event);
    }

    return 0;
}
