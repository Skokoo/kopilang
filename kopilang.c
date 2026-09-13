#include "syntax/main_syn.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("\033[1;31minvalid argument\033[0m usage: %s [file .kopi]\n", argv[0]);
        return 1;
    }

    FILE *in = fopen(argv[1], "r");
    if (!in) {
        printf("\033[1;31mwhere is the file?\033[0m file '%s' not found\n", argv[1]);
        return 1;
    }

    static char lines_array[1000][256];
    int total_lines = 0;
    while (fgets(lines_array[total_lines], sizeof(lines_array[0]), in)) {
        lines_array[total_lines][strcspn(lines_array[total_lines], "\r\n")] = '\0';
        total_lines++;
        if (total_lines >= 1000) break;
    }
    fclose(in);

    const int max_var = 50;
    Var db[max_var];
    int count = 0, has_err = 0;
    int condition_status = 1;
    int inside_if_block = 0;

    for (int i = 0; i < total_lines; i++) {
        static char raw_backup[256];
        strcpy(raw_backup, lines_array[i]);

        char *cleaned = trim(lines_array[i]);
        int current_line_num = i + 1;

        if (strlen(cleaned) == 0) continue;

        int len_str = strlen(cleaned);
        if (cleaned[len_str - 1] == ',') {
            cleaned[len_str - 1] = '\0';
            cleaned = trim(cleaned);
        } else if (i < total_lines - 1) {
            int has_next_code = 0;
            for (int j = i + 1; j < total_lines; j++) {
                if (strlen(trim(lines_array[j])) > 0) {
                    has_next_code = 1;
                    break;
                }
            }
            if (has_next_code) {
                printf("\033[1m%s:%d:%d: \033[1;35mwarning:\033[0m\033[1m expected ',' at end of statement\033[0m\n", argv[1], current_line_num, len_str + 1);
                printf(" |\n | %s\n |\n", raw_backup);
            }
        }

        if (strcmp(cleaned, ">") == 0) {
            inside_if_block = 0;
            condition_status = 1;
            continue;
        }

        if (inside_if_block && !condition_status) {
            continue;
        }

        if (strncmp(cleaned, "INTEGER", 7) == 0) {
            do_int(argv[1], raw_backup, cleaned, db, &count, current_line_num, &has_err);
        } else if (strncmp(cleaned, "SAY", 3) == 0) {
            do_say(argv[1], raw_backup, cleaned, db, count, current_line_num, &has_err);
        } else if (strncmp(cleaned, "IF", 2) == 0) {
            inside_if_block = 1;
            do_if(argv[1], raw_backup, cleaned, db, count, current_line_num, &has_err, &condition_status);
        } else if (strncmp(cleaned, "INPUT", 5) == 0) {
            do_input(argv[1], raw_backup, cleaned, db, count, current_line_num, &has_err);
        } else {
            printf("\033[1m%s:%d:1: \033[1;31msyntax error:\033[0m\033[1m unknown token -> \"%s\"\033[0m\n", argv[1], current_line_num, cleaned);
            printf(" |\n | %s\n |\n", raw_backup);
            has_err = 1;
        }
    }

    if (has_err) {
        printf("\nexecution aborted due to previous errors.\n");
        return 1;
    }
    return 0;
}