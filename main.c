#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <getopt.h>

typedef struct MyFrame {
    int size_frame, flag1, flag2;
    char *name;
    char *content;
} MyFrame;

typedef struct MyFile {
    FILE *file;
    int size_frames, count;
    MyFrame *myFrame;
} MyFile;

void init_array(MyFile *myFile, int count) {
    myFile->myFrame = calloc(count, sizeof(MyFrame));
}

void resize_array(MyFile *myFile, int count) {
    myFile->myFrame = realloc(myFile->myFrame, sizeof(MyFrame) * count);
}

void init_file(char *str, MyFile *myFile) {
    FILE *file = fopen(str, "rb");
    if (file == NULL) {
        fprintf(stderr, "File does not exist!");
        exit(1);
    }
    myFile->file = file;
}

int get_size_tag(MyFile *myFile) {
    char header[3];
    for (int i = 0; i < 3; i++) {
        header[i] = (char) fgetc(myFile->file);
    }
    if (strncmp("ID3", header, strlen("ID3")) != 0) {
        return 0;
    }
    fseek(myFile->file, 3, SEEK_CUR);
    size_t byte0 = 128 * 128 * 128 * fgetc(myFile->file);
    size_t byte1 = 128 * 128 * fgetc(myFile->file);
    size_t byte2 = 128 * fgetc(myFile->file);
    size_t byte3 = fgetc(myFile->file);
    myFile->size_frames = (int) (byte0 + byte1 + byte2 + byte3);
    return 1;
}

int parse_frame(MyFile *myFile, int counter) {
    MyFrame *myFrame = calloc(1, sizeof(MyFrame));
    myFrame->name = calloc(4, sizeof(char));
    for (int i = 0; i < 4; i++) {
        myFrame->name[i] = (char) fgetc(myFile->file);
        if (!(myFrame->name[i] <= 'Z' && myFrame->name[i] >= 'A' ||
              myFrame->name[i] <= '9' && myFrame->name[i] >= '0')) {
            free(myFrame->name);
            free(myFrame);
            return 0;
        }
    }
    size_t byte0 = 128 * 128 * 128 * fgetc(myFile->file);
    size_t byte1 = 128 * 128 * fgetc(myFile->file);
    size_t byte2 = 128 * fgetc(myFile->file);
    size_t byte3 = fgetc(myFile->file);
    myFrame->size_frame = (int) (byte0 + byte1 + byte2 + byte3);
    myFrame->flag1 = fgetc(myFile->file);
    myFrame->flag2 = fgetc(myFile->file);
    myFrame->content = (char *) calloc(myFrame->size_frame, sizeof(char));
    for (int i = 0; i < myFrame->size_frame; i++) {
        myFrame->content[i] = (char) fgetc(myFile->file);
    }
    if (strncmp("APIC", myFrame->name, strlen("APIC")) == 0) {
        return 0;
    }
    myFile->myFrame[counter] = *myFrame;
    return 1;
}

void parse_frames(MyFile *myFile) {
    int counter = 0, total = 4;
    init_array(myFile, total);
    while (ftell(myFile->file) < myFile->size_frames) {
        if (parse_frame(myFile, counter))
            counter++;
        if (counter == total) {
            total *= 2;
            resize_array(myFile, total);
            if (myFile->myFrame == NULL) {
                fprintf(stderr, "Not enough memory!");
                exit(1);
            }
        }
    }
    myFile->count = counter;
}

void show_tags(MyFile *myFile) {
    for (int i = 0; i < myFile->count; i++) {
        printf("%s -", myFile->myFrame[i].name);
        if (myFile->myFrame[i].content[0] != '\0') {
            printf(" ");
        }
        if (strcmp(myFile->myFrame->name, "APIC") == 0) continue;
        for (int j = 0; j < myFile->myFrame[i].size_frame; j++) {
            printf("%c", myFile->myFrame[i].content[j]);
        }
        printf("\n");
    }
}

void show_tag(MyFile *myFile, const char *tag_name) {
    for (int i = 0; i < myFile->count; i++) {
        if (myFile->myFrame[i].name[0] == tag_name[0] && myFile->myFrame[i].name[1] == tag_name[1] &&
            myFile->myFrame[i].name[3] == tag_name[3] && myFile->myFrame[i].name[2] == tag_name[2] &&
            strlen(tag_name) == 4) {
            printf("%s -", myFile->myFrame[i].name);
            if (myFile->myFrame[i].content[0] != '\0') {
                printf(" ");
            }
            for (int j = 0; j < myFile->myFrame[i].size_frame; j++) {
                printf("%c", myFile->myFrame[i].content[j]);
            }
            return;
        }
    }
    printf("Tag with this name does not exist!");
}

void set_tag(MyFile *myFile, const char *tag_name, const char *tag_value) {
    for (int i = 0; i < myFile->count; i++) {
        if (myFile->myFrame[i].name[0] == tag_name[0] && myFile->myFrame[i].name[1] == tag_name[1] &&
            myFile->myFrame[i].name[3] == tag_name[3] && myFile->myFrame[i].name[2] == tag_name[2] &&
            strlen(tag_name) == 4) {
            myFile->myFrame[i].content = (char *) tag_value;
            return;
        }
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "Russian");

    int c, help = 0;

    MyFile *myFile = calloc(1, sizeof(MyFile));

    while (1) {
        int option_index = 0;
        static struct option long_options[] = {
                {"filepath", 1, 0, 0},
                {"show",     0, 0, 1},
                {"set",      1, 0, 2},
                {"value",    1, 0, 3},
                {"get",      1, 0, 4}
        };
        c = getopt_long(argc, argv, "1",
                        long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 0:
                init_file(optarg, myFile);
                if (get_size_tag(myFile) == 0) {
                    fprintf(stderr, "FIle has an invalid structure!");
                    return 1;
                }
                break;
            case 1:
                parse_frames(myFile);
                show_tags(myFile);
                break;
            case 2:
                help++;
                char *label = optarg;
                c = getopt_long(argc, argv, "1",
                                long_options, &option_index);
                if (c != 3)
                    break;
                char *value = optarg;
                show_tag(myFile, label);
                printf("\n");
                set_tag(myFile, label, value);
                printf("\n");
                show_tag(myFile, label);
                break;
            case 4:
                parse_frames(myFile);
                show_tag(myFile, optarg);
                break;
            default:
                break;
        }
    }
    fclose(myFile->file);
    return 0;
}
