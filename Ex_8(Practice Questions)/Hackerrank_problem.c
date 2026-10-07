long* riddle(int arr_count, long* arr, int* result_count) {
    long* result = malloc(arr_count * sizeof(long));

    *result_count = arr_count;

    for (int size = 1; size <= arr_count; size++) {
        long max = LONG_MIN;

        for (int i = 0; i <= arr_count - size; i++) {
            long min = arr[i];

            for (int j = i; j < i + size; j++) {
                if (arr[j] < min) {
                    min = arr[j];
                }
            }

            if (min > max) {
                max = min;
            }
        }

        result[size - 1] = max;
    }

    return result;
}
