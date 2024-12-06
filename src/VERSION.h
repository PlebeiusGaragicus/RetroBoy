#define BUILD 112

// define version string that is based on the compile time
// #define VERSION "v" __DATE__ "-" __TIME__
#define VERSION __DATE__ " " __TIME__

// define a macro that returns a string concatenated with the version string
#define VERSION_STRING(str) str VERSION
