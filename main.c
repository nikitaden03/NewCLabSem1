#include <stdio.h>
#include <getopt.h>
#include <malloc.h>
#include <stdlib.h>
#include <dirent.h>

const int SIZE_TABLE = 25013;

typedef struct MyConfiguration {
    char *input_name, *output_name;
    int max_iter, dump_freq;
} MyConfiguration;

typedef struct MyBmpInstance {
    char type_object[2], reserved1[2], reserved2[2], planes[2], bit_count[2], resolution[8];
    int size_file, offset_bits, size_image_header, width, height, compression, size_image, colors_used, colors_important;
    char *useless_staff;
} MyBmpInstance;

MyConfiguration *myConfiguration;
MyBmpInstance *myBmpInstance;
unsigned char game_map[560][560];

unsigned char *hash_table;

/**
 * @brief Проверяет, существует ли директория. В противном случае создает.
 */
void check_dir() {
    DIR *dir = opendir(myConfiguration->output_name);
    if (dir == NULL) {
        mkdir(myConfiguration->output_name);
    }
    closedir(dir);
}

/**
 * @brief Генерирует строчку вида путь к папке/Generation номер поколения.
 * @return Возвращает путь к файлу
 */
char *generate_name() {
    static int num_picture = 0;
    num_picture++;
    char *num_pictures_str = calloc(1000, 1);
    char *label = calloc(1000, 1);
    strcat(label, myConfiguration->output_name);
    strcat(label, "/Generation ");
    sprintf(num_pictures_str, "%d", num_picture);
    strcat(label, num_pictures_str);
    strcat(label, ".bmp");
    free(num_pictures_str);
    return label;
}

/**
 * @brief Записывает числа в заголовок bmp.
 * @param file Объект открытого файла bmp
 * @param num Число, которое надо напечатать.
 */
void print_num(FILE *file, int num) {
    char byte0 = (char) (num / (256 * 256 * 256));
    char byte1 = (char) (num / (256 * 256) % 256);
    char byte2 = (char) (num / 256 % 256);
    char byte3 = (char) (num % 256);
    fprintf(file, "%c%c%c%c", byte3, byte2, byte1, byte0);
}

/**
 * @brief Генерирует 2 значения хеша.
 * @param hash1 Значение первого хеша
 * @param hash2 Значение второго хеша
 */
void get_hash(int *hash1, int *hash2) {
    long long key1 = 1, key2 = 1;
    for (int i = 0; i < myBmpInstance->height; i++) {
        for (int j = 0; j < ((myBmpInstance->width + 31) / 32 * 32); j++) {
            *hash1 = (int) (*hash1 + game_map[i][j] * key1) % SIZE_TABLE;
            *hash2 = (int) (*hash2 + game_map[i][j] * key2) % SIZE_TABLE;
            key1++;
            key2 = (key2 + 3) % INT_MAX;
        }
    }
}

/**
 * @brief Инициализирует хеш таблицу
 * @return Возвращает код операции, показывающий успех выделения памяти
 */
int init_table() {
    hash_table = (unsigned char *) calloc(sizeof(unsigned char), SIZE_TABLE * SIZE_TABLE);
    if (hash_table == NULL) {
        fprintf(stderr, "There is not enough memory!");
        return 1;
    }
    return 0;
}

/**
 * @brief Парсит изображение из файла, записывает основные данные хедера в структуру.
 * @return Возвращает код операции, показывающий успех чтения картинки
 */
int parseImage() {
    FILE *file = fopen(myConfiguration->input_name, "rb");
    if (file == NULL) {
        fprintf(stderr, "File does not exist!");
        return 1;
    }
    fread(myBmpInstance->type_object, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->size_file, sizeof(int), 1, file);
    fread(myBmpInstance->reserved1, sizeof(char) * 2, 1, file);
    fread(myBmpInstance->reserved2, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->offset_bits, sizeof(int), 1, file);
    fread(&myBmpInstance->size_image_header, sizeof(int), 1, file);
    fread(&myBmpInstance->width, sizeof(int), 1, file);
    fread(&myBmpInstance->height, sizeof(int), 1, file);
    fread(myBmpInstance->planes, sizeof(char) * 2, 1, file);
    fread(myBmpInstance->bit_count, sizeof(char) * 2, 1, file);
    fread(&myBmpInstance->compression, sizeof(int), 1, file);
    fread(&myBmpInstance->size_image, sizeof(int), 1, file);
    fread(myBmpInstance->resolution, sizeof(char) * 8, 1, file);
    fread(&myBmpInstance->colors_used, sizeof(int), 1, file);
    fread(&myBmpInstance->colors_important, sizeof(int), 1, file);
    myBmpInstance->useless_staff = (char *) malloc(sizeof(char) * myBmpInstance->offset_bits - 54);
    fread(myBmpInstance->useless_staff, sizeof(char), myBmpInstance->offset_bits - 54, file);
    if (myBmpInstance->bit_count[0] != 1) {
        printf("Picture must be monochrome!");
        return 2;
    }

    for (int i = 0; i < 500; i++) {
        for (int j = 0; j < 500; j++) {
            game_map[i][j] = 1;
        }
    }

    int help, i = 0, j = 0;

    while (i * myBmpInstance->height + j < myBmpInstance->size_image * 8) {
        help = fgetc(file);
        game_map[i][j] = help / (128);
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help / (64) % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help / (32) % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help / (16) % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help / 8 % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help / 4 % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help / 2 % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
        game_map[i][j] = help % 2;
        j++;
        if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
            j = 0;
            i++;
        }
    }

    fclose(file);
    return 0;
}

/**
 * @brief Сохраняет данные об игре в картинку, имя которой получает в функции выше.
 */
void save_image() {
    check_dir();
    FILE *file = fopen(generate_name(), "wb");
    fwrite(myBmpInstance->type_object, sizeof(char), 2, file);
    print_num(file, myBmpInstance->size_file);
    fwrite(&myBmpInstance->reserved1, sizeof(char), 2, file);
    fwrite(&myBmpInstance->reserved2, sizeof(char), 2, file);
    print_num(file, myBmpInstance->offset_bits);
    print_num(file, myBmpInstance->size_image_header);
    print_num(file, myBmpInstance->width);
    print_num(file, myBmpInstance->height);
    fwrite(&myBmpInstance->planes, sizeof(char), 2, file);
    fwrite(&myBmpInstance->bit_count, sizeof(char), 2, file);
    print_num(file, myBmpInstance->compression);
    print_num(file, myBmpInstance->size_image);
    fwrite(&myBmpInstance->resolution, sizeof(char), 8, file);
    print_num(file, myBmpInstance->colors_used);
    print_num(file, myBmpInstance->colors_important);

    unsigned char help;

    for (int i = 0; i < myBmpInstance->offset_bits - 54; i++) {
        fprintf(file, "%c", myBmpInstance->useless_staff[i]);
    }

    int j = 0, i = 0;

    while (i * myBmpInstance->height + j < myBmpInstance->size_image * 8) {
        help = 0;
        unsigned char k = (unsigned char) 128, c = 0;
        while (c < 8) {
            help += (char) (k * game_map[i][j]);
            k /= 2;
            c++;
            j++;
            if (j == ((myBmpInstance->width + 31) / 32 * 32)) {
                j = 0;
                i++;
            }
        }
        fprintf(file, "%c", help);
    }
    fclose(file);
}

/**
 * @brief Генерирует следующее поколение игры, также записывает его в хеш функцию и, в случае, если такая расстановка
 * была, прекращает игры
 */
void make_step() {

    unsigned char game_map_new[560][560];

    for (int i = 0; i < myBmpInstance->height; i++) {
        for (int j = 0; j < ((myBmpInstance->width + 31) / 32 * 32); j++) {
            if (i == 5 && j == 74) {}
            int count_live_cells = 0;
            if (i - 1 >= 0 && j - 1 >= 0 && game_map[i - 1][j - 1] == 0)
                count_live_cells++;

            if (i - 1 >= 0 && game_map[i - 1][j] == 0)
                count_live_cells++;

            if (i - 1 >= 0 && j + 1 < (myBmpInstance->width + 31) / 32 * 32 && game_map[i - 1][j + 1] == 0)
                count_live_cells++;

            if (j - 1 >= 0 && game_map[i][j - 1] == 0)
                count_live_cells++;

            if (j + 1 < (myBmpInstance->width + 31) / 32 * 32 && game_map[i][j + 1] == 0)
                count_live_cells++;

            if (i + 1 < myBmpInstance->height && j - 1 >= 0 && game_map[i + 1][j - 1] == 0)
                count_live_cells++;

            if (i + 1 < myBmpInstance->height && game_map[i + 1][j] == 0)
                count_live_cells++;

            if (i + 1 < myBmpInstance->height && j + 1 < (myBmpInstance->width + 31) / 32 * 32 &&
                game_map[i + 1][j + 1] == 0)
                count_live_cells++;
            if (game_map[i][j] == 1 && count_live_cells == 3 ||
                game_map[i][j] == 0 && (count_live_cells == 3 || count_live_cells == 2)) {
                game_map_new[i][j] = 0;
            } else {
                game_map_new[i][j] = (unsigned char) 1;
            }
        }
    }

    for (int i = 0; i < myBmpInstance->height; i++) {
        for (int j = 0; j < ((myBmpInstance->width + 31) / 32 * 32); j++) {
            game_map[i][j] = game_map_new[i][j];
        }
    }

    int hash1 = 0, hash2 = 0;
    get_hash(&hash1, &hash2);
    if (hash_table[hash1 * SIZE_TABLE + hash2] == 1) {
        exit(0);
    }

    hash_table[hash1 * SIZE_TABLE + hash2] = 1;

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

    result = init_table();

    if (result != 0) {
        return 0;
    }

    int i = 0;
    while ((i < myConfiguration->max_iter || (myConfiguration->max_iter == 0))) {
        make_step();
        if (i % myConfiguration->dump_freq == 0)
            save_image();
        i++;
    }

    return 0;
}
