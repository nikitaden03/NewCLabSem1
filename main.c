#include <stdio.h>
#include <getopt.h>
#include <malloc.h>
#include <stdlib.h>

typedef struct MyConfiguration {
    char *input_name, *output_name;
    int max_iter, dump_freq;
} MyConfiguration;

MyConfiguration *myConfiguration;

int main(int argc, char *argv[]) {
    myConfiguration = (MyConfiguration *) calloc(1, sizeof(MyConfiguration));
    int c;
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
                {"input", 1, 0, 0},
                {"output",     1, 0, 1},
                {"max_iter",      1, 0, 2},
                {"dump_freq",    1, 0, 3}
        };
        c = getopt_long(argc, argv, "1",
                        long_options, &option_index);
        char *help_end = NULL;
        if (c == -1)
            break;

        switch (c) {
            case 0:
                myConfiguration->input_name = optarg;
                break;
            case 1:
                myConfiguration->output_name = optarg;
                break;
            case 2:
                myConfiguration->max_iter = strtol(optarg, &help_end, 10);
                break;
            case 3:
                myConfiguration->dump_freq = strtol(optarg, &help_end, 10);
                break;
            default:
                fprintf(stderr, "The program get a wrong parameter");
                exit(1);
        }
    }
    printf("%s, %s, %d, %d", myConfiguration->input_name, myConfiguration->output_name, myConfiguration->max_iter, myConfiguration->dump_freq);
    return 0;
}
