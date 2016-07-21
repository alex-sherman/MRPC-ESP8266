#ifndef _JRPC_PATH_H_
#define _JRPC_PATH_H_
namespace MRPC {
    class Path {
    public:
        Path() { };
        Path(const char* path);
        bool is_wildcard(int index) { return index < identifier_count && identifiers[identifier_offsets[index]] == '*'; }
        bool is_broadcast = false;
        bool is_absolute;
        char path[256];
        bool match(Path &other);
        const char *operator [](int i) { return &identifiers[identifier_offsets[i]]; }
        Path concat(Path &suffix);
    private:
        char identifiers[256];
        int identifier_offsets[16];
        int identifier_count = 0;
    };
}

#endif