#ifndef TEST_H
#define TEST_H

// Morlock.h

#define ROGUE_GC_AUTO

// Handle Apple's wonky defines which used to ALWAYS be defined as 0 or 1 and
// are now only defined if the platform is active.
#if defined(__APPLE__)
  #if defined(TARGET_IPHONE_SIMULATOR)
    #if TARGET_IPHONE_SIMULATOR
      #define ROGUE_PLATFORM_IOS 1
    #endif
  #endif

  #if !defined(ROGUE_PLATFORM_IOS)
    #if defined(TARGET_OS_IPHONE)
      #if TARGET_OS_IPHONE
        #define ROGUE_PLATFORM_IOS 1
      #endif
    #endif
  #endif

  #if !defined(ROGUE_PLATFORM_IOS)
    #define ROGUE_PLATFORM_MACOS 1
    #define ROGUE_PLATFORM_UNIX_COMPATIBLE 1
  #endif
#endif

#if !defined(ROGUE_PLATFORM_IOS) && !defined(ROGUE_PLATFORM_MACOS) && !defined(ROGUE_PLATFORM_PLAYDATE)
  #if defined(_WIN32)
  #  define ROGUE_PLATFORM_WINDOWS 1
  #elif defined(__ANDROID__)
  #  define ROGUE_PLATFORM_ANDROID 1
  #elif defined(__linux__)
  #  define ROGUE_PLATFORM_LINUX 1
  #  define ROGUE_PLATFORM_UNIX_COMPATIBLE 1
  #elif defined(__CYGWIN__)
  #  define ROGUE_PLATFORM_LINUX  1
  #  define ROGUE_PLATFORM_CYGWIN 1
  #  define ROGUE_PLATFORM_UNIX_COMPATIBLE 1
  #elif defined(EMSCRIPTEN)
  #  define ROGUE_PLATFORM_WEB 1
  #else
  #  define ROGUE_PLATFORM_GENERIC 1
  #endif
#endif

#if defined(ROGUE_PLATFORM_WINDOWS)
  #define NOGDI
  #pragma warning(disable: 4297) /* unexpected throw warnings */
  #if !defined(UNICODE)
    #define UNICODE
  #endif
  #include <windows.h>
  #include <signal.h>
#else
  #include <stdint.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>

//------------------------------------------------------------------------------
// Logging
//------------------------------------------------------------------------------
#ifdef __ANDROID__
  #include <android/log.h>
  #define ROGUE_LOG(...)       __android_log_print( ANDROID_LOG_INFO,  "Rogue", __VA_ARGS__ )
  #define ROGUE_LOG_ERROR(...) __android_log_print( ANDROID_LOG_ERROR, "Rogue", __VA_ARGS__ )
#else
  #define ROGUE_LOG(...)       printf( __VA_ARGS__ )
  #define ROGUE_LOG_ERROR(...) printf( __VA_ARGS__ )
#endif

//------------------------------------------------------------------------------
// Primitive Types
//------------------------------------------------------------------------------
#if defined(ROGUE_PLATFORM_WINDOWS)
  typedef double           RogueReal64;
  typedef float            RogueReal32;
  typedef __int64          RogueInt64;
  typedef __int32          RogueInt32;
  typedef __int32          RogueCharacter;
  typedef unsigned __int16 RogueWord;
  typedef unsigned char    RogueByte;
  typedef int              RogueLogical;
  typedef unsigned __int64 RogueUInt64;
  typedef unsigned __int32 RogueUInt32;
#else
  typedef double           RogueReal64;
  typedef float            RogueReal32;
  typedef int64_t          RogueInt64;
  typedef int32_t          RogueInt32;
  typedef int32_t          RogueCharacter;
  typedef uint16_t         RogueWord;
  typedef uint8_t          RogueByte;
  typedef int              RogueLogical;
  typedef uint64_t         RogueUInt64;
  typedef uint32_t         RogueUInt32;
#endif
typedef RogueReal64 RogueReal;
#define ROGUE_TYPE_PRIMITIVE 1
#define ROGUE_TYPE_COMPOUND  2
#define ROGUE_TYPE_ENUM      4
#define ROGUE_TYPE_OBJECT    8
#define ROGUE_TYPE_ASPECT    16
#define ROGUE_TYPE_MUTATING  32

#if !defined(ROGUE_MM_GC_THRESHOLD)
  #define ROGUE_MM_GC_THRESHOLD (32 * 1024 * 1024)
#endif

//------------------------------------------------------------------------------
// Classes
//------------------------------------------------------------------------------
#if !defined(ROGUE_MALLOC)
  #define ROGUE_MALLOC malloc
#endif

#if !defined(ROGUE_FREE)
  #define ROGUE_FREE free
#endif

#if !defined(ROGUE_CREATE_OBJECT)
  #define ROGUE_CREATE_OBJECT( TypeName ) \
      ((TypeName*)Rogue_create_object(&Type##TypeName))
#endif

#if !defined(ROGUE_NEW_OBJECT)
  // Creates an untracked Rogue object
  #define ROGUE_NEW_OBJECT( TypeName ) \
      ((TypeName*)Rogue_new_object(&Type##TypeName))
#endif

#if !defined(ROGUE_SINGLETON)
  #define ROGUE_SINGLETON( TypeName ) \
      ((TypeName*)Rogue_singleton(&Type##TypeName,&TypeName##_singleton))
#endif

#if !defined(ROGUE_SET_SINGLETON)
  #define ROGUE_SET_SINGLETON( TypeName, new_singleton ) \
      Rogue_set_singleton(&Type##TypeName,&TypeName##_singleton,new_singleton)
#endif

#if !defined(ROGUE_RELEASE)
  // Allow 'obj' to be GC'd if there are no other references to it.
  #define ROGUE_RELEASE( obj )  Rogue_release( obj )
#endif

#if !defined(ROGUE_RETAIN)
  // Prevent 'obj' from being GC'd if there are no other references to it.
  #define ROGUE_RETAIN( obj )   Rogue_retain( obj )
#endif

typedef struct RogueRuntimeType RogueRuntimeType;
typedef void (*RogueFn_Object)(void*);
typedef void (*RogueFn_Object_Object)(void*,void*);

typedef struct RogueObject RogueObject;
typedef struct RogueString RogueString;

struct RogueRuntimeType
{
  const char*    name;
  RogueString*   name_object;
  RogueInt32     index;
  RogueInt32     id;
  RogueInt32     module_name_index;
  RogueInt32     class_data_index;
  RogueInt64     attributes;
  void**         vtable;
  void**         local_pointer_stack; // Objects + compounds with embedded refs
  RogueInt32     local_pointer_capacity;
  RogueInt32     local_pointer_count;
  RogueInt32     size;
  RogueInt32*    base_type_ids;
  RogueInt32     base_type_count;
  RogueObject*   type_info;
  RogueFn_Object fn_init_object;
  RogueFn_Object fn_init;
  RogueFn_Object fn_gc_trace;
  RogueFn_Object fn_on_cleanup;
  RogueFn_Object_Object fn_on_singleton_change;
};

typedef struct RogueCallFrame
{
  const char* procedure;
  const char* filename;
  RogueInt32 line;
} RogueCallFrame;

typedef struct RogueObjectList
{
  RogueObject** data;
  RogueInt32    count;
  RogueInt32    capacity;
} RogueObjectList;

//------------------------------------------------------------------------------
// Runtime
//------------------------------------------------------------------------------
void  Rogue_configure( int argc, char** argv );
int   Rogue_launch(void);
void  Rogue_check_gc(void);
void  Rogue_collect_garbage(void);
void  Rogue_request_gc(void);
int   Rogue_quit(void);
void* Rogue_release( void* obj );
void* Rogue_retain( void* obj );

void* Rogue_create_object( void* type );
void  Rogue_destroy_object( void* obj );
void* Rogue_new_object( void* type );
void* Rogue_singleton( void* type, void* singleton_ref );
void  Rogue_set_singleton( void* type, void* singleton_ref, void* new_singleton );

void  Rogue_print_exception(void);

void  Rogue_call_stack_push( const char* procedure, const char* filename, int line );
void  Rogue_call_stack_pop(void);
void  RogueRuntimeType_local_pointer_stack_add( RogueRuntimeType* type, void* local_pointer );
void  RogueObjectList_add( RogueObjectList* list, void* obj );

struct RogueObject;

extern struct RogueObject* Rogue_exception;  // if non-null then an exception is being thrown
extern RogueCallFrame*     Rogue_call_stack;
extern int                 Rogue_call_stack_count;
extern int                 Rogue_call_stack_capacity;
extern int                 Rogue_call_stack_line;

extern int RogueMM_bytes_allocated_since_gc;
extern int RogueMM_gc_request;  // 0:none, !0:requested
extern RogueObjectList RogueMM_objects;
extern RogueObjectList RogueMM_objects_requiring_cleanup;

void*        Rogue_as( void* obj, RogueInt32 recast_type_id );
RogueLogical Rogue_instance_of( void* obj, RogueInt32 ancestor_id );

//------------------------------------------------------------------------------
// Generated
//------------------------------------------------------------------------------
extern const RogueUInt32 Rogue_crc32_table[256];

typedef struct RogueRuntimeType RogueRuntimeType;
extern RogueRuntimeType TypeRogueLogical;

extern RogueRuntimeType TypeRogueByte;

extern RogueRuntimeType TypeRogueCharacter;

extern RogueRuntimeType TypeRogueInt32;

extern RogueRuntimeType TypeRogueInt64;

extern RogueRuntimeType TypeRogueReal32;

extern RogueRuntimeType TypeRogueReal64;

extern RogueRuntimeType TypeRogueRogueCNativeProperty;

typedef struct RogueStackTraceFrame RogueStackTraceFrame;
extern RogueRuntimeType TypeRogueStackTraceFrame;

typedef struct RogueFile RogueFile;
extern RogueRuntimeType TypeRogueFile;

typedef struct RogueOptionalFile RogueOptionalFile;
extern RogueRuntimeType TypeRogueOptionalFile;

typedef struct RogueOptionalInt32 RogueOptionalInt32;
extern RogueRuntimeType TypeRogueOptionalInt32;

typedef struct RogueOptionalString RogueOptionalString;
extern RogueRuntimeType TypeRogueOptionalString;

typedef struct RogueListRewriterxRogueStringx RogueListRewriterxRogueStringx;
extern RogueRuntimeType TypeRogueListRewriterxRogueStringx;

typedef struct RogueStringEncoding RogueStringEncoding;
extern RogueRuntimeType TypeRogueStringEncoding;

typedef struct RogueSpan RogueSpan;
extern RogueRuntimeType TypeRogueSpan;

typedef struct RogueOptionalSpan RogueOptionalSpan;
extern RogueRuntimeType TypeRogueOptionalSpan;

typedef struct GeometryXY GeometryXY;
extern RogueRuntimeType TypeGeometryXY;

typedef struct GeometryBox GeometryBox;
extern RogueRuntimeType TypeGeometryBox;

typedef struct RogueValue RogueValue;
extern RogueRuntimeType TypeRogueValue;

typedef struct RogueConsoleCursor RogueConsoleCursor;
extern RogueRuntimeType TypeRogueConsoleCursor;

typedef struct RogueFilePattern RogueFilePattern;
extern RogueRuntimeType TypeRogueFilePattern;

typedef struct RogueOptionalFilePattern RogueOptionalFilePattern;
extern RogueRuntimeType TypeRogueOptionalFilePattern;

typedef struct RogueFileListingOption RogueFileListingOption;
extern RogueRuntimeType TypeRogueFileListingOption;

typedef struct RogueTableEntriesIteratorxRogueString_RogueValuex RogueTableEntriesIteratorxRogueString_RogueValuex;
extern RogueRuntimeType TypeRogueTableEntriesIteratorxRogueString_RogueValuex;

typedef struct RogueTableKeysIteratorxRogueString_RogueValuex RogueTableKeysIteratorxRogueString_RogueValuex;
extern RogueRuntimeType TypeRogueTableKeysIteratorxRogueString_RogueValuex;

typedef struct RogueOptionalTableEntryxRogueString_RogueValuex RogueOptionalTableEntryxRogueString_RogueValuex;
extern RogueRuntimeType TypeRogueOptionalTableEntryxRogueString_RogueValuex;

typedef struct RogueConsoleEventType RogueConsoleEventType;
extern RogueRuntimeType TypeRogueConsoleEventType;

typedef struct RogueConsoleEvent RogueConsoleEvent;
extern RogueRuntimeType TypeRogueConsoleEvent;

typedef struct RogueWordPointer RogueWordPointer;
extern RogueRuntimeType TypeRogueWordPointer;

typedef struct RogueRangeUpToLessThanxRogueInt32x RogueRangeUpToLessThanxRogueInt32x;
extern RogueRuntimeType TypeRogueRangeUpToLessThanxRogueInt32x;

typedef struct RogueRangeUpToLessThanIteratorxRogueInt32x RogueRangeUpToLessThanIteratorxRogueInt32x;
extern RogueRuntimeType TypeRogueRangeUpToLessThanIteratorxRogueInt32x;

typedef struct RogueRangeUpToxRogueInt32x RogueRangeUpToxRogueInt32x;
extern RogueRuntimeType TypeRogueRangeUpToxRogueInt32x;

typedef struct RogueRangeUpToIteratorxRogueInt32x RogueRangeUpToIteratorxRogueInt32x;
extern RogueRuntimeType TypeRogueRangeUpToIteratorxRogueInt32x;

typedef struct RogueRangeDownToxRogueInt32x RogueRangeDownToxRogueInt32x;
extern RogueRuntimeType TypeRogueRangeDownToxRogueInt32x;

typedef struct RogueRangeDownToIteratorxRogueInt32x RogueRangeDownToIteratorxRogueInt32x;
extern RogueRuntimeType TypeRogueRangeDownToIteratorxRogueInt32x;

typedef struct RogueTableKeysIteratorxRogueString_RogueStringx RogueTableKeysIteratorxRogueString_RogueStringx;
extern RogueRuntimeType TypeRogueTableKeysIteratorxRogueString_RogueStringx;

typedef struct RogueTableValuesIteratorxRogueString_RogueStringx RogueTableValuesIteratorxRogueString_RogueStringx;
extern RogueRuntimeType TypeRogueTableValuesIteratorxRogueString_RogueStringx;

typedef struct RogueVersionNumber RogueVersionNumber;
extern RogueRuntimeType TypeRogueVersionNumber;

typedef struct RogueBestxRogueStringx RogueBestxRogueStringx;
extern RogueRuntimeType TypeRogueBestxRogueStringx;

typedef struct RogueZipEntry RogueZipEntry;
extern RogueRuntimeType TypeRogueZipEntry;

typedef struct RogueUnixConsoleMouseEventType RogueUnixConsoleMouseEventType;
extern RogueRuntimeType TypeRogueUnixConsoleMouseEventType;

typedef struct RogueByteList RogueByteList;
extern RogueRuntimeType TypeRogueByteList;

typedef struct RogueString RogueString;
extern RogueRuntimeType TypeRogueString;

typedef struct RogueOPARENFunctionOPARENCPARENCPAREN RogueOPARENFunctionOPARENCPARENCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENCPARENCPAREN;

typedef struct RogueOPARENFunctionOPARENCPARENCPARENList RogueOPARENFunctionOPARENCPARENCPARENList;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENCPARENCPARENList;

typedef struct RogueGlobal RogueGlobal;
extern RogueRuntimeType TypeRogueGlobal;

typedef struct RogueObject RogueObject;
extern RogueRuntimeType TypeRogueObject;

typedef struct RogueStackTraceFrameList RogueStackTraceFrameList;
extern RogueRuntimeType TypeRogueStackTraceFrameList;

typedef struct RogueStackTrace RogueStackTrace;
extern RogueRuntimeType TypeRogueStackTrace;

typedef struct RogueException RogueException;
extern RogueRuntimeType TypeRogueException;

typedef struct RogueRoutine RogueRoutine;
extern RogueRuntimeType TypeRogueRoutine;

typedef struct RogueStringList RogueStringList;
extern RogueRuntimeType TypeRogueStringList;

typedef struct RogueSystem RogueSystem;
extern RogueRuntimeType TypeRogueSystem;

typedef struct RogueError RogueError;
extern RogueRuntimeType TypeRogueError;

typedef struct RogueProcessResult RogueProcessResult;
extern RogueRuntimeType TypeRogueProcessResult;

typedef struct RogueProcess RogueProcess;
extern RogueRuntimeType TypeRogueProcess;

typedef struct RogueOPARENFunctionOPARENRogueStringCPARENCPAREN RogueOPARENFunctionOPARENRogueStringCPARENCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueStringCPARENCPAREN;

typedef struct RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN;

typedef struct RogueListReaderxRogueStringx RogueListReaderxRogueStringx;
extern RogueRuntimeType TypeRogueListReaderxRogueStringx;

typedef struct RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN;

typedef struct RogueCharacterList RogueCharacterList;
extern RogueRuntimeType TypeRogueCharacterList;

typedef struct RogueStringReader RogueStringReader;
extern RogueRuntimeType TypeRogueStringReader;

typedef struct RogueBufferedPrintWriter RogueBufferedPrintWriter;
extern RogueRuntimeType TypeRogueBufferedPrintWriter;

typedef struct RogueStringPool RogueStringPool;
extern RogueRuntimeType TypeRogueStringPool;

typedef struct RogueListReaderxRogueBytex RogueListReaderxRogueBytex;
extern RogueRuntimeType TypeRogueListReaderxRogueBytex;

typedef struct RogueConsoleMode RogueConsoleMode;
extern RogueRuntimeType TypeRogueConsoleMode;

typedef struct RogueConsole RogueConsole;
extern RogueRuntimeType TypeRogueConsole;

typedef struct RogueFileReader RogueFileReader;
extern RogueRuntimeType TypeRogueFileReader;

typedef struct RogueFileWriter RogueFileWriter;
extern RogueRuntimeType TypeRogueFileWriter;

typedef struct RogueUTF16String RogueUTF16String;
extern RogueRuntimeType TypeRogueUTF16String;

typedef struct RogueWindowsProcess RogueWindowsProcess;
extern RogueRuntimeType TypeRogueWindowsProcess;

typedef struct RoguePosixProcess RoguePosixProcess;
extern RogueRuntimeType TypeRoguePosixProcess;

typedef struct RogueStringEncodingList RogueStringEncodingList;
extern RogueRuntimeType TypeRogueStringEncodingList;

typedef struct RogueObjectPoolxRogueStringx RogueObjectPoolxRogueStringx;
extern RogueRuntimeType TypeRogueObjectPoolxRogueStringx;

typedef struct RogueValueList RogueValueList;
extern RogueRuntimeType TypeRogueValueList;

typedef struct RogueTableEntryxRogueString_RogueValuex RogueTableEntryxRogueString_RogueValuex;
extern RogueRuntimeType TypeRogueTableEntryxRogueString_RogueValuex;

typedef struct RogueTableEntryxRogueString_RogueValuexList RogueTableEntryxRogueString_RogueValuexList;
extern RogueRuntimeType TypeRogueTableEntryxRogueString_RogueValuexList;

typedef struct RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN;

typedef struct RogueTableBTABLERogueString_RogueValueETABLE RogueTableBTABLERogueString_RogueValueETABLE;
extern RogueRuntimeType TypeRogueTableBTABLERogueString_RogueValueETABLE;

typedef struct RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN;

typedef struct RogueConsoleEventTypeList RogueConsoleEventTypeList;
extern RogueRuntimeType TypeRogueConsoleEventTypeList;

typedef struct RogueFileListingOptionList RogueFileListingOptionList;
extern RogueRuntimeType TypeRogueFileListingOptionList;

typedef struct RogueWindowsProcessReader RogueWindowsProcessReader;
extern RogueRuntimeType TypeRogueWindowsProcessReader;

typedef struct RogueWindowsProcessWriter RogueWindowsProcessWriter;
extern RogueRuntimeType TypeRogueWindowsProcessWriter;

typedef struct RogueFDReader RogueFDReader;
extern RogueRuntimeType TypeRogueFDReader;

typedef struct RoguePosixProcessReader RoguePosixProcessReader;
extern RogueRuntimeType TypeRoguePosixProcessReader;

typedef struct RogueMorlock RogueMorlock;
extern RogueRuntimeType TypeRogueMorlock;

typedef struct RoguePackageInfo RoguePackageInfo;
extern RogueRuntimeType TypeRoguePackageInfo;

typedef struct RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN;

typedef struct RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx;
extern RogueRuntimeType TypeRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx;

typedef struct RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList;
extern RogueRuntimeType TypeRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList;

typedef struct RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN;

typedef struct RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE;
extern RogueRuntimeType TypeRogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE;

typedef struct RogueCommandLineParser RogueCommandLineParser;
extern RogueRuntimeType TypeRogueCommandLineParser;

typedef struct RogueFunction_326 RogueFunction_326;
extern RogueRuntimeType TypeRogueFunction_326;

typedef struct RogueFunction_327 RogueFunction_327;
extern RogueRuntimeType TypeRogueFunction_327;

typedef struct RogueFunction_388 RogueFunction_388;
extern RogueRuntimeType TypeRogueFunction_388;

typedef struct RogueFunction_389 RogueFunction_389;
extern RogueRuntimeType TypeRogueFunction_389;

typedef struct RogueInt32List RogueInt32List;
extern RogueRuntimeType TypeRogueInt32List;

typedef struct RogueTableEntryxRogueString_RogueStringx RogueTableEntryxRogueString_RogueStringx;
extern RogueRuntimeType TypeRogueTableEntryxRogueString_RogueStringx;

typedef struct RogueTableEntryxRogueString_RogueStringxList RogueTableEntryxRogueString_RogueStringxList;
extern RogueRuntimeType TypeRogueTableEntryxRogueString_RogueStringxList;

typedef struct RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN;
extern RogueRuntimeType TypeRogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN;

typedef struct RogueTableBTABLERogueString_RogueStringETABLE RogueTableBTABLERogueString_RogueStringETABLE;
extern RogueRuntimeType TypeRogueTableBTABLERogueString_RogueStringETABLE;

typedef struct RogueSystemEnvironment RogueSystemEnvironment;
extern RogueRuntimeType TypeRogueSystemEnvironment;

typedef struct RogueUTF16StringList RogueUTF16StringList;
extern RogueRuntimeType TypeRogueUTF16StringList;

typedef struct RogueObjectPoolxRogueUTF16Stringx RogueObjectPoolxRogueUTF16Stringx;
extern RogueRuntimeType TypeRogueObjectPoolxRogueUTF16Stringx;

typedef struct RogueDataReader RogueDataReader;
extern RogueRuntimeType TypeRogueDataReader;

typedef struct RogueBootstrap RogueBootstrap;
extern RogueRuntimeType TypeRogueBootstrap;

typedef struct RoguePackage RoguePackage;
extern RogueRuntimeType TypeRoguePackage;

typedef struct RoguePlatforms RoguePlatforms;
extern RogueRuntimeType TypeRoguePlatforms;

typedef struct RogueFDWriter RogueFDWriter;
extern RogueRuntimeType TypeRogueFDWriter;

typedef struct RogueFunction_490 RogueFunction_490;
extern RogueRuntimeType TypeRogueFunction_490;

typedef struct RogueFunction_491 RogueFunction_491;
extern RogueRuntimeType TypeRogueFunction_491;

typedef struct RogueLineReader RogueLineReader;
extern RogueRuntimeType TypeRogueLineReader;

typedef struct RogueFileListing RogueFileListing;
extern RogueRuntimeType TypeRogueFileListing;

typedef struct RogueFunction_495 RogueFunction_495;
extern RogueRuntimeType TypeRogueFunction_495;

typedef struct RogueFunction_496 RogueFunction_496;
extern RogueRuntimeType TypeRogueFunction_496;

typedef struct RogueSpanList RogueSpanList;
extern RogueRuntimeType TypeRogueSpanList;

typedef struct RogueTimsortxRogueStringx RogueTimsortxRogueStringx;
extern RogueRuntimeType TypeRogueTimsortxRogueStringx;

typedef struct RogueWorkListxRogueStringx RogueWorkListxRogueStringx;
extern RogueRuntimeType TypeRogueWorkListxRogueStringx;

typedef struct RogueWorkListxRogueString_Defaultx RogueWorkListxRogueString_Defaultx;
extern RogueRuntimeType TypeRogueWorkListxRogueString_Defaultx;

typedef struct RogueFunction_513 RogueFunction_513;
extern RogueRuntimeType TypeRogueFunction_513;

typedef struct RogueFunction_514 RogueFunction_514;
extern RogueRuntimeType TypeRogueFunction_514;

typedef struct RoguePackageError RoguePackageError;
extern RogueRuntimeType TypeRoguePackageError;

typedef struct RogueJSON RogueJSON;
extern RogueRuntimeType TypeRogueJSON;

typedef struct RogueJSONParseError RogueJSONParseError;
extern RogueRuntimeType TypeRogueJSONParseError;

typedef struct RogueScanner RogueScanner;
extern RogueRuntimeType TypeRogueScanner;

typedef struct RogueJSONParser RogueJSONParser;
extern RogueRuntimeType TypeRogueJSONParser;

typedef struct RogueFunction_529 RogueFunction_529;
extern RogueRuntimeType TypeRogueFunction_529;

typedef struct RogueFunction_530 RogueFunction_530;
extern RogueRuntimeType TypeRogueFunction_530;

typedef struct RogueFunction_531 RogueFunction_531;
extern RogueRuntimeType TypeRogueFunction_531;

typedef struct RogueZip RogueZip;
extern RogueRuntimeType TypeRogueZip;

typedef struct RogueFiles RogueFiles;
extern RogueRuntimeType TypeRogueFiles;

typedef struct RogueFunction_551 RogueFunction_551;
extern RogueRuntimeType TypeRogueFunction_551;

typedef struct RogueFunction_552 RogueFunction_552;
extern RogueRuntimeType TypeRogueFunction_552;

typedef struct RogueFunction_553 RogueFunction_553;
extern RogueRuntimeType TypeRogueFunction_553;

typedef struct RogueFunction_554 RogueFunction_554;
extern RogueRuntimeType TypeRogueFunction_554;

typedef struct RogueFunction_555 RogueFunction_555;
extern RogueRuntimeType TypeRogueFunction_555;

typedef struct RogueWorkListxRogueBytex RogueWorkListxRogueBytex;
extern RogueRuntimeType TypeRogueWorkListxRogueBytex;

typedef struct RogueWorkListxRogueByte_Defaultx RogueWorkListxRogueByte_Defaultx;
extern RogueRuntimeType TypeRogueWorkListxRogueByte_Defaultx;

typedef struct RogueFunction_558 RogueFunction_558;
extern RogueRuntimeType TypeRogueFunction_558;

typedef struct RogueFunction_559 RogueFunction_559;
extern RogueRuntimeType TypeRogueFunction_559;

typedef struct RogueFunction_560 RogueFunction_560;
extern RogueRuntimeType TypeRogueFunction_560;

typedef struct RogueFunction_574 RogueFunction_574;
extern RogueRuntimeType TypeRogueFunction_574;

typedef struct RogueFunction_575 RogueFunction_575;
extern RogueRuntimeType TypeRogueFunction_575;

typedef struct RogueSetxRogueStringx RogueSetxRogueStringx;
extern RogueRuntimeType TypeRogueSetxRogueStringx;

typedef struct RogueConsoleErrorPrinter RogueConsoleErrorPrinter;
extern RogueRuntimeType TypeRogueConsoleErrorPrinter;

typedef struct RogueFunction_578 RogueFunction_578;
extern RogueRuntimeType TypeRogueFunction_578;

typedef struct RogueStandardConsoleMode RogueStandardConsoleMode;
extern RogueRuntimeType TypeRogueStandardConsoleMode;

typedef struct RogueFunction_580 RogueFunction_580;
extern RogueRuntimeType TypeRogueFunction_580;

typedef struct RogueConsoleEventList RogueConsoleEventList;
extern RogueRuntimeType TypeRogueConsoleEventList;

typedef struct RogueImmediateConsoleMode RogueImmediateConsoleMode;
extern RogueRuntimeType TypeRogueImmediateConsoleMode;

typedef struct RogueUnixConsoleMouseEventTypeList RogueUnixConsoleMouseEventTypeList;
extern RogueRuntimeType TypeRogueUnixConsoleMouseEventTypeList;

typedef struct RogueUnrecognizedOptionError RogueUnrecognizedOptionError;
extern RogueRuntimeType TypeRogueUnrecognizedOptionError;

typedef struct RogueValueExpectedError RogueValueExpectedError;
extern RogueRuntimeType TypeRogueValueExpectedError;

typedef struct RogueUnexpectedValueError RogueUnexpectedValueError;
extern RogueRuntimeType TypeRogueUnexpectedValueError;

#include <stdio.h>
#include <sys/stat.h>

#if !defined(ROGUE_PLATFORM_WINDOWS) && !defined(ROGUE_PLATFORM_EMBEDDED)
  #include <dirent.h>
#endif
#define ROGUE_VALUE_TYPE_UNDEFINED 0
#define ROGUE_VALUE_TYPE_BYTE      1
#define ROGUE_VALUE_TYPE_CHARACTER 2
#define ROGUE_VALUE_TYPE_INT32     3
#define ROGUE_VALUE_TYPE_INT64     4
#define ROGUE_VALUE_TYPE_REAL32    5
#define ROGUE_VALUE_TYPE_REAL64    6
#define ROGUE_VALUE_TYPE_LOGICAL   7
#define ROGUE_VALUE_TYPE_NULL      8
#define ROGUE_VALUE_TYPE_XY        9
#define ROGUE_VALUE_TYPE_BOX       10
#define ROGUE_VALUE_TYPE_OBJECT    11
#define ROGUE_VALUE_TYPE_STRING    12
#define ROGUE_VALUE_TYPE_LIST      13
#define ROGUE_VALUE_TYPE_TABLE     14
#define ROGUE_STRING_COPY           0
#define ROGUE_STRING_BORROW         1
#define ROGUE_STRING_ADOPT          2
#define ROGUE_STRING_PERMANENT      3
#define ROGUE_STRING_PERMANENT_COPY 4

RogueString* RogueString_create( const char* cstring );
RogueString* RogueString_create_from_utf8( const char* cstring, int byte_count, int usage );
RogueString* RogueString_create_from_ascii256( const char* cstring, int byte_count, int usage );
RogueString* RogueString_create_permanent( const char* cstring );
RogueString* RogueString_create_string_table_entry( const char* cstring );
RogueInt32   RogueString_compute_hash_code( RogueString* THIS, RogueInt32 starting_hash );
const char*  RogueString_to_c_string( RogueString* st );
RogueInt32   RogueString_utf8_character_count( const char* cstring, int byte_count );

extern RogueInt32 Rogue_string_table_count;
#if defined(ROGUE_PLATFORM_LINUX)
  #include <signal.h>
#endif
#if defined(ROGUE_PLATFORM_WINDOWS)
  #include <time.h>
  #include <sys/timeb.h>
#else
  #include <sys/time.h>
  #include <spawn.h>
#endif

extern int    Rogue_argc;
extern char** Rogue_argv;

#ifndef PATH_MAX
  #define PATH_MAX 4096
#endif

extern char **environ;
#if defined(ROGUE_PLATFORM_WINDOWS)
  #include <io.h>
  #define ROGUE_READ_CALL _read
#elif !defined(ROGUE_PLATFORM_EMBEDDED)
  #include <fcntl.h>
  #include <termios.h>
  #include <unistd.h>
  #include <sys/ioctl.h>
  #define ROGUE_READ_CALL read
#endif

#ifndef STDIN_FILENO      /* Probably Windows */
  #define STDIN_FILENO  0 /* Probably correct */
  #define STDOUT_FILENO 1
  #define STDERR_FILENO 2
#endif

void Rogue_fwrite( const char* utf8, int byte_count, int out );
#ifdef ROGUE_PLATFORM_WINDOWS
#else
  #include <spawn.h>
  #include <poll.h>
  #include <sys/wait.h>
  #include <sys/errno.h>
  extern char** environ;
#endif
#if !defined(ROGUE_PLATFORM_WINDOWS) && !defined(ROGUE_PLATFORM_EMBEDDED)
  #include <dirent.h>
#endif
#define MINIZ_EXPORT
/* miniz.c 2.2.0 - public domain deflate/inflate, zlib-subset, ZIP
   reading/writing/appending, PNG writing See "unlicense" statement at the end
   of this file. Rich Geldreich <richgel99@gmail.com>, last updated Oct. 13,
   2013 Implements RFC 1950: http://www.ietf.org/rfc/rfc1950.txt and RFC 1951:
   http://www.ietf.org/rfc/rfc1951.txt

   Most API's defined in miniz.c are optional. For example, to disable the
   archive related functions just define MINIZ_NO_ARCHIVE_APIS, or to get rid of
   all stdio usage define MINIZ_NO_STDIO (see the list below for more macros).

   * Low-level Deflate/Inflate implementation notes:

     Compression: Use the "tdefl" API's. The compressor supports raw, static,
   and dynamic blocks, lazy or greedy parsing, match length filtering, RLE-only,
   and Huffman-only streams. It performs and compresses approximately as well as
   zlib.

     Decompression: Use the "tinfl" API's. The entire decompressor is
   implemented as a single function coroutine: see tinfl_decompress(). It
   supports decompression into a 32KB (or larger power of 2) wrapping buffer, or
   into a memory block large enough to hold the entire file.

     The low-level tdefl/tinfl API's do not make any use of dynamic memory
   allocation.

   * zlib-style API notes:

     miniz.c implements a fairly large subset of zlib. There's enough
   functionality present for it to be a drop-in zlib replacement in many apps:
        The z_stream struct, optional memory allocation callbacks
        deflateInit/deflateInit2/deflate/deflateReset/deflateEnd/deflateBound
        inflateInit/inflateInit2/inflate/inflateReset/inflateEnd
        compress, compress2, compressBound, uncompress
        CRC-32, Adler-32 - Using modern, minimal code size, CPU cache friendly
   routines. Supports raw deflate streams or standard zlib streams with adler-32
   checking.

     Limitations:
      The callback API's are not implemented yet. No support for gzip headers or
   zlib static dictionaries. I've tried to closely emulate zlib's various
   flavors of stream flushing and return status codes, but there are no
   guarantees that miniz.c pulls this off perfectly.

   * PNG writing: See the tdefl_write_image_to_png_file_in_memory() function,
   originally written by Alex Evans. Supports 1-4 bytes/pixel images.

   * ZIP archive API notes:

     The ZIP archive API's where designed with simplicity and efficiency in
   mind, with just enough abstraction to get the job done with minimal fuss.
   There are simple API's to retrieve file information, read files from existing
   archives, create new archives, append new files to existing archives, or
   clone archive data from one archive to another. It supports archives located
   in memory or the heap, on disk (using stdio.h), or you can specify custom
   file read/write callbacks.

     - Archive reading: Just call this function to read a single file from a
   disk archive:

      void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename, const
   char *pArchive_name, size_t *pSize, mz_uint zip_flags);

     For more complex cases, use the "mz_zip_reader" functions. Upon opening an
   archive, the entire central directory is located and read as-is into memory,
   and subsequent file access only occurs when reading individual files.

     - Archives file scanning: The simple way is to use this function to scan a
   loaded archive for a specific file:

     int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName,
   const char *pComment, mz_uint flags);

     The locate operation can optionally check file comments too, which (as one
   example) can be used to identify multiple versions of the same file in an
   archive. This function uses a simple linear search through the central
     directory, so it's not very fast.

     Alternately, you can iterate through all the files in an archive (using
   mz_zip_reader_get_num_files()) and retrieve detailed info on each file by
   calling mz_zip_reader_file_stat().

     - Archive creation: Use the "mz_zip_writer" functions. The ZIP writer
   immediately writes compressed file data to disk and builds an exact image of
   the central directory in memory. The central directory image is written all
   at once at the end of the archive file when the archive is finalized.

     The archive writer can optionally align each file's local header and file
   data to any power of 2 alignment, which can be useful when the archive will
   be read from optical media. Also, the writer supports placing arbitrary data
   blobs at the very beginning of ZIP archives. Archives written using either
   feature are still readable by any ZIP tool.

     - Archive appending: The simple way to add a single file to an archive is
   to call this function:

      mz_bool mz_zip_add_mem_to_archive_file_in_place(const char *pZip_filename,
   const char *pArchive_name, const void *pBuf, size_t buf_size, const void
   *pComment, mz_uint16 comment_size, mz_uint level_and_flags);

     The archive will be created if it doesn't already exist, otherwise it'll be
   appended to. Note the appending is done in-place and is not an atomic
   operation, so if something goes wrong during the operation it's possible the
   archive could be left without a central directory (although the local file
   headers and file data will be fine, so the archive will be recoverable).

     For more complex archive modification scenarios:
     1. The safest way is to use a mz_zip_reader to read the existing archive,
   cloning only those bits you want to preserve into a new archive using using
   the mz_zip_writer_add_from_zip_reader() function (which compiles the
     compressed file data as-is). When you're done, delete the old archive and
   rename the newly written archive, and you're done. This is safe but requires
   a bunch of temporary disk space or heap memory.

     2. Or, you can convert an mz_zip_reader in-place to an mz_zip_writer using
   mz_zip_writer_init_from_reader(), append new files as needed, then finalize
   the archive which will write an updated central directory to the original
   archive. (This is basically what mz_zip_add_mem_to_archive_file_in_place()
   does.) There's a possibility that the archive's central directory could be
   lost with this method if anything goes wrong, though.

     - ZIP archive support limitations:
     No spanning support. Extraction functions can only handle unencrypted,
   stored or deflated files. Requires streams capable of seeking.

   * This is a header file library, like stb_image.c. To get only a header file,
   either cut and paste the below header, or create miniz.h, #define
   MINIZ_HEADER_FILE_ONLY, and then include miniz.c from it.

   * Important: For best perf. be sure to customize the below macros for your
   target platform: #define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1 #define
   MINIZ_LITTLE_ENDIAN 1 #define MINIZ_HAS_64BIT_REGISTERS 1

   * On platforms using glibc, Be sure to "#define _LARGEFILE64_SOURCE 1" before
   including miniz.c to ensure miniz uses the 64-bit variants: fopen64(),
   stat64(), etc. Otherwise you won't be able to process large files (i.e.
   32-bit stat() fails for me on files > 0x7FFFFFFF bytes).
*/
#pragma once

/* Defines to completely disable specific portions of miniz.c:
   If all macros here are defined the only functionality remaining will be
   CRC-32, adler-32, tinfl, and tdefl. */

/* Define MINIZ_NO_STDIO to disable all usage and any functions which rely on
 * stdio for file I/O. */
/*#define MINIZ_NO_STDIO */

/* If MINIZ_NO_TIME is specified then the ZIP archive functions will not be able
 * to get the current time, or */
/* get/set file times, and the C run-time funcs that get/set times won't be
 * called. */
/* The current downside is the times written to your archives will be from 1979.
 */
/*#define MINIZ_NO_TIME */

/* Define MINIZ_NO_ARCHIVE_APIS to disable all ZIP archive API's. */
/*#define MINIZ_NO_ARCHIVE_APIS */

/* Define MINIZ_NO_ARCHIVE_WRITING_APIS to disable all writing related ZIP
 * archive API's. */
/*#define MINIZ_NO_ARCHIVE_WRITING_APIS */

/* Define MINIZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression
 * API's. */
/*#define MINIZ_NO_ZLIB_APIS */

/* Define MINIZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent
 * conflicts against stock zlib. */
/*#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES */

/* Define MINIZ_NO_MALLOC to disable all calls to malloc, free, and realloc.
   Note if MINIZ_NO_MALLOC is defined then the user must always provide custom
   user alloc/free/realloc callbacks to the zlib and archive API's, and a few
   stand-alone helper API's which don't provide custom user functions (such as
   tdefl_compress_mem_to_heap() and tinfl_decompress_mem_to_heap()) won't work.
 */
/*#define MINIZ_NO_MALLOC */

#if defined(__TINYC__) && (defined(__linux) || defined(__linux__))
/* TODO: Work around "error: include file 'sys\utime.h' when compiling with tcc
 * on Linux */
#define MINIZ_NO_TIME
#endif

#include <stddef.h>

#if !defined(MINIZ_NO_TIME) && !defined(MINIZ_NO_ARCHIVE_APIS)
#include <time.h>
#endif

#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) ||                \
    defined(__i386) || defined(__i486__) || defined(__i486) ||                 \
    defined(i386) || defined(__ia64__) || defined(__x86_64__)
/* MINIZ_X86_OR_X64_CPU is only used to help set the below macros. */
#define MINIZ_X86_OR_X64_CPU 1
#else
#define MINIZ_X86_OR_X64_CPU 0
#endif

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || MINIZ_X86_OR_X64_CPU
/* Set MINIZ_LITTLE_ENDIAN to 1 if the processor is little endian. */
#define MINIZ_LITTLE_ENDIAN 1
#else
#define MINIZ_LITTLE_ENDIAN 0
#endif

/* Set MINIZ_USE_UNALIGNED_LOADS_AND_STORES only if not set */
#if !defined(MINIZ_USE_UNALIGNED_LOADS_AND_STORES)
#if MINIZ_X86_OR_X64_CPU
/* Set MINIZ_USE_UNALIGNED_LOADS_AND_STORES to 1 on CPU's that permit efficient
 * integer loads and stores from unaligned addresses. */
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#define MINIZ_UNALIGNED_USE_MEMCPY
#else
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 0
#endif
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) ||              \
    defined(_LP64) || defined(__LP64__) || defined(__ia64__) ||                \
    defined(__x86_64__)
/* Set MINIZ_HAS_64BIT_REGISTERS to 1 if operations on 64-bit integers are
 * reasonably fast (and don't involve compiler generated calls to helper
 * functions). */
#define MINIZ_HAS_64BIT_REGISTERS 1
#else
#define MINIZ_HAS_64BIT_REGISTERS 0
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- zlib-style API Definitions. */

/* For more compatibility with zlib, miniz.c uses unsigned long for some
 * parameters/struct members. Beware: mz_ulong can be either 32 or 64-bits! */
typedef unsigned long mz_ulong;

/* mz_free() internally uses the MZ_FREE() macro (which by default calls free()
 * unless you've modified the MZ_MALLOC macro) to release a block allocated from
 * the heap. */
MINIZ_EXPORT void mz_free(void *p);

#define MZ_ADLER32_INIT (1)
/* mz_adler32() returns the initial adler-32 value to use when called with
 * ptr==NULL. */
MINIZ_EXPORT mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr,
                                 size_t buf_len);

#define MZ_CRC32_INIT (0)
/* mz_crc32() returns the initial CRC-32 value to use when called with
 * ptr==NULL. */
MINIZ_EXPORT mz_ulong mz_crc32(mz_ulong crc, const unsigned char *ptr,
                               size_t buf_len);

/* Compression strategies. */
enum {
  MZ_DEFAULT_STRATEGY = 0,
  MZ_FILTERED = 1,
  MZ_HUFFMAN_ONLY = 2,
  MZ_RLE = 3,
  MZ_FIXED = 4
};

/* Method */
#define MZ_DEFLATED 8

/* Heap allocation callbacks.
Note that mz_alloc_func parameter types purposely differ from zlib's: items/size
is size_t, not unsigned long. */
typedef void *(*mz_alloc_func)(void *opaque, size_t items, size_t size);
typedef void (*mz_free_func)(void *opaque, void *address);
typedef void *(*mz_realloc_func)(void *opaque, void *address, size_t items,
                                 size_t size);

/* Compression levels: 0-9 are the standard zlib-style levels, 10 is best
 * possible compression (not zlib compatible, and may be very slow),
 * MZ_DEFAULT_COMPRESSION=MZ_DEFAULT_LEVEL. */
enum {
  MZ_NO_COMPRESSION = 0,
  MZ_BEST_SPEED = 1,
  MZ_BEST_COMPRESSION = 9,
  MZ_UBER_COMPRESSION = 10,
  MZ_DEFAULT_LEVEL = 6,
  MZ_DEFAULT_COMPRESSION = -1
};

#define MZ_VERSION "10.2.0"
#define MZ_VERNUM 0xA100
#define MZ_VER_MAJOR 10
#define MZ_VER_MINOR 2
#define MZ_VER_REVISION 0
#define MZ_VER_SUBREVISION 0

#ifndef MINIZ_NO_ZLIB_APIS

/* Flush values. For typical usage you only need MZ_NO_FLUSH and MZ_FINISH. The
 * other values are for advanced use (refer to the zlib docs). */
enum {
  MZ_NO_FLUSH = 0,
  MZ_PARTIAL_FLUSH = 1,
  MZ_SYNC_FLUSH = 2,
  MZ_FULL_FLUSH = 3,
  MZ_FINISH = 4,
  MZ_BLOCK = 5
};

/* Return status codes. MZ_PARAM_ERROR is non-standard. */
enum {
  MZ_OK = 0,
  MZ_STREAM_END = 1,
  MZ_NEED_DICT = 2,
  MZ_ERRNO = -1,
  MZ_STREAM_ERROR = -2,
  MZ_DATA_ERROR = -3,
  MZ_MEM_ERROR = -4,
  MZ_BUF_ERROR = -5,
  MZ_VERSION_ERROR = -6,
  MZ_PARAM_ERROR = -10000
};

/* Window bits */
#define MZ_DEFAULT_WINDOW_BITS 15

struct mz_internal_state;

/* Compression/decompression stream struct. */
typedef struct mz_stream_s {
  const unsigned char *next_in; /* pointer to next byte to read */
  unsigned int avail_in;        /* number of bytes available at next_in */
  mz_ulong total_in;            /* total number of bytes consumed so far */

  unsigned char *next_out; /* pointer to next byte to write */
  unsigned int avail_out;  /* number of bytes that can be written to next_out */
  mz_ulong total_out;      /* total number of bytes produced so far */

  char *msg; /* error msg (unused) */
  struct mz_internal_state
      *state; /* internal state, allocated by zalloc/zfree */

  mz_alloc_func
      zalloc; /* optional heap allocation function (defaults to malloc) */
  mz_free_func zfree; /* optional heap free function (defaults to free) */
  void *opaque;       /* heap alloc function user pointer */

  int data_type;     /* data_type (unused) */
  mz_ulong adler;    /* adler32 of the source or uncompressed data */
  mz_ulong reserved; /* not used */
} mz_stream;

typedef mz_stream *mz_streamp;

/* Returns the version string of miniz.c. */
MINIZ_EXPORT const char *mz_version(void);

/* mz_deflateInit() initializes a compressor with default options: */
/* Parameters: */
/*  pStream must point to an initialized mz_stream struct. */
/*  level must be between [MZ_NO_COMPRESSION, MZ_BEST_COMPRESSION]. */
/*  level 1 enables a specially optimized compression function that's been
 * optimized purely for performance, not ratio. */
/*  (This special func. is currently only enabled when
 * MINIZ_USE_UNALIGNED_LOADS_AND_STORES and MINIZ_LITTLE_ENDIAN are defined.) */
/* Return values: */
/*  MZ_OK on success. */
/*  MZ_STREAM_ERROR if the stream is bogus. */
/*  MZ_PARAM_ERROR if the input parameters are bogus. */
/*  MZ_MEM_ERROR on out of memory. */
MINIZ_EXPORT int mz_deflateInit(mz_streamp pStream, int level);

/* mz_deflateInit2() is like mz_deflate(), except with more control: */
/* Additional parameters: */
/*   method must be MZ_DEFLATED */
/*   window_bits must be MZ_DEFAULT_WINDOW_BITS (to wrap the deflate stream with
 * zlib header/adler-32 footer) or -MZ_DEFAULT_WINDOW_BITS (raw deflate/no
 * header or footer) */
/*   mem_level must be between [1, 9] (it's checked but ignored by miniz.c) */
MINIZ_EXPORT int mz_deflateInit2(mz_streamp pStream, int level, int method,
                                 int window_bits, int mem_level, int strategy);

/* Quickly resets a compressor without having to reallocate anything. Same as
 * calling mz_deflateEnd() followed by mz_deflateInit()/mz_deflateInit2(). */
MINIZ_EXPORT int mz_deflateReset(mz_streamp pStream);

/* mz_deflate() compresses the input to output, consuming as much of the input
 * and producing as much output as possible. */
/* Parameters: */
/*   pStream is the stream to read from and write to. You must initialize/update
 * the next_in, avail_in, next_out, and avail_out members. */
/*   flush may be MZ_NO_FLUSH, MZ_PARTIAL_FLUSH/MZ_SYNC_FLUSH, MZ_FULL_FLUSH, or
 * MZ_FINISH. */
/* Return values: */
/*   MZ_OK on success (when flushing, or if more input is needed but not
 * available, and/or there's more output to be written but the output buffer is
 * full). */
/*   MZ_STREAM_END if all input has been consumed and all output bytes have been
 * written. Don't call mz_deflate() on the stream anymore. */
/*   MZ_STREAM_ERROR if the stream is bogus. */
/*   MZ_PARAM_ERROR if one of the parameters is invalid. */
/*   MZ_BUF_ERROR if no forward progress is possible because the input and/or
 * output buffers are empty. (Fill up the input buffer or free up some output
 * space and try again.) */
MINIZ_EXPORT int mz_deflate(mz_streamp pStream, int flush);

/* mz_deflateEnd() deinitializes a compressor: */
/* Return values: */
/*  MZ_OK on success. */
/*  MZ_STREAM_ERROR if the stream is bogus. */
MINIZ_EXPORT int mz_deflateEnd(mz_streamp pStream);

/* mz_deflateBound() returns a (very) conservative upper bound on the amount of
 * data that could be generated by deflate(), assuming flush is set to only
 * MZ_NO_FLUSH or MZ_FINISH. */
MINIZ_EXPORT mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len);

/* Single-call compression functions mz_compress() and mz_compress2(): */
/* Returns MZ_OK on success, or one of the error codes from mz_deflate() on
 * failure. */
MINIZ_EXPORT int mz_compress(unsigned char *pDest, mz_ulong *pDest_len,
                             const unsigned char *pSource, mz_ulong source_len);
MINIZ_EXPORT int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len,
                              const unsigned char *pSource, mz_ulong source_len,
                              int level);

/* mz_compressBound() returns a (very) conservative upper bound on the amount of
 * data that could be generated by calling mz_compress(). */
MINIZ_EXPORT mz_ulong mz_compressBound(mz_ulong source_len);

/* Initializes a decompressor. */
MINIZ_EXPORT int mz_inflateInit(mz_streamp pStream);

/* mz_inflateInit2() is like mz_inflateInit() with an additional option that
 * controls the window size and whether or not the stream has been wrapped with
 * a zlib header/footer: */
/* window_bits must be MZ_DEFAULT_WINDOW_BITS (to parse zlib header/footer) or
 * -MZ_DEFAULT_WINDOW_BITS (raw deflate). */
MINIZ_EXPORT int mz_inflateInit2(mz_streamp pStream, int window_bits);

/* Quickly resets a compressor without having to reallocate anything. Same as
 * calling mz_inflateEnd() followed by mz_inflateInit()/mz_inflateInit2(). */
MINIZ_EXPORT int mz_inflateReset(mz_streamp pStream);

/* Decompresses the input stream to the output, consuming only as much of the
 * input as needed, and writing as much to the output as possible. */
/* Parameters: */
/*   pStream is the stream to read from and write to. You must initialize/update
 * the next_in, avail_in, next_out, and avail_out members. */
/*   flush may be MZ_NO_FLUSH, MZ_SYNC_FLUSH, or MZ_FINISH. */
/*   On the first call, if flush is MZ_FINISH it's assumed the input and output
 * buffers are both sized large enough to decompress the entire stream in a
 * single call (this is slightly faster). */
/*   MZ_FINISH implies that there are no more source bytes available beside
 * what's already in the input buffer, and that the output buffer is large
 * enough to hold the rest of the decompressed data. */
/* Return values: */
/*   MZ_OK on success. Either more input is needed but not available, and/or
 * there's more output to be written but the output buffer is full. */
/*   MZ_STREAM_END if all needed input has been consumed and all output bytes
 * have been written. For zlib streams, the adler-32 of the decompressed data
 * has also been verified. */
/*   MZ_STREAM_ERROR if the stream is bogus. */
/*   MZ_DATA_ERROR if the deflate stream is invalid. */
/*   MZ_PARAM_ERROR if one of the parameters is invalid. */
/*   MZ_BUF_ERROR if no forward progress is possible because the input buffer is
 * empty but the inflater needs more input to continue, or if the output buffer
 * is not large enough. Call mz_inflate() again */
/*   with more input data, or with more room in the output buffer (except when
 * using single call decompression, described above). */
MINIZ_EXPORT int mz_inflate(mz_streamp pStream, int flush);

/* Deinitializes a decompressor. */
MINIZ_EXPORT int mz_inflateEnd(mz_streamp pStream);

/* Single-call decompression. */
/* Returns MZ_OK on success, or one of the error codes from mz_inflate() on
 * failure. */
MINIZ_EXPORT int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len,
                               const unsigned char *pSource,
                               mz_ulong source_len);
MINIZ_EXPORT int mz_uncompress2(unsigned char *pDest, mz_ulong *pDest_len,
                                const unsigned char *pSource,
                                mz_ulong *pSource_len);

/* Returns a string description of the specified error code, or NULL if the
 * error code is invalid. */
MINIZ_EXPORT const char *mz_error(int err);

/* Redefine zlib-compatible names to miniz equivalents, so miniz.c can be used
 * as a drop-in replacement for the subset of zlib that miniz.c supports. */
/* Define MINIZ_NO_ZLIB_COMPATIBLE_NAMES to disable zlib-compatibility if you
 * use zlib in the same project. */
#ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES
typedef unsigned char Byte;
typedef unsigned int uInt;
typedef mz_ulong uLong;
typedef Byte Bytef;
typedef uInt uIntf;
typedef char charf;
typedef int intf;
typedef void *voidpf;
typedef uLong uLongf;
typedef void *voidp;
typedef void *const voidpc;
#define Z_NULL 0
#define Z_NO_FLUSH MZ_NO_FLUSH
#define Z_PARTIAL_FLUSH MZ_PARTIAL_FLUSH
#define Z_SYNC_FLUSH MZ_SYNC_FLUSH
#define Z_FULL_FLUSH MZ_FULL_FLUSH
#define Z_FINISH MZ_FINISH
#define Z_BLOCK MZ_BLOCK
#define Z_OK MZ_OK
#define Z_STREAM_END MZ_STREAM_END
#define Z_NEED_DICT MZ_NEED_DICT
#define Z_ERRNO MZ_ERRNO
#define Z_STREAM_ERROR MZ_STREAM_ERROR
#define Z_DATA_ERROR MZ_DATA_ERROR
#define Z_MEM_ERROR MZ_MEM_ERROR
#define Z_BUF_ERROR MZ_BUF_ERROR
#define Z_VERSION_ERROR MZ_VERSION_ERROR
#define Z_PARAM_ERROR MZ_PARAM_ERROR
#define Z_NO_COMPRESSION MZ_NO_COMPRESSION
#define Z_BEST_SPEED MZ_BEST_SPEED
#define Z_BEST_COMPRESSION MZ_BEST_COMPRESSION
#define Z_DEFAULT_COMPRESSION MZ_DEFAULT_COMPRESSION
#define Z_DEFAULT_STRATEGY MZ_DEFAULT_STRATEGY
#define Z_FILTERED MZ_FILTERED
#define Z_HUFFMAN_ONLY MZ_HUFFMAN_ONLY
#define Z_RLE MZ_RLE
#define Z_FIXED MZ_FIXED
#define Z_DEFLATED MZ_DEFLATED
#define Z_DEFAULT_WINDOW_BITS MZ_DEFAULT_WINDOW_BITS
#define alloc_func mz_alloc_func
#define free_func mz_free_func
#define internal_state mz_internal_state
#define z_stream mz_stream
#define deflateInit mz_deflateInit
#define deflateInit2 mz_deflateInit2
#define deflateReset mz_deflateReset
#define deflate mz_deflate
#define deflateEnd mz_deflateEnd
#define deflateBound mz_deflateBound
#define compress mz_compress
#define compress2 mz_compress2
#define compressBound mz_compressBound
#define inflateInit mz_inflateInit
#define inflateInit2 mz_inflateInit2
#define inflateReset mz_inflateReset
#define inflate mz_inflate
#define inflateEnd mz_inflateEnd
#define uncompress mz_uncompress
#define uncompress2 mz_uncompress2
#define crc32 mz_crc32
#define adler32 mz_adler32
#define MAX_WBITS 15
#define MAX_MEM_LEVEL 9
#define zError mz_error
#define ZLIB_VERSION MZ_VERSION
#define ZLIB_VERNUM MZ_VERNUM
#define ZLIB_VER_MAJOR MZ_VER_MAJOR
#define ZLIB_VER_MINOR MZ_VER_MINOR
#define ZLIB_VER_REVISION MZ_VER_REVISION
#define ZLIB_VER_SUBREVISION MZ_VER_SUBREVISION
#define zlibVersion mz_version
#define zlib_version mz_version()
#endif /* #ifndef MINIZ_NO_ZLIB_COMPATIBLE_NAMES */

#endif /* MINIZ_NO_ZLIB_APIS */

#ifdef __cplusplus
}
#endif

#pragma once
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ------------------- Types and macros */
typedef unsigned char mz_uint8;
typedef signed short mz_int16;
typedef unsigned short mz_uint16;
typedef unsigned int mz_uint32;
typedef unsigned int mz_uint;
typedef int64_t mz_int64;
typedef uint64_t mz_uint64;
typedef int mz_bool;

#define MZ_FALSE (0)
#define MZ_TRUE (1)

/* Works around MSVC's spammy "warning C4127: conditional expression is
 * constant" message. */
#ifdef _MSC_VER
#define MZ_MACRO_END while (0, 0)
#else
#define MZ_MACRO_END while (0)
#endif

#ifdef MINIZ_NO_STDIO
#define MZ_FILE void *
#else
#include <stdio.h>
#define MZ_FILE FILE
#endif /* #ifdef MINIZ_NO_STDIO */

#ifdef MINIZ_NO_TIME
typedef struct mz_dummy_time_t_tag {
  int m_dummy;
} mz_dummy_time_t;
#define MZ_TIME_T mz_dummy_time_t
#else
#define MZ_TIME_T time_t
#endif

#define MZ_ASSERT(x) assert(x)

#ifdef MINIZ_NO_MALLOC
#define MZ_MALLOC(x) NULL
#define MZ_FREE(x) (void)x, ((void)0)
#define MZ_REALLOC(p, x) NULL
#else
#define MZ_MALLOC(x) malloc(x)
#define MZ_FREE(x) free(x)
#define MZ_REALLOC(p, x) realloc(p, x)
#endif

#define MZ_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MZ_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MZ_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
#define MZ_READ_LE16(p) *((const mz_uint16 *)(p))
#define MZ_READ_LE32(p) *((const mz_uint32 *)(p))
#else
#define MZ_READ_LE16(p)                                                        \
  ((mz_uint32)(((const mz_uint8 *)(p))[0]) |                                   \
   ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U))
#define MZ_READ_LE32(p)                                                        \
  ((mz_uint32)(((const mz_uint8 *)(p))[0]) |                                   \
   ((mz_uint32)(((const mz_uint8 *)(p))[1]) << 8U) |                           \
   ((mz_uint32)(((const mz_uint8 *)(p))[2]) << 16U) |                          \
   ((mz_uint32)(((const mz_uint8 *)(p))[3]) << 24U))
#endif

#define MZ_READ_LE64(p)                                                        \
  (((mz_uint64)MZ_READ_LE32(p)) |                                              \
   (((mz_uint64)MZ_READ_LE32((const mz_uint8 *)(p) + sizeof(mz_uint32)))       \
    << 32U))

#ifdef _MSC_VER
#define MZ_FORCEINLINE __forceinline
#elif defined(__GNUC__)
#define MZ_FORCEINLINE __inline__ __attribute__((__always_inline__))
#else
#define MZ_FORCEINLINE inline
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern MINIZ_EXPORT void *miniz_def_alloc_func(void *opaque, size_t items,
                                               size_t size);
extern MINIZ_EXPORT void miniz_def_free_func(void *opaque, void *address);
extern MINIZ_EXPORT void *miniz_def_realloc_func(void *opaque, void *address,
                                                 size_t items, size_t size);

#define MZ_UINT16_MAX (0xFFFFU)
#define MZ_UINT32_MAX (0xFFFFFFFFU)

#ifdef __cplusplus
}
#endif
#pragma once

#ifdef __cplusplus
extern "C" {
#endif
/* ------------------- Low-level Compression API Definitions */

/* Set TDEFL_LESS_MEMORY to 1 to use less memory (compression will be slightly
 * slower, and raw/dynamic blocks will be output more frequently). */
#define TDEFL_LESS_MEMORY 0

/* tdefl_init() compression flags logically OR'd together (low 12 bits contain
 * the max. number of probes per dictionary search): */
/* TDEFL_DEFAULT_MAX_PROBES: The compressor defaults to 128 dictionary probes
 * per dictionary search. 0=Huffman only, 1=Huffman+LZ (fastest/crap
 * compression), 4095=Huffman+LZ (slowest/best compression). */
enum {
  TDEFL_HUFFMAN_ONLY = 0,
  TDEFL_DEFAULT_MAX_PROBES = 128,
  TDEFL_MAX_PROBES_MASK = 0xFFF
};

/* TDEFL_WRITE_ZLIB_HEADER: If set, the compressor outputs a zlib header before
 * the deflate data, and the Adler-32 of the source data at the end. Otherwise,
 * you'll get raw deflate data. */
/* TDEFL_COMPUTE_ADLER32: Always compute the adler-32 of the input data (even
 * when not writing zlib headers). */
/* TDEFL_GREEDY_PARSING_FLAG: Set to use faster greedy parsing, instead of more
 * efficient lazy parsing. */
/* TDEFL_NONDETERMINISTIC_PARSING_FLAG: Enable to decrease the compressor's
 * initialization time to the minimum, but the output may vary from run to run
 * given the same input (depending on the contents of memory). */
/* TDEFL_RLE_MATCHES: Only look for RLE matches (matches with a distance of 1)
 */
/* TDEFL_FILTER_MATCHES: Discards matches <= 5 chars if enabled. */
/* TDEFL_FORCE_ALL_STATIC_BLOCKS: Disable usage of optimized Huffman tables. */
/* TDEFL_FORCE_ALL_RAW_BLOCKS: Only use raw (uncompressed) deflate blocks. */
/* The low 12 bits are reserved to control the max # of hash probes per
 * dictionary lookup (see TDEFL_MAX_PROBES_MASK). */
enum {
  TDEFL_WRITE_ZLIB_HEADER = 0x01000,
  TDEFL_COMPUTE_ADLER32 = 0x02000,
  TDEFL_GREEDY_PARSING_FLAG = 0x04000,
  TDEFL_NONDETERMINISTIC_PARSING_FLAG = 0x08000,
  TDEFL_RLE_MATCHES = 0x10000,
  TDEFL_FILTER_MATCHES = 0x20000,
  TDEFL_FORCE_ALL_STATIC_BLOCKS = 0x40000,
  TDEFL_FORCE_ALL_RAW_BLOCKS = 0x80000
};

/* High level compression functions: */
/* tdefl_compress_mem_to_heap() compresses a block in memory to a heap block
 * allocated via malloc(). */
/* On entry: */
/*  pSrc_buf, src_buf_len: Pointer and size of source block to compress. */
/*  flags: The max match finder probes (default is 128) logically OR'd against
 * the above flags. Higher probes are slower but improve compression. */
/* On return: */
/*  Function returns a pointer to the compressed data, or NULL on failure. */
/*  *pOut_len will be set to the compressed data's size, which could be larger
 * than src_buf_len on uncompressible data. */
/*  The caller must free() the returned block when it's no longer needed. */
MINIZ_EXPORT void *tdefl_compress_mem_to_heap(const void *pSrc_buf,
                                              size_t src_buf_len,
                                              size_t *pOut_len, int flags);

/* tdefl_compress_mem_to_mem() compresses a block in memory to another block in
 * memory. */
/* Returns 0 on failure. */
MINIZ_EXPORT size_t tdefl_compress_mem_to_mem(void *pOut_buf,
                                              size_t out_buf_len,
                                              const void *pSrc_buf,
                                              size_t src_buf_len, int flags);

/* Compresses an image to a compressed PNG file in memory. */
/* On entry: */
/*  pImage, w, h, and num_chans describe the image to compress. num_chans may be
 * 1, 2, 3, or 4. */
/*  The image pitch in bytes per scanline will be w*num_chans. The leftmost
 * pixel on the top scanline is stored first in memory. */
/*  level may range from [0,10], use MZ_NO_COMPRESSION, MZ_BEST_SPEED,
 * MZ_BEST_COMPRESSION, etc. or a decent default is MZ_DEFAULT_LEVEL */
/*  If flip is true, the image will be flipped on the Y axis (useful for OpenGL
 * apps). */
/* On return: */
/*  Function returns a pointer to the compressed data, or NULL on failure. */
/*  *pLen_out will be set to the size of the PNG image file. */
/*  The caller must mz_free() the returned heap block (which will typically be
 * larger than *pLen_out) when it's no longer needed. */
MINIZ_EXPORT void *
tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w, int h,
                                           int num_chans, size_t *pLen_out,
                                           mz_uint level, mz_bool flip);
MINIZ_EXPORT void *tdefl_write_image_to_png_file_in_memory(const void *pImage,
                                                           int w, int h,
                                                           int num_chans,
                                                           size_t *pLen_out);

/* Output stream interface. The compressor uses this interface to write
 * compressed data. It'll typically be called TDEFL_OUT_BUF_SIZE at a time. */
typedef mz_bool (*tdefl_put_buf_func_ptr)(const void *pBuf, int len,
                                          void *pUser);

/* tdefl_compress_mem_to_output() compresses a block to an output stream. The
 * above helpers use this function internally. */
MINIZ_EXPORT mz_bool tdefl_compress_mem_to_output(
    const void *pBuf, size_t buf_len, tdefl_put_buf_func_ptr pPut_buf_func,
    void *pPut_buf_user, int flags);

enum {
  TDEFL_MAX_HUFF_TABLES = 3,
  TDEFL_MAX_HUFF_SYMBOLS_0 = 288,
  TDEFL_MAX_HUFF_SYMBOLS_1 = 32,
  TDEFL_MAX_HUFF_SYMBOLS_2 = 19,
  TDEFL_LZ_DICT_SIZE = 32768,
  TDEFL_LZ_DICT_SIZE_MASK = TDEFL_LZ_DICT_SIZE - 1,
  TDEFL_MIN_MATCH_LEN = 3,
  TDEFL_MAX_MATCH_LEN = 258
};

/* TDEFL_OUT_BUF_SIZE MUST be large enough to hold a single entire compressed
 * output block (using static/fixed Huffman codes). */
#if TDEFL_LESS_MEMORY
enum {
  TDEFL_LZ_CODE_BUF_SIZE = 24 * 1024,
  TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13) / 10,
  TDEFL_MAX_HUFF_SYMBOLS = 288,
  TDEFL_LZ_HASH_BITS = 12,
  TDEFL_LEVEL1_HASH_SIZE_MASK = 4095,
  TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3,
  TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS
};
#else
enum {
  TDEFL_LZ_CODE_BUF_SIZE = 64 * 1024,
  TDEFL_OUT_BUF_SIZE = (TDEFL_LZ_CODE_BUF_SIZE * 13) / 10,
  TDEFL_MAX_HUFF_SYMBOLS = 288,
  TDEFL_LZ_HASH_BITS = 15,
  TDEFL_LEVEL1_HASH_SIZE_MASK = 4095,
  TDEFL_LZ_HASH_SHIFT = (TDEFL_LZ_HASH_BITS + 2) / 3,
  TDEFL_LZ_HASH_SIZE = 1 << TDEFL_LZ_HASH_BITS
};
#endif

/* The low-level tdefl functions below may be used directly if the above helper
 * functions aren't flexible enough. The low-level functions don't make any heap
 * allocations, unlike the above helper functions. */
typedef enum {
  TDEFL_STATUS_BAD_PARAM = -2,
  TDEFL_STATUS_PUT_BUF_FAILED = -1,
  TDEFL_STATUS_OKAY = 0,
  TDEFL_STATUS_DONE = 1
} tdefl_status;

/* Must map to MZ_NO_FLUSH, MZ_SYNC_FLUSH, etc. enums */
typedef enum {
  TDEFL_NO_FLUSH = 0,
  TDEFL_SYNC_FLUSH = 2,
  TDEFL_FULL_FLUSH = 3,
  TDEFL_FINISH = 4
} tdefl_flush;

/* tdefl's compression state structure. */
typedef struct {
  tdefl_put_buf_func_ptr m_pPut_buf_func;
  void *m_pPut_buf_user;
  mz_uint m_flags, m_max_probes[2];
  int m_greedy_parsing;
  mz_uint m_adler32, m_lookahead_pos, m_lookahead_size, m_dict_size;
  mz_uint8 *m_pLZ_code_buf, *m_pLZ_flags, *m_pOutput_buf, *m_pOutput_buf_end;
  mz_uint m_num_flags_left, m_total_lz_bytes, m_lz_code_buf_dict_pos, m_bits_in,
      m_bit_buffer;
  mz_uint m_saved_match_dist, m_saved_match_len, m_saved_lit,
      m_output_flush_ofs, m_output_flush_remaining, m_finished, m_block_index,
      m_wants_to_finish;
  tdefl_status m_prev_return_status;
  const void *m_pIn_buf;
  void *m_pOut_buf;
  size_t *m_pIn_buf_size, *m_pOut_buf_size;
  tdefl_flush m_flush;
  const mz_uint8 *m_pSrc;
  size_t m_src_buf_left, m_out_buf_ofs;
  mz_uint8 m_dict[TDEFL_LZ_DICT_SIZE + TDEFL_MAX_MATCH_LEN - 1];
  mz_uint16 m_huff_count[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint16 m_huff_codes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint8 m_huff_code_sizes[TDEFL_MAX_HUFF_TABLES][TDEFL_MAX_HUFF_SYMBOLS];
  mz_uint8 m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE];
  mz_uint16 m_next[TDEFL_LZ_DICT_SIZE];
  mz_uint16 m_hash[TDEFL_LZ_HASH_SIZE];
  mz_uint8 m_output_buf[TDEFL_OUT_BUF_SIZE];
} tdefl_compressor;

/* Initializes the compressor. */
/* There is no corresponding deinit() function because the tdefl API's do not
 * dynamically allocate memory. */
/* pBut_buf_func: If NULL, output data will be supplied to the specified
 * callback. In this case, the user should call the tdefl_compress_buffer() API
 * for compression. */
/* If pBut_buf_func is NULL the user should always call the tdefl_compress()
 * API. */
/* flags: See the above enums (TDEFL_HUFFMAN_ONLY, TDEFL_WRITE_ZLIB_HEADER,
 * etc.) */
MINIZ_EXPORT tdefl_status tdefl_init(tdefl_compressor *d,
                                     tdefl_put_buf_func_ptr pPut_buf_func,
                                     void *pPut_buf_user, int flags);

/* Compresses a block of data, consuming as much of the specified input buffer
 * as possible, and writing as much compressed data to the specified output
 * buffer as possible. */
MINIZ_EXPORT tdefl_status tdefl_compress(tdefl_compressor *d,
                                         const void *pIn_buf,
                                         size_t *pIn_buf_size, void *pOut_buf,
                                         size_t *pOut_buf_size,
                                         tdefl_flush flush);

/* tdefl_compress_buffer() is only usable when the tdefl_init() is called with a
 * non-NULL tdefl_put_buf_func_ptr. */
/* tdefl_compress_buffer() always consumes the entire input buffer. */
MINIZ_EXPORT tdefl_status tdefl_compress_buffer(tdefl_compressor *d,
                                                const void *pIn_buf,
                                                size_t in_buf_size,
                                                tdefl_flush flush);

MINIZ_EXPORT tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d);
MINIZ_EXPORT mz_uint32 tdefl_get_adler32(tdefl_compressor *d);

/* Create tdefl_compress() flags given zlib-style compression parameters. */
/* level may range from [0,10] (where 10 is absolute max compression, but may be
 * much slower on some files) */
/* window_bits may be -15 (raw deflate) or 15 (zlib) */
/* strategy may be either MZ_DEFAULT_STRATEGY, MZ_FILTERED, MZ_HUFFMAN_ONLY,
 * MZ_RLE, or MZ_FIXED */
MINIZ_EXPORT mz_uint tdefl_create_comp_flags_from_zip_params(int level,
                                                             int window_bits,
                                                             int strategy);

#ifndef MINIZ_NO_MALLOC
/* Allocate the tdefl_compressor structure in C so that */
/* non-C language bindings to tdefl_ API don't need to worry about */
/* structure size and allocation mechanism. */
MINIZ_EXPORT tdefl_compressor *tdefl_compressor_alloc(void);
MINIZ_EXPORT void tdefl_compressor_free(tdefl_compressor *pComp);
#endif

#ifdef __cplusplus
}
#endif
#pragma once

/* ------------------- Low-level Decompression API Definitions */

#ifdef __cplusplus
extern "C" {
#endif
/* Decompression flags used by tinfl_decompress(). */
/* TINFL_FLAG_PARSE_ZLIB_HEADER: If set, the input has a valid zlib header and
 * ends with an adler32 checksum (it's a valid zlib stream). Otherwise, the
 * input is a raw deflate stream. */
/* TINFL_FLAG_HAS_MORE_INPUT: If set, there are more input bytes available
 * beyond the end of the supplied input buffer. If clear, the input buffer
 * contains all remaining input. */
/* TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF: If set, the output buffer is large
 * enough to hold the entire decompressed stream. If clear, the output buffer is
 * at least the size of the dictionary (typically 32KB). */
/* TINFL_FLAG_COMPUTE_ADLER32: Force adler-32 checksum computation of the
 * decompressed bytes. */
enum {
  TINFL_FLAG_PARSE_ZLIB_HEADER = 1,
  TINFL_FLAG_HAS_MORE_INPUT = 2,
  TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF = 4,
  TINFL_FLAG_COMPUTE_ADLER32 = 8
};

/* High level decompression functions: */
/* tinfl_decompress_mem_to_heap() decompresses a block in memory to a heap block
 * allocated via malloc(). */
/* On entry: */
/*  pSrc_buf, src_buf_len: Pointer and size of the Deflate or zlib source data
 * to decompress. */
/* On return: */
/*  Function returns a pointer to the decompressed data, or NULL on failure. */
/*  *pOut_len will be set to the decompressed data's size, which could be larger
 * than src_buf_len on uncompressible data. */
/*  The caller must call mz_free() on the returned block when it's no longer
 * needed. */
MINIZ_EXPORT void *tinfl_decompress_mem_to_heap(const void *pSrc_buf,
                                                size_t src_buf_len,
                                                size_t *pOut_len, int flags);

/* tinfl_decompress_mem_to_mem() decompresses a block in memory to another block
 * in memory. */
/* Returns TINFL_DECOMPRESS_MEM_TO_MEM_FAILED on failure, or the number of bytes
 * written on success. */
#define TINFL_DECOMPRESS_MEM_TO_MEM_FAILED ((size_t)(-1))
MINIZ_EXPORT size_t tinfl_decompress_mem_to_mem(void *pOut_buf,
                                                size_t out_buf_len,
                                                const void *pSrc_buf,
                                                size_t src_buf_len, int flags);

/* tinfl_decompress_mem_to_callback() decompresses a block in memory to an
 * internal 32KB buffer, and a user provided callback function will be called to
 * flush the buffer. */
/* Returns 1 on success or 0 on failure. */
typedef int (*tinfl_put_buf_func_ptr)(const void *pBuf, int len, void *pUser);
MINIZ_EXPORT int
tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size,
                                 tinfl_put_buf_func_ptr pPut_buf_func,
                                 void *pPut_buf_user, int flags);

struct tinfl_decompressor_tag;
typedef struct tinfl_decompressor_tag tinfl_decompressor;

#ifndef MINIZ_NO_MALLOC
/* Allocate the tinfl_decompressor structure in C so that */
/* non-C language bindings to tinfl_ API don't need to worry about */
/* structure size and allocation mechanism. */
MINIZ_EXPORT tinfl_decompressor *tinfl_decompressor_alloc(void);
MINIZ_EXPORT void tinfl_decompressor_free(tinfl_decompressor *pDecomp);
#endif

/* Max size of LZ dictionary. */
#define TINFL_LZ_DICT_SIZE 32768

/* Return status. */
typedef enum {
  /* This flags indicates the inflator needs 1 or more input bytes to make
     forward progress, but the caller is indicating that no more are available.
     The compressed data */
  /* is probably corrupted. If you call the inflator again with more bytes it'll
     try to continue processing the input but this is a BAD sign (either the
     data is corrupted or you called it incorrectly). */
  /* If you call it again with no input you'll just get
     TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS again. */
  TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS = -4,

  /* This flag indicates that one or more of the input parameters was obviously
     bogus. (You can try calling it again, but if you get this error the calling
     code is wrong.) */
  TINFL_STATUS_BAD_PARAM = -3,

  /* This flags indicate the inflator is finished but the adler32 check of the
     uncompressed data didn't match. If you call it again it'll return
     TINFL_STATUS_DONE. */
  TINFL_STATUS_ADLER32_MISMATCH = -2,

  /* This flags indicate the inflator has somehow failed (bad code, corrupted
     input, etc.). If you call it again without resetting via tinfl_init() it
     it'll just keep on returning the same status failure code. */
  TINFL_STATUS_FAILED = -1,

  /* Any status code less than TINFL_STATUS_DONE must indicate a failure. */

  /* This flag indicates the inflator has returned every byte of uncompressed
     data that it can, has consumed every byte that it needed, has successfully
     reached the end of the deflate stream, and */
  /* if zlib headers and adler32 checking enabled that it has successfully
     checked the uncompressed data's adler32. If you call it again you'll just
     get TINFL_STATUS_DONE over and over again. */
  TINFL_STATUS_DONE = 0,

  /* This flag indicates the inflator MUST have more input data (even 1 byte)
     before it can make any more forward progress, or you need to clear the
     TINFL_FLAG_HAS_MORE_INPUT */
  /* flag on the next call if you don't have any more source data. If the source
     data was somehow corrupted it's also possible (but unlikely) for the
     inflator to keep on demanding input to */
  /* proceed, so be sure to properly set the TINFL_FLAG_HAS_MORE_INPUT flag. */
  TINFL_STATUS_NEEDS_MORE_INPUT = 1,

  /* This flag indicates the inflator definitely has 1 or more bytes of
     uncompressed data available, but it cannot write this data into the output
     buffer. */
  /* Note if the source compressed data was corrupted it's possible for the
     inflator to return a lot of uncompressed data to the caller. I've been
     assuming you know how much uncompressed data to expect */
  /* (either exact or worst case) and will stop calling the inflator and fail
     after receiving too much. In pure streaming scenarios where you have no
     idea how many bytes to expect this may not be possible */
  /* so I may need to add some code to address this. */
  TINFL_STATUS_HAS_MORE_OUTPUT = 2
} tinfl_status;

/* Initializes the decompressor to its initial state. */
#define tinfl_init(r)                                                          \
  do {                                                                         \
    (r)->m_state = 0;                                                          \
  }                                                                            \
  MZ_MACRO_END
#define tinfl_get_adler32(r) (r)->m_check_adler32

/* Main low-level decompressor coroutine function. This is the only function
 * actually needed for decompression. All the other functions are just
 * high-level helpers for improved usability. */
/* This is a universal API, i.e. it can be used as a building block to build any
 * desired higher level decompression API. In the limit case, it can be called
 * once per every byte input or output. */
MINIZ_EXPORT tinfl_status tinfl_decompress(
    tinfl_decompressor *r, const mz_uint8 *pIn_buf_next, size_t *pIn_buf_size,
    mz_uint8 *pOut_buf_start, mz_uint8 *pOut_buf_next, size_t *pOut_buf_size,
    const mz_uint32 decomp_flags);

/* Internal/private bits follow. */
enum {
  TINFL_MAX_HUFF_TABLES = 3,
  TINFL_MAX_HUFF_SYMBOLS_0 = 288,
  TINFL_MAX_HUFF_SYMBOLS_1 = 32,
  TINFL_MAX_HUFF_SYMBOLS_2 = 19,
  TINFL_FAST_LOOKUP_BITS = 10,
  TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
};

typedef struct {
  mz_uint8 m_code_size[TINFL_MAX_HUFF_SYMBOLS_0];
  mz_int16 m_look_up[TINFL_FAST_LOOKUP_SIZE],
      m_tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
} tinfl_huff_table;

#if MINIZ_HAS_64BIT_REGISTERS
#define TINFL_USE_64BIT_BITBUF 1
#else
#define TINFL_USE_64BIT_BITBUF 0
#endif

#if TINFL_USE_64BIT_BITBUF
typedef mz_uint64 tinfl_bit_buf_t;
#define TINFL_BITBUF_SIZE (64)
#else
typedef mz_uint32 tinfl_bit_buf_t;
#define TINFL_BITBUF_SIZE (32)
#endif

struct tinfl_decompressor_tag {
  mz_uint32 m_state, m_num_bits, m_zhdr0, m_zhdr1, m_z_adler32, m_final, m_type,
      m_check_adler32, m_dist, m_counter, m_num_extra,
      m_table_sizes[TINFL_MAX_HUFF_TABLES];
  tinfl_bit_buf_t m_bit_buf;
  size_t m_dist_from_out_buf_start;
  tinfl_huff_table m_tables[TINFL_MAX_HUFF_TABLES];
  mz_uint8 m_raw_header[4],
      m_len_codes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
};

#ifdef __cplusplus
}
#endif

#pragma once

/* ------------------- ZIP archive reading/writing */

#ifndef MINIZ_NO_ARCHIVE_APIS

#ifdef __cplusplus
extern "C" {
#endif

enum {
  /* Note: These enums can be reduced as needed to save memory or stack space -
     they are pretty conservative. */
  MZ_ZIP_MAX_IO_BUF_SIZE = 8 * 1024,
  MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE = 512,
  MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE = 512
};

typedef struct {
  /* Central directory file index. */
  mz_uint32 m_file_index;

  /* Byte offset of this entry in the archive's central directory. Note we
   * currently only support up to UINT_MAX or less bytes in the central dir. */
  mz_uint64 m_central_dir_ofs;

  /* These fields are copied directly from the zip's central dir. */
  mz_uint16 m_version_made_by;
  mz_uint16 m_version_needed;
  mz_uint16 m_bit_flag;
  mz_uint16 m_method;

#ifndef MINIZ_NO_TIME
  MZ_TIME_T m_time;
#endif

  /* CRC-32 of uncompressed data. */
  mz_uint32 m_crc32;

  /* File's compressed size. */
  mz_uint64 m_comp_size;

  /* File's uncompressed size. Note, I've seen some old archives where directory
   * entries had 512 bytes for their uncompressed sizes, but when you try to
   * unpack them you actually get 0 bytes. */
  mz_uint64 m_uncomp_size;

  /* Zip internal and external file attributes. */
  mz_uint16 m_internal_attr;
  mz_uint32 m_external_attr;

  /* Entry's local header file offset in bytes. */
  mz_uint64 m_local_header_ofs;

  /* Size of comment in bytes. */
  mz_uint32 m_comment_size;

  /* MZ_TRUE if the entry appears to be a directory. */
  mz_bool m_is_directory;

  /* MZ_TRUE if the entry uses encryption/strong encryption (which miniz_zip
   * doesn't support) */
  mz_bool m_is_encrypted;

  /* MZ_TRUE if the file is not encrypted, a patch file, and if it uses a
   * compression method we support. */
  mz_bool m_is_supported;

  /* Filename. If string ends in '/' it's a subdirectory entry. */
  /* Guaranteed to be zero terminated, may be truncated to fit. */
  char m_filename[MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE];

  /* Comment field. */
  /* Guaranteed to be zero terminated, may be truncated to fit. */
  char m_comment[MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE];

} mz_zip_archive_file_stat;

typedef size_t (*mz_file_read_func)(void *pOpaque, mz_uint64 file_ofs,
                                    void *pBuf, size_t n);
typedef size_t (*mz_file_write_func)(void *pOpaque, mz_uint64 file_ofs,
                                     const void *pBuf, size_t n);
typedef mz_bool (*mz_file_needs_keepalive)(void *pOpaque);

struct mz_zip_internal_state_tag;
typedef struct mz_zip_internal_state_tag mz_zip_internal_state;

typedef enum {
  MZ_ZIP_MODE_INVALID = 0,
  MZ_ZIP_MODE_READING = 1,
  MZ_ZIP_MODE_WRITING = 2,
  MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED = 3
} mz_zip_mode;

typedef enum {
  MZ_ZIP_FLAG_CASE_SENSITIVE = 0x0100,
  MZ_ZIP_FLAG_IGNORE_PATH = 0x0200,
  MZ_ZIP_FLAG_COMPRESSED_DATA = 0x0400,
  MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY = 0x0800,
  MZ_ZIP_FLAG_VALIDATE_LOCATE_FILE_FLAG =
      0x1000, /* if enabled, mz_zip_reader_locate_file() will be called on each
                 file as its validated to ensure the func finds the file in the
                 central dir (intended for testing) */
  MZ_ZIP_FLAG_VALIDATE_HEADERS_ONLY =
      0x2000, /* validate the local headers, but don't decompress the entire
                 file and check the crc32 */
  MZ_ZIP_FLAG_WRITE_ZIP64 =
      0x4000, /* always use the zip64 file format, instead of the original zip
                 file format with automatic switch to zip64. Use as flags
                 parameter with mz_zip_writer_init*_v2 */
  MZ_ZIP_FLAG_WRITE_ALLOW_READING = 0x8000,
  MZ_ZIP_FLAG_ASCII_FILENAME = 0x10000,
  /*After adding a compressed file, seek back
  to local file header and set the correct sizes*/
  MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE = 0x20000
} mz_zip_flags;

typedef enum {
  MZ_ZIP_TYPE_INVALID = 0,
  MZ_ZIP_TYPE_USER,
  MZ_ZIP_TYPE_MEMORY,
  MZ_ZIP_TYPE_HEAP,
  MZ_ZIP_TYPE_FILE,
  MZ_ZIP_TYPE_CFILE,
  MZ_ZIP_TOTAL_TYPES
} mz_zip_type;

/* miniz error codes. Be sure to update mz_zip_get_error_string() if you add or
 * modify this enum. */
typedef enum {
  MZ_ZIP_NO_ERROR = 0,
  MZ_ZIP_UNDEFINED_ERROR,
  MZ_ZIP_TOO_MANY_FILES,
  MZ_ZIP_FILE_TOO_LARGE,
  MZ_ZIP_UNSUPPORTED_METHOD,
  MZ_ZIP_UNSUPPORTED_ENCRYPTION,
  MZ_ZIP_UNSUPPORTED_FEATURE,
  MZ_ZIP_FAILED_FINDING_CENTRAL_DIR,
  MZ_ZIP_NOT_AN_ARCHIVE,
  MZ_ZIP_INVALID_HEADER_OR_CORRUPTED,
  MZ_ZIP_UNSUPPORTED_MULTIDISK,
  MZ_ZIP_DECOMPRESSION_FAILED,
  MZ_ZIP_COMPRESSION_FAILED,
  MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE,
  MZ_ZIP_CRC_CHECK_FAILED,
  MZ_ZIP_UNSUPPORTED_CDIR_SIZE,
  MZ_ZIP_ALLOC_FAILED,
  MZ_ZIP_FILE_OPEN_FAILED,
  MZ_ZIP_FILE_CREATE_FAILED,
  MZ_ZIP_FILE_WRITE_FAILED,
  MZ_ZIP_FILE_READ_FAILED,
  MZ_ZIP_FILE_CLOSE_FAILED,
  MZ_ZIP_FILE_SEEK_FAILED,
  MZ_ZIP_FILE_STAT_FAILED,
  MZ_ZIP_INVALID_PARAMETER,
  MZ_ZIP_INVALID_FILENAME,
  MZ_ZIP_BUF_TOO_SMALL,
  MZ_ZIP_INTERNAL_ERROR,
  MZ_ZIP_FILE_NOT_FOUND,
  MZ_ZIP_ARCHIVE_TOO_LARGE,
  MZ_ZIP_VALIDATION_FAILED,
  MZ_ZIP_WRITE_CALLBACK_FAILED,
  MZ_ZIP_TOTAL_ERRORS
} mz_zip_error;

typedef struct {
  mz_uint64 m_archive_size;
  mz_uint64 m_central_directory_file_ofs;

  /* We only support up to UINT32_MAX files in zip64 mode. */
  mz_uint32 m_total_files;
  mz_zip_mode m_zip_mode;
  mz_zip_type m_zip_type;
  mz_zip_error m_last_error;

  mz_uint64 m_file_offset_alignment;

  mz_alloc_func m_pAlloc;
  mz_free_func m_pFree;
  mz_realloc_func m_pRealloc;
  void *m_pAlloc_opaque;

  mz_file_read_func m_pRead;
  mz_file_write_func m_pWrite;
  mz_file_needs_keepalive m_pNeeds_keepalive;
  void *m_pIO_opaque;

  mz_zip_internal_state *m_pState;

} mz_zip_archive;

typedef struct {
  mz_zip_archive *pZip;
  mz_uint flags;

  int status;
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
  mz_uint file_crc32;
#endif
  mz_uint64 read_buf_size, read_buf_ofs, read_buf_avail, comp_remaining,
      out_buf_ofs, cur_file_ofs;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf;
  void *pWrite_buf;

  size_t out_blk_remain;

  tinfl_decompressor inflator;

} mz_zip_reader_extract_iter_state;

/* -------- ZIP reading */

/* Inits a ZIP archive reader. */
/* These functions read and validate the archive's central directory. */
MINIZ_EXPORT mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size,
                                        mz_uint flags);

MINIZ_EXPORT mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip,
                                            const void *pMem, size_t size,
                                            mz_uint flags);

#ifndef MINIZ_NO_STDIO
/* Read a archive from a disk file. */
/* file_start_ofs is the file offset where the archive actually begins, or 0. */
/* actual_archive_size is the true total size of the archive, which may be
 * smaller than the file's actual size on disk. If zero the entire file is
 * treated as the archive. */
MINIZ_EXPORT mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip,
                                             const char *pFilename,
                                             mz_uint32 flags);
MINIZ_EXPORT mz_bool mz_zip_reader_init_file_v2(mz_zip_archive *pZip,
                                                const char *pFilename,
                                                mz_uint flags,
                                                mz_uint64 file_start_ofs,
                                                mz_uint64 archive_size);
MINIZ_EXPORT mz_bool mz_zip_reader_init_file_v2_rpb(mz_zip_archive *pZip,
                                                    const char *pFilename,
                                                    mz_uint flags,
                                                    mz_uint64 file_start_ofs,
                                                    mz_uint64 archive_size);

/* Read an archive from an already opened FILE, beginning at the current file
 * position. */
/* The archive is assumed to be archive_size bytes long. If archive_size is 0,
 * then the entire rest of the file is assumed to contain the archive. */
/* The FILE will NOT be closed when mz_zip_reader_end() is called. */
MINIZ_EXPORT mz_bool mz_zip_reader_init_cfile(mz_zip_archive *pZip,
                                              MZ_FILE *pFile,
                                              mz_uint64 archive_size,
                                              mz_uint flags);
#endif

/* Ends archive reading, freeing all allocations, and closing the input archive
 * file if mz_zip_reader_init_file() was used. */
MINIZ_EXPORT mz_bool mz_zip_reader_end(mz_zip_archive *pZip);

/* -------- ZIP reading or writing */

/* Clears a mz_zip_archive struct to all zeros. */
/* Important: This must be done before passing the struct to any mz_zip
 * functions. */
MINIZ_EXPORT void mz_zip_zero_struct(mz_zip_archive *pZip);

MINIZ_EXPORT mz_zip_mode mz_zip_get_mode(mz_zip_archive *pZip);
MINIZ_EXPORT mz_zip_type mz_zip_get_type(mz_zip_archive *pZip);

/* Returns the total number of files in the archive. */
MINIZ_EXPORT mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip);

MINIZ_EXPORT mz_uint64 mz_zip_get_archive_size(mz_zip_archive *pZip);
MINIZ_EXPORT mz_uint64
mz_zip_get_archive_file_start_offset(mz_zip_archive *pZip);
MINIZ_EXPORT MZ_FILE *mz_zip_get_cfile(mz_zip_archive *pZip);

/* Reads n bytes of raw archive data, starting at file offset file_ofs, to pBuf.
 */
MINIZ_EXPORT size_t mz_zip_read_archive_data(mz_zip_archive *pZip,
                                             mz_uint64 file_ofs, void *pBuf,
                                             size_t n);

/* All mz_zip funcs set the m_last_error field in the mz_zip_archive struct.
 * These functions retrieve/manipulate this field. */
/* Note that the m_last_error functionality is not thread safe. */
MINIZ_EXPORT mz_zip_error mz_zip_set_last_error(mz_zip_archive *pZip,
                                                mz_zip_error err_num);
MINIZ_EXPORT mz_zip_error mz_zip_peek_last_error(mz_zip_archive *pZip);
MINIZ_EXPORT mz_zip_error mz_zip_clear_last_error(mz_zip_archive *pZip);
MINIZ_EXPORT mz_zip_error mz_zip_get_last_error(mz_zip_archive *pZip);
MINIZ_EXPORT const char *mz_zip_get_error_string(mz_zip_error mz_err);

/* MZ_TRUE if the archive file entry is a directory entry. */
MINIZ_EXPORT mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip,
                                                       mz_uint file_index);

/* MZ_TRUE if the file is encrypted/strong encrypted. */
MINIZ_EXPORT mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip,
                                                     mz_uint file_index);

/* MZ_TRUE if the compression method is supported, and the file is not
 * encrypted, and the file is not a compressed patch file. */
MINIZ_EXPORT mz_bool mz_zip_reader_is_file_supported(mz_zip_archive *pZip,
                                                     mz_uint file_index);

/* Retrieves the filename of an archive file entry. */
/* Returns the number of bytes written to pFilename, or if filename_buf_size is
 * 0 this function returns the number of bytes needed to fully store the
 * filename. */
MINIZ_EXPORT mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip,
                                                mz_uint file_index,
                                                char *pFilename,
                                                mz_uint filename_buf_size);

/* Attempts to locates a file in the archive's central directory. */
/* Valid flags: MZ_ZIP_FLAG_CASE_SENSITIVE, MZ_ZIP_FLAG_IGNORE_PATH */
/* Returns -1 if the file cannot be found. */
MINIZ_EXPORT int mz_zip_reader_locate_file(mz_zip_archive *pZip,
                                           const char *pName,
                                           const char *pComment, mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_reader_locate_file_v2(mz_zip_archive *pZip,
                                                  const char *pName,
                                                  const char *pComment,
                                                  mz_uint flags,
                                                  mz_uint32 *file_index);

/* Returns detailed information about an archive file entry. */
MINIZ_EXPORT mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip,
                                             mz_uint file_index,
                                             mz_zip_archive_file_stat *pStat);

/* MZ_TRUE if the file is in zip64 format. */
/* A file is considered zip64 if it contained a zip64 end of central directory
 * marker, or if it contained any zip64 extended file information fields in the
 * central directory. */
MINIZ_EXPORT mz_bool mz_zip_is_zip64(mz_zip_archive *pZip);

/* Returns the total central directory size in bytes. */
/* The current max supported size is <= MZ_UINT32_MAX. */
MINIZ_EXPORT size_t mz_zip_get_central_dir_size(mz_zip_archive *pZip);

/* Extracts a archive file to a memory buffer using no memory allocation. */
/* There must be at least enough room on the stack to store the inflator's state
 * (~34KB or so). */
MINIZ_EXPORT mz_bool mz_zip_reader_extract_to_mem_no_alloc(
    mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size,
    mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);
MINIZ_EXPORT mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(
    mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size,
    mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size);

/* Extracts a archive file to a memory buffer. */
MINIZ_EXPORT mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip,
                                                  mz_uint file_index,
                                                  void *pBuf, size_t buf_size,
                                                  mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip,
                                                       const char *pFilename,
                                                       void *pBuf,
                                                       size_t buf_size,
                                                       mz_uint flags);

/* Extracts a archive file to a dynamically allocated heap buffer. */
/* The memory will be allocated via the mz_zip_archive's alloc/realloc
 * functions. */
/* Returns NULL and sets the last error on failure. */
MINIZ_EXPORT void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip,
                                                 mz_uint file_index,
                                                 size_t *pSize, mz_uint flags);
MINIZ_EXPORT void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip,
                                                      const char *pFilename,
                                                      size_t *pSize,
                                                      mz_uint flags);

/* Extracts a archive file using a callback function to output the file's data.
 */
MINIZ_EXPORT mz_bool mz_zip_reader_extract_to_callback(
    mz_zip_archive *pZip, mz_uint file_index, mz_file_write_func pCallback,
    void *pOpaque, mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_reader_extract_file_to_callback(
    mz_zip_archive *pZip, const char *pFilename, mz_file_write_func pCallback,
    void *pOpaque, mz_uint flags);

/* Extract a file iteratively */
MINIZ_EXPORT mz_zip_reader_extract_iter_state *
mz_zip_reader_extract_iter_new(mz_zip_archive *pZip, mz_uint file_index,
                               mz_uint flags);
MINIZ_EXPORT mz_zip_reader_extract_iter_state *
mz_zip_reader_extract_file_iter_new(mz_zip_archive *pZip, const char *pFilename,
                                    mz_uint flags);
MINIZ_EXPORT size_t mz_zip_reader_extract_iter_read(
    mz_zip_reader_extract_iter_state *pState, void *pvBuf, size_t buf_size);
MINIZ_EXPORT mz_bool
mz_zip_reader_extract_iter_free(mz_zip_reader_extract_iter_state *pState);

#ifndef MINIZ_NO_STDIO
/* Extracts a archive file to a disk file and sets its last accessed and
 * modified times. */
/* This function only extracts files, not archive directory records. */
MINIZ_EXPORT mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip,
                                                   mz_uint file_index,
                                                   const char *pDst_filename,
                                                   mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_reader_extract_file_to_file(
    mz_zip_archive *pZip, const char *pArchive_filename,
    const char *pDst_filename, mz_uint flags);

/* Extracts a archive file starting at the current position in the destination
 * FILE stream. */
MINIZ_EXPORT mz_bool mz_zip_reader_extract_to_cfile(mz_zip_archive *pZip,
                                                    mz_uint file_index,
                                                    MZ_FILE *File,
                                                    mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_reader_extract_file_to_cfile(
    mz_zip_archive *pZip, const char *pArchive_filename, MZ_FILE *pFile,
    mz_uint flags);
#endif

#if 0
/* TODO */
	typedef void *mz_zip_streaming_extract_state_ptr;
	mz_zip_streaming_extract_state_ptr mz_zip_streaming_extract_begin(mz_zip_archive *pZip, mz_uint file_index, mz_uint flags);
	uint64_t mz_zip_streaming_extract_get_size(mz_zip_archive *pZip, mz_zip_streaming_extract_state_ptr pState);
	uint64_t mz_zip_streaming_extract_get_cur_ofs(mz_zip_archive *pZip, mz_zip_streaming_extract_state_ptr pState);
	mz_bool mz_zip_streaming_extract_seek(mz_zip_archive *pZip, mz_zip_streaming_extract_state_ptr pState, uint64_t new_ofs);
	size_t mz_zip_streaming_extract_read(mz_zip_archive *pZip, mz_zip_streaming_extract_state_ptr pState, void *pBuf, size_t buf_size);
	mz_bool mz_zip_streaming_extract_end(mz_zip_archive *pZip, mz_zip_streaming_extract_state_ptr pState);
#endif

/* This function compares the archive's local headers, the optional local zip64
 * extended information block, and the optional descriptor following the
 * compressed data vs. the data in the central directory. */
/* It also validates that each file can be successfully uncompressed unless the
 * MZ_ZIP_FLAG_VALIDATE_HEADERS_ONLY is specified. */
MINIZ_EXPORT mz_bool mz_zip_validate_file(mz_zip_archive *pZip,
                                          mz_uint file_index, mz_uint flags);

/* Validates an entire archive by calling mz_zip_validate_file() on each file.
 */
MINIZ_EXPORT mz_bool mz_zip_validate_archive(mz_zip_archive *pZip,
                                             mz_uint flags);

/* Misc utils/helpers, valid for ZIP reading or writing */
MINIZ_EXPORT mz_bool mz_zip_validate_mem_archive(const void *pMem, size_t size,
                                                 mz_uint flags,
                                                 mz_zip_error *pErr);
MINIZ_EXPORT mz_bool mz_zip_validate_file_archive(const char *pFilename,
                                                  mz_uint flags,
                                                  mz_zip_error *pErr);

/* Universal end function - calls either mz_zip_reader_end() or
 * mz_zip_writer_end(). */
MINIZ_EXPORT mz_bool mz_zip_end(mz_zip_archive *pZip);

/* -------- ZIP writing */

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

/* Inits a ZIP archive writer. */
/*Set pZip->m_pWrite (and pZip->m_pIO_opaque) before calling mz_zip_writer_init
 * or mz_zip_writer_init_v2*/
/*The output is streamable, i.e. file_ofs in mz_file_write_func always increases
 * only by n*/
MINIZ_EXPORT mz_bool mz_zip_writer_init(mz_zip_archive *pZip,
                                        mz_uint64 existing_size);
MINIZ_EXPORT mz_bool mz_zip_writer_init_v2(mz_zip_archive *pZip,
                                           mz_uint64 existing_size,
                                           mz_uint flags);

MINIZ_EXPORT mz_bool mz_zip_writer_init_heap(
    mz_zip_archive *pZip, size_t size_to_reserve_at_beginning,
    size_t initial_allocation_size);
MINIZ_EXPORT mz_bool mz_zip_writer_init_heap_v2(
    mz_zip_archive *pZip, size_t size_to_reserve_at_beginning,
    size_t initial_allocation_size, mz_uint flags);

#ifndef MINIZ_NO_STDIO
MINIZ_EXPORT mz_bool
mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename,
                        mz_uint64 size_to_reserve_at_beginning);
MINIZ_EXPORT mz_bool mz_zip_writer_init_file_v2(
    mz_zip_archive *pZip, const char *pFilename,
    mz_uint64 size_to_reserve_at_beginning, mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_writer_init_cfile(mz_zip_archive *pZip,
                                              MZ_FILE *pFile, mz_uint flags);
#endif

/* Converts a ZIP archive reader object into a writer object, to allow efficient
 * in-place file appends to occur on an existing archive. */
/* For archives opened using mz_zip_reader_init_file, pFilename must be the
 * archive's filename so it can be reopened for writing. If the file can't be
 * reopened, mz_zip_reader_end() will be called. */
/* For archives opened using mz_zip_reader_init_mem, the memory block must be
 * growable using the realloc callback (which defaults to realloc unless you've
 * overridden it). */
/* Finally, for archives opened using mz_zip_reader_init, the mz_zip_archive's
 * user provided m_pWrite function cannot be NULL. */
/* Note: In-place archive modification is not recommended unless you know what
 * you're doing, because if execution stops or something goes wrong before */
/* the archive is finalized the file's central directory will be hosed. */
MINIZ_EXPORT mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip,
                                                    const char *pFilename);
MINIZ_EXPORT mz_bool mz_zip_writer_init_from_reader_v2(mz_zip_archive *pZip,
                                                       const char *pFilename,
                                                       mz_uint flags);
MINIZ_EXPORT mz_bool mz_zip_writer_init_from_reader_v2_noreopen(
    mz_zip_archive *pZip, const char *pFilename, mz_uint flags);

/* Adds the contents of a memory buffer to an archive. These functions record
 * the current local time into the archive. */
/* To add a directory entry, call this method with an archive name ending in a
 * forwardslash with an empty buffer. */
/* level_and_flags - compression level (0-10, see MZ_BEST_SPEED,
 * MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or
 * just set to MZ_DEFAULT_COMPRESSION. */
MINIZ_EXPORT mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip,
                                           const char *pArchive_name,
                                           const void *pBuf, size_t buf_size,
                                           mz_uint level_and_flags);

/* Like mz_zip_writer_add_mem(), except you can specify a file comment field,
 * and optionally supply the function with already compressed data. */
/* uncomp_size/uncomp_crc32 are only used if the MZ_ZIP_FLAG_COMPRESSED_DATA
 * flag is specified. */
MINIZ_EXPORT mz_bool mz_zip_writer_add_mem_ex(
    mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32);

MINIZ_EXPORT mz_bool mz_zip_writer_add_mem_ex_v2(
    mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32,
    MZ_TIME_T *last_modified, const char *user_extra_data_local,
    mz_uint user_extra_data_local_len, const char *user_extra_data_central,
    mz_uint user_extra_data_central_len);

/* Adds the contents of a file to an archive. This function also records the
 * disk file's modified time into the archive. */
/* File data is supplied via a read callback function. User
 * mz_zip_writer_add_(c)file to add a file directly.*/
MINIZ_EXPORT mz_bool mz_zip_writer_add_read_buf_callback(
    mz_zip_archive *pZip, const char *pArchive_name,
    mz_file_read_func read_callback, void *callback_opaque, mz_uint64 max_size,
    const MZ_TIME_T *pFile_time, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_uint32 ext_attributes,
    const char *user_extra_data_local, mz_uint user_extra_data_local_len,
    const char *user_extra_data_central, mz_uint user_extra_data_central_len);

#ifndef MINIZ_NO_STDIO
/* Adds the contents of a disk file to an archive. This function also records
 * the disk file's modified time into the archive. */
/* level_and_flags - compression level (0-10, see MZ_BEST_SPEED,
 * MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or
 * just set to MZ_DEFAULT_COMPRESSION. */
MINIZ_EXPORT mz_bool mz_zip_writer_add_file(
    mz_zip_archive *pZip, const char *pArchive_name, const char *pSrc_filename,
    const void *pComment, mz_uint16 comment_size, mz_uint level_and_flags,
    mz_uint32 ext_attributes);

/* Like mz_zip_writer_add_file(), except the file data is read from the
 * specified FILE stream. */
MINIZ_EXPORT mz_bool mz_zip_writer_add_cfile(
    mz_zip_archive *pZip, const char *pArchive_name, MZ_FILE *pSrc_file,
    mz_uint64 max_size, const MZ_TIME_T *pFile_time, const void *pComment,
    mz_uint16 comment_size, mz_uint level_and_flags, mz_uint32 ext_attributes,
    const char *user_extra_data_local, mz_uint user_extra_data_local_len,
    const char *user_extra_data_central, mz_uint user_extra_data_central_len);
#endif

/* Adds a file to an archive by fully cloning the data from another archive. */
/* This function fully clones the source file's compressed data (no
 * recompression), along with its full filename, extra data (it may add or
 * modify the zip64 local header extra data field), and the optional descriptor
 * following the compressed data. */
MINIZ_EXPORT mz_bool mz_zip_writer_add_from_zip_reader(
    mz_zip_archive *pZip, mz_zip_archive *pSource_zip, mz_uint src_file_index);

/* Finalizes the archive by writing the central directory records followed by
 * the end of central directory record. */
/* After an archive is finalized, the only valid call on the mz_zip_archive
 * struct is mz_zip_writer_end(). */
/* An archive must be manually finalized by calling this function for it to be
 * valid. */
MINIZ_EXPORT mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip);

/* Finalizes a heap archive, returning a poiner to the heap block and its size.
 */
/* The heap block will be allocated using the mz_zip_archive's alloc/realloc
 * callbacks. */
MINIZ_EXPORT mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip,
                                                         void **ppBuf,
                                                         size_t *pSize);

/* Ends archive writing, freeing all allocations, and closing the output file if
 * mz_zip_writer_init_file() was used. */
/* Note for the archive to be valid, it *must* have been finalized before ending
 * (this function will not do it for you). */
MINIZ_EXPORT mz_bool mz_zip_writer_end(mz_zip_archive *pZip);

/* -------- Misc. high-level helper functions: */

/* mz_zip_add_mem_to_archive_file_in_place() efficiently (but not atomically)
 * appends a memory blob to a ZIP archive. */
/* Note this is NOT a fully safe operation. If it crashes or dies in some way
 * your archive can be left in a screwed up state (without a central directory).
 */
/* level_and_flags - compression level (0-10, see MZ_BEST_SPEED,
 * MZ_BEST_COMPRESSION, etc.) logically OR'd with zero or more mz_zip_flags, or
 * just set to MZ_DEFAULT_COMPRESSION. */
/* TODO: Perhaps add an option to leave the existing central dir in place in
 * case the add dies? We could then truncate the file (so the old central dir
 * would be at the end) if something goes wrong. */
MINIZ_EXPORT mz_bool mz_zip_add_mem_to_archive_file_in_place(
    const char *pZip_filename, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags);
MINIZ_EXPORT mz_bool mz_zip_add_mem_to_archive_file_in_place_v2(
    const char *pZip_filename, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_zip_error *pErr);

/* Reads a single file from an archive into a heap block. */
/* If pComment is not NULL, only the file with the specified comment will be
 * extracted. */
/* Returns NULL on failure. */
MINIZ_EXPORT void *
mz_zip_extract_archive_file_to_heap(const char *pZip_filename,
                                    const char *pArchive_name, size_t *pSize,
                                    mz_uint flags);
MINIZ_EXPORT void *mz_zip_extract_archive_file_to_heap_v2(
    const char *pZip_filename, const char *pArchive_name, const char *pComment,
    size_t *pSize, mz_uint flags, mz_zip_error *pErr);

#endif /* #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS */

#ifdef __cplusplus
}
#endif

#endif /* MINIZ_NO_ARCHIVE_APIS */
/**************************************************************************
 *
 * Copyright 2013-2014 RAD Game Tools and Valve Software
 * Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

typedef unsigned char mz_validate_uint16[sizeof(mz_uint16) == 2 ? 1 : -1];
typedef unsigned char mz_validate_uint32[sizeof(mz_uint32) == 4 ? 1 : -1];
typedef unsigned char mz_validate_uint64[sizeof(mz_uint64) == 8 ? 1 : -1];

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- zlib-style API's */

mz_ulong mz_adler32(mz_ulong adler, const unsigned char *ptr, size_t buf_len) {
  mz_uint32 i, s1 = (mz_uint32)(adler & 0xffff), s2 = (mz_uint32)(adler >> 16);
  size_t block_len = buf_len % 5552;
  if (!ptr)
    return MZ_ADLER32_INIT;
  while (buf_len) {
    for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
      s1 += ptr[0], s2 += s1;
      s1 += ptr[1], s2 += s1;
      s1 += ptr[2], s2 += s1;
      s1 += ptr[3], s2 += s1;
      s1 += ptr[4], s2 += s1;
      s1 += ptr[5], s2 += s1;
      s1 += ptr[6], s2 += s1;
      s1 += ptr[7], s2 += s1;
    }
    for (; i < block_len; ++i)
      s1 += *ptr++, s2 += s1;
    s1 %= 65521U, s2 %= 65521U;
    buf_len -= block_len;
    block_len = 5552;
  }
  return (s2 << 16) + s1;
}

/* Karl Malbrain's compact CRC-32. See "A compact CCITT crc16 and crc32 C
 * implementation that balances processor cache usage against speed":
 * http://www.geocities.com/malbrain/ */
#if 0
    mz_ulong mz_crc32(mz_ulong crc, const mz_uint8 *ptr, size_t buf_len)
    {
        static const mz_uint32 s_crc32[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
                                               0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
        mz_uint32 crcu32 = (mz_uint32)crc;
        if (!ptr)
            return MZ_CRC32_INIT;
        crcu32 = ~crcu32;
        while (buf_len--)
        {
            mz_uint8 b = *ptr++;
            crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b & 0xF)];
            crcu32 = (crcu32 >> 4) ^ s_crc32[(crcu32 & 0xF) ^ (b >> 4)];
        }
        return ~crcu32;
    }
#elif defined(USE_EXTERNAL_MZCRC)
/* If USE_EXTERNAL_CRC is defined, an external module will export the
 * mz_crc32() symbol for us to use, e.g. an SSE-accelerated version.
 * Depending on the impl, it may be necessary to ~ the input/output crc values.
 */
mz_ulong mz_crc32(mz_ulong crc, const mz_uint8 *ptr, size_t buf_len);
#else
/* Faster, but larger CPU cache footprint.
 */
mz_ulong mz_crc32(mz_ulong crc, const mz_uint8 *ptr, size_t buf_len) {
  static const mz_uint32 s_crc_table[256] = {
      0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F,
      0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
      0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2,
      0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
      0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9,
      0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
      0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C,
      0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
      0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423,
      0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
      0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106,
      0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
      0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D,
      0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
      0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950,
      0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
      0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7,
      0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
      0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA,
      0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
      0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81,
      0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
      0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84,
      0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
      0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB,
      0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
      0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E,
      0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
      0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55,
      0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
      0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28,
      0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
      0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F,
      0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
      0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242,
      0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
      0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69,
      0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
      0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC,
      0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
      0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693,
      0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
      0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D};

  mz_uint32 crc32 = (mz_uint32)crc ^ 0xFFFFFFFF;
  const mz_uint8 *pByte_buf = (const mz_uint8 *)ptr;

  while (buf_len >= 4) {
    crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[0]) & 0xFF];
    crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[1]) & 0xFF];
    crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[2]) & 0xFF];
    crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[3]) & 0xFF];
    pByte_buf += 4;
    buf_len -= 4;
  }

  while (buf_len) {
    crc32 = (crc32 >> 8) ^ s_crc_table[(crc32 ^ pByte_buf[0]) & 0xFF];
    ++pByte_buf;
    --buf_len;
  }

  return ~crc32;
}
#endif

void mz_free(void *p) { MZ_FREE(p); }

MINIZ_EXPORT void *miniz_def_alloc_func(void *opaque, size_t items,
                                        size_t size) {
  (void)opaque, (void)items, (void)size;
  return MZ_MALLOC(items * size);
}
MINIZ_EXPORT void miniz_def_free_func(void *opaque, void *address) {
  (void)opaque, (void)address;
  MZ_FREE(address);
}
MINIZ_EXPORT void *miniz_def_realloc_func(void *opaque, void *address,
                                          size_t items, size_t size) {
  (void)opaque, (void)address, (void)items, (void)size;
  return MZ_REALLOC(address, items * size);
}

const char *mz_version(void) { return MZ_VERSION; }

#ifndef MINIZ_NO_ZLIB_APIS

int mz_deflateInit(mz_streamp pStream, int level) {
  return mz_deflateInit2(pStream, level, MZ_DEFLATED, MZ_DEFAULT_WINDOW_BITS, 9,
                         MZ_DEFAULT_STRATEGY);
}

int mz_deflateInit2(mz_streamp pStream, int level, int method, int window_bits,
                    int mem_level, int strategy) {
  tdefl_compressor *pComp;
  mz_uint comp_flags =
      TDEFL_COMPUTE_ADLER32 |
      tdefl_create_comp_flags_from_zip_params(level, window_bits, strategy);

  if (!pStream)
    return MZ_STREAM_ERROR;
  if ((method != MZ_DEFLATED) || ((mem_level < 1) || (mem_level > 9)) ||
      ((window_bits != MZ_DEFAULT_WINDOW_BITS) &&
       (-window_bits != MZ_DEFAULT_WINDOW_BITS)))
    return MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = MZ_ADLER32_INIT;
  pStream->msg = NULL;
  pStream->reserved = 0;
  pStream->total_in = 0;
  pStream->total_out = 0;
  if (!pStream->zalloc)
    pStream->zalloc = miniz_def_alloc_func;
  if (!pStream->zfree)
    pStream->zfree = miniz_def_free_func;

  pComp = (tdefl_compressor *)pStream->zalloc(pStream->opaque, 1,
                                              sizeof(tdefl_compressor));
  if (!pComp)
    return MZ_MEM_ERROR;

  pStream->state = (struct mz_internal_state *)pComp;

  if (tdefl_init(pComp, NULL, NULL, comp_flags) != TDEFL_STATUS_OKAY) {
    mz_deflateEnd(pStream);
    return MZ_PARAM_ERROR;
  }

  return MZ_OK;
}

int mz_deflateReset(mz_streamp pStream) {
  if ((!pStream) || (!pStream->state) || (!pStream->zalloc) ||
      (!pStream->zfree))
    return MZ_STREAM_ERROR;
  pStream->total_in = pStream->total_out = 0;
  tdefl_init((tdefl_compressor *)pStream->state, NULL, NULL,
             ((tdefl_compressor *)pStream->state)->m_flags);
  return MZ_OK;
}

int mz_deflate(mz_streamp pStream, int flush) {
  size_t in_bytes, out_bytes;
  mz_ulong orig_total_in, orig_total_out;
  int mz_status = MZ_OK;

  if ((!pStream) || (!pStream->state) || (flush < 0) || (flush > MZ_FINISH) ||
      (!pStream->next_out))
    return MZ_STREAM_ERROR;
  if (!pStream->avail_out)
    return MZ_BUF_ERROR;

  if (flush == MZ_PARTIAL_FLUSH)
    flush = MZ_SYNC_FLUSH;

  if (((tdefl_compressor *)pStream->state)->m_prev_return_status ==
      TDEFL_STATUS_DONE)
    return (flush == MZ_FINISH) ? MZ_STREAM_END : MZ_BUF_ERROR;

  orig_total_in = pStream->total_in;
  orig_total_out = pStream->total_out;
  for (;;) {
    tdefl_status defl_status;
    in_bytes = pStream->avail_in;
    out_bytes = pStream->avail_out;

    defl_status = tdefl_compress((tdefl_compressor *)pStream->state,
                                 pStream->next_in, &in_bytes, pStream->next_out,
                                 &out_bytes, (tdefl_flush)flush);
    pStream->next_in += (mz_uint)in_bytes;
    pStream->avail_in -= (mz_uint)in_bytes;
    pStream->total_in += (mz_uint)in_bytes;
    pStream->adler = tdefl_get_adler32((tdefl_compressor *)pStream->state);

    pStream->next_out += (mz_uint)out_bytes;
    pStream->avail_out -= (mz_uint)out_bytes;
    pStream->total_out += (mz_uint)out_bytes;

    if (defl_status < 0) {
      mz_status = MZ_STREAM_ERROR;
      break;
    } else if (defl_status == TDEFL_STATUS_DONE) {
      mz_status = MZ_STREAM_END;
      break;
    } else if (!pStream->avail_out)
      break;
    else if ((!pStream->avail_in) && (flush != MZ_FINISH)) {
      if ((flush) || (pStream->total_in != orig_total_in) ||
          (pStream->total_out != orig_total_out))
        break;
      return MZ_BUF_ERROR; /* Can't make forward progress without some input.
                            */
    }
  }
  return mz_status;
}

int mz_deflateEnd(mz_streamp pStream) {
  if (!pStream)
    return MZ_STREAM_ERROR;
  if (pStream->state) {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return MZ_OK;
}

mz_ulong mz_deflateBound(mz_streamp pStream, mz_ulong source_len) {
  (void)pStream;
  /* This is really over conservative. (And lame, but it's actually pretty
   * tricky to compute a true upper bound given the way tdefl's blocking works.)
   */
  return MZ_MAX(128 + (source_len * 110) / 100,
                128 + source_len + ((source_len / (31 * 1024)) + 1) * 5);
}

int mz_compress2(unsigned char *pDest, mz_ulong *pDest_len,
                 const unsigned char *pSource, mz_ulong source_len, int level) {
  int status;
  mz_stream stream;
  memset(&stream, 0, sizeof(stream));

  /* In case mz_ulong is 64-bits (argh I hate longs). */
  if ((source_len | *pDest_len) > 0xFFFFFFFFU)
    return MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mz_uint32)source_len;
  stream.next_out = pDest;
  stream.avail_out = (mz_uint32)*pDest_len;

  status = mz_deflateInit(&stream, level);
  if (status != MZ_OK)
    return status;

  status = mz_deflate(&stream, MZ_FINISH);
  if (status != MZ_STREAM_END) {
    mz_deflateEnd(&stream);
    return (status == MZ_OK) ? MZ_BUF_ERROR : status;
  }

  *pDest_len = stream.total_out;
  return mz_deflateEnd(&stream);
}

int mz_compress(unsigned char *pDest, mz_ulong *pDest_len,
                const unsigned char *pSource, mz_ulong source_len) {
  return mz_compress2(pDest, pDest_len, pSource, source_len,
                      MZ_DEFAULT_COMPRESSION);
}

mz_ulong mz_compressBound(mz_ulong source_len) {
  return mz_deflateBound(NULL, source_len);
}

typedef struct {
  tinfl_decompressor m_decomp;
  mz_uint m_dict_ofs, m_dict_avail, m_first_call, m_has_flushed;
  int m_window_bits;
  mz_uint8 m_dict[TINFL_LZ_DICT_SIZE];
  tinfl_status m_last_status;
} inflate_state;

int mz_inflateInit2(mz_streamp pStream, int window_bits) {
  inflate_state *pDecomp;
  if (!pStream)
    return MZ_STREAM_ERROR;
  if ((window_bits != MZ_DEFAULT_WINDOW_BITS) &&
      (-window_bits != MZ_DEFAULT_WINDOW_BITS))
    return MZ_PARAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = 0;
  pStream->msg = NULL;
  pStream->total_in = 0;
  pStream->total_out = 0;
  pStream->reserved = 0;
  if (!pStream->zalloc)
    pStream->zalloc = miniz_def_alloc_func;
  if (!pStream->zfree)
    pStream->zfree = miniz_def_free_func;

  pDecomp = (inflate_state *)pStream->zalloc(pStream->opaque, 1,
                                             sizeof(inflate_state));
  if (!pDecomp)
    return MZ_MEM_ERROR;

  pStream->state = (struct mz_internal_state *)pDecomp;

  tinfl_init(&pDecomp->m_decomp);
  pDecomp->m_dict_ofs = 0;
  pDecomp->m_dict_avail = 0;
  pDecomp->m_last_status = TINFL_STATUS_NEEDS_MORE_INPUT;
  pDecomp->m_first_call = 1;
  pDecomp->m_has_flushed = 0;
  pDecomp->m_window_bits = window_bits;

  return MZ_OK;
}

int mz_inflateInit(mz_streamp pStream) {
  return mz_inflateInit2(pStream, MZ_DEFAULT_WINDOW_BITS);
}

int mz_inflateReset(mz_streamp pStream) {
  inflate_state *pDecomp;
  if (!pStream)
    return MZ_STREAM_ERROR;

  pStream->data_type = 0;
  pStream->adler = 0;
  pStream->msg = NULL;
  pStream->total_in = 0;
  pStream->total_out = 0;
  pStream->reserved = 0;

  pDecomp = (inflate_state *)pStream->state;

  tinfl_init(&pDecomp->m_decomp);
  pDecomp->m_dict_ofs = 0;
  pDecomp->m_dict_avail = 0;
  pDecomp->m_last_status = TINFL_STATUS_NEEDS_MORE_INPUT;
  pDecomp->m_first_call = 1;
  pDecomp->m_has_flushed = 0;
  /* pDecomp->m_window_bits = window_bits */;

  return MZ_OK;
}

int mz_inflate(mz_streamp pStream, int flush) {
  inflate_state *pState;
  mz_uint n, first_call, decomp_flags = TINFL_FLAG_COMPUTE_ADLER32;
  size_t in_bytes, out_bytes, orig_avail_in;
  tinfl_status status;

  if ((!pStream) || (!pStream->state))
    return MZ_STREAM_ERROR;
  if (flush == MZ_PARTIAL_FLUSH)
    flush = MZ_SYNC_FLUSH;
  if ((flush) && (flush != MZ_SYNC_FLUSH) && (flush != MZ_FINISH))
    return MZ_STREAM_ERROR;

  pState = (inflate_state *)pStream->state;
  if (pState->m_window_bits > 0)
    decomp_flags |= TINFL_FLAG_PARSE_ZLIB_HEADER;
  orig_avail_in = pStream->avail_in;

  first_call = pState->m_first_call;
  pState->m_first_call = 0;
  if (pState->m_last_status < 0)
    return MZ_DATA_ERROR;

  if (pState->m_has_flushed && (flush != MZ_FINISH))
    return MZ_STREAM_ERROR;
  pState->m_has_flushed |= (flush == MZ_FINISH);

  if ((flush == MZ_FINISH) && (first_call)) {
    /* MZ_FINISH on the first call implies that the input and output buffers are
     * large enough to hold the entire compressed/decompressed file. */
    decomp_flags |= TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF;
    in_bytes = pStream->avail_in;
    out_bytes = pStream->avail_out;
    status = tinfl_decompress(&pState->m_decomp, pStream->next_in, &in_bytes,
                              pStream->next_out, pStream->next_out, &out_bytes,
                              decomp_flags);
    pState->m_last_status = status;
    pStream->next_in += (mz_uint)in_bytes;
    pStream->avail_in -= (mz_uint)in_bytes;
    pStream->total_in += (mz_uint)in_bytes;
    pStream->adler = tinfl_get_adler32(&pState->m_decomp);
    pStream->next_out += (mz_uint)out_bytes;
    pStream->avail_out -= (mz_uint)out_bytes;
    pStream->total_out += (mz_uint)out_bytes;

    if (status < 0)
      return MZ_DATA_ERROR;
    else if (status != TINFL_STATUS_DONE) {
      pState->m_last_status = TINFL_STATUS_FAILED;
      return MZ_BUF_ERROR;
    }
    return MZ_STREAM_END;
  }
  /* flush != MZ_FINISH then we must assume there's more input. */
  if (flush != MZ_FINISH)
    decomp_flags |= TINFL_FLAG_HAS_MORE_INPUT;

  if (pState->m_dict_avail) {
    n = MZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n;
    pStream->avail_out -= n;
    pStream->total_out += n;
    pState->m_dict_avail -= n;
    pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);
    return ((pState->m_last_status == TINFL_STATUS_DONE) &&
            (!pState->m_dict_avail))
               ? MZ_STREAM_END
               : MZ_OK;
  }

  for (;;) {
    in_bytes = pStream->avail_in;
    out_bytes = TINFL_LZ_DICT_SIZE - pState->m_dict_ofs;

    status = tinfl_decompress(
        &pState->m_decomp, pStream->next_in, &in_bytes, pState->m_dict,
        pState->m_dict + pState->m_dict_ofs, &out_bytes, decomp_flags);
    pState->m_last_status = status;

    pStream->next_in += (mz_uint)in_bytes;
    pStream->avail_in -= (mz_uint)in_bytes;
    pStream->total_in += (mz_uint)in_bytes;
    pStream->adler = tinfl_get_adler32(&pState->m_decomp);

    pState->m_dict_avail = (mz_uint)out_bytes;

    n = MZ_MIN(pState->m_dict_avail, pStream->avail_out);
    memcpy(pStream->next_out, pState->m_dict + pState->m_dict_ofs, n);
    pStream->next_out += n;
    pStream->avail_out -= n;
    pStream->total_out += n;
    pState->m_dict_avail -= n;
    pState->m_dict_ofs = (pState->m_dict_ofs + n) & (TINFL_LZ_DICT_SIZE - 1);

    if (status < 0)
      return MZ_DATA_ERROR; /* Stream is corrupted (there could be some
                               uncompressed data left in the output dictionary -
                               oh well). */
    else if ((status == TINFL_STATUS_NEEDS_MORE_INPUT) && (!orig_avail_in))
      return MZ_BUF_ERROR; /* Signal caller that we can't make forward progress
                              without supplying more input or by setting flush
                              to MZ_FINISH. */
    else if (flush == MZ_FINISH) {
      /* The output buffer MUST be large to hold the remaining uncompressed data
       * when flush==MZ_FINISH. */
      if (status == TINFL_STATUS_DONE)
        return pState->m_dict_avail ? MZ_BUF_ERROR : MZ_STREAM_END;
      /* status here must be TINFL_STATUS_HAS_MORE_OUTPUT, which means there's
       * at least 1 more byte on the way. If there's no more room left in the
       * output buffer then something is wrong. */
      else if (!pStream->avail_out)
        return MZ_BUF_ERROR;
    } else if ((status == TINFL_STATUS_DONE) || (!pStream->avail_in) ||
               (!pStream->avail_out) || (pState->m_dict_avail))
      break;
  }

  return ((status == TINFL_STATUS_DONE) && (!pState->m_dict_avail))
             ? MZ_STREAM_END
             : MZ_OK;
}

int mz_inflateEnd(mz_streamp pStream) {
  if (!pStream)
    return MZ_STREAM_ERROR;
  if (pStream->state) {
    pStream->zfree(pStream->opaque, pStream->state);
    pStream->state = NULL;
  }
  return MZ_OK;
}
int mz_uncompress2(unsigned char *pDest, mz_ulong *pDest_len,
                   const unsigned char *pSource, mz_ulong *pSource_len) {
  mz_stream stream;
  int status;
  memset(&stream, 0, sizeof(stream));

  /* In case mz_ulong is 64-bits (argh I hate longs). */
  if ((*pSource_len | *pDest_len) > 0xFFFFFFFFU)
    return MZ_PARAM_ERROR;

  stream.next_in = pSource;
  stream.avail_in = (mz_uint32)*pSource_len;
  stream.next_out = pDest;
  stream.avail_out = (mz_uint32)*pDest_len;

  status = mz_inflateInit(&stream);
  if (status != MZ_OK)
    return status;

  status = mz_inflate(&stream, MZ_FINISH);
  *pSource_len = *pSource_len - stream.avail_in;
  if (status != MZ_STREAM_END) {
    mz_inflateEnd(&stream);
    return ((status == MZ_BUF_ERROR) && (!stream.avail_in)) ? MZ_DATA_ERROR
                                                            : status;
  }
  *pDest_len = stream.total_out;

  return mz_inflateEnd(&stream);
}

int mz_uncompress(unsigned char *pDest, mz_ulong *pDest_len,
                  const unsigned char *pSource, mz_ulong source_len) {
  return mz_uncompress2(pDest, pDest_len, pSource, &source_len);
}

const char *mz_error(int err) {
  static struct {
    int m_err;
    const char *m_pDesc;
  } s_error_descs[] = {{MZ_OK, ""},
                       {MZ_STREAM_END, "stream end"},
                       {MZ_NEED_DICT, "need dictionary"},
                       {MZ_ERRNO, "file error"},
                       {MZ_STREAM_ERROR, "stream error"},
                       {MZ_DATA_ERROR, "data error"},
                       {MZ_MEM_ERROR, "out of memory"},
                       {MZ_BUF_ERROR, "buf error"},
                       {MZ_VERSION_ERROR, "version error"},
                       {MZ_PARAM_ERROR, "parameter error"}};
  mz_uint i;
  for (i = 0; i < sizeof(s_error_descs) / sizeof(s_error_descs[0]); ++i)
    if (s_error_descs[i].m_err == err)
      return s_error_descs[i].m_pDesc;
  return NULL;
}

#endif /*MINIZ_NO_ZLIB_APIS */

#ifdef __cplusplus
}
#endif

/*
  This is free and unencumbered software released into the public domain.

  Anyone is free to copy, modify, publish, use, compile, sell, or
  distribute this software, either in source code form or as a compiled
  binary, for any purpose, commercial or non-commercial, and by any
  means.

  In jurisdictions that recognize copyright laws, the author or authors
  of this software dedicate any and all copyright interest in the
  software to the public domain. We make this dedication for the benefit
  of the public at large and to the detriment of our heirs and
  successors. We intend this dedication to be an overt act of
  relinquishment in perpetuity of all present and future rights to this
  software under copyright law.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
  OTHER DEALINGS IN THE SOFTWARE.

  For more information, please refer to <http://unlicense.org/>
*/
/**************************************************************************
 *
 * Copyright 2013-2014 RAD Game Tools and Valve Software
 * Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- Low-level Compression (independent from all decompression
 * API's) */

/* Purposely making these tables static for faster init and thread safety. */
static const mz_uint16 s_tdefl_len_sym[256] = {
    257, 258, 259, 260, 261, 262, 263, 264, 265, 265, 266, 266, 267, 267, 268,
    268, 269, 269, 269, 269, 270, 270, 270, 270, 271, 271, 271, 271, 272, 272,
    272, 272, 273, 273, 273, 273, 273, 273, 273, 273, 274, 274, 274, 274, 274,
    274, 274, 274, 275, 275, 275, 275, 275, 275, 275, 275, 276, 276, 276, 276,
    276, 276, 276, 276, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277, 277,
    277, 277, 277, 277, 277, 278, 278, 278, 278, 278, 278, 278, 278, 278, 278,
    278, 278, 278, 278, 278, 278, 279, 279, 279, 279, 279, 279, 279, 279, 279,
    279, 279, 279, 279, 279, 279, 279, 280, 280, 280, 280, 280, 280, 280, 280,
    280, 280, 280, 280, 280, 280, 280, 280, 281, 281, 281, 281, 281, 281, 281,
    281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 281,
    281, 281, 281, 281, 281, 281, 281, 281, 281, 281, 282, 282, 282, 282, 282,
    282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282,
    282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 282, 283, 283, 283,
    283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283,
    283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 283, 284,
    284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284,
    284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284, 284,
    285};

static const mz_uint8 s_tdefl_len_extra[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0};

static const mz_uint8 s_tdefl_small_dist_sym[512] = {
    0,  1,  2,  3,  4,  4,  5,  5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,
    8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17,
    17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17};

static const mz_uint8 s_tdefl_small_dist_extra[512] = {
    0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

static const mz_uint8 s_tdefl_large_dist_sym[128] = {
    0,  0,  18, 19, 20, 20, 21, 21, 22, 22, 22, 22, 23, 23, 23, 23, 24, 24, 24,
    24, 24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26, 26,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29};

static const mz_uint8 s_tdefl_large_dist_extra[128] = {
    0,  0,  8,  8,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13};

/* Radix sorts tdefl_sym_freq[] array by 16-bit key m_key. Returns ptr to sorted
 * values. */
typedef struct {
  mz_uint16 m_key, m_sym_index;
} tdefl_sym_freq;
static tdefl_sym_freq *tdefl_radix_sort_syms(mz_uint num_syms,
                                             tdefl_sym_freq *pSyms0,
                                             tdefl_sym_freq *pSyms1) {
  mz_uint32 total_passes = 2, pass_shift, pass, i, hist[256 * 2];
  tdefl_sym_freq *pCur_syms = pSyms0, *pNew_syms = pSyms1;
  MZ_CLEAR_OBJ(hist);
  for (i = 0; i < num_syms; i++) {
    mz_uint freq = pSyms0[i].m_key;
    hist[freq & 0xFF]++;
    hist[256 + ((freq >> 8) & 0xFF)]++;
  }
  while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256]))
    total_passes--;
  for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8) {
    const mz_uint32 *pHist = &hist[pass << 8];
    mz_uint offsets[256], cur_ofs = 0;
    for (i = 0; i < 256; i++) {
      offsets[i] = cur_ofs;
      cur_ofs += pHist[i];
    }
    for (i = 0; i < num_syms; i++)
      pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] =
          pCur_syms[i];
    {
      tdefl_sym_freq *t = pCur_syms;
      pCur_syms = pNew_syms;
      pNew_syms = t;
    }
  }
  return pCur_syms;
}

/* tdefl_calculate_minimum_redundancy() originally written by: Alistair Moffat,
 * alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996. */
static void tdefl_calculate_minimum_redundancy(tdefl_sym_freq *A, int n) {
  int root, leaf, next, avbl, used, dpth;
  if (n == 0)
    return;
  else if (n == 1) {
    A[0].m_key = 1;
    return;
  }
  A[0].m_key += A[1].m_key;
  root = 0;
  leaf = 2;
  for (next = 1; next < n - 1; next++) {
    if (leaf >= n || A[root].m_key < A[leaf].m_key) {
      A[next].m_key = A[root].m_key;
      A[root++].m_key = (mz_uint16)next;
    } else
      A[next].m_key = A[leaf++].m_key;
    if (leaf >= n || (root < next && A[root].m_key < A[leaf].m_key)) {
      A[next].m_key = (mz_uint16)(A[next].m_key + A[root].m_key);
      A[root++].m_key = (mz_uint16)next;
    } else
      A[next].m_key = (mz_uint16)(A[next].m_key + A[leaf++].m_key);
  }
  A[n - 2].m_key = 0;
  for (next = n - 3; next >= 0; next--)
    A[next].m_key = A[A[next].m_key].m_key + 1;
  avbl = 1;
  used = dpth = 0;
  root = n - 2;
  next = n - 1;
  while (avbl > 0) {
    while (root >= 0 && (int)A[root].m_key == dpth) {
      used++;
      root--;
    }
    while (avbl > used) {
      A[next--].m_key = (mz_uint16)(dpth);
      avbl--;
    }
    avbl = 2 * used;
    dpth++;
    used = 0;
  }
}

/* Limits canonical Huffman code table's max code size. */
enum { TDEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32 };
static void tdefl_huffman_enforce_max_code_size(int *pNum_codes,
                                                int code_list_len,
                                                int max_code_size) {
  int i;
  mz_uint32 total = 0;
  if (code_list_len <= 1)
    return;
  for (i = max_code_size + 1; i <= TDEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++)
    pNum_codes[max_code_size] += pNum_codes[i];
  for (i = max_code_size; i > 0; i--)
    total += (((mz_uint32)pNum_codes[i]) << (max_code_size - i));
  while (total != (1UL << max_code_size)) {
    pNum_codes[max_code_size]--;
    for (i = max_code_size - 1; i > 0; i--)
      if (pNum_codes[i]) {
        pNum_codes[i]--;
        pNum_codes[i + 1] += 2;
        break;
      }
    total--;
  }
}

static void tdefl_optimize_huffman_table(tdefl_compressor *d, int table_num,
                                         int table_len, int code_size_limit,
                                         int static_table) {
  int i, j, l, num_codes[1 + TDEFL_MAX_SUPPORTED_HUFF_CODESIZE];
  mz_uint next_code[TDEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1];
  MZ_CLEAR_OBJ(num_codes);
  if (static_table) {
    for (i = 0; i < table_len; i++)
      num_codes[d->m_huff_code_sizes[table_num][i]]++;
  } else {
    tdefl_sym_freq syms0[TDEFL_MAX_HUFF_SYMBOLS], syms1[TDEFL_MAX_HUFF_SYMBOLS],
        *pSyms;
    int num_used_syms = 0;
    const mz_uint16 *pSym_count = &d->m_huff_count[table_num][0];
    for (i = 0; i < table_len; i++)
      if (pSym_count[i]) {
        syms0[num_used_syms].m_key = (mz_uint16)pSym_count[i];
        syms0[num_used_syms++].m_sym_index = (mz_uint16)i;
      }

    pSyms = tdefl_radix_sort_syms(num_used_syms, syms0, syms1);
    tdefl_calculate_minimum_redundancy(pSyms, num_used_syms);

    for (i = 0; i < num_used_syms; i++)
      num_codes[pSyms[i].m_key]++;

    tdefl_huffman_enforce_max_code_size(num_codes, num_used_syms,
                                        code_size_limit);

    MZ_CLEAR_OBJ(d->m_huff_code_sizes[table_num]);
    MZ_CLEAR_OBJ(d->m_huff_codes[table_num]);
    for (i = 1, j = num_used_syms; i <= code_size_limit; i++)
      for (l = num_codes[i]; l > 0; l--)
        d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (mz_uint8)(i);
  }

  next_code[1] = 0;
  for (j = 0, i = 2; i <= code_size_limit; i++)
    next_code[i] = j = ((j + num_codes[i - 1]) << 1);

  for (i = 0; i < table_len; i++) {
    mz_uint rev_code = 0, code, code_size;
    if ((code_size = d->m_huff_code_sizes[table_num][i]) == 0)
      continue;
    code = next_code[code_size]++;
    for (l = code_size; l > 0; l--, code >>= 1)
      rev_code = (rev_code << 1) | (code & 1);
    d->m_huff_codes[table_num][i] = (mz_uint16)rev_code;
  }
}

#define TDEFL_PUT_BITS(b, l)                                                   \
  do {                                                                         \
    mz_uint bits = b;                                                          \
    mz_uint len = l;                                                           \
    MZ_ASSERT(bits <= ((1U << len) - 1U));                                     \
    d->m_bit_buffer |= (bits << d->m_bits_in);                                 \
    d->m_bits_in += len;                                                       \
    while (d->m_bits_in >= 8) {                                                \
      if (d->m_pOutput_buf < d->m_pOutput_buf_end)                             \
        *d->m_pOutput_buf++ = (mz_uint8)(d->m_bit_buffer);                     \
      d->m_bit_buffer >>= 8;                                                   \
      d->m_bits_in -= 8;                                                       \
    }                                                                          \
  }                                                                            \
  MZ_MACRO_END

#define TDEFL_RLE_PREV_CODE_SIZE()                                             \
  {                                                                            \
    if (rle_repeat_count) {                                                    \
      if (rle_repeat_count < 3) {                                              \
        d->m_huff_count[2][prev_code_size] =                                   \
            (mz_uint16)(d->m_huff_count[2][prev_code_size] +                   \
                        rle_repeat_count);                                     \
        while (rle_repeat_count--)                                             \
          packed_code_sizes[num_packed_code_sizes++] = prev_code_size;         \
      } else {                                                                 \
        d->m_huff_count[2][16] = (mz_uint16)(d->m_huff_count[2][16] + 1);      \
        packed_code_sizes[num_packed_code_sizes++] = 16;                       \
        packed_code_sizes[num_packed_code_sizes++] =                           \
            (mz_uint8)(rle_repeat_count - 3);                                  \
      }                                                                        \
      rle_repeat_count = 0;                                                    \
    }                                                                          \
  }

#define TDEFL_RLE_ZERO_CODE_SIZE()                                             \
  {                                                                            \
    if (rle_z_count) {                                                         \
      if (rle_z_count < 3) {                                                   \
        d->m_huff_count[2][0] =                                                \
            (mz_uint16)(d->m_huff_count[2][0] + rle_z_count);                  \
        while (rle_z_count--)                                                  \
          packed_code_sizes[num_packed_code_sizes++] = 0;                      \
      } else if (rle_z_count <= 10) {                                          \
        d->m_huff_count[2][17] = (mz_uint16)(d->m_huff_count[2][17] + 1);      \
        packed_code_sizes[num_packed_code_sizes++] = 17;                       \
        packed_code_sizes[num_packed_code_sizes++] =                           \
            (mz_uint8)(rle_z_count - 3);                                       \
      } else {                                                                 \
        d->m_huff_count[2][18] = (mz_uint16)(d->m_huff_count[2][18] + 1);      \
        packed_code_sizes[num_packed_code_sizes++] = 18;                       \
        packed_code_sizes[num_packed_code_sizes++] =                           \
            (mz_uint8)(rle_z_count - 11);                                      \
      }                                                                        \
      rle_z_count = 0;                                                         \
    }                                                                          \
  }

static mz_uint8 s_tdefl_packed_code_size_syms_swizzle[] = {
    16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static void tdefl_start_dynamic_block(tdefl_compressor *d) {
  int num_lit_codes, num_dist_codes, num_bit_lengths;
  mz_uint i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count,
      rle_repeat_count, packed_code_sizes_index;
  mz_uint8
      code_sizes_to_pack[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1],
      packed_code_sizes[TDEFL_MAX_HUFF_SYMBOLS_0 + TDEFL_MAX_HUFF_SYMBOLS_1],
      prev_code_size = 0xFF;

  d->m_huff_count[0][256] = 1;

  tdefl_optimize_huffman_table(d, 0, TDEFL_MAX_HUFF_SYMBOLS_0, 15, MZ_FALSE);
  tdefl_optimize_huffman_table(d, 1, TDEFL_MAX_HUFF_SYMBOLS_1, 15, MZ_FALSE);

  for (num_lit_codes = 286; num_lit_codes > 257; num_lit_codes--)
    if (d->m_huff_code_sizes[0][num_lit_codes - 1])
      break;
  for (num_dist_codes = 30; num_dist_codes > 1; num_dist_codes--)
    if (d->m_huff_code_sizes[1][num_dist_codes - 1])
      break;

  memcpy(code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes);
  memcpy(code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0],
         num_dist_codes);
  total_code_sizes_to_pack = num_lit_codes + num_dist_codes;
  num_packed_code_sizes = 0;
  rle_z_count = 0;
  rle_repeat_count = 0;

  memset(&d->m_huff_count[2][0], 0,
         sizeof(d->m_huff_count[2][0]) * TDEFL_MAX_HUFF_SYMBOLS_2);
  for (i = 0; i < total_code_sizes_to_pack; i++) {
    mz_uint8 code_size = code_sizes_to_pack[i];
    if (!code_size) {
      TDEFL_RLE_PREV_CODE_SIZE();
      if (++rle_z_count == 138) {
        TDEFL_RLE_ZERO_CODE_SIZE();
      }
    } else {
      TDEFL_RLE_ZERO_CODE_SIZE();
      if (code_size != prev_code_size) {
        TDEFL_RLE_PREV_CODE_SIZE();
        d->m_huff_count[2][code_size] =
            (mz_uint16)(d->m_huff_count[2][code_size] + 1);
        packed_code_sizes[num_packed_code_sizes++] = code_size;
      } else if (++rle_repeat_count == 6) {
        TDEFL_RLE_PREV_CODE_SIZE();
      }
    }
    prev_code_size = code_size;
  }
  if (rle_repeat_count) {
    TDEFL_RLE_PREV_CODE_SIZE();
  } else {
    TDEFL_RLE_ZERO_CODE_SIZE();
  }

  tdefl_optimize_huffman_table(d, 2, TDEFL_MAX_HUFF_SYMBOLS_2, 7, MZ_FALSE);

  TDEFL_PUT_BITS(2, 2);

  TDEFL_PUT_BITS(num_lit_codes - 257, 5);
  TDEFL_PUT_BITS(num_dist_codes - 1, 5);

  for (num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths--)
    if (d->m_huff_code_sizes
            [2][s_tdefl_packed_code_size_syms_swizzle[num_bit_lengths]])
      break;
  num_bit_lengths = MZ_MAX(4, (num_bit_lengths + 1));
  TDEFL_PUT_BITS(num_bit_lengths - 4, 4);
  for (i = 0; (int)i < num_bit_lengths; i++)
    TDEFL_PUT_BITS(
        d->m_huff_code_sizes[2][s_tdefl_packed_code_size_syms_swizzle[i]], 3);

  for (packed_code_sizes_index = 0;
       packed_code_sizes_index < num_packed_code_sizes;) {
    mz_uint code = packed_code_sizes[packed_code_sizes_index++];
    MZ_ASSERT(code < TDEFL_MAX_HUFF_SYMBOLS_2);
    TDEFL_PUT_BITS(d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code]);
    if (code >= 16)
      TDEFL_PUT_BITS(packed_code_sizes[packed_code_sizes_index++],
                     "\02\03\07"[code - 16]);
  }
}

static void tdefl_start_static_block(tdefl_compressor *d) {
  mz_uint i;
  mz_uint8 *p = &d->m_huff_code_sizes[0][0];

  for (i = 0; i <= 143; ++i)
    *p++ = 8;
  for (; i <= 255; ++i)
    *p++ = 9;
  for (; i <= 279; ++i)
    *p++ = 7;
  for (; i <= 287; ++i)
    *p++ = 8;

  memset(d->m_huff_code_sizes[1], 5, 32);

  tdefl_optimize_huffman_table(d, 0, 288, 15, MZ_TRUE);
  tdefl_optimize_huffman_table(d, 1, 32, 15, MZ_TRUE);

  TDEFL_PUT_BITS(1, 2);
}

static const mz_uint mz_bitmasks[17] = {
    0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF,
    0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF};

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN &&             \
    MINIZ_HAS_64BIT_REGISTERS
static mz_bool tdefl_compress_lz_codes(tdefl_compressor *d) {
  mz_uint flags;
  mz_uint8 *pLZ_codes;
  mz_uint8 *pOutput_buf = d->m_pOutput_buf;
  mz_uint8 *pLZ_code_buf_end = d->m_pLZ_code_buf;
  mz_uint64 bit_buffer = d->m_bit_buffer;
  mz_uint bits_in = d->m_bits_in;

#define TDEFL_PUT_BITS_FAST(b, l)                                              \
  {                                                                            \
    bit_buffer |= (((mz_uint64)(b)) << bits_in);                               \
    bits_in += (l);                                                            \
  }

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < pLZ_code_buf_end;
       flags >>= 1) {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;

    if (flags & 1) {
      mz_uint s0, s1, n0, n1, sym, num_extra_bits;
      mz_uint match_len = pLZ_codes[0],
              match_dist = *(const mz_uint16 *)(pLZ_codes + 1);
      pLZ_codes += 3;

      MZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][s_tdefl_len_sym[match_len]],
                          d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS_FAST(match_len & mz_bitmasks[s_tdefl_len_extra[match_len]],
                          s_tdefl_len_extra[match_len]);

      /* This sequence coaxes MSVC into using cmov's vs. jmp's. */
      s0 = s_tdefl_small_dist_sym[match_dist & 511];
      n0 = s_tdefl_small_dist_extra[match_dist & 511];
      s1 = s_tdefl_large_dist_sym[match_dist >> 8];
      n1 = s_tdefl_large_dist_extra[match_dist >> 8];
      sym = (match_dist < 512) ? s0 : s1;
      num_extra_bits = (match_dist < 512) ? n0 : n1;

      MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS_FAST(d->m_huff_codes[1][sym],
                          d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS_FAST(match_dist & mz_bitmasks[num_extra_bits],
                          num_extra_bits);
    } else {
      mz_uint lit = *pLZ_codes++;
      MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit],
                          d->m_huff_code_sizes[0][lit]);

      if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end)) {
        flags >>= 1;
        lit = *pLZ_codes++;
        MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
        TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit],
                            d->m_huff_code_sizes[0][lit]);

        if (((flags & 2) == 0) && (pLZ_codes < pLZ_code_buf_end)) {
          flags >>= 1;
          lit = *pLZ_codes++;
          MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
          TDEFL_PUT_BITS_FAST(d->m_huff_codes[0][lit],
                              d->m_huff_code_sizes[0][lit]);
        }
      }
    }

    if (pOutput_buf >= d->m_pOutput_buf_end)
      return MZ_FALSE;

    *(mz_uint64 *)pOutput_buf = bit_buffer;
    pOutput_buf += (bits_in >> 3);
    bit_buffer >>= (bits_in & ~7);
    bits_in &= 7;
  }

#undef TDEFL_PUT_BITS_FAST

  d->m_pOutput_buf = pOutput_buf;
  d->m_bits_in = 0;
  d->m_bit_buffer = 0;

  while (bits_in) {
    mz_uint32 n = MZ_MIN(bits_in, 16);
    TDEFL_PUT_BITS((mz_uint)bit_buffer & mz_bitmasks[n], n);
    bit_buffer >>= n;
    bits_in -= n;
  }

  TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#else
static mz_bool tdefl_compress_lz_codes(tdefl_compressor *d) {
  mz_uint flags;
  mz_uint8 *pLZ_codes;

  flags = 1;
  for (pLZ_codes = d->m_lz_code_buf; pLZ_codes < d->m_pLZ_code_buf;
       flags >>= 1) {
    if (flags == 1)
      flags = *pLZ_codes++ | 0x100;
    if (flags & 1) {
      mz_uint sym, num_extra_bits;
      mz_uint match_len = pLZ_codes[0],
              match_dist = (pLZ_codes[1] | (pLZ_codes[2] << 8));
      pLZ_codes += 3;

      MZ_ASSERT(d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS(d->m_huff_codes[0][s_tdefl_len_sym[match_len]],
                     d->m_huff_code_sizes[0][s_tdefl_len_sym[match_len]]);
      TDEFL_PUT_BITS(match_len & mz_bitmasks[s_tdefl_len_extra[match_len]],
                     s_tdefl_len_extra[match_len]);

      if (match_dist < 512) {
        sym = s_tdefl_small_dist_sym[match_dist];
        num_extra_bits = s_tdefl_small_dist_extra[match_dist];
      } else {
        sym = s_tdefl_large_dist_sym[match_dist >> 8];
        num_extra_bits = s_tdefl_large_dist_extra[match_dist >> 8];
      }
      MZ_ASSERT(d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS(d->m_huff_codes[1][sym], d->m_huff_code_sizes[1][sym]);
      TDEFL_PUT_BITS(match_dist & mz_bitmasks[num_extra_bits], num_extra_bits);
    } else {
      mz_uint lit = *pLZ_codes++;
      MZ_ASSERT(d->m_huff_code_sizes[0][lit]);
      TDEFL_PUT_BITS(d->m_huff_codes[0][lit], d->m_huff_code_sizes[0][lit]);
    }
  }

  TDEFL_PUT_BITS(d->m_huff_codes[0][256], d->m_huff_code_sizes[0][256]);

  return (d->m_pOutput_buf < d->m_pOutput_buf_end);
}
#endif /* MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN &&       \
          MINIZ_HAS_64BIT_REGISTERS */

static mz_bool tdefl_compress_block(tdefl_compressor *d, mz_bool static_block) {
  if (static_block)
    tdefl_start_static_block(d);
  else
    tdefl_start_dynamic_block(d);
  return tdefl_compress_lz_codes(d);
}

static int tdefl_flush_block(tdefl_compressor *d, int flush) {
  mz_uint saved_bit_buf, saved_bits_in;
  mz_uint8 *pSaved_output_buf;
  mz_bool comp_block_succeeded = MZ_FALSE;
  int n, use_raw_block =
             ((d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS) != 0) &&
             (d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size;
  mz_uint8 *pOutput_buf_start =
      ((d->m_pPut_buf_func == NULL) &&
       ((*d->m_pOut_buf_size - d->m_out_buf_ofs) >= TDEFL_OUT_BUF_SIZE))
          ? ((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs)
          : d->m_output_buf;

  d->m_pOutput_buf = pOutput_buf_start;
  d->m_pOutput_buf_end = d->m_pOutput_buf + TDEFL_OUT_BUF_SIZE - 16;

  MZ_ASSERT(!d->m_output_flush_remaining);
  d->m_output_flush_ofs = 0;
  d->m_output_flush_remaining = 0;

  *d->m_pLZ_flags = (mz_uint8)(*d->m_pLZ_flags >> d->m_num_flags_left);
  d->m_pLZ_code_buf -= (d->m_num_flags_left == 8);

  if ((d->m_flags & TDEFL_WRITE_ZLIB_HEADER) && (!d->m_block_index)) {
    TDEFL_PUT_BITS(0x78, 8);
    TDEFL_PUT_BITS(0x01, 8);
  }

  TDEFL_PUT_BITS(flush == TDEFL_FINISH, 1);

  pSaved_output_buf = d->m_pOutput_buf;
  saved_bit_buf = d->m_bit_buffer;
  saved_bits_in = d->m_bits_in;

  if (!use_raw_block)
    comp_block_succeeded =
        tdefl_compress_block(d, (d->m_flags & TDEFL_FORCE_ALL_STATIC_BLOCKS) ||
                                    (d->m_total_lz_bytes < 48));

  /* If the block gets expanded, forget the current contents of the output
   * buffer and send a raw block instead. */
  if (((use_raw_block) ||
       ((d->m_total_lz_bytes) && ((d->m_pOutput_buf - pSaved_output_buf + 1U) >=
                                  d->m_total_lz_bytes))) &&
      ((d->m_lookahead_pos - d->m_lz_code_buf_dict_pos) <= d->m_dict_size)) {
    mz_uint i;
    d->m_pOutput_buf = pSaved_output_buf;
    d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    TDEFL_PUT_BITS(0, 2);
    if (d->m_bits_in) {
      TDEFL_PUT_BITS(0, 8 - d->m_bits_in);
    }
    for (i = 2; i; --i, d->m_total_lz_bytes ^= 0xFFFF) {
      TDEFL_PUT_BITS(d->m_total_lz_bytes & 0xFFFF, 16);
    }
    for (i = 0; i < d->m_total_lz_bytes; ++i) {
      TDEFL_PUT_BITS(
          d->m_dict[(d->m_lz_code_buf_dict_pos + i) & TDEFL_LZ_DICT_SIZE_MASK],
          8);
    }
  }
  /* Check for the extremely unlikely (if not impossible) case of the compressed
     block not fitting into the output buffer when using dynamic codes. */
  else if (!comp_block_succeeded) {
    d->m_pOutput_buf = pSaved_output_buf;
    d->m_bit_buffer = saved_bit_buf, d->m_bits_in = saved_bits_in;
    tdefl_compress_block(d, MZ_TRUE);
  }

  if (flush) {
    if (flush == TDEFL_FINISH) {
      if (d->m_bits_in) {
        TDEFL_PUT_BITS(0, 8 - d->m_bits_in);
      }
      if (d->m_flags & TDEFL_WRITE_ZLIB_HEADER) {
        mz_uint i, a = d->m_adler32;
        for (i = 0; i < 4; i++) {
          TDEFL_PUT_BITS((a >> 24) & 0xFF, 8);
          a <<= 8;
        }
      }
    } else {
      mz_uint i, z = 0;
      TDEFL_PUT_BITS(0, 3);
      if (d->m_bits_in) {
        TDEFL_PUT_BITS(0, 8 - d->m_bits_in);
      }
      for (i = 2; i; --i, z ^= 0xFFFF) {
        TDEFL_PUT_BITS(z & 0xFFFF, 16);
      }
    }
  }

  MZ_ASSERT(d->m_pOutput_buf < d->m_pOutput_buf_end);

  memset(&d->m_huff_count[0][0], 0,
         sizeof(d->m_huff_count[0][0]) * TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0,
         sizeof(d->m_huff_count[1][0]) * TDEFL_MAX_HUFF_SYMBOLS_1);

  d->m_pLZ_code_buf = d->m_lz_code_buf + 1;
  d->m_pLZ_flags = d->m_lz_code_buf;
  d->m_num_flags_left = 8;
  d->m_lz_code_buf_dict_pos += d->m_total_lz_bytes;
  d->m_total_lz_bytes = 0;
  d->m_block_index++;

  if ((n = (int)(d->m_pOutput_buf - pOutput_buf_start)) != 0) {
    if (d->m_pPut_buf_func) {
      *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
      if (!(*d->m_pPut_buf_func)(d->m_output_buf, n, d->m_pPut_buf_user))
        return (d->m_prev_return_status = TDEFL_STATUS_PUT_BUF_FAILED);
    } else if (pOutput_buf_start == d->m_output_buf) {
      int bytes_to_copy = (int)MZ_MIN(
          (size_t)n, (size_t)(*d->m_pOut_buf_size - d->m_out_buf_ofs));
      memcpy((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs, d->m_output_buf,
             bytes_to_copy);
      d->m_out_buf_ofs += bytes_to_copy;
      if ((n -= bytes_to_copy) != 0) {
        d->m_output_flush_ofs = bytes_to_copy;
        d->m_output_flush_remaining = n;
      }
    } else {
      d->m_out_buf_ofs += n;
    }
  }

  return d->m_output_flush_remaining;
}

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
#ifdef MINIZ_UNALIGNED_USE_MEMCPY
static mz_uint16 TDEFL_READ_UNALIGNED_WORD(const mz_uint8 *p) {
  mz_uint16 ret;
  memcpy(&ret, p, sizeof(mz_uint16));
  return ret;
}
static mz_uint16 TDEFL_READ_UNALIGNED_WORD2(const mz_uint16 *p) {
  mz_uint16 ret;
  memcpy(&ret, p, sizeof(mz_uint16));
  return ret;
}
#else
#define TDEFL_READ_UNALIGNED_WORD(p) *(const mz_uint16 *)(p)
#define TDEFL_READ_UNALIGNED_WORD2(p) *(const mz_uint16 *)(p)
#endif
static MZ_FORCEINLINE void
tdefl_find_match(tdefl_compressor *d, mz_uint lookahead_pos, mz_uint max_dist,
                 mz_uint max_match_len, mz_uint *pMatch_dist,
                 mz_uint *pMatch_len) {
  mz_uint dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK,
                match_len = *pMatch_len, probe_pos = pos, next_probe_pos,
                probe_len;
  mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mz_uint16 *s = (const mz_uint16 *)(d->m_dict + pos), *p, *q;
  mz_uint16 c01 = TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]),
            s01 = TDEFL_READ_UNALIGNED_WORD2(s);
  MZ_ASSERT(max_match_len <= TDEFL_MAX_MATCH_LEN);
  if (max_match_len <= match_len)
    return;
  for (;;) {
    for (;;) {
      if (--num_probes_left == 0)
        return;
#define TDEFL_PROBE                                                            \
  next_probe_pos = d->m_next[probe_pos];                                       \
  if ((!next_probe_pos) ||                                                     \
      ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist))       \
    return;                                                                    \
  probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK;                        \
  if (TDEFL_READ_UNALIGNED_WORD(&d->m_dict[probe_pos + match_len - 1]) == c01) \
    break;
      TDEFL_PROBE;
      TDEFL_PROBE;
      TDEFL_PROBE;
    }
    if (!dist)
      break;
    q = (const mz_uint16 *)(d->m_dict + probe_pos);
    if (TDEFL_READ_UNALIGNED_WORD2(q) != s01)
      continue;
    p = s;
    probe_len = 32;
    do {
    } while (
        (TDEFL_READ_UNALIGNED_WORD2(++p) == TDEFL_READ_UNALIGNED_WORD2(++q)) &&
        (TDEFL_READ_UNALIGNED_WORD2(++p) == TDEFL_READ_UNALIGNED_WORD2(++q)) &&
        (TDEFL_READ_UNALIGNED_WORD2(++p) == TDEFL_READ_UNALIGNED_WORD2(++q)) &&
        (TDEFL_READ_UNALIGNED_WORD2(++p) == TDEFL_READ_UNALIGNED_WORD2(++q)) &&
        (--probe_len > 0));
    if (!probe_len) {
      *pMatch_dist = dist;
      *pMatch_len = MZ_MIN(max_match_len, (mz_uint)TDEFL_MAX_MATCH_LEN);
      break;
    } else if ((probe_len = ((mz_uint)(p - s) * 2) +
                            (mz_uint)(*(const mz_uint8 *)p ==
                                      *(const mz_uint8 *)q)) > match_len) {
      *pMatch_dist = dist;
      if ((*pMatch_len = match_len = MZ_MIN(max_match_len, probe_len)) ==
          max_match_len)
        break;
      c01 = TDEFL_READ_UNALIGNED_WORD(&d->m_dict[pos + match_len - 1]);
    }
  }
}
#else
static MZ_FORCEINLINE void
tdefl_find_match(tdefl_compressor *d, mz_uint lookahead_pos, mz_uint max_dist,
                 mz_uint max_match_len, mz_uint *pMatch_dist,
                 mz_uint *pMatch_len) {
  mz_uint dist, pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK,
                match_len = *pMatch_len, probe_pos = pos, next_probe_pos,
                probe_len;
  mz_uint num_probes_left = d->m_max_probes[match_len >= 32];
  const mz_uint8 *s = d->m_dict + pos, *p, *q;
  mz_uint8 c0 = d->m_dict[pos + match_len], c1 = d->m_dict[pos + match_len - 1];
  MZ_ASSERT(max_match_len <= TDEFL_MAX_MATCH_LEN);
  if (max_match_len <= match_len)
    return;
  for (;;) {
    for (;;) {
      if (--num_probes_left == 0)
        return;
#define TDEFL_PROBE                                                            \
  next_probe_pos = d->m_next[probe_pos];                                       \
  if ((!next_probe_pos) ||                                                     \
      ((dist = (mz_uint16)(lookahead_pos - next_probe_pos)) > max_dist))       \
    return;                                                                    \
  probe_pos = next_probe_pos & TDEFL_LZ_DICT_SIZE_MASK;                        \
  if ((d->m_dict[probe_pos + match_len] == c0) &&                              \
      (d->m_dict[probe_pos + match_len - 1] == c1))                            \
    break;
      TDEFL_PROBE;
      TDEFL_PROBE;
      TDEFL_PROBE;
    }
    if (!dist)
      break;
    p = s;
    q = d->m_dict + probe_pos;
    for (probe_len = 0; probe_len < max_match_len; probe_len++)
      if (*p++ != *q++)
        break;
    if (probe_len > match_len) {
      *pMatch_dist = dist;
      if ((*pMatch_len = match_len = probe_len) == max_match_len)
        return;
      c0 = d->m_dict[pos + match_len];
      c1 = d->m_dict[pos + match_len - 1];
    }
  }
}
#endif /* #if MINIZ_USE_UNALIGNED_LOADS_AND_STORES */

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
#ifdef MINIZ_UNALIGNED_USE_MEMCPY
static mz_uint32 TDEFL_READ_UNALIGNED_WORD32(const mz_uint8 *p) {
  mz_uint32 ret;
  memcpy(&ret, p, sizeof(mz_uint32));
  return ret;
}
#else
#define TDEFL_READ_UNALIGNED_WORD32(p) *(const mz_uint32 *)(p)
#endif
static mz_bool tdefl_compress_fast(tdefl_compressor *d) {
  /* Faster, minimally featured LZRW1-style match+parse loop with better
   * register utilization. Intended for applications where raw throughput is
   * valued more highly than ratio. */
  mz_uint lookahead_pos = d->m_lookahead_pos,
          lookahead_size = d->m_lookahead_size, dict_size = d->m_dict_size,
          total_lz_bytes = d->m_total_lz_bytes,
          num_flags_left = d->m_num_flags_left;
  mz_uint8 *pLZ_code_buf = d->m_pLZ_code_buf, *pLZ_flags = d->m_pLZ_flags;
  mz_uint cur_pos = lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;

  while ((d->m_src_buf_left) || ((d->m_flush) && (lookahead_size))) {
    const mz_uint TDEFL_COMP_FAST_LOOKAHEAD_SIZE = 4096;
    mz_uint dst_pos =
        (lookahead_pos + lookahead_size) & TDEFL_LZ_DICT_SIZE_MASK;
    mz_uint num_bytes_to_process = (mz_uint)MZ_MIN(
        d->m_src_buf_left, TDEFL_COMP_FAST_LOOKAHEAD_SIZE - lookahead_size);
    d->m_src_buf_left -= num_bytes_to_process;
    lookahead_size += num_bytes_to_process;

    while (num_bytes_to_process) {
      mz_uint32 n = MZ_MIN(TDEFL_LZ_DICT_SIZE - dst_pos, num_bytes_to_process);
      memcpy(d->m_dict + dst_pos, d->m_pSrc, n);
      if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
        memcpy(d->m_dict + TDEFL_LZ_DICT_SIZE + dst_pos, d->m_pSrc,
               MZ_MIN(n, (TDEFL_MAX_MATCH_LEN - 1) - dst_pos));
      d->m_pSrc += n;
      dst_pos = (dst_pos + n) & TDEFL_LZ_DICT_SIZE_MASK;
      num_bytes_to_process -= n;
    }

    dict_size = MZ_MIN(TDEFL_LZ_DICT_SIZE - lookahead_size, dict_size);
    if ((!d->m_flush) && (lookahead_size < TDEFL_COMP_FAST_LOOKAHEAD_SIZE))
      break;

    while (lookahead_size >= 4) {
      mz_uint cur_match_dist, cur_match_len = 1;
      mz_uint8 *pCur_dict = d->m_dict + cur_pos;
      mz_uint first_trigram = TDEFL_READ_UNALIGNED_WORD32(pCur_dict) & 0xFFFFFF;
      mz_uint hash =
          (first_trigram ^ (first_trigram >> (24 - (TDEFL_LZ_HASH_BITS - 8)))) &
          TDEFL_LEVEL1_HASH_SIZE_MASK;
      mz_uint probe_pos = d->m_hash[hash];
      d->m_hash[hash] = (mz_uint16)lookahead_pos;

      if (((cur_match_dist = (mz_uint16)(lookahead_pos - probe_pos)) <=
           dict_size) &&
          ((TDEFL_READ_UNALIGNED_WORD32(
                d->m_dict + (probe_pos &= TDEFL_LZ_DICT_SIZE_MASK)) &
            0xFFFFFF) == first_trigram)) {
        const mz_uint16 *p = (const mz_uint16 *)pCur_dict;
        const mz_uint16 *q = (const mz_uint16 *)(d->m_dict + probe_pos);
        mz_uint32 probe_len = 32;
        do {
        } while ((TDEFL_READ_UNALIGNED_WORD2(++p) ==
                  TDEFL_READ_UNALIGNED_WORD2(++q)) &&
                 (TDEFL_READ_UNALIGNED_WORD2(++p) ==
                  TDEFL_READ_UNALIGNED_WORD2(++q)) &&
                 (TDEFL_READ_UNALIGNED_WORD2(++p) ==
                  TDEFL_READ_UNALIGNED_WORD2(++q)) &&
                 (TDEFL_READ_UNALIGNED_WORD2(++p) ==
                  TDEFL_READ_UNALIGNED_WORD2(++q)) &&
                 (--probe_len > 0));
        cur_match_len = ((mz_uint)(p - (const mz_uint16 *)pCur_dict) * 2) +
                        (mz_uint)(*(const mz_uint8 *)p == *(const mz_uint8 *)q);
        if (!probe_len)
          cur_match_len = cur_match_dist ? TDEFL_MAX_MATCH_LEN : 0;

        if ((cur_match_len < TDEFL_MIN_MATCH_LEN) ||
            ((cur_match_len == TDEFL_MIN_MATCH_LEN) &&
             (cur_match_dist >= 8U * 1024U))) {
          cur_match_len = 1;
          *pLZ_code_buf++ = (mz_uint8)first_trigram;
          *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
          d->m_huff_count[0][(mz_uint8)first_trigram]++;
        } else {
          mz_uint32 s0, s1;
          cur_match_len = MZ_MIN(cur_match_len, lookahead_size);

          MZ_ASSERT((cur_match_len >= TDEFL_MIN_MATCH_LEN) &&
                    (cur_match_dist >= 1) &&
                    (cur_match_dist <= TDEFL_LZ_DICT_SIZE));

          cur_match_dist--;

          pLZ_code_buf[0] = (mz_uint8)(cur_match_len - TDEFL_MIN_MATCH_LEN);
#ifdef MINIZ_UNALIGNED_USE_MEMCPY
          memcpy(&pLZ_code_buf[1], &cur_match_dist, sizeof(cur_match_dist));
#else
          *(mz_uint16 *)(&pLZ_code_buf[1]) = (mz_uint16)cur_match_dist;
#endif
          pLZ_code_buf += 3;
          *pLZ_flags = (mz_uint8)((*pLZ_flags >> 1) | 0x80);

          s0 = s_tdefl_small_dist_sym[cur_match_dist & 511];
          s1 = s_tdefl_large_dist_sym[cur_match_dist >> 8];
          d->m_huff_count[1][(cur_match_dist < 512) ? s0 : s1]++;

          d->m_huff_count[0][s_tdefl_len_sym[cur_match_len -
                                             TDEFL_MIN_MATCH_LEN]]++;
        }
      } else {
        *pLZ_code_buf++ = (mz_uint8)first_trigram;
        *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
        d->m_huff_count[0][(mz_uint8)first_trigram]++;
      }

      if (--num_flags_left == 0) {
        num_flags_left = 8;
        pLZ_flags = pLZ_code_buf++;
      }

      total_lz_bytes += cur_match_len;
      lookahead_pos += cur_match_len;
      dict_size =
          MZ_MIN(dict_size + cur_match_len, (mz_uint)TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + cur_match_len) & TDEFL_LZ_DICT_SIZE_MASK;
      MZ_ASSERT(lookahead_size >= cur_match_len);
      lookahead_size -= cur_match_len;

      if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8]) {
        int n;
        d->m_lookahead_pos = lookahead_pos;
        d->m_lookahead_size = lookahead_size;
        d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes;
        d->m_pLZ_code_buf = pLZ_code_buf;
        d->m_pLZ_flags = pLZ_flags;
        d->m_num_flags_left = num_flags_left;
        if ((n = tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? MZ_FALSE : MZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes;
        pLZ_code_buf = d->m_pLZ_code_buf;
        pLZ_flags = d->m_pLZ_flags;
        num_flags_left = d->m_num_flags_left;
      }
    }

    while (lookahead_size) {
      mz_uint8 lit = d->m_dict[cur_pos];

      total_lz_bytes++;
      *pLZ_code_buf++ = lit;
      *pLZ_flags = (mz_uint8)(*pLZ_flags >> 1);
      if (--num_flags_left == 0) {
        num_flags_left = 8;
        pLZ_flags = pLZ_code_buf++;
      }

      d->m_huff_count[0][lit]++;

      lookahead_pos++;
      dict_size = MZ_MIN(dict_size + 1, (mz_uint)TDEFL_LZ_DICT_SIZE);
      cur_pos = (cur_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK;
      lookahead_size--;

      if (pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8]) {
        int n;
        d->m_lookahead_pos = lookahead_pos;
        d->m_lookahead_size = lookahead_size;
        d->m_dict_size = dict_size;
        d->m_total_lz_bytes = total_lz_bytes;
        d->m_pLZ_code_buf = pLZ_code_buf;
        d->m_pLZ_flags = pLZ_flags;
        d->m_num_flags_left = num_flags_left;
        if ((n = tdefl_flush_block(d, 0)) != 0)
          return (n < 0) ? MZ_FALSE : MZ_TRUE;
        total_lz_bytes = d->m_total_lz_bytes;
        pLZ_code_buf = d->m_pLZ_code_buf;
        pLZ_flags = d->m_pLZ_flags;
        num_flags_left = d->m_num_flags_left;
      }
    }
  }

  d->m_lookahead_pos = lookahead_pos;
  d->m_lookahead_size = lookahead_size;
  d->m_dict_size = dict_size;
  d->m_total_lz_bytes = total_lz_bytes;
  d->m_pLZ_code_buf = pLZ_code_buf;
  d->m_pLZ_flags = pLZ_flags;
  d->m_num_flags_left = num_flags_left;
  return MZ_TRUE;
}
#endif /* MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN */

static MZ_FORCEINLINE void tdefl_record_literal(tdefl_compressor *d,
                                                mz_uint8 lit) {
  d->m_total_lz_bytes++;
  *d->m_pLZ_code_buf++ = lit;
  *d->m_pLZ_flags = (mz_uint8)(*d->m_pLZ_flags >> 1);
  if (--d->m_num_flags_left == 0) {
    d->m_num_flags_left = 8;
    d->m_pLZ_flags = d->m_pLZ_code_buf++;
  }
  d->m_huff_count[0][lit]++;
}

static MZ_FORCEINLINE void
tdefl_record_match(tdefl_compressor *d, mz_uint match_len, mz_uint match_dist) {
  mz_uint32 s0, s1;

  MZ_ASSERT((match_len >= TDEFL_MIN_MATCH_LEN) && (match_dist >= 1) &&
            (match_dist <= TDEFL_LZ_DICT_SIZE));

  d->m_total_lz_bytes += match_len;

  d->m_pLZ_code_buf[0] = (mz_uint8)(match_len - TDEFL_MIN_MATCH_LEN);

  match_dist -= 1;
  d->m_pLZ_code_buf[1] = (mz_uint8)(match_dist & 0xFF);
  d->m_pLZ_code_buf[2] = (mz_uint8)(match_dist >> 8);
  d->m_pLZ_code_buf += 3;

  *d->m_pLZ_flags = (mz_uint8)((*d->m_pLZ_flags >> 1) | 0x80);
  if (--d->m_num_flags_left == 0) {
    d->m_num_flags_left = 8;
    d->m_pLZ_flags = d->m_pLZ_code_buf++;
  }

  s0 = s_tdefl_small_dist_sym[match_dist & 511];
  s1 = s_tdefl_large_dist_sym[(match_dist >> 8) & 127];
  d->m_huff_count[1][(match_dist < 512) ? s0 : s1]++;
  d->m_huff_count[0][s_tdefl_len_sym[match_len - TDEFL_MIN_MATCH_LEN]]++;
}

static mz_bool tdefl_compress_normal(tdefl_compressor *d) {
  const mz_uint8 *pSrc = d->m_pSrc;
  size_t src_buf_left = d->m_src_buf_left;
  tdefl_flush flush = d->m_flush;

  while ((src_buf_left) || ((flush) && (d->m_lookahead_size))) {
    mz_uint len_to_move, cur_match_dist, cur_match_len, cur_pos;
    /* Update dictionary and hash chains. Keeps the lookahead size equal to
     * TDEFL_MAX_MATCH_LEN. */
    if ((d->m_lookahead_size + d->m_dict_size) >= (TDEFL_MIN_MATCH_LEN - 1)) {
      mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) &
                        TDEFL_LZ_DICT_SIZE_MASK,
              ins_pos = d->m_lookahead_pos + d->m_lookahead_size - 2;
      mz_uint hash = (d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK]
                      << TDEFL_LZ_HASH_SHIFT) ^
                     d->m_dict[(ins_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK];
      mz_uint num_bytes_to_process = (mz_uint)MZ_MIN(
          src_buf_left, TDEFL_MAX_MATCH_LEN - d->m_lookahead_size);
      const mz_uint8 *pSrc_end = pSrc + num_bytes_to_process;
      src_buf_left -= num_bytes_to_process;
      d->m_lookahead_size += num_bytes_to_process;
      while (pSrc != pSrc_end) {
        mz_uint8 c = *pSrc++;
        d->m_dict[dst_pos] = c;
        if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
          d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        hash = ((hash << TDEFL_LZ_HASH_SHIFT) ^ c) & (TDEFL_LZ_HASH_SIZE - 1);
        d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash];
        d->m_hash[hash] = (mz_uint16)(ins_pos);
        dst_pos = (dst_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK;
        ins_pos++;
      }
    } else {
      while ((src_buf_left) && (d->m_lookahead_size < TDEFL_MAX_MATCH_LEN)) {
        mz_uint8 c = *pSrc++;
        mz_uint dst_pos = (d->m_lookahead_pos + d->m_lookahead_size) &
                          TDEFL_LZ_DICT_SIZE_MASK;
        src_buf_left--;
        d->m_dict[dst_pos] = c;
        if (dst_pos < (TDEFL_MAX_MATCH_LEN - 1))
          d->m_dict[TDEFL_LZ_DICT_SIZE + dst_pos] = c;
        if ((++d->m_lookahead_size + d->m_dict_size) >= TDEFL_MIN_MATCH_LEN) {
          mz_uint ins_pos = d->m_lookahead_pos + (d->m_lookahead_size - 1) - 2;
          mz_uint hash = ((d->m_dict[ins_pos & TDEFL_LZ_DICT_SIZE_MASK]
                           << (TDEFL_LZ_HASH_SHIFT * 2)) ^
                          (d->m_dict[(ins_pos + 1) & TDEFL_LZ_DICT_SIZE_MASK]
                           << TDEFL_LZ_HASH_SHIFT) ^
                          c) &
                         (TDEFL_LZ_HASH_SIZE - 1);
          d->m_next[ins_pos & TDEFL_LZ_DICT_SIZE_MASK] = d->m_hash[hash];
          d->m_hash[hash] = (mz_uint16)(ins_pos);
        }
      }
    }
    d->m_dict_size =
        MZ_MIN(TDEFL_LZ_DICT_SIZE - d->m_lookahead_size, d->m_dict_size);
    if ((!flush) && (d->m_lookahead_size < TDEFL_MAX_MATCH_LEN))
      break;

    /* Simple lazy/greedy parsing state machine. */
    len_to_move = 1;
    cur_match_dist = 0;
    cur_match_len =
        d->m_saved_match_len ? d->m_saved_match_len : (TDEFL_MIN_MATCH_LEN - 1);
    cur_pos = d->m_lookahead_pos & TDEFL_LZ_DICT_SIZE_MASK;
    if (d->m_flags & (TDEFL_RLE_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS)) {
      if ((d->m_dict_size) && (!(d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS))) {
        mz_uint8 c = d->m_dict[(cur_pos - 1) & TDEFL_LZ_DICT_SIZE_MASK];
        cur_match_len = 0;
        while (cur_match_len < d->m_lookahead_size) {
          if (d->m_dict[cur_pos + cur_match_len] != c)
            break;
          cur_match_len++;
        }
        if (cur_match_len < TDEFL_MIN_MATCH_LEN)
          cur_match_len = 0;
        else
          cur_match_dist = 1;
      }
    } else {
      tdefl_find_match(d, d->m_lookahead_pos, d->m_dict_size,
                       d->m_lookahead_size, &cur_match_dist, &cur_match_len);
    }
    if (((cur_match_len == TDEFL_MIN_MATCH_LEN) &&
         (cur_match_dist >= 8U * 1024U)) ||
        (cur_pos == cur_match_dist) ||
        ((d->m_flags & TDEFL_FILTER_MATCHES) && (cur_match_len <= 5))) {
      cur_match_dist = cur_match_len = 0;
    }
    if (d->m_saved_match_len) {
      if (cur_match_len > d->m_saved_match_len) {
        tdefl_record_literal(d, (mz_uint8)d->m_saved_lit);
        if (cur_match_len >= 128) {
          tdefl_record_match(d, cur_match_len, cur_match_dist);
          d->m_saved_match_len = 0;
          len_to_move = cur_match_len;
        } else {
          d->m_saved_lit = d->m_dict[cur_pos];
          d->m_saved_match_dist = cur_match_dist;
          d->m_saved_match_len = cur_match_len;
        }
      } else {
        tdefl_record_match(d, d->m_saved_match_len, d->m_saved_match_dist);
        len_to_move = d->m_saved_match_len - 1;
        d->m_saved_match_len = 0;
      }
    } else if (!cur_match_dist)
      tdefl_record_literal(d,
                           d->m_dict[MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)]);
    else if ((d->m_greedy_parsing) || (d->m_flags & TDEFL_RLE_MATCHES) ||
             (cur_match_len >= 128)) {
      tdefl_record_match(d, cur_match_len, cur_match_dist);
      len_to_move = cur_match_len;
    } else {
      d->m_saved_lit = d->m_dict[MZ_MIN(cur_pos, sizeof(d->m_dict) - 1)];
      d->m_saved_match_dist = cur_match_dist;
      d->m_saved_match_len = cur_match_len;
    }
    /* Move the lookahead forward by len_to_move bytes. */
    d->m_lookahead_pos += len_to_move;
    MZ_ASSERT(d->m_lookahead_size >= len_to_move);
    d->m_lookahead_size -= len_to_move;
    d->m_dict_size =
        MZ_MIN(d->m_dict_size + len_to_move, (mz_uint)TDEFL_LZ_DICT_SIZE);
    /* Check if it's time to flush the current LZ codes to the internal output
     * buffer. */
    if ((d->m_pLZ_code_buf > &d->m_lz_code_buf[TDEFL_LZ_CODE_BUF_SIZE - 8]) ||
        ((d->m_total_lz_bytes > 31 * 1024) &&
         (((((mz_uint)(d->m_pLZ_code_buf - d->m_lz_code_buf) * 115) >> 7) >=
           d->m_total_lz_bytes) ||
          (d->m_flags & TDEFL_FORCE_ALL_RAW_BLOCKS)))) {
      int n;
      d->m_pSrc = pSrc;
      d->m_src_buf_left = src_buf_left;
      if ((n = tdefl_flush_block(d, 0)) != 0)
        return (n < 0) ? MZ_FALSE : MZ_TRUE;
    }
  }

  d->m_pSrc = pSrc;
  d->m_src_buf_left = src_buf_left;
  return MZ_TRUE;
}

static tdefl_status tdefl_flush_output_buffer(tdefl_compressor *d) {
  if (d->m_pIn_buf_size) {
    *d->m_pIn_buf_size = d->m_pSrc - (const mz_uint8 *)d->m_pIn_buf;
  }

  if (d->m_pOut_buf_size) {
    size_t n = MZ_MIN(*d->m_pOut_buf_size - d->m_out_buf_ofs,
                      d->m_output_flush_remaining);
    memcpy((mz_uint8 *)d->m_pOut_buf + d->m_out_buf_ofs,
           d->m_output_buf + d->m_output_flush_ofs, n);
    d->m_output_flush_ofs += (mz_uint)n;
    d->m_output_flush_remaining -= (mz_uint)n;
    d->m_out_buf_ofs += n;

    *d->m_pOut_buf_size = d->m_out_buf_ofs;
  }

  return (d->m_finished && !d->m_output_flush_remaining) ? TDEFL_STATUS_DONE
                                                         : TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_compress(tdefl_compressor *d, const void *pIn_buf,
                            size_t *pIn_buf_size, void *pOut_buf,
                            size_t *pOut_buf_size, tdefl_flush flush) {
  if (!d) {
    if (pIn_buf_size)
      *pIn_buf_size = 0;
    if (pOut_buf_size)
      *pOut_buf_size = 0;
    return TDEFL_STATUS_BAD_PARAM;
  }

  d->m_pIn_buf = pIn_buf;
  d->m_pIn_buf_size = pIn_buf_size;
  d->m_pOut_buf = pOut_buf;
  d->m_pOut_buf_size = pOut_buf_size;
  d->m_pSrc = (const mz_uint8 *)(pIn_buf);
  d->m_src_buf_left = pIn_buf_size ? *pIn_buf_size : 0;
  d->m_out_buf_ofs = 0;
  d->m_flush = flush;

  if (((d->m_pPut_buf_func != NULL) ==
       ((pOut_buf != NULL) || (pOut_buf_size != NULL))) ||
      (d->m_prev_return_status != TDEFL_STATUS_OKAY) ||
      (d->m_wants_to_finish && (flush != TDEFL_FINISH)) ||
      (pIn_buf_size && *pIn_buf_size && !pIn_buf) ||
      (pOut_buf_size && *pOut_buf_size && !pOut_buf)) {
    if (pIn_buf_size)
      *pIn_buf_size = 0;
    if (pOut_buf_size)
      *pOut_buf_size = 0;
    return (d->m_prev_return_status = TDEFL_STATUS_BAD_PARAM);
  }
  d->m_wants_to_finish |= (flush == TDEFL_FINISH);

  if ((d->m_output_flush_remaining) || (d->m_finished))
    return (d->m_prev_return_status = tdefl_flush_output_buffer(d));

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
  if (((d->m_flags & TDEFL_MAX_PROBES_MASK) == 1) &&
      ((d->m_flags & TDEFL_GREEDY_PARSING_FLAG) != 0) &&
      ((d->m_flags & (TDEFL_FILTER_MATCHES | TDEFL_FORCE_ALL_RAW_BLOCKS |
                      TDEFL_RLE_MATCHES)) == 0)) {
    if (!tdefl_compress_fast(d))
      return d->m_prev_return_status;
  } else
#endif /* #if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN */
  {
    if (!tdefl_compress_normal(d))
      return d->m_prev_return_status;
  }

  if ((d->m_flags & (TDEFL_WRITE_ZLIB_HEADER | TDEFL_COMPUTE_ADLER32)) &&
      (pIn_buf))
    d->m_adler32 =
        (mz_uint32)mz_adler32(d->m_adler32, (const mz_uint8 *)pIn_buf,
                              d->m_pSrc - (const mz_uint8 *)pIn_buf);

  if ((flush) && (!d->m_lookahead_size) && (!d->m_src_buf_left) &&
      (!d->m_output_flush_remaining)) {
    if (tdefl_flush_block(d, flush) < 0)
      return d->m_prev_return_status;
    d->m_finished = (flush == TDEFL_FINISH);
    if (flush == TDEFL_FULL_FLUSH) {
      MZ_CLEAR_OBJ(d->m_hash);
      MZ_CLEAR_OBJ(d->m_next);
      d->m_dict_size = 0;
    }
  }

  return (d->m_prev_return_status = tdefl_flush_output_buffer(d));
}

tdefl_status tdefl_compress_buffer(tdefl_compressor *d, const void *pIn_buf,
                                   size_t in_buf_size, tdefl_flush flush) {
  MZ_ASSERT(d->m_pPut_buf_func);
  return tdefl_compress(d, pIn_buf, &in_buf_size, NULL, NULL, flush);
}

tdefl_status tdefl_init(tdefl_compressor *d,
                        tdefl_put_buf_func_ptr pPut_buf_func,
                        void *pPut_buf_user, int flags) {
  d->m_pPut_buf_func = pPut_buf_func;
  d->m_pPut_buf_user = pPut_buf_user;
  d->m_flags = (mz_uint)(flags);
  d->m_max_probes[0] = 1 + ((flags & 0xFFF) + 2) / 3;
  d->m_greedy_parsing = (flags & TDEFL_GREEDY_PARSING_FLAG) != 0;
  d->m_max_probes[1] = 1 + (((flags & 0xFFF) >> 2) + 2) / 3;
  if (!(flags & TDEFL_NONDETERMINISTIC_PARSING_FLAG))
    MZ_CLEAR_OBJ(d->m_hash);
  d->m_lookahead_pos = d->m_lookahead_size = d->m_dict_size =
      d->m_total_lz_bytes = d->m_lz_code_buf_dict_pos = d->m_bits_in = 0;
  d->m_output_flush_ofs = d->m_output_flush_remaining = d->m_finished =
      d->m_block_index = d->m_bit_buffer = d->m_wants_to_finish = 0;
  d->m_pLZ_code_buf = d->m_lz_code_buf + 1;
  d->m_pLZ_flags = d->m_lz_code_buf;
  *d->m_pLZ_flags = 0;
  d->m_num_flags_left = 8;
  d->m_pOutput_buf = d->m_output_buf;
  d->m_pOutput_buf_end = d->m_output_buf;
  d->m_prev_return_status = TDEFL_STATUS_OKAY;
  d->m_saved_match_dist = d->m_saved_match_len = d->m_saved_lit = 0;
  d->m_adler32 = 1;
  d->m_pIn_buf = NULL;
  d->m_pOut_buf = NULL;
  d->m_pIn_buf_size = NULL;
  d->m_pOut_buf_size = NULL;
  d->m_flush = TDEFL_NO_FLUSH;
  d->m_pSrc = NULL;
  d->m_src_buf_left = 0;
  d->m_out_buf_ofs = 0;
  if (!(flags & TDEFL_NONDETERMINISTIC_PARSING_FLAG))
    MZ_CLEAR_OBJ(d->m_dict);
  memset(&d->m_huff_count[0][0], 0,
         sizeof(d->m_huff_count[0][0]) * TDEFL_MAX_HUFF_SYMBOLS_0);
  memset(&d->m_huff_count[1][0], 0,
         sizeof(d->m_huff_count[1][0]) * TDEFL_MAX_HUFF_SYMBOLS_1);
  return TDEFL_STATUS_OKAY;
}

tdefl_status tdefl_get_prev_return_status(tdefl_compressor *d) {
  return d->m_prev_return_status;
}

mz_uint32 tdefl_get_adler32(tdefl_compressor *d) { return d->m_adler32; }

mz_bool tdefl_compress_mem_to_output(const void *pBuf, size_t buf_len,
                                     tdefl_put_buf_func_ptr pPut_buf_func,
                                     void *pPut_buf_user, int flags) {
  tdefl_compressor *pComp;
  mz_bool succeeded;
  if (((buf_len) && (!pBuf)) || (!pPut_buf_func))
    return MZ_FALSE;
  pComp = (tdefl_compressor *)MZ_MALLOC(sizeof(tdefl_compressor));
  if (!pComp)
    return MZ_FALSE;
  succeeded = (tdefl_init(pComp, pPut_buf_func, pPut_buf_user, flags) ==
               TDEFL_STATUS_OKAY);
  succeeded =
      succeeded && (tdefl_compress_buffer(pComp, pBuf, buf_len, TDEFL_FINISH) ==
                    TDEFL_STATUS_DONE);
  MZ_FREE(pComp);
  return succeeded;
}

typedef struct {
  size_t m_size, m_capacity;
  mz_uint8 *m_pBuf;
  mz_bool m_expandable;
} tdefl_output_buffer;

static mz_bool tdefl_output_buffer_putter(const void *pBuf, int len,
                                          void *pUser) {
  tdefl_output_buffer *p = (tdefl_output_buffer *)pUser;
  size_t new_size = p->m_size + len;
  if (new_size > p->m_capacity) {
    size_t new_capacity = p->m_capacity;
    mz_uint8 *pNew_buf;
    if (!p->m_expandable)
      return MZ_FALSE;
    do {
      new_capacity = MZ_MAX(128U, new_capacity << 1U);
    } while (new_size > new_capacity);
    pNew_buf = (mz_uint8 *)MZ_REALLOC(p->m_pBuf, new_capacity);
    if (!pNew_buf)
      return MZ_FALSE;
    p->m_pBuf = pNew_buf;
    p->m_capacity = new_capacity;
  }
  memcpy((mz_uint8 *)p->m_pBuf + p->m_size, pBuf, len);
  p->m_size = new_size;
  return MZ_TRUE;
}

void *tdefl_compress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len,
                                 size_t *pOut_len, int flags) {
  tdefl_output_buffer out_buf;
  MZ_CLEAR_OBJ(out_buf);
  if (!pOut_len)
    return MZ_FALSE;
  else
    *pOut_len = 0;
  out_buf.m_expandable = MZ_TRUE;
  if (!tdefl_compress_mem_to_output(
          pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags))
    return NULL;
  *pOut_len = out_buf.m_size;
  return out_buf.m_pBuf;
}

size_t tdefl_compress_mem_to_mem(void *pOut_buf, size_t out_buf_len,
                                 const void *pSrc_buf, size_t src_buf_len,
                                 int flags) {
  tdefl_output_buffer out_buf;
  MZ_CLEAR_OBJ(out_buf);
  if (!pOut_buf)
    return 0;
  out_buf.m_pBuf = (mz_uint8 *)pOut_buf;
  out_buf.m_capacity = out_buf_len;
  if (!tdefl_compress_mem_to_output(
          pSrc_buf, src_buf_len, tdefl_output_buffer_putter, &out_buf, flags))
    return 0;
  return out_buf.m_size;
}

static const mz_uint s_tdefl_num_probes[11] = {0,   1,   6,   32,  16,  32,
                                               128, 256, 512, 768, 1500};

/* level may actually range from [0,10] (10 is a "hidden" max level, where we
 * want a bit more compression and it's fine if throughput to fall off a cliff
 * on some files). */
mz_uint tdefl_create_comp_flags_from_zip_params(int level, int window_bits,
                                                int strategy) {
  mz_uint comp_flags =
      s_tdefl_num_probes[(level >= 0) ? MZ_MIN(10, level) : MZ_DEFAULT_LEVEL] |
      ((level <= 3) ? TDEFL_GREEDY_PARSING_FLAG : 0);
  if (window_bits > 0)
    comp_flags |= TDEFL_WRITE_ZLIB_HEADER;

  if (!level)
    comp_flags |= TDEFL_FORCE_ALL_RAW_BLOCKS;
  else if (strategy == MZ_FILTERED)
    comp_flags |= TDEFL_FILTER_MATCHES;
  else if (strategy == MZ_HUFFMAN_ONLY)
    comp_flags &= ~TDEFL_MAX_PROBES_MASK;
  else if (strategy == MZ_FIXED)
    comp_flags |= TDEFL_FORCE_ALL_STATIC_BLOCKS;
  else if (strategy == MZ_RLE)
    comp_flags |= TDEFL_RLE_MATCHES;

  return comp_flags;
}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4204) /* nonstandard extension used : non-constant   \
                                   aggregate initializer (also supported by    \
                                   GNU C and C99, so no big deal) */
#endif

/* Simple PNG writer function by Alex Evans, 2011. Released into the public
 domain: https://gist.github.com/908299, more context at
 http://altdevblogaday.org/2011/04/06/a-smaller-jpg-encoder/.
 This is actually a modification of Alex's original code so PNG files generated
 by this function pass pngcheck. */
void *tdefl_write_image_to_png_file_in_memory_ex(const void *pImage, int w,
                                                 int h, int num_chans,
                                                 size_t *pLen_out,
                                                 mz_uint level, mz_bool flip) {
  /* Using a local copy of this array here in case MINIZ_NO_ZLIB_APIS was
   * defined. */
  static const mz_uint s_tdefl_png_num_probes[11] = {
      0, 1, 6, 32, 16, 32, 128, 256, 512, 768, 1500};
  tdefl_compressor *pComp =
      (tdefl_compressor *)MZ_MALLOC(sizeof(tdefl_compressor));
  tdefl_output_buffer out_buf;
  int i, bpl = w * num_chans, y, z;
  mz_uint32 c;
  *pLen_out = 0;
  if (!pComp)
    return NULL;
  MZ_CLEAR_OBJ(out_buf);
  out_buf.m_expandable = MZ_TRUE;
  out_buf.m_capacity = 57 + MZ_MAX(64, (1 + bpl) * h);
  if (NULL == (out_buf.m_pBuf = (mz_uint8 *)MZ_MALLOC(out_buf.m_capacity))) {
    MZ_FREE(pComp);
    return NULL;
  }
  /* write dummy header */
  for (z = 41; z; --z)
    tdefl_output_buffer_putter(&z, 1, &out_buf);
  /* compress image data */
  tdefl_init(pComp, tdefl_output_buffer_putter, &out_buf,
             s_tdefl_png_num_probes[MZ_MIN(10, level)] |
                 TDEFL_WRITE_ZLIB_HEADER);
  for (y = 0; y < h; ++y) {
    tdefl_compress_buffer(pComp, &z, 1, TDEFL_NO_FLUSH);
    tdefl_compress_buffer(pComp,
                          (mz_uint8 *)pImage + (flip ? (h - 1 - y) : y) * bpl,
                          bpl, TDEFL_NO_FLUSH);
  }
  if (tdefl_compress_buffer(pComp, NULL, 0, TDEFL_FINISH) !=
      TDEFL_STATUS_DONE) {
    MZ_FREE(pComp);
    MZ_FREE(out_buf.m_pBuf);
    return NULL;
  }
  /* write real header */
  *pLen_out = out_buf.m_size - 41;
  {
    static const mz_uint8 chans[] = {0x00, 0x00, 0x04, 0x02, 0x06};
    mz_uint8 pnghdr[41] = {0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00,
                           0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x49, 0x44, 0x41, 0x54};
    pnghdr[18] = (mz_uint8)(w >> 8);
    pnghdr[19] = (mz_uint8)w;
    pnghdr[22] = (mz_uint8)(h >> 8);
    pnghdr[23] = (mz_uint8)h;
    pnghdr[25] = chans[num_chans];
    pnghdr[33] = (mz_uint8)(*pLen_out >> 24);
    pnghdr[34] = (mz_uint8)(*pLen_out >> 16);
    pnghdr[35] = (mz_uint8)(*pLen_out >> 8);
    pnghdr[36] = (mz_uint8)*pLen_out;
    c = (mz_uint32)mz_crc32(MZ_CRC32_INIT, pnghdr + 12, 17);
    for (i = 0; i < 4; ++i, c <<= 8)
      ((mz_uint8 *)(pnghdr + 29))[i] = (mz_uint8)(c >> 24);
    memcpy(out_buf.m_pBuf, pnghdr, 41);
  }
  /* write footer (IDAT CRC-32, followed by IEND chunk) */
  if (!tdefl_output_buffer_putter(
          "\0\0\0\0\0\0\0\0\x49\x45\x4e\x44\xae\x42\x60\x82", 16, &out_buf)) {
    *pLen_out = 0;
    MZ_FREE(pComp);
    MZ_FREE(out_buf.m_pBuf);
    return NULL;
  }
  c = (mz_uint32)mz_crc32(MZ_CRC32_INIT, out_buf.m_pBuf + 41 - 4,
                          *pLen_out + 4);
  for (i = 0; i < 4; ++i, c <<= 8)
    (out_buf.m_pBuf + out_buf.m_size - 16)[i] = (mz_uint8)(c >> 24);
  /* compute final size of file, grab compressed data buffer and return */
  *pLen_out += 57;
  MZ_FREE(pComp);
  return out_buf.m_pBuf;
}
void *tdefl_write_image_to_png_file_in_memory(const void *pImage, int w, int h,
                                              int num_chans, size_t *pLen_out) {
  /* Level 6 corresponds to TDEFL_DEFAULT_MAX_PROBES or MZ_DEFAULT_LEVEL (but we
   * can't depend on MZ_DEFAULT_LEVEL being available in case the zlib API's
   * where #defined out) */
  return tdefl_write_image_to_png_file_in_memory_ex(pImage, w, h, num_chans,
                                                    pLen_out, 6, MZ_FALSE);
}

#ifndef MINIZ_NO_MALLOC
/* Allocate the tdefl_compressor and tinfl_decompressor structures in C so that
 */
/* non-C language bindings to tdefL_ and tinfl_ API don't need to worry about */
/* structure size and allocation mechanism. */
tdefl_compressor *tdefl_compressor_alloc() {
  return (tdefl_compressor *)MZ_MALLOC(sizeof(tdefl_compressor));
}

void tdefl_compressor_free(tdefl_compressor *pComp) { MZ_FREE(pComp); }
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef __cplusplus
}
#endif
/**************************************************************************
 *
 * Copyright 2013-2014 RAD Game Tools and Valve Software
 * Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- Low-level Decompression (completely independent from all
 * compression API's) */

#define TINFL_MEMCPY(d, s, l) memcpy(d, s, l)
#define TINFL_MEMSET(p, c, l) memset(p, c, l)

#define TINFL_CR_BEGIN                                                         \
  switch (r->m_state) {                                                        \
  case 0:
#define TINFL_CR_RETURN(state_index, result)                                   \
  do {                                                                         \
    status = result;                                                           \
    r->m_state = state_index;                                                  \
    goto common_exit;                                                          \
  case state_index:;                                                           \
  }                                                                            \
  MZ_MACRO_END
#define TINFL_CR_RETURN_FOREVER(state_index, result)                           \
  do {                                                                         \
    for (;;) {                                                                 \
      TINFL_CR_RETURN(state_index, result);                                    \
    }                                                                          \
  }                                                                            \
  MZ_MACRO_END
#define TINFL_CR_FINISH }

#define TINFL_GET_BYTE(state_index, c)                                         \
  do {                                                                         \
    while (pIn_buf_cur >= pIn_buf_end) {                                       \
      TINFL_CR_RETURN(state_index,                                             \
                      (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT)               \
                          ? TINFL_STATUS_NEEDS_MORE_INPUT                      \
                          : TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS);         \
    }                                                                          \
    c = *pIn_buf_cur++;                                                        \
  }                                                                            \
  MZ_MACRO_END

#define TINFL_NEED_BITS(state_index, n)                                        \
  do {                                                                         \
    mz_uint c;                                                                 \
    TINFL_GET_BYTE(state_index, c);                                            \
    bit_buf |= (((tinfl_bit_buf_t)c) << num_bits);                             \
    num_bits += 8;                                                             \
  } while (num_bits < (mz_uint)(n))
#define TINFL_SKIP_BITS(state_index, n)                                        \
  do {                                                                         \
    if (num_bits < (mz_uint)(n)) {                                             \
      TINFL_NEED_BITS(state_index, n);                                         \
    }                                                                          \
    bit_buf >>= (n);                                                           \
    num_bits -= (n);                                                           \
  }                                                                            \
  MZ_MACRO_END
#define TINFL_GET_BITS(state_index, b, n)                                      \
  do {                                                                         \
    if (num_bits < (mz_uint)(n)) {                                             \
      TINFL_NEED_BITS(state_index, n);                                         \
    }                                                                          \
    b = bit_buf & ((1 << (n)) - 1);                                            \
    bit_buf >>= (n);                                                           \
    num_bits -= (n);                                                           \
  }                                                                            \
  MZ_MACRO_END

/* TINFL_HUFF_BITBUF_FILL() is only used rarely, when the number of bytes
 * remaining in the input buffer falls below 2. */
/* It reads just enough bytes from the input stream that are needed to decode
 * the next Huffman code (and absolutely no more). It works by trying to fully
 * decode a */
/* Huffman code by using whatever bits are currently present in the bit buffer.
 * If this fails, it reads another byte, and tries again until it succeeds or
 * until the */
/* bit buffer contains >=15 bits (deflate's max. Huffman code size). */
#define TINFL_HUFF_BITBUF_FILL(state_index, pHuff)                             \
  do {                                                                         \
    temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)];         \
    if (temp >= 0) {                                                           \
      code_len = temp >> 9;                                                    \
      if ((code_len) && (num_bits >= code_len))                                \
        break;                                                                 \
    } else if (num_bits > TINFL_FAST_LOOKUP_BITS) {                            \
      code_len = TINFL_FAST_LOOKUP_BITS;                                       \
      do {                                                                     \
        temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)];         \
      } while ((temp < 0) && (num_bits >= (code_len + 1)));                    \
      if (temp >= 0)                                                           \
        break;                                                                 \
    }                                                                          \
    TINFL_GET_BYTE(state_index, c);                                            \
    bit_buf |= (((tinfl_bit_buf_t)c) << num_bits);                             \
    num_bits += 8;                                                             \
  } while (num_bits < 15);

/* TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex
 * than you would initially expect because the zlib API expects the decompressor
 * to never read */
/* beyond the final byte of the deflate stream. (In other words, when this macro
 * wants to read another byte from the input, it REALLY needs another byte in
 * order to fully */
/* decode the next Huffman code.) Handling this properly is particularly
 * important on raw deflate (non-zlib) streams, which aren't followed by a byte
 * aligned adler-32. */
/* The slow path is only executed at the very end of the input buffer. */
/* v1.16: The original macro handled the case at the very end of the passed-in
 * input buffer, but we also need to handle the case where the user passes in
 * 1+zillion bytes */
/* following the deflate data and our non-conservative read-ahead path won't
 * kick in here on this code. This is much trickier. */
#define TINFL_HUFF_DECODE(state_index, sym, pHuff)                             \
  do {                                                                         \
    int temp;                                                                  \
    mz_uint code_len, c;                                                       \
    if (num_bits < 15) {                                                       \
      if ((pIn_buf_end - pIn_buf_cur) < 2) {                                   \
        TINFL_HUFF_BITBUF_FILL(state_index, pHuff);                            \
      } else {                                                                 \
        bit_buf |= (((tinfl_bit_buf_t)pIn_buf_cur[0]) << num_bits) |           \
                   (((tinfl_bit_buf_t)pIn_buf_cur[1]) << (num_bits + 8));      \
        pIn_buf_cur += 2;                                                      \
        num_bits += 16;                                                        \
      }                                                                        \
    }                                                                          \
    if ((temp = (pHuff)->m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= \
        0)                                                                     \
      code_len = temp >> 9, temp &= 511;                                       \
    else {                                                                     \
      code_len = TINFL_FAST_LOOKUP_BITS;                                       \
      do {                                                                     \
        temp = (pHuff)->m_tree[~temp + ((bit_buf >> code_len++) & 1)];         \
      } while (temp < 0);                                                      \
    }                                                                          \
    sym = temp;                                                                \
    bit_buf >>= code_len;                                                      \
    num_bits -= code_len;                                                      \
  }                                                                            \
  MZ_MACRO_END

tinfl_status tinfl_decompress(tinfl_decompressor *r,
                              const mz_uint8 *pIn_buf_next,
                              size_t *pIn_buf_size, mz_uint8 *pOut_buf_start,
                              mz_uint8 *pOut_buf_next, size_t *pOut_buf_size,
                              const mz_uint32 decomp_flags) {
  static const int s_length_base[31] = {
      3,  4,  5,  6,  7,  8,  9,  10,  11,  13,  15,  17,  19,  23, 27, 31,
      35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0,  0};
  static const int s_length_extra[31] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
                                         1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4,
                                         4, 4, 5, 5, 5, 5, 0, 0, 0};
  static const int s_dist_base[32] = {
      1,    2,    3,    4,    5,    7,     9,     13,    17,  25,   33,
      49,   65,   97,   129,  193,  257,   385,   513,   769, 1025, 1537,
      2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0,   0};
  static const int s_dist_extra[32] = {0, 0, 0,  0,  1,  1,  2,  2,  3,  3,
                                       4, 4, 5,  5,  6,  6,  7,  7,  8,  8,
                                       9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
  static const mz_uint8 s_length_dezigzag[19] = {
      16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
  static const int s_min_table_sizes[3] = {257, 1, 4};

  tinfl_status status = TINFL_STATUS_FAILED;
  mz_uint32 num_bits, dist, counter, num_extra;
  tinfl_bit_buf_t bit_buf;
  const mz_uint8 *pIn_buf_cur = pIn_buf_next, *const pIn_buf_end =
                                                  pIn_buf_next + *pIn_buf_size;
  mz_uint8 *pOut_buf_cur = pOut_buf_next, *const pOut_buf_end =
                                              pOut_buf_next + *pOut_buf_size;
  size_t out_buf_size_mask =
             (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)
                 ? (size_t)-1
                 : ((pOut_buf_next - pOut_buf_start) + *pOut_buf_size) - 1,
         dist_from_out_buf_start;

  /* Ensure the output buffer's size is a power of 2, unless the output buffer
   * is large enough to hold the entire output file (in which case it doesn't
   * matter). */
  if (((out_buf_size_mask + 1) & out_buf_size_mask) ||
      (pOut_buf_next < pOut_buf_start)) {
    *pIn_buf_size = *pOut_buf_size = 0;
    return TINFL_STATUS_BAD_PARAM;
  }

  num_bits = r->m_num_bits;
  bit_buf = r->m_bit_buf;
  dist = r->m_dist;
  counter = r->m_counter;
  num_extra = r->m_num_extra;
  dist_from_out_buf_start = r->m_dist_from_out_buf_start;
  TINFL_CR_BEGIN

  bit_buf = num_bits = dist = counter = num_extra = r->m_zhdr0 = r->m_zhdr1 = 0;
  r->m_z_adler32 = r->m_check_adler32 = 1;
  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) {
    TINFL_GET_BYTE(1, r->m_zhdr0);
    TINFL_GET_BYTE(2, r->m_zhdr1);
    counter = (((r->m_zhdr0 * 256 + r->m_zhdr1) % 31 != 0) ||
               (r->m_zhdr1 & 32) || ((r->m_zhdr0 & 15) != 8));
    if (!(decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF))
      counter |= (((1U << (8U + (r->m_zhdr0 >> 4))) > 32768U) ||
                  ((out_buf_size_mask + 1) <
                   (size_t)(1U << (8U + (r->m_zhdr0 >> 4)))));
    if (counter) {
      TINFL_CR_RETURN_FOREVER(36, TINFL_STATUS_FAILED);
    }
  }

  do {
    TINFL_GET_BITS(3, r->m_final, 3);
    r->m_type = r->m_final >> 1;
    if (r->m_type == 0) {
      TINFL_SKIP_BITS(5, num_bits & 7);
      for (counter = 0; counter < 4; ++counter) {
        if (num_bits)
          TINFL_GET_BITS(6, r->m_raw_header[counter], 8);
        else
          TINFL_GET_BYTE(7, r->m_raw_header[counter]);
      }
      if ((counter = (r->m_raw_header[0] | (r->m_raw_header[1] << 8))) !=
          (mz_uint)(0xFFFF ^
                    (r->m_raw_header[2] | (r->m_raw_header[3] << 8)))) {
        TINFL_CR_RETURN_FOREVER(39, TINFL_STATUS_FAILED);
      }
      while ((counter) && (num_bits)) {
        TINFL_GET_BITS(51, dist, 8);
        while (pOut_buf_cur >= pOut_buf_end) {
          TINFL_CR_RETURN(52, TINFL_STATUS_HAS_MORE_OUTPUT);
        }
        *pOut_buf_cur++ = (mz_uint8)dist;
        counter--;
      }
      while (counter) {
        size_t n;
        while (pOut_buf_cur >= pOut_buf_end) {
          TINFL_CR_RETURN(9, TINFL_STATUS_HAS_MORE_OUTPUT);
        }
        while (pIn_buf_cur >= pIn_buf_end) {
          TINFL_CR_RETURN(38, (decomp_flags & TINFL_FLAG_HAS_MORE_INPUT)
                                  ? TINFL_STATUS_NEEDS_MORE_INPUT
                                  : TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS);
        }
        n = MZ_MIN(MZ_MIN((size_t)(pOut_buf_end - pOut_buf_cur),
                          (size_t)(pIn_buf_end - pIn_buf_cur)),
                   counter);
        TINFL_MEMCPY(pOut_buf_cur, pIn_buf_cur, n);
        pIn_buf_cur += n;
        pOut_buf_cur += n;
        counter -= (mz_uint)n;
      }
    } else if (r->m_type == 3) {
      TINFL_CR_RETURN_FOREVER(10, TINFL_STATUS_FAILED);
    } else {
      if (r->m_type == 1) {
        mz_uint8 *p = r->m_tables[0].m_code_size;
        mz_uint i;
        r->m_table_sizes[0] = 288;
        r->m_table_sizes[1] = 32;
        TINFL_MEMSET(r->m_tables[1].m_code_size, 5, 32);
        for (i = 0; i <= 143; ++i)
          *p++ = 8;
        for (; i <= 255; ++i)
          *p++ = 9;
        for (; i <= 279; ++i)
          *p++ = 7;
        for (; i <= 287; ++i)
          *p++ = 8;
      } else {
        for (counter = 0; counter < 3; counter++) {
          TINFL_GET_BITS(11, r->m_table_sizes[counter], "\05\05\04"[counter]);
          r->m_table_sizes[counter] += s_min_table_sizes[counter];
        }
        MZ_CLEAR_OBJ(r->m_tables[2].m_code_size);
        for (counter = 0; counter < r->m_table_sizes[2]; counter++) {
          mz_uint s;
          TINFL_GET_BITS(14, s, 3);
          r->m_tables[2].m_code_size[s_length_dezigzag[counter]] = (mz_uint8)s;
        }
        r->m_table_sizes[2] = 19;
      }
      for (; (int)r->m_type >= 0; r->m_type--) {
        int tree_next, tree_cur;
        tinfl_huff_table *pTable;
        mz_uint i, j, used_syms, total, sym_index, next_code[17],
            total_syms[16];
        pTable = &r->m_tables[r->m_type];
        MZ_CLEAR_OBJ(total_syms);
        MZ_CLEAR_OBJ(pTable->m_look_up);
        MZ_CLEAR_OBJ(pTable->m_tree);
        for (i = 0; i < r->m_table_sizes[r->m_type]; ++i)
          total_syms[pTable->m_code_size[i]]++;
        used_syms = 0, total = 0;
        next_code[0] = next_code[1] = 0;
        for (i = 1; i <= 15; ++i) {
          used_syms += total_syms[i];
          next_code[i + 1] = (total = ((total + total_syms[i]) << 1));
        }
        if ((65536 != total) && (used_syms > 1)) {
          TINFL_CR_RETURN_FOREVER(35, TINFL_STATUS_FAILED);
        }
        for (tree_next = -1, sym_index = 0;
             sym_index < r->m_table_sizes[r->m_type]; ++sym_index) {
          mz_uint rev_code = 0, l, cur_code,
                  code_size = pTable->m_code_size[sym_index];

          if (!code_size)
            continue;
          cur_code = next_code[code_size]++;
          for (l = code_size; l > 0; l--, cur_code >>= 1)
            rev_code = (rev_code << 1) | (cur_code & 1);
          if (code_size <= TINFL_FAST_LOOKUP_BITS) {
            mz_int16 k = (mz_int16)((code_size << 9) | sym_index);
            while (rev_code < TINFL_FAST_LOOKUP_SIZE) {
              pTable->m_look_up[rev_code] = k;
              rev_code += (1 << code_size);
            }
            continue;
          }
          if (0 ==
              (tree_cur = pTable->m_look_up[rev_code &
                                            (TINFL_FAST_LOOKUP_SIZE - 1)])) {
            pTable->m_look_up[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] =
                (mz_int16)tree_next;
            tree_cur = tree_next;
            tree_next -= 2;
          }
          rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
          for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--) {
            tree_cur -= ((rev_code >>= 1) & 1);
            if (!pTable->m_tree[-tree_cur - 1]) {
              pTable->m_tree[-tree_cur - 1] = (mz_int16)tree_next;
              tree_cur = tree_next;
              tree_next -= 2;
            } else
              tree_cur = pTable->m_tree[-tree_cur - 1];
          }
          tree_cur -= ((rev_code >>= 1) & 1);
          (void)rev_code; // unused
          pTable->m_tree[-tree_cur - 1] = (mz_int16)sym_index;
        }
        if (r->m_type == 2) {
          for (counter = 0;
               counter < (r->m_table_sizes[0] + r->m_table_sizes[1]);) {
            mz_uint s;
            TINFL_HUFF_DECODE(16, dist, &r->m_tables[2]);
            if (dist < 16) {
              r->m_len_codes[counter++] = (mz_uint8)dist;
              continue;
            }
            if ((dist == 16) && (!counter)) {
              TINFL_CR_RETURN_FOREVER(17, TINFL_STATUS_FAILED);
            }
            num_extra = "\02\03\07"[dist - 16];
            TINFL_GET_BITS(18, s, num_extra);
            s += "\03\03\013"[dist - 16];
            TINFL_MEMSET(r->m_len_codes + counter,
                         (dist == 16) ? r->m_len_codes[counter - 1] : 0, s);
            counter += s;
          }
          if ((r->m_table_sizes[0] + r->m_table_sizes[1]) != counter) {
            TINFL_CR_RETURN_FOREVER(21, TINFL_STATUS_FAILED);
          }
          TINFL_MEMCPY(r->m_tables[0].m_code_size, r->m_len_codes,
                       r->m_table_sizes[0]);
          TINFL_MEMCPY(r->m_tables[1].m_code_size,
                       r->m_len_codes + r->m_table_sizes[0],
                       r->m_table_sizes[1]);
        }
      }
      for (;;) {
        mz_uint8 *pSrc;
        for (;;) {
          if (((pIn_buf_end - pIn_buf_cur) < 4) ||
              ((pOut_buf_end - pOut_buf_cur) < 2)) {
            TINFL_HUFF_DECODE(23, counter, &r->m_tables[0]);
            if (counter >= 256)
              break;
            while (pOut_buf_cur >= pOut_buf_end) {
              TINFL_CR_RETURN(24, TINFL_STATUS_HAS_MORE_OUTPUT);
            }
            *pOut_buf_cur++ = (mz_uint8)counter;
          } else {
            int sym2;
            mz_uint code_len;
#if TINFL_USE_64BIT_BITBUF
            if (num_bits < 30) {
              bit_buf |=
                  (((tinfl_bit_buf_t)MZ_READ_LE32(pIn_buf_cur)) << num_bits);
              pIn_buf_cur += 4;
              num_bits += 32;
            }
#else
            if (num_bits < 15) {
              bit_buf |=
                  (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits);
              pIn_buf_cur += 2;
              num_bits += 16;
            }
#endif
            if ((sym2 =
                     r->m_tables[0]
                         .m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >=
                0)
              code_len = sym2 >> 9;
            else {
              code_len = TINFL_FAST_LOOKUP_BITS;
              do {
                sym2 = r->m_tables[0]
                           .m_tree[~sym2 + ((bit_buf >> code_len++) & 1)];
              } while (sym2 < 0);
            }
            counter = sym2;
            bit_buf >>= code_len;
            num_bits -= code_len;
            if (counter & 256)
              break;

#if !TINFL_USE_64BIT_BITBUF
            if (num_bits < 15) {
              bit_buf |=
                  (((tinfl_bit_buf_t)MZ_READ_LE16(pIn_buf_cur)) << num_bits);
              pIn_buf_cur += 2;
              num_bits += 16;
            }
#endif
            if ((sym2 =
                     r->m_tables[0]
                         .m_look_up[bit_buf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >=
                0)
              code_len = sym2 >> 9;
            else {
              code_len = TINFL_FAST_LOOKUP_BITS;
              do {
                sym2 = r->m_tables[0]
                           .m_tree[~sym2 + ((bit_buf >> code_len++) & 1)];
              } while (sym2 < 0);
            }
            bit_buf >>= code_len;
            num_bits -= code_len;

            pOut_buf_cur[0] = (mz_uint8)counter;
            if (sym2 & 256) {
              pOut_buf_cur++;
              counter = sym2;
              break;
            }
            pOut_buf_cur[1] = (mz_uint8)sym2;
            pOut_buf_cur += 2;
          }
        }
        if ((counter &= 511) == 256)
          break;

        num_extra = s_length_extra[counter - 257];
        counter = s_length_base[counter - 257];
        if (num_extra) {
          mz_uint extra_bits;
          TINFL_GET_BITS(25, extra_bits, num_extra);
          counter += extra_bits;
        }

        TINFL_HUFF_DECODE(26, dist, &r->m_tables[1]);
        num_extra = s_dist_extra[dist];
        dist = s_dist_base[dist];
        if (num_extra) {
          mz_uint extra_bits;
          TINFL_GET_BITS(27, extra_bits, num_extra);
          dist += extra_bits;
        }

        dist_from_out_buf_start = pOut_buf_cur - pOut_buf_start;
        if ((dist == 0 || dist > dist_from_out_buf_start ||
             dist_from_out_buf_start == 0) &&
            (decomp_flags & TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)) {
          TINFL_CR_RETURN_FOREVER(37, TINFL_STATUS_FAILED);
        }

        pSrc = pOut_buf_start +
               ((dist_from_out_buf_start - dist) & out_buf_size_mask);

        if ((MZ_MAX(pOut_buf_cur, pSrc) + counter) > pOut_buf_end) {
          while (counter--) {
            while (pOut_buf_cur >= pOut_buf_end) {
              TINFL_CR_RETURN(53, TINFL_STATUS_HAS_MORE_OUTPUT);
            }
            *pOut_buf_cur++ =
                pOut_buf_start[(dist_from_out_buf_start++ - dist) &
                               out_buf_size_mask];
          }
          continue;
        }
#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
        else if ((counter >= 9) && (counter <= dist)) {
          const mz_uint8 *pSrc_end = pSrc + (counter & ~7);
          do {
#ifdef MINIZ_UNALIGNED_USE_MEMCPY
            memcpy(pOut_buf_cur, pSrc, sizeof(mz_uint32) * 2);
#else
            ((mz_uint32 *)pOut_buf_cur)[0] = ((const mz_uint32 *)pSrc)[0];
            ((mz_uint32 *)pOut_buf_cur)[1] = ((const mz_uint32 *)pSrc)[1];
#endif
            pOut_buf_cur += 8;
          } while ((pSrc += 8) < pSrc_end);
          if ((counter &= 7) < 3) {
            if (counter) {
              pOut_buf_cur[0] = pSrc[0];
              if (counter > 1)
                pOut_buf_cur[1] = pSrc[1];
              pOut_buf_cur += counter;
            }
            continue;
          }
        }
#endif
        while (counter > 2) {
          pOut_buf_cur[0] = pSrc[0];
          pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur[2] = pSrc[2];
          pOut_buf_cur += 3;
          pSrc += 3;
          counter -= 3;
        }
        if (counter > 0) {
          pOut_buf_cur[0] = pSrc[0];
          if (counter > 1)
            pOut_buf_cur[1] = pSrc[1];
          pOut_buf_cur += counter;
        }
      }
    }
  } while (!(r->m_final & 1));

  /* Ensure byte alignment and put back any bytes from the bitbuf if we've
   * looked ahead too far on gzip, or other Deflate streams followed by
   * arbitrary data. */
  /* I'm being super conservative here. A number of simplifications can be made
   * to the byte alignment part, and the Adler32 check shouldn't ever need to
   * worry about reading from the bitbuf now. */
  TINFL_SKIP_BITS(32, num_bits & 7);
  while ((pIn_buf_cur > pIn_buf_next) && (num_bits >= 8)) {
    --pIn_buf_cur;
    num_bits -= 8;
  }
  bit_buf &= (tinfl_bit_buf_t)((((mz_uint64)1) << num_bits) - (mz_uint64)1);
  MZ_ASSERT(!num_bits); /* if this assert fires then we've read beyond the end
                           of non-deflate/zlib streams with following data (such
                           as gzip streams). */

  if (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) {
    for (counter = 0; counter < 4; ++counter) {
      mz_uint s;
      if (num_bits)
        TINFL_GET_BITS(41, s, 8);
      else
        TINFL_GET_BYTE(42, s);
      r->m_z_adler32 = (r->m_z_adler32 << 8) | s;
    }
  }
  TINFL_CR_RETURN_FOREVER(34, TINFL_STATUS_DONE);

  TINFL_CR_FINISH

common_exit:
  /* As long as we aren't telling the caller that we NEED more input to make
   * forward progress: */
  /* Put back any bytes from the bitbuf in case we've looked ahead too far on
   * gzip, or other Deflate streams followed by arbitrary data. */
  /* We need to be very careful here to NOT push back any bytes we definitely
   * know we need to make forward progress, though, or we'll lock the caller up
   * into an inf loop. */
  if ((status != TINFL_STATUS_NEEDS_MORE_INPUT) &&
      (status != TINFL_STATUS_FAILED_CANNOT_MAKE_PROGRESS)) {
    while ((pIn_buf_cur > pIn_buf_next) && (num_bits >= 8)) {
      --pIn_buf_cur;
      num_bits -= 8;
    }
  }
  r->m_num_bits = num_bits;
  r->m_bit_buf =
      bit_buf & (tinfl_bit_buf_t)((((mz_uint64)1) << num_bits) - (mz_uint64)1);
  r->m_dist = dist;
  r->m_counter = counter;
  r->m_num_extra = num_extra;
  r->m_dist_from_out_buf_start = dist_from_out_buf_start;
  *pIn_buf_size = pIn_buf_cur - pIn_buf_next;
  *pOut_buf_size = pOut_buf_cur - pOut_buf_next;
  if ((decomp_flags &
       (TINFL_FLAG_PARSE_ZLIB_HEADER | TINFL_FLAG_COMPUTE_ADLER32)) &&
      (status >= 0)) {
    const mz_uint8 *ptr = pOut_buf_next;
    size_t buf_len = *pOut_buf_size;
    mz_uint32 i, s1 = r->m_check_adler32 & 0xffff,
                 s2 = r->m_check_adler32 >> 16;
    size_t block_len = buf_len % 5552;
    while (buf_len) {
      for (i = 0; i + 7 < block_len; i += 8, ptr += 8) {
        s1 += ptr[0], s2 += s1;
        s1 += ptr[1], s2 += s1;
        s1 += ptr[2], s2 += s1;
        s1 += ptr[3], s2 += s1;
        s1 += ptr[4], s2 += s1;
        s1 += ptr[5], s2 += s1;
        s1 += ptr[6], s2 += s1;
        s1 += ptr[7], s2 += s1;
      }
      for (; i < block_len; ++i)
        s1 += *ptr++, s2 += s1;
      s1 %= 65521U, s2 %= 65521U;
      buf_len -= block_len;
      block_len = 5552;
    }
    r->m_check_adler32 = (s2 << 16) + s1;
    if ((status == TINFL_STATUS_DONE) &&
        (decomp_flags & TINFL_FLAG_PARSE_ZLIB_HEADER) &&
        (r->m_check_adler32 != r->m_z_adler32))
      status = TINFL_STATUS_ADLER32_MISMATCH;
  }
  return status;
}

/* Higher level helper functions. */
void *tinfl_decompress_mem_to_heap(const void *pSrc_buf, size_t src_buf_len,
                                   size_t *pOut_len, int flags) {
  tinfl_decompressor decomp;
  void *pBuf = NULL, *pNew_buf;
  size_t src_buf_ofs = 0, out_buf_capacity = 0;
  *pOut_len = 0;
  tinfl_init(&decomp);
  for (;;) {
    size_t src_buf_size = src_buf_len - src_buf_ofs,
           dst_buf_size = out_buf_capacity - *pOut_len, new_out_buf_capacity;
    tinfl_status status = tinfl_decompress(
        &decomp, (const mz_uint8 *)pSrc_buf + src_buf_ofs, &src_buf_size,
        (mz_uint8 *)pBuf, pBuf ? (mz_uint8 *)pBuf + *pOut_len : NULL,
        &dst_buf_size,
        (flags & ~TINFL_FLAG_HAS_MORE_INPUT) |
            TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
    if ((status < 0) || (status == TINFL_STATUS_NEEDS_MORE_INPUT)) {
      MZ_FREE(pBuf);
      *pOut_len = 0;
      return NULL;
    }
    src_buf_ofs += src_buf_size;
    *pOut_len += dst_buf_size;
    if (status == TINFL_STATUS_DONE)
      break;
    new_out_buf_capacity = out_buf_capacity * 2;
    if (new_out_buf_capacity < 128)
      new_out_buf_capacity = 128;
    pNew_buf = MZ_REALLOC(pBuf, new_out_buf_capacity);
    if (!pNew_buf) {
      MZ_FREE(pBuf);
      *pOut_len = 0;
      return NULL;
    }
    pBuf = pNew_buf;
    out_buf_capacity = new_out_buf_capacity;
  }
  return pBuf;
}

size_t tinfl_decompress_mem_to_mem(void *pOut_buf, size_t out_buf_len,
                                   const void *pSrc_buf, size_t src_buf_len,
                                   int flags) {
  tinfl_decompressor decomp;
  tinfl_status status;
  tinfl_init(&decomp);
  status =
      tinfl_decompress(&decomp, (const mz_uint8 *)pSrc_buf, &src_buf_len,
                       (mz_uint8 *)pOut_buf, (mz_uint8 *)pOut_buf, &out_buf_len,
                       (flags & ~TINFL_FLAG_HAS_MORE_INPUT) |
                           TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF);
  return (status != TINFL_STATUS_DONE) ? TINFL_DECOMPRESS_MEM_TO_MEM_FAILED
                                       : out_buf_len;
}

int tinfl_decompress_mem_to_callback(const void *pIn_buf, size_t *pIn_buf_size,
                                     tinfl_put_buf_func_ptr pPut_buf_func,
                                     void *pPut_buf_user, int flags) {
  int result = 0;
  tinfl_decompressor decomp;
  mz_uint8 *pDict = (mz_uint8 *)MZ_MALLOC(TINFL_LZ_DICT_SIZE);
  size_t in_buf_ofs = 0, dict_ofs = 0;
  if (!pDict)
    return TINFL_STATUS_FAILED;
  tinfl_init(&decomp);
  for (;;) {
    size_t in_buf_size = *pIn_buf_size - in_buf_ofs,
           dst_buf_size = TINFL_LZ_DICT_SIZE - dict_ofs;
    tinfl_status status =
        tinfl_decompress(&decomp, (const mz_uint8 *)pIn_buf + in_buf_ofs,
                         &in_buf_size, pDict, pDict + dict_ofs, &dst_buf_size,
                         (flags & ~(TINFL_FLAG_HAS_MORE_INPUT |
                                    TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF)));
    in_buf_ofs += in_buf_size;
    if ((dst_buf_size) &&
        (!(*pPut_buf_func)(pDict + dict_ofs, (int)dst_buf_size, pPut_buf_user)))
      break;
    if (status != TINFL_STATUS_HAS_MORE_OUTPUT) {
      result = (status == TINFL_STATUS_DONE);
      break;
    }
    dict_ofs = (dict_ofs + dst_buf_size) & (TINFL_LZ_DICT_SIZE - 1);
  }
  MZ_FREE(pDict);
  *pIn_buf_size = in_buf_ofs;
  return result;
}

#ifndef MINIZ_NO_MALLOC
tinfl_decompressor *tinfl_decompressor_alloc() {
  tinfl_decompressor *pDecomp =
      (tinfl_decompressor *)MZ_MALLOC(sizeof(tinfl_decompressor));
  if (pDecomp)
    tinfl_init(pDecomp);
  return pDecomp;
}

void tinfl_decompressor_free(tinfl_decompressor *pDecomp) { MZ_FREE(pDecomp); }
#endif

#ifdef __cplusplus
}
#endif
/**************************************************************************
 *
 * Copyright 2013-2014 RAD Game Tools and Valve Software
 * Copyright 2010-2014 Rich Geldreich and Tenacious Software LLC
 * Copyright 2016 Martin Raiber
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#ifndef MINIZ_NO_ARCHIVE_APIS

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------- .ZIP archive reading */

#ifdef MINIZ_NO_STDIO
#define MZ_FILE void *
#else
#include <sys/stat.h>

#if defined(_MSC_VER)
#include <windows.h>
#ifndef MINIZ_NO_TIME
#include <sys/utime.h>
#endif
static wchar_t *str2wstr(const char *str) {
  size_t len = strlen(str) + 1;
  wchar_t *wstr = (wchar_t *)malloc(len * sizeof(wchar_t));
  MultiByteToWideChar(CP_UTF8, 0, str, (int)(len * sizeof(char)), wstr,
                      (int)len);
  return wstr;
}

static FILE *mz_fopen(const char *pFilename, const char *pMode) {
  FILE *pFile = NULL;
  wchar_t *wFilename = str2wstr(pFilename);
  wchar_t *wMode = str2wstr(pMode);

#ifdef ZIP_ENABLE_SHARABLE_FILE_OPEN
  pFile = _wfopen(wFilename, wMode);
#else
  _wfopen_s(&pFile, wFilename, wMode);
#endif
  free(wFilename);
  free(wMode);

  return pFile;
}

static FILE *mz_freopen(const char *pPath, const char *pMode, FILE *pStream) {
  FILE *pFile = NULL;
  int res = 0;

  wchar_t *wPath = str2wstr(pPath);
  wchar_t *wMode = str2wstr(pMode);

#ifdef ZIP_ENABLE_SHARABLE_FILE_OPEN
  pFile = _wfreopen(wPath, wMode, pStream);
#else
  res = _wfreopen_s(&pFile, wPath, wMode, pStream);
#endif

  free(wPath);
  free(wMode);

#ifndef ZIP_ENABLE_SHARABLE_FILE_OPEN
  if (res) {
    return NULL;
  }
#endif

  return pFile;
}

static int mz_stat(const char *pPath, struct _stat64 *buffer) {
  wchar_t *wPath = str2wstr(pPath);
  int res = _wstat64(wPath, buffer);

  free(wPath);

  return res;
}

static int mz_mkdir(const char *pDirname) {
  wchar_t *wDirname = str2wstr(pDirname);
  int res = _wmkdir(wDirname);

  free(wDirname);

  return res;
}

#define MZ_FOPEN mz_fopen
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 _ftelli64
#define MZ_FSEEK64 _fseeki64
#define MZ_FILE_STAT_STRUCT _stat64
#define MZ_FILE_STAT mz_stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN mz_freopen
#define MZ_DELETE_FILE remove
#define MZ_MKDIR(d) mz_mkdir(d)

#elif defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#ifndef MINIZ_NO_TIME
#include <sys/utime.h>
#endif

#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftell
#define MZ_FSEEK64 fseek
#define MZ_FILE_STAT_STRUCT stat
#define MZ_FILE_STAT stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
#define MZ_DELETE_FILE remove
#define MZ_MKDIR(d) _mkdir(d)

#elif defined(__TINYC__)
#ifndef MINIZ_NO_TIME
#include <sys/utime.h>
#endif

#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftell
#define MZ_FSEEK64 fseek
#define MZ_FILE_STAT_STRUCT stat
#define MZ_FILE_STAT stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
#define MZ_DELETE_FILE remove
#if defined(_WIN32) || defined(_WIN64)
#define MZ_MKDIR(d) _mkdir(d)
#else
#define MZ_MKDIR(d) mkdir(d, 0755)
#endif

#elif defined(__USE_LARGEFILE64) /* gcc, clang */
#ifndef MINIZ_NO_TIME
#include <utime.h>
#endif

#define MZ_FOPEN(f, m) fopen64(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftello64
#define MZ_FSEEK64 fseeko64
#define MZ_FILE_STAT_STRUCT stat64
#define MZ_FILE_STAT stat64
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(p, m, s) freopen64(p, m, s)
#define MZ_DELETE_FILE remove
#define MZ_MKDIR(d) mkdir(d, 0755)

#elif defined(__APPLE__)
#ifndef MINIZ_NO_TIME
#include <utime.h>
#endif

#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#define MZ_FTELL64 ftello
#define MZ_FSEEK64 fseeko
#define MZ_FILE_STAT_STRUCT stat
#define MZ_FILE_STAT stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(p, m, s) freopen(p, m, s)
#define MZ_DELETE_FILE remove
#define MZ_MKDIR(d) mkdir(d, 0755)

#else
/*#pragma message(                                                               \
//    "Using fopen, ftello, fseeko, stat() etc. path for file I/O - this path may not support large files.")
*/
#ifndef MINIZ_NO_TIME
#include <utime.h>
#endif

#define MZ_FOPEN(f, m) fopen(f, m)
#define MZ_FCLOSE fclose
#define MZ_FREAD fread
#define MZ_FWRITE fwrite
#ifdef __STRICT_ANSI__
#define MZ_FTELL64 ftell
#define MZ_FSEEK64 fseek
#else
#define MZ_FTELL64 ftello
#define MZ_FSEEK64 fseeko
#endif
#define MZ_FILE_STAT_STRUCT stat
#define MZ_FILE_STAT stat
#define MZ_FFLUSH fflush
#define MZ_FREOPEN(f, m, s) freopen(f, m, s)
#define MZ_DELETE_FILE remove
#define MZ_MKDIR(d) mkdir(d, 0755)

#endif /* #ifdef _MSC_VER */
#endif /* #ifdef MINIZ_NO_STDIO */

#ifndef CHMOD
// Upon successful completion, a value of 0 is returned.
// Otherwise, a value of -1 is returned and errno is set to indicate the error.
// int chmod(const char *path, mode_t mode);
#define CHMOD(f, m) chmod(f, m)
#endif

#define MZ_TOLOWER(c) ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c))

/* Various ZIP archive enums. To completely avoid cross platform compiler
 * alignment and platform endian issues, miniz.c doesn't use structs for any of
 * this stuff. */
enum {
  /* ZIP archive identifiers and record sizes */
  MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06054b50,
  MZ_ZIP_CENTRAL_DIR_HEADER_SIG = 0x02014b50,
  MZ_ZIP_LOCAL_DIR_HEADER_SIG = 0x04034b50,
  MZ_ZIP_LOCAL_DIR_HEADER_SIZE = 30,
  MZ_ZIP_CENTRAL_DIR_HEADER_SIZE = 46,
  MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE = 22,

  /* ZIP64 archive identifier and record sizes */
  MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG = 0x06064b50,
  MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG = 0x07064b50,
  MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE = 56,
  MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE = 20,
  MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID = 0x0001,
  MZ_ZIP_DATA_DESCRIPTOR_ID = 0x08074b50,
  MZ_ZIP_DATA_DESCRIPTER_SIZE64 = 24,
  MZ_ZIP_DATA_DESCRIPTER_SIZE32 = 16,

  /* Central directory header record offsets */
  MZ_ZIP_CDH_SIG_OFS = 0,
  MZ_ZIP_CDH_VERSION_MADE_BY_OFS = 4,
  MZ_ZIP_CDH_VERSION_NEEDED_OFS = 6,
  MZ_ZIP_CDH_BIT_FLAG_OFS = 8,
  MZ_ZIP_CDH_METHOD_OFS = 10,
  MZ_ZIP_CDH_FILE_TIME_OFS = 12,
  MZ_ZIP_CDH_FILE_DATE_OFS = 14,
  MZ_ZIP_CDH_CRC32_OFS = 16,
  MZ_ZIP_CDH_COMPRESSED_SIZE_OFS = 20,
  MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS = 24,
  MZ_ZIP_CDH_FILENAME_LEN_OFS = 28,
  MZ_ZIP_CDH_EXTRA_LEN_OFS = 30,
  MZ_ZIP_CDH_COMMENT_LEN_OFS = 32,
  MZ_ZIP_CDH_DISK_START_OFS = 34,
  MZ_ZIP_CDH_INTERNAL_ATTR_OFS = 36,
  MZ_ZIP_CDH_EXTERNAL_ATTR_OFS = 38,
  MZ_ZIP_CDH_LOCAL_HEADER_OFS = 42,

  /* Local directory header offsets */
  MZ_ZIP_LDH_SIG_OFS = 0,
  MZ_ZIP_LDH_VERSION_NEEDED_OFS = 4,
  MZ_ZIP_LDH_BIT_FLAG_OFS = 6,
  MZ_ZIP_LDH_METHOD_OFS = 8,
  MZ_ZIP_LDH_FILE_TIME_OFS = 10,
  MZ_ZIP_LDH_FILE_DATE_OFS = 12,
  MZ_ZIP_LDH_CRC32_OFS = 14,
  MZ_ZIP_LDH_COMPRESSED_SIZE_OFS = 18,
  MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS = 22,
  MZ_ZIP_LDH_FILENAME_LEN_OFS = 26,
  MZ_ZIP_LDH_EXTRA_LEN_OFS = 28,
  MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR = 1 << 3,

  /* End of central directory offsets */
  MZ_ZIP_ECDH_SIG_OFS = 0,
  MZ_ZIP_ECDH_NUM_THIS_DISK_OFS = 4,
  MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS = 6,
  MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 8,
  MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS = 10,
  MZ_ZIP_ECDH_CDIR_SIZE_OFS = 12,
  MZ_ZIP_ECDH_CDIR_OFS_OFS = 16,
  MZ_ZIP_ECDH_COMMENT_SIZE_OFS = 20,

  /* ZIP64 End of central directory locator offsets */
  MZ_ZIP64_ECDL_SIG_OFS = 0,                    /* 4 bytes */
  MZ_ZIP64_ECDL_NUM_DISK_CDIR_OFS = 4,          /* 4 bytes */
  MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS = 8,  /* 8 bytes */
  MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS = 16, /* 4 bytes */

  /* ZIP64 End of central directory header offsets */
  MZ_ZIP64_ECDH_SIG_OFS = 0,                       /* 4 bytes */
  MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS = 4,            /* 8 bytes */
  MZ_ZIP64_ECDH_VERSION_MADE_BY_OFS = 12,          /* 2 bytes */
  MZ_ZIP64_ECDH_VERSION_NEEDED_OFS = 14,           /* 2 bytes */
  MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS = 16,            /* 4 bytes */
  MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS = 20,            /* 4 bytes */
  MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS = 24, /* 8 bytes */
  MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS = 32,       /* 8 bytes */
  MZ_ZIP64_ECDH_CDIR_SIZE_OFS = 40,                /* 8 bytes */
  MZ_ZIP64_ECDH_CDIR_OFS_OFS = 48,                 /* 8 bytes */
  MZ_ZIP_VERSION_MADE_BY_DOS_FILESYSTEM_ID = 0,
  MZ_ZIP_DOS_DIR_ATTRIBUTE_BITFLAG = 0x10,
  MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED = 1,
  MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG = 32,
  MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION = 64,
  MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_LOCAL_DIR_IS_MASKED = 8192,
  MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8 = 1 << 11
};

typedef struct {
  void *m_p;
  size_t m_size, m_capacity;
  mz_uint m_element_size;
} mz_zip_array;

struct mz_zip_internal_state_tag {
  mz_zip_array m_central_dir;
  mz_zip_array m_central_dir_offsets;
  mz_zip_array m_sorted_central_dir_offsets;

  /* The flags passed in when the archive is initially opened. */
  uint32_t m_init_flags;

  /* MZ_TRUE if the archive has a zip64 end of central directory headers, etc.
   */
  mz_bool m_zip64;

  /* MZ_TRUE if we found zip64 extended info in the central directory (m_zip64
   * will also be slammed to true too, even if we didn't find a zip64 end of
   * central dir header, etc.) */
  mz_bool m_zip64_has_extended_info_fields;

  /* These fields are used by the file, FILE, memory, and memory/heap read/write
   * helpers. */
  MZ_FILE *m_pFile;
  mz_uint64 m_file_archive_start_ofs;

  void *m_pMem;
  size_t m_mem_size;
  size_t m_mem_capacity;
};

#define MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(array_ptr, element_size)                 \
  (array_ptr)->m_element_size = element_size

#if defined(DEBUG) || defined(_DEBUG)
static MZ_FORCEINLINE mz_uint
mz_zip_array_range_check(const mz_zip_array *pArray, mz_uint index) {
  MZ_ASSERT(index < pArray->m_size);
  return index;
}
#define MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index)                   \
  ((element_type *)((array_ptr)                                                \
                        ->m_p))[mz_zip_array_range_check(array_ptr, index)]
#else
#define MZ_ZIP_ARRAY_ELEMENT(array_ptr, element_type, index)                   \
  ((element_type *)((array_ptr)->m_p))[index]
#endif

static MZ_FORCEINLINE void mz_zip_array_init(mz_zip_array *pArray,
                                             mz_uint32 element_size) {
  memset(pArray, 0, sizeof(mz_zip_array));
  pArray->m_element_size = element_size;
}

static MZ_FORCEINLINE void mz_zip_array_clear(mz_zip_archive *pZip,
                                              mz_zip_array *pArray) {
  pZip->m_pFree(pZip->m_pAlloc_opaque, pArray->m_p);
  memset(pArray, 0, sizeof(mz_zip_array));
}

static mz_bool mz_zip_array_ensure_capacity(mz_zip_archive *pZip,
                                            mz_zip_array *pArray,
                                            size_t min_new_capacity,
                                            mz_uint growing) {
  void *pNew_p;
  size_t new_capacity = min_new_capacity;
  MZ_ASSERT(pArray->m_element_size);
  if (pArray->m_capacity >= min_new_capacity)
    return MZ_TRUE;
  if (growing) {
    new_capacity = MZ_MAX(1, pArray->m_capacity);
    while (new_capacity < min_new_capacity)
      new_capacity *= 2;
  }
  if (NULL == (pNew_p = pZip->m_pRealloc(pZip->m_pAlloc_opaque, pArray->m_p,
                                         pArray->m_element_size, new_capacity)))
    return MZ_FALSE;
  pArray->m_p = pNew_p;
  pArray->m_capacity = new_capacity;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_reserve(mz_zip_archive *pZip,
                                                   mz_zip_array *pArray,
                                                   size_t new_capacity,
                                                   mz_uint growing) {
  if (new_capacity > pArray->m_capacity) {
    if (!mz_zip_array_ensure_capacity(pZip, pArray, new_capacity, growing))
      return MZ_FALSE;
  }
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_resize(mz_zip_archive *pZip,
                                                  mz_zip_array *pArray,
                                                  size_t new_size,
                                                  mz_uint growing) {
  if (new_size > pArray->m_capacity) {
    if (!mz_zip_array_ensure_capacity(pZip, pArray, new_size, growing))
      return MZ_FALSE;
  }
  pArray->m_size = new_size;
  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_array_ensure_room(mz_zip_archive *pZip,
                                                       mz_zip_array *pArray,
                                                       size_t n) {
  return mz_zip_array_reserve(pZip, pArray, pArray->m_size + n, MZ_TRUE);
}

static MZ_FORCEINLINE mz_bool mz_zip_array_push_back(mz_zip_archive *pZip,
                                                     mz_zip_array *pArray,
                                                     const void *pElements,
                                                     size_t n) {
  size_t orig_size = pArray->m_size;
  if (!mz_zip_array_resize(pZip, pArray, orig_size + n, MZ_TRUE))
    return MZ_FALSE;
  if (n > 0)
    memcpy((mz_uint8 *)pArray->m_p + orig_size * pArray->m_element_size,
           pElements, n * pArray->m_element_size);
  return MZ_TRUE;
}

#ifndef MINIZ_NO_TIME
static MZ_TIME_T mz_zip_dos_to_time_t(int dos_time, int dos_date) {
  struct tm tm;
  memset(&tm, 0, sizeof(tm));
  tm.tm_isdst = -1;
  tm.tm_year = ((dos_date >> 9) & 127) + 1980 - 1900;
  tm.tm_mon = ((dos_date >> 5) & 15) - 1;
  tm.tm_mday = dos_date & 31;
  tm.tm_hour = (dos_time >> 11) & 31;
  tm.tm_min = (dos_time >> 5) & 63;
  tm.tm_sec = (dos_time << 1) & 62;
  return mktime(&tm);
}

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
static void mz_zip_time_t_to_dos_time(MZ_TIME_T time, mz_uint16 *pDOS_time,
                                      mz_uint16 *pDOS_date) {
#ifdef _MSC_VER
  struct tm tm_struct;
  struct tm *tm = &tm_struct;
  errno_t err = localtime_s(tm, &time);
  if (err) {
    *pDOS_date = 0;
    *pDOS_time = 0;
    return;
  }
#else
  struct tm *tm = localtime(&time);
#endif /* #ifdef _MSC_VER */

  *pDOS_time = (mz_uint16)(((tm->tm_hour) << 11) + ((tm->tm_min) << 5) +
                           ((tm->tm_sec) >> 1));
  *pDOS_date = (mz_uint16)(((tm->tm_year + 1900 - 1980) << 9) +
                           ((tm->tm_mon + 1) << 5) + tm->tm_mday);
}
#endif /* MINIZ_NO_ARCHIVE_WRITING_APIS */

#ifndef MINIZ_NO_STDIO
#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
static mz_bool mz_zip_get_file_modified_time(const char *pFilename,
                                             MZ_TIME_T *pTime) {
  struct MZ_FILE_STAT_STRUCT file_stat;

  /* On Linux with x86 glibc, this call will fail on large files (I think >=
   * 0x80000000 bytes) unless you compiled with _LARGEFILE64_SOURCE. Argh. */
  if (MZ_FILE_STAT(pFilename, &file_stat) != 0)
    return MZ_FALSE;

  *pTime = file_stat.st_mtime;

  return MZ_TRUE;
}
#endif /* #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS*/

static mz_bool mz_zip_set_file_times(const char *pFilename,
                                     MZ_TIME_T access_time,
                                     MZ_TIME_T modified_time) {
  struct utimbuf t;

  memset(&t, 0, sizeof(t));
  t.actime = access_time;
  t.modtime = modified_time;

  return !utime(pFilename, &t);
}
#endif /* #ifndef MINIZ_NO_STDIO */
#endif /* #ifndef MINIZ_NO_TIME */

static MZ_FORCEINLINE mz_bool mz_zip_set_error(mz_zip_archive *pZip,
                                               mz_zip_error err_num) {
  if (pZip)
    pZip->m_last_error = err_num;
  return MZ_FALSE;
}

static mz_bool mz_zip_reader_init_internal(mz_zip_archive *pZip,
                                           mz_uint flags) {
  (void)flags;
  if ((!pZip) || (pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_INVALID))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (!pZip->m_pAlloc)
    pZip->m_pAlloc = miniz_def_alloc_func;
  if (!pZip->m_pFree)
    pZip->m_pFree = miniz_def_free_func;
  if (!pZip->m_pRealloc)
    pZip->m_pRealloc = miniz_def_realloc_func;

  pZip->m_archive_size = 0;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;
  pZip->m_last_error = MZ_ZIP_NO_ERROR;

  if (NULL == (pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc(
                   pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_internal_state))))
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

  memset(pZip->m_pState, 0, sizeof(mz_zip_internal_state));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir,
                                sizeof(mz_uint8));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets,
                                sizeof(mz_uint32));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets,
                                sizeof(mz_uint32));
  pZip->m_pState->m_init_flags = flags;
  pZip->m_pState->m_zip64 = MZ_FALSE;
  pZip->m_pState->m_zip64_has_extended_info_fields = MZ_FALSE;

  pZip->m_zip_mode = MZ_ZIP_MODE_READING;

  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool
mz_zip_reader_filename_less(const mz_zip_array *pCentral_dir_array,
                            const mz_zip_array *pCentral_dir_offsets,
                            mz_uint l_index, mz_uint r_index) {
  const mz_uint8 *pL = &MZ_ZIP_ARRAY_ELEMENT(
                     pCentral_dir_array, mz_uint8,
                     MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, mz_uint32,
                                          l_index)),
                 *pE;
  const mz_uint8 *pR = &MZ_ZIP_ARRAY_ELEMENT(
      pCentral_dir_array, mz_uint8,
      MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, mz_uint32, r_index));
  mz_uint l_len = MZ_READ_LE16(pL + MZ_ZIP_CDH_FILENAME_LEN_OFS),
          r_len = MZ_READ_LE16(pR + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  mz_uint8 l = 0, r = 0;
  pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pR += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + MZ_MIN(l_len, r_len);
  while (pL < pE) {
    if ((l = MZ_TOLOWER(*pL)) != (r = MZ_TOLOWER(*pR)))
      break;
    pL++;
    pR++;
  }
  return (pL == pE) ? (l_len < r_len) : (l < r);
}

#define MZ_SWAP_UINT32(a, b)                                                   \
  do {                                                                         \
    mz_uint32 t = a;                                                           \
    a = b;                                                                     \
    b = t;                                                                     \
  }                                                                            \
  MZ_MACRO_END

/* Heap sort of lowercased filenames, used to help accelerate plain central
 * directory searches by mz_zip_reader_locate_file(). (Could also use qsort(),
 * but it could allocate memory.) */
static void
mz_zip_reader_sort_central_dir_offsets_by_filename(mz_zip_archive *pZip) {
  mz_zip_internal_state *pState = pZip->m_pState;
  const mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const mz_zip_array *pCentral_dir = &pState->m_central_dir;
  mz_uint32 *pIndices;
  mz_uint32 start, end;
  const mz_uint32 size = pZip->m_total_files;

  if (size <= 1U)
    return;

  pIndices = &MZ_ZIP_ARRAY_ELEMENT(&pState->m_sorted_central_dir_offsets,
                                   mz_uint32, 0);

  start = (size - 2U) >> 1U;
  for (;;) {
    mz_uint64 child, root = start;
    for (;;) {
      if ((child = (root << 1U) + 1U) >= size)
        break;
      child += (((child + 1U) < size) &&
                (mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets,
                                             pIndices[child],
                                             pIndices[child + 1U])));
      if (!mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets,
                                       pIndices[root], pIndices[child]))
        break;
      MZ_SWAP_UINT32(pIndices[root], pIndices[child]);
      root = child;
    }
    if (!start)
      break;
    start--;
  }

  end = size - 1;
  while (end > 0) {
    mz_uint64 child, root = 0;
    MZ_SWAP_UINT32(pIndices[end], pIndices[0]);
    for (;;) {
      if ((child = (root << 1U) + 1U) >= end)
        break;
      child +=
          (((child + 1U) < end) &&
           mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets,
                                       pIndices[child], pIndices[child + 1U]));
      if (!mz_zip_reader_filename_less(pCentral_dir, pCentral_dir_offsets,
                                       pIndices[root], pIndices[child]))
        break;
      MZ_SWAP_UINT32(pIndices[root], pIndices[child]);
      root = child;
    }
    end--;
  }
}

static mz_bool mz_zip_reader_locate_header_sig(mz_zip_archive *pZip,
                                               mz_uint32 record_sig,
                                               mz_uint32 record_size,
                                               mz_int64 *pOfs) {
  mz_int64 cur_file_ofs;
  mz_uint32 buf_u32[4096 / sizeof(mz_uint32)];
  mz_uint8 *pBuf = (mz_uint8 *)buf_u32;

  /* Basic sanity checks - reject files which are too small */
  if (pZip->m_archive_size < record_size)
    return MZ_FALSE;

  /* Find the record by scanning the file from the end towards the beginning. */
  cur_file_ofs =
      MZ_MAX((mz_int64)pZip->m_archive_size - (mz_int64)sizeof(buf_u32), 0);
  for (;;) {
    int i,
        n = (int)MZ_MIN(sizeof(buf_u32), pZip->m_archive_size - cur_file_ofs);

    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf, n) != (mz_uint)n)
      return MZ_FALSE;

    for (i = n - 4; i >= 0; --i) {
      mz_uint s = MZ_READ_LE32(pBuf + i);
      if (s == record_sig) {
        if ((pZip->m_archive_size - (cur_file_ofs + i)) >= record_size)
          break;
      }
    }

    if (i >= 0) {
      cur_file_ofs += i;
      break;
    }

    /* Give up if we've searched the entire file, or we've gone back "too far"
     * (~64kb) */
    if ((!cur_file_ofs) || ((pZip->m_archive_size - cur_file_ofs) >=
                            (MZ_UINT16_MAX + record_size)))
      return MZ_FALSE;

    cur_file_ofs = MZ_MAX(cur_file_ofs - (sizeof(buf_u32) - 3), 0);
  }

  *pOfs = cur_file_ofs;
  return MZ_TRUE;
}

static mz_bool mz_zip_reader_read_central_dir(mz_zip_archive *pZip,
                                              mz_uint flags) {
  mz_uint cdir_size = 0, cdir_entries_on_this_disk = 0, num_this_disk = 0,
          cdir_disk_index = 0;
  mz_uint64 cdir_ofs = 0;
  mz_int64 cur_file_ofs = 0;
  const mz_uint8 *p;

  mz_uint32 buf_u32[4096 / sizeof(mz_uint32)];
  mz_uint8 *pBuf = (mz_uint8 *)buf_u32;
  mz_bool sort_central_dir =
      ((flags & MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY) == 0);
  mz_uint32 zip64_end_of_central_dir_locator_u32
      [(MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE + sizeof(mz_uint32) - 1) /
       sizeof(mz_uint32)];
  mz_uint8 *pZip64_locator = (mz_uint8 *)zip64_end_of_central_dir_locator_u32;

  mz_uint32 zip64_end_of_central_dir_header_u32
      [(MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) /
       sizeof(mz_uint32)];
  mz_uint8 *pZip64_end_of_central_dir =
      (mz_uint8 *)zip64_end_of_central_dir_header_u32;

  mz_uint64 zip64_end_of_central_dir_ofs = 0;

  /* Basic sanity checks - reject files which are too small, and check the first
   * 4 bytes of the file to make sure a local header is there. */
  if (pZip->m_archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);

  if (!mz_zip_reader_locate_header_sig(
          pZip, MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG,
          MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE, &cur_file_ofs))
    return mz_zip_set_error(pZip, MZ_ZIP_FAILED_FINDING_CENTRAL_DIR);

  /* Read and verify the end of central directory record. */
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf,
                    MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

  if (MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_SIG_OFS) !=
      MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG)
    return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);

  if (cur_file_ofs >= (MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE +
                       MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE)) {
    if (pZip->m_pRead(pZip->m_pIO_opaque,
                      cur_file_ofs - MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE,
                      pZip64_locator,
                      MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE) ==
        MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE) {
      if (MZ_READ_LE32(pZip64_locator + MZ_ZIP64_ECDL_SIG_OFS) ==
          MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG) {
        zip64_end_of_central_dir_ofs = MZ_READ_LE64(
            pZip64_locator + MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS);
        if (zip64_end_of_central_dir_ofs >
            (pZip->m_archive_size - MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE))
          return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);

        if (pZip->m_pRead(pZip->m_pIO_opaque, zip64_end_of_central_dir_ofs,
                          pZip64_end_of_central_dir,
                          MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE) ==
            MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE) {
          if (MZ_READ_LE32(pZip64_end_of_central_dir + MZ_ZIP64_ECDH_SIG_OFS) ==
              MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG) {
            pZip->m_pState->m_zip64 = MZ_TRUE;
          }
        }
      }
    }
  }

  pZip->m_total_files = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS);
  cdir_entries_on_this_disk =
      MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS);
  num_this_disk = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_NUM_THIS_DISK_OFS);
  cdir_disk_index = MZ_READ_LE16(pBuf + MZ_ZIP_ECDH_NUM_DISK_CDIR_OFS);
  cdir_size = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_SIZE_OFS);
  cdir_ofs = MZ_READ_LE32(pBuf + MZ_ZIP_ECDH_CDIR_OFS_OFS);

  if (pZip->m_pState->m_zip64) {
    mz_uint32 zip64_total_num_of_disks =
        MZ_READ_LE32(pZip64_locator + MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS);
    mz_uint64 zip64_cdir_total_entries = MZ_READ_LE64(
        pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS);
    mz_uint64 zip64_cdir_total_entries_on_this_disk = MZ_READ_LE64(
        pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS);
    mz_uint64 zip64_size_of_end_of_central_dir_record = MZ_READ_LE64(
        pZip64_end_of_central_dir + MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS);
    mz_uint64 zip64_size_of_central_directory =
        MZ_READ_LE64(pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_SIZE_OFS);

    if (zip64_size_of_end_of_central_dir_record <
        (MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE - 12))
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

    if (zip64_total_num_of_disks != 1U)
      return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK);

    /* Check for miniz's practical limits */
    if (zip64_cdir_total_entries > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);

    pZip->m_total_files = (mz_uint32)zip64_cdir_total_entries;

    if (zip64_cdir_total_entries_on_this_disk > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);

    cdir_entries_on_this_disk =
        (mz_uint32)zip64_cdir_total_entries_on_this_disk;

    /* Check for miniz's current practical limits (sorry, this should be enough
     * for millions of files) */
    if (zip64_size_of_central_directory > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE);

    cdir_size = (mz_uint32)zip64_size_of_central_directory;

    num_this_disk = MZ_READ_LE32(pZip64_end_of_central_dir +
                                 MZ_ZIP64_ECDH_NUM_THIS_DISK_OFS);

    cdir_disk_index = MZ_READ_LE32(pZip64_end_of_central_dir +
                                   MZ_ZIP64_ECDH_NUM_DISK_CDIR_OFS);

    cdir_ofs =
        MZ_READ_LE64(pZip64_end_of_central_dir + MZ_ZIP64_ECDH_CDIR_OFS_OFS);
  }

  if (pZip->m_total_files != cdir_entries_on_this_disk)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK);

  if (((num_this_disk | cdir_disk_index) != 0) &&
      ((num_this_disk != 1) || (cdir_disk_index != 1)))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK);

  if (cdir_size < pZip->m_total_files * MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  if ((cdir_ofs + (mz_uint64)cdir_size) > pZip->m_archive_size)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  pZip->m_central_directory_file_ofs = cdir_ofs;

  if (pZip->m_total_files) {
    mz_uint i, n;
    /* Read the entire central directory into a heap block, and allocate another
     * heap block to hold the unsorted central dir file record offsets, and
     * possibly another to hold the sorted indices. */
    if ((!mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir, cdir_size,
                              MZ_FALSE)) ||
        (!mz_zip_array_resize(pZip, &pZip->m_pState->m_central_dir_offsets,
                              pZip->m_total_files, MZ_FALSE)))
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

    if (sort_central_dir) {
      if (!mz_zip_array_resize(pZip,
                               &pZip->m_pState->m_sorted_central_dir_offsets,
                               pZip->m_total_files, MZ_FALSE))
        return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    if (pZip->m_pRead(pZip->m_pIO_opaque, cdir_ofs,
                      pZip->m_pState->m_central_dir.m_p,
                      cdir_size) != cdir_size)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

    /* Now create an index into the central directory file records, do some
     * basic sanity checking on each record */
    p = (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p;
    for (n = cdir_size, i = 0; i < pZip->m_total_files; ++i) {
      mz_uint total_header_size, disk_index, bit_flags, filename_size,
          ext_data_size;
      mz_uint64 comp_size, decomp_size, local_header_ofs;

      if ((n < MZ_ZIP_CENTRAL_DIR_HEADER_SIZE) ||
          (MZ_READ_LE32(p) != MZ_ZIP_CENTRAL_DIR_HEADER_SIG))
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

      MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32,
                           i) =
          (mz_uint32)(p - (const mz_uint8 *)pZip->m_pState->m_central_dir.m_p);

      if (sort_central_dir)
        MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_sorted_central_dir_offsets,
                             mz_uint32, i) = i;

      comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
      decomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
      local_header_ofs = MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS);
      filename_size = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
      ext_data_size = MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS);

      if ((!pZip->m_pState->m_zip64_has_extended_info_fields) &&
          (ext_data_size) &&
          (MZ_MAX(MZ_MAX(comp_size, decomp_size), local_header_ofs) ==
           MZ_UINT32_MAX)) {
        /* Attempt to find zip64 extended information field in the entry's extra
         * data */
        mz_uint32 extra_size_remaining = ext_data_size;

        if (extra_size_remaining) {
          const mz_uint8 *pExtra_data;
          void *buf = NULL;

          if (MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + ext_data_size >
              n) {
            buf = MZ_MALLOC(ext_data_size);
            if (buf == NULL)
              return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

            if (pZip->m_pRead(pZip->m_pIO_opaque,
                              cdir_ofs + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
                                  filename_size,
                              buf, ext_data_size) != ext_data_size) {
              MZ_FREE(buf);
              return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
            }

            pExtra_data = (mz_uint8 *)buf;
          } else {
            pExtra_data = p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size;
          }

          do {
            mz_uint32 field_id;
            mz_uint32 field_data_size;

            if (extra_size_remaining < (sizeof(mz_uint16) * 2)) {
              MZ_FREE(buf);
              return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
            }

            field_id = MZ_READ_LE16(pExtra_data);
            field_data_size = MZ_READ_LE16(pExtra_data + sizeof(mz_uint16));

            if ((field_data_size + sizeof(mz_uint16) * 2) >
                extra_size_remaining) {
              MZ_FREE(buf);
              return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
            }

            if (field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID) {
              /* Ok, the archive didn't have any zip64 headers but it uses a
               * zip64 extended information field so mark it as zip64 anyway
               * (this can occur with infozip's zip util when it reads
               * compresses files from stdin). */
              pZip->m_pState->m_zip64 = MZ_TRUE;
              pZip->m_pState->m_zip64_has_extended_info_fields = MZ_TRUE;
              break;
            }

            pExtra_data += sizeof(mz_uint16) * 2 + field_data_size;
            extra_size_remaining =
                extra_size_remaining - sizeof(mz_uint16) * 2 - field_data_size;
          } while (extra_size_remaining);

          MZ_FREE(buf);
        }
      }

      /* I've seen archives that aren't marked as zip64 that uses zip64 ext
       * data, argh */
      if ((comp_size != MZ_UINT32_MAX) && (decomp_size != MZ_UINT32_MAX)) {
        if (((!MZ_READ_LE32(p + MZ_ZIP_CDH_METHOD_OFS)) &&
             (decomp_size != comp_size)) ||
            (decomp_size && !comp_size))
          return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
      }

      disk_index = MZ_READ_LE16(p + MZ_ZIP_CDH_DISK_START_OFS);
      if ((disk_index == MZ_UINT16_MAX) ||
          ((disk_index != num_this_disk) && (disk_index != 1)))
        return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_MULTIDISK);

      if (comp_size != MZ_UINT32_MAX) {
        if (((mz_uint64)MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS) +
             MZ_ZIP_LOCAL_DIR_HEADER_SIZE + comp_size) > pZip->m_archive_size)
          return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
      }

      bit_flags = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
      if (bit_flags & MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_LOCAL_DIR_IS_MASKED)
        return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION);

      if ((total_header_size = MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
                               MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS) +
                               MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS) +
                               MZ_READ_LE16(p + MZ_ZIP_CDH_COMMENT_LEN_OFS)) >
          n)
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

      n -= total_header_size;
      p += total_header_size;
    }
  }

  if (sort_central_dir)
    mz_zip_reader_sort_central_dir_offsets_by_filename(pZip);

  return MZ_TRUE;
}

void mz_zip_zero_struct(mz_zip_archive *pZip) {
  if (pZip)
    MZ_CLEAR_OBJ(*pZip);
}

static mz_bool mz_zip_reader_end_internal(mz_zip_archive *pZip,
                                          mz_bool set_last_error) {
  mz_bool status = MZ_TRUE;

  if (!pZip)
    return MZ_FALSE;

  if ((!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) ||
      (pZip->m_zip_mode != MZ_ZIP_MODE_READING)) {
    if (set_last_error)
      pZip->m_last_error = MZ_ZIP_INVALID_PARAMETER;

    return MZ_FALSE;
  }

  if (pZip->m_pState) {
    mz_zip_internal_state *pState = pZip->m_pState;
    pZip->m_pState = NULL;

    mz_zip_array_clear(pZip, &pState->m_central_dir);
    mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
    mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

#ifndef MINIZ_NO_STDIO
    if (pState->m_pFile) {
      if (pZip->m_zip_type == MZ_ZIP_TYPE_FILE) {
        if (MZ_FCLOSE(pState->m_pFile) == EOF) {
          if (set_last_error)
            pZip->m_last_error = MZ_ZIP_FILE_CLOSE_FAILED;
          status = MZ_FALSE;
        }
      }
      pState->m_pFile = NULL;
    }
#endif /* #ifndef MINIZ_NO_STDIO */

    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  }
  pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;

  return status;
}

mz_bool mz_zip_reader_end(mz_zip_archive *pZip) {
  return mz_zip_reader_end_internal(pZip, MZ_TRUE);
}
mz_bool mz_zip_reader_init(mz_zip_archive *pZip, mz_uint64 size,
                           mz_uint flags) {
  if ((!pZip) || (!pZip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (!mz_zip_reader_init_internal(pZip, flags))
    return MZ_FALSE;

  pZip->m_zip_type = MZ_ZIP_TYPE_USER;
  pZip->m_archive_size = size;

  if (!mz_zip_reader_read_central_dir(pZip, flags)) {
    mz_zip_reader_end_internal(pZip, MZ_FALSE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

static size_t mz_zip_mem_read_func(void *pOpaque, mz_uint64 file_ofs,
                                   void *pBuf, size_t n) {
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  size_t s = (file_ofs >= pZip->m_archive_size)
                 ? 0
                 : (size_t)MZ_MIN(pZip->m_archive_size - file_ofs, n);
  memcpy(pBuf, (const mz_uint8 *)pZip->m_pState->m_pMem + file_ofs, s);
  return s;
}

mz_bool mz_zip_reader_init_mem(mz_zip_archive *pZip, const void *pMem,
                               size_t size, mz_uint flags) {
  if (!pMem)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);

  if (!mz_zip_reader_init_internal(pZip, flags))
    return MZ_FALSE;

  pZip->m_zip_type = MZ_ZIP_TYPE_MEMORY;
  pZip->m_archive_size = size;
  pZip->m_pRead = mz_zip_mem_read_func;
  pZip->m_pIO_opaque = pZip;
  pZip->m_pNeeds_keepalive = NULL;

#ifdef __cplusplus
  pZip->m_pState->m_pMem = const_cast<void *>(pMem);
#else
  pZip->m_pState->m_pMem = (void *)pMem;
#endif

  pZip->m_pState->m_mem_size = size;

  if (!mz_zip_reader_read_central_dir(pZip, flags)) {
    mz_zip_reader_end_internal(pZip, MZ_FALSE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_read_func(void *pOpaque, mz_uint64 file_ofs,
                                    void *pBuf, size_t n) {
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_int64 cur_ofs = MZ_FTELL64(pZip->m_pState->m_pFile);

  file_ofs += pZip->m_pState->m_file_archive_start_ofs;

  if (((mz_int64)file_ofs < 0) ||
      (((cur_ofs != (mz_int64)file_ofs)) &&
       (MZ_FSEEK64(pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET))))
    return 0;

  return MZ_FREAD(pBuf, 1, n, pZip->m_pState->m_pFile);
}

mz_bool mz_zip_reader_init_file(mz_zip_archive *pZip, const char *pFilename,
                                mz_uint32 flags) {
  return mz_zip_reader_init_file_v2(pZip, pFilename, flags, 0, 0);
}

mz_bool mz_zip_reader_init_file_v2(mz_zip_archive *pZip, const char *pFilename,
                                   mz_uint flags, mz_uint64 file_start_ofs,
                                   mz_uint64 archive_size) {
  mz_uint64 file_size;
  MZ_FILE *pFile;

  if ((!pZip) || (!pFilename) ||
      ((archive_size) &&
       (archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pFile = MZ_FOPEN(pFilename, "rb");
  if (!pFile)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);

  file_size = archive_size;
  if (!file_size) {
    if (MZ_FSEEK64(pFile, 0, SEEK_END)) {
      MZ_FCLOSE(pFile);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_SEEK_FAILED);
    }

    file_size = MZ_FTELL64(pFile);
  }

  /* TODO: Better sanity check archive_size and the # of actual remaining bytes
   */

  if (file_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) {
    MZ_FCLOSE(pFile);
    return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);
  }

  if (!mz_zip_reader_init_internal(pZip, flags)) {
    MZ_FCLOSE(pFile);
    return MZ_FALSE;
  }

  pZip->m_zip_type = MZ_ZIP_TYPE_FILE;
  pZip->m_pRead = mz_zip_file_read_func;
  pZip->m_pIO_opaque = pZip;
  pZip->m_pState->m_pFile = pFile;
  pZip->m_archive_size = file_size;
  pZip->m_pState->m_file_archive_start_ofs = file_start_ofs;

  if (!mz_zip_reader_read_central_dir(pZip, flags)) {
    mz_zip_reader_end_internal(pZip, MZ_FALSE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

mz_bool mz_zip_reader_init_file_v2_rpb(mz_zip_archive *pZip,
                                       const char *pFilename, mz_uint flags,
                                       mz_uint64 file_start_ofs,
                                       mz_uint64 archive_size) {
  mz_uint64 file_size;
  MZ_FILE *pFile;

  if ((!pZip) || (!pFilename) ||
      ((archive_size) &&
       (archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pFile = MZ_FOPEN(pFilename, "r+b");
  if (!pFile)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);

  file_size = archive_size;
  if (!file_size) {
    if (MZ_FSEEK64(pFile, 0, SEEK_END)) {
      MZ_FCLOSE(pFile);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_SEEK_FAILED);
    }

    file_size = MZ_FTELL64(pFile);
  }

  /* TODO: Better sanity check archive_size and the # of actual remaining bytes
   */

  if (file_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) {
    MZ_FCLOSE(pFile);
    return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);
  }

  if (!mz_zip_reader_init_internal(pZip, flags)) {
    MZ_FCLOSE(pFile);
    return MZ_FALSE;
  }

  pZip->m_zip_type = MZ_ZIP_TYPE_FILE;
  pZip->m_pRead = mz_zip_file_read_func;
  pZip->m_pIO_opaque = pZip;
  pZip->m_pState->m_pFile = pFile;
  pZip->m_archive_size = file_size;
  pZip->m_pState->m_file_archive_start_ofs = file_start_ofs;

  if (!mz_zip_reader_read_central_dir(pZip, flags)) {
    mz_zip_reader_end_internal(pZip, MZ_FALSE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

mz_bool mz_zip_reader_init_cfile(mz_zip_archive *pZip, MZ_FILE *pFile,
                                 mz_uint64 archive_size, mz_uint flags) {
  mz_uint64 cur_file_ofs;

  if ((!pZip) || (!pFile))
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);

  cur_file_ofs = MZ_FTELL64(pFile);

  if (!archive_size) {
    if (MZ_FSEEK64(pFile, 0, SEEK_END))
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_SEEK_FAILED);

    archive_size = MZ_FTELL64(pFile) - cur_file_ofs;

    if (archive_size < MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
      return mz_zip_set_error(pZip, MZ_ZIP_NOT_AN_ARCHIVE);
  }

  if (!mz_zip_reader_init_internal(pZip, flags))
    return MZ_FALSE;

  pZip->m_zip_type = MZ_ZIP_TYPE_CFILE;
  pZip->m_pRead = mz_zip_file_read_func;

  pZip->m_pIO_opaque = pZip;
  pZip->m_pState->m_pFile = pFile;
  pZip->m_archive_size = archive_size;
  pZip->m_pState->m_file_archive_start_ofs = cur_file_ofs;

  if (!mz_zip_reader_read_central_dir(pZip, flags)) {
    mz_zip_reader_end_internal(pZip, MZ_FALSE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

#endif /* #ifndef MINIZ_NO_STDIO */

static MZ_FORCEINLINE const mz_uint8 *mz_zip_get_cdh(mz_zip_archive *pZip,
                                                     mz_uint file_index) {
  if ((!pZip) || (!pZip->m_pState) || (file_index >= pZip->m_total_files))
    return NULL;
  return &MZ_ZIP_ARRAY_ELEMENT(
      &pZip->m_pState->m_central_dir, mz_uint8,
      MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32,
                           file_index));
}

mz_bool mz_zip_reader_is_file_encrypted(mz_zip_archive *pZip,
                                        mz_uint file_index) {
  mz_uint m_bit_flag;
  const mz_uint8 *p = mz_zip_get_cdh(pZip, file_index);
  if (!p) {
    mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
    return MZ_FALSE;
  }

  m_bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
  return (m_bit_flag &
          (MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED |
           MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION)) != 0;
}

mz_bool mz_zip_reader_is_file_supported(mz_zip_archive *pZip,
                                        mz_uint file_index) {
  mz_uint bit_flag;
  mz_uint method;

  const mz_uint8 *p = mz_zip_get_cdh(pZip, file_index);
  if (!p) {
    mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
    return MZ_FALSE;
  }

  method = MZ_READ_LE16(p + MZ_ZIP_CDH_METHOD_OFS);
  bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);

  if ((method != 0) && (method != MZ_DEFLATED)) {
    mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_METHOD);
    return MZ_FALSE;
  }

  if (bit_flag & (MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED |
                  MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION)) {
    mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION);
    return MZ_FALSE;
  }

  if (bit_flag & MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG) {
    mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_FEATURE);
    return MZ_FALSE;
  }

  return MZ_TRUE;
}

mz_bool mz_zip_reader_is_file_a_directory(mz_zip_archive *pZip,
                                          mz_uint file_index) {
  mz_uint filename_len, attribute_mapping_id, external_attr;
  const mz_uint8 *p = mz_zip_get_cdh(pZip, file_index);
  if (!p) {
    mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
    return MZ_FALSE;
  }

  filename_len = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_len) {
    if (*(p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_len - 1) == '/')
      return MZ_TRUE;
  }

  /* Bugfix: This code was also checking if the internal attribute was non-zero,
   * which wasn't correct. */
  /* Most/all zip writers (hopefully) set DOS file/directory attributes in the
   * low 16-bits, so check for the DOS directory flag and ignore the source OS
   * ID in the created by field. */
  /* FIXME: Remove this check? Is it necessary - we already check the filename.
   */
  attribute_mapping_id = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_MADE_BY_OFS) >> 8;
  (void)attribute_mapping_id;

  external_attr = MZ_READ_LE32(p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  if ((external_attr & MZ_ZIP_DOS_DIR_ATTRIBUTE_BITFLAG) != 0) {
    return MZ_TRUE;
  }

  return MZ_FALSE;
}

static mz_bool mz_zip_file_stat_internal(mz_zip_archive *pZip,
                                         mz_uint file_index,
                                         const mz_uint8 *pCentral_dir_header,
                                         mz_zip_archive_file_stat *pStat,
                                         mz_bool *pFound_zip64_extra_data) {
  mz_uint n;
  const mz_uint8 *p = pCentral_dir_header;

  if (pFound_zip64_extra_data)
    *pFound_zip64_extra_data = MZ_FALSE;

  if ((!p) || (!pStat))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  /* Extract fields from the central directory record. */
  pStat->m_file_index = file_index;
  pStat->m_central_dir_ofs = MZ_ZIP_ARRAY_ELEMENT(
      &pZip->m_pState->m_central_dir_offsets, mz_uint32, file_index);
  pStat->m_version_made_by = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_MADE_BY_OFS);
  pStat->m_version_needed = MZ_READ_LE16(p + MZ_ZIP_CDH_VERSION_NEEDED_OFS);
  pStat->m_bit_flag = MZ_READ_LE16(p + MZ_ZIP_CDH_BIT_FLAG_OFS);
  pStat->m_method = MZ_READ_LE16(p + MZ_ZIP_CDH_METHOD_OFS);
#ifndef MINIZ_NO_TIME
  pStat->m_time =
      mz_zip_dos_to_time_t(MZ_READ_LE16(p + MZ_ZIP_CDH_FILE_TIME_OFS),
                           MZ_READ_LE16(p + MZ_ZIP_CDH_FILE_DATE_OFS));
#endif
  pStat->m_crc32 = MZ_READ_LE32(p + MZ_ZIP_CDH_CRC32_OFS);
  pStat->m_comp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS);
  pStat->m_uncomp_size = MZ_READ_LE32(p + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS);
  pStat->m_internal_attr = MZ_READ_LE16(p + MZ_ZIP_CDH_INTERNAL_ATTR_OFS);
  pStat->m_external_attr = MZ_READ_LE32(p + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS);
  pStat->m_local_header_ofs = MZ_READ_LE32(p + MZ_ZIP_CDH_LOCAL_HEADER_OFS);

  /* Copy as much of the filename and comment as possible. */
  n = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  n = MZ_MIN(n, MZ_ZIP_MAX_ARCHIVE_FILENAME_SIZE - 1);
  memcpy(pStat->m_filename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n);
  pStat->m_filename[n] = '\0';

  n = MZ_READ_LE16(p + MZ_ZIP_CDH_COMMENT_LEN_OFS);
  n = MZ_MIN(n, MZ_ZIP_MAX_ARCHIVE_FILE_COMMENT_SIZE - 1);
  pStat->m_comment_size = n;
  memcpy(pStat->m_comment,
         p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
             MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS) +
             MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS),
         n);
  pStat->m_comment[n] = '\0';

  /* Set some flags for convienance */
  pStat->m_is_directory = mz_zip_reader_is_file_a_directory(pZip, file_index);
  pStat->m_is_encrypted = mz_zip_reader_is_file_encrypted(pZip, file_index);
  pStat->m_is_supported = mz_zip_reader_is_file_supported(pZip, file_index);

  /* See if we need to read any zip64 extended information fields. */
  /* Confusingly, these zip64 fields can be present even on non-zip64 archives
   * (Debian zip on a huge files from stdin piped to stdout creates them). */
  if (MZ_MAX(MZ_MAX(pStat->m_comp_size, pStat->m_uncomp_size),
             pStat->m_local_header_ofs) == MZ_UINT32_MAX) {
    /* Attempt to find zip64 extended information field in the entry's extra
     * data */
    mz_uint32 extra_size_remaining = MZ_READ_LE16(p + MZ_ZIP_CDH_EXTRA_LEN_OFS);

    if (extra_size_remaining) {
      const mz_uint8 *pExtra_data =
          p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
          MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);

      do {
        mz_uint32 field_id;
        mz_uint32 field_data_size;

        if (extra_size_remaining < (sizeof(mz_uint16) * 2))
          return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

        field_id = MZ_READ_LE16(pExtra_data);
        field_data_size = MZ_READ_LE16(pExtra_data + sizeof(mz_uint16));

        if ((field_data_size + sizeof(mz_uint16) * 2) > extra_size_remaining)
          return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

        if (field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID) {
          const mz_uint8 *pField_data = pExtra_data + sizeof(mz_uint16) * 2;
          mz_uint32 field_data_remaining = field_data_size;

          if (pFound_zip64_extra_data)
            *pFound_zip64_extra_data = MZ_TRUE;

          if (pStat->m_uncomp_size == MZ_UINT32_MAX) {
            if (field_data_remaining < sizeof(mz_uint64))
              return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

            pStat->m_uncomp_size = MZ_READ_LE64(pField_data);
            pField_data += sizeof(mz_uint64);
            field_data_remaining -= sizeof(mz_uint64);
          }

          if (pStat->m_comp_size == MZ_UINT32_MAX) {
            if (field_data_remaining < sizeof(mz_uint64))
              return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

            pStat->m_comp_size = MZ_READ_LE64(pField_data);
            pField_data += sizeof(mz_uint64);
            field_data_remaining -= sizeof(mz_uint64);
          }

          if (pStat->m_local_header_ofs == MZ_UINT32_MAX) {
            if (field_data_remaining < sizeof(mz_uint64))
              return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

            pStat->m_local_header_ofs = MZ_READ_LE64(pField_data);
            pField_data += sizeof(mz_uint64);
            (void)pField_data; // unused

            field_data_remaining -= sizeof(mz_uint64);
            (void)field_data_remaining; // unused
          }

          break;
        }

        pExtra_data += sizeof(mz_uint16) * 2 + field_data_size;
        extra_size_remaining =
            extra_size_remaining - sizeof(mz_uint16) * 2 - field_data_size;
      } while (extra_size_remaining);
    }
  }

  return MZ_TRUE;
}

static MZ_FORCEINLINE mz_bool mz_zip_string_equal(const char *pA,
                                                  const char *pB, mz_uint len,
                                                  mz_uint flags) {
  mz_uint i;
  if (flags & MZ_ZIP_FLAG_CASE_SENSITIVE)
    return 0 == memcmp(pA, pB, len);
  for (i = 0; i < len; ++i)
    if (MZ_TOLOWER(pA[i]) != MZ_TOLOWER(pB[i]))
      return MZ_FALSE;
  return MZ_TRUE;
}

static MZ_FORCEINLINE int
mz_zip_filename_compare(const mz_zip_array *pCentral_dir_array,
                        const mz_zip_array *pCentral_dir_offsets,
                        mz_uint l_index, const char *pR, mz_uint r_len) {
  const mz_uint8 *pL = &MZ_ZIP_ARRAY_ELEMENT(
                     pCentral_dir_array, mz_uint8,
                     MZ_ZIP_ARRAY_ELEMENT(pCentral_dir_offsets, mz_uint32,
                                          l_index)),
                 *pE;
  mz_uint l_len = MZ_READ_LE16(pL + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  mz_uint8 l = 0, r = 0;
  pL += MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
  pE = pL + MZ_MIN(l_len, r_len);
  while (pL < pE) {
    if ((l = MZ_TOLOWER(*pL)) != (r = MZ_TOLOWER(*pR)))
      break;
    pL++;
    pR++;
  }
  return (pL == pE) ? (int)(l_len - r_len) : (l - r);
}

static mz_bool mz_zip_locate_file_binary_search(mz_zip_archive *pZip,
                                                const char *pFilename,
                                                mz_uint32 *pIndex) {
  mz_zip_internal_state *pState = pZip->m_pState;
  const mz_zip_array *pCentral_dir_offsets = &pState->m_central_dir_offsets;
  const mz_zip_array *pCentral_dir = &pState->m_central_dir;
  mz_uint32 *pIndices = &MZ_ZIP_ARRAY_ELEMENT(
      &pState->m_sorted_central_dir_offsets, mz_uint32, 0);
  const uint32_t size = pZip->m_total_files;
  const mz_uint filename_len = (mz_uint)strlen(pFilename);

  if (pIndex)
    *pIndex = 0;

  if (size) {
    /* yes I could use uint32_t's, but then we would have to add some special
     * case checks in the loop, argh, and */
    /* honestly the major expense here on 32-bit CPU's will still be the
     * filename compare */
    mz_int64 l = 0, h = (mz_int64)size - 1;

    while (l <= h) {
      mz_int64 m = l + ((h - l) >> 1);
      uint32_t file_index = pIndices[(uint32_t)m];

      int comp = mz_zip_filename_compare(pCentral_dir, pCentral_dir_offsets,
                                         file_index, pFilename, filename_len);
      if (!comp) {
        if (pIndex)
          *pIndex = file_index;
        return MZ_TRUE;
      } else if (comp < 0)
        l = m + 1;
      else
        h = m - 1;
    }
  }

  return mz_zip_set_error(pZip, MZ_ZIP_FILE_NOT_FOUND);
}

int mz_zip_reader_locate_file(mz_zip_archive *pZip, const char *pName,
                              const char *pComment, mz_uint flags) {
  mz_uint32 index;
  if (!mz_zip_reader_locate_file_v2(pZip, pName, pComment, flags, &index))
    return -1;
  else
    return (int)index;
}

mz_bool mz_zip_reader_locate_file_v2(mz_zip_archive *pZip, const char *pName,
                                     const char *pComment, mz_uint flags,
                                     mz_uint32 *pIndex) {
  mz_uint file_index;
  size_t name_len, comment_len;

  if (pIndex)
    *pIndex = 0;

  if ((!pZip) || (!pZip->m_pState) || (!pName))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  /* See if we can use a binary search */
  if (((pZip->m_pState->m_init_flags &
        MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY) == 0) &&
      (pZip->m_zip_mode == MZ_ZIP_MODE_READING) &&
      ((flags & (MZ_ZIP_FLAG_IGNORE_PATH | MZ_ZIP_FLAG_CASE_SENSITIVE)) == 0) &&
      (!pComment) && (pZip->m_pState->m_sorted_central_dir_offsets.m_size)) {
    return mz_zip_locate_file_binary_search(pZip, pName, pIndex);
  }

  /* Locate the entry by scanning the entire central directory */
  name_len = strlen(pName);
  if (name_len > MZ_UINT16_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  comment_len = pComment ? strlen(pComment) : 0;
  if (comment_len > MZ_UINT16_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  for (file_index = 0; file_index < pZip->m_total_files; file_index++) {
    const mz_uint8 *pHeader = &MZ_ZIP_ARRAY_ELEMENT(
        &pZip->m_pState->m_central_dir, mz_uint8,
        MZ_ZIP_ARRAY_ELEMENT(&pZip->m_pState->m_central_dir_offsets, mz_uint32,
                             file_index));
    mz_uint filename_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_FILENAME_LEN_OFS);
    const char *pFilename =
        (const char *)pHeader + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE;
    if (filename_len < name_len)
      continue;
    if (comment_len) {
      mz_uint file_extra_len = MZ_READ_LE16(pHeader + MZ_ZIP_CDH_EXTRA_LEN_OFS),
              file_comment_len =
                  MZ_READ_LE16(pHeader + MZ_ZIP_CDH_COMMENT_LEN_OFS);
      const char *pFile_comment = pFilename + filename_len + file_extra_len;
      if ((file_comment_len != comment_len) ||
          (!mz_zip_string_equal(pComment, pFile_comment, file_comment_len,
                                flags)))
        continue;
    }
    if ((flags & MZ_ZIP_FLAG_IGNORE_PATH) && (filename_len)) {
      int ofs = filename_len - 1;
      do {
        if ((pFilename[ofs] == '/') || (pFilename[ofs] == '\\') ||
            (pFilename[ofs] == ':'))
          break;
      } while (--ofs >= 0);
      ofs++;
      pFilename += ofs;
      filename_len -= ofs;
    }
    if ((filename_len == name_len) &&
        (mz_zip_string_equal(pName, pFilename, filename_len, flags))) {
      if (pIndex)
        *pIndex = file_index;
      return MZ_TRUE;
    }
  }

  return mz_zip_set_error(pZip, MZ_ZIP_FILE_NOT_FOUND);
}

static mz_bool mz_zip_reader_extract_to_mem_no_alloc1(
    mz_zip_archive *pZip, mz_uint file_index, void *pBuf, size_t buf_size,
    mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size,
    const mz_zip_archive_file_stat *st) {
  int status = TINFL_STATUS_DONE;
  mz_uint64 needed_size, cur_file_ofs, comp_remaining,
      out_buf_ofs = 0, read_buf_size, read_buf_ofs = 0, read_buf_avail;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf;
  mz_uint32
      local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) /
                       sizeof(mz_uint32)];
  mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  tinfl_decompressor inflator;

  if ((!pZip) || (!pZip->m_pState) || ((buf_size) && (!pBuf)) ||
      ((user_read_buf_size) && (!pUser_read_buf)) || (!pZip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (st) {
    file_stat = *st;
  } else if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;

  /* A directory or zero length file */
  if ((file_stat.m_is_directory) || (!file_stat.m_comp_size))
    return MZ_TRUE;

  /* Encryption and patch files are not supported. */
  if (file_stat.m_bit_flag &
      (MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED |
       MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION |
       MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION);

  /* This function only supports decompressing stored and deflate. */
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) &&
      (file_stat.m_method != MZ_DEFLATED))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_METHOD);

  /* Ensure supplied output buffer is large enough. */
  needed_size = (flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size
                                                      : file_stat.m_uncomp_size;
  if (buf_size < needed_size)
    return mz_zip_set_error(pZip, MZ_ZIP_BUF_TOO_SMALL);

  /* Read and parse the local directory entry. */
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header,
                    MZ_ZIP_LOCAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  cur_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
                  MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) +
                  MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method)) {
    /* The file is stored or the caller has requested the compressed data. */
    if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pBuf,
                      (size_t)needed_size) != needed_size)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
    if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) == 0) {
      if (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf,
                   (size_t)file_stat.m_uncomp_size) != file_stat.m_crc32)
        return mz_zip_set_error(pZip, MZ_ZIP_CRC_CHECK_FAILED);
    }
#endif

    return MZ_TRUE;
  }

  /* Decompress the file either directly from memory or from a file input
   * buffer. */
  tinfl_init(&inflator);

  if (pZip->m_pState->m_pMem) {
    /* Read directly from the archive in memory. */
    pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  } else if (pUser_read_buf) {
    /* Use a user provided read buffer. */
    if (!user_read_buf_size)
      return MZ_FALSE;
    pRead_buf = (mz_uint8 *)pUser_read_buf;
    read_buf_size = user_read_buf_size;
    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  } else {
    /* Temporarily allocate a read buffer. */
    read_buf_size =
        MZ_MIN(file_stat.m_comp_size, (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE);
    if (((sizeof(size_t) == sizeof(mz_uint32))) && (read_buf_size > 0x7FFFFFFF))
      return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1,
                                            (size_t)read_buf_size)))
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  do {
    /* The size_t cast here should be OK because we've verified that the output
     * buffer is >= file_stat.m_uncomp_size above */
    size_t in_buf_size,
        out_buf_size = (size_t)(file_stat.m_uncomp_size - out_buf_ofs);
    if ((!read_buf_avail) && (!pZip->m_pState->m_pMem)) {
      read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
      if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf,
                        (size_t)read_buf_avail) != read_buf_avail) {
        status = TINFL_STATUS_FAILED;
        mz_zip_set_error(pZip, MZ_ZIP_DECOMPRESSION_FAILED);
        break;
      }
      cur_file_ofs += read_buf_avail;
      comp_remaining -= read_buf_avail;
      read_buf_ofs = 0;
    }
    in_buf_size = (size_t)read_buf_avail;
    status = tinfl_decompress(
        &inflator, (mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size,
        (mz_uint8 *)pBuf, (mz_uint8 *)pBuf + out_buf_ofs, &out_buf_size,
        TINFL_FLAG_USING_NON_WRAPPING_OUTPUT_BUF |
            (comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0));
    read_buf_avail -= in_buf_size;
    read_buf_ofs += in_buf_size;
    out_buf_ofs += out_buf_size;
  } while (status == TINFL_STATUS_NEEDS_MORE_INPUT);

  if (status == TINFL_STATUS_DONE) {
    /* Make sure the entire file was decompressed, and check its CRC. */
    if (out_buf_ofs != file_stat.m_uncomp_size) {
      mz_zip_set_error(pZip, MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE);
      status = TINFL_STATUS_FAILED;
    }
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
    else if (mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf,
                      (size_t)file_stat.m_uncomp_size) != file_stat.m_crc32) {
      mz_zip_set_error(pZip, MZ_ZIP_CRC_CHECK_FAILED);
      status = TINFL_STATUS_FAILED;
    }
#endif
  }

  if ((!pZip->m_pState->m_pMem) && (!pUser_read_buf))
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);

  return status == TINFL_STATUS_DONE;
}

mz_bool mz_zip_reader_extract_to_mem_no_alloc(mz_zip_archive *pZip,
                                              mz_uint file_index, void *pBuf,
                                              size_t buf_size, mz_uint flags,
                                              void *pUser_read_buf,
                                              size_t user_read_buf_size) {
  return mz_zip_reader_extract_to_mem_no_alloc1(pZip, file_index, pBuf,
                                                buf_size, flags, pUser_read_buf,
                                                user_read_buf_size, NULL);
}

mz_bool mz_zip_reader_extract_file_to_mem_no_alloc(
    mz_zip_archive *pZip, const char *pFilename, void *pBuf, size_t buf_size,
    mz_uint flags, void *pUser_read_buf, size_t user_read_buf_size) {
  mz_uint32 file_index;
  if (!mz_zip_reader_locate_file_v2(pZip, pFilename, NULL, flags, &file_index))
    return MZ_FALSE;
  return mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size,
                                               flags, pUser_read_buf,
                                               user_read_buf_size);
}

mz_bool mz_zip_reader_extract_to_mem(mz_zip_archive *pZip, mz_uint file_index,
                                     void *pBuf, size_t buf_size,
                                     mz_uint flags) {
  return mz_zip_reader_extract_to_mem_no_alloc(pZip, file_index, pBuf, buf_size,
                                               flags, NULL, 0);
}

mz_bool mz_zip_reader_extract_file_to_mem(mz_zip_archive *pZip,
                                          const char *pFilename, void *pBuf,
                                          size_t buf_size, mz_uint flags) {
  return mz_zip_reader_extract_file_to_mem_no_alloc(pZip, pFilename, pBuf,
                                                    buf_size, flags, NULL, 0);
}

void *mz_zip_reader_extract_to_heap(mz_zip_archive *pZip, mz_uint file_index,
                                    size_t *pSize, mz_uint flags) {
  mz_zip_archive_file_stat file_stat;
  mz_uint64 alloc_size;
  void *pBuf;

  if (pSize)
    *pSize = 0;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return NULL;

  alloc_size = (flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ? file_stat.m_comp_size
                                                     : file_stat.m_uncomp_size;
  if (((sizeof(size_t) == sizeof(mz_uint32))) && (alloc_size > 0x7FFFFFFF)) {
    mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);
    return NULL;
  }

  if (NULL ==
      (pBuf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, (size_t)alloc_size))) {
    mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    return NULL;
  }

  if (!mz_zip_reader_extract_to_mem_no_alloc1(pZip, file_index, pBuf,
                                              (size_t)alloc_size, flags, NULL,
                                              0, &file_stat)) {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
    return NULL;
  }

  if (pSize)
    *pSize = (size_t)alloc_size;
  return pBuf;
}

void *mz_zip_reader_extract_file_to_heap(mz_zip_archive *pZip,
                                         const char *pFilename, size_t *pSize,
                                         mz_uint flags) {
  mz_uint32 file_index;
  if (!mz_zip_reader_locate_file_v2(pZip, pFilename, NULL, flags,
                                    &file_index)) {
    if (pSize)
      *pSize = 0;
    return MZ_FALSE;
  }
  return mz_zip_reader_extract_to_heap(pZip, file_index, pSize, flags);
}

mz_bool mz_zip_reader_extract_to_callback(mz_zip_archive *pZip,
                                          mz_uint file_index,
                                          mz_file_write_func pCallback,
                                          void *pOpaque, mz_uint flags) {
  int status = TINFL_STATUS_DONE;
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
  mz_uint file_crc32 = MZ_CRC32_INIT;
#endif
  mz_uint64 read_buf_size, read_buf_ofs = 0, read_buf_avail, comp_remaining,
                           out_buf_ofs = 0, cur_file_ofs;
  mz_zip_archive_file_stat file_stat;
  void *pRead_buf = NULL;
  void *pWrite_buf = NULL;
  mz_uint32
      local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) /
                       sizeof(mz_uint32)];
  mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;

  if ((!pZip) || (!pZip->m_pState) || (!pCallback) || (!pZip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;

  /* A directory or zero length file */
  if (file_stat.m_is_directory || (!file_stat.m_comp_size))
    return MZ_TRUE;

  /* Encryption and patch files are not supported. */
  if (file_stat.m_bit_flag &
      (MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED |
       MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION |
       MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION);

  /* This function only supports decompressing stored and deflate. */
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (file_stat.m_method != 0) &&
      (file_stat.m_method != MZ_DEFLATED))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_METHOD);

  /* Read and do some minimal validation of the local directory entry (this
   * doesn't crack the zip64 stuff, which we already have from the central dir)
   */
  cur_file_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pLocal_header,
                    MZ_ZIP_LOCAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  cur_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
                  MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) +
                  MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((cur_file_ofs + file_stat.m_comp_size) > pZip->m_archive_size)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  /* Decompress the file either directly from memory or from a file input
   * buffer. */
  if (pZip->m_pState->m_pMem) {
    pRead_buf = (mz_uint8 *)pZip->m_pState->m_pMem + cur_file_ofs;
    read_buf_size = read_buf_avail = file_stat.m_comp_size;
    comp_remaining = 0;
  } else {
    read_buf_size =
        MZ_MIN(file_stat.m_comp_size, (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE);
    if (NULL == (pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1,
                                            (size_t)read_buf_size)))
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

    read_buf_avail = 0;
    comp_remaining = file_stat.m_comp_size;
  }

  if ((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) || (!file_stat.m_method)) {
    /* The file is stored or the caller has requested the compressed data. */
    if (pZip->m_pState->m_pMem) {
      if (((sizeof(size_t) == sizeof(mz_uint32))) &&
          (file_stat.m_comp_size > MZ_UINT32_MAX))
        return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

      if (pCallback(pOpaque, out_buf_ofs, pRead_buf,
                    (size_t)file_stat.m_comp_size) != file_stat.m_comp_size) {
        mz_zip_set_error(pZip, MZ_ZIP_WRITE_CALLBACK_FAILED);
        status = TINFL_STATUS_FAILED;
      } else if (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) {
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
        file_crc32 =
            (mz_uint32)mz_crc32(file_crc32, (const mz_uint8 *)pRead_buf,
                                (size_t)file_stat.m_comp_size);
#endif
      }

      cur_file_ofs += file_stat.m_comp_size;
      out_buf_ofs += file_stat.m_comp_size;
      comp_remaining = 0;
    } else {
      while (comp_remaining) {
        read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
        if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf,
                          (size_t)read_buf_avail) != read_buf_avail) {
          mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
          status = TINFL_STATUS_FAILED;
          break;
        }

#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
        if (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) {
          file_crc32 = (mz_uint32)mz_crc32(
              file_crc32, (const mz_uint8 *)pRead_buf, (size_t)read_buf_avail);
        }
#endif

        if (pCallback(pOpaque, out_buf_ofs, pRead_buf,
                      (size_t)read_buf_avail) != read_buf_avail) {
          mz_zip_set_error(pZip, MZ_ZIP_WRITE_CALLBACK_FAILED);
          status = TINFL_STATUS_FAILED;
          break;
        }

        cur_file_ofs += read_buf_avail;
        out_buf_ofs += read_buf_avail;
        comp_remaining -= read_buf_avail;
      }
    }
  } else {
    tinfl_decompressor inflator;
    tinfl_init(&inflator);

    if (NULL == (pWrite_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1,
                                             TINFL_LZ_DICT_SIZE))) {
      mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
      status = TINFL_STATUS_FAILED;
    } else {
      do {
        mz_uint8 *pWrite_buf_cur =
            (mz_uint8 *)pWrite_buf + (out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));
        size_t in_buf_size,
            out_buf_size =
                TINFL_LZ_DICT_SIZE - (out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));
        if ((!read_buf_avail) && (!pZip->m_pState->m_pMem)) {
          read_buf_avail = MZ_MIN(read_buf_size, comp_remaining);
          if (pZip->m_pRead(pZip->m_pIO_opaque, cur_file_ofs, pRead_buf,
                            (size_t)read_buf_avail) != read_buf_avail) {
            mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
            status = TINFL_STATUS_FAILED;
            break;
          }
          cur_file_ofs += read_buf_avail;
          comp_remaining -= read_buf_avail;
          read_buf_ofs = 0;
        }

        in_buf_size = (size_t)read_buf_avail;
        status = tinfl_decompress(
            &inflator, (const mz_uint8 *)pRead_buf + read_buf_ofs, &in_buf_size,
            (mz_uint8 *)pWrite_buf, pWrite_buf_cur, &out_buf_size,
            comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0);
        read_buf_avail -= in_buf_size;
        read_buf_ofs += in_buf_size;

        if (out_buf_size) {
          if (pCallback(pOpaque, out_buf_ofs, pWrite_buf_cur, out_buf_size) !=
              out_buf_size) {
            mz_zip_set_error(pZip, MZ_ZIP_WRITE_CALLBACK_FAILED);
            status = TINFL_STATUS_FAILED;
            break;
          }

#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
          file_crc32 =
              (mz_uint32)mz_crc32(file_crc32, pWrite_buf_cur, out_buf_size);
#endif
          if ((out_buf_ofs += out_buf_size) > file_stat.m_uncomp_size) {
            mz_zip_set_error(pZip, MZ_ZIP_DECOMPRESSION_FAILED);
            status = TINFL_STATUS_FAILED;
            break;
          }
        }
      } while ((status == TINFL_STATUS_NEEDS_MORE_INPUT) ||
               (status == TINFL_STATUS_HAS_MORE_OUTPUT));
    }
  }

  if ((status == TINFL_STATUS_DONE) &&
      (!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA))) {
    /* Make sure the entire file was decompressed, and check its CRC. */
    if (out_buf_ofs != file_stat.m_uncomp_size) {
      mz_zip_set_error(pZip, MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE);
      status = TINFL_STATUS_FAILED;
    }
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
    else if (file_crc32 != file_stat.m_crc32) {
      mz_zip_set_error(pZip, MZ_ZIP_DECOMPRESSION_FAILED);
      status = TINFL_STATUS_FAILED;
    }
#endif
  }

  if (!pZip->m_pState->m_pMem)
    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);

  if (pWrite_buf)
    pZip->m_pFree(pZip->m_pAlloc_opaque, pWrite_buf);

  return status == TINFL_STATUS_DONE;
}

mz_bool mz_zip_reader_extract_file_to_callback(mz_zip_archive *pZip,
                                               const char *pFilename,
                                               mz_file_write_func pCallback,
                                               void *pOpaque, mz_uint flags) {
  mz_uint32 file_index;
  if (!mz_zip_reader_locate_file_v2(pZip, pFilename, NULL, flags, &file_index))
    return MZ_FALSE;

  return mz_zip_reader_extract_to_callback(pZip, file_index, pCallback, pOpaque,
                                           flags);
}

mz_zip_reader_extract_iter_state *
mz_zip_reader_extract_iter_new(mz_zip_archive *pZip, mz_uint file_index,
                               mz_uint flags) {
  mz_zip_reader_extract_iter_state *pState;
  mz_uint32
      local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) /
                       sizeof(mz_uint32)];
  mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;

  /* Argument sanity check */
  if ((!pZip) || (!pZip->m_pState))
    return NULL;

  /* Allocate an iterator status structure */
  pState = (mz_zip_reader_extract_iter_state *)pZip->m_pAlloc(
      pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_reader_extract_iter_state));
  if (!pState) {
    mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    return NULL;
  }

  /* Fetch file details */
  if (!mz_zip_reader_file_stat(pZip, file_index, &pState->file_stat)) {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
    return NULL;
  }

  /* Encryption and patch files are not supported. */
  if (pState->file_stat.m_bit_flag &
      (MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_IS_ENCRYPTED |
       MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_USES_STRONG_ENCRYPTION |
       MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_COMPRESSED_PATCH_FLAG)) {
    mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION);
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
    return NULL;
  }

  /* This function only supports decompressing stored and deflate. */
  if ((!(flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) &&
      (pState->file_stat.m_method != 0) &&
      (pState->file_stat.m_method != MZ_DEFLATED)) {
    mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_METHOD);
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
    return NULL;
  }

  /* Init state - save args */
  pState->pZip = pZip;
  pState->flags = flags;

  /* Init state - reset variables to defaults */
  pState->status = TINFL_STATUS_DONE;
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
  pState->file_crc32 = MZ_CRC32_INIT;
#endif
  pState->read_buf_ofs = 0;
  pState->out_buf_ofs = 0;
  pState->pRead_buf = NULL;
  pState->pWrite_buf = NULL;
  pState->out_blk_remain = 0;

  /* Read and parse the local directory entry. */
  pState->cur_file_ofs = pState->file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, pState->cur_file_ofs, pLocal_header,
                    MZ_ZIP_LOCAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE) {
    mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
    return NULL;
  }

  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG) {
    mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
    return NULL;
  }

  pState->cur_file_ofs +=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS) +
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  if ((pState->cur_file_ofs + pState->file_stat.m_comp_size) >
      pZip->m_archive_size) {
    mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
    return NULL;
  }

  /* Decompress the file either directly from memory or from a file input
   * buffer. */
  if (pZip->m_pState->m_pMem) {
    pState->pRead_buf =
        (mz_uint8 *)pZip->m_pState->m_pMem + pState->cur_file_ofs;
    pState->read_buf_size = pState->read_buf_avail =
        pState->file_stat.m_comp_size;
    pState->comp_remaining = pState->file_stat.m_comp_size;
  } else {
    if (!((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ||
          (!pState->file_stat.m_method))) {
      /* Decompression required, therefore intermediate read buffer required */
      pState->read_buf_size = MZ_MIN(pState->file_stat.m_comp_size,
                                     (mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE);
      if (NULL ==
          (pState->pRead_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1,
                                              (size_t)pState->read_buf_size))) {
        mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
        pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
        return NULL;
      }
    } else {
      /* Decompression not required - we will be reading directly into user
       * buffer, no temp buf required */
      pState->read_buf_size = 0;
    }
    pState->read_buf_avail = 0;
    pState->comp_remaining = pState->file_stat.m_comp_size;
  }

  if (!((flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ||
        (!pState->file_stat.m_method))) {
    /* Decompression required, init decompressor */
    tinfl_init(&pState->inflator);

    /* Allocate write buffer */
    if (NULL == (pState->pWrite_buf = pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1,
                                                     TINFL_LZ_DICT_SIZE))) {
      mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
      if (pState->pRead_buf)
        pZip->m_pFree(pZip->m_pAlloc_opaque, pState->pRead_buf);
      pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
      return NULL;
    }
  }

  return pState;
}

mz_zip_reader_extract_iter_state *
mz_zip_reader_extract_file_iter_new(mz_zip_archive *pZip, const char *pFilename,
                                    mz_uint flags) {
  mz_uint32 file_index;

  /* Locate file index by name */
  if (!mz_zip_reader_locate_file_v2(pZip, pFilename, NULL, flags, &file_index))
    return NULL;

  /* Construct iterator */
  return mz_zip_reader_extract_iter_new(pZip, file_index, flags);
}

size_t mz_zip_reader_extract_iter_read(mz_zip_reader_extract_iter_state *pState,
                                       void *pvBuf, size_t buf_size) {
  size_t copied_to_caller = 0;

  /* Argument sanity check */
  if ((!pState) || (!pState->pZip) || (!pState->pZip->m_pState) || (!pvBuf))
    return 0;

  if ((pState->flags & MZ_ZIP_FLAG_COMPRESSED_DATA) ||
      (!pState->file_stat.m_method)) {
    /* The file is stored or the caller has requested the compressed data, calc
     * amount to return. */
    copied_to_caller = (size_t)MZ_MIN(buf_size, pState->comp_remaining);

    /* Zip is in memory....or requires reading from a file? */
    if (pState->pZip->m_pState->m_pMem) {
      /* Copy data to caller's buffer */
      memcpy(pvBuf, pState->pRead_buf, copied_to_caller);
      pState->pRead_buf = ((mz_uint8 *)pState->pRead_buf) + copied_to_caller;
    } else {
      /* Read directly into caller's buffer */
      if (pState->pZip->m_pRead(pState->pZip->m_pIO_opaque,
                                pState->cur_file_ofs, pvBuf,
                                copied_to_caller) != copied_to_caller) {
        /* Failed to read all that was asked for, flag failure and alert user */
        mz_zip_set_error(pState->pZip, MZ_ZIP_FILE_READ_FAILED);
        pState->status = TINFL_STATUS_FAILED;
        copied_to_caller = 0;
      }
    }

#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
    /* Compute CRC if not returning compressed data only */
    if (!(pState->flags & MZ_ZIP_FLAG_COMPRESSED_DATA))
      pState->file_crc32 = (mz_uint32)mz_crc32(
          pState->file_crc32, (const mz_uint8 *)pvBuf, copied_to_caller);
#endif

    /* Advance offsets, dec counters */
    pState->cur_file_ofs += copied_to_caller;
    pState->out_buf_ofs += copied_to_caller;
    pState->comp_remaining -= copied_to_caller;
  } else {
    do {
      /* Calc ptr to write buffer - given current output pos and block size */
      mz_uint8 *pWrite_buf_cur =
          (mz_uint8 *)pState->pWrite_buf +
          (pState->out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));

      /* Calc max output size - given current output pos and block size */
      size_t in_buf_size,
          out_buf_size = TINFL_LZ_DICT_SIZE -
                         (pState->out_buf_ofs & (TINFL_LZ_DICT_SIZE - 1));

      if (!pState->out_blk_remain) {
        /* Read more data from file if none available (and reading from file) */
        if ((!pState->read_buf_avail) && (!pState->pZip->m_pState->m_pMem)) {
          /* Calc read size */
          pState->read_buf_avail =
              MZ_MIN(pState->read_buf_size, pState->comp_remaining);
          if (pState->pZip->m_pRead(pState->pZip->m_pIO_opaque,
                                    pState->cur_file_ofs, pState->pRead_buf,
                                    (size_t)pState->read_buf_avail) !=
              pState->read_buf_avail) {
            mz_zip_set_error(pState->pZip, MZ_ZIP_FILE_READ_FAILED);
            pState->status = TINFL_STATUS_FAILED;
            break;
          }

          /* Advance offsets, dec counters */
          pState->cur_file_ofs += pState->read_buf_avail;
          pState->comp_remaining -= pState->read_buf_avail;
          pState->read_buf_ofs = 0;
        }

        /* Perform decompression */
        in_buf_size = (size_t)pState->read_buf_avail;
        pState->status = tinfl_decompress(
            &pState->inflator,
            (const mz_uint8 *)pState->pRead_buf + pState->read_buf_ofs,
            &in_buf_size, (mz_uint8 *)pState->pWrite_buf, pWrite_buf_cur,
            &out_buf_size,
            pState->comp_remaining ? TINFL_FLAG_HAS_MORE_INPUT : 0);
        pState->read_buf_avail -= in_buf_size;
        pState->read_buf_ofs += in_buf_size;

        /* Update current output block size remaining */
        pState->out_blk_remain = out_buf_size;
      }

      if (pState->out_blk_remain) {
        /* Calc amount to return. */
        size_t to_copy =
            MZ_MIN((buf_size - copied_to_caller), pState->out_blk_remain);

        /* Copy data to caller's buffer */
        memcpy((uint8_t *)pvBuf + copied_to_caller, pWrite_buf_cur, to_copy);

#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
        /* Perform CRC */
        pState->file_crc32 =
            (mz_uint32)mz_crc32(pState->file_crc32, pWrite_buf_cur, to_copy);
#endif

        /* Decrement data consumed from block */
        pState->out_blk_remain -= to_copy;

        /* Inc output offset, while performing sanity check */
        if ((pState->out_buf_ofs += to_copy) >
            pState->file_stat.m_uncomp_size) {
          mz_zip_set_error(pState->pZip, MZ_ZIP_DECOMPRESSION_FAILED);
          pState->status = TINFL_STATUS_FAILED;
          break;
        }

        /* Increment counter of data copied to caller */
        copied_to_caller += to_copy;
      }
    } while ((copied_to_caller < buf_size) &&
             ((pState->status == TINFL_STATUS_NEEDS_MORE_INPUT) ||
              (pState->status == TINFL_STATUS_HAS_MORE_OUTPUT)));
  }

  /* Return how many bytes were copied into user buffer */
  return copied_to_caller;
}

mz_bool
mz_zip_reader_extract_iter_free(mz_zip_reader_extract_iter_state *pState) {
  int status;

  /* Argument sanity check */
  if ((!pState) || (!pState->pZip) || (!pState->pZip->m_pState))
    return MZ_FALSE;

  /* Was decompression completed and requested? */
  if ((pState->status == TINFL_STATUS_DONE) &&
      (!(pState->flags & MZ_ZIP_FLAG_COMPRESSED_DATA))) {
    /* Make sure the entire file was decompressed, and check its CRC. */
    if (pState->out_buf_ofs != pState->file_stat.m_uncomp_size) {
      mz_zip_set_error(pState->pZip, MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE);
      pState->status = TINFL_STATUS_FAILED;
    }
#ifndef MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS
    else if (pState->file_crc32 != pState->file_stat.m_crc32) {
      mz_zip_set_error(pState->pZip, MZ_ZIP_DECOMPRESSION_FAILED);
      pState->status = TINFL_STATUS_FAILED;
    }
#endif
  }

  /* Free buffers */
  if (!pState->pZip->m_pState->m_pMem)
    pState->pZip->m_pFree(pState->pZip->m_pAlloc_opaque, pState->pRead_buf);
  if (pState->pWrite_buf)
    pState->pZip->m_pFree(pState->pZip->m_pAlloc_opaque, pState->pWrite_buf);

  /* Save status */
  status = pState->status;

  /* Free context */
  pState->pZip->m_pFree(pState->pZip->m_pAlloc_opaque, pState);

  return status == TINFL_STATUS_DONE;
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_write_callback(void *pOpaque, mz_uint64 ofs,
                                         const void *pBuf, size_t n) {
  (void)ofs;

  return MZ_FWRITE(pBuf, 1, n, (MZ_FILE *)pOpaque);
}

mz_bool mz_zip_reader_extract_to_file(mz_zip_archive *pZip, mz_uint file_index,
                                      const char *pDst_filename,
                                      mz_uint flags) {
  mz_bool status;
  mz_zip_archive_file_stat file_stat;
  MZ_FILE *pFile;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;

  if (file_stat.m_is_directory || (!file_stat.m_is_supported))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_FEATURE);

  pFile = MZ_FOPEN(pDst_filename, "wb");
  if (!pFile)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);

  status = mz_zip_reader_extract_to_callback(
      pZip, file_index, mz_zip_file_write_callback, pFile, flags);

  if (MZ_FCLOSE(pFile) == EOF) {
    if (status)
      mz_zip_set_error(pZip, MZ_ZIP_FILE_CLOSE_FAILED);

    status = MZ_FALSE;
  }

#if !defined(MINIZ_NO_TIME) && !defined(MINIZ_NO_STDIO)
  if (status)
    mz_zip_set_file_times(pDst_filename, file_stat.m_time, file_stat.m_time);
#endif

  return status;
}

mz_bool mz_zip_reader_extract_file_to_file(mz_zip_archive *pZip,
                                           const char *pArchive_filename,
                                           const char *pDst_filename,
                                           mz_uint flags) {
  mz_uint32 file_index;
  if (!mz_zip_reader_locate_file_v2(pZip, pArchive_filename, NULL, flags,
                                    &file_index))
    return MZ_FALSE;

  return mz_zip_reader_extract_to_file(pZip, file_index, pDst_filename, flags);
}

mz_bool mz_zip_reader_extract_to_cfile(mz_zip_archive *pZip, mz_uint file_index,
                                       MZ_FILE *pFile, mz_uint flags) {
  mz_zip_archive_file_stat file_stat;

  if (!mz_zip_reader_file_stat(pZip, file_index, &file_stat))
    return MZ_FALSE;

  if (file_stat.m_is_directory || (!file_stat.m_is_supported))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_FEATURE);

  return mz_zip_reader_extract_to_callback(
      pZip, file_index, mz_zip_file_write_callback, pFile, flags);
}

mz_bool mz_zip_reader_extract_file_to_cfile(mz_zip_archive *pZip,
                                            const char *pArchive_filename,
                                            MZ_FILE *pFile, mz_uint flags) {
  mz_uint32 file_index;
  if (!mz_zip_reader_locate_file_v2(pZip, pArchive_filename, NULL, flags,
                                    &file_index))
    return MZ_FALSE;

  return mz_zip_reader_extract_to_cfile(pZip, file_index, pFile, flags);
}
#endif /* #ifndef MINIZ_NO_STDIO */

static size_t mz_zip_compute_crc32_callback(void *pOpaque, mz_uint64 file_ofs,
                                            const void *pBuf, size_t n) {
  mz_uint32 *p = (mz_uint32 *)pOpaque;
  (void)file_ofs;
  *p = (mz_uint32)mz_crc32(*p, (const mz_uint8 *)pBuf, n);
  return n;
}

mz_bool mz_zip_validate_file(mz_zip_archive *pZip, mz_uint file_index,
                             mz_uint flags) {
  mz_zip_archive_file_stat file_stat;
  mz_zip_internal_state *pState;
  const mz_uint8 *pCentral_dir_header;
  mz_bool found_zip64_ext_data_in_cdir = MZ_FALSE;
  mz_bool found_zip64_ext_data_in_ldir = MZ_FALSE;
  mz_uint32
      local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) /
                       sizeof(mz_uint32)];
  mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  mz_uint64 local_header_ofs = 0;
  mz_uint32 local_header_filename_len, local_header_extra_len,
      local_header_crc32;
  mz_uint64 local_header_comp_size, local_header_uncomp_size;
  mz_uint32 uncomp_crc32 = MZ_CRC32_INIT;
  mz_bool has_data_descriptor;
  mz_uint32 local_header_bit_flags;

  mz_zip_array file_data_array;
  mz_zip_array_init(&file_data_array, 1);

  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) ||
      (!pZip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (file_index > pZip->m_total_files)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pState = pZip->m_pState;

  pCentral_dir_header = mz_zip_get_cdh(pZip, file_index);

  if (!mz_zip_file_stat_internal(pZip, file_index, pCentral_dir_header,
                                 &file_stat, &found_zip64_ext_data_in_cdir))
    return MZ_FALSE;

  /* A directory or zero length file */
  if (file_stat.m_is_directory || (!file_stat.m_uncomp_size))
    return MZ_TRUE;

  /* Encryption and patch files are not supported. */
  if (file_stat.m_is_encrypted)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_ENCRYPTION);

  /* This function only supports stored and deflate. */
  if ((file_stat.m_method != 0) && (file_stat.m_method != MZ_DEFLATED))
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_METHOD);

  if (!file_stat.m_is_supported)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_FEATURE);

  /* Read and parse the local directory entry. */
  local_header_ofs = file_stat.m_local_header_ofs;
  if (pZip->m_pRead(pZip->m_pIO_opaque, local_header_ofs, pLocal_header,
                    MZ_ZIP_LOCAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  local_header_filename_len =
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS);
  local_header_extra_len =
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  local_header_comp_size =
      MZ_READ_LE32(pLocal_header + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS);
  local_header_uncomp_size =
      MZ_READ_LE32(pLocal_header + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS);
  local_header_crc32 = MZ_READ_LE32(pLocal_header + MZ_ZIP_LDH_CRC32_OFS);
  local_header_bit_flags =
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS);
  has_data_descriptor = (local_header_bit_flags & 8) != 0;

  if (local_header_filename_len != strlen(file_stat.m_filename))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  if ((local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
       local_header_filename_len + local_header_extra_len +
       file_stat.m_comp_size) > pZip->m_archive_size)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  if (!mz_zip_array_resize(
          pZip, &file_data_array,
          MZ_MAX(local_header_filename_len, local_header_extra_len),
          MZ_FALSE)) {
    mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    goto handle_failure;
  }

  if (local_header_filename_len) {
    if (pZip->m_pRead(pZip->m_pIO_opaque,
                      local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE,
                      file_data_array.m_p,
                      local_header_filename_len) != local_header_filename_len) {
      mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
      goto handle_failure;
    }

    /* I've seen 1 archive that had the same pathname, but used backslashes in
     * the local dir and forward slashes in the central dir. Do we care about
     * this? For now, this case will fail validation. */
    if (memcmp(file_stat.m_filename, file_data_array.m_p,
               local_header_filename_len) != 0) {
      mz_zip_set_error(pZip, MZ_ZIP_VALIDATION_FAILED);
      goto handle_failure;
    }
  }

  if ((local_header_extra_len) &&
      ((local_header_comp_size == MZ_UINT32_MAX) ||
       (local_header_uncomp_size == MZ_UINT32_MAX))) {
    mz_uint32 extra_size_remaining = local_header_extra_len;
    const mz_uint8 *pExtra_data = (const mz_uint8 *)file_data_array.m_p;

    if (pZip->m_pRead(pZip->m_pIO_opaque,
                      local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
                          local_header_filename_len,
                      file_data_array.m_p,
                      local_header_extra_len) != local_header_extra_len) {
      mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
      goto handle_failure;
    }

    do {
      mz_uint32 field_id, field_data_size, field_total_size;

      if (extra_size_remaining < (sizeof(mz_uint16) * 2)) {
        mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
        goto handle_failure;
      }

      field_id = MZ_READ_LE16(pExtra_data);
      field_data_size = MZ_READ_LE16(pExtra_data + sizeof(mz_uint16));
      field_total_size = field_data_size + sizeof(mz_uint16) * 2;

      if (field_total_size > extra_size_remaining) {
        mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
        goto handle_failure;
      }

      if (field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID) {
        const mz_uint8 *pSrc_field_data = pExtra_data + sizeof(mz_uint32);

        if (field_data_size < sizeof(mz_uint64) * 2) {
          mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
          goto handle_failure;
        }

        local_header_uncomp_size = MZ_READ_LE64(pSrc_field_data);
        local_header_comp_size =
            MZ_READ_LE64(pSrc_field_data + sizeof(mz_uint64));

        found_zip64_ext_data_in_ldir = MZ_TRUE;
        break;
      }

      pExtra_data += field_total_size;
      extra_size_remaining -= field_total_size;
    } while (extra_size_remaining);
  }

  /* TODO: parse local header extra data when local_header_comp_size is
   * 0xFFFFFFFF! (big_descriptor.zip) */
  /* I've seen zips in the wild with the data descriptor bit set, but proper
   * local header values and bogus data descriptors */
  if ((has_data_descriptor) && (!local_header_comp_size) &&
      (!local_header_crc32)) {
    mz_uint8 descriptor_buf[32];
    mz_bool has_id;
    const mz_uint8 *pSrc;
    mz_uint32 file_crc32;
    mz_uint64 comp_size = 0, uncomp_size = 0;

    mz_uint32 num_descriptor_uint32s =
        ((pState->m_zip64) || (found_zip64_ext_data_in_ldir)) ? 6 : 4;

    if (pZip->m_pRead(pZip->m_pIO_opaque,
                      local_header_ofs + MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
                          local_header_filename_len + local_header_extra_len +
                          file_stat.m_comp_size,
                      descriptor_buf,
                      sizeof(mz_uint32) * num_descriptor_uint32s) !=
        (sizeof(mz_uint32) * num_descriptor_uint32s)) {
      mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
      goto handle_failure;
    }

    has_id = (MZ_READ_LE32(descriptor_buf) == MZ_ZIP_DATA_DESCRIPTOR_ID);
    pSrc = has_id ? (descriptor_buf + sizeof(mz_uint32)) : descriptor_buf;

    file_crc32 = MZ_READ_LE32(pSrc);

    if ((pState->m_zip64) || (found_zip64_ext_data_in_ldir)) {
      comp_size = MZ_READ_LE64(pSrc + sizeof(mz_uint32));
      uncomp_size = MZ_READ_LE64(pSrc + sizeof(mz_uint32) + sizeof(mz_uint64));
    } else {
      comp_size = MZ_READ_LE32(pSrc + sizeof(mz_uint32));
      uncomp_size = MZ_READ_LE32(pSrc + sizeof(mz_uint32) + sizeof(mz_uint32));
    }

    if ((file_crc32 != file_stat.m_crc32) ||
        (comp_size != file_stat.m_comp_size) ||
        (uncomp_size != file_stat.m_uncomp_size)) {
      mz_zip_set_error(pZip, MZ_ZIP_VALIDATION_FAILED);
      goto handle_failure;
    }
  } else {
    if ((local_header_crc32 != file_stat.m_crc32) ||
        (local_header_comp_size != file_stat.m_comp_size) ||
        (local_header_uncomp_size != file_stat.m_uncomp_size)) {
      mz_zip_set_error(pZip, MZ_ZIP_VALIDATION_FAILED);
      goto handle_failure;
    }
  }

  mz_zip_array_clear(pZip, &file_data_array);

  if ((flags & MZ_ZIP_FLAG_VALIDATE_HEADERS_ONLY) == 0) {
    if (!mz_zip_reader_extract_to_callback(
            pZip, file_index, mz_zip_compute_crc32_callback, &uncomp_crc32, 0))
      return MZ_FALSE;

    /* 1 more check to be sure, although the extract checks too. */
    if (uncomp_crc32 != file_stat.m_crc32) {
      mz_zip_set_error(pZip, MZ_ZIP_VALIDATION_FAILED);
      return MZ_FALSE;
    }
  }

  return MZ_TRUE;

handle_failure:
  mz_zip_array_clear(pZip, &file_data_array);
  return MZ_FALSE;
}

mz_bool mz_zip_validate_archive(mz_zip_archive *pZip, mz_uint flags) {
  mz_zip_internal_state *pState;
  uint32_t i;

  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) ||
      (!pZip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pState = pZip->m_pState;

  /* Basic sanity checks */
  if (!pState->m_zip64) {
    if (pZip->m_total_files > MZ_UINT16_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);

    if (pZip->m_archive_size > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);
  } else {
    if (pZip->m_total_files >= MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);

    if (pState->m_central_dir.m_size >= MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);
  }

  for (i = 0; i < pZip->m_total_files; i++) {
    if (MZ_ZIP_FLAG_VALIDATE_LOCATE_FILE_FLAG & flags) {
      mz_uint32 found_index;
      mz_zip_archive_file_stat stat;

      if (!mz_zip_reader_file_stat(pZip, i, &stat))
        return MZ_FALSE;

      if (!mz_zip_reader_locate_file_v2(pZip, stat.m_filename, NULL, 0,
                                        &found_index))
        return MZ_FALSE;

      /* This check can fail if there are duplicate filenames in the archive
       * (which we don't check for when writing - that's up to the user) */
      if (found_index != i)
        return mz_zip_set_error(pZip, MZ_ZIP_VALIDATION_FAILED);
    }

    if (!mz_zip_validate_file(pZip, i, flags))
      return MZ_FALSE;
  }

  return MZ_TRUE;
}

mz_bool mz_zip_validate_mem_archive(const void *pMem, size_t size,
                                    mz_uint flags, mz_zip_error *pErr) {
  mz_bool success = MZ_TRUE;
  mz_zip_archive zip;
  mz_zip_error actual_err = MZ_ZIP_NO_ERROR;

  if ((!pMem) || (!size)) {
    if (pErr)
      *pErr = MZ_ZIP_INVALID_PARAMETER;
    return MZ_FALSE;
  }

  mz_zip_zero_struct(&zip);

  if (!mz_zip_reader_init_mem(&zip, pMem, size, flags)) {
    if (pErr)
      *pErr = zip.m_last_error;
    return MZ_FALSE;
  }

  if (!mz_zip_validate_archive(&zip, flags)) {
    actual_err = zip.m_last_error;
    success = MZ_FALSE;
  }

  if (!mz_zip_reader_end_internal(&zip, success)) {
    if (!actual_err)
      actual_err = zip.m_last_error;
    success = MZ_FALSE;
  }

  if (pErr)
    *pErr = actual_err;

  return success;
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_validate_file_archive(const char *pFilename, mz_uint flags,
                                     mz_zip_error *pErr) {
  mz_bool success = MZ_TRUE;
  mz_zip_archive zip;
  mz_zip_error actual_err = MZ_ZIP_NO_ERROR;

  if (!pFilename) {
    if (pErr)
      *pErr = MZ_ZIP_INVALID_PARAMETER;
    return MZ_FALSE;
  }

  mz_zip_zero_struct(&zip);

  if (!mz_zip_reader_init_file_v2(&zip, pFilename, flags, 0, 0)) {
    if (pErr)
      *pErr = zip.m_last_error;
    return MZ_FALSE;
  }

  if (!mz_zip_validate_archive(&zip, flags)) {
    actual_err = zip.m_last_error;
    success = MZ_FALSE;
  }

  if (!mz_zip_reader_end_internal(&zip, success)) {
    if (!actual_err)
      actual_err = zip.m_last_error;
    success = MZ_FALSE;
  }

  if (pErr)
    *pErr = actual_err;

  return success;
}
#endif /* #ifndef MINIZ_NO_STDIO */

/* ------------------- .ZIP archive writing */

#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS

static MZ_FORCEINLINE void mz_write_le16(mz_uint8 *p, mz_uint16 v) {
  p[0] = (mz_uint8)v;
  p[1] = (mz_uint8)(v >> 8);
}
static MZ_FORCEINLINE void mz_write_le32(mz_uint8 *p, mz_uint32 v) {
  p[0] = (mz_uint8)v;
  p[1] = (mz_uint8)(v >> 8);
  p[2] = (mz_uint8)(v >> 16);
  p[3] = (mz_uint8)(v >> 24);
}
static MZ_FORCEINLINE void mz_write_le64(mz_uint8 *p, mz_uint64 v) {
  mz_write_le32(p, (mz_uint32)v);
  mz_write_le32(p + sizeof(mz_uint32), (mz_uint32)(v >> 32));
}

#define MZ_WRITE_LE16(p, v) mz_write_le16((mz_uint8 *)(p), (mz_uint16)(v))
#define MZ_WRITE_LE32(p, v) mz_write_le32((mz_uint8 *)(p), (mz_uint32)(v))
#define MZ_WRITE_LE64(p, v) mz_write_le64((mz_uint8 *)(p), (mz_uint64)(v))

static size_t mz_zip_heap_write_func(void *pOpaque, mz_uint64 file_ofs,
                                     const void *pBuf, size_t n) {
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint64 new_size = MZ_MAX(file_ofs + n, pState->m_mem_size);

  if (!n)
    return 0;

  /* An allocation this big is likely to just fail on 32-bit systems, so don't
   * even go there. */
  if ((sizeof(size_t) == sizeof(mz_uint32)) && (new_size > 0x7FFFFFFF)) {
    mz_zip_set_error(pZip, MZ_ZIP_FILE_TOO_LARGE);
    return 0;
  }

  if (new_size > pState->m_mem_capacity) {
    void *pNew_block;
    size_t new_capacity = MZ_MAX(64, pState->m_mem_capacity);

    while (new_capacity < new_size)
      new_capacity *= 2;

    if (NULL == (pNew_block = pZip->m_pRealloc(
                     pZip->m_pAlloc_opaque, pState->m_pMem, 1, new_capacity))) {
      mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
      return 0;
    }

    pState->m_pMem = pNew_block;
    pState->m_mem_capacity = new_capacity;
  }
  memcpy((mz_uint8 *)pState->m_pMem + file_ofs, pBuf, n);
  pState->m_mem_size = (size_t)new_size;
  return n;
}

static mz_bool mz_zip_writer_end_internal(mz_zip_archive *pZip,
                                          mz_bool set_last_error) {
  mz_zip_internal_state *pState;
  mz_bool status = MZ_TRUE;

  if ((!pZip) || (!pZip->m_pState) || (!pZip->m_pAlloc) || (!pZip->m_pFree) ||
      ((pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) &&
       (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED))) {
    if (set_last_error)
      mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
    return MZ_FALSE;
  }

  pState = pZip->m_pState;
  pZip->m_pState = NULL;
  mz_zip_array_clear(pZip, &pState->m_central_dir);
  mz_zip_array_clear(pZip, &pState->m_central_dir_offsets);
  mz_zip_array_clear(pZip, &pState->m_sorted_central_dir_offsets);

#ifndef MINIZ_NO_STDIO
  if (pState->m_pFile) {
    if (pZip->m_zip_type == MZ_ZIP_TYPE_FILE) {
      if (MZ_FCLOSE(pState->m_pFile) == EOF) {
        if (set_last_error)
          mz_zip_set_error(pZip, MZ_ZIP_FILE_CLOSE_FAILED);
        status = MZ_FALSE;
      }
    }

    pState->m_pFile = NULL;
  }
#endif /* #ifndef MINIZ_NO_STDIO */

  if ((pZip->m_pWrite == mz_zip_heap_write_func) && (pState->m_pMem)) {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pState->m_pMem);
    pState->m_pMem = NULL;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pState);
  pZip->m_zip_mode = MZ_ZIP_MODE_INVALID;
  return status;
}

mz_bool mz_zip_writer_init_v2(mz_zip_archive *pZip, mz_uint64 existing_size,
                              mz_uint flags) {
  mz_bool zip64 = (flags & MZ_ZIP_FLAG_WRITE_ZIP64) != 0;

  if ((!pZip) || (pZip->m_pState) || (!pZip->m_pWrite) ||
      (pZip->m_zip_mode != MZ_ZIP_MODE_INVALID))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING) {
    if (!pZip->m_pRead)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
  }

  if (pZip->m_file_offset_alignment) {
    /* Ensure user specified file offset alignment is a power of 2. */
    if (pZip->m_file_offset_alignment & (pZip->m_file_offset_alignment - 1))
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
  }

  if (!pZip->m_pAlloc)
    pZip->m_pAlloc = miniz_def_alloc_func;
  if (!pZip->m_pFree)
    pZip->m_pFree = miniz_def_free_func;
  if (!pZip->m_pRealloc)
    pZip->m_pRealloc = miniz_def_realloc_func;

  pZip->m_archive_size = existing_size;
  pZip->m_central_directory_file_ofs = 0;
  pZip->m_total_files = 0;

  if (NULL == (pZip->m_pState = (mz_zip_internal_state *)pZip->m_pAlloc(
                   pZip->m_pAlloc_opaque, 1, sizeof(mz_zip_internal_state))))
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

  memset(pZip->m_pState, 0, sizeof(mz_zip_internal_state));

  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir,
                                sizeof(mz_uint8));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_central_dir_offsets,
                                sizeof(mz_uint32));
  MZ_ZIP_ARRAY_SET_ELEMENT_SIZE(&pZip->m_pState->m_sorted_central_dir_offsets,
                                sizeof(mz_uint32));

  pZip->m_pState->m_zip64 = zip64;
  pZip->m_pState->m_zip64_has_extended_info_fields = zip64;

  pZip->m_zip_type = MZ_ZIP_TYPE_USER;
  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_init(mz_zip_archive *pZip, mz_uint64 existing_size) {
  return mz_zip_writer_init_v2(pZip, existing_size, 0);
}

mz_bool mz_zip_writer_init_heap_v2(mz_zip_archive *pZip,
                                   size_t size_to_reserve_at_beginning,
                                   size_t initial_allocation_size,
                                   mz_uint flags) {
  pZip->m_pWrite = mz_zip_heap_write_func;
  pZip->m_pNeeds_keepalive = NULL;

  if (flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING)
    pZip->m_pRead = mz_zip_mem_read_func;

  pZip->m_pIO_opaque = pZip;

  if (!mz_zip_writer_init_v2(pZip, size_to_reserve_at_beginning, flags))
    return MZ_FALSE;

  pZip->m_zip_type = MZ_ZIP_TYPE_HEAP;

  if (0 != (initial_allocation_size = MZ_MAX(initial_allocation_size,
                                             size_to_reserve_at_beginning))) {
    if (NULL == (pZip->m_pState->m_pMem = pZip->m_pAlloc(
                     pZip->m_pAlloc_opaque, 1, initial_allocation_size))) {
      mz_zip_writer_end_internal(pZip, MZ_FALSE);
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }
    pZip->m_pState->m_mem_capacity = initial_allocation_size;
  }

  return MZ_TRUE;
}

mz_bool mz_zip_writer_init_heap(mz_zip_archive *pZip,
                                size_t size_to_reserve_at_beginning,
                                size_t initial_allocation_size) {
  return mz_zip_writer_init_heap_v2(pZip, size_to_reserve_at_beginning,
                                    initial_allocation_size, 0);
}

#ifndef MINIZ_NO_STDIO
static size_t mz_zip_file_write_func(void *pOpaque, mz_uint64 file_ofs,
                                     const void *pBuf, size_t n) {
  mz_zip_archive *pZip = (mz_zip_archive *)pOpaque;
  mz_int64 cur_ofs = MZ_FTELL64(pZip->m_pState->m_pFile);

  file_ofs += pZip->m_pState->m_file_archive_start_ofs;

  if (((mz_int64)file_ofs < 0) ||
      (((cur_ofs != (mz_int64)file_ofs)) &&
       (MZ_FSEEK64(pZip->m_pState->m_pFile, (mz_int64)file_ofs, SEEK_SET)))) {
    mz_zip_set_error(pZip, MZ_ZIP_FILE_SEEK_FAILED);
    return 0;
  }

  return MZ_FWRITE(pBuf, 1, n, pZip->m_pState->m_pFile);
}

mz_bool mz_zip_writer_init_file(mz_zip_archive *pZip, const char *pFilename,
                                mz_uint64 size_to_reserve_at_beginning) {
  return mz_zip_writer_init_file_v2(pZip, pFilename,
                                    size_to_reserve_at_beginning, 0);
}

mz_bool mz_zip_writer_init_file_v2(mz_zip_archive *pZip, const char *pFilename,
                                   mz_uint64 size_to_reserve_at_beginning,
                                   mz_uint flags) {
  MZ_FILE *pFile;

  pZip->m_pWrite = mz_zip_file_write_func;
  pZip->m_pNeeds_keepalive = NULL;

  if (flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING)
    pZip->m_pRead = mz_zip_file_read_func;

  pZip->m_pIO_opaque = pZip;

  if (!mz_zip_writer_init_v2(pZip, size_to_reserve_at_beginning, flags))
    return MZ_FALSE;

  if (NULL == (pFile = MZ_FOPEN(
                   pFilename,
                   (flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING) ? "w+b" : "wb"))) {
    mz_zip_writer_end(pZip);
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);
  }

  pZip->m_pState->m_pFile = pFile;
  pZip->m_zip_type = MZ_ZIP_TYPE_FILE;

  if (size_to_reserve_at_beginning) {
    mz_uint64 cur_ofs = 0;
    char buf[4096];

    MZ_CLEAR_OBJ(buf);

    do {
      size_t n = (size_t)MZ_MIN(sizeof(buf), size_to_reserve_at_beginning);
      if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_ofs, buf, n) != n) {
        mz_zip_writer_end(pZip);
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
      }
      cur_ofs += n;
      size_to_reserve_at_beginning -= n;
    } while (size_to_reserve_at_beginning);
  }

  return MZ_TRUE;
}

mz_bool mz_zip_writer_init_cfile(mz_zip_archive *pZip, MZ_FILE *pFile,
                                 mz_uint flags) {
  pZip->m_pWrite = mz_zip_file_write_func;
  pZip->m_pNeeds_keepalive = NULL;

  if (flags & MZ_ZIP_FLAG_WRITE_ALLOW_READING)
    pZip->m_pRead = mz_zip_file_read_func;

  pZip->m_pIO_opaque = pZip;

  if (!mz_zip_writer_init_v2(pZip, 0, flags))
    return MZ_FALSE;

  pZip->m_pState->m_pFile = pFile;
  pZip->m_pState->m_file_archive_start_ofs =
      MZ_FTELL64(pZip->m_pState->m_pFile);
  pZip->m_zip_type = MZ_ZIP_TYPE_CFILE;

  return MZ_TRUE;
}
#endif /* #ifndef MINIZ_NO_STDIO */

mz_bool mz_zip_writer_init_from_reader_v2(mz_zip_archive *pZip,
                                          const char *pFilename,
                                          mz_uint flags) {
  mz_zip_internal_state *pState;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (flags & MZ_ZIP_FLAG_WRITE_ZIP64) {
    /* We don't support converting a non-zip64 file to zip64 - this seems like
     * more trouble than it's worth. (What about the existing 32-bit data
     * descriptors that could follow the compressed data?) */
    if (!pZip->m_pState->m_zip64)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
  }

  /* No sense in trying to write to an archive that's already at the support max
   * size */
  if (pZip->m_pState->m_zip64) {
    if (pZip->m_total_files == MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  } else {
    if (pZip->m_total_files == MZ_UINT16_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);

    if ((pZip->m_archive_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
         MZ_ZIP_LOCAL_DIR_HEADER_SIZE) > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_TOO_LARGE);
  }

  pState = pZip->m_pState;

  if (pState->m_pFile) {
#ifdef MINIZ_NO_STDIO
    (void)pFilename;
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
#else
    if (pZip->m_pIO_opaque != pZip)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

    if (pZip->m_zip_type == MZ_ZIP_TYPE_FILE) {
      if (!pFilename)
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

      /* Archive is being read from stdio and was originally opened only for
       * reading. Try to reopen as writable. */
      if (NULL ==
          (pState->m_pFile = MZ_FREOPEN(pFilename, "r+b", pState->m_pFile))) {
        /* The mz_zip_archive is now in a bogus state because pState->m_pFile is
         * NULL, so just close it. */
        mz_zip_reader_end_internal(pZip, MZ_FALSE);
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);
      }
    }

    pZip->m_pWrite = mz_zip_file_write_func;
    pZip->m_pNeeds_keepalive = NULL;
#endif /* #ifdef MINIZ_NO_STDIO */
  } else if (pState->m_pMem) {
    /* Archive lives in a memory block. Assume it's from the heap that we can
     * resize using the realloc callback. */
    if (pZip->m_pIO_opaque != pZip)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

    pState->m_mem_capacity = pState->m_mem_size;
    pZip->m_pWrite = mz_zip_heap_write_func;
    pZip->m_pNeeds_keepalive = NULL;
  }
  /* Archive is being read via a user provided read function - make sure the
     user has specified a write function too. */
  else if (!pZip->m_pWrite)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  /* Start writing new files at the archive's current central directory
   * location. */
  /* TODO: We could add a flag that lets the user start writing immediately
   * AFTER the existing central dir - this would be safer. */
  pZip->m_archive_size = pZip->m_central_directory_file_ofs;
  pZip->m_central_directory_file_ofs = 0;

  /* Clear the sorted central dir offsets, they aren't useful or maintained now.
   */
  /* Even though we're now in write mode, files can still be extracted and
   * verified, but file locates will be slow. */
  /* TODO: We could easily maintain the sorted central directory offsets. */
  mz_zip_array_clear(pZip, &pZip->m_pState->m_sorted_central_dir_offsets);

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_init_from_reader_v2_noreopen(mz_zip_archive *pZip,
                                                   const char *pFilename,
                                                   mz_uint flags) {
  mz_zip_internal_state *pState;

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_READING))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (flags & MZ_ZIP_FLAG_WRITE_ZIP64) {
    /* We don't support converting a non-zip64 file to zip64 - this seems like
     * more trouble than it's worth. (What about the existing 32-bit data
     * descriptors that could follow the compressed data?) */
    if (!pZip->m_pState->m_zip64)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
  }

  /* No sense in trying to write to an archive that's already at the support max
   * size */
  if (pZip->m_pState->m_zip64) {
    if (pZip->m_total_files == MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  } else {
    if (pZip->m_total_files == MZ_UINT16_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);

    if ((pZip->m_archive_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
         MZ_ZIP_LOCAL_DIR_HEADER_SIZE) > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_TOO_LARGE);
  }

  pState = pZip->m_pState;

  if (pState->m_pFile) {
#ifdef MINIZ_NO_STDIO
    (void)pFilename;
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
#else
    if (pZip->m_pIO_opaque != pZip)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

    if (pZip->m_zip_type == MZ_ZIP_TYPE_FILE) {
      if (!pFilename)
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
    }

    pZip->m_pWrite = mz_zip_file_write_func;
    pZip->m_pNeeds_keepalive = NULL;
#endif /* #ifdef MINIZ_NO_STDIO */
  } else if (pState->m_pMem) {
    /* Archive lives in a memory block. Assume it's from the heap that we can
     * resize using the realloc callback. */
    if (pZip->m_pIO_opaque != pZip)
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

    pState->m_mem_capacity = pState->m_mem_size;
    pZip->m_pWrite = mz_zip_heap_write_func;
    pZip->m_pNeeds_keepalive = NULL;
  }
  /* Archive is being read via a user provided read function - make sure the
     user has specified a write function too. */
  else if (!pZip->m_pWrite)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  /* Start writing new files at the archive's current central directory
   * location. */
  /* TODO: We could add a flag that lets the user start writing immediately
   * AFTER the existing central dir - this would be safer. */
  pZip->m_archive_size = pZip->m_central_directory_file_ofs;
  pZip->m_central_directory_file_ofs = 0;

  /* Clear the sorted central dir offsets, they aren't useful or maintained now.
   */
  /* Even though we're now in write mode, files can still be extracted and
   * verified, but file locates will be slow. */
  /* TODO: We could easily maintain the sorted central directory offsets. */
  mz_zip_array_clear(pZip, &pZip->m_pState->m_sorted_central_dir_offsets);

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_init_from_reader(mz_zip_archive *pZip,
                                       const char *pFilename) {
  return mz_zip_writer_init_from_reader_v2(pZip, pFilename, 0);
}

/* TODO: pArchive_name is a terrible name here! */
mz_bool mz_zip_writer_add_mem(mz_zip_archive *pZip, const char *pArchive_name,
                              const void *pBuf, size_t buf_size,
                              mz_uint level_and_flags) {
  return mz_zip_writer_add_mem_ex(pZip, pArchive_name, pBuf, buf_size, NULL, 0,
                                  level_and_flags, 0, 0);
}

typedef struct {
  mz_zip_archive *m_pZip;
  mz_uint64 m_cur_archive_file_ofs;
  mz_uint64 m_comp_size;
} mz_zip_writer_add_state;

static mz_bool mz_zip_writer_add_put_buf_callback(const void *pBuf, int len,
                                                  void *pUser) {
  mz_zip_writer_add_state *pState = (mz_zip_writer_add_state *)pUser;
  if ((int)pState->m_pZip->m_pWrite(pState->m_pZip->m_pIO_opaque,
                                    pState->m_cur_archive_file_ofs, pBuf,
                                    len) != len)
    return MZ_FALSE;

  pState->m_cur_archive_file_ofs += len;
  pState->m_comp_size += len;
  return MZ_TRUE;
}

#define MZ_ZIP64_MAX_LOCAL_EXTRA_FIELD_SIZE                                    \
  (sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 2)
#define MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE                                  \
  (sizeof(mz_uint16) * 2 + sizeof(mz_uint64) * 3)
static mz_uint32
mz_zip_writer_create_zip64_extra_data(mz_uint8 *pBuf, mz_uint64 *pUncomp_size,
                                      mz_uint64 *pComp_size,
                                      mz_uint64 *pLocal_header_ofs) {
  mz_uint8 *pDst = pBuf;
  mz_uint32 field_size = 0;

  MZ_WRITE_LE16(pDst + 0, MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID);
  MZ_WRITE_LE16(pDst + 2, 0);
  pDst += sizeof(mz_uint16) * 2;

  if (pUncomp_size) {
    MZ_WRITE_LE64(pDst, *pUncomp_size);
    pDst += sizeof(mz_uint64);
    field_size += sizeof(mz_uint64);
  }

  if (pComp_size) {
    MZ_WRITE_LE64(pDst, *pComp_size);
    pDst += sizeof(mz_uint64);
    field_size += sizeof(mz_uint64);
  }

  if (pLocal_header_ofs) {
    MZ_WRITE_LE64(pDst, *pLocal_header_ofs);
    pDst += sizeof(mz_uint64);
    field_size += sizeof(mz_uint64);
  }

  MZ_WRITE_LE16(pBuf + 2, field_size);

  return (mz_uint32)(pDst - pBuf);
}

static mz_bool mz_zip_writer_create_local_dir_header(
    mz_zip_archive *pZip, mz_uint8 *pDst, mz_uint16 filename_size,
    mz_uint16 extra_size, mz_uint64 uncomp_size, mz_uint64 comp_size,
    mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags,
    mz_uint16 dos_time, mz_uint16 dos_date) {
  (void)pZip;
  memset(pDst, 0, MZ_ZIP_LOCAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_SIG_OFS, MZ_ZIP_LOCAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_VERSION_NEEDED_OFS, method ? 20 : 0);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_BIT_FLAG_OFS, bit_flags);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_METHOD_OFS, method);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILE_TIME_OFS, dos_time);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILE_DATE_OFS, dos_date);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_CRC32_OFS, uncomp_crc32);
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS,
                MZ_MIN(comp_size, MZ_UINT32_MAX));
  MZ_WRITE_LE32(pDst + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS,
                MZ_MIN(uncomp_size, MZ_UINT32_MAX));
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_FILENAME_LEN_OFS, filename_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_LDH_EXTRA_LEN_OFS, extra_size);
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_create_central_dir_header(
    mz_zip_archive *pZip, mz_uint8 *pDst, mz_uint16 filename_size,
    mz_uint16 extra_size, mz_uint16 comment_size, mz_uint64 uncomp_size,
    mz_uint64 comp_size, mz_uint32 uncomp_crc32, mz_uint16 method,
    mz_uint16 bit_flags, mz_uint16 dos_time, mz_uint16 dos_date,
    mz_uint64 local_header_ofs, mz_uint32 ext_attributes) {
  (void)pZip;
  memset(pDst, 0, MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_SIG_OFS, MZ_ZIP_CENTRAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_VERSION_NEEDED_OFS, method ? 20 : 0);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_BIT_FLAG_OFS, bit_flags);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_METHOD_OFS, method);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILE_TIME_OFS, dos_time);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILE_DATE_OFS, dos_date);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_CRC32_OFS, uncomp_crc32);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS,
                MZ_MIN(comp_size, MZ_UINT32_MAX));
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS,
                MZ_MIN(uncomp_size, MZ_UINT32_MAX));
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_FILENAME_LEN_OFS, filename_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_EXTRA_LEN_OFS, extra_size);
  MZ_WRITE_LE16(pDst + MZ_ZIP_CDH_COMMENT_LEN_OFS, comment_size);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_EXTERNAL_ATTR_OFS, ext_attributes);
  MZ_WRITE_LE32(pDst + MZ_ZIP_CDH_LOCAL_HEADER_OFS,
                MZ_MIN(local_header_ofs, MZ_UINT32_MAX));
  return MZ_TRUE;
}

static mz_bool mz_zip_writer_add_to_central_dir(
    mz_zip_archive *pZip, const char *pFilename, mz_uint16 filename_size,
    const void *pExtra, mz_uint16 extra_size, const void *pComment,
    mz_uint16 comment_size, mz_uint64 uncomp_size, mz_uint64 comp_size,
    mz_uint32 uncomp_crc32, mz_uint16 method, mz_uint16 bit_flags,
    mz_uint16 dos_time, mz_uint16 dos_date, mz_uint64 local_header_ofs,
    mz_uint32 ext_attributes, const char *user_extra_data,
    mz_uint user_extra_data_len) {
  mz_zip_internal_state *pState = pZip->m_pState;
  mz_uint32 central_dir_ofs = (mz_uint32)pState->m_central_dir.m_size;
  size_t orig_central_dir_size = pState->m_central_dir.m_size;
  mz_uint8 central_dir_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];

  if (!pZip->m_pState->m_zip64) {
    if (local_header_ofs > 0xFFFFFFFF)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_TOO_LARGE);
  }

  /* miniz doesn't support central dirs >= MZ_UINT32_MAX bytes yet */
  if (((mz_uint64)pState->m_central_dir.m_size +
       MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + filename_size + extra_size +
       user_extra_data_len + comment_size) >= MZ_UINT32_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE);

  if (!mz_zip_writer_create_central_dir_header(
          pZip, central_dir_header, filename_size,
          (mz_uint16)(extra_size + user_extra_data_len), comment_size,
          uncomp_size, comp_size, uncomp_crc32, method, bit_flags, dos_time,
          dos_date, local_header_ofs, ext_attributes))
    return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

  if ((!mz_zip_array_push_back(pZip, &pState->m_central_dir, central_dir_header,
                               MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pFilename,
                               filename_size)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pExtra,
                               extra_size)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, user_extra_data,
                               user_extra_data_len)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir, pComment,
                               comment_size)) ||
      (!mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets,
                               &central_dir_ofs, 1))) {
    /* Try to resize the central directory array back into its original state.
     */
    mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                        MZ_FALSE);
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
  }

  return MZ_TRUE;
}

static mz_bool mz_zip_writer_validate_archive_name(const char *pArchive_name) {
  /* Basic ZIP archive filename validity checks: Valid filenames cannot start
   * with a forward slash, cannot contain a drive letter, and cannot use
   * DOS-style backward slashes. */
  if (*pArchive_name == '/')
    return MZ_FALSE;

  /* Making sure the name does not contain drive letters or DOS style backward
   * slashes is the responsibility of the program using miniz*/

  return MZ_TRUE;
}

static mz_uint
mz_zip_writer_compute_padding_needed_for_file_alignment(mz_zip_archive *pZip) {
  mz_uint32 n;
  if (!pZip->m_file_offset_alignment)
    return 0;
  n = (mz_uint32)(pZip->m_archive_size & (pZip->m_file_offset_alignment - 1));
  return (mz_uint)((pZip->m_file_offset_alignment - n) &
                   (pZip->m_file_offset_alignment - 1));
}

static mz_bool mz_zip_writer_write_zeros(mz_zip_archive *pZip,
                                         mz_uint64 cur_file_ofs, mz_uint32 n) {
  char buf[4096];
  memset(buf, 0, MZ_MIN(sizeof(buf), n));
  while (n) {
    mz_uint32 s = MZ_MIN(sizeof(buf), n);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_file_ofs, buf, s) != s)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_file_ofs += s;
    n -= s;
  }
  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_mem_ex(mz_zip_archive *pZip,
                                 const char *pArchive_name, const void *pBuf,
                                 size_t buf_size, const void *pComment,
                                 mz_uint16 comment_size,
                                 mz_uint level_and_flags, mz_uint64 uncomp_size,
                                 mz_uint32 uncomp_crc32) {
  return mz_zip_writer_add_mem_ex_v2(
      pZip, pArchive_name, pBuf, buf_size, pComment, comment_size,
      level_and_flags, uncomp_size, uncomp_crc32, NULL, NULL, 0, NULL, 0);
}

mz_bool mz_zip_writer_add_mem_ex_v2(
    mz_zip_archive *pZip, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_uint64 uncomp_size, mz_uint32 uncomp_crc32,
    MZ_TIME_T *last_modified, const char *user_extra_data,
    mz_uint user_extra_data_len, const char *user_extra_data_central,
    mz_uint user_extra_data_central_len) {
  mz_uint16 method = 0, dos_time = 0, dos_date = 0;
  mz_uint level, ext_attributes = 0, num_alignment_padding_bytes;
  mz_uint64 local_dir_header_ofs = 0, cur_archive_file_ofs = 0, comp_size = 0;
  size_t archive_name_size;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  tdefl_compressor *pComp = NULL;
  mz_bool store_data_uncompressed;
  mz_zip_internal_state *pState;
  mz_uint8 *pExtra_data = NULL;
  mz_uint32 extra_size = 0;
  mz_uint8 extra_data[MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE];
  mz_uint16 bit_flags = 0;

  if ((int)level_and_flags < 0)
    level_and_flags = MZ_DEFAULT_LEVEL;

  if (uncomp_size ||
      (buf_size && !(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)))
    bit_flags |= MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR;

  if (!(level_and_flags & MZ_ZIP_FLAG_ASCII_FILENAME))
    bit_flags |= MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8;

  level = level_and_flags & 0xF;
  store_data_uncompressed =
      ((!level) || (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA));

  if ((!pZip) || (!pZip->m_pState) ||
      (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || ((buf_size) && (!pBuf)) ||
      (!pArchive_name) || ((comment_size) && (!pComment)) ||
      (level > MZ_UBER_COMPRESSION))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pState = pZip->m_pState;
  local_dir_header_ofs = pZip->m_archive_size;
  cur_archive_file_ofs = pZip->m_archive_size;

  if (pState->m_zip64) {
    if (pZip->m_total_files == MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  } else {
    if (pZip->m_total_files == MZ_UINT16_MAX) {
      pState->m_zip64 = MZ_TRUE;
      /*return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES); */
    }
    if ((buf_size > 0xFFFFFFFF) || (uncomp_size > 0xFFFFFFFF)) {
      pState->m_zip64 = MZ_TRUE;
      /*return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE); */
    }
  }

  if ((!(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) && (uncomp_size))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (!mz_zip_writer_validate_archive_name(pArchive_name))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_FILENAME);

#ifndef MINIZ_NO_TIME
  if (last_modified != NULL) {
    mz_zip_time_t_to_dos_time(*last_modified, &dos_time, &dos_date);
  } else {
    MZ_TIME_T cur_time;
    time(&cur_time);
    mz_zip_time_t_to_dos_time(cur_time, &dos_time, &dos_date);
  }
#endif /* #ifndef MINIZ_NO_TIME */

  if (!(level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) {
    uncomp_crc32 =
        (mz_uint32)mz_crc32(MZ_CRC32_INIT, (const mz_uint8 *)pBuf, buf_size);
    uncomp_size = buf_size;
    if (uncomp_size <= 3) {
      level = 0;
      store_data_uncompressed = MZ_TRUE;
    }
  }

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > MZ_UINT16_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_FILENAME);

  num_alignment_padding_bytes =
      mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  /* miniz doesn't support central dirs >= MZ_UINT32_MAX bytes yet */
  if (((mz_uint64)pState->m_central_dir.m_size +
       MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size +
       MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE + comment_size) >= MZ_UINT32_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE);

  if (!pState->m_zip64) {
    /* Bail early if the archive would obviously become too large */
    if ((pZip->m_archive_size + num_alignment_padding_bytes +
         MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size +
         MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size +
         user_extra_data_len + pState->m_central_dir.m_size +
         MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE + user_extra_data_central_len +
         MZ_ZIP_DATA_DESCRIPTER_SIZE32) > 0xFFFFFFFF) {
      pState->m_zip64 = MZ_TRUE;
      /*return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE); */
    }
  }

  if ((archive_name_size) && (pArchive_name[archive_name_size - 1] == '/')) {
    /* Set DOS Subdirectory attribute bit. */
    ext_attributes |= MZ_ZIP_DOS_DIR_ATTRIBUTE_BITFLAG;

    /* Subdirectories cannot contain data. */
    if ((buf_size) || (uncomp_size))
      return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
  }

  /* Try to do any allocations before writing to the archive, so if an
   * allocation fails the file remains unmodified. (A good idea if we're doing
   * an in-place modification.) */
  if ((!mz_zip_array_ensure_room(
          pZip, &pState->m_central_dir,
          MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size +
              (pState->m_zip64 ? MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE : 0))) ||
      (!mz_zip_array_ensure_room(pZip, &pState->m_central_dir_offsets, 1)))
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

  if ((!store_data_uncompressed) && (buf_size)) {
    if (NULL == (pComp = (tdefl_compressor *)pZip->m_pAlloc(
                     pZip->m_pAlloc_opaque, 1, sizeof(tdefl_compressor))))
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
  }

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs,
                                 num_alignment_padding_bytes)) {
    pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
    return MZ_FALSE;
  }

  local_dir_header_ofs += num_alignment_padding_bytes;
  if (pZip->m_file_offset_alignment) {
    MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) ==
              0);
  }
  cur_archive_file_ofs += num_alignment_padding_bytes;

  MZ_CLEAR_OBJ(local_dir_header);

  if (!store_data_uncompressed ||
      (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)) {
    method = MZ_DEFLATED;
  }

  if (pState->m_zip64) {
    if (uncomp_size >= MZ_UINT32_MAX || local_dir_header_ofs >= MZ_UINT32_MAX) {
      pExtra_data = extra_data;
      extra_size = mz_zip_writer_create_zip64_extra_data(
          extra_data, (uncomp_size >= MZ_UINT32_MAX) ? &uncomp_size : NULL,
          (uncomp_size >= MZ_UINT32_MAX) ? &comp_size : NULL,
          (local_dir_header_ofs >= MZ_UINT32_MAX) ? &local_dir_header_ofs
                                                  : NULL);
    }

    if (!mz_zip_writer_create_local_dir_header(
            pZip, local_dir_header, (mz_uint16)archive_name_size,
            (mz_uint16)(extra_size + user_extra_data_len), 0, 0, 0, method,
            bit_flags, dos_time, dos_date))
      return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs,
                       local_dir_header,
                       sizeof(local_dir_header)) != sizeof(local_dir_header))
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += sizeof(local_dir_header);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name,
                       archive_name_size) != archive_name_size) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }
    cur_archive_file_ofs += archive_name_size;

    if (pExtra_data != NULL) {
      if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, extra_data,
                         extra_size) != extra_size)
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

      cur_archive_file_ofs += extra_size;
    }
  } else {
    if ((comp_size > MZ_UINT32_MAX) || (cur_archive_file_ofs > MZ_UINT32_MAX))
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);
    if (!mz_zip_writer_create_local_dir_header(
            pZip, local_dir_header, (mz_uint16)archive_name_size,
            (mz_uint16)user_extra_data_len, 0, 0, 0, method, bit_flags,
            dos_time, dos_date))
      return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, local_dir_header_ofs,
                       local_dir_header,
                       sizeof(local_dir_header)) != sizeof(local_dir_header))
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += sizeof(local_dir_header);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name,
                       archive_name_size) != archive_name_size) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }
    cur_archive_file_ofs += archive_name_size;
  }

  if (user_extra_data_len > 0) {
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs,
                       user_extra_data,
                       user_extra_data_len) != user_extra_data_len)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += user_extra_data_len;
  }

  if (store_data_uncompressed) {
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pBuf,
                       buf_size) != buf_size) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }

    cur_archive_file_ofs += buf_size;
    comp_size = buf_size;
  } else if (buf_size) {
    mz_zip_writer_add_state state;

    state.m_pZip = pZip;
    state.m_cur_archive_file_ofs = cur_archive_file_ofs;
    state.m_comp_size = 0;

    if ((tdefl_init(pComp, mz_zip_writer_add_put_buf_callback, &state,
                    tdefl_create_comp_flags_from_zip_params(
                        level, -15, MZ_DEFAULT_STRATEGY)) !=
         TDEFL_STATUS_OKAY) ||
        (tdefl_compress_buffer(pComp, pBuf, buf_size, TDEFL_FINISH) !=
         TDEFL_STATUS_DONE)) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
      return mz_zip_set_error(pZip, MZ_ZIP_COMPRESSION_FAILED);
    }

    comp_size = state.m_comp_size;
    cur_archive_file_ofs = state.m_cur_archive_file_ofs;
  }

  pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
  pComp = NULL;

  if (uncomp_size) {
    mz_uint8 local_dir_footer[MZ_ZIP_DATA_DESCRIPTER_SIZE64];
    mz_uint32 local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE32;

    MZ_ASSERT(bit_flags & MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR);

    MZ_WRITE_LE32(local_dir_footer + 0, MZ_ZIP_DATA_DESCRIPTOR_ID);
    MZ_WRITE_LE32(local_dir_footer + 4, uncomp_crc32);
    if (pExtra_data == NULL) {
      if (comp_size > MZ_UINT32_MAX)
        return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);

      MZ_WRITE_LE32(local_dir_footer + 8, comp_size);
      MZ_WRITE_LE32(local_dir_footer + 12, uncomp_size);
    } else {
      MZ_WRITE_LE64(local_dir_footer + 8, comp_size);
      MZ_WRITE_LE64(local_dir_footer + 16, uncomp_size);
      local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE64;
    }

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs,
                       local_dir_footer,
                       local_dir_footer_size) != local_dir_footer_size)
      return MZ_FALSE;

    cur_archive_file_ofs += local_dir_footer_size;
  }

  if (pExtra_data != NULL) {
    extra_size = mz_zip_writer_create_zip64_extra_data(
        extra_data, (uncomp_size >= MZ_UINT32_MAX) ? &uncomp_size : NULL,
        (uncomp_size >= MZ_UINT32_MAX) ? &comp_size : NULL,
        (local_dir_header_ofs >= MZ_UINT32_MAX) ? &local_dir_header_ofs : NULL);
  }

  if (!mz_zip_writer_add_to_central_dir(
          pZip, pArchive_name, (mz_uint16)archive_name_size, pExtra_data,
          (mz_uint16)extra_size, pComment, comment_size, uncomp_size, comp_size,
          uncomp_crc32, method, bit_flags, dos_time, dos_date,
          local_dir_header_ofs, ext_attributes, user_extra_data_central,
          user_extra_data_central_len))
    return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_add_read_buf_callback(
    mz_zip_archive *pZip, const char *pArchive_name,
    mz_file_read_func read_callback, void *callback_opaque, mz_uint64 max_size,
    const MZ_TIME_T *pFile_time, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_uint32 ext_attributes,
    const char *user_extra_data, mz_uint user_extra_data_len,
    const char *user_extra_data_central, mz_uint user_extra_data_central_len) {
  mz_uint16 gen_flags = (level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE)
                            ? 0
                            : MZ_ZIP_LDH_BIT_FLAG_HAS_LOCATOR;
  mz_uint uncomp_crc32 = MZ_CRC32_INIT, level, num_alignment_padding_bytes;
  mz_uint16 method = 0, dos_time = 0, dos_date = 0;
  mz_uint64 local_dir_header_ofs, cur_archive_file_ofs = 0, uncomp_size = 0,
                                  comp_size = 0;
  size_t archive_name_size;
  mz_uint8 local_dir_header[MZ_ZIP_LOCAL_DIR_HEADER_SIZE];
  mz_uint8 *pExtra_data = NULL;
  mz_uint32 extra_size = 0;
  mz_uint8 extra_data[MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE];
  mz_zip_internal_state *pState;
  mz_uint64 file_ofs = 0, cur_archive_header_file_ofs;

  if (!(level_and_flags & MZ_ZIP_FLAG_ASCII_FILENAME))
    gen_flags |= MZ_ZIP_GENERAL_PURPOSE_BIT_FLAG_UTF8;

  if ((int)level_and_flags < 0)
    level_and_flags = MZ_DEFAULT_LEVEL;
  level = level_and_flags & 0xF;

  /* Sanity checks */
  if ((!pZip) || (!pZip->m_pState) ||
      (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || (!pArchive_name) ||
      ((comment_size) && (!pComment)) || (level > MZ_UBER_COMPRESSION))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pState = pZip->m_pState;
  cur_archive_file_ofs = pZip->m_archive_size;

  if ((!pState->m_zip64) && (max_size > MZ_UINT32_MAX)) {
    /* Source file is too large for non-zip64 */
    /*return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE); */
    pState->m_zip64 = MZ_TRUE;
  }

  /* We could support this, but why? */
  if (level_and_flags & MZ_ZIP_FLAG_COMPRESSED_DATA)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (!mz_zip_writer_validate_archive_name(pArchive_name))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_FILENAME);

  if (pState->m_zip64) {
    if (pZip->m_total_files == MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  } else {
    if (pZip->m_total_files == MZ_UINT16_MAX) {
      pState->m_zip64 = MZ_TRUE;
      /*return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES); */
    }
  }

  archive_name_size = strlen(pArchive_name);
  if (archive_name_size > MZ_UINT16_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_FILENAME);

  num_alignment_padding_bytes =
      mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  /* miniz doesn't support central dirs >= MZ_UINT32_MAX bytes yet */
  if (((mz_uint64)pState->m_central_dir.m_size +
       MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size +
       MZ_ZIP64_MAX_CENTRAL_EXTRA_FIELD_SIZE + comment_size) >= MZ_UINT32_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE);

  if (!pState->m_zip64) {
    /* Bail early if the archive would obviously become too large */
    if ((pZip->m_archive_size + num_alignment_padding_bytes +
         MZ_ZIP_LOCAL_DIR_HEADER_SIZE + archive_name_size +
         MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + archive_name_size + comment_size +
         user_extra_data_len + pState->m_central_dir.m_size +
         MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE + 1024 +
         MZ_ZIP_DATA_DESCRIPTER_SIZE32 + user_extra_data_central_len) >
        0xFFFFFFFF) {
      pState->m_zip64 = MZ_TRUE;
      /*return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE); */
    }
  }

#ifndef MINIZ_NO_TIME
  if (pFile_time) {
    mz_zip_time_t_to_dos_time(*pFile_time, &dos_time, &dos_date);
  }
#endif

  if (max_size <= 3)
    level = 0;

  if (!mz_zip_writer_write_zeros(pZip, cur_archive_file_ofs,
                                 num_alignment_padding_bytes)) {
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
  }

  cur_archive_file_ofs += num_alignment_padding_bytes;
  local_dir_header_ofs = cur_archive_file_ofs;

  if (pZip->m_file_offset_alignment) {
    MZ_ASSERT((cur_archive_file_ofs & (pZip->m_file_offset_alignment - 1)) ==
              0);
  }

  if (max_size && level) {
    method = MZ_DEFLATED;
  }

  MZ_CLEAR_OBJ(local_dir_header);
  if (pState->m_zip64) {
    if (max_size >= MZ_UINT32_MAX || local_dir_header_ofs >= MZ_UINT32_MAX) {
      pExtra_data = extra_data;
      if (level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE)
        extra_size = mz_zip_writer_create_zip64_extra_data(
            extra_data, (max_size >= MZ_UINT32_MAX) ? &uncomp_size : NULL,
            (max_size >= MZ_UINT32_MAX) ? &comp_size : NULL,
            (local_dir_header_ofs >= MZ_UINT32_MAX) ? &local_dir_header_ofs
                                                    : NULL);
      else
        extra_size = mz_zip_writer_create_zip64_extra_data(
            extra_data, NULL, NULL,
            (local_dir_header_ofs >= MZ_UINT32_MAX) ? &local_dir_header_ofs
                                                    : NULL);
    }

    if (!mz_zip_writer_create_local_dir_header(
            pZip, local_dir_header, (mz_uint16)archive_name_size,
            (mz_uint16)(extra_size + user_extra_data_len), 0, 0, 0, method,
            gen_flags, dos_time, dos_date))
      return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs,
                       local_dir_header,
                       sizeof(local_dir_header)) != sizeof(local_dir_header))
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += sizeof(local_dir_header);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name,
                       archive_name_size) != archive_name_size) {
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }

    cur_archive_file_ofs += archive_name_size;

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, extra_data,
                       extra_size) != extra_size)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += extra_size;
  } else {
    if ((comp_size > MZ_UINT32_MAX) || (cur_archive_file_ofs > MZ_UINT32_MAX))
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);
    if (!mz_zip_writer_create_local_dir_header(
            pZip, local_dir_header, (mz_uint16)archive_name_size,
            (mz_uint16)user_extra_data_len, 0, 0, 0, method, gen_flags,
            dos_time, dos_date))
      return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs,
                       local_dir_header,
                       sizeof(local_dir_header)) != sizeof(local_dir_header))
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += sizeof(local_dir_header);

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pArchive_name,
                       archive_name_size) != archive_name_size) {
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }

    cur_archive_file_ofs += archive_name_size;
  }

  if (user_extra_data_len > 0) {
    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs,
                       user_extra_data,
                       user_extra_data_len) != user_extra_data_len)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    cur_archive_file_ofs += user_extra_data_len;
  }

  if (max_size) {
    void *pRead_buf =
        pZip->m_pAlloc(pZip->m_pAlloc_opaque, 1, MZ_ZIP_MAX_IO_BUF_SIZE);
    if (!pRead_buf) {
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    if (!level) {
      while (1) {
        size_t n = read_callback(callback_opaque, file_ofs, pRead_buf,
                                 MZ_ZIP_MAX_IO_BUF_SIZE);
        if (n == 0)
          break;

        if ((n > MZ_ZIP_MAX_IO_BUF_SIZE) || (file_ofs + n > max_size)) {
          pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
          return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
        }
        if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs, pRead_buf,
                           n) != n) {
          pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
          return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
        }
        file_ofs += n;
        uncomp_crc32 =
            (mz_uint32)mz_crc32(uncomp_crc32, (const mz_uint8 *)pRead_buf, n);
        cur_archive_file_ofs += n;
      }
      uncomp_size = file_ofs;
      comp_size = uncomp_size;
    } else {
      mz_bool result = MZ_FALSE;
      mz_zip_writer_add_state state;
      tdefl_compressor *pComp = (tdefl_compressor *)pZip->m_pAlloc(
          pZip->m_pAlloc_opaque, 1, sizeof(tdefl_compressor));
      if (!pComp) {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
      }

      state.m_pZip = pZip;
      state.m_cur_archive_file_ofs = cur_archive_file_ofs;
      state.m_comp_size = 0;

      if (tdefl_init(pComp, mz_zip_writer_add_put_buf_callback, &state,
                     tdefl_create_comp_flags_from_zip_params(
                         level, -15, MZ_DEFAULT_STRATEGY)) !=
          TDEFL_STATUS_OKAY) {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);
      }

      for (;;) {
        tdefl_status status;
        tdefl_flush flush = TDEFL_NO_FLUSH;

        size_t n = read_callback(callback_opaque, file_ofs, pRead_buf,
                                 MZ_ZIP_MAX_IO_BUF_SIZE);
        if ((n > MZ_ZIP_MAX_IO_BUF_SIZE) || (file_ofs + n > max_size)) {
          mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
          break;
        }

        file_ofs += n;
        uncomp_crc32 =
            (mz_uint32)mz_crc32(uncomp_crc32, (const mz_uint8 *)pRead_buf, n);

        if (pZip->m_pNeeds_keepalive != NULL &&
            pZip->m_pNeeds_keepalive(pZip->m_pIO_opaque))
          flush = TDEFL_FULL_FLUSH;

        if (n == 0)
          flush = TDEFL_FINISH;

        status = tdefl_compress_buffer(pComp, pRead_buf, n, flush);
        if (status == TDEFL_STATUS_DONE) {
          result = MZ_TRUE;
          break;
        } else if (status != TDEFL_STATUS_OKAY) {
          mz_zip_set_error(pZip, MZ_ZIP_COMPRESSION_FAILED);
          break;
        }
      }

      pZip->m_pFree(pZip->m_pAlloc_opaque, pComp);

      if (!result) {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
        return MZ_FALSE;
      }

      uncomp_size = file_ofs;
      comp_size = state.m_comp_size;
      cur_archive_file_ofs = state.m_cur_archive_file_ofs;
    }

    pZip->m_pFree(pZip->m_pAlloc_opaque, pRead_buf);
  }

  if (!(level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE)) {
    mz_uint8 local_dir_footer[MZ_ZIP_DATA_DESCRIPTER_SIZE64];
    mz_uint32 local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE32;

    MZ_WRITE_LE32(local_dir_footer + 0, MZ_ZIP_DATA_DESCRIPTOR_ID);
    MZ_WRITE_LE32(local_dir_footer + 4, uncomp_crc32);
    if (pExtra_data == NULL) {
      if (comp_size > MZ_UINT32_MAX)
        return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);

      MZ_WRITE_LE32(local_dir_footer + 8, comp_size);
      MZ_WRITE_LE32(local_dir_footer + 12, uncomp_size);
    } else {
      MZ_WRITE_LE64(local_dir_footer + 8, comp_size);
      MZ_WRITE_LE64(local_dir_footer + 16, uncomp_size);
      local_dir_footer_size = MZ_ZIP_DATA_DESCRIPTER_SIZE64;
    }

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_file_ofs,
                       local_dir_footer,
                       local_dir_footer_size) != local_dir_footer_size)
      return MZ_FALSE;

    cur_archive_file_ofs += local_dir_footer_size;
  }

  if (level_and_flags & MZ_ZIP_FLAG_WRITE_HEADER_SET_SIZE) {
    if (pExtra_data != NULL) {
      extra_size = mz_zip_writer_create_zip64_extra_data(
          extra_data, (max_size >= MZ_UINT32_MAX) ? &uncomp_size : NULL,
          (max_size >= MZ_UINT32_MAX) ? &comp_size : NULL,
          (local_dir_header_ofs >= MZ_UINT32_MAX) ? &local_dir_header_ofs
                                                  : NULL);
    }

    if (!mz_zip_writer_create_local_dir_header(
            pZip, local_dir_header, (mz_uint16)archive_name_size,
            (mz_uint16)(extra_size + user_extra_data_len),
            (max_size >= MZ_UINT32_MAX) ? MZ_UINT32_MAX : uncomp_size,
            (max_size >= MZ_UINT32_MAX) ? MZ_UINT32_MAX : comp_size,
            uncomp_crc32, method, gen_flags, dos_time, dos_date))
      return mz_zip_set_error(pZip, MZ_ZIP_INTERNAL_ERROR);

    cur_archive_header_file_ofs = local_dir_header_ofs;

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_header_file_ofs,
                       local_dir_header,
                       sizeof(local_dir_header)) != sizeof(local_dir_header))
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    if (pExtra_data != NULL) {
      cur_archive_header_file_ofs += sizeof(local_dir_header);

      if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_header_file_ofs,
                         pArchive_name,
                         archive_name_size) != archive_name_size) {
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
      }

      cur_archive_header_file_ofs += archive_name_size;

      if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_archive_header_file_ofs,
                         extra_data, extra_size) != extra_size)
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

      cur_archive_header_file_ofs += extra_size;
    }
  }

  if (pExtra_data != NULL) {
    extra_size = mz_zip_writer_create_zip64_extra_data(
        extra_data, (uncomp_size >= MZ_UINT32_MAX) ? &uncomp_size : NULL,
        (uncomp_size >= MZ_UINT32_MAX) ? &comp_size : NULL,
        (local_dir_header_ofs >= MZ_UINT32_MAX) ? &local_dir_header_ofs : NULL);
  }

  if (!mz_zip_writer_add_to_central_dir(
          pZip, pArchive_name, (mz_uint16)archive_name_size, pExtra_data,
          (mz_uint16)extra_size, pComment, comment_size, uncomp_size, comp_size,
          uncomp_crc32, method, gen_flags, dos_time, dos_date,
          local_dir_header_ofs, ext_attributes, user_extra_data_central,
          user_extra_data_central_len))
    return MZ_FALSE;

  pZip->m_total_files++;
  pZip->m_archive_size = cur_archive_file_ofs;

  return MZ_TRUE;
}

#ifndef MINIZ_NO_STDIO

static size_t mz_file_read_func_stdio(void *pOpaque, mz_uint64 file_ofs,
                                      void *pBuf, size_t n) {
  MZ_FILE *pSrc_file = (MZ_FILE *)pOpaque;
  mz_int64 cur_ofs = MZ_FTELL64(pSrc_file);

  if (((mz_int64)file_ofs < 0) ||
      (((cur_ofs != (mz_int64)file_ofs)) &&
       (MZ_FSEEK64(pSrc_file, (mz_int64)file_ofs, SEEK_SET))))
    return 0;

  return MZ_FREAD(pBuf, 1, n, pSrc_file);
}

mz_bool mz_zip_writer_add_cfile(
    mz_zip_archive *pZip, const char *pArchive_name, MZ_FILE *pSrc_file,
    mz_uint64 max_size, const MZ_TIME_T *pFile_time, const void *pComment,
    mz_uint16 comment_size, mz_uint level_and_flags, mz_uint32 ext_attributes,
    const char *user_extra_data, mz_uint user_extra_data_len,
    const char *user_extra_data_central, mz_uint user_extra_data_central_len) {
  return mz_zip_writer_add_read_buf_callback(
      pZip, pArchive_name, mz_file_read_func_stdio, pSrc_file, max_size,
      pFile_time, pComment, comment_size, level_and_flags, ext_attributes,
      user_extra_data, user_extra_data_len, user_extra_data_central,
      user_extra_data_central_len);
}

mz_bool mz_zip_writer_add_file(mz_zip_archive *pZip, const char *pArchive_name,
                               const char *pSrc_filename, const void *pComment,
                               mz_uint16 comment_size, mz_uint level_and_flags,
                               mz_uint32 ext_attributes) {
  MZ_FILE *pSrc_file = NULL;
  mz_uint64 uncomp_size = 0;
  MZ_TIME_T file_modified_time;
  MZ_TIME_T *pFile_time = NULL;
  mz_bool status;

  memset(&file_modified_time, 0, sizeof(file_modified_time));

#if !defined(MINIZ_NO_TIME) && !defined(MINIZ_NO_STDIO)
  pFile_time = &file_modified_time;
  if (!mz_zip_get_file_modified_time(pSrc_filename, &file_modified_time))
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_STAT_FAILED);
#endif

  pSrc_file = MZ_FOPEN(pSrc_filename, "rb");
  if (!pSrc_file)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_OPEN_FAILED);

  MZ_FSEEK64(pSrc_file, 0, SEEK_END);
  uncomp_size = MZ_FTELL64(pSrc_file);
  MZ_FSEEK64(pSrc_file, 0, SEEK_SET);

  status = mz_zip_writer_add_cfile(
      pZip, pArchive_name, pSrc_file, uncomp_size, pFile_time, pComment,
      comment_size, level_and_flags, ext_attributes, NULL, 0, NULL, 0);

  MZ_FCLOSE(pSrc_file);

  return status;
}
#endif /* #ifndef MINIZ_NO_STDIO */

static mz_bool mz_zip_writer_update_zip64_extension_block(
    mz_zip_array *pNew_ext, mz_zip_archive *pZip, const mz_uint8 *pExt,
    uint32_t ext_len, mz_uint64 *pComp_size, mz_uint64 *pUncomp_size,
    mz_uint64 *pLocal_header_ofs, mz_uint32 *pDisk_start) {
  /* + 64 should be enough for any new zip64 data */
  if (!mz_zip_array_reserve(pZip, pNew_ext, ext_len + 64, MZ_FALSE))
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

  mz_zip_array_resize(pZip, pNew_ext, 0, MZ_FALSE);

  if ((pUncomp_size) || (pComp_size) || (pLocal_header_ofs) || (pDisk_start)) {
    mz_uint8 new_ext_block[64];
    mz_uint8 *pDst = new_ext_block;
    mz_write_le16(pDst, MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID);
    mz_write_le16(pDst + sizeof(mz_uint16), 0);
    pDst += sizeof(mz_uint16) * 2;

    if (pUncomp_size) {
      mz_write_le64(pDst, *pUncomp_size);
      pDst += sizeof(mz_uint64);
    }

    if (pComp_size) {
      mz_write_le64(pDst, *pComp_size);
      pDst += sizeof(mz_uint64);
    }

    if (pLocal_header_ofs) {
      mz_write_le64(pDst, *pLocal_header_ofs);
      pDst += sizeof(mz_uint64);
    }

    if (pDisk_start) {
      mz_write_le32(pDst, *pDisk_start);
      pDst += sizeof(mz_uint32);
    }

    mz_write_le16(new_ext_block + sizeof(mz_uint16),
                  (mz_uint16)((pDst - new_ext_block) - sizeof(mz_uint16) * 2));

    if (!mz_zip_array_push_back(pZip, pNew_ext, new_ext_block,
                                pDst - new_ext_block))
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
  }

  if ((pExt) && (ext_len)) {
    mz_uint32 extra_size_remaining = ext_len;
    const mz_uint8 *pExtra_data = pExt;

    do {
      mz_uint32 field_id, field_data_size, field_total_size;

      if (extra_size_remaining < (sizeof(mz_uint16) * 2))
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

      field_id = MZ_READ_LE16(pExtra_data);
      field_data_size = MZ_READ_LE16(pExtra_data + sizeof(mz_uint16));
      field_total_size = field_data_size + sizeof(mz_uint16) * 2;

      if (field_total_size > extra_size_remaining)
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

      if (field_id != MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID) {
        if (!mz_zip_array_push_back(pZip, pNew_ext, pExtra_data,
                                    field_total_size))
          return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
      }

      pExtra_data += field_total_size;
      extra_size_remaining -= field_total_size;
    } while (extra_size_remaining);
  }

  return MZ_TRUE;
}

/* TODO: This func is now pretty freakin complex due to zip64, split it up? */
mz_bool mz_zip_writer_add_from_zip_reader(mz_zip_archive *pZip,
                                          mz_zip_archive *pSource_zip,
                                          mz_uint src_file_index) {
  mz_uint n, bit_flags, num_alignment_padding_bytes,
      src_central_dir_following_data_size;
  mz_uint64 src_archive_bytes_remaining, local_dir_header_ofs;
  mz_uint64 cur_src_file_ofs, cur_dst_file_ofs;
  mz_uint32
      local_header_u32[(MZ_ZIP_LOCAL_DIR_HEADER_SIZE + sizeof(mz_uint32) - 1) /
                       sizeof(mz_uint32)];
  mz_uint8 *pLocal_header = (mz_uint8 *)local_header_u32;
  mz_uint8 new_central_header[MZ_ZIP_CENTRAL_DIR_HEADER_SIZE];
  size_t orig_central_dir_size;
  mz_zip_internal_state *pState;
  void *pBuf;
  const mz_uint8 *pSrc_central_header;
  mz_zip_archive_file_stat src_file_stat;
  mz_uint32 src_filename_len, src_comment_len, src_ext_len;
  mz_uint32 local_header_filename_size, local_header_extra_len;
  mz_uint64 local_header_comp_size, local_header_uncomp_size;
  mz_bool found_zip64_ext_data_in_ldir = MZ_FALSE;

  /* Sanity checks */
  if ((!pZip) || (!pZip->m_pState) ||
      (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING) || (!pSource_zip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pState = pZip->m_pState;

  /* Don't support copying files from zip64 archives to non-zip64, even though
   * in some cases this is possible */
  if ((pSource_zip->m_pState->m_zip64) && (!pZip->m_pState->m_zip64))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  /* Get pointer to the source central dir header and crack it */
  if (NULL ==
      (pSrc_central_header = mz_zip_get_cdh(pSource_zip, src_file_index)))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (MZ_READ_LE32(pSrc_central_header + MZ_ZIP_CDH_SIG_OFS) !=
      MZ_ZIP_CENTRAL_DIR_HEADER_SIG)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  src_filename_len =
      MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  src_comment_len =
      MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_COMMENT_LEN_OFS);
  src_ext_len = MZ_READ_LE16(pSrc_central_header + MZ_ZIP_CDH_EXTRA_LEN_OFS);
  src_central_dir_following_data_size =
      src_filename_len + src_ext_len + src_comment_len;

  /* TODO: We don't support central dir's >= MZ_UINT32_MAX bytes right now (+32
   * fudge factor in case we need to add more extra data) */
  if ((pState->m_central_dir.m_size + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
       src_central_dir_following_data_size + 32) >= MZ_UINT32_MAX)
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE);

  num_alignment_padding_bytes =
      mz_zip_writer_compute_padding_needed_for_file_alignment(pZip);

  if (!pState->m_zip64) {
    if (pZip->m_total_files == MZ_UINT16_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  } else {
    /* TODO: Our zip64 support still has some 32-bit limits that may not be
     * worth fixing. */
    if (pZip->m_total_files == MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  }

  if (!mz_zip_file_stat_internal(pSource_zip, src_file_index,
                                 pSrc_central_header, &src_file_stat, NULL))
    return MZ_FALSE;

  cur_src_file_ofs = src_file_stat.m_local_header_ofs;
  cur_dst_file_ofs = pZip->m_archive_size;

  /* Read the source archive's local dir header */
  if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs,
                           pLocal_header, MZ_ZIP_LOCAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);

  if (MZ_READ_LE32(pLocal_header) != MZ_ZIP_LOCAL_DIR_HEADER_SIG)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);

  cur_src_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE;

  /* Compute the total size we need to copy (filename+extra data+compressed
   * data) */
  local_header_filename_size =
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_FILENAME_LEN_OFS);
  local_header_extra_len =
      MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_EXTRA_LEN_OFS);
  local_header_comp_size =
      MZ_READ_LE32(pLocal_header + MZ_ZIP_LDH_COMPRESSED_SIZE_OFS);
  local_header_uncomp_size =
      MZ_READ_LE32(pLocal_header + MZ_ZIP_LDH_DECOMPRESSED_SIZE_OFS);
  src_archive_bytes_remaining = local_header_filename_size +
                                local_header_extra_len +
                                src_file_stat.m_comp_size;

  /* Try to find a zip64 extended information field */
  if ((local_header_extra_len) &&
      ((local_header_comp_size == MZ_UINT32_MAX) ||
       (local_header_uncomp_size == MZ_UINT32_MAX))) {
    mz_zip_array file_data_array;
    const mz_uint8 *pExtra_data;
    mz_uint32 extra_size_remaining = local_header_extra_len;

    mz_zip_array_init(&file_data_array, 1);
    if (!mz_zip_array_resize(pZip, &file_data_array, local_header_extra_len,
                             MZ_FALSE)) {
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque,
                             src_file_stat.m_local_header_ofs +
                                 MZ_ZIP_LOCAL_DIR_HEADER_SIZE +
                                 local_header_filename_size,
                             file_data_array.m_p, local_header_extra_len) !=
        local_header_extra_len) {
      mz_zip_array_clear(pZip, &file_data_array);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
    }

    pExtra_data = (const mz_uint8 *)file_data_array.m_p;

    do {
      mz_uint32 field_id, field_data_size, field_total_size;

      if (extra_size_remaining < (sizeof(mz_uint16) * 2)) {
        mz_zip_array_clear(pZip, &file_data_array);
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
      }

      field_id = MZ_READ_LE16(pExtra_data);
      field_data_size = MZ_READ_LE16(pExtra_data + sizeof(mz_uint16));
      field_total_size = field_data_size + sizeof(mz_uint16) * 2;

      if (field_total_size > extra_size_remaining) {
        mz_zip_array_clear(pZip, &file_data_array);
        return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
      }

      if (field_id == MZ_ZIP64_EXTENDED_INFORMATION_FIELD_HEADER_ID) {
        const mz_uint8 *pSrc_field_data = pExtra_data + sizeof(mz_uint32);

        if (field_data_size < sizeof(mz_uint64) * 2) {
          mz_zip_array_clear(pZip, &file_data_array);
          return mz_zip_set_error(pZip, MZ_ZIP_INVALID_HEADER_OR_CORRUPTED);
        }

        local_header_uncomp_size = MZ_READ_LE64(pSrc_field_data);
        local_header_comp_size = MZ_READ_LE64(
            pSrc_field_data +
            sizeof(mz_uint64)); /* may be 0 if there's a descriptor */

        found_zip64_ext_data_in_ldir = MZ_TRUE;
        break;
      }

      pExtra_data += field_total_size;
      extra_size_remaining -= field_total_size;
    } while (extra_size_remaining);

    mz_zip_array_clear(pZip, &file_data_array);
  }

  if (!pState->m_zip64) {
    /* Try to detect if the new archive will most likely wind up too big and
     * bail early (+(sizeof(mz_uint32) * 4) is for the optional descriptor which
     * could be present, +64 is a fudge factor). */
    /* We also check when the archive is finalized so this doesn't need to be
     * perfect. */
    mz_uint64 approx_new_archive_size =
        cur_dst_file_ofs + num_alignment_padding_bytes +
        MZ_ZIP_LOCAL_DIR_HEADER_SIZE + src_archive_bytes_remaining +
        (sizeof(mz_uint32) * 4) + pState->m_central_dir.m_size +
        MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + src_central_dir_following_data_size +
        MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE + 64;

    if (approx_new_archive_size >= MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);
  }

  /* Write dest archive padding */
  if (!mz_zip_writer_write_zeros(pZip, cur_dst_file_ofs,
                                 num_alignment_padding_bytes))
    return MZ_FALSE;

  cur_dst_file_ofs += num_alignment_padding_bytes;

  local_dir_header_ofs = cur_dst_file_ofs;
  if (pZip->m_file_offset_alignment) {
    MZ_ASSERT((local_dir_header_ofs & (pZip->m_file_offset_alignment - 1)) ==
              0);
  }

  /* The original zip's local header+ext block doesn't change, even with zip64,
   * so we can just copy it over to the dest zip */
  if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pLocal_header,
                     MZ_ZIP_LOCAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_LOCAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

  cur_dst_file_ofs += MZ_ZIP_LOCAL_DIR_HEADER_SIZE;

  /* Copy over the source archive bytes to the dest archive, also ensure we have
   * enough buf space to handle optional data descriptor */
  if (NULL == (pBuf = pZip->m_pAlloc(
                   pZip->m_pAlloc_opaque, 1,
                   (size_t)MZ_MAX(32U, MZ_MIN((mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE,
                                              src_archive_bytes_remaining)))))
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

  while (src_archive_bytes_remaining) {
    n = (mz_uint)MZ_MIN((mz_uint64)MZ_ZIP_MAX_IO_BUF_SIZE,
                        src_archive_bytes_remaining);
    if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs, pBuf,
                             n) != n) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
    }
    cur_src_file_ofs += n;

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }
    cur_dst_file_ofs += n;

    src_archive_bytes_remaining -= n;
  }

  /* Now deal with the optional data descriptor */
  bit_flags = MZ_READ_LE16(pLocal_header + MZ_ZIP_LDH_BIT_FLAG_OFS);
  if (bit_flags & 8) {
    /* Copy data descriptor */
    if ((pSource_zip->m_pState->m_zip64) || (found_zip64_ext_data_in_ldir)) {
      /* src is zip64, dest must be zip64 */

      /* name			uint32_t's */
      /* id				1 (optional in zip64?) */
      /* crc			1 */
      /* comp_size	2 */
      /* uncomp_size 2 */
      if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs,
                               pBuf, (sizeof(mz_uint32) * 6)) !=
          (sizeof(mz_uint32) * 6)) {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
      }

      n = sizeof(mz_uint32) *
          ((MZ_READ_LE32(pBuf) == MZ_ZIP_DATA_DESCRIPTOR_ID) ? 6 : 5);
    } else {
      /* src is NOT zip64 */
      mz_bool has_id;

      if (pSource_zip->m_pRead(pSource_zip->m_pIO_opaque, cur_src_file_ofs,
                               pBuf, sizeof(mz_uint32) * 4) !=
          sizeof(mz_uint32) * 4) {
        pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
        return mz_zip_set_error(pZip, MZ_ZIP_FILE_READ_FAILED);
      }

      has_id = (MZ_READ_LE32(pBuf) == MZ_ZIP_DATA_DESCRIPTOR_ID);

      if (pZip->m_pState->m_zip64) {
        /* dest is zip64, so upgrade the data descriptor */
        const mz_uint32 *pSrc_descriptor =
            (const mz_uint32 *)((const mz_uint8 *)pBuf +
                                (has_id ? sizeof(mz_uint32) : 0));
        const mz_uint32 src_crc32 = pSrc_descriptor[0];
        const mz_uint64 src_comp_size = pSrc_descriptor[1];
        const mz_uint64 src_uncomp_size = pSrc_descriptor[2];

        mz_write_le32((mz_uint8 *)pBuf, MZ_ZIP_DATA_DESCRIPTOR_ID);
        mz_write_le32((mz_uint8 *)pBuf + sizeof(mz_uint32) * 1, src_crc32);
        mz_write_le64((mz_uint8 *)pBuf + sizeof(mz_uint32) * 2, src_comp_size);
        mz_write_le64((mz_uint8 *)pBuf + sizeof(mz_uint32) * 4,
                      src_uncomp_size);

        n = sizeof(mz_uint32) * 6;
      } else {
        /* dest is NOT zip64, just copy it as-is */
        n = sizeof(mz_uint32) * (has_id ? 4 : 3);
      }
    }

    if (pZip->m_pWrite(pZip->m_pIO_opaque, cur_dst_file_ofs, pBuf, n) != n) {
      pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);
    }

    cur_src_file_ofs += n;
    cur_dst_file_ofs += n;
  }
  pZip->m_pFree(pZip->m_pAlloc_opaque, pBuf);

  /* Finally, add the new central dir header */
  orig_central_dir_size = pState->m_central_dir.m_size;

  memcpy(new_central_header, pSrc_central_header,
         MZ_ZIP_CENTRAL_DIR_HEADER_SIZE);

  if (pState->m_zip64) {
    /* This is the painful part: We need to write a new central dir header + ext
     * block with updated zip64 fields, and ensure the old fields (if any) are
     * not included. */
    const mz_uint8 *pSrc_ext =
        pSrc_central_header + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE + src_filename_len;
    mz_zip_array new_ext_block;

    mz_zip_array_init(&new_ext_block, sizeof(mz_uint8));

    MZ_WRITE_LE32(new_central_header + MZ_ZIP_CDH_COMPRESSED_SIZE_OFS,
                  MZ_UINT32_MAX);
    MZ_WRITE_LE32(new_central_header + MZ_ZIP_CDH_DECOMPRESSED_SIZE_OFS,
                  MZ_UINT32_MAX);
    MZ_WRITE_LE32(new_central_header + MZ_ZIP_CDH_LOCAL_HEADER_OFS,
                  MZ_UINT32_MAX);

    if (!mz_zip_writer_update_zip64_extension_block(
            &new_ext_block, pZip, pSrc_ext, src_ext_len,
            &src_file_stat.m_comp_size, &src_file_stat.m_uncomp_size,
            &local_dir_header_ofs, NULL)) {
      mz_zip_array_clear(pZip, &new_ext_block);
      return MZ_FALSE;
    }

    MZ_WRITE_LE16(new_central_header + MZ_ZIP_CDH_EXTRA_LEN_OFS,
                  new_ext_block.m_size);

    if (!mz_zip_array_push_back(pZip, &pState->m_central_dir,
                                new_central_header,
                                MZ_ZIP_CENTRAL_DIR_HEADER_SIZE)) {
      mz_zip_array_clear(pZip, &new_ext_block);
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    if (!mz_zip_array_push_back(pZip, &pState->m_central_dir,
                                pSrc_central_header +
                                    MZ_ZIP_CENTRAL_DIR_HEADER_SIZE,
                                src_filename_len)) {
      mz_zip_array_clear(pZip, &new_ext_block);
      mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                          MZ_FALSE);
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    if (!mz_zip_array_push_back(pZip, &pState->m_central_dir, new_ext_block.m_p,
                                new_ext_block.m_size)) {
      mz_zip_array_clear(pZip, &new_ext_block);
      mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                          MZ_FALSE);
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    if (!mz_zip_array_push_back(pZip, &pState->m_central_dir,
                                pSrc_central_header +
                                    MZ_ZIP_CENTRAL_DIR_HEADER_SIZE +
                                    src_filename_len + src_ext_len,
                                src_comment_len)) {
      mz_zip_array_clear(pZip, &new_ext_block);
      mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                          MZ_FALSE);
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }

    mz_zip_array_clear(pZip, &new_ext_block);
  } else {
    /* sanity checks */
    if (cur_dst_file_ofs > MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);

    if (local_dir_header_ofs >= MZ_UINT32_MAX)
      return mz_zip_set_error(pZip, MZ_ZIP_ARCHIVE_TOO_LARGE);

    MZ_WRITE_LE32(new_central_header + MZ_ZIP_CDH_LOCAL_HEADER_OFS,
                  local_dir_header_ofs);

    if (!mz_zip_array_push_back(pZip, &pState->m_central_dir,
                                new_central_header,
                                MZ_ZIP_CENTRAL_DIR_HEADER_SIZE))
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);

    if (!mz_zip_array_push_back(pZip, &pState->m_central_dir,
                                pSrc_central_header +
                                    MZ_ZIP_CENTRAL_DIR_HEADER_SIZE,
                                src_central_dir_following_data_size)) {
      mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                          MZ_FALSE);
      return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
    }
  }

  /* This shouldn't trigger unless we screwed up during the initial sanity
   * checks */
  if (pState->m_central_dir.m_size >= MZ_UINT32_MAX) {
    /* TODO: Support central dirs >= 32-bits in size */
    mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                        MZ_FALSE);
    return mz_zip_set_error(pZip, MZ_ZIP_UNSUPPORTED_CDIR_SIZE);
  }

  n = (mz_uint32)orig_central_dir_size;
  if (!mz_zip_array_push_back(pZip, &pState->m_central_dir_offsets, &n, 1)) {
    mz_zip_array_resize(pZip, &pState->m_central_dir, orig_central_dir_size,
                        MZ_FALSE);
    return mz_zip_set_error(pZip, MZ_ZIP_ALLOC_FAILED);
  }

  pZip->m_total_files++;
  pZip->m_archive_size = cur_dst_file_ofs;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_finalize_archive(mz_zip_archive *pZip) {
  mz_zip_internal_state *pState;
  mz_uint64 central_dir_ofs, central_dir_size;
  mz_uint8 hdr[256];

  if ((!pZip) || (!pZip->m_pState) || (pZip->m_zip_mode != MZ_ZIP_MODE_WRITING))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  pState = pZip->m_pState;

  if (pState->m_zip64) {
    if ((pZip->m_total_files > MZ_UINT32_MAX) ||
        (pState->m_central_dir.m_size >= MZ_UINT32_MAX))
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  } else {
    if ((pZip->m_total_files > MZ_UINT16_MAX) ||
        ((pZip->m_archive_size + pState->m_central_dir.m_size +
          MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) > MZ_UINT32_MAX))
      return mz_zip_set_error(pZip, MZ_ZIP_TOO_MANY_FILES);
  }

  central_dir_ofs = 0;
  central_dir_size = 0;
  if (pZip->m_total_files) {
    /* Write central directory */
    central_dir_ofs = pZip->m_archive_size;
    central_dir_size = pState->m_central_dir.m_size;
    pZip->m_central_directory_file_ofs = central_dir_ofs;
    if (pZip->m_pWrite(pZip->m_pIO_opaque, central_dir_ofs,
                       pState->m_central_dir.m_p,
                       (size_t)central_dir_size) != central_dir_size)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    pZip->m_archive_size += central_dir_size;
  }

  if (pState->m_zip64) {
    /* Write zip64 end of central directory header */
    mz_uint64 rel_ofs_to_zip64_ecdr = pZip->m_archive_size;

    MZ_CLEAR_OBJ(hdr);
    MZ_WRITE_LE32(hdr + MZ_ZIP64_ECDH_SIG_OFS,
                  MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIG);
    MZ_WRITE_LE64(hdr + MZ_ZIP64_ECDH_SIZE_OF_RECORD_OFS,
                  MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE - sizeof(mz_uint32) -
                      sizeof(mz_uint64));
    MZ_WRITE_LE16(hdr + MZ_ZIP64_ECDH_VERSION_MADE_BY_OFS,
                  0x031E); /* TODO: always Unix */
    MZ_WRITE_LE16(hdr + MZ_ZIP64_ECDH_VERSION_NEEDED_OFS, 0x002D);
    MZ_WRITE_LE64(hdr + MZ_ZIP64_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS,
                  pZip->m_total_files);
    MZ_WRITE_LE64(hdr + MZ_ZIP64_ECDH_CDIR_TOTAL_ENTRIES_OFS,
                  pZip->m_total_files);
    MZ_WRITE_LE64(hdr + MZ_ZIP64_ECDH_CDIR_SIZE_OFS, central_dir_size);
    MZ_WRITE_LE64(hdr + MZ_ZIP64_ECDH_CDIR_OFS_OFS, central_dir_ofs);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, hdr,
                       MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE) !=
        MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    pZip->m_archive_size += MZ_ZIP64_END_OF_CENTRAL_DIR_HEADER_SIZE;

    /* Write zip64 end of central directory locator */
    MZ_CLEAR_OBJ(hdr);
    MZ_WRITE_LE32(hdr + MZ_ZIP64_ECDL_SIG_OFS,
                  MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG);
    MZ_WRITE_LE64(hdr + MZ_ZIP64_ECDL_REL_OFS_TO_ZIP64_ECDR_OFS,
                  rel_ofs_to_zip64_ecdr);
    MZ_WRITE_LE32(hdr + MZ_ZIP64_ECDL_TOTAL_NUMBER_OF_DISKS_OFS, 1);
    if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, hdr,
                       MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE) !=
        MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE)
      return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

    pZip->m_archive_size += MZ_ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIZE;
  }

  /* Write end of central directory record */
  MZ_CLEAR_OBJ(hdr);
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_SIG_OFS,
                MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIG);
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_CDIR_NUM_ENTRIES_ON_DISK_OFS,
                MZ_MIN(MZ_UINT16_MAX, pZip->m_total_files));
  MZ_WRITE_LE16(hdr + MZ_ZIP_ECDH_CDIR_TOTAL_ENTRIES_OFS,
                MZ_MIN(MZ_UINT16_MAX, pZip->m_total_files));
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_CDIR_SIZE_OFS,
                MZ_MIN(MZ_UINT32_MAX, central_dir_size));
  MZ_WRITE_LE32(hdr + MZ_ZIP_ECDH_CDIR_OFS_OFS,
                MZ_MIN(MZ_UINT32_MAX, central_dir_ofs));

  if (pZip->m_pWrite(pZip->m_pIO_opaque, pZip->m_archive_size, hdr,
                     MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE) !=
      MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE)
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_WRITE_FAILED);

#ifndef MINIZ_NO_STDIO
  if ((pState->m_pFile) && (MZ_FFLUSH(pState->m_pFile) == EOF))
    return mz_zip_set_error(pZip, MZ_ZIP_FILE_CLOSE_FAILED);
#endif /* #ifndef MINIZ_NO_STDIO */

  pZip->m_archive_size += MZ_ZIP_END_OF_CENTRAL_DIR_HEADER_SIZE;

  pZip->m_zip_mode = MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED;
  return MZ_TRUE;
}

mz_bool mz_zip_writer_finalize_heap_archive(mz_zip_archive *pZip, void **ppBuf,
                                            size_t *pSize) {
  if ((!ppBuf) || (!pSize))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  *ppBuf = NULL;
  *pSize = 0;

  if ((!pZip) || (!pZip->m_pState))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (pZip->m_pWrite != mz_zip_heap_write_func)
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  if (!mz_zip_writer_finalize_archive(pZip))
    return MZ_FALSE;

  *ppBuf = pZip->m_pState->m_pMem;
  *pSize = pZip->m_pState->m_mem_size;
  pZip->m_pState->m_pMem = NULL;
  pZip->m_pState->m_mem_size = pZip->m_pState->m_mem_capacity = 0;

  return MZ_TRUE;
}

mz_bool mz_zip_writer_end(mz_zip_archive *pZip) {
  return mz_zip_writer_end_internal(pZip, MZ_TRUE);
}

#ifndef MINIZ_NO_STDIO
mz_bool mz_zip_add_mem_to_archive_file_in_place(
    const char *pZip_filename, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags) {
  return mz_zip_add_mem_to_archive_file_in_place_v2(
      pZip_filename, pArchive_name, pBuf, buf_size, pComment, comment_size,
      level_and_flags, NULL);
}

mz_bool mz_zip_add_mem_to_archive_file_in_place_v2(
    const char *pZip_filename, const char *pArchive_name, const void *pBuf,
    size_t buf_size, const void *pComment, mz_uint16 comment_size,
    mz_uint level_and_flags, mz_zip_error *pErr) {
  mz_bool status, created_new_archive = MZ_FALSE;
  mz_zip_archive zip_archive;
  struct MZ_FILE_STAT_STRUCT file_stat;
  mz_zip_error actual_err = MZ_ZIP_NO_ERROR;

  mz_zip_zero_struct(&zip_archive);
  if ((int)level_and_flags < 0)
    level_and_flags = MZ_DEFAULT_LEVEL;

  if ((!pZip_filename) || (!pArchive_name) || ((buf_size) && (!pBuf)) ||
      ((comment_size) && (!pComment)) ||
      ((level_and_flags & 0xF) > MZ_UBER_COMPRESSION)) {
    if (pErr)
      *pErr = MZ_ZIP_INVALID_PARAMETER;
    return MZ_FALSE;
  }

  if (!mz_zip_writer_validate_archive_name(pArchive_name)) {
    if (pErr)
      *pErr = MZ_ZIP_INVALID_FILENAME;
    return MZ_FALSE;
  }

  /* Important: The regular non-64 bit version of stat() can fail here if the
   * file is very large, which could cause the archive to be overwritten. */
  /* So be sure to compile with _LARGEFILE64_SOURCE 1 */
  if (MZ_FILE_STAT(pZip_filename, &file_stat) != 0) {
    /* Create a new archive. */
    if (!mz_zip_writer_init_file_v2(&zip_archive, pZip_filename, 0,
                                    level_and_flags)) {
      if (pErr)
        *pErr = zip_archive.m_last_error;
      return MZ_FALSE;
    }

    created_new_archive = MZ_TRUE;
  } else {
    /* Append to an existing archive. */
    if (!mz_zip_reader_init_file_v2(
            &zip_archive, pZip_filename,
            level_and_flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY, 0,
            0)) {
      if (pErr)
        *pErr = zip_archive.m_last_error;
      return MZ_FALSE;
    }

    if (!mz_zip_writer_init_from_reader_v2(&zip_archive, pZip_filename,
                                           level_and_flags)) {
      if (pErr)
        *pErr = zip_archive.m_last_error;

      mz_zip_reader_end_internal(&zip_archive, MZ_FALSE);

      return MZ_FALSE;
    }
  }

  status =
      mz_zip_writer_add_mem_ex(&zip_archive, pArchive_name, pBuf, buf_size,
                               pComment, comment_size, level_and_flags, 0, 0);
  actual_err = zip_archive.m_last_error;

  /* Always finalize, even if adding failed for some reason, so we have a valid
   * central directory. (This may not always succeed, but we can try.) */
  if (!mz_zip_writer_finalize_archive(&zip_archive)) {
    if (!actual_err)
      actual_err = zip_archive.m_last_error;

    status = MZ_FALSE;
  }

  if (!mz_zip_writer_end_internal(&zip_archive, status)) {
    if (!actual_err)
      actual_err = zip_archive.m_last_error;

    status = MZ_FALSE;
  }

  if ((!status) && (created_new_archive)) {
    /* It's a new archive and something went wrong, so just delete it. */
    int ignoredStatus = MZ_DELETE_FILE(pZip_filename);
    (void)ignoredStatus;
  }

  if (pErr)
    *pErr = actual_err;

  return status;
}

void *mz_zip_extract_archive_file_to_heap_v2(const char *pZip_filename,
                                             const char *pArchive_name,
                                             const char *pComment,
                                             size_t *pSize, mz_uint flags,
                                             mz_zip_error *pErr) {
  mz_uint32 file_index;
  mz_zip_archive zip_archive;
  void *p = NULL;

  if (pSize)
    *pSize = 0;

  if ((!pZip_filename) || (!pArchive_name)) {
    if (pErr)
      *pErr = MZ_ZIP_INVALID_PARAMETER;

    return NULL;
  }

  mz_zip_zero_struct(&zip_archive);
  if (!mz_zip_reader_init_file_v2(
          &zip_archive, pZip_filename,
          flags | MZ_ZIP_FLAG_DO_NOT_SORT_CENTRAL_DIRECTORY, 0, 0)) {
    if (pErr)
      *pErr = zip_archive.m_last_error;

    return NULL;
  }

  if (mz_zip_reader_locate_file_v2(&zip_archive, pArchive_name, pComment, flags,
                                   &file_index)) {
    p = mz_zip_reader_extract_to_heap(&zip_archive, file_index, pSize, flags);
  }

  mz_zip_reader_end_internal(&zip_archive, p != NULL);

  if (pErr)
    *pErr = zip_archive.m_last_error;

  return p;
}

void *mz_zip_extract_archive_file_to_heap(const char *pZip_filename,
                                          const char *pArchive_name,
                                          size_t *pSize, mz_uint flags) {
  return mz_zip_extract_archive_file_to_heap_v2(pZip_filename, pArchive_name,
                                                NULL, pSize, flags, NULL);
}

#endif /* #ifndef MINIZ_NO_STDIO */

#endif /* #ifndef MINIZ_NO_ARCHIVE_WRITING_APIS */

/* ------------------- Misc utils */

mz_zip_mode mz_zip_get_mode(mz_zip_archive *pZip) {
  return pZip ? pZip->m_zip_mode : MZ_ZIP_MODE_INVALID;
}

mz_zip_type mz_zip_get_type(mz_zip_archive *pZip) {
  return pZip ? pZip->m_zip_type : MZ_ZIP_TYPE_INVALID;
}

mz_zip_error mz_zip_set_last_error(mz_zip_archive *pZip, mz_zip_error err_num) {
  mz_zip_error prev_err;

  if (!pZip)
    return MZ_ZIP_INVALID_PARAMETER;

  prev_err = pZip->m_last_error;

  pZip->m_last_error = err_num;
  return prev_err;
}

mz_zip_error mz_zip_peek_last_error(mz_zip_archive *pZip) {
  if (!pZip)
    return MZ_ZIP_INVALID_PARAMETER;

  return pZip->m_last_error;
}

mz_zip_error mz_zip_clear_last_error(mz_zip_archive *pZip) {
  return mz_zip_set_last_error(pZip, MZ_ZIP_NO_ERROR);
}

mz_zip_error mz_zip_get_last_error(mz_zip_archive *pZip) {
  mz_zip_error prev_err;

  if (!pZip)
    return MZ_ZIP_INVALID_PARAMETER;

  prev_err = pZip->m_last_error;

  pZip->m_last_error = MZ_ZIP_NO_ERROR;
  return prev_err;
}

const char *mz_zip_get_error_string(mz_zip_error mz_err) {
  switch (mz_err) {
  case MZ_ZIP_NO_ERROR:
    return "no error";
  case MZ_ZIP_UNDEFINED_ERROR:
    return "undefined error";
  case MZ_ZIP_TOO_MANY_FILES:
    return "too many files";
  case MZ_ZIP_FILE_TOO_LARGE:
    return "file too large";
  case MZ_ZIP_UNSUPPORTED_METHOD:
    return "unsupported method";
  case MZ_ZIP_UNSUPPORTED_ENCRYPTION:
    return "unsupported encryption";
  case MZ_ZIP_UNSUPPORTED_FEATURE:
    return "unsupported feature";
  case MZ_ZIP_FAILED_FINDING_CENTRAL_DIR:
    return "failed finding central directory";
  case MZ_ZIP_NOT_AN_ARCHIVE:
    return "not a ZIP archive";
  case MZ_ZIP_INVALID_HEADER_OR_CORRUPTED:
    return "invalid header or archive is corrupted";
  case MZ_ZIP_UNSUPPORTED_MULTIDISK:
    return "unsupported multidisk archive";
  case MZ_ZIP_DECOMPRESSION_FAILED:
    return "decompression failed or archive is corrupted";
  case MZ_ZIP_COMPRESSION_FAILED:
    return "compression failed";
  case MZ_ZIP_UNEXPECTED_DECOMPRESSED_SIZE:
    return "unexpected decompressed size";
  case MZ_ZIP_CRC_CHECK_FAILED:
    return "CRC-32 check failed";
  case MZ_ZIP_UNSUPPORTED_CDIR_SIZE:
    return "unsupported central directory size";
  case MZ_ZIP_ALLOC_FAILED:
    return "allocation failed";
  case MZ_ZIP_FILE_OPEN_FAILED:
    return "file open failed";
  case MZ_ZIP_FILE_CREATE_FAILED:
    return "file create failed";
  case MZ_ZIP_FILE_WRITE_FAILED:
    return "file write failed";
  case MZ_ZIP_FILE_READ_FAILED:
    return "file read failed";
  case MZ_ZIP_FILE_CLOSE_FAILED:
    return "file close failed";
  case MZ_ZIP_FILE_SEEK_FAILED:
    return "file seek failed";
  case MZ_ZIP_FILE_STAT_FAILED:
    return "file stat failed";
  case MZ_ZIP_INVALID_PARAMETER:
    return "invalid parameter";
  case MZ_ZIP_INVALID_FILENAME:
    return "invalid filename";
  case MZ_ZIP_BUF_TOO_SMALL:
    return "buffer too small";
  case MZ_ZIP_INTERNAL_ERROR:
    return "internal error";
  case MZ_ZIP_FILE_NOT_FOUND:
    return "file not found";
  case MZ_ZIP_ARCHIVE_TOO_LARGE:
    return "archive is too large";
  case MZ_ZIP_VALIDATION_FAILED:
    return "validation failed";
  case MZ_ZIP_WRITE_CALLBACK_FAILED:
    return "write callback failed";
  case MZ_ZIP_TOTAL_ERRORS:
    return "total errors";
  default:
    break;
  }

  return "unknown error";
}

/* Note: Just because the archive is not zip64 doesn't necessarily mean it
 * doesn't have Zip64 extended information extra field, argh. */
mz_bool mz_zip_is_zip64(mz_zip_archive *pZip) {
  if ((!pZip) || (!pZip->m_pState))
    return MZ_FALSE;

  return pZip->m_pState->m_zip64;
}

size_t mz_zip_get_central_dir_size(mz_zip_archive *pZip) {
  if ((!pZip) || (!pZip->m_pState))
    return 0;

  return pZip->m_pState->m_central_dir.m_size;
}

mz_uint mz_zip_reader_get_num_files(mz_zip_archive *pZip) {
  return pZip ? pZip->m_total_files : 0;
}

mz_uint64 mz_zip_get_archive_size(mz_zip_archive *pZip) {
  if (!pZip)
    return 0;
  return pZip->m_archive_size;
}

mz_uint64 mz_zip_get_archive_file_start_offset(mz_zip_archive *pZip) {
  if ((!pZip) || (!pZip->m_pState))
    return 0;
  return pZip->m_pState->m_file_archive_start_ofs;
}

MZ_FILE *mz_zip_get_cfile(mz_zip_archive *pZip) {
  if ((!pZip) || (!pZip->m_pState))
    return 0;
  return pZip->m_pState->m_pFile;
}

size_t mz_zip_read_archive_data(mz_zip_archive *pZip, mz_uint64 file_ofs,
                                void *pBuf, size_t n) {
  if ((!pZip) || (!pZip->m_pState) || (!pBuf) || (!pZip->m_pRead))
    return mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);

  return pZip->m_pRead(pZip->m_pIO_opaque, file_ofs, pBuf, n);
}

mz_uint mz_zip_reader_get_filename(mz_zip_archive *pZip, mz_uint file_index,
                                   char *pFilename, mz_uint filename_buf_size) {
  mz_uint n;
  const mz_uint8 *p = mz_zip_get_cdh(pZip, file_index);
  if (!p) {
    if (filename_buf_size)
      pFilename[0] = '\0';
    mz_zip_set_error(pZip, MZ_ZIP_INVALID_PARAMETER);
    return 0;
  }
  n = MZ_READ_LE16(p + MZ_ZIP_CDH_FILENAME_LEN_OFS);
  if (filename_buf_size) {
    n = MZ_MIN(n, filename_buf_size - 1);
    memcpy(pFilename, p + MZ_ZIP_CENTRAL_DIR_HEADER_SIZE, n);
    pFilename[n] = '\0';
  }
  return n + 1;
}

mz_bool mz_zip_reader_file_stat(mz_zip_archive *pZip, mz_uint file_index,
                                mz_zip_archive_file_stat *pStat) {
  return mz_zip_file_stat_internal(
      pZip, file_index, mz_zip_get_cdh(pZip, file_index), pStat, NULL);
}

mz_bool mz_zip_end(mz_zip_archive *pZip) {
  if (!pZip)
    return MZ_FALSE;

  if (pZip->m_zip_mode == MZ_ZIP_MODE_READING)
    return mz_zip_reader_end(pZip);
#ifndef MINIZ_NO_ARCHIVE_WRITING_APIS
  else if ((pZip->m_zip_mode == MZ_ZIP_MODE_WRITING) ||
           (pZip->m_zip_mode == MZ_ZIP_MODE_WRITING_HAS_BEEN_FINALIZED))
    return mz_zip_writer_end(pZip);
#endif

  return MZ_FALSE;
}

#ifdef __cplusplus
}
#endif

#endif /*#ifndef MINIZ_NO_ARCHIVE_APIS*/

/*
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once
#ifndef ZIP_H
#define ZIP_H

#include <stdint.h>
#include <string.h>
#include <sys/types.h>

#ifndef ZIP_SHARED
#define ZIP_EXPORT
#else
#ifdef _WIN32
#ifdef ZIP_BUILD_SHARED
#define ZIP_EXPORT __declspec(dllexport)
#else
#define ZIP_EXPORT __declspec(dllimport)
#endif
#else
#define ZIP_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(_POSIX_C_SOURCE) && defined(_MSC_VER)
// 64-bit Windows is the only mainstream platform
// where sizeof(long) != sizeof(void*)
#ifdef _WIN64
typedef long long ssize_t; /* byte count or error */
#else
typedef long ssize_t; /* byte count or error */
#endif
#endif

#ifndef MAX_PATH
#define MAX_PATH 1024 /* # chars in a path name including NULL */
#endif

/**
 * @mainpage
 *
 * Documenation for @ref zip.
 */

/**
 * @addtogroup zip
 * @{
 */

/**
 * Default zip compression level.
 */
#define ZIP_DEFAULT_COMPRESSION_LEVEL 6

/**
 * Error codes
 */
#define ZIP_ENOINIT -1      // not initialized
#define ZIP_EINVENTNAME -2  // invalid entry name
#define ZIP_ENOENT -3       // entry not found
#define ZIP_EINVMODE -4     // invalid zip mode
#define ZIP_EINVLVL -5      // invalid compression level
#define ZIP_ENOSUP64 -6     // no zip 64 support
#define ZIP_EMEMSET -7      // memset error
#define ZIP_EWRTENT -8      // cannot write data to entry
#define ZIP_ETDEFLINIT -9   // cannot initialize tdefl compressor
#define ZIP_EINVIDX -10     // invalid index
#define ZIP_ENOHDR -11      // header not found
#define ZIP_ETDEFLBUF -12   // cannot flush tdefl buffer
#define ZIP_ECRTHDR -13     // cannot create entry header
#define ZIP_EWRTHDR -14     // cannot write entry header
#define ZIP_EWRTDIR -15     // cannot write to central dir
#define ZIP_EOPNFILE -16    // cannot open file
#define ZIP_EINVENTTYPE -17 // invalid entry type
#define ZIP_EMEMNOALLOC -18 // extracting data using no memory allocation
#define ZIP_ENOFILE -19     // file not found
#define ZIP_ENOPERM -20     // no permission
#define ZIP_EOOMEM -21      // out of memory
#define ZIP_EINVZIPNAME -22 // invalid zip archive name
#define ZIP_EMKDIR -23      // make dir error
#define ZIP_ESYMLINK -24    // symlink error
#define ZIP_ECLSZIP -25     // close archive error
#define ZIP_ECAPSIZE -26    // capacity size too small
#define ZIP_EFSEEK -27      // fseek error
#define ZIP_EFREAD -28      // fread error
#define ZIP_EFWRITE -29     // fwrite error

/**
 * Looks up the error message string coresponding to an error number.
 * @param errnum error number
 * @return error message string coresponding to errnum or NULL if error is not
 * found.
 */
extern ZIP_EXPORT const char *zip_strerror(int errnum);

/**
 * @struct zip_t
 *
 * This data structure is used throughout the library to represent zip archive -
 * forward declaration.
 */
struct zip_t;

/**
 * Opens zip archive with compression level using the given mode.
 *
 * @param zipname zip archive file name.
 * @param level compression level (0-9 are the standard zlib-style levels).
 * @param mode file access mode.
 *        - 'r': opens a file for reading/extracting (the file must exists).
 *        - 'w': creates an empty file for writing.
 *        - 'a': appends to an existing archive.
 *
 * @return the zip archive handler or NULL on error
 */
extern ZIP_EXPORT struct zip_t *zip_open(const char *zipname, int level,
                                         char mode);

/**
 * Closes the zip archive, releases resources - always finalize.
 *
 * @param zip zip archive handler.
 */
extern ZIP_EXPORT void zip_close(struct zip_t *zip);

/**
 * Determines if the archive has a zip64 end of central directory headers.
 *
 * @param zip zip archive handler.
 *
 * @return the return code - 1 (true), 0 (false), negative number (< 0) on
 *         error.
 */
extern ZIP_EXPORT int zip_is64(struct zip_t *zip);

/**
 * Opens an entry by name in the zip archive.
 *
 * For zip archive opened in 'w' or 'a' mode the function will append
 * a new entry. In readonly mode the function tries to locate the entry
 * in global dictionary.
 *
 * @param zip zip archive handler.
 * @param entryname an entry name in local dictionary.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_open(struct zip_t *zip, const char *entryname);

/**
 * Opens an entry by name in the zip archive.
 *
 * For zip archive opened in 'w' or 'a' mode the function will append
 * a new entry. In readonly mode the function tries to locate the entry
 * in global dictionary (case sensitive).
 *
 * @param zip zip archive handler.
 * @param entryname an entry name in local dictionary (case sensitive).
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_opencasesensitive(struct zip_t *zip,
                                                  const char *entryname);

/**
 * Opens a new entry by index in the zip archive.
 *
 * This function is only valid if zip archive was opened in 'r' (readonly) mode.
 *
 * @param zip zip archive handler.
 * @param index index in local dictionary.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_openbyindex(struct zip_t *zip, size_t index);

/**
 * Closes a zip entry, flushes buffer and releases resources.
 *
 * @param zip zip archive handler.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_close(struct zip_t *zip);

/**
 * Returns a local name of the current zip entry.
 *
 * The main difference between user's entry name and local entry name
 * is optional relative path.
 * Following .ZIP File Format Specification - the path stored MUST not contain
 * a drive or device letter, or a leading slash.
 * All slashes MUST be forward slashes '/' as opposed to backwards slashes '\'
 * for compatibility with Amiga and UNIX file systems etc.
 *
 * @param zip: zip archive handler.
 *
 * @return the pointer to the current zip entry name, or NULL on error.
 */
extern ZIP_EXPORT const char *zip_entry_name(struct zip_t *zip);

/**
 * Returns an index of the current zip entry.
 *
 * @param zip zip archive handler.
 *
 * @return the index on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT ssize_t zip_entry_index(struct zip_t *zip);

/**
 * Determines if the current zip entry is a directory entry.
 *
 * @param zip zip archive handler.
 *
 * @return the return code - 1 (true), 0 (false), negative number (< 0) on
 *         error.
 */
extern ZIP_EXPORT int zip_entry_isdir(struct zip_t *zip);

/**
 * Returns the uncompressed size of the current zip entry.
 * Alias for zip_entry_uncomp_size (for backward compatibility).
 *
 * @param zip zip archive handler.
 *
 * @return the uncompressed size in bytes.
 */
extern ZIP_EXPORT unsigned long long zip_entry_size(struct zip_t *zip);

/**
 * Returns the uncompressed size of the current zip entry.
 *
 * @param zip zip archive handler.
 *
 * @return the uncompressed size in bytes.
 */
extern ZIP_EXPORT unsigned long long zip_entry_uncomp_size(struct zip_t *zip);

/**
 * Returns the compressed size of the current zip entry.
 *
 * @param zip zip archive handler.
 *
 * @return the compressed size in bytes.
 */
extern ZIP_EXPORT unsigned long long zip_entry_comp_size(struct zip_t *zip);

/**
 * Returns CRC-32 checksum of the current zip entry.
 *
 * @param zip zip archive handler.
 *
 * @return the CRC-32 checksum.
 */
extern ZIP_EXPORT unsigned int zip_entry_crc32(struct zip_t *zip);

/**
 * Compresses an input buffer for the current zip entry.
 *
 * @param zip zip archive handler.
 * @param buf input buffer.
 * @param bufsize input buffer size (in bytes).
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_write(struct zip_t *zip, const void *buf,
                                      size_t bufsize);

/**
 * Compresses a file for the current zip entry.
 *
 * @param zip zip archive handler.
 * @param filename input file.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_fwrite(struct zip_t *zip, const char *filename);

/**
 * Extracts the current zip entry into output buffer.
 *
 * The function allocates sufficient memory for a output buffer.
 *
 * @param zip zip archive handler.
 * @param buf output buffer.
 * @param bufsize output buffer size (in bytes).
 *
 * @note remember to release memory allocated for a output buffer.
 *       for large entries, please take a look at zip_entry_extract function.
 *
 * @return the return code - the number of bytes actually read on success.
 *         Otherwise a negative number (< 0) on error.
 */
extern ZIP_EXPORT ssize_t zip_entry_read(struct zip_t *zip, void **buf,
                                         size_t *bufsize);

/**
 * Extracts the current zip entry into a memory buffer using no memory
 * allocation.
 *
 * @param zip zip archive handler.
 * @param buf preallocated output buffer.
 * @param bufsize output buffer size (in bytes).
 *
 * @note ensure supplied output buffer is large enough.
 *       zip_entry_size function (returns uncompressed size for the current
 *       entry) can be handy to estimate how big buffer is needed.
 *       For large entries, please take a look at zip_entry_extract function.
 *
 * @return the return code - the number of bytes actually read on success.
 *         Otherwise a negative number (< 0) on error (e.g. bufsize is not large
 * enough).
 */
extern ZIP_EXPORT ssize_t zip_entry_noallocread(struct zip_t *zip, void *buf,
                                                size_t bufsize);

/**
 * Extracts the current zip entry into output file.
 *
 * @param zip zip archive handler.
 * @param filename output file.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_entry_fread(struct zip_t *zip, const char *filename);

/**
 * Extracts the current zip entry using a callback function (on_extract).
 *
 * @param zip zip archive handler.
 * @param on_extract callback function.
 * @param arg opaque pointer (optional argument, which you can pass to the
 *        on_extract callback)
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int
zip_entry_extract(struct zip_t *zip,
                  size_t (*on_extract)(void *arg, uint64_t offset,
                                       const void *data, size_t size),
                  void *arg);

/**
 * Returns the number of all entries (files and directories) in the zip archive.
 *
 * @param zip zip archive handler.
 *
 * @return the return code - the number of entries on success, negative number
 *         (< 0) on error.
 */
extern ZIP_EXPORT ssize_t zip_entries_total(struct zip_t *zip);

/**
 * Deletes zip archive entries.
 *
 * @param zip zip archive handler.
 * @param entries array of zip archive entries to be deleted.
 * @param len the number of entries to be deleted.
 * @return the number of deleted entries, or negative number (< 0) on error.
 */
extern ZIP_EXPORT ssize_t zip_entries_delete(struct zip_t *zip,
                                             char *const entries[], size_t len);

/**
 * Extracts a zip archive stream into directory.
 *
 * If on_extract is not NULL, the callback will be called after
 * successfully extracted each zip entry.
 * Returning a negative value from the callback will cause abort and return an
 * error. The last argument (void *arg) is optional, which you can use to pass
 * data to the on_extract callback.
 *
 * @param stream zip archive stream.
 * @param size stream size.
 * @param dir output directory.
 * @param on_extract on extract callback.
 * @param arg opaque pointer.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int
zip_stream_extract(const char *stream, size_t size, const char *dir,
                   int (*on_extract)(const char *filename, void *arg),
                   void *arg);

/**
 * Opens zip archive stream into memory.
 *
 * @param stream zip archive stream.
 * @param size stream size.
 *
 * @return the zip archive handler or NULL on error
 */
extern ZIP_EXPORT struct zip_t *zip_stream_open(const char *stream, size_t size,
                                                int level, char mode);

/**
 * Copy zip archive stream output buffer.
 *
 * @param zip zip archive handler.
 * @param buf output buffer. User should free buf.
 * @param bufsize output buffer size (in bytes).
 *
 * @return copy size
 */
extern ZIP_EXPORT ssize_t zip_stream_copy(struct zip_t *zip, void **buf,
                                          size_t *bufsize);

/**
 * Close zip archive releases resources.
 *
 * @param zip zip archive handler.
 *
 * @return
 */
extern ZIP_EXPORT void zip_stream_close(struct zip_t *zip);

/**
 * Creates a new archive and puts files into a single zip archive.
 *
 * @param zipname zip archive file.
 * @param filenames input files.
 * @param len: number of input files.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_create(const char *zipname, const char *filenames[],
                                 size_t len);

/**
 * Extracts a zip archive file into directory.
 *
 * If on_extract_entry is not NULL, the callback will be called after
 * successfully extracted each zip entry.
 * Returning a negative value from the callback will cause abort and return an
 * error. The last argument (void *arg) is optional, which you can use to pass
 * data to the on_extract_entry callback.
 *
 * @param zipname zip archive file.
 * @param dir output directory.
 * @param on_extract_entry on extract callback.
 * @param arg opaque pointer.
 *
 * @return the return code - 0 on success, negative number (< 0) on error.
 */
extern ZIP_EXPORT int zip_extract(const char *zipname, const char *dir,
                                  int (*on_extract_entry)(const char *filename,
                                                          void *arg),
                                  void *arg);
/** @} */
#ifdef __cplusplus
}
#endif

#endif

struct RogueStackTraceFrame
{
  RogueString* procedure;
  RogueString* filename;
  RogueInt32 line;
};

void RogueStackTraceFrame_gc_trace( void* THIS );

struct RogueFile
{
  RogueString* filepath;
};

void RogueFile_gc_trace( void* THIS );

struct RogueOptionalFile
{
  RogueFile value;
  RogueLogical exists;
};

void RogueOptionalFile_gc_trace( void* THIS );

struct RogueOptionalInt32
{
  RogueInt32 value;
  RogueLogical exists;
};

struct RogueOptionalString
{
  RogueString* value;
  RogueLogical exists;
};

void RogueOptionalString_gc_trace( void* THIS );

struct RogueListRewriterxRogueStringx
{
  RogueStringList* list;
  RogueInt32 read_index;
  RogueInt32 write_index;
};

void RogueListRewriterxRogueStringx_gc_trace( void* THIS );

struct RogueStringEncoding
{
  RogueInt32 value;
};

struct RogueSpan
{
  RogueInt32 index;
  RogueInt32 count;
};

struct RogueOptionalSpan
{
  RogueSpan value;
  RogueLogical exists;
};

struct GeometryXY
{
  RogueInt32 x;
  RogueInt32 y;
};

struct GeometryBox
{
  GeometryXY position;
  GeometryXY size;
};

struct RogueValue
{
  RogueInt32 type;
  RogueObject* object;
  union
  {
    GeometryBox    box;
    RogueByte      byte;
    RogueCharacter character;
    RogueInt32     int32;
    RogueInt64     int64;
    RogueLogical   logical;
    RogueReal      real;
    RogueReal32    real32;
    RogueReal64    real64;
    GeometryXY     xy;
  };
};

void RogueValue_gc_trace( void* THIS );

struct RogueConsoleCursor
{
  int dummy;
};

struct RogueFilePattern
{
  RogueString* pattern;
};

void RogueFilePattern_gc_trace( void* THIS );

struct RogueOptionalFilePattern
{
  RogueFilePattern value;
  RogueLogical exists;
};

void RogueOptionalFilePattern_gc_trace( void* THIS );

struct RogueFileListingOption
{
  RogueInt32 value;
};

struct RogueTableEntriesIteratorxRogueString_RogueValuex
{
  RogueTableEntryxRogueString_RogueValuex* cur;
};

void RogueTableEntriesIteratorxRogueString_RogueValuex_gc_trace( void* THIS );

struct RogueTableKeysIteratorxRogueString_RogueValuex
{
  RogueTableEntryxRogueString_RogueValuex* cur;
};

void RogueTableKeysIteratorxRogueString_RogueValuex_gc_trace( void* THIS );

struct RogueOptionalTableEntryxRogueString_RogueValuex
{
  RogueTableEntryxRogueString_RogueValuex* value;
  RogueLogical exists;
};

void RogueOptionalTableEntryxRogueString_RogueValuex_gc_trace( void* THIS );

struct RogueConsoleEventType
{
  RogueInt32 value;
};

struct RogueConsoleEvent
{
  RogueConsoleEventType type;
  RogueInt32 x;
  RogueInt32 y;
};

struct RogueWordPointer
{
  RogueWord* value;
};

struct RogueRangeUpToLessThanxRogueInt32x
{
  RogueInt32 start;
  RogueInt32 limit;
  RogueInt32 step;
};

struct RogueRangeUpToLessThanIteratorxRogueInt32x
{
  RogueInt32 cur;
  RogueInt32 limit;
  RogueInt32 step;
};

struct RogueRangeUpToxRogueInt32x
{
  RogueInt32 start;
  RogueInt32 limit;
  RogueInt32 step;
};

struct RogueRangeUpToIteratorxRogueInt32x
{
  RogueInt32 cur;
  RogueInt32 limit;
  RogueInt32 step;
};

struct RogueRangeDownToxRogueInt32x
{
  RogueInt32 start;
  RogueInt32 limit;
  RogueInt32 step;
};

struct RogueRangeDownToIteratorxRogueInt32x
{
  RogueInt32 cur;
  RogueInt32 limit;
  RogueInt32 step;
};

struct RogueTableKeysIteratorxRogueString_RogueStringx
{
  RogueTableEntryxRogueString_RogueStringx* cur;
};

void RogueTableKeysIteratorxRogueString_RogueStringx_gc_trace( void* THIS );

struct RogueTableValuesIteratorxRogueString_RogueStringx
{
  RogueTableEntryxRogueString_RogueStringx* cur;
};

void RogueTableValuesIteratorxRogueString_RogueStringx_gc_trace( void* THIS );

struct RogueVersionNumber
{
  RogueString* version;
};

void RogueVersionNumber_gc_trace( void* THIS );

struct RogueBestxRogueStringx
{
  RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* better_fn;
  RogueString* value;
  RogueLogical exists;
};

void RogueBestxRogueStringx_gc_trace( void* THIS );

struct RogueZipEntry
{
  RogueZip* zip;
  RogueString* name;
  RogueLogical is_folder;
  RogueInt64 size;
  RogueInt32 crc32;
};

void RogueZipEntry_gc_trace( void* THIS );

struct RogueUnixConsoleMouseEventType
{
  RogueInt32 value;
};

struct RogueByteList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueByte element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueByteList_gc_trace( void* THIS );

struct RogueString
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 cursor_offset;
  RogueInt32 cursor_index;
  RogueInt32 hash_code;
  RogueInt32 indent;
  RogueLogical at_newline;
  RogueLogical is_immutable;
  RogueLogical is_ascii;
  RogueByteList* data;
};

void RogueString_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENCPARENCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENCPARENCPAREN_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENCPARENCPARENList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueOPARENFunctionOPARENCPARENCPAREN* element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueOPARENFunctionOPARENCPARENCPARENList_gc_trace( void* THIS );

struct RogueGlobal
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* global_output_buffer;
  RogueObject* output;
  RogueObject* error;
  RogueOPARENFunctionOPARENCPARENCPARENList* exit_functions;
};

void RogueGlobal_gc_trace( void* THIS );

struct RogueObject
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueObject_gc_trace( void* THIS );

struct RogueStackTraceFrameList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueStackTraceFrame element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueStackTraceFrameList_gc_trace( void* THIS );

struct RogueStackTrace
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStackTraceFrameList* frames;
};

void RogueStackTrace_gc_trace( void* THIS );

struct RogueException
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
};

void RogueException_gc_trace( void* THIS );

struct RogueRoutine
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueRoutine_gc_trace( void* THIS );

struct RogueStringList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueString* element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueStringList_gc_trace( void* THIS );

struct RogueSystem
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueSystem_gc_trace( void* THIS );

struct RogueError
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
};

void RogueError_gc_trace( void* THIS );

struct RogueProcessResult
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 exit_code;
  RogueByteList* output_bytes;
  RogueByteList* error_bytes;
  RogueString* output_string;
  RogueString* error_string;
};

void RogueProcessResult_gc_trace( void* THIS );

struct RogueProcess
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* args;
  RogueObject* output_reader;
  RogueObject* error_reader;
  RogueObject* input_writer;
  RogueLogical error;
  RogueLogical is_finished;
  RogueInt32 exit_code;
  RogueLogical is_blocking;
  RogueProcessResult* result;
};

void RogueProcess_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueStringCPARENCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueStringCPARENCPAREN_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN_gc_trace( void* THIS );

struct RogueListReaderxRogueStringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueStringList* list;
  RogueInt32 limit;
  RogueLogical is_limited;
};

void RogueListReaderxRogueStringx_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN_gc_trace( void* THIS );

struct RogueCharacterList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueCharacter element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueCharacterList_gc_trace( void* THIS );

struct RogueStringReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueInt32 count;
  RogueString* string;
};

void RogueStringReader_gc_trace( void* THIS );

struct RogueBufferedPrintWriter
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* buffer;
  RogueObject* output;
};

void RogueBufferedPrintWriter_gc_trace( void* THIS );

struct RogueStringPool
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* available;
};

void RogueStringPool_gc_trace( void* THIS );

struct RogueListReaderxRogueBytex
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueByteList* list;
  RogueInt32 limit;
  RogueLogical is_limited;
};

void RogueListReaderxRogueBytex_gc_trace( void* THIS );

struct RogueConsoleMode
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueConsoleMode_gc_trace( void* THIS );

struct RogueConsole
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueString* output_buffer;
  RogueObject* error;
  RogueConsoleCursor cursor;
  RogueLogical is_end_of_input;
  RogueLogical immediate_mode;
  RogueLogical decode_utf8;
  RogueConsoleMode* mode;
  RogueLogical windows_in_quick_edit_mode;
  RogueByteList* input_buffer;
  RogueOptionalInt32 next_input_character;
  RogueByteList* _input_bytes;
  #if !defined(ROGUE_PLATFORM_WINDOWS) && !defined(ROGUE_PLATFORM_EMBEDDED)
    struct termios original_terminal_settings;
    int            original_stdin_flags;
  #endif
};

void RogueConsole_gc_trace( void* THIS );

struct RogueFileReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueString* filepath;
  RogueInt32 count;
  RogueInt32 buffer_position;
  RogueByteList* buffer;
  RogueLogical error;
  FILE* fp;
};

void RogueFileReader_gc_trace( void* THIS );

struct RogueFileWriter
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueString* filepath;
  RogueLogical error;
  RogueByteList* buffer;
  FILE* fp;
};

void RogueFileWriter_gc_trace( void* THIS );

struct RogueUTF16String
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueByteList* data;
};

void RogueUTF16String_gc_trace( void* THIS );

struct RogueWindowsProcess
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* args;
  RogueObject* output_reader;
  RogueObject* error_reader;
  RogueObject* input_writer;
  RogueLogical error;
  RogueLogical is_finished;
  RogueInt32 exit_code;
  RogueLogical is_blocking;
  RogueProcessResult* result;
  RogueLogical process_created;
  RogueUTF16String* arg_string_utf16;
  #ifdef ROGUE_PLATFORM_WINDOWS
  STARTUPINFO         startup_info;
  PROCESS_INFORMATION process_info;
  #endif
};

void RogueWindowsProcess_gc_trace( void* THIS );

struct RoguePosixProcess
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* args;
  RogueObject* output_reader;
  RogueObject* error_reader;
  RogueObject* input_writer;
  RogueLogical error;
  RogueLogical is_finished;
  RogueInt32 exit_code;
  RogueLogical is_blocking;
  RogueProcessResult* result;
  #ifndef ROGUE_PLATFORM_WINDOWS
  pid_t pid;
  int cin_pipe[2];
  int cout_pipe[2];
  int cerr_pipe[2];
  posix_spawn_file_actions_t actions;
  struct pollfd poll_list[2];
  #endif
};

void RoguePosixProcess_gc_trace( void* THIS );

struct RogueStringEncodingList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueStringEncoding element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueStringEncodingList_gc_trace( void* THIS );

struct RogueObjectPoolxRogueStringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* available;
};

void RogueObjectPoolxRogueStringx_gc_trace( void* THIS );

struct RogueValueList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueValue element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueValueList_gc_trace( void* THIS );

struct RogueTableEntryxRogueString_RogueValuex
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* key;
  RogueValue value;
  RogueTableEntryxRogueString_RogueValuex* adjacent_entry;
  RogueTableEntryxRogueString_RogueValuex* next_entry;
  RogueTableEntryxRogueString_RogueValuex* previous_entry;
  RogueInt32 hash;
};

void RogueTableEntryxRogueString_RogueValuex_gc_trace( void* THIS );

struct RogueTableEntryxRogueString_RogueValuexList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueTableEntryxRogueString_RogueValuex* element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueTableEntryxRogueString_RogueValuexList_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN_gc_trace( void* THIS );

struct RogueTableBTABLERogueString_RogueValueETABLE
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 bin_mask;
  RogueInt32 cur_entry_index;
  RogueTableEntryxRogueString_RogueValuexList* bins;
  RogueTableEntryxRogueString_RogueValuex* first_entry;
  RogueTableEntryxRogueString_RogueValuex* last_entry;
  RogueTableEntryxRogueString_RogueValuex* cur_entry;
  RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN* sort_function;
};

void RogueTableBTABLERogueString_RogueValueETABLE_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN_gc_trace( void* THIS );

struct RogueConsoleEventTypeList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueConsoleEventType element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueConsoleEventTypeList_gc_trace( void* THIS );

struct RogueFileListingOptionList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueFileListingOption element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueFileListingOptionList_gc_trace( void* THIS );

struct RogueWindowsProcessReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueByteList* buffer;
  #ifdef ROGUE_PLATFORM_WINDOWS
  HANDLE output_writer;
  HANDLE output_reader;
  #endif
};

void RogueWindowsProcessReader_gc_trace( void* THIS );

struct RogueWindowsProcessWriter
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueByteList* buffer;
  #ifdef ROGUE_PLATFORM_WINDOWS
  HANDLE input_writer;
  HANDLE input_reader;
  #endif
};

void RogueWindowsProcessWriter_gc_trace( void* THIS );

struct RogueFDReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueInt32 fd;
  RogueInt32 buffer_position;
  RogueLogical auto_close;
  RogueByteList* buffer;
};

void RogueFDReader_gc_trace( void* THIS );

struct RoguePosixProcessReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueProcess* process;
  RogueFDReader* fd_reader;
};

void RoguePosixProcessReader_gc_trace( void* THIS );

struct RogueMorlock
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* HOME;
  RogueLogical is_dependency;
};

void RogueMorlock_gc_trace( void* THIS );

struct RoguePackageInfo
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* name;
  RogueString* provider;
  RogueString* app_name;
  RogueString* repo;
  RogueString* version;
  RogueString* host;
  RogueString* folder;
  RogueString* filepath;
  RogueString* url;
  RogueString* build_folder;
  RogueStringList* installed_versions;
  RogueLogical using_local_script;
};

void RoguePackageInfo_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN_gc_trace( void* THIS );

struct RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* key;
  RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* value;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* adjacent_entry;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* next_entry;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* previous_entry;
  RogueInt32 hash;
};

void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_gc_trace( void* THIS );

struct RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN_gc_trace( void* THIS );

struct RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 bin_mask;
  RogueInt32 cur_entry_index;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* bins;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* first_entry;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* last_entry;
  RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* cur_entry;
  RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN* sort_function;
};

void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE_gc_trace( void* THIS );

struct RogueCommandLineParser
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* handlers;
  RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* unknown_handler;
  RogueValue defaults;
  RogueLogical allow_batching;
  RogueLogical ddash_disables;
  RogueString* cur_group;
  RogueString* arg;
  RogueString* name;
  RogueObject* arg_reader;
  RogueValue options;
  RogueValue command;
};

void RogueCommandLineParser_gc_trace( void* THIS );

struct RogueFunction_326
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_326_gc_trace( void* THIS );

struct RogueFunction_327
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* from_name;
  RogueString* to_name;
  RogueLogical require_value;
};

void RogueFunction_327_gc_trace( void* THIS );

struct RogueFunction_388
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueLogical require_value;
  RogueLogical multi;
};

void RogueFunction_388_gc_trace( void* THIS );

struct RogueFunction_389
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueLogical require_value;
  RogueLogical multi;
};

void RogueFunction_389_gc_trace( void* THIS );

struct RogueInt32List
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueInt32 element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueInt32List_gc_trace( void* THIS );

struct RogueTableEntryxRogueString_RogueStringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* key;
  RogueString* value;
  RogueTableEntryxRogueString_RogueStringx* adjacent_entry;
  RogueTableEntryxRogueString_RogueStringx* next_entry;
  RogueTableEntryxRogueString_RogueStringx* previous_entry;
  RogueInt32 hash;
};

void RogueTableEntryxRogueString_RogueStringx_gc_trace( void* THIS );

struct RogueTableEntryxRogueString_RogueStringxList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueTableEntryxRogueString_RogueStringx* element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueTableEntryxRogueString_RogueStringxList_gc_trace( void* THIS );

struct RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN_gc_trace( void* THIS );

struct RogueTableBTABLERogueString_RogueStringETABLE
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 bin_mask;
  RogueInt32 cur_entry_index;
  RogueTableEntryxRogueString_RogueStringxList* bins;
  RogueTableEntryxRogueString_RogueStringx* first_entry;
  RogueTableEntryxRogueString_RogueStringx* last_entry;
  RogueTableEntryxRogueString_RogueStringx* cur_entry;
  RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN* sort_function;
};

void RogueTableBTABLERogueString_RogueStringETABLE_gc_trace( void* THIS );

struct RogueSystemEnvironment
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueTableBTABLERogueString_RogueStringETABLE* definitions;
  RogueStringList* names;
};

void RogueSystemEnvironment_gc_trace( void* THIS );

struct RogueUTF16StringList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueUTF16String* element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueUTF16StringList_gc_trace( void* THIS );

struct RogueObjectPoolxRogueUTF16Stringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueUTF16StringList* available;
};

void RogueObjectPoolxRogueUTF16Stringx_gc_trace( void* THIS );

struct RogueDataReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueObject* input;
};

void RogueDataReader_gc_trace( void* THIS );

struct RogueBootstrap
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueValue cmd;
  RogueLogical printed_installing_header;
};

void RogueBootstrap_gc_trace( void* THIS );

struct RoguePackage
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* action;
  RogueStringList* args;
  RogueString* name;
  RogueString* host;
  RogueString* provider;
  RogueString* repo;
  RogueString* app_name;
  RogueString* specified_version;
  RogueString* version;
  RogueInt32 release_id;
  RogueString* url;
  RogueString* morlock_home;
  RogueString* launcher_folder;
  RogueString* package_folder;
  RogueString* install_folder;
  RogueString* bin_folder;
  RogueString* archive_filename;
  RogueString* archive_folder;
  RogueValue releases;
  RogueValue assets;
  RogueValue properties;
  RogueValue cache;
  RogueString* is_unpacked;
};

void RoguePackage_gc_trace( void* THIS );

struct RoguePlatforms
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* combined;
};

void RoguePlatforms_gc_trace( void* THIS );

struct RogueFDWriter
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueInt32 fd;
  RogueLogical auto_close;
  RogueLogical error;
  RogueByteList* buffer;
};

void RogueFDWriter_gc_trace( void* THIS );

struct RogueFunction_490
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_490_gc_trace( void* THIS );

struct RogueFunction_491
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* binpath;
};

void RogueFunction_491_gc_trace( void* THIS );

struct RogueLineReader
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueObject* source;
  RogueString* next;
  RogueCharacter prev;
};

void RogueLineReader_gc_trace( void* THIS );

struct RogueFileListing
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueFilePattern pattern;
  RogueFileListingOption options;
  RogueStringList* path_segments;
  RogueStringList* pattern_segments;
  RogueStringList* filepath_segments;
  RogueStringList* empty_segments;
  RogueStringList* filepaths;
  RogueOPARENFunctionOPARENRogueStringCPARENCPAREN* callback;
};

void RogueFileListing_gc_trace( void* THIS );

struct RogueFunction_495
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueFileListing* _THIS;
};

void RogueFunction_495_gc_trace( void* THIS );

struct RogueFunction_496
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_496_gc_trace( void* THIS );

struct RogueSpanList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueSpan element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueSpanList_gc_trace( void* THIS );

struct RogueTimsortxRogueStringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueTimsortxRogueStringx_gc_trace( void* THIS );

struct RogueWorkListxRogueStringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* frame;
  RogueStringList* stack;
  RogueInt32List* sp_stack;
};

void RogueWorkListxRogueStringx_gc_trace( void* THIS );

struct RogueWorkListxRogueString_Defaultx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueStringList* frame;
  RogueStringList* stack;
  RogueInt32List* sp_stack;
};

void RogueWorkListxRogueString_Defaultx_gc_trace( void* THIS );

struct RogueFunction_513
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_513_gc_trace( void* THIS );

struct RogueFunction_514
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_514_gc_trace( void* THIS );

struct RoguePackageError
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
  RogueString* package_name;
};

void RoguePackageError_gc_trace( void* THIS );

struct RogueJSON
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueJSON_gc_trace( void* THIS );

struct RogueJSONParseError
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
};

void RogueJSONParseError_gc_trace( void* THIS );

struct RogueScanner
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 position;
  RogueCharacterList* data;
  RogueString* source;
  RogueInt32 count;
  RogueInt32 line;
  RogueInt32 column;
  RogueInt32 spaces_per_tab;
};

void RogueScanner_gc_trace( void* THIS );

struct RogueJSONParser
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueScanner* reader;
};

void RogueJSONParser_gc_trace( void* THIS );

struct RogueFunction_529
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_529_gc_trace( void* THIS );

struct RogueFunction_530
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_530_gc_trace( void* THIS );

struct RogueFunction_531
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_531_gc_trace( void* THIS );

struct RogueZip
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* filepath;
  RogueInt32 compression;
  RogueInt32 mode;
  struct zip_t* zip;
};

void RogueZip_gc_trace( void* THIS );

struct RogueFiles
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* base_folder;
  RogueTableBTABLERogueString_RogueStringETABLE* filepath_set;
};

void RogueFiles_gc_trace( void* THIS );

struct RogueFunction_551
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_551_gc_trace( void* THIS );

struct RogueFunction_552
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_552_gc_trace( void* THIS );

struct RogueFunction_553
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_553_gc_trace( void* THIS );

struct RogueFunction_554
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* app_name;
};

void RogueFunction_554_gc_trace( void* THIS );

struct RogueFunction_555
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* app_name;
};

void RogueFunction_555_gc_trace( void* THIS );

struct RogueWorkListxRogueBytex
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueByteList* frame;
  RogueByteList* stack;
  RogueInt32List* sp_stack;
};

void RogueWorkListxRogueBytex_gc_trace( void* THIS );

struct RogueWorkListxRogueByte_Defaultx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueByteList* frame;
  RogueByteList* stack;
  RogueInt32List* sp_stack;
};

void RogueWorkListxRogueByte_Defaultx_gc_trace( void* THIS );

struct RogueFunction_558
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_558_gc_trace( void* THIS );

struct RogueFunction_559
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_559_gc_trace( void* THIS );

struct RogueFunction_560
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_560_gc_trace( void* THIS );

struct RogueFunction_574
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_574_gc_trace( void* THIS );

struct RogueFunction_575
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_575_gc_trace( void* THIS );

struct RogueSetxRogueStringx
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 bin_mask;
  RogueInt32 cur_entry_index;
  RogueTableEntryxRogueString_RogueStringxList* bins;
  RogueTableEntryxRogueString_RogueStringx* first_entry;
  RogueTableEntryxRogueString_RogueStringx* last_entry;
  RogueTableEntryxRogueString_RogueStringx* cur_entry;
  RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN* sort_function;
};

void RogueSetxRogueStringx_gc_trace( void* THIS );

struct RogueConsoleErrorPrinter
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* output_buffer;
};

void RogueConsoleErrorPrinter_gc_trace( void* THIS );

struct RogueFunction_578
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_578_gc_trace( void* THIS );

struct RogueStandardConsoleMode
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueOptionalInt32 next_input_character;
};

void RogueStandardConsoleMode_gc_trace( void* THIS );

struct RogueFunction_580
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
};

void RogueFunction_580_gc_trace( void* THIS );

struct RogueConsoleEventList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueConsoleEvent element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueConsoleEventList_gc_trace( void* THIS );

struct RogueImmediateConsoleMode
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueConsoleEventList* events;
  RogueLogical decode_utf8;
  RogueInt32 windows_button_state;
  RogueConsoleEventType windows_last_press_type;
};

void RogueImmediateConsoleMode_gc_trace( void* THIS );

struct RogueUnixConsoleMouseEventTypeList
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueInt32 count;
  RogueInt32 capacity;
  RogueInt32 element_size;
  RogueLogical is_ref_array;
  RogueLogical is_borrowed;
  RogueUnixConsoleMouseEventType element_type;
  union
  {
    void*           data;
    void*           as_void;
    RogueObject**   as_objects;
    RogueString**   as_strings;
    RogueReal64*    as_real64s;
    RogueReal32*    as_real32s;
    RogueInt64*     as_int64s;
    RogueInt32*     as_int32s;
    RogueCharacter* as_characters;
    RogueWord*      as_words;
    RogueByte*      as_bytes;
    char*           as_utf8; // only valid for String data; includes null terminator
    RogueByte*      as_logicals;
    #if defined(ROGUE_PLATFORM_WINDOWS)
      wchar_t*      as_wchars;
    #endif
  };
};

void RogueUnixConsoleMouseEventTypeList_gc_trace( void* THIS );

struct RogueUnrecognizedOptionError
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
  RogueString* name;
};

void RogueUnrecognizedOptionError_gc_trace( void* THIS );

struct RogueValueExpectedError
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
  RogueString* name;
};

void RogueValueExpectedError_gc_trace( void* THIS );

struct RogueUnexpectedValueError
{
  RogueRuntimeType* __type;
  RogueInt32 __refcount;
  RogueString* message;
  RogueStackTrace* stack_trace;
  RogueString* name;
};

void RogueUnexpectedValueError_gc_trace( void* THIS );

RogueString* RogueLogical__toxRogueStringx( RogueLogical THIS );
RogueString* RogueByte__toxRogueStringx( RogueByte THIS );
RogueLogical RogueCharacter__is_alphanumeric( RogueCharacter THIS );
RogueLogical RogueCharacter__is_identifier__RogueString( RogueCharacter THIS, RogueString* additional_characters_0 );
RogueLogical RogueCharacter__is_identifier_start__RogueString( RogueCharacter THIS, RogueString* additional_characters_0 );
RogueLogical RogueCharacter__is_letter( RogueCharacter THIS );
RogueLogical RogueCharacter__is_number__RogueInt32( RogueCharacter THIS, RogueInt32 base_0 );
RogueLogical RogueCharacter__is_uppercase( RogueCharacter THIS );
void RogueCharacter__print_escaped_ascii__RogueString_RoguePrintWriter( RogueCharacter THIS, RogueString* additional_characters_to_escape_0, RogueObject* writer_1 );
RogueString* RogueCharacter__toxRogueStringx( RogueCharacter THIS );
RogueCharacter RogueCharacter__to_lowercase( RogueCharacter THIS );
RogueInt32 RogueCharacter__to_number__RogueInt32( RogueCharacter THIS, RogueInt32 base_0 );
RogueInt32 RogueInt32__operatorSHR__RogueInt32( RogueInt32 THIS, RogueInt32 bits_0 );
RogueString* RogueInt32__toxRogueStringx( RogueInt32 THIS );
RogueCharacter RogueInt32__to_digit__RogueLogical( RogueInt32 THIS, RogueLogical base64_0 );
RogueInt32 RogueInt32__abs( RogueInt32 THIS );
RogueInt32 RogueInt32__clamped_low__RogueInt32( RogueInt32 THIS, RogueInt32 low_0 );
RogueInt32 RogueInt32__digit_count( RogueInt32 THIS );
RogueInt32 RogueInt32__or_larger__RogueInt32( RogueInt32 THIS, RogueInt32 other_0 );
RogueInt32 RogueInt32__or_smaller__RogueInt32( RogueInt32 THIS, RogueInt32 other_0 );
RogueInt32 RogueInt32__sign( RogueInt32 THIS );
RogueInt64 RogueInt64__operatorMOD__RogueInt64( RogueInt64 THIS, RogueInt64 other_0 );
RogueInt64 RogueInt64__operatorSHR__RogueInt64( RogueInt64 THIS, RogueInt64 bits_0 );
RogueObject* RogueInt64__print_power_of_2__RogueInt32_RogueInt32_RoguePrintWriter( RogueInt64 THIS, RogueInt32 base_0, RogueInt32 digits_1, RogueObject* buffer_2 );
RogueString* RogueInt64__toxRogueStringx( RogueInt64 THIS );
RogueReal32 RogueReal32__floor( RogueReal32 THIS );
RogueLogical RogueReal32__is_infinite( RogueReal32 THIS );
RogueLogical RogueReal32__is_NaN( RogueReal32 THIS );
RogueLogical RogueReal32__is_string_accurate_at__RogueInt32_RogueString( RogueReal32 THIS, RogueInt32 decimal_places_0, RogueString* buffer_1 );
RogueReal32 RogueReal32__operatorMOD__RogueReal32( RogueReal32 THIS, RogueReal32 other_0 );
RogueString* RogueReal32__toxRogueStringx( RogueReal32 THIS );
RogueReal64 RogueReal64__floor( RogueReal64 THIS );
RogueString* RogueStackTraceFrame__toxRogueStringx( RogueStackTraceFrame THIS );
RogueString* RogueStackTraceFrame__toxRogueStringx__RogueInt32_RogueInt32( RogueStackTraceFrame THIS, RogueInt32 left_w_0, RogueInt32 right_w_1 );
RogueFile RogueFile__operatorPLUS__RogueFile_RogueString(RogueFile left_0, RogueString* right_1);
RogueFile RogueFile__abs( RogueFile THIS );
RogueOptionalInt32 RogueFile__copy_to__RogueFile_RogueLogical_RogueLogical_RogueLogical_RogueLogical_RogueLogical( RogueFile THIS, RogueFile destination_0, RogueLogical if_different_1, RogueLogical if_newer_2, RogueLogical dry_run_3, RogueLogical verbose_4, RogueLogical ignore_hidden_5 );
RogueString* RogueFile__conventional_filepath( RogueFile THIS );
RogueInt32 RogueFile__crc32( RogueFile THIS );
RogueLogical RogueFile__create_folder( RogueFile THIS );
RogueLogical RogueFile__delete( RogueFile THIS );
RogueString* RogueFile__esc( RogueFile THIS );
RogueLogical RogueFile__exists( RogueFile THIS );
RogueString* RogueFile__extension( RogueFile THIS );
RogueString* RogueFile__filename( RogueFile THIS );
RogueString* RogueFile__folder( RogueFile THIS );
RogueLogical RogueFile__has_parent( RogueFile THIS );
RogueLogical RogueFile__is_folder( RogueFile THIS );
RogueLogical RogueFile__is_newer_than__RogueFile( RogueFile THIS, RogueFile other_0 );
RogueStringList* RogueFile__listing__RogueOptionalFilePattern_RogueLogical_RogueLogical_RogueLogical_RogueLogical_RogueLogical_RogueLogical_RogueLogical( RogueFile THIS, RogueOptionalFilePattern filepattern_0, RogueLogical ignore_hidden_1, RogueLogical absolute_2, RogueLogical omit_path_3, RogueLogical files_4, RogueLogical folders_5, RogueLogical unsorted_6, RogueLogical recursive_7 );
RogueByteList* RogueFile__load_as_bytes__RogueLogical( RogueFile THIS, RogueLogical discard_bom_0 );
RogueString* RogueFile__load_as_string__RogueStringEncoding( RogueFile THIS, RogueStringEncoding encoding_0 );
RogueLogical RogueFile__operator____RogueFile( RogueFile THIS, RogueFile other_0 );
RogueFile RogueFile__operatorDIVIDE__RogueString( RogueFile THIS, RogueString* other_filepath_0 );
RogueFile RogueFile__parent__RogueInt32( RogueFile THIS, RogueInt32 level_0 );
RogueObject* RogueFile__print_writer__RogueLogical( RogueFile THIS, RogueLogical append_0 );
RogueFileReader* RogueFile__reader( RogueFile THIS );
RogueFile RogueFile__resolved( RogueFile THIS );
RogueLogical RogueFile__save__RogueByteList( RogueFile THIS, RogueByteList* data_0 );
RogueLogical RogueFile__save__RogueString_RogueLogical( RogueFile THIS, RogueString* data_0, RogueLogical bom_1 );
RogueInt64 RogueFile__size( RogueFile THIS );
RogueInt64 RogueFile__timestamp_ms( RogueFile THIS );
RogueString* RogueFile__toxRogueStringx( RogueFile THIS );
RogueFile RogueFile__with_destination__RogueFile( RogueFile THIS, RogueFile destination_0 );
RogueFile RogueFile__without_trailing_separator( RogueFile THIS );
RogueFileWriter* RogueFile__writer__RogueLogical( RogueFile THIS, RogueLogical append_0 );
RogueString* RogueOptionalString__toxRogueStringx( RogueOptionalString THIS );
void RogueListRewriterxRogueStringx__finish( RogueListRewriterxRogueStringx* THIS );
RogueOptionalString RogueListRewriterxRogueStringx__read_another( RogueListRewriterxRogueStringx* THIS );
void RogueListRewriterxRogueStringx__write__RogueString( RogueListRewriterxRogueStringx* THIS, RogueString* value_0 );
extern RogueStringEncodingList* RogueStringEncoding__g_categories;
void RogueStringEncoding__init_class(void);
RogueLogical RogueStringEncoding__operator____RogueStringEncoding( RogueStringEncoding THIS, RogueStringEncoding other_0 );
RogueString* RogueStringEncoding__toxRogueStringx( RogueStringEncoding THIS );
RogueString* RogueSpan__toxRogueStringx( RogueSpan THIS );
RogueString* GeometryXY__toxRogueStringx( GeometryXY THIS );
RogueString* GeometryBox__toxRogueStringx( GeometryBox THIS );
RogueValue RogueValue__create(void);
RogueValue RogueValue__create__RogueLogical(RogueLogical value_0);
RogueValue RogueValue__create__RogueCharacter(RogueCharacter value_0);
RogueValue RogueValue__create__RogueInt32(RogueInt32 value_0);
RogueValue RogueValue__create__RogueReal64(RogueReal64 value_0);
RogueValue RogueValue__create__RogueString(RogueString* value_0);
RogueValue RogueValue__create_list(void);
RogueValue RogueValue__create_table(void);
void RogueValue__add__RogueValue( RogueValue THIS, RogueValue value_0 );
RogueLogical RogueValue__contains__RogueValue( RogueValue THIS, RogueValue value_0 );
RogueInt32 RogueValue__count( RogueValue THIS );
RogueValue RogueValue__first( RogueValue THIS );
RogueValue RogueValue__first__RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN( RogueValue THIS, RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN* query_0 );
RogueValue RogueValue__get__RogueInt32( RogueValue THIS, RogueInt32 index_0 );
RogueValue RogueValue__get__RogueString( RogueValue THIS, RogueString* key_0 );
RogueLogical RogueValue__is_empty( RogueValue THIS );
RogueLogical RogueValue__is_list( RogueValue THIS );
RogueLogical RogueValue__is_number( RogueValue THIS );
RogueTableKeysIteratorxRogueString_RogueValuex RogueValue__keys( RogueValue THIS );
RogueLogical RogueValue__Optionaloperator( RogueValue THIS );
RogueValue RogueValue__remove_first( RogueValue THIS );
void RogueValue__set__RogueInt32_RogueValue( RogueValue THIS, RogueInt32 index_0, RogueValue value_1 );
void RogueValue__set__RogueString_RogueValue( RogueValue THIS, RogueString* key_0, RogueValue value_1 );
void RogueValue__set__RogueValue_RogueValue( RogueValue THIS, RogueValue key_0, RogueValue value_1 );
RogueInt32 RogueValue__toxRogueInt32x( RogueValue THIS );
RogueLogical RogueValue__toxRogueLogicalx( RogueValue THIS );
RogueString* RogueValue__toxRogueStringx( RogueValue THIS );
RogueString* RogueValue__to_json__RogueLogical_RogueLogical( RogueValue THIS, RogueLogical formatted_0, RogueLogical omit_commas_1 );
void RogueValue__write_json__RogueString_RogueLogical_RogueLogical( RogueValue THIS, RogueString* builder_0, RogueLogical formatted_1, RogueLogical omit_commas_2 );
void RogueValue__write_json_string__RogueString_RogueString( RogueValue THIS, RogueString* st_0, RogueString* builder_1 );
void RogueValue__write_json_character__RogueCharacter_RogueString( RogueValue THIS, RogueCharacter ch_0, RogueString* builder_1 );
RogueLogical RogueValue__operator____RogueValue( RogueValue THIS, RogueValue other_0 );
RogueStringList* RogueValue__to_listxRogueStringx( RogueValue THIS );
extern RogueLogical RogueConsoleCursor__g_cursor_hidden;
void RogueConsoleCursor__init_class(void);
void RogueConsoleCursor__hide__RogueLogical_RoguePrintWriter( RogueConsoleCursor THIS, RogueLogical setting_0, RogueObject* output_1 );
void RogueConsoleCursor__show__RoguePrintWriter( RogueConsoleCursor THIS, RogueObject* output_0 );
RogueLogical RogueFilePattern__matches__RogueString_RogueLogical( RogueFilePattern THIS, RogueString* filepath_0, RogueLogical ignore_case_1 );
RogueLogical RogueFilePattern___matches__RogueString_RogueInt32_RogueInt32_RogueString_RogueInt32_RogueInt32_RogueLogical( RogueFilePattern THIS, RogueString* filepath_0, RogueInt32 f0_1, RogueInt32 fcount_2, RogueString* pattern_3, RogueInt32 p0_4, RogueInt32 pcount_5, RogueLogical ignore_case_6 );
RogueString* RogueFilePattern__toxRogueStringx( RogueFilePattern THIS );
RogueString* RogueOptionalFilePattern__toxRogueStringx( RogueOptionalFilePattern THIS );
extern RogueFileListingOptionList* RogueFileListingOption__g_categories;
void RogueFileListingOption__init_class(void);
RogueString* RogueFileListingOption__toxRogueStringx( RogueFileListingOption THIS );
RogueLogical RogueFileListingOption__is_absolute( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_absolute__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueLogical RogueFileListingOption__is_files( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_files__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueLogical RogueFileListingOption__is_folders( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_folders__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueLogical RogueFileListingOption__is_ignore_hidden( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_ignore_hidden__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueLogical RogueFileListingOption__is_omit_path( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_omit_path__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueLogical RogueFileListingOption__is_recursive( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_recursive__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueLogical RogueFileListingOption__is_unsorted( RogueFileListingOption THIS );
void RogueFileListingOption__set_is_unsorted__RogueLogical( RogueFileListingOption* THIS, RogueLogical setting_0 );
RogueOptionalTableEntryxRogueString_RogueValuex RogueTableEntriesIteratorxRogueString_RogueValuex__read_another( RogueTableEntriesIteratorxRogueString_RogueValuex* THIS );
RogueOptionalString RogueTableKeysIteratorxRogueString_RogueValuex__read_another( RogueTableKeysIteratorxRogueString_RogueValuex* THIS );
extern RogueConsoleEventTypeList* RogueConsoleEventType__g_categories;
void RogueConsoleEventType__init_class(void);
RogueLogical RogueConsoleEventType__operator____RogueConsoleEventType( RogueConsoleEventType THIS, RogueConsoleEventType other_0 );
RogueString* RogueConsoleEventType__toxRogueStringx( RogueConsoleEventType THIS );
RogueLogical RogueConsoleEvent__is_character( RogueConsoleEvent THIS );
RogueString* RogueConsoleEvent__toxRogueStringx( RogueConsoleEvent THIS );
RogueRangeUpToLessThanIteratorxRogueInt32x RogueRangeUpToLessThanxRogueInt32x__iterator( RogueRangeUpToLessThanxRogueInt32x THIS );
RogueOptionalInt32 RogueRangeUpToLessThanIteratorxRogueInt32x__read_another( RogueRangeUpToLessThanIteratorxRogueInt32x* THIS );
RogueRangeUpToIteratorxRogueInt32x RogueRangeUpToxRogueInt32x__iterator( RogueRangeUpToxRogueInt32x THIS );
RogueOptionalInt32 RogueRangeUpToIteratorxRogueInt32x__read_another( RogueRangeUpToIteratorxRogueInt32x* THIS );
RogueRangeDownToIteratorxRogueInt32x RogueRangeDownToxRogueInt32x__iterator( RogueRangeDownToxRogueInt32x THIS );
RogueOptionalInt32 RogueRangeDownToIteratorxRogueInt32x__read_another( RogueRangeDownToIteratorxRogueInt32x* THIS );
RogueOptionalString RogueTableKeysIteratorxRogueString_RogueStringx__read_another( RogueTableKeysIteratorxRogueString_RogueStringx* THIS );
RogueStringList* RogueTableKeysIteratorxRogueString_RogueStringx__to_list__RogueStringList( RogueTableKeysIteratorxRogueString_RogueStringx THIS, RogueStringList* result_0 );
RogueOptionalString RogueTableValuesIteratorxRogueString_RogueStringx__read_another( RogueTableValuesIteratorxRogueString_RogueStringx* THIS );
RogueString* RogueTableValuesIteratorxRogueString_RogueStringx__toxRogueStringx( RogueTableValuesIteratorxRogueString_RogueStringx THIS );
RogueStringList* RogueTableValuesIteratorxRogueString_RogueStringx__to_list__RogueStringList( RogueTableValuesIteratorxRogueString_RogueStringx THIS, RogueStringList* result_0 );
RogueInt32 RogueVersionNumber__count( RogueVersionNumber THIS );
RogueLogical RogueVersionNumber__is_compatible_with__RogueString( RogueVersionNumber THIS, RogueString* other_0 );
RogueLogical RogueVersionNumber__is_compatible_with__RogueVersionNumber( RogueVersionNumber THIS, RogueVersionNumber other_0 );
RogueInt32 RogueVersionNumber__operatorLTGT__RogueString( RogueVersionNumber THIS, RogueString* other_0 );
RogueInt32 RogueVersionNumber__operatorLTGT__RogueVersionNumber( RogueVersionNumber THIS, RogueVersionNumber other_0 );
RogueInt32 RogueVersionNumber__part__RogueInt32( RogueVersionNumber THIS, RogueInt32 n_0 );
RogueString* RogueVersionNumber__toxRogueStringx( RogueVersionNumber THIS );
RogueLogical RogueBestxRogueStringx__consider__RogueString( RogueBestxRogueStringx* THIS, RogueString* candidate_value_0 );
extern RogueByteList* RogueZipEntry__g_buffer;
void RogueZipEntry__init_class(void);
RogueByteList* RogueZipEntry__extract__RogueByteList( RogueZipEntry THIS, RogueByteList* buffer_0 );
void RogueZipEntry__extract__RogueFile( RogueZipEntry THIS, RogueFile file_0 );
extern RogueUnixConsoleMouseEventTypeList* RogueUnixConsoleMouseEventType__g_categories;
void RogueUnixConsoleMouseEventType__init_class(void);
RogueString* RogueUnixConsoleMouseEventType__toxRogueStringx( RogueUnixConsoleMouseEventType THIS );
void RogueByteList__init( RogueByteList* THIS );
void RogueByteList__init__RogueInt32( RogueByteList* THIS, RogueInt32 capacity_0 );
void RogueByteList__init_object( RogueByteList* THIS );
RogueByteList* RogueByteList__cloned( RogueByteList* THIS );
void RogueByteList__on_cleanup( RogueByteList* THIS );
void RogueByteList__add__RogueByte( RogueByteList* THIS, RogueByte value_0 );
void RogueByteList__add__RogueByteList( RogueByteList* THIS, RogueByteList* other_0 );
void RogueByteList__clear( RogueByteList* THIS );
void RogueByteList__copy__RogueInt32_RogueInt32_RogueByteList_RogueInt32( RogueByteList* THIS, RogueInt32 src_i1_0, RogueInt32 src_count_1, RogueByteList* dest_2, RogueInt32 dest_i1_3 );
RogueString* RogueByteList__description( RogueByteList* THIS );
void RogueByteList__discard__RogueInt32_RogueInt32( RogueByteList* THIS, RogueInt32 i1_0, RogueInt32 n_1 );
void RogueByteList__discard_from__RogueInt32( RogueByteList* THIS, RogueInt32 index_0 );
void RogueByteList__ensure_capacity__RogueInt32( RogueByteList* THIS, RogueInt32 desired_capacity_0 );
void RogueByteList__expand__RogueInt32( RogueByteList* THIS, RogueInt32 additional_count_0 );
void RogueByteList__expand_to_count__RogueInt32( RogueByteList* THIS, RogueInt32 minimum_count_0 );
RogueByte RogueByteList__first( RogueByteList* THIS );
RogueByte RogueByteList__get__RogueInt32( RogueByteList* THIS, RogueInt32 index_0 );
void RogueByteList__on_return_to_pool( RogueByteList* THIS );
RogueByte RogueByteList__remove_first( RogueByteList* THIS );
void RogueByteList__reserve__RogueInt32( RogueByteList* THIS, RogueInt32 additional_capacity_0 );
void RogueByteList__set__RogueInt32_RogueByte( RogueByteList* THIS, RogueInt32 index_0, RogueByte value_1 );
void RogueByteList__shift__RogueInt32( RogueByteList* THIS, RogueInt32 delta_0 );
RogueString* RogueByteList__toxRogueStringx( RogueByteList* THIS );
void RogueByteList__zero__RogueInt32_RogueOptionalInt32( RogueByteList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueByteList__type_name( RogueByteList* THIS );
RogueString* RogueString__create__RogueByteList_RogueStringEncoding(RogueByteList* data_0, RogueStringEncoding encoding_1);
RogueString* RogueString__create__RogueCharacterList(RogueCharacterList* data_0);
RogueLogical RogueString__exists__RogueString(RogueString* string_0);
RogueLogical RogueString__operator____RogueString_RogueString(RogueString* a_0, RogueString* b_1);
RogueInt32 RogueString__operatorLTGT__RogueString_RogueString(RogueString* a_0, RogueString* b_1);
RogueString* RogueString__operatorPLUS__RogueString_RogueCharacter(RogueString* left_0, RogueCharacter right_1);
RogueString* RogueString__operatorPLUS__RogueString_RogueString(RogueString* left_0, RogueString* right_1);
RogueString* RogueString__operatorTIMES__RogueString_RogueInt32(RogueString* left_0, RogueInt32 right_1);
RogueString* RogueString__operatorDIVIDE__RogueString_RogueString(RogueString* prefix_0, RogueString* suffix_1);
RogueString* RogueString__create__RogueFile(RogueFile file_0);
void RogueString__init( RogueString* THIS );
void RogueString__init__RogueString( RogueString* THIS, RogueString* existing_0 );
void RogueString__init__RogueInt32( RogueString* THIS, RogueInt32 byte_capacity_0 );
void RogueString__init__RogueByteList_RogueInt32( RogueString* THIS, RogueByteList* _data_0, RogueInt32 _auto_store_count_1 );
RogueString* RogueString__cloned( RogueString* THIS );
RogueString* RogueString__after_any__RogueString_RogueLogical( RogueString* THIS, RogueString* st_0, RogueLogical ignore_case_1 );
RogueString* RogueString__after_first__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueString* RogueString__after_first__RogueString_RogueLogical( RogueString* THIS, RogueString* st_0, RogueLogical ignore_case_1 );
RogueString* RogueString__after_last__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueString* RogueString__after_last__RogueString_RogueLogical( RogueString* THIS, RogueString* st_0, RogueLogical ignore_case_1 );
RogueString* RogueString__before_first__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueString* RogueString__before_first__RogueString_RogueLogical( RogueString* THIS, RogueString* st_0, RogueLogical ignore_case_1 );
RogueString* RogueString__before_last__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueString* RogueString__before_last__RogueString_RogueLogical( RogueString* THIS, RogueString* st_0, RogueLogical ignore_case_1 );
RogueString* RogueString__before_suffix__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__begins_with__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__begins_with__RogueString_RogueLogical( RogueString* THIS, RogueString* other_0, RogueLogical ignore_case_1 );
RogueInt32 RogueString__byte_count( RogueString* THIS );
RogueString* RogueString__capitalized( RogueString* THIS );
void RogueString__clear( RogueString* THIS );
RogueInt32 RogueString__compare_to__RogueString_RogueLogical( RogueString* THIS, RogueString* other_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__contains__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__contains__RogueString_RogueLogical( RogueString* THIS, RogueString* substring_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__contains_at__RogueString_RogueInt32_RogueLogical( RogueString* THIS, RogueString* substring_0, RogueInt32 at_index_1, RogueLogical ignore_case_2 );
RogueInt32 RogueString__count( RogueString* THIS );
RogueInt32 RogueString__count__RogueCharacter( RogueString* THIS, RogueCharacter look_for_0 );
RogueLogical RogueString__ends_with__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__ends_with__RogueString_RogueLogical( RogueString* THIS, RogueString* other_0, RogueLogical ignore_case_1 );
RogueLogical RogueString__equals__RogueString_RogueLogical( RogueString* THIS, RogueString* other_0, RogueLogical ignore_case_1 );
RogueString* RogueString__extract_string__RogueString_RogueLogical( RogueString* THIS, RogueString* format_0, RogueLogical ignore_case_1 );
RogueStringList* RogueString__extract_strings__RogueString_RogueLogical( RogueString* THIS, RogueString* format_0, RogueLogical ignore_case_1 );
RogueStringList* RogueString__extract_strings__RogueString_RogueStringList_RogueLogical( RogueString* THIS, RogueString* format_0, RogueStringList* results_1, RogueLogical ignore_case_2 );
void RogueString__flush( RogueString* THIS );
RogueString* RogueString__from__RogueInt32( RogueString* THIS, RogueInt32 i1_0 );
RogueString* RogueString__from__RogueInt32_RogueInt32( RogueString* THIS, RogueInt32 i1_0, RogueInt32 i2_1 );
RogueCharacter RogueString__get__RogueInt32( RogueString* THIS, RogueInt32 index_0 );
RogueInt32 RogueString__hash_code( RogueString* THIS );
RogueInt32 RogueString__indent( RogueString* THIS );
RogueString* RogueString__indented__RogueInt32( RogueString* THIS, RogueInt32 spaces_0 );
void RogueString__insert__RogueCharacter( RogueString* THIS, RogueCharacter ch_0 );
RogueString* RogueString__justified__RogueInt32_RogueCharacter( RogueString* THIS, RogueInt32 spaces_0, RogueCharacter fill_1 );
RogueCharacter RogueString__last( RogueString* THIS );
RogueString* RogueString__leftmost__RogueInt32( RogueString* THIS, RogueInt32 n_0 );
RogueOptionalInt32 RogueString__locate__RogueCharacter_RogueOptionalInt32_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueOptionalInt32 optional_i1_1, RogueLogical ignore_case_2 );
RogueOptionalInt32 RogueString__locate__RogueString_RogueOptionalInt32_RogueLogical( RogueString* THIS, RogueString* other_0, RogueOptionalInt32 optional_i1_1, RogueLogical ignore_case_2 );
RogueOptionalInt32 RogueString__locate_last__RogueCharacter_RogueOptionalInt32_RogueLogical( RogueString* THIS, RogueCharacter ch_0, RogueOptionalInt32 starting_index_1, RogueLogical ignore_case_2 );
RogueOptionalInt32 RogueString__locate_last__RogueString_RogueOptionalInt32_RogueLogical( RogueString* THIS, RogueString* other_0, RogueOptionalInt32 starting_index_1, RogueLogical ignore_case_2 );
RogueOptionalSpan RogueString__locate_pattern__RogueString_RogueInt32_RogueLogical( RogueString* THIS, RogueString* pattern_0, RogueInt32 i1_1, RogueLogical ignore_case_2 );
void RogueString__on_return_to_pool( RogueString* THIS );
void RogueString__print__RogueByte( RogueString* THIS, RogueByte value_0 );
void RogueString__print__RogueCharacter( RogueString* THIS, RogueCharacter value_0 );
void RogueString__print__RogueInt32( RogueString* THIS, RogueInt32 value_0 );
void RogueString__print__RogueInt64( RogueString* THIS, RogueInt64 value_0 );
void RogueString__print__RogueLogical( RogueString* THIS, RogueLogical value_0 );
void RogueString__print__RogueObject( RogueString* THIS, RogueObject* value_0 );
void RogueString__print__RogueReal32( RogueString* THIS, RogueReal32 value_0 );
void RogueString__print__RogueReal32_RogueInt32( RogueString* THIS, RogueReal32 value_0, RogueInt32 decimal_places_1 );
void RogueString__print__RogueString( RogueString* THIS, RogueString* value_0 );
void RogueString__println( RogueString* THIS );
void RogueString__println__RogueObject( RogueString* THIS, RogueObject* value_0 );
void RogueString__println__RogueString( RogueString* THIS, RogueString* value_0 );
RogueStringReader* RogueString__reader( RogueString* THIS );
RogueCharacter RogueString__remove_last( RogueString* THIS );
void RogueString__reserve__RogueInt32( RogueString* THIS, RogueInt32 additional_bytes_0 );
RogueString* RogueString__replacing__RogueCharacter_RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter look_for_0, RogueCharacter replace_with_1, RogueLogical ignore_case_2 );
RogueString* RogueString__replacing__RogueString_RogueString_RogueLogical( RogueString* THIS, RogueString* look_for_0, RogueString* replace_with_1, RogueLogical ignore_case_2 );
RogueString* RogueString__replacing_at__RogueInt32_RogueInt32_RogueString( RogueString* THIS, RogueInt32 index_0, RogueInt32 n_1, RogueString* replace_with_2 );
void RogueString__reverse( RogueString* THIS );
RogueString* RogueString__rightmost__RogueInt32( RogueString* THIS, RogueInt32 n_0 );
RogueInt32 RogueString__set_cursor__RogueInt32( RogueString* THIS, RogueInt32 character_index_0 );
void RogueString__set_indent__RogueInt32( RogueString* THIS, RogueInt32 _auto_store_indent_0 );
RogueStringList* RogueString__split__RogueCharacter_RogueLogical( RogueString* THIS, RogueCharacter separator_0, RogueLogical ignore_case_1 );
RogueStringList* RogueString__split__RogueString_RogueLogical( RogueString* THIS, RogueString* separator_0, RogueLogical ignore_case_1 );
RogueString* RogueString__substring__RogueInt32( RogueString* THIS, RogueInt32 i1_0 );
RogueString* RogueString__substring__RogueInt32_RogueInt32( RogueString* THIS, RogueInt32 i1_0, RogueInt32 n_1 );
RogueInt32 RogueString__toxRogueInt32x__RogueInt32( RogueString* THIS, RogueInt32 base_0 );
RogueReal32 RogueString__toxRogueReal32x( RogueString* THIS );
RogueString* RogueString__toxRogueStringx( RogueString* THIS );
RogueString* RogueString__to_lowercase( RogueString* THIS );
RogueString* RogueString__to_escaped_ascii__RogueString( RogueString* THIS, RogueString* additional_characters_to_escape_0 );
RogueString* RogueString__trimmed( RogueString* THIS );
RogueString* RogueString__with_suffix__RogueString( RogueString* THIS, RogueString* text_0 );
RogueString* RogueString__without_suffix__RogueCharacter( RogueString* THIS, RogueCharacter ch_0 );
RogueLogical RogueString___extract_strings__RogueInt32_RogueInt32_RogueString_RogueInt32_RogueInt32_RogueStringList_RogueLogical( RogueString* THIS, RogueInt32 i0_0, RogueInt32 remaining_count_1, RogueString* format_2, RogueInt32 f0_3, RogueInt32 fcount_4, RogueStringList* results_5, RogueLogical ignore_case_6 );
void RogueString___round_off( RogueString* THIS );
RogueInt32 RogueString___pattern_match_count__RogueInt32_RogueInt32_RogueString_RogueInt32_RogueInt32_RogueLogical( RogueString* THIS, RogueInt32 i0_0, RogueInt32 remaining_count_1, RogueString* format_2, RogueInt32 f0_3, RogueInt32 fcount_4, RogueLogical ignore_case_5 );
void RogueString__init_object( RogueString* THIS );
RogueString* RogueString__type_name( RogueString* THIS );
void RogueString__close( RogueString* THIS );
RogueObject* RoguePrintWriter__create__RogueWriterxRogueBytex(RogueObject* writer_0);
void RogueOPARENFunctionOPARENCPARENCPAREN__init_object( RogueOPARENFunctionOPARENCPARENCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENCPARENCPAREN__type_name( RogueOPARENFunctionOPARENCPARENCPAREN* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__init( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__init_object( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__on_cleanup( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__add__RogueOPARENFunctionOPARENCPARENCPAREN( RogueOPARENFunctionOPARENCPARENCPARENList* THIS, RogueOPARENFunctionOPARENCPARENCPAREN* value_0 );
void RogueOPARENFunctionOPARENCPARENCPARENList__clear( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
RogueString* RogueOPARENFunctionOPARENCPARENCPARENList__description( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__discard_from__RogueInt32( RogueOPARENFunctionOPARENCPARENCPARENList* THIS, RogueInt32 index_0 );
RogueOPARENFunctionOPARENCPARENCPAREN* RogueOPARENFunctionOPARENCPARENCPARENList__get__RogueInt32( RogueOPARENFunctionOPARENCPARENCPARENList* THIS, RogueInt32 index_0 );
void RogueOPARENFunctionOPARENCPARENCPARENList__on_return_to_pool( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__reserve__RogueInt32( RogueOPARENFunctionOPARENCPARENCPARENList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueOPARENFunctionOPARENCPARENCPARENList__toxRogueStringx( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueOPARENFunctionOPARENCPARENCPARENList__zero__RogueInt32_RogueOptionalInt32( RogueOPARENFunctionOPARENCPARENCPARENList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueOPARENFunctionOPARENCPARENCPARENList__type_name( RogueOPARENFunctionOPARENCPARENCPARENList* THIS );
void RogueGlobal__call_exit_functions(void);
void RogueGlobal__on_control_c__RogueInt32(RogueInt32 signum_0);
void RogueGlobal__on_segmentation_fault__RogueInt32(RogueInt32 signum_0);
extern RogueGlobal* RogueGlobal_singleton;

void RogueGlobal__init( RogueGlobal* THIS );
void RogueGlobal__configure_standard_output( RogueGlobal* THIS );
void RogueGlobal__flush__RogueString( RogueGlobal* THIS, RogueString* buffer_0 );
void RogueGlobal__on_exit__RogueOPARENFunctionOPARENCPARENCPAREN( RogueGlobal* THIS, RogueOPARENFunctionOPARENCPARENCPAREN* fn_0 );
void RogueGlobal__init_object( RogueGlobal* THIS );
RogueString* RogueGlobal__type_name( RogueGlobal* THIS );
void RogueGlobal__close( RogueGlobal* THIS );
void RogueGlobal__flush( RogueGlobal* THIS );
void RogueGlobal__print__RogueCharacter( RogueGlobal* THIS, RogueCharacter value_0 );
void RogueGlobal__print__RogueObject( RogueGlobal* THIS, RogueObject* value_0 );
void RogueGlobal__print__RogueString( RogueGlobal* THIS, RogueString* value_0 );
void RogueGlobal__println( RogueGlobal* THIS );
void RogueGlobal__println__RogueObject( RogueGlobal* THIS, RogueObject* value_0 );
void RogueGlobal__println__RogueString( RogueGlobal* THIS, RogueString* value_0 );
RogueString* RogueObject____type_name__RogueInt32(RogueInt32 type_index_0);
RogueString* RogueObject__toxRogueStringx( RogueObject* THIS );
void RogueObject__init_object( RogueObject* THIS );
void RogueObject__write_json__RogueString_RogueLogical_RogueLogical( RogueObject* THIS, RogueString* builder_0, RogueLogical formatted_1, RogueLogical omit_commas_2 );
RogueString* RogueObject__type_name( RogueObject* THIS );
void RogueStackTraceFrameList__init__RogueInt32( RogueStackTraceFrameList* THIS, RogueInt32 capacity_0 );
void RogueStackTraceFrameList__init_object( RogueStackTraceFrameList* THIS );
void RogueStackTraceFrameList__on_cleanup( RogueStackTraceFrameList* THIS );
void RogueStackTraceFrameList__add__RogueStackTraceFrame( RogueStackTraceFrameList* THIS, RogueStackTraceFrame value_0 );
void RogueStackTraceFrameList__clear( RogueStackTraceFrameList* THIS );
RogueString* RogueStackTraceFrameList__description( RogueStackTraceFrameList* THIS );
void RogueStackTraceFrameList__discard_from__RogueInt32( RogueStackTraceFrameList* THIS, RogueInt32 index_0 );
RogueStackTraceFrame RogueStackTraceFrameList__get__RogueInt32( RogueStackTraceFrameList* THIS, RogueInt32 index_0 );
RogueStackTraceFrame RogueStackTraceFrameList__last( RogueStackTraceFrameList* THIS );
void RogueStackTraceFrameList__on_return_to_pool( RogueStackTraceFrameList* THIS );
RogueStackTraceFrame RogueStackTraceFrameList__remove_last( RogueStackTraceFrameList* THIS );
void RogueStackTraceFrameList__reserve__RogueInt32( RogueStackTraceFrameList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueStackTraceFrameList__toxRogueStringx( RogueStackTraceFrameList* THIS );
void RogueStackTraceFrameList__zero__RogueInt32_RogueOptionalInt32( RogueStackTraceFrameList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueStackTraceFrameList__type_name( RogueStackTraceFrameList* THIS );
void RogueStackTrace__init( RogueStackTrace* THIS );
RogueString* RogueStackTrace__filename__RogueInt32( RogueStackTrace* THIS, RogueInt32 index_0 );
RogueInt32 RogueStackTrace__line__RogueInt32( RogueStackTrace* THIS, RogueInt32 index_0 );
RogueString* RogueStackTrace__procedure__RogueInt32( RogueStackTrace* THIS, RogueInt32 index_0 );
RogueString* RogueStackTrace__toxRogueStringx( RogueStackTrace* THIS );
void RogueStackTrace__init_object( RogueStackTrace* THIS );
RogueString* RogueStackTrace__type_name( RogueStackTrace* THIS );
void RogueException__display__RogueException(RogueException* err_0);
void RogueException__init_object( RogueException* THIS );
void RogueException__init__RogueString( RogueException* THIS, RogueString* _auto_store_message_0 );
void RogueException__display( RogueException* THIS );
RogueString* RogueException__toxRogueStringx( RogueException* THIS );
RogueString* RogueException__type_name( RogueException* THIS );
RogueString* RogueRoutine__cd_cmd__RogueString(RogueString* folder_0);
RogueLogical RogueRoutine__execute__RogueString_RogueLogical_RogueLogical_RogueLogical_RogueLogical(RogueString* cmd_0, RogueLogical suppress_error_1, RogueLogical allow_sudo_2, RogueLogical quiet_3, RogueLogical exit_on_error_4);
void RogueRoutine__on_launch(void);
RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* RogueRoutine__DimxRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxx__RogueInt32(RogueInt32 n_0);
RogueTableEntryxRogueString_RogueValuexList* RogueRoutine__DimxRogueTableEntryxRogueString_RogueValuexx__RogueInt32(RogueInt32 n_0);
RogueTableEntryxRogueString_RogueStringxList* RogueRoutine__DimxRogueTableEntryxRogueString_RogueStringxx__RogueInt32(RogueInt32 n_0);
void RogueRoutine__init_object( RogueRoutine* THIS );
RogueString* RogueRoutine__type_name( RogueRoutine* THIS );
void RogueStringList__init( RogueStringList* THIS );
void RogueStringList__init__RogueInt32( RogueStringList* THIS, RogueInt32 capacity_0 );
void RogueStringList__init_object( RogueStringList* THIS );
RogueStringList* RogueStringList__cloned( RogueStringList* THIS );
void RogueStringList__on_cleanup( RogueStringList* THIS );
void RogueStringList__add__RogueString( RogueStringList* THIS, RogueString* value_0 );
void RogueStringList__add__RogueStringList( RogueStringList* THIS, RogueStringList* other_0 );
void RogueStringList__clear( RogueStringList* THIS );
RogueLogical RogueStringList__contains__RogueString( RogueStringList* THIS, RogueString* value_0 );
void RogueStringList__copy__RogueInt32_RogueInt32_RogueStringList_RogueInt32( RogueStringList* THIS, RogueInt32 src_i1_0, RogueInt32 src_count_1, RogueStringList* dest_2, RogueInt32 dest_i1_3 );
RogueString* RogueStringList__description( RogueStringList* THIS );
void RogueStringList__discard_from__RogueInt32( RogueStringList* THIS, RogueInt32 index_0 );
void RogueStringList__ensure_capacity__RogueInt32( RogueStringList* THIS, RogueInt32 desired_capacity_0 );
void RogueStringList__expand_to_count__RogueInt32( RogueStringList* THIS, RogueInt32 minimum_count_0 );
RogueOptionalString RogueStringList__find__RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN( RogueStringList* THIS, RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN* query_0 );
RogueString* RogueStringList__first( RogueStringList* THIS );
RogueString* RogueStringList__get__RogueInt32( RogueStringList* THIS, RogueInt32 index_0 );
void RogueStringList__insert__RogueString_RogueInt32( RogueStringList* THIS, RogueString* value_0, RogueInt32 before_index_1 );
RogueLogical RogueStringList__is_empty( RogueStringList* THIS );
RogueString* RogueStringList__join__RogueString_RogueString( RogueStringList* THIS, RogueString* separator_0, RogueString* output_1 );
void RogueStringList__keep__RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN( RogueStringList* THIS, RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN* keep_if_0 );
RogueString* RogueStringList__last( RogueStringList* THIS );
RogueOptionalInt32 RogueStringList__locate__RogueString_RogueOptionalInt32( RogueStringList* THIS, RogueString* value_0, RogueOptionalInt32 i1_1 );
RogueOptionalInt32 RogueStringList__locate__RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN_RogueOptionalInt32( RogueStringList* THIS, RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN* query_0, RogueOptionalInt32 i1_1 );
void RogueStringList__on_return_to_pool( RogueStringList* THIS );
RogueListReaderxRogueStringx* RogueStringList__reader( RogueStringList* THIS );
RogueString* RogueStringList__remove_at__RogueInt32( RogueStringList* THIS, RogueInt32 index_0 );
RogueString* RogueStringList__remove_first( RogueStringList* THIS );
RogueString* RogueStringList__remove_last( RogueStringList* THIS );
void RogueStringList__reserve__RogueInt32( RogueStringList* THIS, RogueInt32 additional_capacity_0 );
RogueListRewriterxRogueStringx RogueStringList__rewriter( RogueStringList* THIS );
void RogueStringList__set__RogueInt32_RogueString( RogueStringList* THIS, RogueInt32 index_0, RogueString* value_1 );
void RogueStringList__shift__RogueInt32( RogueStringList* THIS, RogueInt32 delta_0 );
void RogueStringList__sort__RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN( RogueStringList* THIS, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_0 );
void RogueStringList__swap__RogueInt32_RogueInt32( RogueStringList* THIS, RogueInt32 i1_0, RogueInt32 i2_1 );
RogueString* RogueStringList__toxRogueStringx( RogueStringList* THIS );
void RogueStringList__zero__RogueInt32_RogueOptionalInt32( RogueStringList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueStringList__type_name( RogueStringList* THIS );
extern RogueStringList* RogueSystem__g_command_line_arguments;
extern RogueString* RogueSystem__g_executable_filepath;
extern RogueInt64 RogueSystem__g_execution_start_ms;
void RogueSystem__exit__RogueInt32(RogueInt32 result_code_0);
RogueOptionalFile RogueSystem__find_executable__RogueString(RogueString* name_0);
RogueLogical RogueSystem__is_linux(void);
RogueLogical RogueSystem__is_macos(void);
RogueLogical RogueSystem__is_windows(void);
RogueString* RogueSystem__os(void);
RogueInt32 RogueSystem__run__RogueString(RogueString* command_0);
void RogueSystem__sync_storage(void);
RogueInt64 RogueSystem__time_ms(void);
void RogueSystem___add_command_line_argument__RogueString(RogueString* arg_0);
void RogueSystem__init_class(void);
void RogueSystem__init_object( RogueSystem* THIS );
RogueString* RogueSystem__type_name( RogueSystem* THIS );
void RogueError__init_object( RogueError* THIS );
RogueString* RogueError__type_name( RogueError* THIS );
void RogueProcessResult__init__RogueInt32_RogueByteList_RogueByteList( RogueProcessResult* THIS, RogueInt32 _auto_store_exit_code_0, RogueByteList* _auto_store_output_bytes_1, RogueByteList* _auto_store_error_bytes_2 );
RogueString* RogueProcessResult__error_string( RogueProcessResult* THIS );
RogueString* RogueProcessResult__output_string( RogueProcessResult* THIS );
RogueLogical RogueProcessResult__success( RogueProcessResult* THIS );
RogueString* RogueProcessResult__toxRogueStringx( RogueProcessResult* THIS );
void RogueProcessResult__init_object( RogueProcessResult* THIS );
RogueString* RogueProcessResult__type_name( RogueProcessResult* THIS );
RogueProcess* RogueProcess__create__RogueString_RogueLogical_RogueLogical_RogueLogical_RogueLogical(RogueString* cmd_0, RogueLogical readable_1, RogueLogical writable_2, RogueLogical is_blocking_3, RogueLogical env_4);
RogueProcessResult* RogueProcess__run__RogueString_RogueLogical_RogueLogical(RogueString* cmd_0, RogueLogical env_1, RogueLogical writable_2);
RogueProcessResult* RogueProcess__run__RogueProcess(RogueProcess* process_0);
RogueByteList* RogueProcess__error_bytes( RogueProcess* THIS );
RogueProcessResult* RogueProcess__finish( RogueProcess* THIS );
void RogueProcess__on_cleanup( RogueProcess* THIS );
RogueByteList* RogueProcess__output_bytes( RogueProcess* THIS );
RogueLogical RogueProcess__update_io__RogueLogical( RogueProcess* THIS, RogueLogical poll_blocks_0 );
void RogueProcess__init_object( RogueProcess* THIS );
RogueString* RogueProcess__type_name( RogueProcess* THIS );
void RogueOPARENFunctionOPARENRogueStringCPARENCPAREN__init_object( RogueOPARENFunctionOPARENRogueStringCPARENCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueStringCPARENCPAREN__type_name( RogueOPARENFunctionOPARENRogueStringCPARENCPAREN* THIS );
void RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN__init_object( RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN__type_name( RogueOPARENFunctionOPARENRogueStringCPARENRETURNSRogueLogicalCPAREN* THIS );
void RogueListReaderxRogueStringx__init__RogueStringList_RogueInt32( RogueListReaderxRogueStringx* THIS, RogueStringList* _auto_store_list_0, RogueInt32 _auto_store_position_1 );
RogueLogical RogueListReaderxRogueStringx__has_another( RogueListReaderxRogueStringx* THIS );
RogueString* RogueListReaderxRogueStringx__read( RogueListReaderxRogueStringx* THIS );
void RogueListReaderxRogueStringx__init_object( RogueListReaderxRogueStringx* THIS );
RogueString* RogueListReaderxRogueStringx__toxRogueStringx( RogueListReaderxRogueStringx* THIS );
RogueString* RogueListReaderxRogueStringx__type_name( RogueListReaderxRogueStringx* THIS );
RogueInt32 RogueListReaderxRogueStringx__position( RogueListReaderxRogueStringx* THIS );
void RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN__init_object( RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN__type_name( RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* THIS );
void RogueCharacterList__init( RogueCharacterList* THIS );
void RogueCharacterList__init__RogueInt32( RogueCharacterList* THIS, RogueInt32 capacity_0 );
void RogueCharacterList__init_object( RogueCharacterList* THIS );
void RogueCharacterList__on_cleanup( RogueCharacterList* THIS );
void RogueCharacterList__add__RogueCharacter( RogueCharacterList* THIS, RogueCharacter value_0 );
void RogueCharacterList__clear( RogueCharacterList* THIS );
RogueString* RogueCharacterList__description( RogueCharacterList* THIS );
void RogueCharacterList__discard_from__RogueInt32( RogueCharacterList* THIS, RogueInt32 index_0 );
RogueCharacter RogueCharacterList__get__RogueInt32( RogueCharacterList* THIS, RogueInt32 index_0 );
void RogueCharacterList__on_return_to_pool( RogueCharacterList* THIS );
void RogueCharacterList__reserve__RogueInt32( RogueCharacterList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueCharacterList__toxRogueStringx( RogueCharacterList* THIS );
void RogueCharacterList__zero__RogueInt32_RogueOptionalInt32( RogueCharacterList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueCharacterList__type_name( RogueCharacterList* THIS );
void RogueStringReader__init__RogueString( RogueStringReader* THIS, RogueString* _auto_store_string_0 );
RogueLogical RogueStringReader__has_another( RogueStringReader* THIS );
RogueCharacter RogueStringReader__read( RogueStringReader* THIS );
void RogueStringReader__init_object( RogueStringReader* THIS );
RogueString* RogueStringReader__toxRogueStringx( RogueStringReader* THIS );
RogueString* RogueStringReader__type_name( RogueStringReader* THIS );
RogueInt32 RogueStringReader__position( RogueStringReader* THIS );
void RogueBufferedPrintWriter__init__RogueWriterxRogueBytex( RogueBufferedPrintWriter* THIS, RogueObject* _auto_store_output_0 );
void RogueBufferedPrintWriter__close( RogueBufferedPrintWriter* THIS );
void RogueBufferedPrintWriter__flush__RogueString( RogueBufferedPrintWriter* THIS, RogueString* buffer_0 );
void RogueBufferedPrintWriter__init_object( RogueBufferedPrintWriter* THIS );
RogueString* RogueBufferedPrintWriter__type_name( RogueBufferedPrintWriter* THIS );
void RogueBufferedPrintWriter__flush( RogueBufferedPrintWriter* THIS );
void RogueBufferedPrintWriter__print__RogueCharacter( RogueBufferedPrintWriter* THIS, RogueCharacter value_0 );
void RogueBufferedPrintWriter__print__RogueObject( RogueBufferedPrintWriter* THIS, RogueObject* value_0 );
void RogueBufferedPrintWriter__print__RogueString( RogueBufferedPrintWriter* THIS, RogueString* value_0 );
void RogueBufferedPrintWriter__println( RogueBufferedPrintWriter* THIS );
void RogueBufferedPrintWriter__println__RogueObject( RogueBufferedPrintWriter* THIS, RogueObject* value_0 );
void RogueBufferedPrintWriter__println__RogueString( RogueBufferedPrintWriter* THIS, RogueString* value_0 );
extern RogueStringPool* RogueStringPool_singleton;

void RogueStringPool__init_object( RogueStringPool* THIS );
RogueString* RogueStringPool__type_name( RogueStringPool* THIS );
void RogueListReaderxRogueBytex__init__RogueByteList_RogueInt32( RogueListReaderxRogueBytex* THIS, RogueByteList* _auto_store_list_0, RogueInt32 _auto_store_position_1 );
RogueLogical RogueListReaderxRogueBytex__has_another( RogueListReaderxRogueBytex* THIS );
RogueByte RogueListReaderxRogueBytex__read( RogueListReaderxRogueBytex* THIS );
void RogueListReaderxRogueBytex__seek__RogueInt32( RogueListReaderxRogueBytex* THIS, RogueInt32 pos_0 );
void RogueListReaderxRogueBytex__init_object( RogueListReaderxRogueBytex* THIS );
RogueString* RogueListReaderxRogueBytex__toxRogueStringx( RogueListReaderxRogueBytex* THIS );
RogueString* RogueListReaderxRogueBytex__type_name( RogueListReaderxRogueBytex* THIS );
void RogueListReaderxRogueBytex__close( RogueListReaderxRogueBytex* THIS );
RogueInt32 RogueListReaderxRogueBytex__position( RogueListReaderxRogueBytex* THIS );
extern RogueLogical RogueConsoleMode__g_configured_on_exit;
void RogueConsoleMode__init_class(void);
void RogueConsoleMode__init_object( RogueConsoleMode* THIS );
RogueString* RogueConsoleMode__type_name( RogueConsoleMode* THIS );
extern RogueConsole* RogueConsole_singleton;

void RogueConsole__init( RogueConsole* THIS );
RogueObject* RogueConsole__error( RogueConsole* THIS );
void RogueConsole__flush__RogueString( RogueConsole* THIS, RogueString* buffer_0 );
RogueLogical RogueConsole__has_another( RogueConsole* THIS );
RogueConsoleMode* RogueConsole__mode( RogueConsole* THIS );
RogueCharacter RogueConsole__read( RogueConsole* THIS );
void RogueConsole__set_immediate_mode__RogueLogical( RogueConsole* THIS, RogueLogical setting_0 );
RogueInt32 RogueConsole__width( RogueConsole* THIS );
void RogueConsole__write__RogueString( RogueConsole* THIS, RogueString* value_0 );
RogueLogical RogueConsole___fill_input_buffer__RogueInt32( RogueConsole* THIS, RogueInt32 minimum_0 );
void RogueConsole__init_object( RogueConsole* THIS );
RogueString* RogueConsole__toxRogueStringx( RogueConsole* THIS );
RogueString* RogueConsole__type_name( RogueConsole* THIS );
void RogueConsole__close( RogueConsole* THIS );
void RogueConsole__flush( RogueConsole* THIS );
void RogueConsole__print__RogueCharacter( RogueConsole* THIS, RogueCharacter value_0 );
void RogueConsole__print__RogueObject( RogueConsole* THIS, RogueObject* value_0 );
void RogueConsole__print__RogueString( RogueConsole* THIS, RogueString* value_0 );
void RogueConsole__println( RogueConsole* THIS );
void RogueConsole__println__RogueObject( RogueConsole* THIS, RogueObject* value_0 );
void RogueConsole__println__RogueString( RogueConsole* THIS, RogueString* value_0 );
void RogueFileReader__init__RogueString( RogueFileReader* THIS, RogueString* _filepath_0 );
void RogueFileReader__on_cleanup( RogueFileReader* THIS );
void RogueFileReader__close( RogueFileReader* THIS );
RogueInt64 RogueFileReader__fp( RogueFileReader* THIS );
RogueLogical RogueFileReader__has_another( RogueFileReader* THIS );
RogueFileReader* RogueFileReader__on_use( RogueFileReader* THIS );
void RogueFileReader__on_end_use__RogueFileReader( RogueFileReader* THIS, RogueFileReader* this_reader_0 );
RogueLogical RogueFileReader__open__RogueString( RogueFileReader* THIS, RogueString* _auto_store_filepath_0 );
RogueByte RogueFileReader__peek( RogueFileReader* THIS );
RogueByte RogueFileReader__read( RogueFileReader* THIS );
RogueInt32 RogueFileReader__read__RogueByteList_RogueInt32( RogueFileReader* THIS, RogueByteList* result_0, RogueInt32 limit_1 );
void RogueFileReader__seek__RogueInt32( RogueFileReader* THIS, RogueInt32 pos_0 );
void RogueFileReader__init_object( RogueFileReader* THIS );
RogueString* RogueFileReader__toxRogueStringx( RogueFileReader* THIS );
RogueString* RogueFileReader__type_name( RogueFileReader* THIS );
RogueInt32 RogueFileReader__position( RogueFileReader* THIS );
void RogueFileWriter__init__RogueString_RogueLogical( RogueFileWriter* THIS, RogueString* _filepath_0, RogueLogical append_1 );
void RogueFileWriter__on_cleanup( RogueFileWriter* THIS );
void RogueFileWriter__close( RogueFileWriter* THIS );
void RogueFileWriter__flush( RogueFileWriter* THIS );
RogueInt64 RogueFileWriter__fp( RogueFileWriter* THIS );
RogueLogical RogueFileWriter__open__RogueString_RogueLogical( RogueFileWriter* THIS, RogueString* _auto_store_filepath_0, RogueLogical append_1 );
void RogueFileWriter__write__RogueByte( RogueFileWriter* THIS, RogueByte ch_0 );
void RogueFileWriter__write__RogueByteList( RogueFileWriter* THIS, RogueByteList* bytes_0 );
void RogueFileWriter__write__RogueString( RogueFileWriter* THIS, RogueString* data_0 );
void RogueFileWriter__init_object( RogueFileWriter* THIS );
RogueString* RogueFileWriter__type_name( RogueFileWriter* THIS );
RogueInt32 RogueFileWriter__position( RogueFileWriter* THIS );
void RogueUTF16String__init( RogueUTF16String* THIS );
void RogueUTF16String__init__RogueString( RogueUTF16String* THIS, RogueString* text_0 );
void RogueUTF16String__init__RogueWordPointer( RogueUTF16String* THIS, RogueWordPointer wchar_data_0 );
void RogueUTF16String__on_return_to_pool( RogueUTF16String* THIS );
RogueString* RogueUTF16String__toxRogueStringx( RogueUTF16String* THIS );
void RogueUTF16String__init_object( RogueUTF16String* THIS );
RogueString* RogueUTF16String__type_name( RogueUTF16String* THIS );
void RogueWindowsProcess__init__RogueString_RogueLogical_RogueLogical_RogueLogical_RogueLogical( RogueWindowsProcess* THIS, RogueString* cmd_0, RogueLogical readable_1, RogueLogical writable_2, RogueLogical is_blocking_3, RogueLogical env_4 );
void RogueWindowsProcess__init__RogueStringList_RogueLogical_RogueLogical_RogueLogical_RogueLogical( RogueWindowsProcess* THIS, RogueStringList* _auto_store_args_0, RogueLogical readable_1, RogueLogical writable_2, RogueLogical _auto_store_is_blocking_3, RogueLogical env_4 );
RogueLogical RogueWindowsProcess__launch__RogueLogical_RogueLogical_RogueLogical( RogueWindowsProcess* THIS, RogueLogical readable_0, RogueLogical writable_1, RogueLogical env_2 );
RogueLogical RogueWindowsProcess__is_finished( RogueWindowsProcess* THIS );
RogueProcessResult* RogueWindowsProcess__finish( RogueWindowsProcess* THIS );
void RogueWindowsProcess__on_cleanup( RogueWindowsProcess* THIS );
RogueLogical RogueWindowsProcess__update_io( RogueWindowsProcess* THIS );
void RogueWindowsProcess__init_object( RogueWindowsProcess* THIS );
RogueString* RogueWindowsProcess__type_name( RogueWindowsProcess* THIS );
void RoguePosixProcess__init__RogueStringList_RogueLogical_RogueLogical_RogueLogical_RogueLogical( RoguePosixProcess* THIS, RogueStringList* _auto_store_args_0, RogueLogical readable_1, RogueLogical writable_2, RogueLogical _auto_store_is_blocking_3, RogueLogical env_4 );
RogueLogical RoguePosixProcess__launch__RogueLogical_RogueLogical_RogueLogical( RoguePosixProcess* THIS, RogueLogical readable_0, RogueLogical writable_1, RogueLogical env_2 );
RogueProcessResult* RoguePosixProcess__finish( RoguePosixProcess* THIS );
RogueLogical RoguePosixProcess__update_io__RogueLogical( RoguePosixProcess* THIS, RogueLogical poll_blocks_0 );
void RoguePosixProcess__init_object( RoguePosixProcess* THIS );
RogueString* RoguePosixProcess__type_name( RoguePosixProcess* THIS );
void RogueStringEncodingList__init__RogueInt32( RogueStringEncodingList* THIS, RogueInt32 capacity_0 );
void RogueStringEncodingList__init_object( RogueStringEncodingList* THIS );
void RogueStringEncodingList__on_cleanup( RogueStringEncodingList* THIS );
void RogueStringEncodingList__add__RogueStringEncoding( RogueStringEncodingList* THIS, RogueStringEncoding value_0 );
void RogueStringEncodingList__clear( RogueStringEncodingList* THIS );
RogueString* RogueStringEncodingList__description( RogueStringEncodingList* THIS );
void RogueStringEncodingList__discard_from__RogueInt32( RogueStringEncodingList* THIS, RogueInt32 index_0 );
RogueStringEncoding RogueStringEncodingList__get__RogueInt32( RogueStringEncodingList* THIS, RogueInt32 index_0 );
void RogueStringEncodingList__on_return_to_pool( RogueStringEncodingList* THIS );
void RogueStringEncodingList__reserve__RogueInt32( RogueStringEncodingList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueStringEncodingList__toxRogueStringx( RogueStringEncodingList* THIS );
void RogueStringEncodingList__zero__RogueInt32_RogueOptionalInt32( RogueStringEncodingList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueStringEncodingList__type_name( RogueStringEncodingList* THIS );
extern RogueObjectPoolxRogueStringx* RogueObjectPoolxRogueStringx_singleton;

RogueString* RogueObjectPoolxRogueStringx__on_use( RogueObjectPoolxRogueStringx* THIS );
void RogueObjectPoolxRogueStringx__on_end_use__RogueString( RogueObjectPoolxRogueStringx* THIS, RogueString* object_0 );
void RogueObjectPoolxRogueStringx__init_object( RogueObjectPoolxRogueStringx* THIS );
RogueString* RogueObjectPoolxRogueStringx__type_name( RogueObjectPoolxRogueStringx* THIS );
void RogueValueList__init( RogueValueList* THIS );
void RogueValueList__init_object( RogueValueList* THIS );
void RogueValueList__on_cleanup( RogueValueList* THIS );
void RogueValueList__add__RogueValue( RogueValueList* THIS, RogueValue value_0 );
void RogueValueList__clear( RogueValueList* THIS );
RogueLogical RogueValueList__contains__RogueValue( RogueValueList* THIS, RogueValue value_0 );
void RogueValueList__copy__RogueInt32_RogueInt32_RogueValueList_RogueInt32( RogueValueList* THIS, RogueInt32 src_i1_0, RogueInt32 src_count_1, RogueValueList* dest_2, RogueInt32 dest_i1_3 );
RogueString* RogueValueList__description( RogueValueList* THIS );
void RogueValueList__discard_from__RogueInt32( RogueValueList* THIS, RogueInt32 index_0 );
void RogueValueList__ensure_capacity__RogueInt32( RogueValueList* THIS, RogueInt32 desired_capacity_0 );
void RogueValueList__expand_to_count__RogueInt32( RogueValueList* THIS, RogueInt32 minimum_count_0 );
RogueValue RogueValueList__get__RogueInt32( RogueValueList* THIS, RogueInt32 index_0 );
RogueOptionalInt32 RogueValueList__locate__RogueValue_RogueOptionalInt32( RogueValueList* THIS, RogueValue value_0, RogueOptionalInt32 i1_1 );
void RogueValueList__on_return_to_pool( RogueValueList* THIS );
RogueValue RogueValueList__remove_at__RogueInt32( RogueValueList* THIS, RogueInt32 index_0 );
void RogueValueList__reserve__RogueInt32( RogueValueList* THIS, RogueInt32 additional_capacity_0 );
void RogueValueList__set__RogueInt32_RogueValue( RogueValueList* THIS, RogueInt32 index_0, RogueValue value_1 );
RogueString* RogueValueList__toxRogueStringx( RogueValueList* THIS );
void RogueValueList__zero__RogueInt32_RogueOptionalInt32( RogueValueList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueValueList__type_name( RogueValueList* THIS );
void RogueTableEntryxRogueString_RogueValuex__init__RogueString_RogueValue_RogueInt32( RogueTableEntryxRogueString_RogueValuex* THIS, RogueString* _key_0, RogueValue _value_1, RogueInt32 _hash_2 );
RogueString* RogueTableEntryxRogueString_RogueValuex__toxRogueStringx( RogueTableEntryxRogueString_RogueValuex* THIS );
void RogueTableEntryxRogueString_RogueValuex__init_object( RogueTableEntryxRogueString_RogueValuex* THIS );
RogueString* RogueTableEntryxRogueString_RogueValuex__type_name( RogueTableEntryxRogueString_RogueValuex* THIS );
void RogueTableEntryxRogueString_RogueValuexList__init( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueTableEntryxRogueString_RogueValuexList__init_object( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueTableEntryxRogueString_RogueValuexList__on_cleanup( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueTableEntryxRogueString_RogueValuexList__clear( RogueTableEntryxRogueString_RogueValuexList* THIS );
RogueString* RogueTableEntryxRogueString_RogueValuexList__description( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueTableEntryxRogueString_RogueValuexList__discard_from__RogueInt32( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 index_0 );
void RogueTableEntryxRogueString_RogueValuexList__ensure_capacity__RogueInt32( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 desired_capacity_0 );
void RogueTableEntryxRogueString_RogueValuexList__expand_to_count__RogueInt32( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 minimum_count_0 );
RogueTableEntryxRogueString_RogueValuex* RogueTableEntryxRogueString_RogueValuexList__get__RogueInt32( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 index_0 );
void RogueTableEntryxRogueString_RogueValuexList__on_return_to_pool( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueTableEntryxRogueString_RogueValuexList__reserve__RogueInt32( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 additional_capacity_0 );
void RogueTableEntryxRogueString_RogueValuexList__set__RogueInt32_RogueTableEntryxRogueString_RogueValuex( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 index_0, RogueTableEntryxRogueString_RogueValuex* value_1 );
RogueString* RogueTableEntryxRogueString_RogueValuexList__toxRogueStringx( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueTableEntryxRogueString_RogueValuexList__zero__RogueInt32_RogueOptionalInt32( RogueTableEntryxRogueString_RogueValuexList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueTableEntryxRogueString_RogueValuexList__type_name( RogueTableEntryxRogueString_RogueValuexList* THIS );
void RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN__init_object( RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN__type_name( RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueValuex_RogueTableEntryxRogueString_RogueValuexCPARENRETURNSRogueLogicalCPAREN* THIS );
void RogueTableBTABLERogueString_RogueValueETABLE__init( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
void RogueTableBTABLERogueString_RogueValueETABLE__init__RogueInt32( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueInt32 bin_count_0 );
RogueValue RogueTableBTABLERogueString_RogueValueETABLE__at__RogueInt32( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueInt32 index_0 );
RogueLogical RogueTableBTABLERogueString_RogueValueETABLE__contains__RogueString( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueString* key_0 );
RogueTableEntriesIteratorxRogueString_RogueValuex RogueTableBTABLERogueString_RogueValueETABLE__entries( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
RogueTableEntryxRogueString_RogueValuex* RogueTableBTABLERogueString_RogueValueETABLE__entry_at__RogueInt32( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueInt32 index_0 );
RogueTableEntryxRogueString_RogueValuex* RogueTableBTABLERogueString_RogueValueETABLE__find__RogueString( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueString* key_0 );
RogueValue RogueTableBTABLERogueString_RogueValueETABLE__get__RogueString( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueString* key_0 );
RogueTableKeysIteratorxRogueString_RogueValuex RogueTableBTABLERogueString_RogueValueETABLE__keys( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
RogueString* RogueTableBTABLERogueString_RogueValueETABLE__print_to__RogueString( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueString* buffer_0 );
RogueTableEntryxRogueString_RogueValuex* RogueTableBTABLERogueString_RogueValueETABLE__remove__RogueTableEntryxRogueString_RogueValuex( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueTableEntryxRogueString_RogueValuex* entry_0 );
RogueValue RogueTableBTABLERogueString_RogueValueETABLE__remove_at__RogueInt32( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueInt32 index_0 );
void RogueTableBTABLERogueString_RogueValueETABLE__set__RogueString_RogueValue( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueString* key_0, RogueValue value_1 );
RogueString* RogueTableBTABLERogueString_RogueValueETABLE__toxRogueStringx( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
void RogueTableBTABLERogueString_RogueValueETABLE___adjust_entry_order__RogueTableEntryxRogueString_RogueValuex( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueTableEntryxRogueString_RogueValuex* entry_0 );
void RogueTableBTABLERogueString_RogueValueETABLE___grow( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
void RogueTableBTABLERogueString_RogueValueETABLE___place_entry_in_order__RogueTableEntryxRogueString_RogueValuex( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueTableEntryxRogueString_RogueValuex* entry_0 );
void RogueTableBTABLERogueString_RogueValueETABLE___unlink__RogueTableEntryxRogueString_RogueValuex( RogueTableBTABLERogueString_RogueValueETABLE* THIS, RogueTableEntryxRogueString_RogueValuex* entry_0 );
void RogueTableBTABLERogueString_RogueValueETABLE__init_object( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
RogueString* RogueTableBTABLERogueString_RogueValueETABLE__type_name( RogueTableBTABLERogueString_RogueValueETABLE* THIS );
void RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN__init_object( RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN__type_name( RogueOPARENFunctionOPARENRogueValueCPARENRETURNSRogueLogicalCPAREN* THIS );
void RogueConsoleEventTypeList__init__RogueInt32( RogueConsoleEventTypeList* THIS, RogueInt32 capacity_0 );
void RogueConsoleEventTypeList__init_object( RogueConsoleEventTypeList* THIS );
void RogueConsoleEventTypeList__on_cleanup( RogueConsoleEventTypeList* THIS );
void RogueConsoleEventTypeList__add__RogueConsoleEventType( RogueConsoleEventTypeList* THIS, RogueConsoleEventType value_0 );
void RogueConsoleEventTypeList__clear( RogueConsoleEventTypeList* THIS );
RogueString* RogueConsoleEventTypeList__description( RogueConsoleEventTypeList* THIS );
void RogueConsoleEventTypeList__discard_from__RogueInt32( RogueConsoleEventTypeList* THIS, RogueInt32 index_0 );
RogueConsoleEventType RogueConsoleEventTypeList__get__RogueInt32( RogueConsoleEventTypeList* THIS, RogueInt32 index_0 );
void RogueConsoleEventTypeList__on_return_to_pool( RogueConsoleEventTypeList* THIS );
void RogueConsoleEventTypeList__reserve__RogueInt32( RogueConsoleEventTypeList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueConsoleEventTypeList__toxRogueStringx( RogueConsoleEventTypeList* THIS );
void RogueConsoleEventTypeList__zero__RogueInt32_RogueOptionalInt32( RogueConsoleEventTypeList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueConsoleEventTypeList__type_name( RogueConsoleEventTypeList* THIS );
void RogueFileListingOptionList__init__RogueInt32( RogueFileListingOptionList* THIS, RogueInt32 capacity_0 );
void RogueFileListingOptionList__init_object( RogueFileListingOptionList* THIS );
void RogueFileListingOptionList__on_cleanup( RogueFileListingOptionList* THIS );
void RogueFileListingOptionList__add__RogueFileListingOption( RogueFileListingOptionList* THIS, RogueFileListingOption value_0 );
void RogueFileListingOptionList__clear( RogueFileListingOptionList* THIS );
RogueString* RogueFileListingOptionList__description( RogueFileListingOptionList* THIS );
void RogueFileListingOptionList__discard_from__RogueInt32( RogueFileListingOptionList* THIS, RogueInt32 index_0 );
RogueFileListingOption RogueFileListingOptionList__get__RogueInt32( RogueFileListingOptionList* THIS, RogueInt32 index_0 );
void RogueFileListingOptionList__on_return_to_pool( RogueFileListingOptionList* THIS );
void RogueFileListingOptionList__reserve__RogueInt32( RogueFileListingOptionList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueFileListingOptionList__toxRogueStringx( RogueFileListingOptionList* THIS );
void RogueFileListingOptionList__zero__RogueInt32_RogueOptionalInt32( RogueFileListingOptionList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueFileListingOptionList__type_name( RogueFileListingOptionList* THIS );
void RogueWindowsProcessReader__init( RogueWindowsProcessReader* THIS );
void RogueWindowsProcessReader__close( RogueWindowsProcessReader* THIS );
RogueLogical RogueWindowsProcessReader__fill_buffer( RogueWindowsProcessReader* THIS );
RogueLogical RogueWindowsProcessReader__has_another( RogueWindowsProcessReader* THIS );
RogueByte RogueWindowsProcessReader__read( RogueWindowsProcessReader* THIS );
void RogueWindowsProcessReader__init_object( RogueWindowsProcessReader* THIS );
RogueString* RogueWindowsProcessReader__toxRogueStringx( RogueWindowsProcessReader* THIS );
RogueString* RogueWindowsProcessReader__type_name( RogueWindowsProcessReader* THIS );
RogueInt32 RogueWindowsProcessReader__position( RogueWindowsProcessReader* THIS );
void RogueWindowsProcessReader__seek__RogueInt32( RogueWindowsProcessReader* THIS, RogueInt32 pos_0 );
void RogueWindowsProcessReader__skip__RogueInt32( RogueWindowsProcessReader* THIS, RogueInt32 n_0 );
void RogueWindowsProcessWriter__init( RogueWindowsProcessWriter* THIS );
void RogueWindowsProcessWriter__close( RogueWindowsProcessWriter* THIS );
void RogueWindowsProcessWriter__close_pipe( RogueWindowsProcessWriter* THIS );
void RogueWindowsProcessWriter__flush( RogueWindowsProcessWriter* THIS );
void RogueWindowsProcessWriter__write__RogueByteList( RogueWindowsProcessWriter* THIS, RogueByteList* list_0 );
void RogueWindowsProcessWriter__init_object( RogueWindowsProcessWriter* THIS );
RogueString* RogueWindowsProcessWriter__type_name( RogueWindowsProcessWriter* THIS );
void RogueFDReader__init__RogueInt32_RogueLogical( RogueFDReader* THIS, RogueInt32 _auto_store_fd_0, RogueLogical _auto_store_auto_close_1 );
void RogueFDReader__on_cleanup( RogueFDReader* THIS );
RogueLogical RogueFDReader__buffer_more( RogueFDReader* THIS );
void RogueFDReader__close( RogueFDReader* THIS );
RogueLogical RogueFDReader__has_another( RogueFDReader* THIS );
RogueByte RogueFDReader__peek( RogueFDReader* THIS );
RogueByte RogueFDReader__read( RogueFDReader* THIS );
void RogueFDReader__init_object( RogueFDReader* THIS );
RogueString* RogueFDReader__toxRogueStringx( RogueFDReader* THIS );
RogueString* RogueFDReader__type_name( RogueFDReader* THIS );
RogueInt32 RogueFDReader__position( RogueFDReader* THIS );
void RogueFDReader__seek__RogueInt32( RogueFDReader* THIS, RogueInt32 pos_0 );
void RogueFDReader__skip__RogueInt32( RogueFDReader* THIS, RogueInt32 n_0 );
void RoguePosixProcessReader__init__RogueProcess_RogueInt32( RoguePosixProcessReader* THIS, RogueProcess* _auto_store_process_0, RogueInt32 fd_1 );
void RoguePosixProcessReader__close( RoguePosixProcessReader* THIS );
RogueLogical RoguePosixProcessReader__has_another( RoguePosixProcessReader* THIS );
RogueByte RoguePosixProcessReader__read( RoguePosixProcessReader* THIS );
void RoguePosixProcessReader__init_object( RoguePosixProcessReader* THIS );
RogueString* RoguePosixProcessReader__toxRogueStringx( RoguePosixProcessReader* THIS );
RogueString* RoguePosixProcessReader__type_name( RoguePosixProcessReader* THIS );
RogueInt32 RoguePosixProcessReader__position( RoguePosixProcessReader* THIS );
void RoguePosixProcessReader__seek__RogueInt32( RoguePosixProcessReader* THIS, RogueInt32 pos_0 );
void RoguePosixProcessReader__skip__RogueInt32( RoguePosixProcessReader* THIS, RogueInt32 n_0 );
extern RogueMorlock* RogueMorlock_singleton;

void RogueMorlock__init__RogueStringList( RogueMorlock* THIS, RogueStringList* args_0 );
void RogueMorlock__perform_action__RogueValue( RogueMorlock* THIS, RogueValue cmd_0 );
void RogueMorlock__create_folder__RogueString_RogueLogical( RogueMorlock* THIS, RogueString* path_0, RogueLogical chown_1 );
RogueError* RogueMorlock__error__RogueString( RogueMorlock* THIS, RogueString* message_0 );
void RogueMorlock__header( RogueMorlock* THIS );
void RogueMorlock__header__RogueString( RogueMorlock* THIS, RogueString* message_0 );
RogueStringList* RogueMorlock__installed_packages( RogueMorlock* THIS );
void RogueMorlock__run_script__RogueValue_RoguePackageInfo( RogueMorlock* THIS, RogueValue command_0, RoguePackageInfo* info_1 );
RoguePackageInfo* RogueMorlock__resolve_package__RogueString_RogueLogical( RogueMorlock* THIS, RogueString* name_0, RogueLogical allow_local_script_1 );
RogueValue RogueMorlock__parse_args__RogueStringList( RogueMorlock* THIS, RogueStringList* args_0 );
void RogueMorlock__print_usage( RogueMorlock* THIS );
void RogueMorlock__init_object( RogueMorlock* THIS );
RogueString* RogueMorlock__type_name( RogueMorlock* THIS );
void RoguePackageInfo__init__RogueString_RogueLogical( RoguePackageInfo* THIS, RogueString* text_0, RogueLogical is_script_1 );
void RoguePackageInfo__ensure_script_exists( RoguePackageInfo* THIS );
void RoguePackageInfo__fetch_latest_script( RoguePackageInfo* THIS );
RogueLogical RoguePackageInfo__create_default_script__RogueValue( RoguePackageInfo* THIS, RogueValue contents_0 );
RogueLogical RoguePackageInfo__execute__RogueString_RogueLogical_RogueLogical_RogueLogical( RoguePackageInfo* THIS, RogueString* cmd_0, RogueLogical suppress_error_1, RogueLogical allow_sudo_2, RogueLogical quiet_3 );
RogueValue RoguePackageInfo__package_args( RoguePackageInfo* THIS );
RogueString* RoguePackageInfo__parse_package_name__RogueString( RoguePackageInfo* THIS, RogueString* script_0 );
void RoguePackageInfo__prepare_build_folder( RoguePackageInfo* THIS );
void RoguePackageInfo__init_object( RoguePackageInfo* THIS );
RogueString* RoguePackageInfo__type_name( RoguePackageInfo* THIS );
void RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN__init_object( RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN__type_name( RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx__init__RogueString_RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN_RogueInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* THIS, RogueString* _key_0, RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* _value_1, RogueInt32 _hash_2 );
RogueString* RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx__toxRogueStringx( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx__init_object( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* THIS );
RogueString* RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx__type_name( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__init( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__init_object( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__on_cleanup( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__clear( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
RogueString* RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__description( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__discard_from__RogueInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 index_0 );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__ensure_capacity__RogueInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 desired_capacity_0 );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__expand_to_count__RogueInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 minimum_count_0 );
RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__get__RogueInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 index_0 );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__on_return_to_pool( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__reserve__RogueInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 additional_capacity_0 );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__set__RogueInt32_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 index_0, RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* value_1 );
RogueString* RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__toxRogueStringx( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__zero__RogueInt32_RogueOptionalInt32( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList__type_name( RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxList* THIS );
void RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN__init_object( RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN__type_name( RogueOPARENFunctionOPARENRogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx_RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENxCPARENRETURNSRogueLogicalCPAREN* THIS );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__init( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__init__RogueInt32( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueInt32 bin_count_0 );
RogueLogical RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__contains__RogueString( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueString* key_0 );
RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__find__RogueString( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueString* key_0 );
RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__get__RogueString( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueString* key_0 );
RogueString* RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__print_to__RogueString( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueString* buffer_0 );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__set__RogueString_RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueString* key_0, RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* value_1 );
RogueString* RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__toxRogueStringx( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE___adjust_entry_order__RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* entry_0 );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE___grow( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE___place_entry_in_order__RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* entry_0 );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE___unlink__RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS, RogueTableEntryxRogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENx* entry_0 );
void RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__init_object( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS );
RogueString* RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE__type_name( RogueTableBTABLERogueString_OPARENFunctionOPARENRogueCommandLineParserCPARENCPARENETABLE* THIS );
void RogueCommandLineParser__init( RogueCommandLineParser* THIS );
void RogueCommandLineParser__alias__RogueString_RogueString_RogueLogical( RogueCommandLineParser* THIS, RogueString* from_name_0, RogueString* to_name_1, RogueLogical require_value_2 );
void RogueCommandLineParser__on_unknown__RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN( RogueCommandLineParser* THIS, RogueOPARENFunctionOPARENRogueCommandLineParserCPARENCPAREN* _auto_store_unknown_handler_0 );
void RogueCommandLineParser__option__RogueString_RogueLogical_RogueLogical_RogueLogical_RogueValue_RogueString_RogueStringList( RogueCommandLineParser* THIS, RogueString* name_0, RogueLogical require_value_1, RogueLogical optional_2, RogueLogical multi_3, RogueValue default_4, RogueString* alias_5, RogueStringList* aliases_6 );
RogueValue RogueCommandLineParser__parse__RogueStringList( RogueCommandLineParser* THIS, RogueStringList* args_0 );
void RogueCommandLineParser___handle__RogueString( RogueCommandLineParser* THIS, RogueString* arg_0 );
RogueString* RogueCommandLineParser___extract_name__RogueString( RogueCommandLineParser* THIS, RogueString* name_0 );
void RogueCommandLineParser__init_object( RogueCommandLineParser* THIS );
RogueString* RogueCommandLineParser__type_name( RogueCommandLineParser* THIS );
extern RogueFunction_326* RogueFunction_326_singleton;

void RogueFunction_326__call__RogueCommandLineParser( RogueFunction_326* THIS, RogueCommandLineParser* parser_0 );
void RogueFunction_326__init_object( RogueFunction_326* THIS );
RogueString* RogueFunction_326__type_name( RogueFunction_326* THIS );
extern RogueFunction_327* RogueFunction_327_singleton;

void RogueFunction_327__call__RogueCommandLineParser( RogueFunction_327* THIS, RogueCommandLineParser* parser_0 );
void RogueFunction_327__init__RogueString_RogueString_RogueLogical( RogueFunction_327* THIS, RogueString* _auto_store_from_name_0, RogueString* _auto_store_to_name_1, RogueLogical _auto_store_require_value_2 );
void RogueFunction_327__init_object( RogueFunction_327* THIS );
RogueString* RogueFunction_327__type_name( RogueFunction_327* THIS );
extern RogueFunction_388* RogueFunction_388_singleton;

void RogueFunction_388__call__RogueCommandLineParser( RogueFunction_388* THIS, RogueCommandLineParser* parser_0 );
void RogueFunction_388__init__RogueLogical_RogueLogical( RogueFunction_388* THIS, RogueLogical _auto_store_require_value_0, RogueLogical _auto_store_multi_1 );
void RogueFunction_388__init_object( RogueFunction_388* THIS );
RogueString* RogueFunction_388__type_name( RogueFunction_388* THIS );
extern RogueFunction_389* RogueFunction_389_singleton;

void RogueFunction_389__call__RogueCommandLineParser( RogueFunction_389* THIS, RogueCommandLineParser* parser_0 );
void RogueFunction_389__init__RogueLogical_RogueLogical( RogueFunction_389* THIS, RogueLogical _auto_store_require_value_0, RogueLogical _auto_store_multi_1 );
void RogueFunction_389__init_object( RogueFunction_389* THIS );
RogueString* RogueFunction_389__type_name( RogueFunction_389* THIS );
void RogueInt32List__init( RogueInt32List* THIS );
void RogueInt32List__init_object( RogueInt32List* THIS );
void RogueInt32List__on_cleanup( RogueInt32List* THIS );
void RogueInt32List__add__RogueInt32( RogueInt32List* THIS, RogueInt32 value_0 );
void RogueInt32List__clear( RogueInt32List* THIS );
RogueString* RogueInt32List__description( RogueInt32List* THIS );
void RogueInt32List__discard_from__RogueInt32( RogueInt32List* THIS, RogueInt32 index_0 );
RogueInt32 RogueInt32List__get__RogueInt32( RogueInt32List* THIS, RogueInt32 index_0 );
void RogueInt32List__on_return_to_pool( RogueInt32List* THIS );
RogueInt32 RogueInt32List__remove_last( RogueInt32List* THIS );
void RogueInt32List__reserve__RogueInt32( RogueInt32List* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueInt32List__toxRogueStringx( RogueInt32List* THIS );
void RogueInt32List__zero__RogueInt32_RogueOptionalInt32( RogueInt32List* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueInt32List__type_name( RogueInt32List* THIS );
void RogueTableEntryxRogueString_RogueStringx__init__RogueString_RogueString_RogueInt32( RogueTableEntryxRogueString_RogueStringx* THIS, RogueString* _key_0, RogueString* _value_1, RogueInt32 _hash_2 );
RogueString* RogueTableEntryxRogueString_RogueStringx__toxRogueStringx( RogueTableEntryxRogueString_RogueStringx* THIS );
void RogueTableEntryxRogueString_RogueStringx__init_object( RogueTableEntryxRogueString_RogueStringx* THIS );
RogueString* RogueTableEntryxRogueString_RogueStringx__type_name( RogueTableEntryxRogueString_RogueStringx* THIS );
void RogueTableEntryxRogueString_RogueStringxList__init( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueTableEntryxRogueString_RogueStringxList__init_object( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueTableEntryxRogueString_RogueStringxList__on_cleanup( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueTableEntryxRogueString_RogueStringxList__clear( RogueTableEntryxRogueString_RogueStringxList* THIS );
RogueString* RogueTableEntryxRogueString_RogueStringxList__description( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueTableEntryxRogueString_RogueStringxList__discard_from__RogueInt32( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 index_0 );
void RogueTableEntryxRogueString_RogueStringxList__ensure_capacity__RogueInt32( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 desired_capacity_0 );
void RogueTableEntryxRogueString_RogueStringxList__expand_to_count__RogueInt32( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 minimum_count_0 );
RogueTableEntryxRogueString_RogueStringx* RogueTableEntryxRogueString_RogueStringxList__get__RogueInt32( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 index_0 );
void RogueTableEntryxRogueString_RogueStringxList__on_return_to_pool( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueTableEntryxRogueString_RogueStringxList__reserve__RogueInt32( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 additional_capacity_0 );
void RogueTableEntryxRogueString_RogueStringxList__set__RogueInt32_RogueTableEntryxRogueString_RogueStringx( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 index_0, RogueTableEntryxRogueString_RogueStringx* value_1 );
RogueString* RogueTableEntryxRogueString_RogueStringxList__toxRogueStringx( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueTableEntryxRogueString_RogueStringxList__zero__RogueInt32_RogueOptionalInt32( RogueTableEntryxRogueString_RogueStringxList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueTableEntryxRogueString_RogueStringxList__type_name( RogueTableEntryxRogueString_RogueStringxList* THIS );
void RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN__init_object( RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN* THIS );
RogueString* RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN__type_name( RogueOPARENFunctionOPARENRogueTableEntryxRogueString_RogueStringx_RogueTableEntryxRogueString_RogueStringxCPARENRETURNSRogueLogicalCPAREN* THIS );
void RogueTableBTABLERogueString_RogueStringETABLE__init( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
void RogueTableBTABLERogueString_RogueStringETABLE__init__RogueInt32( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueInt32 bin_count_0 );
RogueLogical RogueTableBTABLERogueString_RogueStringETABLE__contains__RogueString( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueString* key_0 );
RogueTableEntryxRogueString_RogueStringx* RogueTableBTABLERogueString_RogueStringETABLE__find__RogueString( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueString* key_0 );
RogueString* RogueTableBTABLERogueString_RogueStringETABLE__get__RogueString( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueString* key_0 );
RogueTableKeysIteratorxRogueString_RogueStringx RogueTableBTABLERogueString_RogueStringETABLE__keys( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
RogueString* RogueTableBTABLERogueString_RogueStringETABLE__print_to__RogueString( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueString* buffer_0 );
RogueString* RogueTableBTABLERogueString_RogueStringETABLE__remove__RogueString( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueString* key_0 );
RogueTableEntryxRogueString_RogueStringx* RogueTableBTABLERogueString_RogueStringETABLE__remove__RogueTableEntryxRogueString_RogueStringx( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueTableEntryxRogueString_RogueStringx* entry_0 );
void RogueTableBTABLERogueString_RogueStringETABLE__set__RogueString_RogueString( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueString* key_0, RogueString* value_1 );
RogueString* RogueTableBTABLERogueString_RogueStringETABLE__toxRogueStringx( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
RogueStringList* RogueTableBTABLERogueString_RogueStringETABLE__to_list( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
RogueTableValuesIteratorxRogueString_RogueStringx RogueTableBTABLERogueString_RogueStringETABLE__values( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
void RogueTableBTABLERogueString_RogueStringETABLE___adjust_entry_order__RogueTableEntryxRogueString_RogueStringx( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueTableEntryxRogueString_RogueStringx* entry_0 );
void RogueTableBTABLERogueString_RogueStringETABLE___grow( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
void RogueTableBTABLERogueString_RogueStringETABLE___place_entry_in_order__RogueTableEntryxRogueString_RogueStringx( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueTableEntryxRogueString_RogueStringx* entry_0 );
void RogueTableBTABLERogueString_RogueStringETABLE___unlink__RogueTableEntryxRogueString_RogueStringx( RogueTableBTABLERogueString_RogueStringETABLE* THIS, RogueTableEntryxRogueString_RogueStringx* entry_0 );
void RogueTableBTABLERogueString_RogueStringETABLE__init_object( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
RogueString* RogueTableBTABLERogueString_RogueStringETABLE__type_name( RogueTableBTABLERogueString_RogueStringETABLE* THIS );
extern RogueSystemEnvironment* RogueSystemEnvironment_singleton;

RogueTableBTABLERogueString_RogueStringETABLE* RogueSystemEnvironment__definitions( RogueSystemEnvironment* THIS );
RogueString* RogueSystemEnvironment__get__RogueString( RogueSystemEnvironment* THIS, RogueString* name_0 );
RogueStringList* RogueSystemEnvironment__names( RogueSystemEnvironment* THIS );
void RogueSystemEnvironment__init_object( RogueSystemEnvironment* THIS );
RogueString* RogueSystemEnvironment__type_name( RogueSystemEnvironment* THIS );
void RogueUTF16StringList__init( RogueUTF16StringList* THIS );
void RogueUTF16StringList__init_object( RogueUTF16StringList* THIS );
void RogueUTF16StringList__on_cleanup( RogueUTF16StringList* THIS );
void RogueUTF16StringList__add__RogueUTF16String( RogueUTF16StringList* THIS, RogueUTF16String* value_0 );
void RogueUTF16StringList__clear( RogueUTF16StringList* THIS );
RogueString* RogueUTF16StringList__description( RogueUTF16StringList* THIS );
void RogueUTF16StringList__discard_from__RogueInt32( RogueUTF16StringList* THIS, RogueInt32 index_0 );
RogueUTF16String* RogueUTF16StringList__get__RogueInt32( RogueUTF16StringList* THIS, RogueInt32 index_0 );
RogueLogical RogueUTF16StringList__is_empty( RogueUTF16StringList* THIS );
void RogueUTF16StringList__on_return_to_pool( RogueUTF16StringList* THIS );
RogueUTF16String* RogueUTF16StringList__remove_last( RogueUTF16StringList* THIS );
void RogueUTF16StringList__reserve__RogueInt32( RogueUTF16StringList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueUTF16StringList__toxRogueStringx( RogueUTF16StringList* THIS );
void RogueUTF16StringList__zero__RogueInt32_RogueOptionalInt32( RogueUTF16StringList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueUTF16StringList__type_name( RogueUTF16StringList* THIS );
extern RogueObjectPoolxRogueUTF16Stringx* RogueObjectPoolxRogueUTF16Stringx_singleton;

RogueUTF16String* RogueObjectPoolxRogueUTF16Stringx__on_use( RogueObjectPoolxRogueUTF16Stringx* THIS );
void RogueObjectPoolxRogueUTF16Stringx__on_end_use__RogueUTF16String( RogueObjectPoolxRogueUTF16Stringx* THIS, RogueUTF16String* object_0 );
void RogueObjectPoolxRogueUTF16Stringx__init_object( RogueObjectPoolxRogueUTF16Stringx* THIS );
RogueString* RogueObjectPoolxRogueUTF16Stringx__type_name( RogueObjectPoolxRogueUTF16Stringx* THIS );
void RogueDataReader__init__RogueReaderxRogueBytex( RogueDataReader* THIS, RogueObject* _auto_store_input_0 );
void RogueDataReader__init__RogueByteList( RogueDataReader* THIS, RogueByteList* list_0 );
RogueLogical RogueDataReader__has_another( RogueDataReader* THIS );
RogueInt32 RogueDataReader__position( RogueDataReader* THIS );
RogueByte RogueDataReader__read( RogueDataReader* THIS );
RogueInt32 RogueDataReader__read_int16_low_high( RogueDataReader* THIS );
void RogueDataReader__seek__RogueInt32( RogueDataReader* THIS, RogueInt32 pos_0 );
void RogueDataReader__init_object( RogueDataReader* THIS );
RogueString* RogueDataReader__toxRogueStringx( RogueDataReader* THIS );
RogueString* RogueDataReader__type_name( RogueDataReader* THIS );
void RogueDataReader__close( RogueDataReader* THIS );
extern RogueBootstrap* RogueBootstrap_singleton;

void RogueBootstrap__configure__RogueValue( RogueBootstrap* THIS, RogueValue _auto_store_cmd_0 );
void RogueBootstrap__delete_unused_package_versions( RogueBootstrap* THIS );
RogueLogical RogueBootstrap__execute__RogueString_RogueString_RogueLogical_RogueLogical( RogueBootstrap* THIS, RogueString* cmd_0, RogueString* error_message_1, RogueLogical suppress_error_2, RogueLogical quiet_3 );
RoguePackage* RogueBootstrap__package_instance__RogueString( RogueBootstrap* THIS, RogueString* url_0 );
void RogueBootstrap__install_morlock( RogueBootstrap* THIS );
void RogueBootstrap__install_rogo( RogueBootstrap* THIS );
void RogueBootstrap__install_rogue( RogueBootstrap* THIS );
void RogueBootstrap__print_installing_header( RogueBootstrap* THIS );
void RogueBootstrap__init_object( RogueBootstrap* THIS );
RogueString* RogueBootstrap__type_name( RogueBootstrap* THIS );
void RoguePackage__init( RoguePackage* THIS );
void RoguePackage__init__RogueString_RogueValue( RoguePackage* THIS, RogueString* _auto_store_name_0, RogueValue properties_1 );
void RoguePackage__init__RogueValue( RoguePackage* THIS, RogueValue _auto_store_properties_0 );
RogueString* RoguePackage__archive_folder( RoguePackage* THIS );
void RoguePackage__copy_executable__RogueString_RogueString( RoguePackage* THIS, RogueString* src_filepath_0, RogueString* dest_filename_1 );
void RoguePackage__create_folder__RogueString( RoguePackage* THIS, RogueString* folder_0 );
RogueString* RoguePackage__download( RoguePackage* THIS );
RogueError* RoguePackage__error__RogueString( RoguePackage* THIS, RogueString* message_0 );
void RoguePackage__execute__RogueString_RogueLogical( RoguePackage* THIS, RogueString* cmd_0, RogueLogical quiet_1 );
RogueString* RoguePackage__filename_for_url__RogueString( RoguePackage* THIS, RogueString* url_0 );
void RoguePackage__install_executable__RogueString_RogueString_RogueString_RogueString_RogueString_RogueLogical( RoguePackage* THIS, RogueString* default_0, RogueString* windows_1, RogueString* macos_2, RogueString* linux_3, RogueString* dest_filename_4, RogueLogical link_5 );
void RoguePackage__link( RoguePackage* THIS );
void RoguePackage__release__RogueInt32_RogueString_RoguePlatforms_RogueString( RoguePackage* THIS, RogueInt32 id_0, RogueString* url_1, RoguePlatforms* platforms_2, RogueString* version_3 );
void RoguePackage__save_cache( RoguePackage* THIS );
void RoguePackage__scan_repo_releases__RogueString_RogueString_RoguePlatforms( RoguePackage* THIS, RogueString* min_version_0, RogueString* max_version_1, RoguePlatforms* platforms_2 );
void RoguePackage__select_version( RoguePackage* THIS );
void RoguePackage__unpack__RogueString( RoguePackage* THIS, RogueString* destination_folder_0 );
void RoguePackage__init_object( RoguePackage* THIS );
RogueString* RoguePackage__type_name( RoguePackage* THIS );
RoguePlatforms* RoguePlatforms__all(void);
RoguePlatforms* RoguePlatforms__unix(void);
RoguePlatforms* RoguePlatforms__windows(void);
void RoguePlatforms__init__RogueString_RogueLogical_RogueLogical_RogueLogical( RoguePlatforms* THIS, RogueString* _auto_store_combined_0, RogueLogical windows_1, RogueLogical macos_2, RogueLogical linux_3 );
RoguePlatforms* RoguePlatforms__operatorPLUS__RoguePlatforms( RoguePlatforms* THIS, RoguePlatforms* other_0 );
RoguePlatforms* RoguePlatforms__operatorOR__RoguePlatforms( RoguePlatforms* THIS, RoguePlatforms* other_0 );
RogueString* RoguePlatforms__toxRogueStringx( RoguePlatforms* THIS );
void RoguePlatforms__init_object( RoguePlatforms* THIS );
RogueString* RoguePlatforms__type_name( RoguePlatforms* THIS );
void RogueFDWriter__init__RogueInt32_RogueLogical( RogueFDWriter* THIS, RogueInt32 _auto_store_fd_0, RogueLogical _auto_store_auto_close_1 );
void RogueFDWriter__on_cleanup( RogueFDWriter* THIS );
void RogueFDWriter__close( RogueFDWriter* THIS );
void RogueFDWriter__flush( RogueFDWriter* THIS );
void RogueFDWriter__write__RogueByte( RogueFDWriter* THIS, RogueByte ch_0 );
void RogueFDWriter__write__RogueByteList( RogueFDWriter* THIS, RogueByteList* bytes_0 );
void RogueFDWriter__init_object( RogueFDWriter* THIS );
RogueString* RogueFDWriter__type_name( RogueFDWriter* THIS );
RogueInt32 RogueFDWriter__position( RogueFDWriter* THIS );
extern RogueFunction_490* RogueFunction_490_singleton;

RogueLogical RogueFunction_490__call__RogueString( RogueFunction_490* THIS, RogueString* name_0 );
void RogueFunction_490__init_object( RogueFunction_490* THIS );
RogueString* RogueFunction_490__type_name( RogueFunction_490* THIS );
extern RogueFunction_491* RogueFunction_491_singleton;

RogueLogical RogueFunction_491__call__RogueString( RogueFunction_491* THIS, RogueString* path_0 );
void RogueFunction_491__init__RogueString( RogueFunction_491* THIS, RogueString* _auto_store_binpath_0 );
void RogueFunction_491__init_object( RogueFunction_491* THIS );
RogueString* RogueFunction_491__type_name( RogueFunction_491* THIS );
void RogueLineReader__init__RogueReaderxRogueCharacterx( RogueLineReader* THIS, RogueObject* _auto_store_source_0 );
void RogueLineReader__init__RogueString( RogueLineReader* THIS, RogueString* string_0 );
RogueLogical RogueLineReader__has_another( RogueLineReader* THIS );
RogueString* RogueLineReader__peek( RogueLineReader* THIS );
RogueLogical RogueLineReader__prepare_next( RogueLineReader* THIS );
RogueString* RogueLineReader__read( RogueLineReader* THIS );
void RogueLineReader__init_object( RogueLineReader* THIS );
RogueString* RogueLineReader__toxRogueStringx( RogueLineReader* THIS );
RogueString* RogueLineReader__type_name( RogueLineReader* THIS );
RogueInt32 RogueLineReader__position( RogueLineReader* THIS );
void RogueFileListing___listing__RogueFile_RogueOPARENFunctionOPARENRogueStringCPARENCPAREN(RogueFile folder_0, RogueOPARENFunctionOPARENRogueStringCPARENCPAREN* collector_1);
void RogueFileListing__init__RogueFile_RogueOptionalFilePattern_RogueFileListingOption( RogueFileListing* THIS, RogueFile folder_or_filepath_0, RogueOptionalFilePattern _pattern_1, RogueFileListingOption _auto_store_options_2 );
void RogueFileListing__collect__RogueString( RogueFileListing* THIS, RogueString* filename_0 );
RogueString* RogueFileListing__fix__RogueString( RogueFileListing* THIS, RogueString* pattern_0 );
void RogueFileListing__init_object( RogueFileListing* THIS );
RogueString* RogueFileListing__type_name( RogueFileListing* THIS );
extern RogueFunction_495* RogueFunction_495_singleton;

void RogueFunction_495__call__RogueString( RogueFunction_495* THIS, RogueString* filename_0 );
void RogueFunction_495__init__RogueFileListing( RogueFunction_495* THIS, RogueFileListing* _auto_store__THIS_0 );
void RogueFunction_495__init_object( RogueFunction_495* THIS );
RogueString* RogueFunction_495__type_name( RogueFunction_495* THIS );
extern RogueFunction_496* RogueFunction_496_singleton;

RogueLogical RogueFunction_496__call__RogueString( RogueFunction_496* THIS, RogueString* arg_0 );
void RogueFunction_496__init_object( RogueFunction_496* THIS );
RogueString* RogueFunction_496__type_name( RogueFunction_496* THIS );
void RogueSpanList__init( RogueSpanList* THIS );
void RogueSpanList__init_object( RogueSpanList* THIS );
void RogueSpanList__on_cleanup( RogueSpanList* THIS );
void RogueSpanList__add__RogueSpan( RogueSpanList* THIS, RogueSpan value_0 );
void RogueSpanList__clear( RogueSpanList* THIS );
RogueString* RogueSpanList__description( RogueSpanList* THIS );
void RogueSpanList__discard_from__RogueInt32( RogueSpanList* THIS, RogueInt32 index_0 );
RogueSpan RogueSpanList__get__RogueInt32( RogueSpanList* THIS, RogueInt32 index_0 );
void RogueSpanList__on_return_to_pool( RogueSpanList* THIS );
RogueSpan RogueSpanList__remove_last( RogueSpanList* THIS );
void RogueSpanList__reserve__RogueInt32( RogueSpanList* THIS, RogueInt32 additional_capacity_0 );
void RogueSpanList__set__RogueInt32_RogueSpan( RogueSpanList* THIS, RogueInt32 index_0, RogueSpan value_1 );
RogueString* RogueSpanList__toxRogueStringx( RogueSpanList* THIS );
void RogueSpanList__zero__RogueInt32_RogueOptionalInt32( RogueSpanList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueSpanList__type_name( RogueSpanList* THIS );
extern RogueSpanList* RogueTimsortxRogueStringx__g_runs;
extern RogueInt32 RogueTimsortxRogueStringx__g_gallop_threshold;
void RogueTimsortxRogueStringx__sort__RogueStringList_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_1);
RogueInt32 RogueTimsortxRogueStringx___count_and_order_run__RogueStringList_RogueInt32_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueInt32 low_1, RogueInt32 limit_2, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_3);
void RogueTimsortxRogueStringx___binary_sort__RogueStringList_RogueInt32_RogueInt32_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueInt32 low_1, RogueInt32 limit_2, RogueInt32 start_3, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_4);
RogueInt32 RogueTimsortxRogueStringx___gallop_left__RogueString_RogueStringList_RogueInt32_RogueInt32_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueString* key_0, RogueStringList* list_1, RogueInt32 base_2, RogueInt32 len_3, RogueInt32 hint_4, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_5);
RogueInt32 RogueTimsortxRogueStringx___gallop_right__RogueString_RogueStringList_RogueInt32_RogueInt32_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueString* key_0, RogueStringList* list_1, RogueInt32 base_2, RogueInt32 len_3, RogueInt32 hint_4, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_5);
void RogueTimsortxRogueStringx___merge_at__RogueStringList_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueInt32 i_1, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_2);
void RogueTimsortxRogueStringx___merge_collapse__RogueStringList_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_1);
void RogueTimsortxRogueStringx___merge_force_collapse__RogueStringList_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_1);
void RogueTimsortxRogueStringx___merge_high__RogueStringList_RogueInt32_RogueInt32_RogueInt32_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueInt32 base1_1, RogueInt32 len1_2, RogueInt32 base2_3, RogueInt32 len2_4, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_5);
void RogueTimsortxRogueStringx___merge_low__RogueStringList_RogueInt32_RogueInt32_RogueInt32_RogueInt32_RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN(RogueStringList* list_0, RogueInt32 base1_1, RogueInt32 len1_2, RogueInt32 base2_3, RogueInt32 len2_4, RogueOPARENFunctionOPARENRogueString_RogueStringCPARENRETURNSRogueLogicalCPAREN* compare_fn_5);
RogueInt32 RogueTimsortxRogueStringx___min_run_length__RogueInt32(RogueInt32 n_0);
void RogueTimsortxRogueStringx___reverse_range__RogueStringList_RogueInt32_RogueInt32(RogueStringList* list_0, RogueInt32 low_1, RogueInt32 limit_2);
void RogueTimsortxRogueStringx__init_class(void);
void RogueTimsortxRogueStringx__init_object( RogueTimsortxRogueStringx* THIS );
RogueString* RogueTimsortxRogueStringx__type_name( RogueTimsortxRogueStringx* THIS );
extern RogueWorkListxRogueStringx* RogueWorkListxRogueStringx_singleton;

void RogueWorkListxRogueStringx__init_object( RogueWorkListxRogueStringx* THIS );
RogueString* RogueWorkListxRogueStringx__type_name( RogueWorkListxRogueStringx* THIS );
extern RogueWorkListxRogueString_Defaultx* RogueWorkListxRogueString_Defaultx_singleton;

RogueStringList* RogueWorkListxRogueString_Defaultx__on_use( RogueWorkListxRogueString_Defaultx* THIS );
void RogueWorkListxRogueString_Defaultx__on_end_use__RogueStringList( RogueWorkListxRogueString_Defaultx* THIS, RogueStringList* list_0 );
void RogueWorkListxRogueString_Defaultx__init_object( RogueWorkListxRogueString_Defaultx* THIS );
RogueString* RogueWorkListxRogueString_Defaultx__type_name( RogueWorkListxRogueString_Defaultx* THIS );
extern RogueFunction_513* RogueFunction_513_singleton;

RogueLogical RogueFunction_513__call__RogueString_RogueString( RogueFunction_513* THIS, RogueString* arg1_0, RogueString* arg2_1 );
void RogueFunction_513__init_object( RogueFunction_513* THIS );
RogueString* RogueFunction_513__type_name( RogueFunction_513* THIS );
extern RogueFunction_514* RogueFunction_514_singleton;

RogueLogical RogueFunction_514__call__RogueString_RogueString( RogueFunction_514* THIS, RogueString* a_0, RogueString* b_1 );
void RogueFunction_514__init_object( RogueFunction_514* THIS );
RogueString* RogueFunction_514__type_name( RogueFunction_514* THIS );
void RoguePackageError__init__RogueString_RogueString( RoguePackageError* THIS, RogueString* _auto_store_package_name_0, RogueString* _auto_store_message_1 );
void RoguePackageError__init_object( RoguePackageError* THIS );
RogueString* RoguePackageError__type_name( RoguePackageError* THIS );
RogueValue RogueJSON__load__RogueFile_RoguePrintWriter(RogueFile file_0, RogueObject* error_log_1);
RogueValue RogueJSON__parse__RogueString_RoguePrintWriter(RogueString* json_0, RogueObject* error_log_1);
RogueLogical RogueJSON__save__RogueValue_RogueFile_RogueLogical_RogueLogical(RogueValue value_0, RogueFile file_1, RogueLogical formatted_2, RogueLogical omit_commas_3);
void RogueJSON__init_object( RogueJSON* THIS );
RogueString* RogueJSON__type_name( RogueJSON* THIS );
void RogueJSONParseError__init__RogueString( RogueJSONParseError* THIS, RogueString* _auto_store_message_0 );
void RogueJSONParseError__init_object( RogueJSONParseError* THIS );
RogueString* RogueJSONParseError__type_name( RogueJSONParseError* THIS );
void RogueScanner__init__RogueString_RogueInt32_RogueLogical( RogueScanner* THIS, RogueString* _auto_store_source_0, RogueInt32 _auto_store_spaces_per_tab_1, RogueLogical preserve_crlf_2 );
RogueLogical RogueScanner__consume__RogueCharacter( RogueScanner* THIS, RogueCharacter ch_0 );
RogueLogical RogueScanner__consume_eols( RogueScanner* THIS );
RogueLogical RogueScanner__consume_whitespace( RogueScanner* THIS );
RogueLogical RogueScanner__has_another( RogueScanner* THIS );
RogueCharacter RogueScanner__peek( RogueScanner* THIS );
RogueCharacter RogueScanner__read( RogueScanner* THIS );
void RogueScanner__init_object( RogueScanner* THIS );
RogueString* RogueScanner__toxRogueStringx( RogueScanner* THIS );
RogueString* RogueScanner__type_name( RogueScanner* THIS );
RogueInt32 RogueScanner__position( RogueScanner* THIS );
void RogueJSONParser__init__RogueString( RogueJSONParser* THIS, RogueString* json_0 );
RogueValue RogueJSONParser__parse_value( RogueJSONParser* THIS );
RogueValue RogueJSONParser__parse_table__RogueCharacter_RogueCharacter( RogueJSONParser* THIS, RogueCharacter open_ch_0, RogueCharacter close_ch_1 );
RogueValue RogueJSONParser__parse_list__RogueCharacter_RogueCharacter( RogueJSONParser* THIS, RogueCharacter open_ch_0, RogueCharacter close_ch_1 );
RogueString* RogueJSONParser__parse_string( RogueJSONParser* THIS );
RogueCharacter RogueJSONParser__parse_hex_quad( RogueJSONParser* THIS );
RogueString* RogueJSONParser__parse_identifier( RogueJSONParser* THIS );
RogueLogical RogueJSONParser__next_is_identifier( RogueJSONParser* THIS );
RogueValue RogueJSONParser__parse_number( RogueJSONParser* THIS );
void RogueJSONParser__consume_whitespace_and_eols( RogueJSONParser* THIS );
void RogueJSONParser__init_object( RogueJSONParser* THIS );
RogueString* RogueJSONParser__type_name( RogueJSONParser* THIS );
extern RogueFunction_529* RogueFunction_529_singleton;

RogueLogical RogueFunction_529__call__RogueString_RogueString( RogueFunction_529* THIS, RogueString* a_0, RogueString* b_1 );
void RogueFunction_529__init_object( RogueFunction_529* THIS );
RogueString* RogueFunction_529__type_name( RogueFunction_529* THIS );
extern RogueFunction_530* RogueFunction_530_singleton;

RogueLogical RogueFunction_530__call__RogueString_RogueString( RogueFunction_530* THIS, RogueString* a_0, RogueString* b_1 );
void RogueFunction_530__init_object( RogueFunction_530* THIS );
RogueString* RogueFunction_530__type_name( RogueFunction_530* THIS );
extern RogueFunction_531* RogueFunction_531_singleton;

RogueLogical RogueFunction_531__call__RogueString_RogueString( RogueFunction_531* THIS, RogueString* a_0, RogueString* b_1 );
void RogueFunction_531__init_object( RogueFunction_531* THIS );
RogueString* RogueFunction_531__type_name( RogueFunction_531* THIS );
void RogueZip__init__RogueFile_RogueInt32( RogueZip* THIS, RogueFile zipfile_0, RogueInt32 compression_1 );
void RogueZip__close( RogueZip* THIS );
RogueInt32 RogueZip__count( RogueZip* THIS );
void RogueZip__extract__RogueFile_RogueLogical( RogueZip* THIS, RogueFile folder_0, RogueLogical verbose_1 );
RogueZipEntry RogueZip__get__RogueInt32( RogueZip* THIS, RogueInt32 index_0 );
void RogueZip__on_cleanup( RogueZip* THIS );
void RogueZip__open__RogueFile_RogueInt32_RogueInt32( RogueZip* THIS, RogueFile zipfile_0, RogueInt32 _auto_store_compression_1, RogueInt32 new_mode_2 );
void RogueZip__set_compression__RogueInt32( RogueZip* THIS, RogueInt32 new_compression_0 );
void RogueZip__set_mode__RogueInt32( RogueZip* THIS, RogueInt32 new_mode_0 );
RogueString* RogueZip__toxRogueStringx( RogueZip* THIS );
void RogueZip__init_object( RogueZip* THIS );
RogueString* RogueZip__type_name( RogueZip* THIS );
void RogueFiles__init__RogueString_RogueLogical_RogueLogical( RogueFiles* THIS, RogueString* base_folder_0, RogueLogical allow_pattern_1, RogueLogical include_folders_2 );
void RogueFiles__init__RogueString_RogueFilePattern_RogueLogical_RogueLogical( RogueFiles* THIS, RogueString* base_folder_0, RogueFilePattern pattern_1, RogueLogical ignore_hidden_2, RogueLogical include_folders_3 );
void RogueFiles__init__RogueString_RogueString_RogueLogical_RogueLogical( RogueFiles* THIS, RogueString* base_folder_0, RogueString* pattern_1, RogueLogical ignore_hidden_2, RogueLogical include_folders_3 );
void RogueFiles__add__RogueFilePattern_RogueLogical_RogueLogical( RogueFiles* THIS, RogueFilePattern pattern_0, RogueLogical ignore_hidden_1, RogueLogical include_folders_2 );
void RogueFiles__add__RogueString_RogueLogical_RogueLogical( RogueFiles* THIS, RogueString* pattern_0, RogueLogical ignore_hidden_1, RogueLogical include_folders_2 );
RogueLogical RogueFiles__contains__RogueString( RogueFiles* THIS, RogueString* filepath_0 );
RogueString* RogueFiles__relative_filepath__RogueString( RogueFiles* THIS, RogueString* filepath_0 );
void RogueFiles__remove__RogueString( RogueFiles* THIS, RogueString* pattern_0 );
RogueInt32 RogueFiles__sync_to__RogueFile_RogueLogical_RogueLogical_RogueLogical_RogueLogical( RogueFiles* THIS, RogueFile dest_folder_0, RogueLogical verbose_1, RogueLogical keep_unused_2, RogueLogical dry_run_3, RogueLogical missing_only_4 );
RogueString* RogueFiles__toxRogueStringx( RogueFiles* THIS );
void RogueFiles__init_object( RogueFiles* THIS );
RogueString* RogueFiles__type_name( RogueFiles* THIS );
extern RogueFunction_551* RogueFunction_551_singleton;

RogueLogical RogueFunction_551__call__RogueString( RogueFunction_551* THIS, RogueString* f_0 );
void RogueFunction_551__init_object( RogueFunction_551* THIS );
RogueString* RogueFunction_551__type_name( RogueFunction_551* THIS );
extern RogueFunction_552* RogueFunction_552_singleton;

RogueLogical RogueFunction_552__call__RogueString( RogueFunction_552* THIS, RogueString* f_0 );
void RogueFunction_552__init_object( RogueFunction_552* THIS );
RogueString* RogueFunction_552__type_name( RogueFunction_552* THIS );
extern RogueFunction_553* RogueFunction_553_singleton;

RogueLogical RogueFunction_553__call__RogueString( RogueFunction_553* THIS, RogueString* f_0 );
void RogueFunction_553__init_object( RogueFunction_553* THIS );
RogueString* RogueFunction_553__type_name( RogueFunction_553* THIS );
extern RogueFunction_554* RogueFunction_554_singleton;

RogueLogical RogueFunction_554__call__RogueString( RogueFunction_554* THIS, RogueString* f_0 );
void RogueFunction_554__init__RogueString( RogueFunction_554* THIS, RogueString* _auto_store_app_name_0 );
void RogueFunction_554__init_object( RogueFunction_554* THIS );
RogueString* RogueFunction_554__type_name( RogueFunction_554* THIS );
extern RogueFunction_555* RogueFunction_555_singleton;

RogueLogical RogueFunction_555__call__RogueString( RogueFunction_555* THIS, RogueString* f_0 );
void RogueFunction_555__init__RogueString( RogueFunction_555* THIS, RogueString* _auto_store_app_name_0 );
void RogueFunction_555__init_object( RogueFunction_555* THIS );
RogueString* RogueFunction_555__type_name( RogueFunction_555* THIS );
extern RogueWorkListxRogueBytex* RogueWorkListxRogueBytex_singleton;

void RogueWorkListxRogueBytex__init_object( RogueWorkListxRogueBytex* THIS );
RogueString* RogueWorkListxRogueBytex__type_name( RogueWorkListxRogueBytex* THIS );
extern RogueWorkListxRogueByte_Defaultx* RogueWorkListxRogueByte_Defaultx_singleton;

RogueByteList* RogueWorkListxRogueByte_Defaultx__on_use( RogueWorkListxRogueByte_Defaultx* THIS );
void RogueWorkListxRogueByte_Defaultx__on_end_use__RogueByteList( RogueWorkListxRogueByte_Defaultx* THIS, RogueByteList* list_0 );
void RogueWorkListxRogueByte_Defaultx__init_object( RogueWorkListxRogueByte_Defaultx* THIS );
RogueString* RogueWorkListxRogueByte_Defaultx__type_name( RogueWorkListxRogueByte_Defaultx* THIS );
extern RogueFunction_558* RogueFunction_558_singleton;

RogueLogical RogueFunction_558__call__RogueValue( RogueFunction_558* THIS, RogueValue arg_0 );
void RogueFunction_558__init_object( RogueFunction_558* THIS );
RogueString* RogueFunction_558__type_name( RogueFunction_558* THIS );
extern RogueFunction_559* RogueFunction_559_singleton;

RogueLogical RogueFunction_559__call__RogueValue( RogueFunction_559* THIS, RogueValue arg_0 );
void RogueFunction_559__init_object( RogueFunction_559* THIS );
RogueString* RogueFunction_559__type_name( RogueFunction_559* THIS );
extern RogueFunction_560* RogueFunction_560_singleton;

RogueLogical RogueFunction_560__call__RogueValue( RogueFunction_560* THIS, RogueValue arg_0 );
void RogueFunction_560__init_object( RogueFunction_560* THIS );
RogueString* RogueFunction_560__type_name( RogueFunction_560* THIS );
extern RogueFunction_574* RogueFunction_574_singleton;

RogueLogical RogueFunction_574__call__RogueString( RogueFunction_574* THIS, RogueString* arg_0 );
void RogueFunction_574__init_object( RogueFunction_574* THIS );
RogueString* RogueFunction_574__type_name( RogueFunction_574* THIS );
extern RogueFunction_575* RogueFunction_575_singleton;

RogueLogical RogueFunction_575__call__RogueString_RogueString( RogueFunction_575* THIS, RogueString* a_0, RogueString* b_1 );
void RogueFunction_575__init_object( RogueFunction_575* THIS );
RogueString* RogueFunction_575__type_name( RogueFunction_575* THIS );
void RogueSetxRogueStringx__add__RogueString( RogueSetxRogueStringx* THIS, RogueString* value_0 );
RogueString* RogueSetxRogueStringx__print_to__RogueString( RogueSetxRogueStringx* THIS, RogueString* buffer_0 );
void RogueSetxRogueStringx__init_object( RogueSetxRogueStringx* THIS );
RogueString* RogueSetxRogueStringx__type_name( RogueSetxRogueStringx* THIS );
void RogueConsoleErrorPrinter__flush__RogueString( RogueConsoleErrorPrinter* THIS, RogueString* buffer_0 );
void RogueConsoleErrorPrinter__write__RogueString( RogueConsoleErrorPrinter* THIS, RogueString* value_0 );
void RogueConsoleErrorPrinter__init_object( RogueConsoleErrorPrinter* THIS );
RogueString* RogueConsoleErrorPrinter__type_name( RogueConsoleErrorPrinter* THIS );
void RogueConsoleErrorPrinter__close( RogueConsoleErrorPrinter* THIS );
void RogueConsoleErrorPrinter__flush( RogueConsoleErrorPrinter* THIS );
void RogueConsoleErrorPrinter__print__RogueCharacter( RogueConsoleErrorPrinter* THIS, RogueCharacter value_0 );
void RogueConsoleErrorPrinter__print__RogueObject( RogueConsoleErrorPrinter* THIS, RogueObject* value_0 );
void RogueConsoleErrorPrinter__print__RogueString( RogueConsoleErrorPrinter* THIS, RogueString* value_0 );
void RogueConsoleErrorPrinter__println( RogueConsoleErrorPrinter* THIS );
void RogueConsoleErrorPrinter__println__RogueObject( RogueConsoleErrorPrinter* THIS, RogueObject* value_0 );
void RogueConsoleErrorPrinter__println__RogueString( RogueConsoleErrorPrinter* THIS, RogueString* value_0 );
extern RogueFunction_578* RogueFunction_578_singleton;

void RogueFunction_578__call( RogueFunction_578* THIS );
void RogueFunction_578__init_object( RogueFunction_578* THIS );
RogueString* RogueFunction_578__type_name( RogueFunction_578* THIS );
void RogueStandardConsoleMode__init( RogueStandardConsoleMode* THIS );
RogueLogical RogueStandardConsoleMode__has_another( RogueStandardConsoleMode* THIS );
RogueCharacter RogueStandardConsoleMode__read( RogueStandardConsoleMode* THIS );
void RogueStandardConsoleMode__init_object( RogueStandardConsoleMode* THIS );
RogueString* RogueStandardConsoleMode__type_name( RogueStandardConsoleMode* THIS );
extern RogueFunction_580* RogueFunction_580_singleton;

void RogueFunction_580__call( RogueFunction_580* THIS );
void RogueFunction_580__init_object( RogueFunction_580* THIS );
RogueString* RogueFunction_580__type_name( RogueFunction_580* THIS );
void RogueConsoleEventList__init( RogueConsoleEventList* THIS );
void RogueConsoleEventList__init_object( RogueConsoleEventList* THIS );
void RogueConsoleEventList__on_cleanup( RogueConsoleEventList* THIS );
void RogueConsoleEventList__add__RogueConsoleEvent( RogueConsoleEventList* THIS, RogueConsoleEvent value_0 );
void RogueConsoleEventList__clear( RogueConsoleEventList* THIS );
void RogueConsoleEventList__copy__RogueInt32_RogueInt32_RogueConsoleEventList_RogueInt32( RogueConsoleEventList* THIS, RogueInt32 src_i1_0, RogueInt32 src_count_1, RogueConsoleEventList* dest_2, RogueInt32 dest_i1_3 );
RogueString* RogueConsoleEventList__description( RogueConsoleEventList* THIS );
void RogueConsoleEventList__discard_from__RogueInt32( RogueConsoleEventList* THIS, RogueInt32 index_0 );
void RogueConsoleEventList__ensure_capacity__RogueInt32( RogueConsoleEventList* THIS, RogueInt32 desired_capacity_0 );
void RogueConsoleEventList__expand_to_count__RogueInt32( RogueConsoleEventList* THIS, RogueInt32 minimum_count_0 );
RogueConsoleEvent RogueConsoleEventList__get__RogueInt32( RogueConsoleEventList* THIS, RogueInt32 index_0 );
void RogueConsoleEventList__on_return_to_pool( RogueConsoleEventList* THIS );
RogueConsoleEvent RogueConsoleEventList__remove_first( RogueConsoleEventList* THIS );
void RogueConsoleEventList__reserve__RogueInt32( RogueConsoleEventList* THIS, RogueInt32 additional_capacity_0 );
void RogueConsoleEventList__shift__RogueInt32( RogueConsoleEventList* THIS, RogueInt32 delta_0 );
RogueString* RogueConsoleEventList__toxRogueStringx( RogueConsoleEventList* THIS );
void RogueConsoleEventList__zero__RogueInt32_RogueOptionalInt32( RogueConsoleEventList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueConsoleEventList__type_name( RogueConsoleEventList* THIS );
void RogueImmediateConsoleMode__init( RogueImmediateConsoleMode* THIS );
RogueLogical RogueImmediateConsoleMode__has_another( RogueImmediateConsoleMode* THIS );
RogueCharacter RogueImmediateConsoleMode__read( RogueImmediateConsoleMode* THIS );
void RogueImmediateConsoleMode___fill_event_queue( RogueImmediateConsoleMode* THIS );
void RogueImmediateConsoleMode___fill_event_queue_windows( RogueImmediateConsoleMode* THIS );
void RogueImmediateConsoleMode___fill_event_queue_windows_process_next( RogueImmediateConsoleMode* THIS );
void RogueImmediateConsoleMode___fill_event_queue_unix( RogueImmediateConsoleMode* THIS );
void RogueImmediateConsoleMode___fill_event_queue_unix_process_next( RogueImmediateConsoleMode* THIS );
void RogueImmediateConsoleMode__init_object( RogueImmediateConsoleMode* THIS );
RogueString* RogueImmediateConsoleMode__type_name( RogueImmediateConsoleMode* THIS );
void RogueUnixConsoleMouseEventTypeList__init__RogueInt32( RogueUnixConsoleMouseEventTypeList* THIS, RogueInt32 capacity_0 );
void RogueUnixConsoleMouseEventTypeList__init_object( RogueUnixConsoleMouseEventTypeList* THIS );
void RogueUnixConsoleMouseEventTypeList__on_cleanup( RogueUnixConsoleMouseEventTypeList* THIS );
void RogueUnixConsoleMouseEventTypeList__add__RogueUnixConsoleMouseEventType( RogueUnixConsoleMouseEventTypeList* THIS, RogueUnixConsoleMouseEventType value_0 );
void RogueUnixConsoleMouseEventTypeList__clear( RogueUnixConsoleMouseEventTypeList* THIS );
RogueString* RogueUnixConsoleMouseEventTypeList__description( RogueUnixConsoleMouseEventTypeList* THIS );
void RogueUnixConsoleMouseEventTypeList__discard_from__RogueInt32( RogueUnixConsoleMouseEventTypeList* THIS, RogueInt32 index_0 );
RogueUnixConsoleMouseEventType RogueUnixConsoleMouseEventTypeList__get__RogueInt32( RogueUnixConsoleMouseEventTypeList* THIS, RogueInt32 index_0 );
void RogueUnixConsoleMouseEventTypeList__on_return_to_pool( RogueUnixConsoleMouseEventTypeList* THIS );
void RogueUnixConsoleMouseEventTypeList__reserve__RogueInt32( RogueUnixConsoleMouseEventTypeList* THIS, RogueInt32 additional_capacity_0 );
RogueString* RogueUnixConsoleMouseEventTypeList__toxRogueStringx( RogueUnixConsoleMouseEventTypeList* THIS );
void RogueUnixConsoleMouseEventTypeList__zero__RogueInt32_RogueOptionalInt32( RogueUnixConsoleMouseEventTypeList* THIS, RogueInt32 i1_0, RogueOptionalInt32 count_1 );
RogueString* RogueUnixConsoleMouseEventTypeList__type_name( RogueUnixConsoleMouseEventTypeList* THIS );
void RogueUnrecognizedOptionError__init__RogueString( RogueUnrecognizedOptionError* THIS, RogueString* _auto_store_name_0 );
void RogueUnrecognizedOptionError__init_object( RogueUnrecognizedOptionError* THIS );
RogueString* RogueUnrecognizedOptionError__type_name( RogueUnrecognizedOptionError* THIS );
void RogueValueExpectedError__init__RogueString( RogueValueExpectedError* THIS, RogueString* _auto_store_name_0 );
void RogueValueExpectedError__init_object( RogueValueExpectedError* THIS );
RogueString* RogueValueExpectedError__type_name( RogueValueExpectedError* THIS );
void RogueUnexpectedValueError__init__RogueString( RogueUnexpectedValueError* THIS, RogueString* _auto_store_name_0 );
void RogueUnexpectedValueError__init_object( RogueUnexpectedValueError* THIS );
RogueString* RogueUnexpectedValueError__type_name( RogueUnexpectedValueError* THIS );
void* Rogue_dispatch_type_name___RogueObject( void* THIS );
void Rogue_dispatch_close_( void* THIS );
void Rogue_dispatch_flush_( void* THIS );
void Rogue_dispatch_print__RogueCharacter( void* THIS, RogueCharacter p0 );
void Rogue_dispatch_print__RogueString( void* THIS, RogueString* p0 );
void Rogue_dispatch_println__RogueObject( void* THIS, RogueObject* p0 );
void Rogue_dispatch_println__RogueString( void* THIS, RogueString* p0 );
void Rogue_dispatch_on_return_to_pool_( void* THIS );
void* Rogue_dispatch_finish___RogueObject( void* THIS );
RogueLogical Rogue_dispatch_has_another___RogueLogical( void* THIS );
void* Rogue_dispatch_read___RogueObject( void* THIS );
RogueInt32 Rogue_dispatch_position___RogueInt32( void* THIS );
RogueCharacter Rogue_dispatch_read___RogueCharacter( void* THIS );
RogueByte Rogue_dispatch_read___RogueByte( void* THIS );
void Rogue_dispatch_seek__RogueInt32( void* THIS, RogueInt32 p0 );
void Rogue_dispatch_write__RogueByteList( void* THIS, RogueByteList* p0 );
void* Rogue_dispatch_print_to__RogueString__RogueObject( void* THIS, RogueString* p0 );

extern RogueInt32 Rogue_type_count;
extern RogueRuntimeType* Rogue_types[191];
extern RogueInt32 Rogue_base_types[235];
extern RogueString* Rogue_string_table[493];
extern RogueString* str_true;
extern RogueString* str_false;
extern RogueString* str___;
extern RogueString* str__0;
extern RogueString* str__b;
extern RogueString* str__e;
extern RogueString* str__f;
extern RogueString* str__n;
extern RogueString* str__r;
extern RogueString* str__t;
extern RogueString* str__v;
extern RogueString* str__;
extern RogueString* str____1;
extern RogueString* str__x;
extern RogueString* str___1;
extern RogueString* str___2;
extern RogueString* str___3;
extern RogueString* str___4;
extern RogueString* str___5;
extern RogueString* str___6;
extern RogueString* str___7;
extern RogueString* str___8;
extern RogueString* str___9;
extern RogueString* str___10;
extern RogueString* str___11;
extern RogueString* str___12;
extern RogueString* str___13;
extern RogueString* str___14;
extern RogueString* str___15;
extern RogueString* str___16;
extern RogueString* str___17;
extern RogueString* str___18;
extern RogueString* str___19;
extern RogueString* str___20;
extern RogueString* str___21;
extern RogueString* str___22;
extern RogueString* str___23;
extern RogueString* str___24;
extern RogueString* str_0;
extern RogueString* str_1;
extern RogueString* str_2;
extern RogueString* str_3;
extern RogueString* str_4;
extern RogueString* str_5;
extern RogueString* str_6;
extern RogueString* str_7;
extern RogueString* str_8;
extern RogueString* str_9;
extern RogueString* str___25;
extern RogueString* str___26;
extern RogueString* str___27;
extern RogueString* str___28;
extern RogueString* str___29;
extern RogueString* str___30;
extern RogueString* str___31;
extern RogueString* str_A;
extern RogueString* str_B;
extern RogueString* str_C;
extern RogueString* str_D;
extern RogueString* str_E;
extern RogueString* str_F;
extern RogueString* str_G;
extern RogueString* str_H;
extern RogueString* str_I;
extern RogueString* str_J;
extern RogueString* str_K;
extern RogueString* str_L;
extern RogueString* str_M;
extern RogueString* str_N;
extern RogueString* str_O;
extern RogueString* str_P;
extern RogueString* str_Q;
extern RogueString* str_R;
extern RogueString* str_S;
extern RogueString* str_T;
extern RogueString* str_U;
extern RogueString* str_V;
extern RogueString* str_W;
extern RogueString* str_X;
extern RogueString* str_Y;
extern RogueString* str_Z;
extern RogueString* str___32;
extern RogueString* str___33;
extern RogueString* str___34;
extern RogueString* str___35;
extern RogueString* str___36;
extern RogueString* str_a;
extern RogueString* str_b;
extern RogueString* str_c;
extern RogueString* str_d;
extern RogueString* str_e;
extern RogueString* str_f;
extern RogueString* str_g;
extern RogueString* str_h;
extern RogueString* str_i;
extern RogueString* str_j;
extern RogueString* str_k;
extern RogueString* str_l;
extern RogueString* str_m;
extern RogueString* str_n;
extern RogueString* str_o;
extern RogueString* str_p;
extern RogueString* str_q;
extern RogueString* str_r;
extern RogueString* str_s;
extern RogueString* str_t;
extern RogueString* str_u;
extern RogueString* str_v;
extern RogueString* str_w;
extern RogueString* str_x;
extern RogueString* str_y;
extern RogueString* str_z;
extern RogueString* str___37;
extern RogueString* str___38;
extern RogueString* str___39;
extern RogueString* str___40;
extern RogueString* str__1;
extern RogueString* str____SEGFAULT___;
extern RogueString* str___type___;
extern RogueString* str____2;
extern RogueString* str_null;
extern RogueString* str_;
extern RogueString* str__9223372036854775808;
extern RogueString* str_0_0;
extern RogueString* str__infinity;
extern RogueString* str_infinity;
extern RogueString* str_NaN;
extern RogueString* str_cd_;
extern RogueString* str_____;
extern RogueString* str____3;
extern RogueString* str_Error_executing__;
extern RogueString* str____retrying_with__su;
extern RogueString* str_sudo_;
extern RogueString* str_Error_executing___1;
extern RogueString* str_Unable_to_obtain_abs;
extern RogueString* str_____may_not_have_rea;
extern RogueString* str_Cannot_copy_the_cont;
extern RogueString* str__to_file_;
extern RogueString* str____4;
extern RogueString* str______1;
extern RogueString* str________________;
extern RogueString* str_________;
extern RogueString* str____5;
extern RogueString* str____6;
extern RogueString* str_HOMEDRIVE;
extern RogueString* str_HOMEPATH;
extern RogueString* str_HOME;
extern RogueString* str____7;
extern RogueString* str_where;
extern RogueString* str_which;
extern RogueString* str_Android;
extern RogueString* str_iOS;
extern RogueString* str_macOS;
extern RogueString* str_Web;
extern RogueString* str_Windows;
extern RogueString* str_Linux;
extern RogueString* str_sh;
extern RogueString* str__c;
extern RogueString* str_UTF8;
extern RogueString* str_ASCII;
extern RogueString* str_ASCII_256;
extern RogueString* str_AUTODETECT;
extern RogueString* str_StringEncoding_;
extern RogueString* str__x_1;
extern RogueString* str____8;
extern RogueString* str____9;
extern RogueString* str____10;
extern RogueString* str__u;
extern RogueString* str_StackTrace_init__;
extern RogueString* str_INTERNAL;
extern RogueString* str_Unknown_Procedure;
extern RogueString* str__Call_history_unavai;
extern RogueString* str____11;
extern RogueString* str____;
extern RogueString* str____12;
extern RogueString* str_Unable_to_open_;
extern RogueString* str__for_reading_;
extern RogueString* str__for_writing_;
extern RogueString* str_ABSOLUTE;
extern RogueString* str_FILES;
extern RogueString* str_FOLDERS;
extern RogueString* str_IGNORE_HIDDEN;
extern RogueString* str_OMIT_PATH;
extern RogueString* str_RECURSIVE;
extern RogueString* str_UNSORTED;
extern RogueString* str_FileListingOption_;
extern RogueString* str_cmd_exe;
extern RogueString* str__c_1;
extern RogueString* str____25;
extern RogueString* str_BACKSPACE;
extern RogueString* str_TAB;
extern RogueString* str_NEWLINE;
extern RogueString* str_ESCAPE;
extern RogueString* str_UP;
extern RogueString* str_DOWN;
extern RogueString* str_RIGHT;
extern RogueString* str_LEFT;
extern RogueString* str_DELETE;
extern RogueString* str_CHARACTER;
extern RogueString* str_POINTER_PRESS_LEFT;
extern RogueString* str_POINTER_PRESS_RIGHT;
extern RogueString* str_POINTER_RELEASE;
extern RogueString* str_POINTER_MOVE;
extern RogueString* str_SCROLL_UP;
extern RogueString* str_SCROLL_DOWN;
extern RogueString* str_ConsoleEventType_;
extern RogueString* str_Process_reader_could;
extern RogueString* str_Unsupported_operatio;
extern RogueString* str__seek__backwards_pos;
extern RogueString* str_action;
extern RogueString* str_help;
extern RogueString* str_alias;
extern RogueString* str_args;
extern RogueString* str_bin;
extern RogueString* str__bat;
extern RogueString* str____bin;
extern RogueString* str__binary_file_;
extern RogueString* str____bin_sh;
extern RogueString* str_chmod_0766_;
extern RogueString* str_bootstrap;
extern RogueString* str_create;
extern RogueString* str__rogue;
extern RogueString* str_Install_script__;
extern RogueString* str___already_exists_;
extern RogueString* str_class___CLASS_NAME_P;
extern RogueString* str___CLASS_NAME_;
extern RogueString* str___PACKAGE_NAME_;
extern RogueString* str_Creating_script_temp;
extern RogueString* str____13;
extern RogueString* str_install;
extern RogueString* str_Package_name_expecte;
extern RogueString* str__version_;
extern RogueString* str__is_already_installe;
extern RogueString* str_link;
extern RogueString* str_Missing_package_name;
extern RogueString* str_Linking_;
extern RogueString* str_____1;
extern RogueString* str_ln__s_;
extern RogueString* str_list;
extern RogueString* str_uninstall;
extern RogueString* str_Package_name_expecte_1;
extern RogueString* str__is_not_installed_;
extern RogueString* str_unlink;
extern RogueString* str_Missing_package_name_1;
extern RogueString* str_Unlinking_;
extern RogueString* str_No_such_launcher__;
extern RogueString* str_update;
extern RogueString* str_ERROR__;
extern RogueString* str_Missing_package_name_2;
extern RogueString* str_Unable_to_create_fol;
extern RogueString* str_sudo_mkdir__p_;
extern RogueString* str__admin;
extern RogueString* str_chown_;
extern RogueString* str_USER;
extern RogueString* str_Unable_to_chown_Morl;
extern RogueString* str_chmod_755_;
extern RogueString* str_packages______;
extern RogueString* str_Package_;
extern RogueString* str_command;
extern RogueString* str_packages_brombres_mo;
extern RogueString* str__INTERNAL_ERROR__Mor;
extern RogueString* str_packages_brombres_mo_1;
extern RogueString* str__Source_ScriptLaunch;
extern RogueString* str__Source_Package_rogu;
extern RogueString* str__exe;
extern RogueString* str_source_crc32_txt;
extern RogueString* str_2_13;
extern RogueString* str_roguec;
extern RogueString* str____debug___api___mai;
extern RogueString* str_cl__nologo_;
extern RogueString* str__c__Fo;
extern RogueString* str__obj__Fe;
extern RogueString* str____nul;
extern RogueString* str_cc__Wall__fno_strict;
extern RogueString* str__c__o_;
extern RogueString* str___lm;
extern RogueString* str_chmod_u_x_;
extern RogueString* str____14;
extern RogueString* str_____2;
extern RogueString* str_A_local__rogue_scrip;
extern RogueString* str_packages___;
extern RogueString* str_Ambiguous_app_name__;
extern RogueString* str___matches_mulitple_i;
extern RogueString* str__HOMEDRIVE__HOMEPATH;
extern RogueString* str__opt_morlock;
extern RogueString* str___dependency;
extern RogueString* str__d;
extern RogueString* str___home_;
extern RogueString* str__h;
extern RogueString* str___installer_;
extern RogueString* str__i;
extern RogueString* str_options;
extern RogueString* str_home;
extern RogueString* str_dependency;
extern RogueString* str_Morlock_v;
extern RogueString* str_2_2_3;
extern RogueString* str_March_19__2023;
extern RogueString* str__by_Brom_Bresenham;
extern RogueString* str_USAGE___morlock__com;
extern RogueString* str_curl__fsSL_;
extern RogueString* str_Download_failed__;
extern RogueString* str__packages_;
extern RogueString* str_url_txt;
extern RogueString* str_cache_json;
extern RogueString* str________;
extern RogueString* str_https___github_com_a;
extern RogueString* str_Invalid_URL__;
extern RogueString* str_____expected_e_g___;
extern RogueString* str____15;
extern RogueString* str_github;
extern RogueString* str_https___;
extern RogueString* str_github_com;
extern RogueString* str_curl__H__Accept__app;
extern RogueString* str_https___api_github_c;
extern RogueString* str__contents;
extern RogueString* str_Unable_to_list_defau;
extern RogueString* str_Repo_does_not_exist_;
extern RogueString* str_No_morlock_;
extern RogueString* str__rogue_install_scrip;
extern RogueString* str_url;
extern RogueString* str_ref_;
extern RogueString* str_main;
extern RogueString* str_https___raw_githubus;
extern RogueString* str_name;
extern RogueString* str_Morlock_does_not_kno;
extern RogueString* str__URLs_;
extern RogueString* str___o_;
extern RogueString* str_Can_t_find_Morlock_i;
extern RogueString* str_class_;
extern RogueString* str_Package___Package;
extern RogueString* str___PROPERTIES;
extern RogueString* str_____name____;
extern RogueString* str_Using_default_instal;
extern RogueString* str_Using_default_instal_1;
extern RogueString* str_pip3;
extern RogueString* str_pip;
extern RogueString* str__rogue_install_scrip_1;
extern RogueString* str____METHODS_____metho;
extern RogueString* str_endClass;
extern RogueString* str_morlock_home;
extern RogueString* str_version;
extern RogueString* str_script_filepath;
extern RogueString* str_host;
extern RogueString* str_repo;
extern RogueString* str__name_______;
extern RogueString* str_Failed_to_parse_pack;
extern RogueString* str__build_;
extern RogueString* str____16;
extern RogueString* str_cl;
extern RogueString* str_This_command_must_be;
extern RogueString* str_installer;
extern RogueString* str_Open_a_Developer_Com;
extern RogueString* str___curl__L_windows_mo;
extern RogueString* str_Open_a_Developer_Com_1;
extern RogueString* str_which_brew____dev_nu;
extern RogueString* str_Homebrew_must_be_ins;
extern RogueString* str__bin_bash__c____curl;
extern RogueString* str___Then_re_run_the_Mo;
extern RogueString* str_bash__c____curl__L_i;
extern RogueString* str_Creating_home_folder;
extern RogueString* str_build;
extern RogueString* str_packages;
extern RogueString* str_Add_the_following_fo;
extern RogueString* str_1__Start___Search_fo;
extern RogueString* str_2__Click__Environmen;
extern RogueString* str_3__Add_or_edit__Path;
extern RogueString* str_4__Add_the_Morlock__;
extern RogueString* str_____3;
extern RogueString* str_5__Open_a_new_Develo;
extern RogueString* str____curl__L_windows_m;
extern RogueString* str_5__Open_a_new_Develo_1;
extern RogueString* str_Add_the_following_fo_1;
extern RogueString* str_SHELL;
extern RogueString* str_____4;
extern RogueString* str_rc;
extern RogueString* str_You_can_execute_this;
extern RogueString* str_echo_export_PATH__;
extern RogueString* str_____5;
extern RogueString* str_PATH____;
extern RogueString* str_____source_;
extern RogueString* str_Then_re_run_the_Morl;
extern RogueString* str_Make_the_change_by_a;
extern RogueString* str_export_PATH__;
extern RogueString* str____PATH_;
extern RogueString* str_Then_reopen_your_ter;
extern RogueString* str_packages____;
extern RogueString* str_active_version_txt;
extern RogueString* str__bin_morlock;
extern RogueString* str_https___github_com_b;
extern RogueString* str__v_1;
extern RogueString* str_build_brombres_morlo;
extern RogueString* str_Build_rogue;
extern RogueString* str_Failed_to_find_extra;
extern RogueString* str_Compiling_morlock___;
extern RogueString* str_____rogo_build_;
extern RogueString* str_Source;
extern RogueString* str___rogue;
extern RogueString* str_brombres_morlock;
extern RogueString* str__bin_rogo;
extern RogueString* str_https___github_com_b_1;
extern RogueString* str_build_brombres_rogo;
extern RogueString* str_Makefile;
extern RogueString* str_Compiling_rogo___;
extern RogueString* str_____make_build_;
extern RogueString* str_brombres_rogo;
extern RogueString* str__bin_;
extern RogueString* str_https___github_com_b_2;
extern RogueString* str_build_brombres_rogue;
extern RogueString* str_make_bat;
extern RogueString* str_Compiling_;
extern RogueString* str____this_may_take_a_w;
extern RogueString* str_____make_build;
extern RogueString* str_xcopy__I__S__Q__Y_;
extern RogueString* str_Source_Libraries;
extern RogueString* str_Libraries;
extern RogueString* str_____make_build_LIBRA;
extern RogueString* str_brombres_rogue;
extern RogueString* str_Installing_the_Morlo;
extern RogueString* str_Package_name_must_be;
extern RogueString* str__archive_folder__can;
extern RogueString* str_Error_creating_folde;
extern RogueString* str_Downloading_;
extern RogueString* str_curl__LfsS_;
extern RogueString* str_Error_downloading_;
extern RogueString* str_zip;
extern RogueString* str_tar;
extern RogueString* str_gz;
extern RogueString* str_tarball;
extern RogueString* str__tar_gz;
extern RogueString* str_zipball;
extern RogueString* str__zip;
extern RogueString* str_Build;
extern RogueString* str_Build__;
extern RogueString* str_bin__;
extern RogueString* str_No_filepath_or_patte;
extern RogueString* str___OS_;
extern RogueString* str_Cannot_locate_execut;
extern RogueString* str__call_;
extern RogueString* str_v__I_;
extern RogueString* str___I___I_;
extern RogueString* str___I___I__1;
extern RogueString* str___I_;
extern RogueString* str_Cannot_determine_ver;
extern RogueString* str_id;
extern RogueString* str_platforms;
extern RogueString* str_filename;
extern RogueString* str_repo_releases;
extern RogueString* str__releases_latest;
extern RogueString* str_curl__fsSL__H__Accep;
extern RogueString* str_assets;
extern RogueString* str__releases;
extern RogueString* str_tag_name;
extern RogueString* str_tarball_url;
extern RogueString* str_zipball_url;
extern RogueString* str_No_releases_are_avai;
extern RogueString* str_No_release_is_compat;
extern RogueString* str____Available_version;
extern RogueString* str_No_releases_availabl;
extern RogueString* str__INTERNAL__Must_call;
extern RogueString* str_tar__C_;
extern RogueString* str___xf_;
extern RogueString* str_Cannot_unpack___file;
extern RogueString* str_wml;
extern RogueString* str_ml;
extern RogueString* str_path;
extern RogueString* str____17;
extern RogueString* str_____6;
extern RogueString* str______2;
extern RogueString* str_JSON_parse_error__;
extern RogueString* str___expected_;
extern RogueString* str_Identifier_expected_;
extern RogueString* str_Zip_extract___folder;
extern RogueString* str_Extracting_;
extern RogueString* str_Error_opening__;
extern RogueString* str_ERROR__File_sync_to_;
extern RogueString* str_ERROR__File_sync_to__1;
extern RogueString* str____18;
extern RogueString* str_Copying_;
extern RogueString* str_Deleting_unused_file;
extern RogueString* str_morlock;
extern RogueString* str_setup_py;
extern RogueString* str____1003l;
extern RogueString* str____1003h;
extern RogueString* str_PRESS_LEFT;
extern RogueString* str_PRESS_RIGHT;
extern RogueString* str_RELEASE;
extern RogueString* str_DRAG_LEFT;
extern RogueString* str_DRAG_RIGHT;
extern RogueString* str_MOVE;
extern RogueString* str_UnixConsoleMouseEven;
extern RogueString* str_Unrecognized_option_;
extern RogueString* str_Value_expected_after;
extern RogueString* str_Unexpected_value_giv;
#endif // TEST_H
