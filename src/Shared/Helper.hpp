#pragma once

#define HELPER_BUFFER_SIZE 1024
// NOTE: Shared buffer to be used by the server to generate stuff that need a string buffer
//       this is safe because server is 1 threaded but async.
static char strbuffer[HELPER_BUFFER_SIZE] = {};
static const char *used_char = "abcdefghijklmnopqrstuvwxyz1234567890";
static const int used_char_len = strlen(used_char);

#define csprintf(format, ...) snprintf(strbuffer, HELPER_BUFFER_SIZE, fomat, __VA_ARGS__)

// NOTE: Assume the buffer len is the len.
bool _generate_random_id(size_t len, char *buffer) {
    if (len <= 0 || !buffer) return false;
    for (size_t i = 0; i < len; i++) {
        buffer[i] = used_char[rand() % used_char_len];
    }
    return true;
}

char *generate_random_id(size_t len) {
    srand(time(NULL));
    if (len <= 0 && len >= HELPER_BUFFER_SIZE) return nullptr;
    if (_generate_random_id(len, strbuffer)) return strbuffer;
    return nullptr;
}
