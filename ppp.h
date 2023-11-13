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

#ifndef PPP_SEPARATOR
#define PPP_SEPARATOR "~/"
#endif

#ifndef PPP_PREFIX
#define PPP_PREFIX "~^"
#endif

#ifndef PPP_POSTFIX
#define PPP_POSTFIX "~$"
#endif

#define PPP_JSON_FORMAT ""PPP_PREFIX"%s"PPP_POSTFIX":"PPP_PREFIX""SV_Fmt""PPP_POSTFIX

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
void  ppp_elda_init(size_t capacity, PpP_Elda *elda);
void  ppp_elda_clear(PpP_Elda *elda);
void  ppp_elda_insert(PpP_Elem elem, PpP_Elda *elda);
void  ppp_elda_print_raw(PpP_Elda elda);
void  ppp_elda_print_json(PpP_Elda elda);

// Functions for string manipulation
char* ppp_string_append(char *dst, char *str); 
void  ppp_string_append_json_sv(char **entry_value, size_t col_ind, size_t ppp_argc, char *ppp_argv[], String_View sv);
void  ppp_string_rev(char * s1);
char* ppp_string_join(size_t wc,  char *text[]);

/**
 * Transpile pdf into raw text
 * {path} ... string path to pdf file [relative to the script]
 * {text} ... pointer to the string which will store pdf as raw text
*/
void ppp_pdf_to_text(char *path, char **text);

// transformations
void ppp_elem_transform_json(PpP_Elem *elem);

typedef void parser_func(size_t *printc, char *argv[], PpP_Elda *elda, String_View line);
void ppp_text_to_json(
        size_t printc,
        size_t argc,
        char *argv[],
        char *table_vals_end,
        PpP_Elda *elda,
        char *text,
        parser_func
);

#endif // !PpP

#ifdef PPP_IMPLEMENTATION

void ppp_elda_init(size_t capacity, PpP_Elda *elda) {
    elda->capacity = capacity;
    elda->es = malloc(sizeof(PpP_Elem) * capacity);
}

void ppp_elda_clear(PpP_Elda *elda)
{
    free(elda->es);
    elda->size = 0;
}

void ppp_elda_insert(PpP_Elem elem, PpP_Elda *elda) {
    assert(elda->size < elda->capacity);
    elda->es[elda->size] = elem;
    elda->size += 1;
}

void ppp_elda_print_raw(PpP_Elda elda)
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
        PpP_Elem elem = elda.es[i];
        ppp_elem_transform_json(&elem);
        if ((i+1) == elda.size) {
            printf("%4s\"%zu\": { %s }\n","", i, elem.value);
        }
        else {
            printf("%4s\"%zu\": { %s },\n", "", i, elem.value);
        }
    }
    printf("}\n");
}   

char* ppp_string_append(char *dst, char *str) 
{
    size_t dst_len = strlen(dst);
    size_t str_len = strlen(str);
    size_t size = dst_len + str_len + 1;
    char *fstr = calloc(size, sizeof(char));

    for (size_t i = 0; i < dst_len; ++i) {
        fstr[i] = dst[i];
    }

    for (size_t i = 0; i < str_len; ++i) {
        fstr[i + dst_len] = str[i];
    }
    fstr[size - 1] = '\0';
    return fstr;
}
/**
 * Appends the json key:value to the entry_value string
 * {entry_value} ... string pointer to where the json entry will be appended
 * {col_ind}     ... index of the column 
 * {ppp_argc}    ... total count of columns
 * {ppp_argv}    ... array of strings containing colum nameis
 */
void ppp_string_append_json_sv(char **entry_value, size_t col_ind, size_t ppp_argc, char *ppp_argv[], String_View sv)
{
    char buf[512];
    if ((ppp_argc - 1) == col_ind) {
        snprintf(buf, sizeof(buf), PPP_JSON_FORMAT"", ppp_argv[col_ind], SV_Arg(sv));
    } else {
        snprintf(buf, sizeof(buf), PPP_JSON_FORMAT""PPP_SEPARATOR, ppp_argv[col_ind], SV_Arg(sv));
    }
    (*entry_value) = ppp_string_append(*entry_value, buf);
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
    char buf[512]; 
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

void ppp_pdf_to_text(char *path, char **text)
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
    (*text) = ppp_string_append(*text, (char *)buffer);
}

void ppp_elem_transform_json(PpP_Elem *elem)
{
    assert(strlen(PPP_SEPARATOR) == 2);
    assert(strlen(PPP_PREFIX) == 2);
    assert(strlen(PPP_POSTFIX) == 2);
    
    size_t ss = strlen(elem->value);
    for (size_t i = 0; i < ss; ++i) {
        char c = elem->value[i];
        if (c == PPP_SEPARATOR[0]) {
            if (elem->value[i+1] == PPP_SEPARATOR[1]) {
                elem->value[i] = ',';
                elem->value[i+1] = ' ';
            }
        } 
        if (c == PPP_PREFIX[0]) {
            if (elem->value[i+1] == PPP_PREFIX[1]) {
                elem->value[i] = '\02';
                elem->value[i+1] = '"';
            }
        }
        if (c == PPP_POSTFIX[0]) {
            if (elem->value[i+1] == PPP_POSTFIX[1]) {
                elem->value[i] = '"';
                elem->value[i+1] = '\03';
            }
        }
    }
}

void ppp_text_to_json(
        size_t printc,
        size_t argc,
        char *argv[],
        char *table_vals_end,
        PpP_Elda *elda,
        char *text,
        void parser_func(size_t *printc, char *argv[], PpP_Elda *elda, String_View line) 
)
{
    char *table_args = ppp_string_join(argc, argv);

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
