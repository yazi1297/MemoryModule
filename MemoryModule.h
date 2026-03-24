/*
 * Memory DLL loading code
 * Version 0.0.4
 *
 * Copyright (c) 2004-2015 by Joachim Bauch / mail@joachim-bauch.de
 * http://www.joachim-bauch.de
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is MemoryModule.h
 *
 * The Initial Developer of the Original Code is Joachim Bauch.
 *
 * Portions created by Joachim Bauch are Copyright (C) 2004-2015
 * Joachim Bauch. All Rights Reserved.
 *
 */

#ifndef __MEMORY_MODULE_HEADER
#define __MEMORY_MODULE_HEADER

#include <windows.h>
#include <stdint.h>

typedef void *HMEMORYMODULE;

typedef void *HMEMORYRSRC;

typedef void *HCUSTOMMODULE;

// Target architecture selector for emulator-oriented loading.
// AUTO: select by PE header machine and host compatibility.
#define MEMORYMODULE_ARCH_AUTO 0
#define MEMORYMODULE_ARCH_X86  1
#define MEMORYMODULE_ARCH_X64  2

#ifdef __cplusplus
extern "C" {
#endif

typedef LPVOID (*CustomAllocFunc)(LPVOID, SIZE_T, DWORD, DWORD, void*);
typedef BOOL (*CustomFreeFunc)(LPVOID, SIZE_T, DWORD, void*);
typedef HCUSTOMMODULE (*CustomLoadLibraryFunc)(LPCSTR, void *);
typedef FARPROC (*CustomGetProcAddressFunc)(HCUSTOMMODULE, LPCSTR, void *);
typedef void (*CustomFreeLibraryFunc)(HCUSTOMMODULE, void *);

/**
 * Load EXE/DLL from memory location with the given size.
 *
 * All dependencies are resolved using default LoadLibrary/GetProcAddress
 * calls through the Windows API.
 */
HMEMORYMODULE MemoryLoadLibrary(const void *, size_t);

HMEMORYMODULE MemoryLoadLibrary2(const void *, size_t);

/**
 * Load EXE/DLL from memory location with the given size using a requested
 * desired virtual base address.
 *
 * This follows MemoryModule's normal PE loading steps (section alignment,
 * base relocation, import fixing, export/resource support). The loader will
 * relocate the in-memory image as if it was loaded at desired_base_addr,
 * even if host allocation had to fall back to a different address.
 *
 * Returns:
 *  - out_base_addr: the actual host pointer where the image blob is stored
 *  - out_image_size: the contiguous allocated size (aligned image size)
 */
HMEMORYMODULE MemoryLoadLibraryDesiredBase(const void *data,
                                             size_t size,
                                             uintptr_t desired_base_addr,
                                             LPVOID *out_base_addr,
                                             size_t *out_image_size);

/**
 * Load EXE/DLL with explicit target architecture selector.
 *
 * target_arch:
 *   - MEMORYMODULE_ARCH_AUTO / X86 / X64
 *
 * For host-matching targets, strict loader path is used.
 * For cross-arch emulator use-cases, relaxed loader path is used.
 */
HMEMORYMODULE MemoryLoadLibraryDesiredBaseForArch(const void *data,
                                                    size_t size,
                                                    int target_arch,
                                                    uintptr_t desired_base_addr,
                                                    LPVOID *out_base_addr,
                                                    size_t *out_image_size);

/**
 * Load EXE/DLL from memory location with the given size using custom dependency
 * resolvers.
 *
 * Dependencies will be resolved using passed callback methods.
 */
HMEMORYMODULE MemoryLoadLibraryEx(const void *, size_t,
    CustomAllocFunc,
    CustomFreeFunc,
    CustomLoadLibraryFunc,
    CustomGetProcAddressFunc,
    CustomFreeLibraryFunc,
    void *);

HMEMORYMODULE MemoryLoadLibraryEx2(const void *, size_t,
    CustomAllocFunc,
    CustomFreeFunc,
    CustomLoadLibraryFunc,
    CustomGetProcAddressFunc,
    CustomFreeLibraryFunc,
    void *);
/**
 * Get address of exported method. Supports loading both by name and by
 * ordinal value.
 */
FARPROC MemoryGetProcAddress(HMEMORYMODULE, LPCSTR);

/**
 * Free previously loaded EXE/DLL.
 */
void MemoryFreeLibrary(HMEMORYMODULE);

/**
 * Execute entry point (EXE only). The entry point can only be executed
 * if the EXE has been loaded to the correct base address or it could
 * be relocated (i.e. relocation information have not been stripped by
 * the linker).
 *
 * Important: calling this function will not return, i.e. once the loaded
 * EXE finished running, the process will terminate.
 *
 * Returns a negative value if the entry point could not be executed.
 */
int MemoryCallEntryPoint(HMEMORYMODULE);

/**
 * Get entry point address (EXE only).
 *
 * Returns the address of the entry point if the module is a valid EXE,
 * otherwise returns NULL.
 */
LPVOID MemoryGetEntryPoint(HMEMORYMODULE);

/**
 * Get base address of loaded module.
 *
 * Returns the base address where the module is loaded in memory.
 */
LPVOID MemoryGetBaseAddress(HMEMORYMODULE);

/**
 * Get image base address of loaded module.
 *
 * Returns the image base address where the module is loaded in memory.
 */
ULONGLONG MemoryGetImageBase(HMEMORYMODULE);

/**
 * Get the image size of the module in bytes.
 */
DWORD MemoryGetImageSize(HMEMORYMODULE);

/**
 * Find the location of a resource with the specified type and name.
 */
HMEMORYRSRC MemoryFindResource(HMEMORYMODULE, LPCTSTR, LPCTSTR);

/**
 * Find the location of a resource with the specified type, name and language.
 */
HMEMORYRSRC MemoryFindResourceEx(HMEMORYMODULE, LPCTSTR, LPCTSTR, WORD);

/**
 * Get the size of the resource in bytes.
 */
DWORD MemorySizeofResource(HMEMORYMODULE, HMEMORYRSRC);

/**
 * Get a pointer to the contents of the resource.
 */
LPVOID MemoryLoadResource(HMEMORYMODULE, HMEMORYRSRC);

/**
 * Load a string resource.
 */
int MemoryLoadString(HMEMORYMODULE, UINT, LPTSTR, int);

/**
 * Load a string resource with a given language.
 */
int MemoryLoadStringEx(HMEMORYMODULE, UINT, LPTSTR, int, WORD);

/**
* Default implementation of CustomAllocFunc that calls VirtualAlloc
* internally to allocate memory for a library
*
* This is the default as used by MemoryLoadLibrary.
*/
LPVOID MemoryDefaultAlloc(LPVOID, SIZE_T, DWORD, DWORD, void *);

/**
* Default implementation of CustomFreeFunc that calls VirtualFree
* internally to free the memory used by a library
*
* This is the default as used by MemoryLoadLibrary.
*/
BOOL MemoryDefaultFree(LPVOID, SIZE_T, DWORD, void *);

/**
 * Default implementation of CustomLoadLibraryFunc that calls LoadLibraryA
 * internally to load an additional libary.
 *
 * This is the default as used by MemoryLoadLibrary.
 */
HCUSTOMMODULE MemoryDefaultLoadLibrary(LPCSTR, void *);

/**
 * Default implementation of CustomGetProcAddressFunc that calls GetProcAddress
 * internally to get the address of an exported function.
 *
 * This is the default as used by MemoryLoadLibrary.
 */
FARPROC MemoryDefaultGetProcAddress(HCUSTOMMODULE, LPCSTR, void *);

/**
 * Default implementation of CustomFreeLibraryFunc that calls FreeLibrary
 * internally to release an additional libary.
 *
 * This is the default as used by MemoryLoadLibrary.
 */
void MemoryDefaultFreeLibrary(HCUSTOMMODULE, void *);

#ifdef __cplusplus
}
#endif

#endif  // __MEMORY_MODULE_HEADER
