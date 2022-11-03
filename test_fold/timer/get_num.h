#ifndef GET_NUM_H
#define GET_NUM_H
#define GN_NONNEG 01     /* Value must be >= 0 */
#define GN_GT_0 02       /* Value must be > 0 */
                         /* By default, integers are decimal */
#define GN_ANY_BASE 0100 /* Can use any base - like strtol(3) */
#define GN_BASE_8 0200   /* Value is expressed in octal */
#define GN_BASE_16 0400  /* Value is expressed in hexadecimal */
long getLong(const char *arg, int flags, const char *name);
int getInt(const char *arg, int flags, const char *name);
#endif
static void gnFail(const char *fname, const char *msg, const char *arg, const char *name)
{
    fprintf(stderr, "%s error", fname);
    if (name != NULL)
        fprintf(stderr, " (in %s)", name);
    fprintf(stderr, ": %s\n", msg);
    if (arg != NULL && *arg != '\0')
        fprintf(stderr, " offending text: %s\n", arg);
    exit(EXIT_FAILURE);
}

static long getNum(const char *fname, const char *arg, int flags, const char *name)
{
    long res;
    char *endptr;
    int base;
    if (arg == NULL || *arg == '\0')
        gnFail(fname, "null or empty string", arg, name);
    base = (flags & GN_ANY_BASE) ? 0 : (flags & GN_BASE_8) ? 8
                                   : (flags & GN_BASE_16)  ? 16
                                                           : 10;
    errno = 0;
    res = strtol(arg, &endptr, base);
    if (errno != 0)
        gnFail(fname, "strtol() failed", arg, name);
    if (*endptr != '\0')
        gnFail(fname, "nonnumeric characters", arg, name);
    if ((flags & GN_NONNEG) && res < 0)
        gnFail(fname, "negative value not allowed", arg, name);
    if ((flags & GN_GT_0) && res <= 0)
        gnFail(fname, "value must be > 0", arg, name);
    return res;
}

long getLong(const char *arg, int flags, const char *name)
{
    return getNum("getLong", arg, flags, name);
}

int getInt(const char *arg, int flags, const char *name)
{
    long res;
    res = getNum("getInt", arg, flags, name);
    return (int)res;
}
void usageErr(const char *format, ...)
{
    va_list argList;
    fflush(stdout); /* Flush any pending stdout */
    fprintf(stderr, "Usage: ");
    va_start(argList, format);
    vfprintf(stderr, format, argList);
    va_end(argList);
    fflush(stderr); /* In case stderr is not line-buffered */
    exit(EXIT_FAILURE);
}

void errExit(const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    outputError(TRUE, errno, TRUE, format, argList);
    va_end(argList);
    terminate(TRUE);
}