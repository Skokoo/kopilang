#ifndef ERR_H
#define ERR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char name[64];
    long val;
} Var;

typedef long (*AsmOp)(long, long);

long asm_add(long a, long b) { long r; __asm__("add %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); return r; }
long asm_sub(long a, long b) { long r; __asm__("sub %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); return r; }
long asm_mul(long a, long b) { long r; __asm__("mul %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); return r; }
long asm_div(long a, long b) { long r; __asm__("sdiv %0, %1, %2" : "=r"(r) : "r"(a), "r"(b)); return r; }

char *trim(char *s) {
    char *e;
    while(isspace((unsigned char)*s)) s++;
    if(*s == 0) return s;
    e = s + strlen(s) - 1;
    while(e > s && isspace((unsigned char)*e)) e--;
    *(e + 1) = '\0';
    return s;
}

int get_idx(Var *db, int n, const char *name) {
    for (int i = 0; i < n; i++) {
        if (strcmp(db[i].name, name) == 0) return i;
    }
    return -1;
}

void print_num(long val) {
    __asm__ __volatile__ (
        "sub sp, sp, #32\n"
        "mov x1, sp\n"
        "add x1, x1, #30\n"
        "mov w2, #10\n"
        "strb w2, [x1]\n"
        "1:\n"
        "mov x2, #10\n"
        "udiv x3, %0, x2\n"
        "msub x4, x3, x2, %0\n"
        "add w4, w4, #48\n"
        "sub x1, x1, #1\n"
        "strb w4, [x1]\n"
        "mov %0, x3\n"
        "cbnz %0, 1b\n"
        "mov x0, #1\n"
        "mov x2, sp\n"
        "add x2, x2, #31\n"
        "sub x2, x2, x1\n"
        "mov x8, #64\n"
        "svc #0\n"
        "add sp, sp, #32\n"
        : "+r"(val) :: "x0", "x1", "x2", "x3", "x4", "x8", "memory"
    );
}

void print_str(char *s) {
    long len = strlen(s);
    s[len] = '\n';
    len++;
    __asm__ __volatile__ (
        "mov x0, #1\n"
        "mov x1, %0\n"
        "mov x2, %1\n"
        "mov x8, #64\n"
        "svc #0\n"
        : : "r"(s), "r"(len) : "x0", "x1", "x2", "x8", "memory"
    );
}

long read_num() {
    char buf[32] = {0};
    long val = 0;
    int sign = 1;
    long len = 0;
    char *ptr = buf;
    __asm__ __volatile__ (
        "mov x0, #0\n"
        "mov x1, %1\n"
        "mov x2, #32\n"
        "mov x8, #63\n"
        "svc #0\n"
        "mov %0, x0\n"
        : "=r"(len)
        : "r"(ptr)
        : "x0", "x1", "x2", "x8", "memory"
    );
    int i = 0;
    while (i < len && isspace((unsigned char)buf[i])) i++;
    if (i < len && buf[i] == '-') {
        sign = -1;
        i++;
    } else if (i < len && buf[i] == '+') {
        i++;
    }
    while (i < len && isdigit((unsigned char)buf[i])) {
        val = val * 10 + (buf[i] - '0');
        i++;
    }
    return val * sign;
}

#endif