#include <auxv.h>
#include <stdint.h>
#include <string.h>

uint32_t x86_feature_array[38];

void init_cpu_feature_array(void) {
    size_t entc = getauxval(AT_XINIX_CPU_FEATURES_LEN).a_val;
    uint32_t *cpu_feats = getauxval(AT_XINIX_CPU_FEATURES_ARRAY).a_ptr;

    size_t count = entc < 38 ? entc : 38;
    memcpy(x86_feature_array, cpu_feats, sizeof(uint32_t) * count);
}
