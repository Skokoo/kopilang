#ifndef X86_SYN_H
#define X86_SYN_H

#include "../error/err.h"

long x86_add(long a, long b) { long r; __asm__("add %2, %0" : "=r"(r) : "0"(a), "r"(b)); return r; }
long x86_sub(long a, long b) { long r; __asm__("sub %2, %0" : "=r"(r) : "0"(a), "r"(b)); return r; }
long x86_mul(long a, long b) { long r; __asm__("imul %2, %0" : "=r"(r) : "0"(a), "r"(b)); return r; }
long x86_div(long a, long b) { long q, r; __asm__("cqo\n idiv %3" : "=a"(q), "=d"(r) : "a"(a), "r"(b)); return q; }

void x86_put(char *s) {
    long ln = strlen(s);
    s[ln] = '\n';
    ln++;
    __asm__ __volatile__ (
        "mov $1, %%rax\n"
        "mov $1, %%rdi\n"
        "mov %0, %%rsi\n"
        "mov %1, %%rdx\n"
        "syscall\n"
        : : "r"(s), "r"(ln) : "rax", "rdi", "rsi", "rdx", "memory"
    );
}

void x86_num(long v) {
    char bf[32];
    int i = 30;
    bf[31] = '\n';
    long tm = v;
    if (v < 0) tm = -v;
    do {
        bf[i--] = (tm % 10) + '0';
        tm /= 10;
    } while (tm > 0);
    if (v < 0) bf[i--] = '-';
    char *pt = &bf[i + 1];
    long ln = 31 - i;
    __asm__ __volatile__ (
        "mov $1, %%rax\n"
        "mov $1, %%rdi\n"
        "mov %0, %%rsi\n"
        "mov %1, %%rdx\n"
        "syscall\n"
        : : "r"(pt), "r"(ln) : "rax", "rdi", "rsi", "rdx", "memory"
    );
}

long x86_get() {
    char bf[32] = {0};
    long vl = 0;
    int sg = 1;
    long ln = 0;
    char *pt = bf;
    __asm__ __volatile__ (
        "mov $0, %%rax\n"
        "mov $0, %%rdi\n"
        "mov %1, %%rsi\n"
        "mov $32, %%rdx\n"
        "syscall\n"
        "mov %%rax, %0\n"
        : "=r"(ln) : "r"(pt) : "rax", "rdi", "rsi", "rdx", "memory"
    );
    int i = 0;
    while (i < ln && isspace((unsigned char)bf[i])) i++;
    if (i < ln && bf[i] == '-') { sg = -1; i++; }
    else if (i < ln && bf[i] == '+') { i++; }
    while (i < ln && isdigit((unsigned char)bf[i])) {
        vl = vl * 10 + (bf[i] - '0');
        i++;
    }
    return vl * sg;
}

void do_int_x86(char *fn, char *rw, char *cm, Var *db, int *cn, int l, int *er) {
    char nm[64] = {0}, ex[256] = {0};
    if (sscanf(cm, "INTEGER %63s = %255[^\n]", nm, ex) != 2) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m invalid INTEGER format\033[0m\n", fn, l);
        printf(" |\n | %s\n |\n", rw);
        *er = 1;
        return;
    }
    char *cnm = trim(nm);
    char *cex = trim(ex);
    char op = 0;
    if (strchr(cex, '+')) op = '+';
    else if (strchr(cex, '-')) op = '-';
    else if (strchr(cex, '*')) op = '*';
    else if (strchr(cex, '/')) op = '/';

    if (op != 0) {
        char a1[64] = {0}, a2[64] = {0}, fm[32];
        sprintf(fm, "%%63s %c %%63s", op);
        sscanf(cex, fm, a1, a2);
        int i1 = get_idx(db, *cn, trim(a1));
        int i2 = get_idx(db, *cn, trim(a2));
        if (i1 == -1) { printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", fn, l, a1); *er = 1; }
        if (i2 == -1) { printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", fn, l, a2); *er = 1; }
        if (op == '/' && i2 != -1 && db[i2].val == 0) { printf("\033[1m%s:%d:1: \033[1;31mmath error:\033[0m\033[1m division by zero\033[0m\n", fn, l); *er = 1; return; }
        if (*er) return;
        strcpy(db[*cn].name, cnm);
        if (op == '+') db[*cn].val = x86_add(db[i1].val, db[i2].val);
        else if (op == '-') db[*cn].val = x86_sub(db[i1].val, db[i2].val);
        else if (op == '*') db[*cn].val = x86_mul(db[i1].val, db[i2].val);
        else if (op == '/') db[*cn].val = x86_div(db[i1].val, db[i2].val);
        (*cn)++;
    } else {
        for (int i = 0; cex[i]; i++) {
            if (!isdigit((unsigned char)cex[i]) && cex[i] != '-') {
                printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m value must be numeric\033[0m\n", fn, l);
                *er = 1;
                return;
            }
        }
        strcpy(db[*cn].name, cnm);
        db[*cn].val = atol(cex);
        (*cn)++;
    }
}

void do_say_x86(char *fn, char *rw, char *cm, Var *db, int cn, int l, int *er) {
    char *q1 = strchr(cm, '"');
    char *q2 = strrchr(cm, '"');
    if (!q1 || !q2 || q1 == q2) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m matching quotes required\033[0m\n", fn, l);
        *er = 1;
        return;
    }
    int rl = q2 - q1 - 1;
    char ct[512] = {0};
    if (rl > 510) rl = 510;
    strncpy(ct, q1 + 1, rl);
    
    // PERBAIKAN FATAL: Menambahkan alokasi ukuran array siku [512]
    static char pr[512];
    memset(pr, 0, sizeof(pr));
    int pi = 0;
    for (int i = 0; ct[i] != '\0'; i++) {
        if (ct[i] == '\\' && ct[i+1] == 'e') { pr[pi++] = 0x1B; i++; }
        else if (ct[i] == '\\' && ct[i+1] == 'n') { pr[pi++] = '\n'; i++; }
        else if (ct[i] == '!') {
            char tg[64] = {0};
            int vl = 0;
            int st = i + 1;
            while (ct[st] != '\0' && !isspace((unsigned char)ct[st]) && ct[st] != '\\' && ct[st] != '"' && vl < 63) {
                tg[vl++] = ct[st++];
            }
            tg[vl] = '\0';
            int id = get_idx(db, cn, tg);
            if (id != -1) {
                char nb[32];
                sprintf(nb, "%ld", db[id].val);
                for (int b = 0; nb[b] != '\0'; b++) pr[pi++] = nb[b];
                i += vl;
            } else { pr[pi++] = '!'; }
        } else { pr[pi++] = ct[i]; }
    }
    x86_put(pr);
}

void do_if_x86(char *fn, char *rw, char *cm, Var *db, int cn, int l, int *er, int *ab) {
    char cp[256] = {0};
    if (sscanf(cm, "IF %[^\n]", cp) != 1) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m invalid IF syntax\033[0m\n", fn, l);
        *er = 1;
        return;
    }
    char *cc = trim(cp);
    int ln = strlen(cc);
    if (ln < 2 || cc[ln - 1] != '<') {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m expected '<' block opener\033[0m\n", fn, l);
        *er = 1;
        return;
    }
    cc[ln - 1] = '\0';
    cc = trim(cc);
    char vn[64] = {0}, op[8] = {0};
    long vv = 0;
    if (strstr(cc, "==")) { strcpy(op, "=="); sscanf(cc, "%63s == %ld", vn, &vv); }
    else if (strstr(cc, "!=")) { strcpy(op, "!="); sscanf(cc, "%63s != %ld", vn, &vv); }
    else if (strstr(cc, ">=")) { strcpy(op, ">="); sscanf(cc, "%63s >= %ld", vn, &vv); }
    else if (strstr(cc, "<=")) { strcpy(op, "<="); sscanf(cc, "%63s <= %ld", vn, &vv); }
    else if (strchr(cc, '>')) { strcpy(op, ">"); sscanf(cc, "%63s > %ld", vn, &vv); }
    else if (strchr(cc, '<')) { strcpy(op, "<"); sscanf(cc, "%63s < %ld", vn, &vv); }
    int id = get_idx(db, cn, trim(vn));
    if (id == -1) { printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", fn, l, vn); *er = 1; return; }
    long v1 = db[id].val;
    long ps = 0;
    if (strcmp(op, "==") == 0) { __asm__("cmp %2, %1\n sete %%al\n movzbl %%al, %0" : "=r"(ps) : "r"(v1), "r"(vv) : "cc"); }
    else if (strcmp(op, "!=") == 0) { __asm__("cmp %2, %1\n setne %%al\n movzbl %%al, %0" : "=r"(ps) : "r"(v1), "r"(vv) : "cc"); }
    else if (strcmp(op, ">=") == 0) { __asm__("cmp %2, %1\n setge %%al\n movzbl %%al, %0" : "=r"(ps) : "r"(v1), "r"(vv) : "cc"); }
    else if (strcmp(op, "<=") == 0) { __asm__("cmp %2, %1\n setle %%al\n movzbl %%al, %0" : "=r"(ps) : "r"(v1), "r"(vv) : "cc"); }
    else if (strcmp(op, ">") == 0) { __asm__("cmp %2, %1\n setg %%al\n movzbl %%al, %0" : "=r"(ps) : "r"(v1), "r"(vv) : "cc"); }
    else if (strcmp(op, "<") == 0) { __asm__("cmp %2, %1\n setl %%al\n movzbl %%al, %0" : "=r"(ps) : "r"(v1), "r"(vv) : "cc"); }
    *ab = ps ? 1 : 0;
}

void do_input_x86(char *fn, char *rw, char *cm, Var *db, int cn, int l, int *er) {
    char *q1 = strchr(cm, '"');
    char *q2 = strrchr(cm, '"');
    if (!q1 || !q2 || q1 == q2) { printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m matching quotes required for INPUT\033[0m\n", fn, l); *er = 1; return; }
    int ln = q2 - q1 - 1;
    char tg[256] = {0};
    if (ln > 255) ln = 255;
    strncpy(tg, q1 + 1, ln);
    char *ctg = trim(tg);
    if (ctg[0] != '!') { printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m INPUT target must start with '!'\033[0m\n", fn, l); *er = 1; return; }
    char *vn = trim(ctg + 1);
    int id = get_idx(db, cn, vn);
    if (id == -1) { printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", fn, l, vn); *er = 1; return; }
    db[id].val = x86_get();
}

#endif