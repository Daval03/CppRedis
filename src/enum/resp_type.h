#ifndef RESP_TYPE_H
#define RESP_TYPE_H

// RESP2 Value types
enum class RESPType {
    SIMPLE_STRING,
    ERROR,
    INTEGER,
    BULK_STRING,
    ARRAY,
    NULL_VALUE
};

#endif // RESP_TYPE_H