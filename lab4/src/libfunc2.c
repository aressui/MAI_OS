#include <math.h>
#include <stdlib.h>
#include <string.h>

float pi(int k) {
    if (k <= 0) return 0.0f;
    
    float result = 1.0f;
    for (int i = 1; i <= k; i++) {
        float numerator = 4.0f * i * i;
        float denominator = (2.0f * i - 1.0f) * (2.0f * i + 1.0f);
        result *= numerator / denominator;
    }
    return 2.0f * result;
}

static void quick_sort_recursive(int* arr, int low, int high) {
    if (low < high) {
        int pivot = arr[(low + high) / 2];
        int i = low - 1;
        int j = high + 1;
        
        while (1) {
            do i++; while (arr[i] < pivot);
            do j--; while (arr[j] > pivot);
            if (i >= j) break;
            
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
        
        quick_sort_recursive(arr, low, j);
        quick_sort_recursive(arr, j + 1, high);
    }
}

int* sort(int* array, size_t n) {
    if (n == 0 || array == NULL) return NULL;
    
    int* result = malloc(n * sizeof(int));
    if (result == NULL) return NULL;
    
    memcpy(result, array, n * sizeof(int));
    quick_sort_recursive(result, 0, n - 1);
    
    return result;
}