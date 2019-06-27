#include <stdio.h>
#include <getopt.h>
#include "../WcFile/WcFile.h"
#include "../WheyVM.h"

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>

#define DEFAULT_CALL_STACK_SIZE 1000
#define DEFAULT_OPERAND_STACK_SIZE 100
#define DEFAULT_MEMORY_SIZE 10000
#define DEFAULT_LOAD_FACTOR 0.6
#define DEFAULT_COOL_DOWN 1000

int main(int argc, char **argv) {

    char opt;
    uint8_t state = 0;
    uint16_t callStackSize = DEFAULT_CALL_STACK_SIZE;
    uint16_t operandStackSize = DEFAULT_OPERAND_STACK_SIZE;
    uint32_t memorySize = DEFAULT_MEMORY_SIZE;
    double loadFactor = DEFAULT_LOAD_FACTOR;
    uint16_t coolDown = DEFAULT_COOL_DOWN;
    int64_t temp;
    double doubleTemp;

    while((opt = getopt(argc, argv, "c:s:o:m:l:d")) != -1)  
    {  
        switch (opt)
        {
            case 'c':
                temp = strtol(optarg, NULL, 10);
                if (temp < 0)
                {
                    fprintf(stderr, "cooldown %ld must be a postive integer.\n", temp);
                    return 1;
                }
                printf("cooldown set to: %ld.\n", temp);
                coolDown = temp;
                break;

            case 's':
                temp = strtol(optarg, NULL, 10);
                if (temp < 0)
                {
                    fprintf(stderr, "call stack size %ld must be a postive integer.\n", temp);
                    return 1;
                }
                printf("call stack size set to: %ld.\n", temp);
                callStackSize = temp;
                break;

            case 'o':
                temp = strtol(optarg, NULL, 10);
                if (temp < 0)
                {
                    fprintf(stderr, "operand stack size %ld must be a postive integer.\n", temp);
                    return 1;
                }
                printf("operand stack size set to: %ld.\n", temp);
                operandStackSize = temp;
                break;

            case 'm':
                temp = strtol(optarg, NULL, 10);
                if (temp < 0)
                {
                    fprintf(stderr, "gc-memory size %ld must be a postive integer.\n", temp);
                    return 1;
                }
                printf("gc-memory size set to: %ld.\n", temp);
                memorySize = temp;
                break;

            case 'l':
                doubleTemp = strtod(optarg, NULL);
                if (doubleTemp < 0 || doubleTemp > 1)
                {
                    fprintf(stderr, "load factor %f must be between 0 and 1.\n", doubleTemp);
                    return 1;
                }
                printf("load factor set to: %f.\n", doubleTemp);
                loadFactor = doubleTemp;
                break;

            case 'd':
                state = WHEYVM_STATE_DEBUG;
                printf("running in debug mode.\n");
                break;
        
            default:
                fprintf(stderr, "unexpected option %c.\n", opt);
                return 1;
        }
    }

    if (argc - optind != 1) {
        fprintf(stderr, "one filename must be given.\n");
        return 1;
    }

    printf("running program: \"%s\".\n", argv[optind]);

    struct WcFile *program = wcFileNew(argv[optind]);

    int returnStatus = wvmRun(program, callStackSize, operandStackSize, memorySize, loadFactor, coolDown, state);

    wcFileFree(program);

    return returnStatus;
}