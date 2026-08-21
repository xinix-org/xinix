
#include <stdint.h>
#include <auxv.h>

uint32_t x86_feature_array[38];

void init_cpuid_array() {
    uint32_t* ptr = getauxval(AT_XINIX_CPU_FEATURES_ARRAY).a_ptr;
    size_t len = getauxval(AT_XINIX_CPU_FEATURES_LEN).a_val;

    size_t tlen = len <= 38 ? len : 38;

    for(size_t i = 0; i < tlen; i ++)
        x86_feature_array[i] = ptr[i];

}