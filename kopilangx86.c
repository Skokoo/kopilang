#include "syntax/x86_syn.h"

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
    int tl = 0;
    while (fgets(lines_array[tl], sizeof(lines_array[tl]), in)) {
        lines_array[tl][strcspn(lines_array[tl], "\r\n")] = '\0';
        tl++;
        if (tl >= 1000) break;
    }
    fclose(in);

    const int mv = 50;
    Var db[mv];
    int cn = 0, er = 0;
    int st = 1;
    int bk = 0;

    for (int i = 0; i < tl; i++) {
        static char rw[256];
        strcpy(rw, lines_array[i]);

        char *cl = trim(lines_array[i]);
        int ln = i + 1;

        if (strlen(cl) == 0) continue;

        int ls = strlen(cl);
        if (cl[ls - 1] == ',') {
            cl[ls - 1] = '\0';
            cl = trim(cl);
        } else if (i < tl - 1) {
            int nx = 0;
            for (int j = i + 1; j < tl; j++) {
                if (strlen(trim(lines_array[j])) > 0) {
                    nx = 1;
                    break;
                }
            }
            if (nx) {
                printf("\033[1m%s:%d:%d: \033[1;35mwarning:\033[0m\033[1m expected ',' at end of statement\033[0m\n", argv[1], ln, ls + 1);
                printf(" |\n | %s\n |\n", rw);
            }
        }

        if (strcmp(cl, ">") == 0) {
            bk = 0;
            st = 1;
            continue;
        }

        if (bk && !st) {
            continue;
        }

        if (strncmp(cl, "INTEGER", 7) == 0) {
            do_int_x86(argv[1], rw, cl, db, &cn, ln, &er);
        } else if (strncmp(cl, "SAY", 3) == 0) {
            do_say_x86(argv[1], rw, cl, db, cn, ln, &er);
        } else if (strncmp(cl, "IF", 2) == 0) {
            bk = 1;
            do_if_x86(argv[1], rw, cl, db, cn, ln, &er, &st);
        } else if (strncmp(cl, "INPUT", 5) == 0) {
            do_input_x86(argv[1], rw, cl, db, cn, ln, &er);
        } else {
            printf("\033[1m%s:%d:1: \033[1;31msyntax error:\033[0m\033[1m unknown token -> \"%s\"\033[0m\n", argv[1], ln, cl);
            printf(" |\n | %s\n |\n", rw);
            er = 1;
        }
    }

    if (er) {
        printf("\nexecution aborted due to previous errors.\n");
        return 1;
    }
    return 0;
}