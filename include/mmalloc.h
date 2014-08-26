/*
 * (C) Copyright 2011
 * Beijing HLYT Technology Co., Ltd.
 *
 * mmalloc.h - A brief description to describe this file.
 *
 */

#ifndef _HEAD_MMALLOC_2935A66A_0E8348ED_4B59ABBD_H
#define _HEAD_MMALLOC_2935A66A_0E8348ED_4B59ABBD_H

#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

void *mm_allocate(size_t sz);
void mm_deallocate(void *storage);

#if defined(__cplusplus)
}
#endif

#endif /* #ifndef _HEAD_MMALLOC_2935A66A_0E8348ED_4B59ABBD_H */
