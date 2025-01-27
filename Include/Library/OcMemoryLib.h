/** @file
  Copyright (C) 2019, vit9696. All rights reserved.

  All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef OC_MEMORY_LIB_H
#define OC_MEMORY_LIB_H

#include <Uefi.h>
#include <IndustryStandard/VirtualMemory.h>

/**
  Reverse equivalent of NEXT_MEMORY_DESCRIPTOR.
**/
#define PREV_MEMORY_DESCRIPTOR(MemoryDescriptor, Size) \
  ((EFI_MEMORY_DESCRIPTOR *)((UINT8 *)(MemoryDescriptor) - (Size)))


/**
  Get last descriptor address.
  It is assumed that the descriptor contains pages.
**/
#define LAST_DESCRIPTOR_ADDR(Desc) \
  ((Desc)->PhysicalStart + (EFI_PAGES_TO_SIZE ((UINTN) (Desc)->NumberOfPages) - 1))

/**
  Check if area is within the specified descriptor.
  It is assumed that the descriptor contains pages and AreaSize is not 0.
**/
#define AREA_WITHIN_DESCRIPTOR(Desc, Area, AreaSize) \
  ((Area) >= (Desc)->PhysicalStart && ((Area) + ((AreaSize) - 1)) <= LAST_DESCRIPTOR_ADDR (Desc))

/**
  Reasonable default virtual memory page pool size (2 MB).
**/
#define OC_DEFAULT_VMEM_PAGE_COUNT 0x200

/**
  Lock the legacy region specified to enable modification.

  @param[in] LegacyAddress  The address of the region to lock.
  @param[in] LegacyLength   The size of the region to lock.

  @retval EFI_SUCCESS  The region was locked successfully.
**/
EFI_STATUS
LegacyRegionLock (
  IN UINT32  LegacyAddress,
  IN UINT32  LegacyLength
  );

/**
  Unlock the legacy region specified to enable modification.

  @param[in] LegacyAddress  The address of the region to unlock.
  @param[in] LegacyLength   The size of the region to unlock.

  @retval EFI_SUCCESS  The region was unlocked successfully.
**/
EFI_STATUS
LegacyRegionUnlock (
  IN UINT32  LegacyAddress,
  IN UINT32  LegacyLength
  );

/**
  Get current memory map allocated on pool.

  @param[out]  MemoryMapSize      Resulting memory map size in bytes.
  @param[out]  DescriptorSize     Resulting memory map descriptor size in bytes.
  @param[out]  MapKey             Memory map key, optional.
  @param[out]  DescriptorVersion  Memory map descriptor version, optional.

  @retval current memory map or NULL.
**/
EFI_MEMORY_DESCRIPTOR *
GetCurrentMemoryMap (
  OUT UINTN   *MemoryMapSize,
  OUT UINTN   *DescriptorSize,
  OUT UINTN   *MapKey             OPTIONAL,
  OUT UINT32  *DescriptorVersion  OPTIONAL
  );

/**
  Get current memory map of custom allocation.

  @param[out]    MemoryMapSize      Resulting memory map size in bytes.
  @param[out]    MemoryMap          Resulting memory map.
  @param[out]    MapKey             Memory map key.
  @param[out]    DescriptorSize     Resulting memory map descriptor size in bytes.
  @param[out]    DescriptorVersion  Memory map descriptor version.
  @param[in]     GetMemoryMap       Custom GetMemoryMap implementation to use, optional.
  @param[in,out] TopMemory          Base top address for AllocatePagesFromTop allocation.

  @retval EFI_SUCCESS on success.
**/
EFI_STATUS
GetCurrentMemoryMapAlloc (
     OUT UINTN                  *MemoryMapSize,
     OUT EFI_MEMORY_DESCRIPTOR  **MemoryMap,
     OUT UINTN                  *MapKey,
     OUT UINTN                  *DescriptorSize,
     OUT UINT32                 *DescriptorVersion,
  IN     EFI_GET_MEMORY_MAP     GetMemoryMap  OPTIONAL,
  IN OUT UINTN                  *TopMemory  OPTIONAL
  );

/**
  Shrink memory map by joining non-runtime records.

  @param[in,out]  MemoryMapSize      Memory map size in bytes, updated on shrink.
  @param[in,out]  MemoryMap          Memory map to shrink.
  @param[in]      DescriptorSize     Memory map descriptor size in bytes.
**/
VOID
ShrinkMemoryMap (
  IN OUT UINTN                  *MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN     UINTN                  DescriptorSize
  );

/**
  Check range allocation compatibility callback.

  @param[in]  Address      Starting address.
  @param[in]  Size         Size of memory range.

  @retval TRUE when suitable for allocation.
**/
typedef
BOOLEAN
(*CHECK_ALLOCATION_RANGE) (
  IN EFI_PHYSICAL_ADDRESS  Address,
  IN UINTN                 Size
  );


/**
  Filter memory map entries.

  @param[in]      Context            Parameterised filter data.
  @param[in,out]  MemoryMapSize      Memory map size in bytes.
  @param[in,out]  MemoryMap          Memory map to filter.
  @param[in]      DescriptorSize     Memory map descriptor size in bytes.
**/
typedef
VOID
(*OC_MEMORY_FILTER) (
  IN     VOID                        *Context  OPTIONAL,
  IN     UINTN                       MemoryMapSize,
  IN OUT EFI_MEMORY_DESCRIPTOR       *MemoryMap,
  IN     UINTN                       DescriptorSize
  );

/**
  Allocate pages from the top of physical memory up to address specified in Memory.
  Unlike AllocateMaxAddress, this method guarantees to choose top most address.

  @param[in]      MemoryType       Allocated memory type.
  @param[in]      Pages            Amount of pages to allocate.
  @param[in,out]  Memory           Top address for input, allocated address for output.
  @param[in]      GetMemoryMap     Custom GetMemoryMap implementation to use, optional.
  @param[in]      CheckRange       Handler allowing to not allocate select ranges, optional.

  @retval EFI_SUCCESS on successful allocation.
**/
EFI_STATUS
AllocatePagesFromTop (
  IN     EFI_MEMORY_TYPE         MemoryType,
  IN     UINTN                   Pages,
  IN OUT EFI_PHYSICAL_ADDRESS    *Memory,
  IN     EFI_GET_MEMORY_MAP      GetMemoryMap  OPTIONAL,
  IN     CHECK_ALLOCATION_RANGE  CheckRange  OPTIONAL
  );

/**
  Calculate number of runtime pages in the memory map.

  @param[in]     MemoryMapSize      Memory map size in bytes.
  @param[in]     MemoryMap          Memory map to inspect.
  @param[in]     DescriptorSize     Memory map descriptor size in bytes.
  @param[out]    DescriptorCount    Number of relevant descriptors, optional.

  @retval Number of runtime pages.
**/
UINTN
CountRuntimePages (
  IN  UINTN                  MemoryMapSize,
  IN  EFI_MEMORY_DESCRIPTOR  *MemoryMap,
  IN  UINTN                  DescriptorSize,
  OUT UINTN                  *DescriptorCount OPTIONAL
  );

/**
  Return pointer to PML4 table in PageTable and PWT and PCD flags in Flags.

  @param[out]  Flags      Current page table PWT and PCT flags.

  @retval Current page table address.
**/
PAGE_MAP_AND_DIRECTORY_POINTER  *
GetCurrentPageTable (
  OUT UINTN                           *Flags  OPTIONAL
  );

/**
  Return physical addrress for given virtual addrress.

  @param[in]  PageTable       Page table to use for solving.
  @param[in]  VirtualAddr     Virtual address to look up.
  @param[out] PhysicalAddr    Physical address to return.

  @retval EFI_SUCCESS on successful lookup.
**/
EFI_STATUS
GetPhysicalAddress (
  IN  PAGE_MAP_AND_DIRECTORY_POINTER   *PageTable  OPTIONAL,
  IN  EFI_VIRTUAL_ADDRESS              VirtualAddr,
  OUT EFI_PHYSICAL_ADDRESS             *PhysicalAddr
  );

/**
  Virtual memory context
**/
typedef struct OC_VMEM_CONTEXT_ {
  ///
  /// Memory pool containing memory to be spread across allocations.
  ///
  UINT8  *MemoryPool;
  ///
  /// Free pages in the memory pool.
  ///
  UINTN  FreePages;
} OC_VMEM_CONTEXT;

/**
  Allocate EfiBootServicesData virtual memory pool from boot services
  in the end of BASE_4GB of RAM. Should be called while boot services are
  still usable.

  @param[out]  Context   Virtual memory pool context.
  @param[in]   NumPages  Number of pages to be allocated in the pool.

  @retval EFI_SUCCESS on successful allocation.
**/
EFI_STATUS
VmAllocateMemoryPool (
  OUT OC_VMEM_CONTEXT  *Context,
  IN  UINTN            NumPages
  );

/**
  Allocate pages for e.g. vm page maps.

  @param[in,out]  Context   Virtual memory pool context.
  @param[in]      NumPages  Number of pages to allocate.

  @retval allocated pages or NULL.
**/
VOID *
VmAllocatePages (
  IN OUT OC_VMEM_CONTEXT  *Context,
  IN     UINTN            NumPages
  );

/**
  Map (remap) given page at physical address to given virtual address in
  the specified page table.

  @param[in,out]  Context       Virtual memory pool context.
  @param[in]      PageTable     Page table to update.
  @param[in]      VirtualAddr   Virtual memory address to map at.
  @param[in]      PhysicalAddr  Physical memory address to map from.

  @retval EFI_SUCCESS on success.
**/
EFI_STATUS
VmMapVirtualPage (
  IN OUT OC_VMEM_CONTEXT                 *Context,
  IN OUT PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable  OPTIONAL,
  IN     EFI_VIRTUAL_ADDRESS             VirtualAddr,
  IN     EFI_PHYSICAL_ADDRESS            PhysicalAddr
  );

/**
  Map (remap) a range of 4K pages at physical address to given virtual address
  in the specified page table.

  @param[in,out]  Context       Virtual memory pool context.
  @param[in]      PageTable     Page table to update.
  @param[in]      VirtualAddr   Virtual memory address to map at.
  @param[in]      NumPages      Number of 4K pages to map.
  @param[in]      PhysicalAddr  Physical memory address to map from.

  @retval EFI_SUCCESS on success.
**/
EFI_STATUS
VmMapVirtualPages (
  IN OUT OC_VMEM_CONTEXT                 *Context,
  IN OUT PAGE_MAP_AND_DIRECTORY_POINTER  *PageTable  OPTIONAL,
  IN     EFI_VIRTUAL_ADDRESS             VirtualAddr,
  IN     UINTN                           NumPages,
  IN     EFI_PHYSICAL_ADDRESS            PhysicalAddr
  );

/**
  Flushes TLB caches.
**/
VOID
VmFlushCaches (
  VOID
  );

/**
  Check whether built-in allocator is initialized.

  @retval TRUE on success.
**/
BOOLEAN
UmmInitialized (
  VOID
  );

/**
  Initialize built-in allocator.

  @param[in]  Heap  Memory pool used for allocations.
  @param[in]  Size  Memory pool size.
**/
VOID
UmmSetHeap (
  IN VOID    *Heap,
  IN UINT32  Size
  );

/**
  Perform allocation from built-in allocator.

  @param[in]  Size  Allocation size.

  @retval allocated memory on success.
**/
VOID *
UmmMalloc (
  IN UINT32  Size
  );

/**
  Perform free of allocated memory. Accepts NULL pointer
  and checks whether memory belongs to itself.

  @param[in]  Ptr  Memory to free.

  @retval TRUE on success
**/
BOOLEAN
UmmFree (
  IN VOID  *Ptr
  );

#endif // OC_MEMORY_LIB_H
