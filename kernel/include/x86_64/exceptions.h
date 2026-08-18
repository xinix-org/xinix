#pragma once
#include <stdint.h>

enum exception_id : int {
    EXCEPT_DE = 0,
    EXCEPT_DB = 1,
    EXCEPT_NMI = 2,
    EXCEPT_BP = 3,
    EXCEPT_OF = 4,
    EXCEPT_BR = 5,
    EXCEPT_UD = 6,
    EXCEPT_NM = 7,
    EXCEPT_DF = 8,
    EXCEPT_MP = 9,
    EXCEPT_TS = 10,
    EXCEPT_NP = 11,
    EXCEPT_SS = 12,
    EXCEPT_GP = 13,
    EXCEPT_PF = 14,
    EXCEPT_MF = 16,
    EXCEPT_AC = 17,
    EXCEPT_MC = 18,
    EXCEPT_XM = 19,
    EXCEPT_VE = 20,
    EXCEPT_CP = 21,
    EXCEPT_HV = 28,
    EXCEPT_VC = 29,
    EXCEPT_SC = 30,
};

#define EXCEPTION_NAME_ID(name)                                                \
    case EXCEPT_##name:                                                        \
        return #name

const char *exception_name(enum exception_id id) {
    switch (id) {
        EXCEPTION_NAME_ID(DE);
        EXCEPTION_NAME_ID(DB);
        EXCEPTION_NAME_ID(NMI);
        EXCEPTION_NAME_ID(BP);
        EXCEPTION_NAME_ID(OF);
        EXCEPTION_NAME_ID(BR);
        EXCEPTION_NAME_ID(UD);
        EXCEPTION_NAME_ID(NM);
        EXCEPTION_NAME_ID(MP);
        EXCEPTION_NAME_ID(DF);
        EXCEPTION_NAME_ID(TS);
        EXCEPTION_NAME_ID(NP);
        EXCEPTION_NAME_ID(SS);
        EXCEPTION_NAME_ID(GP);
        EXCEPTION_NAME_ID(PF);
        EXCEPTION_NAME_ID(MF);
        EXCEPTION_NAME_ID(AC);
        EXCEPTION_NAME_ID(MC);
        EXCEPTION_NAME_ID(XM);
        EXCEPTION_NAME_ID(VE);
        EXCEPTION_NAME_ID(CP);
        EXCEPTION_NAME_ID(HV);
        EXCEPTION_NAME_ID(VC);
        EXCEPTION_NAME_ID(SC);
    default:
        return nullptr;
    }
}
