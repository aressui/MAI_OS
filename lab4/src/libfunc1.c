#include <math.h>
#include <stdlib.h>
#include <string.h>

float pi(int k) {
    if (k <= 0) return 0.0f;
    
    float result = 0.0f;
    for (int i = 0; i < k; i++) {
        float term = 1.0f / (2 * i + 1);
        result += (i % 2 == 0) ? term : -term;
    }
    return 4 * result;
}

int* sort(int* array, size_t n) {
    if (n == 0 || array == NULL) return NULL;
    
    int* result = malloc(n * sizeof(int));
    if (result == NULL) return NULL;
    
    memcpy(result, array, n * sizeof(int));
    
    for (size_t i = 0; i < n - 1; i++) {
        for (size_t j = 0; j < n - i - 1; j++) {
            if (result[j] > result[j + 1]) {
                int temp = result[j];
                result[j] = result[j + 1];
                result[j + 1] = temp;
            }
        }
    }
    
    return result;
}