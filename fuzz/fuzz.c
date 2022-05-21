#include "timelib.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static const timelib_format_specifier default_format_map[] = {
        {'+', TIMELIB_FORMAT_ALLOW_EXTRA_CHARACTERS},
        {'#', TIMELIB_FORMAT_ANY_SEPARATOR},
        {'j', TIMELIB_FORMAT_DAY_TWO_DIGIT},
        {'d', TIMELIB_FORMAT_DAY_TWO_DIGIT_PADDED},
        {'z', TIMELIB_FORMAT_DAY_OF_YEAR},
        {'S', TIMELIB_FORMAT_DAY_SUFFIX},
        {'U', TIMELIB_FORMAT_EPOCH_SECONDS},
        {'\\', TIMELIB_FORMAT_ESCAPE},
        {'h', TIMELIB_FORMAT_HOUR_TWO_DIGIT_12_MAX},
        {'g', TIMELIB_FORMAT_HOUR_TWO_DIGIT_12_MAX_PADDED},
        {'H', TIMELIB_FORMAT_HOUR_TWO_DIGIT_24_MAX},
        {'G', TIMELIB_FORMAT_HOUR_TWO_DIGIT_24_MAX_PADDED},
        {'a', TIMELIB_FORMAT_MERIDIAN},
        {'A', TIMELIB_FORMAT_MERIDIAN},
        {'u', TIMELIB_FORMAT_MICROSECOND_SIX_DIGIT},
        {'v', TIMELIB_FORMAT_MILLISECOND_THREE_DIGIT},
        {'i', TIMELIB_FORMAT_MINUTE_TWO_DIGIT},
        {'n', TIMELIB_FORMAT_MONTH_TWO_DIGIT},
        {'m', TIMELIB_FORMAT_MONTH_TWO_DIGIT_PADDED},
        {'?', TIMELIB_FORMAT_RANDOM_CHAR},
        {'!', TIMELIB_FORMAT_RESET_ALL},
        {'|', TIMELIB_FORMAT_RESET_ALL_WHEN_NOT_SET},
        {'s', TIMELIB_FORMAT_SECOND_TWO_DIGIT},
        {';', TIMELIB_FORMAT_SEPARATOR},
        {':', TIMELIB_FORMAT_SEPARATOR},
        {'/', TIMELIB_FORMAT_SEPARATOR},
        {'.', TIMELIB_FORMAT_SEPARATOR},
        {',', TIMELIB_FORMAT_SEPARATOR},
        {'-', TIMELIB_FORMAT_SEPARATOR},
        {'(', TIMELIB_FORMAT_SEPARATOR},
        {')', TIMELIB_FORMAT_SEPARATOR},
        {'*', TIMELIB_FORMAT_SKIP_TO_SEPARATOR},
        {'D', TIMELIB_FORMAT_TEXTUAL_DAY_3_LETTER},
        {'l', TIMELIB_FORMAT_TEXTUAL_DAY_FULL},
        {'M', TIMELIB_FORMAT_TEXTUAL_MONTH_3_LETTER},
        {'F', TIMELIB_FORMAT_TEXTUAL_MONTH_FULL},
        {'e', TIMELIB_FORMAT_TIMEZONE_OFFSET},
        {'P', TIMELIB_FORMAT_TIMEZONE_OFFSET},
        {'p', TIMELIB_FORMAT_TIMEZONE_OFFSET},
        {'T', TIMELIB_FORMAT_TIMEZONE_OFFSET},
        {'O', TIMELIB_FORMAT_TIMEZONE_OFFSET},
        {' ', TIMELIB_FORMAT_WHITESPACE},
        {'y', TIMELIB_FORMAT_YEAR_TWO_DIGIT},
        {'Y', TIMELIB_FORMAT_YEAR_FOUR_DIGIT},
        {'\0', TIMELIB_FORMAT_END}
};

bool format_has_proper_syntax(const char* format, size_t flen) {
    for (size_t i = 0; i < flen; ++i) {
        const timelib_format_specifier* map = default_format_map;
        bool char_found = false;
        while (map && map->specifier != '\0') {
            if (map->specifier == format[i]) {
                char_found = true;
                break;
            }
            ++map;
        }
        if (!char_found) {
            return false;
        }
    }
    return true;
}

void free_timelib_time(timelib_time *t) {
    if (t) {
        if (t->tz_info) {
            timelib_tzinfo_dtor(t->tz_info);
        }
        timelib_time_dtor(t);
    }
}

void free_timelib_rel_time(timelib_rel_time *p) {
    if (p) timelib_rel_time_dtor(p);
}

void free_timelib_error(timelib_error_container *e) {
    if (e) timelib_error_container_dtor(e);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 50)
        return 0;

    // Split input into equal thirds
    size_t third = size / 3;
    const uint8_t *data1 = data;
    const uint8_t *data2 = data + third;
    const uint8_t *data3 = data + (third * 2);
    size_t size1 = third;
    size_t size2 = third;
    size_t size3 = size - (third * 2);


    /** Test One **/
    timelib_time *t;
    timelib_error_container *errors;

    t = timelib_strtotime((const char*) data1, size1, &errors, timelib_builtin_db(), timelib_parse_tzfile);

    // Cleanup
    free_timelib_time(t);
    free_timelib_error(errors);

    /** Test Two **/
    if (size2 < 2) {
        return 0;
    }

    // Divide data into two parts: format and time
    size_t half = size2 / 2;
    char format[half];
    char time[size2 - half];
    memcpy(format, data2, half);
    memcpy(time, data2 + half, size2 - half);

    // Format specifier must only contain specific characters
    if (!format_has_proper_syntax(format, half)) {
        return 0;
    }

    t = timelib_parse_from_format((const char*) format, (const char*) time, size2 - half, &errors, timelib_builtin_db(), timelib_parse_tzfile);

    // Cleanup
    free_timelib_time(t);
    free_timelib_error(errors);

    /** Test Three **/
    timelib_time *b = NULL, *e = NULL;
    timelib_rel_time *p = NULL;
    int r = 0;
    timelib_strtointerval((const char*) data3, size3, &b, &e, &p, &r, &errors);

    // Cleanup
    free_timelib_time(b);
    free_timelib_time(e);
    free_timelib_rel_time(p);
    free_timelib_error(errors);

    return 0;
}
