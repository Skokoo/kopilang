#ifndef MAIN_SYN_H
#define MAIN_SYN_H

#include "../error/err.h"

void do_int(char *filename, char *raw, char *cmd, Var *db, int *count, int line, int *err) {
    char v_name[64] = {0}, expr[256] = {0};
    if (sscanf(cmd, "INTEGER %63s = %255[^\n]", v_name, expr) != 2) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m invalid INTEGER format\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }

    char *c_name = trim(v_name);
    char *c_expr = trim(expr);

    AsmOp op_table[256] = {0};
    op_table['+'] = asm_add;
    op_table['-'] = asm_sub;
    op_table['*'] = asm_mul;
    op_table['/'] = asm_div;

    char found_op = 0;
    for (int i = 0; c_expr[i]; i++) {
        if (op_table[(unsigned char)c_expr[i]]) {
            found_op = c_expr[i];
            break;
        }
    }

    if (found_op) {
        char arg1[64] = {0}, arg2[64] = {0}, fmt[32];
        sprintf(fmt, "%%63s %c %%63s", found_op);
        sscanf(c_expr, fmt, arg1, arg2);

        int i1 = get_idx(db, *count, trim(arg1));
        int i2 = get_idx(db, *count, trim(arg2));

        if (i1 == -1) { 
            printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", filename, line, arg1);
            printf(" |\n | %s\n |\n", raw);
            *err = 1; 
        }
        if (i2 == -1) { 
            printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", filename, line, arg2);
            printf(" |\n | %s\n |\n", raw);
            *err = 1; 
        }
        if (found_op == '/' && i2 != -1 && db[i2].val == 0) { 
            printf("\033[1m%s:%d:1: \033[1;31mmath error:\033[0m\033[1m division by zero\033[0m\n", filename, line);
            printf(" |\n | %s\n |\n", raw);
            *err = 1; 
            return; 
        }
        if (*err) return;

        strcpy(db[*count].name, c_name);
        db[*count].val = op_table[(unsigned char)found_op](db[i1].val, db[i2].val);
        (*count)++;
    } else {
        for (int i = 0; c_expr[i]; i++) {
            if (!isdigit((unsigned char)c_expr[i]) && c_expr[i] != '-') {
                printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m value must be numeric\033[0m\n", filename, line);
                printf(" |\n | %s\n |\n", raw);
                *err = 1;
                return;
            }
        }
        strcpy(db[*count].name, c_name);
        db[*count].val = atol(c_expr);
        (*count)++;
    }
}

void do_say(char *filename, char *raw, char *cmd, Var *db, int count, int line, int *err) {
    char *q1 = strchr(cmd, '"');
    char *q2 = strrchr(cmd, '"');

    if (!q1 || !q2 || q1 == q2) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m matching quotes required\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }

    int raw_len = q2 - q1 - 1;
    char content[512] = {0};
    if (raw_len > 510) raw_len = 510;
    strncpy(content, q1 + 1, raw_len);

    static char processed[1024];
    memset(processed, 0, sizeof(processed));
    int p_idx = 0;

    for (int i = 0; content[i] != '\0'; i++) {
        if (content[i] == '\\' && content[i+1] == 'e') {
            processed[p_idx++] = 0x1B;
            i++;
        } else if (content[i] == '\\' && content[i+1] == 'n') {
            processed[p_idx++] = '\n';
            i++;
        } else if (content[i] == '!') {
            char var_target[64] = {0};
            int v_len = 0;
            int start_i = i + 1;
            
            while (content[start_i] != '\0' && !isspace((unsigned char)content[start_i]) && content[start_i] != '\\' && content[start_i] != '"' && v_len < 63) {
                var_target[v_len++] = content[start_i++];
            }
            var_target[v_len] = '\0';

            int idx = get_idx(db, count, var_target);
            if (idx != -1) {
                char num_buf[32];
                sprintf(num_buf, "%ld", db[idx].val);
                for (int b = 0; num_buf[b] != '\0'; b++) {
                    processed[p_idx++] = num_buf[b];
                }
                i += v_len;
            } else {
                processed[p_idx++] = '!';
            }
        } else {
            processed[p_idx++] = content[i];
        }
    }

    print_str(processed);
}

void do_if(char *filename, char *raw, char *cmd, Var *db, int count, int line, int *err, int *active_block) {
    char cond_part[256] = {0};
    if (sscanf(cmd, "IF %[^\n]", cond_part) != 1) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m invalid IF syntax\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }

    char *c_cond = trim(cond_part);
    int len = strlen(c_cond);
    if (len < 2 || c_cond[len - 1] != '<') {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m expected '<' block opener\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }
    c_cond[len - 1] = '\0';
    c_cond = trim(c_cond);

    char v_name[64] = {0}, op[8] = {0};
    long value = 0;

    if (strstr(c_cond, "==")) { strcpy(op, "=="); sscanf(c_cond, "%63s == %ld", v_name, &value); }
    else if (strstr(c_cond, "!=")) { strcpy(op, "!="); sscanf(c_cond, "%63s != %ld", v_name, &value); }
    else if (strstr(c_cond, ">=")) { strcpy(op, ">="); sscanf(c_cond, "%63s >= %ld", v_name, &value); }
    else if (strstr(c_cond, "<=")) { strcpy(op, "<="); sscanf(c_cond, "%63s <= %ld", v_name, &value); }
    else if (strchr(c_cond, '>')) { strcpy(op, ">"); sscanf(c_cond, "%63s > %ld", v_name, &value); }
    else if (strchr(c_cond, '<')) { strcpy(op, "<"); sscanf(c_cond, "%63s < %ld", v_name, &value); }
    else {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m invalid comparison operator\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }

    int idx = get_idx(db, count, trim(v_name));
    if (idx == -1) {
        printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", filename, line, v_name);
        *err = 1;
        return;
    }

    long var_val = db[idx].val;
    long passed = 0;

    if (strcmp(op, "==") == 0) { __asm__("cmp %1, %2\n cset %0, eq" : "=r"(passed) : "r"(var_val), "r"(value)); }
    else if (strcmp(op, "!=") == 0) { __asm__("cmp %1, %2\n cset %0, ne" : "=r"(passed) : "r"(var_val), "r"(value)); }
    else if (strcmp(op, ">=") == 0) { __asm__("cmp %1, %2\n cset %0, ge" : "=r"(passed) : "r"(var_val), "r"(value)); }
    else if (strcmp(op, "<=") == 0) { __asm__("cmp %1, %2\n cset %0, le" : "=r"(passed) : "r"(var_val), "r"(value)); }
    else if (strcmp(op, ">") == 0) { __asm__("cmp %1, %2\n cset %0, gt" : "=r"(passed) : "r"(var_val), "r"(value)); }
    else if (strcmp(op, "<") == 0) { __asm__("cmp %1, %2\n cset %0, lt" : "=r"(passed) : "r"(var_val), "r"(value)); }

    if (!passed) {
        *active_block = 0;
    } else {
        *active_block = 1;
    }
}

void do_input(char *filename, char *raw, char *cmd, Var *db, int count, int line, int *err) {
    char *q1 = strchr(cmd, '"');
    char *q2 = strrchr(cmd, '"');
    if (!q1 || !q2 || q1 == q2) {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m matching quotes required for INPUT\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }
    int len = q2 - q1 - 1;
    char target[256] = {0};
    if (len > 255) len = 255;
    strncpy(target, q1 + 1, len);
    char *trimmed_target = trim(target);
    if (trimmed_target[0] != '!') {
        printf("\033[1m%s:%d:1: \033[1;31merror:\033[0m\033[1m INPUT target must be a variable prefixed with '!'\033[0m\n", filename, line);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }
    char *var_name = trim(trimmed_target + 1);
    int idx = get_idx(db, count, var_name);
    if (idx == -1) {
        printf("\033[1m%s:%d:1: \033[1;31mruntime error:\033[0m\033[1m variable '%s' undefined\033[0m\n", filename, line, var_name);
        printf(" |\n | %s\n |\n", raw);
        *err = 1;
        return;
    }
    db[idx].val = read_num();
}

#endif