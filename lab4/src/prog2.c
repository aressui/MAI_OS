#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <dlfcn.h>

#define BUFFER_SIZE 4096
#define OUTPUT_SIZE 1024

typedef float (*pi_func_t)(int);
typedef int* (*sort_func_t)(int*, size_t);

static void* lib_handle = NULL;
static pi_func_t pi_func = NULL;
static sort_func_t sort_func = NULL;
static int current_lib = 1;

static void write_string(const char* str) {
    write(STDOUT_FILENO, str, strlen(str));
}

static void write_error(const char* str) {
    write(STDERR_FILENO, str, strlen(str));
}

static int parse_int(const char* str, int* value) {
    if (!str || !*str) return 0;
    
    *value = 0;
    int sign = 1;
    int i = 0;
    
    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }
    
    while (str[i] >= '0' && str[i] <= '9') {
        *value = *value * 10 + (str[i] - '0');
        i++;
    }
    
    *value *= sign;
    return 1;
}

static int load_library(int lib_num) {
    char buffer[OUTPUT_SIZE];
    
    if (lib_handle) {
        dlclose(lib_handle);
    }
    
    char lib_name[32];
    int len = snprintf(lib_name, sizeof(lib_name), "./libfunc%d.so", lib_num);
    
    lib_handle = dlopen(lib_name, RTLD_LAZY);
    if (!lib_handle) {
        len = snprintf(buffer, OUTPUT_SIZE, "error: failed to load library %s: %s\n", 
                      lib_name, dlerror());
        write_error(buffer);
        return 0;
    }
    
    pi_func = (pi_func_t)dlsym(lib_handle, "pi");
    sort_func = (sort_func_t)dlsym(lib_handle, "sort");
    
    if (!pi_func || !sort_func) {
        write_error("error: failed to find required functions in library\n");
        dlclose(lib_handle);
        lib_handle = NULL;
        return 0;
    }
    
    current_lib = lib_num;
    len = snprintf(buffer, OUTPUT_SIZE, "switched to library: %s\n", lib_name);
    write_string(buffer);
    return 1;
}

static void command_0() {
    int new_lib = (current_lib == 1) ? 2 : 1;
    load_library(new_lib);
}

static void command_1(char* arg) {
    char buffer[OUTPUT_SIZE];
    int k;
    
    if (!pi_func) {
        write_error("error: pi function not loaded\n");
        return;
    }
    
    if (!parse_int(arg, &k)) {
        write_error("error: invalid argument for command 1\n");
        return;
    }
    
    float result = pi_func(k);
    int len = snprintf(buffer, OUTPUT_SIZE, "pi(%d) = %.6f (lib %d)\n", 
                      k, result, current_lib);
    write(STDOUT_FILENO, buffer, len);
}

static void command_2(int n, char** args) {
    char buffer[OUTPUT_SIZE];
    
    if (!sort_func) {
        write_error("error: sort function not loaded\n");
        return;
    }
    
    if (n <= 0) {
        write_error("error: invalid array size\n");
        return;
    }
    
    int* array = malloc(n * sizeof(int));
    if (!array) {
        write_error("error: memory allocation failed\n");
        return;
    }
    
    for (int i = 0; i < n; i++) {
        if (!parse_int(args[i], &array[i])) {
            write_error("error: invalid array element\n");
            free(array);
            return;
        }
    }
    
    int* sorted = sort_func(array, n);
    if (!sorted) {
        write_error("error: sorting failed\n");
        free(array);
        return;
    }
    
    int len = snprintf(buffer, OUTPUT_SIZE, "sorted: ");
    write(STDOUT_FILENO, buffer, len);
    
    for (int i = 0; i < n; i++) {
        len = snprintf(buffer, OUTPUT_SIZE, "%d ", sorted[i]);
        write(STDOUT_FILENO, buffer, len);
    }
    
    len = snprintf(buffer, OUTPUT_SIZE, "(lib %d)\n", current_lib);
    write(STDOUT_FILENO, buffer, len);
    
    free(sorted);
    free(array);
}

int main() {
    char buffer[BUFFER_SIZE];
    const char* prompt = "> ";
    
    if (!load_library(1)) {
        write_error("error: failed to load initial library\n");
        return 1;
    }
    
    write_string("Dynamic program (dlopen/dlsym)\n");
    write_string("Commands: 0 - switch library\n");
    write_string("          1 k - calculate pi(k)\n");
    write_string("          2 n a1 a2 ... an - sort array\n");
    write_string("          exit - exit program\n");
    write_string(prompt);
    
    while (1) {
        int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1);
        if (bytes_read <= 0) break;
        
        buffer[bytes_read] = '\0';
        
        // Remove newline
        if (buffer[bytes_read - 1] == '\n') {
            buffer[bytes_read - 1] = '\0';
            bytes_read--;
        }
        
        if (strcmp(buffer, "exit") == 0) {
            break;
        }
        
        // Skip empty lines
        if (bytes_read == 0) {
            write_string(prompt);
            continue;
        }
        
        // Parse command
        char* token = strtok(buffer, " ");
        if (!token) {
            write_string(prompt);
            continue;
        }
        
        if (strcmp(token, "0") == 0) {
            command_0();
        }
        else if (strcmp(token, "1") == 0) {
            char* arg = strtok(NULL, " ");
            command_1(arg);
        } 
        else if (strcmp(token, "2") == 0) {
            char* size_str = strtok(NULL, " ");
            int n;
            
            if (!size_str || !parse_int(size_str, &n)) {
                write_error("error: invalid array size\n");
                write_string(prompt);
                continue;
            }
            
            char** args = malloc(n * sizeof(char*));
            if (!args) {
                write_error("error: memory allocation failed\n");
                write_string(prompt);
                continue;
            }
            
            int valid = 1;
            for (int i = 0; i < n; i++) {
                args[i] = strtok(NULL, " ");
                if (!args[i]) {
                    write_error("error: not enough array elements\n");
                    valid = 0;
                    break;
                }
            }
            
            if (valid) {
                command_2(n, args);
            }
            
            free(args);
        }
        else {
            write_error("error: unknown command\n");
        }
        
        write_string(prompt);
    }
    
    if (lib_handle) {
        dlclose(lib_handle);
    }
    
    write_string("\n");
    return 0;
}