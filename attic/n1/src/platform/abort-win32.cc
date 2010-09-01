#include "platform/abort.h"
#include "utils/log.h"
#include "utils/string-inl.h"

#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <imagehlp.h>

namespace neutrino {

void Abort::abort(string format, const var_args &vars) {
  string_stream buf;
  buf.add(format, vars);
  printf("%s", buf.raw_string().start());
  ::abort();
}

typedef BOOL (__stdcall *stack_walk_t)(DWORD, HANDLE, HANDLE,
    LPSTACKFRAME, LPVOID, PREAD_PROCESS_MEMORY_ROUTINE,
    PFUNCTION_TABLE_ACCESS_ROUTINE, PGET_MODULE_BASE_ROUTINE,
    PTRANSLATE_ADDRESS_ROUTINE);

typedef DWORD (__stdcall *sym_get_module_info_t)(HANDLE, DWORD,
    PIMAGEHLP_MODULE);

typedef BOOL (__stdcall *sym_get_sym_from_addr_t)(HANDLE, DWORD,
    PDWORD, PIMAGEHLP_SYMBOL);

typedef LPVOID (__stdcall *sym_function_table_access_t)(HANDLE, DWORD);

typedef DWORD (__stdcall *sym_get_module_base_t)(HANDLE, DWORD);

typedef BOOL ( __stdcall *enumerate_loaded_modules_t)(HANDLE,
    PENUMLOADED_MODULES_CALLBACK, PVOID);

typedef DWORD ( __stdcall *sym_load_module_t)(HANDLE, HANDLE, PSTR,
    PSTR, DWORD, DWORD);

typedef BOOL (__stdcall *sym_initialize_t)(HANDLE, LPSTR, BOOL);

typedef BOOL (__stdcall *sym_get_line_from_addr_t)(HANDLE, DWORD,
    PDWORD, PIMAGEHLP_LINE);

#define eImageHelpProcedures(VISIT)                                  \
  VISIT(StackWalk, stack_walk)                                       \
  VISIT(SymGetModuleInfo, sym_get_module_info)                       \
  VISIT(SymGetSymFromAddr, sym_get_sym_from_addr)                    \
  VISIT(SymFunctionTableAccess, sym_function_table_access)           \
  VISIT(SymGetModuleBase, sym_get_module_base)                       \
  VISIT(EnumerateLoadedModules, enumerate_loaded_modules)            \
  VISIT(SymLoadModule, sym_load_module)                              \
  VISIT(SymInitialize, sym_initialize)                               \
  VISIT(SymGetLineFromAddr, sym_get_line_from_addr)

class ImageHelp {
public:
  ImageHelp(stack_walk_t _stack_walk,
      sym_get_module_info_t _sym_get_module_info,
      sym_get_sym_from_addr_t _sym_get_sym_from_addr,
      sym_function_table_access_t _sym_function_table_access,
      sym_get_module_base_t _sym_get_module_base,
      enumerate_loaded_modules_t _enumerate_loaded_modules,
      sym_load_module_t _sym_load_module,
      sym_initialize_t _sym_initialize,
      sym_get_line_from_addr_t _sym_get_line_from_addr)
    : stack_walk(_stack_walk)
    , sym_get_module_info(_sym_get_module_info)
    , sym_get_sym_from_addr(_sym_get_sym_from_addr)
    , sym_function_table_access(_sym_function_table_access)
    , sym_get_module_base(_sym_get_module_base)
    , enumerate_loaded_modules(_enumerate_loaded_modules)
    , sym_load_module(_sym_load_module)
    , sym_initialize(_sym_initialize)
    , sym_get_line_from_addr(_sym_get_line_from_addr) { }
  static ImageHelp *create();
#define DECLARE_FIELD(Name, name) name##_t name;
  eImageHelpProcedures(DECLARE_FIELD)
#undef DECLARE_FIELD
private:
  static ImageHelp *instance_cache_;
};

static void report_system_error(string message) {
  LPVOID buffer;
  FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      GetLastError(),
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
      reinterpret_cast<LPTSTR>(&buffer),
      0,
      NULL
  );
  LOG().error("%: %", vargs(message, string(reinterpret_cast<char*>(buffer))));
  LocalFree(buffer);
}

ImageHelp *ImageHelp::instance_cache_ = NULL;

ImageHelp *ImageHelp::create() {
  if (instance_cache_ != NULL) return instance_cache_;
  HMODULE module = ::LoadLibrary("IMAGEHLP.DLL");
  if (module == NULL) {
    report_system_error("Error loading IMAGEHLP.DLL");
    return NULL;
  }
#define RESOLVE_PROCEDURE(Name, name)                                        \
  name##_t name = reinterpret_cast<name##_t>(GetProcAddress(module, #Name)); \
  if (name == NULL) {                                                        \
    report_system_error("Error resolving " #Name);                           \
    return NULL;                                                             \
  }
  eImageHelpProcedures(RESOLVE_PROCEDURE)
#undef RESOLVE_PROCEDURE
  sym_initialize(GetCurrentProcess(), 0, true);
  return instance_cache_ = new ImageHelp(stack_walk, sym_get_module_info,
      sym_get_sym_from_addr, sym_function_table_access, sym_get_module_base,
      enumerate_loaded_modules, sym_load_module, sym_initialize,
      sym_get_line_from_addr);
}

class StackWalker {
public:
  StackWalker(ImageHelp &image_help)
    : image_help_(image_help), own_process_(NULL), own_thread_(NULL) { }
  bool initialize();
  void walk(LPCONTEXT context);
  bool get_module_info(DWORD addr, PIMAGEHLP_MODULE module,
      PIMAGEHLP_LINE line_info);
  bool process_module(LPSTR module_name, ULONG module_base,
      ULONG module_size);
  ImageHelp &image_help() { return image_help_; }
  HANDLE own_process() { return own_process_; }
  HANDLE own_thread() { return own_thread_; }
private:
  ImageHelp &image_help_;
  HANDLE own_process_;
  HANDLE own_thread_;
  DWORD current_addr_;
};

bool StackWalker::initialize() {
  own_process_ = GetCurrentProcess();
  own_thread_ = GetCurrentThread();
  return true;
}

static BOOL CALLBACK enumerator_bridge(LPSTR module_name,
    ULONG module_base, ULONG module_size, PVOID data) {
  return reinterpret_cast<StackWalker*>(data)->process_module(module_name,
      module_base, module_size);
}

bool StackWalker::process_module(LPSTR module_name, ULONG module_base,
    ULONG module_size) {
  DWORD addr = current_addr_;
  if (module_base <= addr && addr <= module_base + module_size) {
    if (!image_help().sym_load_module(own_process(), NULL, module_name,
        NULL, module_base, module_size)) {
      report_system_error("Error loading module");
      return false;
    } else {
      return true;
    }
  }
  return false;
}

bool StackWalker::get_module_info(DWORD addr, PIMAGEHLP_MODULE module,
    PIMAGEHLP_LINE line_info) {
  module->SizeOfStruct = sizeof(IMAGEHLP_MODULE);
  if (!image_help().sym_get_module_info(own_process(), addr, module)) {
    // If this doesn't work the module may not have been loaded.  Try
    // to load it and then retry getting the info.
    current_addr_ = addr;
    if (!image_help().enumerate_loaded_modules(own_process(),
        enumerator_bridge, reinterpret_cast<PVOID>(this)))
      return false;
    if (!image_help().sym_get_module_info(own_process(), addr, module))
      return false;
  }
  line_info->SizeOfStruct = sizeof(IMAGEHLP_LINE);
  DWORD displacement = 0;
  return image_help().sym_get_line_from_addr(own_process(), addr,
      &displacement, line_info);
}

void StackWalker::walk(LPCONTEXT context_in) {
  CONTEXT context = *context_in;
  STACKFRAME frame;
  memset(&frame, 0, sizeof frame);
  frame.AddrPC.Offset = context.Eip;
  frame.AddrPC.Mode = AddrModeFlat;
  frame.AddrStack.Offset = context.Esp;
  frame.AddrStack.Mode = AddrModeFlat;
  frame.AddrFrame.Offset = context.Ebp;
  frame.AddrFrame.Mode = AddrModeFlat;

  while (true) {
    bool ok = image_help().stack_walk(IMAGE_FILE_MACHINE_I386,
        own_process(), own_thread(), &frame, &context, 0,
        image_help().sym_function_table_access,
        image_help().sym_get_module_base, 0);
    if (!ok || frame.AddrPC.Offset == 0)
      break;

    IMAGEHLP_MODULE module_info;
    memset(&module_info, 0, sizeof(module_info));
    IMAGEHLP_LINE line_info;
    memset(&line_info, 0, sizeof(line_info));
    bool got_line_info = get_module_info(frame.AddrPC.Offset, &module_info,
        &line_info);

    static const word kSymbolSize = 512;
    char buf[sizeof(IMAGEHLP_SYMBOL) + kSymbolSize];
    PIMAGEHLP_SYMBOL symbol = reinterpret_cast<PIMAGEHLP_SYMBOL>(buf);
    symbol->SizeOfStruct = sizeof(buf);
    symbol->MaxNameLength = kSymbolSize;

    DWORD offset = 0;
    if (image_help().sym_get_sym_from_addr(own_process(),
        frame.AddrPC.Offset, &offset, symbol)) {
      fprintf(stderr, "%s", symbol->Name);
    } else {
      fprintf(stderr, "0x%08x", frame.AddrPC.Offset);
    }

    if (got_line_info) {
      fprintf(stderr, " (%s:%i)\n", line_info.FileName, line_info.LineNumber);
    } else {
      fprintf(stderr, "\n");
    }
  }
  fflush(stderr);
}

static void report_crash(LPCONTEXT context) {
  fprintf(stderr, "--- Crash ---\n");
  ImageHelp *image_help = ImageHelp::create();
  if (image_help == NULL) return;
  StackWalker walker(*image_help);
  if (!walker.initialize()) return;
  string_stream out;
  walker.walk(context);
}

static LONG WINAPI filter_unhandled_exception(LPEXCEPTION_POINTERS e) {
  word code = e->ExceptionRecord->ExceptionCode;
  if (code == EXCEPTION_ACCESS_VIOLATION) {
    report_crash(e->ContextRecord);
  } else {
    LOG().info("Filtered exception %i\n", vargs(code));
  }
  return EXCEPTION_CONTINUE_SEARCH;
}

void Abort::install_signal_handlers() {
  SetUnhandledExceptionFilter(filter_unhandled_exception);
}

} // namespace neutrino
