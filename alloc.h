// Copyright (C) 2026 Uday Tiwari
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef _ALLOC_H
#define _ALLOC_H

#include <stddef.h>
#include <stdint.h>

#ifdef USE_LIBC_MALLOC
#include <stdlib.h>
bool _free(void* ptr) {
  free(ptr);
  return true;
}
#endif

#ifdef ALLOC_DEBUG
#include <assert.h>
#define ASSERT(x) assert(x)
#else
#define ASSERT(x)
#endif

#ifndef ALLOCATOR_DEFAULT_CAP
#define ALLOCATOR_DEFAULT_CAP (16 * 1024)
#endif

typedef void* (*allocator)(size_t);
typedef bool (*deallocator)(void*);

typedef struct __page  page;
typedef struct __alloc alloc;

struct __page {
  page*     next;
  size_t    cap;
  size_t    fill;
  uintptr_t data[];
};

struct __alloc {
  page*       start;
  page*       curr;
  allocator   allocFn;
  deallocator freeFn;
};

page* new_page(alloc*, size_t);
void  clear_page(alloc*, page*);
void  create_alloc(alloc* alloc, size_t cap, allocator allocFn, deallocator freeFn);
void  destroy_alloc(alloc* alloc);

#ifdef ALLOC_IMPL

page* new_page(alloc* alloc, size_t capacity) {
  size_t actual_size = sizeof(page) + sizeof(uintptr_t) * capacity;
  page*  _page = (page*)alloc->allocFn(actual_size);
  _page->next = NULL;
  _page->fill = 0;
  _page->cap = capacity;
  return _page;
}

void clear_page(alloc* alloc, page* p) {
  bool ret = alloc->freeFn(p);
  ASSERT(ret);
}

void create_alloc(alloc* alloc, allocator allocFn, deallocator freeFn, size_t cap) {
#ifdef USE_LIBC_MALLOC
  alloc->allocFn = malloc;
  alloc->freeFn = _free;
#else
  alloc->allocFn = allocFn;
  alloc->freeFn = freeFn;
#endif
  ASSERT((alloc->allocFn != NULL && alloc->freeFn != NULL));
  page* _page = new_page(alloc, cap);
  ASSERT((_page != NULL));
}

void destroy_alloc(alloc* alloc) {
  page* _page = alloc->start;
  while (_page) {
    page* save = _page;
    _page = _page->next;
    clear_page(alloc, save);
  }

  alloc->start = NULL;
  alloc->curr = NULL;
}

#endif // ALLOC_IMPL

#endif // _ALLOC_H
