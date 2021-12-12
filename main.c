#include <stdio.h>
#include <getopt.h>
#include <malloc.h>
#include <stdlib.h>

typedef struct MyConfiguration {
    char *input_name, *output_name;
    int max_iter, dump_freq;
} MyConfiguration;

typedef struct MyBmpInstance {
    char type_object[2], reserved1[2], reserved2[2], planes[2], bit_count[2], resolution[8];
    int size_file, offset_bits, size_image_header, width, height, compression, size_image, colors_used, colors_important;
} MyBmpInstance;

MyConfiguration *myConfiguration;
MyBmpInstance *myBmpInstance;
unsigned char *game_map;

int parseImage() {
    FILE *file = fopen(myConfiguration->input_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "File does not exist!");
        return 1;
    }
    fread(myBmpInstance->type_object, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->size_file, sizeof(int), 1, file);
    fread(&myBmpInstance->reserved1, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->reserved2, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->offset_bits, sizeof(int), 1, file);
    fread(&myBmpInstance->size_image_header, sizeof(int), 1, file);
    fread(&myBmpInstance->width, sizeof(int), 1, file);
    fread(&myBmpInstance->height, sizeof(int), 1, file);
    fread(&myBmpInstance->planes, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->bit_count, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->compression, sizeof(int), 1, file);
    fread(&myBmpInstance->size_image, sizeof(int), 1, file);
    fread(&myBmpInstance->resolution, sizeof(char) * 8, 1, file);
    fread(&myBmpInstance->colors_used, sizeof(int), 1, file);
    fread(&myBmpInstance->colors_important, sizeof(int), 1, file);
    if (myBmpInstance->bit_count[0] != 1) {
        printf("Picture must be monochrome!");
        return 2;
    }
    game_map = malloc(sizeof(char) * myBmpInstance->width * myBmpInstance->height);
    int help, i = 0;
    while (i < myBmpInstance->width * myBmpInstance->height) {
        help = fgetc(file);
        while (help > 0) {
            game_map[i] = help % 2;
            help /= 2;
            i++;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    myConfiguration = (MyConfiguration *) calloc(1, sizeof(MyConfiguration));
    int c;
    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
                {"input",     1, 0, 0},
                {"output",    1, 0, 1},
                {"max_iter",  1, 0, 2},
                {"dump_freq", 1, 0, 3}
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

    myBmpInstance = (MyBmpInstance *) calloc(sizeof(MyBmpInstance), 1);

    int result = parseImage();
    if (result != 0) {
        return 0;
    }

    return 0;
}
