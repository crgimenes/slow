#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <getopt.h>
#include <strings.h>

/* Historical modem speed presets */
typedef struct
{
    const char *name;
    int speed;
    const char *description;
} modem_preset_t;

static const modem_preset_t modem_presets[] = {
    {"110", 110, "Teletype"},
    {"300", 300, "Acoustic coupler"},
    {"1200", 1200, "Bell 212A"},
    {"2400", 2400, "V.22bis"},
    {"4800", 4800, "V.27"},
    {"9600", 9600, "V.32"},
    {"14400", 14400, "V.32bis"},
    {"19200", 19200, "V.32fast"},
    {"28800", 28800, "V.34"},
    {"33600", 33600, "V.34+"},
    {"56000", 56000, "V.90 (56k dialup)"},
    {"acoustic", 300, "Alias for 300"},
    {"dialup", 56000, "Alias for 56000"},
    {NULL, 0, NULL}};

/* Global variables for signal handling */
static volatile int running = 1;
static long char_count = 0;
static time_t start_time;
static int verbose_mode = 0;

/* Signal handler for graceful shutdown */
void signal_handler(int signum)
{
    (void)signum; /* Suppress unused parameter warning */
    running = 0;
    if (verbose_mode)
    {
        time_t elapsed = time(NULL) - start_time;
        fprintf(stderr, "\nProcessed %ld characters in %ld seconds\n",
                char_count, elapsed);
    }
}

/* Parse BPS value with preset support */
int parse_bps(const char *bps_str)
{
    int i;
    char *endptr;
    int bps;

    /* Check if it's a preset first */
    for (i = 0; modem_presets[i].name != NULL; i++)
    {
        if (strcasecmp(bps_str, modem_presets[i].name) == 0)
        {
            return modem_presets[i].speed;
        }
    }

    /* Try to parse as number */
    bps = strtol(bps_str, &endptr, 10);
    if (*endptr != '\0' || bps <= 0)
    {
        return -1;
    }

    if (bps > 1000000)
    {
        fprintf(stderr, "Error: BPS too high (max 1,000,000), got %d\n", bps);
        return -1;
    }

    return bps;
}

/* Get available presets string for error messages */
void get_available_presets(char *buffer, size_t size)
{
    int i;
    size_t pos = 0;

    for (i = 0; modem_presets[i].name != NULL && pos < size - 1; i++)
    {
        if (i > 0 && pos < size - 3)
        {
            strncpy(buffer + pos, ", ", size - pos - 1);
            pos += 2;
        }
        size_t len = strlen(modem_presets[i].name);
        if (pos + len < size - 1)
        {
            strncpy(buffer + pos, modem_presets[i].name, size - pos - 1);
            pos += len;
        }
    }
    buffer[pos] = '\0';
}

/* Show help message */
void show_help(void)
{
    int i;

    printf("slow - simulates old terminal/modem speeds\n\n");
    printf("Usage:\n");
    printf("  slow [options]\n");
    printf("  command | slow [options]\n\n");
    printf("Options:\n");
    printf("  -b, --bps <speed>   Set speed in bits per second (default: 300)\n");
    printf("  -f, --file <file>   Read from file instead of stdin\n");
    printf("  -v, --verbose       Show timing and statistics\n");
    printf("  -h, --help          Show this help\n\n");
    printf("BPS Presets (historical modems):\n");

    for (i = 0; modem_presets[i].name != NULL; i++)
    {
        printf("  %-8s %s\n", modem_presets[i].name, modem_presets[i].description);
    }

    printf("\nExamples:\n");
    printf("  ls -al | slow\n");
    printf("  slow -f text.txt -b 1200\n");
    printf("  cat file.txt | slow -b dialup\n");
    printf("  slow -f story.txt -b acoustic -v\n");
}

/* Sleep for specified nanoseconds using nanosleep */
void sleep_ns(long nanoseconds)
{
    struct timespec req, rem;
    req.tv_sec = nanoseconds / 1000000000L;
    req.tv_nsec = nanoseconds % 1000000000L;

    while (nanosleep(&req, &rem) == -1)
    {
        if (errno == EINTR)
        {
            req = rem;
            continue;
        }
        break;
    }
}

/* Read UTF-8 character from file */
int read_utf8_char(FILE *fp, char *buffer, size_t buffer_size)
{
    int c;
    size_t bytes = 0;

    c = fgetc(fp);
    if (c == EOF)
    {
        return EOF;
    }

    buffer[bytes++] = (char)c;

    /* Determine UTF-8 sequence length */
    if ((c & 0x80) == 0)
    {
        /* ASCII character (0xxxxxxx) */
        buffer[bytes] = '\0';
        return (int)bytes;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        /* 2-byte sequence (110xxxxx) */
        if (bytes >= buffer_size - 1)
            return -1;
        c = fgetc(fp);
        if (c == EOF)
            return EOF;
        buffer[bytes++] = (char)c;
    }
    else if ((c & 0xF0) == 0xE0)
    {
        /* 3-byte sequence (1110xxxx) */
        for (int i = 1; i < 3 && bytes < buffer_size - 1; i++)
        {
            c = fgetc(fp);
            if (c == EOF)
                return EOF;
            buffer[bytes++] = (char)c;
        }
    }
    else if ((c & 0xF8) == 0xF0)
    {
        /* 4-byte sequence (11110xxx) */
        for (int i = 1; i < 4 && bytes < buffer_size - 1; i++)
        {
            c = fgetc(fp);
            if (c == EOF)
                return EOF;
            buffer[bytes++] = (char)c;
        }
    }

    buffer[bytes] = '\0';
    return (int)bytes;
}

int main(int argc, char *argv[])
{
    int bps = 300;
    char *filename = NULL;
    FILE *input_file;
    int opt;
    char char_buffer[5];
    long delay_ns;
    char presets_buffer[512];

    /* Command line options */
    static struct option long_options[] = {
        {"bps", required_argument, 0, 'b'},
        {"file", required_argument, 0, 'f'},
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    /* Parse command line arguments */
    while ((opt = getopt_long(argc, argv, "b:f:vh", long_options, NULL)) != -1)
    {
        switch (opt)
        {
        case 'b':
            bps = parse_bps(optarg);
            if (bps < 0)
            {
                get_available_presets(presets_buffer, sizeof(presets_buffer));
                fprintf(stderr, "Error: invalid BPS value '%s'. Use a number or one of the presets: %s\n",
                        optarg, presets_buffer);
                exit(EXIT_FAILURE);
            }
            break;
        case 'f':
            filename = optarg;
            break;
        case 'v':
            verbose_mode = 1;
            break;
        case 'h':
            show_help();
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, "Use -h or --help for usage information\n");
            exit(EXIT_FAILURE);
        }
    }

    /* Open input file if specified */
    if (filename != NULL)
    {
        input_file = fopen(filename, "r");
        if (input_file == NULL)
        {
            fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        input_file = stdin;
    }

    /* Setup signal handling */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /* Calculate delay: 8 bits per byte at specified bps */
    /* delay_ns = (8 * 1e9) / bps nanoseconds */
    delay_ns = (8L * 1000000000L) / bps;

    if (verbose_mode)
    {
        fprintf(stderr, "Simulating %d bps (%d bytes/sec)\n", bps, bps / 8);
    }

    start_time = time(NULL);

    /* Main processing loop */
    while (running)
    {
        int char_bytes = read_utf8_char(input_file, char_buffer, sizeof(char_buffer));

        if (char_bytes == EOF)
        {
            if (verbose_mode)
            {
                time_t elapsed = time(NULL) - start_time;
                fprintf(stderr, "\nCompleted: %ld characters in %ld seconds\n",
                        char_count, elapsed);
            }
            break;
        }

        if (char_bytes < 0)
        {
            fprintf(stderr, "Error reading character\n");
            break;
        }

        /* Sleep for the calculated delay */
        sleep_ns(delay_ns);

        /* Output the character */
        printf("%s", char_buffer);
        fflush(stdout);
        char_count++;
    }

    /* Clean up */
    if (input_file != stdin && input_file != NULL)
    {
        if (fclose(input_file) != 0)
        {
            fprintf(stderr, "Warning: error closing file: %s\n", strerror(errno));
        }
    }

    return EXIT_SUCCESS;
}