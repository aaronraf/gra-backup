#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include "../includes/structs.hpp"

extern Result run_simulation(int cycles, bool direct_mapped, unsigned cachelines, unsigned cacheline_size, unsigned cache_latency, int memory_latency, size_t num_requests, Request requests[], const char* tf_filename);

const char* usage_msg = 
    "Usage: %s [options] <Dateiname>\n"
    "Options:\n"
    "-c, --cycles <Zahl>         Die Anzahl der Zyklen, die simuliert werden sollen.\n"
    "--directmapped              Simuliert einen direkt assoziativen Cache.\n"
    "--fourway                   Simuliert einen vierfach assoziativen Cache.\n"
    "--cacheline-size <Zahl>     Die Größe einer Cachezeile in Byte.\n"
    "--cachelines <Zahl>         Die Anzahl der Cachezeilen.\n"
    "--cache-latency <Zahl>      Die Latenzzeit eines Caches in Zyklen.\n"
    "--memory-latency <Zahl>     Die Latenzzeit des Hauptspeichers in Zyklen.\n"
    "--tf=<Dateiname>            Ausgabedatei für ein Tracefile mit allen Signalen.\n"
    "<Dateiname>                 Die .csv Eingabedatei, die die zuverarbeitenden Daten enthält.\n"
    "-h, --help                  Eine Beschreibung aller Optionen des Programms.\n";
        
const char* help_msg = "";

void print_usage(const char* progname) {
    fprintf(stderr, usage_msg, progname);
}

void print_help(const char* progname) {
    print_usage(progname);
    fprintf(stderr, "\n%s", help_msg);
}

int fetch_num() {
    char* endptr;
    int num_input = strtol(optarg, &endptr, 10);

    // if endptr is not null-byte, then the optarg value is invalid
    if (endptr[0] != '\0') {
        fprintf(stderr, "Invalid value for cycle!\n");
        exit(EXIT_FAILURE);
    }
    return num_input;
}

char* read_csv(const char* csv_path) {
    FILE* csv_file = fopen(csv_path, "r");
    if (csv_file == NULL) {
        fprintf(stderr, "Error, can't open .csv file!\n");
        return NULL;
    }

    struct stat file_info;

    // edge case read file informations
    if (fstat(fileno(csv_file), &file_info)) {
        fprintf(stderr, "Error retrieving .csv file stat\n");
        fclose(csv_file);
        return NULL;
    }

    // edge case invalid file
    if (!S_ISREG(file_info.st_mode) || file_info.st_size <= 0) {
        fprintf(stderr, "Error processing .csv file\n");
        fclose(csv_file);
        return NULL;
    }

    // allocate memory in heap
    char* content = (char*) malloc(file_info.st_size + 1);
    if (!content) {
        fprintf(stderr, "Error reading .csv file, cannot allocate enough memory\n");
        fclose(csv_file);
        return NULL;
    }

    // read file content and save it in "content"
    if (fread(content, 1, file_info.st_size, csv_file) != (size_t) file_info.st_size) {
        fprintf(stderr, "Error reading .csv file!\n");
        free(content);
        content = NULL;
        fclose(csv_file);
        return NULL;
    }

    // null terminate
    content[file_info.st_size] = '\0';
    return content;
}

int count_num_of_request(const char* content) {
    if (content == NULL) {
        return 0;
    }

    int count = 0;
    const char* line = content;

    while ((line = strchr(line, '\n')) != NULL) {
        count++;
        line++;
    }

    if (strlen(content) > 0 && content[strlen(content) - 1] != '\n') {
        count++;
    }

    return count;
}

void parse_data(const char* content, struct Request request[], int number_of_requests, int* lines_read) {
    char* content_copy = strdup(content);       // copy of the content to avoid modifying the original
    int counter = 0;
    if (content_copy == NULL) {
        fprintf(stderr, "Error, .csv file does not have any content.\n");
        exit(EXIT_FAILURE);
    }
    
    char* rest;
    char* line = strtok_r(content_copy, "\n", &rest);
    *lines_read = 0;

    for (int i = 0; i < number_of_requests && line != NULL; i++, counter++) {
        char we_temp[2];     // Allocate space for "W" or "R" and null-byte terminator
        unsigned int addr;
        int data;

        sscanf(line, "%1s,%x,%d", we_temp, &addr, &data);
        data = we_temp[0] == 'W' ? data : 0;

        request[i].we = we_temp[0] == 'W' ? 1 : 0;
        request[i].addr = addr;
        request[i].data = data;

        line = strtok_r(NULL, "\n", &rest);
        (*lines_read)++;
    }
    printf(".csv line counter: %d\n", counter);
    free(content_copy);
}

int main(int argc, char* const argv[]) {
    const char* progname = argv[0];
    
    if (argc == 1) {
        print_usage(progname);
        return EXIT_FAILURE;
    }

    int option_index = 0;
    int cycles = 0;
    bool direct_mapped = false;
    bool fourway = false;
    int cacheline_size = 0;
    int cachelines = 0;
    int cache_latency = 0;
    int memory_latency = 0;
    char* tf_filename = "";
    bool is_tf_passed = false;
    char* csv_path = "";
    bool is_csv_passed = false;
    char* csv_content;
    size_t num_requests = 0;
    Request* requests;
    int lines_read = 0;

    struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"directmapped", no_argument, 0, 0},
        {"fourway", no_argument, 0, 0},
        {"cacheline-size", required_argument, 0, 0},
        {"cachelines", required_argument, 0, 0},
        {"cache-latency", required_argument, 0, 0},
        {"memory-latency", required_argument, 0, 0},
        {"tf", required_argument, 0, 0},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    int opt;
    while ((opt = getopt_long(argc, argv, "c:t:h", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'c':
            cycles = fetch_num();
            break;
        case 'h':
            print_help(progname);
            exit(EXIT_SUCCESS);
        case 't':
            tf_filename = optarg;
            break;
        case 0:
            if (strcmp(long_options[option_index].name, "directmapped") == 0) {
                if (fourway) {
                    fprintf(stderr, "Error! Cache can't be direct mapped and 4-way associative at the same time.\n");
                    exit(EXIT_FAILURE);
                }
                direct_mapped = true;
            } else if (strcmp(long_options[option_index].name, "fourway") == 0) {
                if (direct_mapped) {
                    fprintf(stderr, "Error! Cache can't be direct mapped and 4-way associative at the same time.\n");
                    exit(EXIT_FAILURE);
                }
                fourway = true;
            } else if (strcmp(long_options[option_index].name, "cacheline-size") == 0) {
                int fetched_number = fetch_num();
                if (fetched_number <= 0) {
                    fprintf(stderr, "Error! Invalid cacheline size value.\n");
                    exit(EXIT_FAILURE);
                }
                cacheline_size = fetched_number;
            } else if (strcmp(long_options[option_index].name, "cachelines") == 0) {
                int fetched_number = fetch_num();
                if (fourway && (fetched_number <= 0 || fetched_number % 4 != 0)) {
                    fprintf(stderr, "Error! Invalid cachelines amount value for 4-way associative cache.\n");
                    exit(EXIT_FAILURE);
                } else if (direct_mapped && (fetched_number & (fetched_number - 1)) != 0) {
                    fprintf(stderr, "Error! Invalid cachelines amount value for direct mapped cache. It should be power of two.\n");
                    exit(EXIT_FAILURE);
                }
                cachelines = fetched_number;
            } else if (strcmp(long_options[option_index].name, "cache-latency") == 0) {
                int fetched_number = fetch_num();
                if (fetched_number <= 0) {
                    fprintf(stderr, "Error! Invalid cache latency value.\n");
                    exit(EXIT_FAILURE);
                }
                cache_latency = fetched_number;
            } else if (strcmp(long_options[option_index].name, "memory-latency") == 0) {
                int fetched_number = fetch_num();
                if (fetched_number <= 0) {
                    fprintf(stderr, "Error! Invalid memory latency value.\n");
                    exit(EXIT_FAILURE);
                }
                memory_latency = fetched_number;
            } else if (strcmp(long_options[option_index].name, "tf") == 0) {
                tf_filename = optarg;       // (nanti apus) optarg refers to the argument which was passed after --tf (--tf "tracefile", "tracefile" is optarg)
                is_tf_passed = true;
            }
            break;
        default:
            print_usage(progname);
            exit(EXIT_FAILURE);
        }
    }

    // Remaining arguments goes to csv_path
    if (optind < argc) {
        csv_path = argv[optind];
        is_csv_passed = true;
    }

    // Check if all options have been initialized
    if (cycles == 0 || direct_mapped == fourway || cacheline_size == 0 || cachelines == 0 || cache_latency == 0 || memory_latency == 0 || !is_csv_passed) {
        fprintf(stderr, "Error! Not all options have been correctly initialized!\n");
        fprintf(stderr, "Type <program name> -h or --help for options.\n");
        exit(EXIT_FAILURE);
    }

    if (csv_path) {
        csv_content = read_csv(csv_path);
        if (!csv_content) {
            fprintf(stderr, "Error reading .csv file.\n");
            return 1;
        }
    } else {
        fprintf(stderr, ".csv file path not provided.\n");
        return 1;
    }

    // printf("%s\n", csv_content);
    printf("Cycles: %d\n", cycles);
    printf("Direct mapped: %d\n", direct_mapped);
    printf("Fourway: %d\n", fourway);
    printf("Cacheline Size: %d\n", cacheline_size);
    printf("Cachelines: %d\n", cachelines);
    printf("Cache Latency: %d\n", cache_latency);
    printf("Memory Latency: %d\n", memory_latency);
    printf("Tracefile name: %s\n", tf_filename);
    printf("Path to .csv file: %s\n", csv_path);
    
    num_requests = count_num_of_request(csv_content);
    requests = (Request *)malloc(num_requests * sizeof(Request));
    printf("num requests: %d", num_requests);
    if (requests == NULL) {
        fprintf(stderr, "Error allocating memory for requests array.\n");
        free(csv_content);
        return 1;
    }
    parse_data(csv_content, requests, num_requests, &lines_read);

    Result result = run_simulation(cycles, direct_mapped, cachelines, cacheline_size, cache_latency, memory_latency, num_requests, requests, tf_filename);
    // Result result = run_simulation(cycles, cachelines, cacheline_size, cache_latency, memory_latency, num_requests, requests, tf_filename);
    printf("Cycles: %zu\n", result.cycles);
    printf("Misses: %zu\n", result.misses);
    printf("Hits: %zu\n", result.hits);
    printf("Primitive Gate Count: %zu\n", result.primitiveGateCount);
    free(csv_content);

    return 0;
}