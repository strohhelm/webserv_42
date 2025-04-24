#ifndef MACROS_HPP
#define MACROS_HPP
//Header to set all default values in the program to avoid looking for them in case of necessary change.
extern bool debug;
//Buffersize to recieve in SimpleServer_Launch.cpp
#define BUFFER_SIZE 1024
#define MAX_IN_MEMORY_BODY_SIZE 1048576 //1 MB
#define MAX_SEND_BYTES 8192 //8kb
#define MAX_URI_LENGTH 8000 //recommended by rcp
#define MAX_METHOD_LENGTH sizeof("DELETE")

//std File Paths for down and uploads
#define DEFAULT_UPLOAD_PATH "./temp/"
#define DEFAULT_DOWNLOAD_PATH "./temp/"

// ServerConfiguration Macros
#define DEFAULT_CONFIG_PATH "./config/test.conf"
#define DEFAULT_ERROR_LOG "./logs/error.log"
#define DEFAULT_ACCESS_LOG "./logs/access.log"
#define ACCESS "access"
#define ERROR "error"
#define CLOSE "close"


#endif
