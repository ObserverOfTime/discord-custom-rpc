#pragma once

#ifdef NDEBUG
#define LOG(msg, ...)
#define LOG_B(bytes)
#else
#define LOG(msg, ...) fprintf(stderr, msg, ##__VA_ARGS__)
#define LOG_B(bytes) \
    LOG("\\x%02x\\x%02x\\x%02x\\x%02x", /**/ \
        (bytes)[0], (bytes)[1], (bytes)[2], (bytes)[3]);
#endif  // NDEBUG

#define TIME ((int32_t) time(NULL))

#define PID ((int32_t) getpid())

#define TMP_PATH \
    ( \
      getenv("XDG_RUNTIME_DIR") ?: \
      getenv("TMPDIR") ?: \
      getenv("TMP") ?: \
      getenv("TEMP") ?: \
      "/tmp" \
    )

#define STR_EQ(s1, s2) (strncmp((s1), (s2), strlen((s2))) == 0)

#define JSON_GET_OBJ(obj, key) json_object_object_get((obj), #key)

#define JSON_GET(obj, key, type) json_object_get_##type(JSON_GET_OBJ(obj, key))

#define JSON_ADD_OBJ(obj, key, val) json_object_object_add((obj), #key, (val))

#define JSON_ADD(obj, key, val, type) \
    JSON_ADD_OBJ(obj, key, json_object_new_##type((val)))

#define JSON_ADD_S(obj, key, val) \
    if ((val) != NULL) JSON_ADD(obj, key, val, string)

#define JSON_TO_STRING(obj) \
    json_object_to_json_string_ext((obj), JSON_C_TO_STRING_PLAIN)

#define JSON_PARSE(str) json_tokener_parse((str))

#define JSON_FREE(obj) json_object_put((obj))

#define JSON_NEW() json_object_new_object()

#define INT_TO_BYTES(n, bytes) \
    { \
        (bytes)[0] = (n) >> 0 & 0xFF; \
        (bytes)[1] = (n) >> 8 & 0xFF; \
        (bytes)[2] = (n) >> 16 & 0xFF; \
        (bytes)[3] = (n) >> 24 & 0xFF; \
    }

#define BYTES_TO_INT(bytes) \
    ((bytes)[0] << 0 | (bytes)[1] << 8 | (bytes)[2] << 16 | (bytes)[3] << 24)
