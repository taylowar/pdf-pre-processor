/**
 *  Pdf preProcessor - PpP
 *  As of right now this library supports only the JSON format
 */
#ifndef PpP
#define PpP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>

#ifndef SV_IMPLEMENTATION
#define SV_IMPLEMENTATION
#include "./sv.h"
#endif

#define ARRAY_LEN(xs) sizeof(xs) / sizeof(xs[0])

#define PDF_READ_BUFFER_CAPACITY 128*64 // this is 128 times 64 chars size 

typedef struct  {
    char value[256];
} PpP_Elem;

typedef struct {
    size_t size;
    size_t capacity;
    PpP_Elem *es;
} PpP_Elda;

// elda functions
void  ppp_elda_init(size_t capacity, PpP_Elda *da);
void  ppp_elda_clear(PpP_Elda *elda);
void  ppp_elda_insert(PpP_Elem v, PpP_Elda *da);
void  ppp_elda_print(PpP_Elda elda);
void  ppp_elda_print_json(PpP_Elda elda);

// Function for string manipulation
char* ppp_string_append(char *s1, char *s2); 
void  ppp_string_rev(char * s1);
char* ppp_string_join(size_t wc,  char *text[]);

char* ppp_pdf_to_text(char *path);

typedef void parser_func(size_t *printc,  char *argv[], PpP_Elda *elda, String_View line);
void ppp_text_to_json(
        size_t printc,
        size_t argc,
        char *argv[],
        const char *table_vals_end,
        PpP_Elda *elda,
        const char *text,
        parser_func
);

#endif // !PpP

#ifdef PPP_IMPLEMENTATION

void ppp_elda_init(size_t capacity, PpP_Elda *da) {
    da->capacity = capacity;
    da->es = malloc(sizeof(PpP_Elem) * capacity);
}

void ppp_elda_clear(PpP_Elda *elda)
{
    free(elda->es);
    elda->size = 0;
}

void ppp_elda_insert(PpP_Elem v, PpP_Elda *da) {
    da->es[da->size] = v;
    da->size += 1;
}

void ppp_elda_print(PpP_Elda elda)
{
    printf("------------------------------------\n");
    for (size_t i = 0; i < elda.size; ++i) {
        printf("[ %s ]\n", elda.es[i].value);
    }
}

void ppp_elda_print_json(PpP_Elda elda)
{
    printf("------------------------------------\n");
    printf("json: {\n");
    for (size_t i = 0; i < elda.size; ++i) {
        if ((i+1) == elda.size) {
            printf("%4s\"%zu\": { %s }\n","", i, elda.es[i].value);
        }
        else {
            printf("%4s\"%zu\": { %s },\n", "", i, elda.es[i].value);
        }
    }
    printf("}\n");
}   

char* ppp_string_append(char *s1, char *s2) 
{
    size_t s1_len = strlen(s1);
    size_t s2_len = strlen(s2);
    size_t size = s1_len + s2_len + 1;
    char *s = calloc(size, sizeof(char));

    for (size_t i = 0; i < s1_len; ++i) {
        s[i] = s1[i];
    }

    for (size_t i = 0; i < s2_len; ++i) {
        s[i + s1_len] = s2[i];
    }
    s[size - 1] = '\0';
    return s;
}

void ppp_string_rev(char *text)
{
    size_t text_len = strlen(text);
    size_t text_middle = text_len / 2;
    char swap_temp;
    
    for (size_t i = 0; i < text_middle; ++i) {
        swap_temp = text[i];
        text[i] = text[text_len - 1 - i];
        text[text_len - 1 - i] = swap_temp;
    }
}

char* ppp_string_join(size_t wc,  char *text[]) 
{
    char *dst = "";
    char buf[64]; 
    for (size_t i = 0; i < wc; ++i) {
        if ((i+1) == wc) {
            snprintf(buf, sizeof(buf), "%s", text[i]);
        } else {
            snprintf(buf, sizeof(buf), "%s ", text[i]);
        }
        dst = ppp_string_append(dst, buf);
    }
    return dst;
}

char* ppp_pdf_to_text(char *path)
{
    char *cmd = malloc (20 + strlen(path)); 
    sprintf(cmd, "pdftotext %s -raw -", path); 

    char *buffer[PDF_READ_BUFFER_CAPACITY];
    int out_pipe[2];
    int saved_stdout;

    /* save stdout for display later */
    saved_stdout = dup(STDOUT_FILENO);  

    /* make a pipe */
    if(pipe(out_pipe) != 0 ) {
        exit(1);
    }

    /* redirect stdout to the pipe */
    dup2(out_pipe[1], STDOUT_FILENO);
    close(out_pipe[1]);

    /* anything sent to printf should now go down the pipe */
    if (system(cmd) !=  0) {
        free(cmd);
        exit(1);
    }
    free(cmd);
    fflush(stdout);

    /* read from pipe into buffer */
    read(out_pipe[0], buffer, PDF_READ_BUFFER_CAPACITY); 

    /* reconnect stdout for testing */
    dup2(saved_stdout, STDOUT_FILENO); 

    printf("------------------------------------\n");
    printf("[PpP]: pdf converted to txt\n");
    return buffer;
}

void ppp_text_to_json(
        size_t printc,
        size_t argc,
        char *argv[],
        const char *table_vals_end,
        PpP_Elda *elda,
        const char *text,
        void parser_func(size_t *printc,  char *argv[], PpP_Elda *elda, String_View line) 
)
{
    const char *table_args = ppp_string_join(argc, argv);

    String_View text_sv = sv_from_cstr(text); 
    String_View table_args_sv = sv_from_cstr(table_args); 
    String_View end_sv = sv_from_cstr(table_vals_end); 

    size_t is_w = 0;

    while (text_sv.count > 0) {
        String_View line = sv_trim_left(sv_chop_by_delim(&text_sv, '\n'));
        if (line.count > 0) {
            if (sv_eq(line, end_sv)) {
                is_w = 0;
            }

            if (is_w) {
                parser_func(&printc, argv, elda, line);
           }

            if (sv_eq(line, table_args_sv)) {
                is_w = 1;
            }
        }
    }
}

#endif // PpP_IMPLEMENTAION
