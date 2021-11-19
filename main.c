#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// make -k to run the file

#define IBVM_STACK_CAPACITY 1028

typedef enum {
    TRAP_OK = 0,
    TRAP_STACK_OVERFLOW,
    TRAP_STACK_UNDERFLOW,
    TRAP_ILLEGAL_INST,
    TRAP_DIVISION_BY_ZERO,
} Trap;

const char* trap_as_cstr(Trap trap){
    switch (trap) {
        case TRAP_OK:
            return "TRAP_OK";
        case TRAP_STACK_OVERFLOW:
            return "TRAP_STACK_OVERFLOW";
        case TRAP_STACK_UNDERFLOW:
            return "TRAP_STACK_UNDERFLOW";
        case TRAP_ILLEGAL_INST:
            return "TRAP_ILLEGAL_INST";
        case TRAP_DIVISION_BY_ZERO:
            return "TRAP_DIVISION_BY_ZERO";    
        default:
            assert(0 && "trap_as_cstr: Unreachable");         
    }
}

typedef int64_t Word;

typedef struct {
    Word stack[IBVM_STACK_CAPACITY];
    size_t stack_size;
    Word ip;
    int halt;
} Ibvm;

typedef enum {
    INST_PUSH,
    INST_PLUS,
    INST_MINUS,
    INST_MULT,
    INST_DIV,
    INST_JMP,
    INST_HALT,
} Inst_Type;

const char* inst_type_as_cstr(Inst_Type type) {
    switch (type){
    case INST_PUSH: return "INST_PUSH";
    case INST_PLUS: return "INST_PLUS";
    case INST_MINUS: return "INST_MINUS";
    case INST_MULT: return "INST_MULT";
    case INST_DIV: return "INST_DIV";
    case INST_JMP: return "INST_JMP";
    case INST_HALT: return "INST_HALT";    
    default: assert(0 && "inst_type_as_cstr: unreachable");
    }
}

typedef struct {
    Inst_Type type;
    Word operand;
} Inst; 

#define MAKE_INST_PUSH(value) {.type = INST_PUSH, .operand = (value)}
#define MAKE_INST_PLUS {.type = INST_PLUS}
#define MAKE_INST_MINUS {.type = INST_MINUS}
#define MAKE_INST_MULT {.type = INST_MULT}
#define MAKE_INST_DIV {.type = INST_DIV}
#define MAKE_INST_JMP(addr) {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_HALT {.type = INST_HALT}

// static inline Inst inst_push(Word operand){
//     return (Inst){  // delegated initializer
//         .type = INST_PUSH,
//         .operand = operand, 
//     };
// }

// static inline Inst inst_plus(void){
//     return (Inst){.type = INST_PLUS};
// }


Trap ibvm_execute_inst(Ibvm *ibvm, Inst inst){
    switch (inst.type){
        case INST_PUSH:
            if(ibvm->stack_size >= IBVM_STACK_CAPACITY)     return TRAP_STACK_OVERFLOW;
            // assert(ibvm->stack_size < IBVM_STACK_CAPACITY);
            ibvm->stack[ibvm->stack_size++] = inst.operand;
            break;

        case INST_PLUS:
            if (ibvm->stack_size < 2)   return TRAP_STACK_UNDERFLOW;
            ibvm->stack[ibvm->stack_size - 2] += ibvm->stack[ibvm->stack_size - 1]; 
            ibvm->stack_size -= 1;
            break;
        case INST_MINUS:
            if (ibvm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            ibvm->stack[ibvm->stack_size - 2] -= ibvm->stack[ibvm->stack_size - 1]; // count 2, index 2-2= 0 index 2-1=1 index(0-1)
            ibvm->stack_size -= 1; // index 2 - 1 = stack_size == 1
            break;
        case INST_MULT:
            if (ibvm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            ibvm->stack[ibvm->stack_size - 2] *= ibvm->stack[ibvm->stack_size -1];
            ibvm->stack_size -= 1;
            break;            
        case INST_DIV:
            if (ibvm->stack_size < 2) return TRAP_STACK_UNDERFLOW;
            if (ibvm->stack[ibvm->stack_size - 1] == 0) return TRAP_DIVISION_BY_ZERO;
            ibvm->stack[ibvm->stack_size - 2] /= ibvm->stack[ibvm->stack_size -1];
            ibvm->stack_size -= 1;
            break; 
        default:
            return TRAP_ILLEGAL_INST;

    }
    
    return TRAP_OK;
}

void ibvm_dump(FILE* stream, const Ibvm *ibvm){
    fprintf(stream, "Stack:\n");
    
    if (ibvm->stack_size > 0){
        for (size_t i = 0; i < ibvm->stack_size; ++i){
            fprintf(stream, " %lld\n", ibvm->stack[i]);
        }
    } else {
        fprintf(stream, "  [empty]\n");
    }

}

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))

Ibvm ibvm = {0};

Inst program[] = {
    MAKE_INST_PUSH(69),
    MAKE_INST_PUSH(420),
    MAKE_INST_PLUS,
    MAKE_INST_PUSH(42),
    MAKE_INST_MINUS, 
    MAKE_INST_PUSH(2),
    MAKE_INST_MULT,
    MAKE_INST_PUSH(4),
    MAKE_INST_DIV
};

int main(){

    ibvm_dump(stdout, &ibvm);

    while(!ibvm.halt) {
        printf("%s\n", inst_type_as_cstr(program[ibvm.ip].type));
        Trap trap = ibvm_execute_inst(&ibvm, program[ibvm.ip]);
        ibvm_dump(stdout, &ibvm);
        if (trap != TRAP_OK) {
            fprintf(stderr, "Trap activated: %s\n", trap_as_cstr(trap));
            exit(0);
        }
    }

    // for (size_t i = 0; i < ARRAY_SIZE(program); ++i) {
    //     printf("%s\n", inst_type_as_cstr(program[i].type));
    //     Trap trap = ibvm_execute_inst(&ibvm, program[i]);
    //     ibvm_dump(stdout, &ibvm);
    //     if (trap != TRAP_OK) {
    //         fprintf(stderr, "Trap activated: %s\n", trap_as_cstr(trap));
    //         // ibvm_dump(stderr, &ibvm);
    //         exit(0);
    //     }    
    // }
    
    // ibvm_dump(&ibvm);
    // ibvm_execute_inst(&ibvm, inst_push(69));
    // ibvm_dump(&ibvm);
    // ibvm_execute_inst(&ibvm, inst_push(420));
    // ibvm_dump(&ibvm);
    // ibvm_execute_inst(&ibvm, inst_plus());
    // ibvm_dump(&ibvm);

    return 0;
}
