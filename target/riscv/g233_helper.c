/*
 * RISC-V G233 Custom Instructions Helpers
 *
 * Copyright (c) 2025 Learning QEMU 2025
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "qemu/osdep.h"
#include "cpu.h"
#include "exec/helper-proto.h"
#include "accel/tcg/cpu-ldst.h"

/* DMA instruction - matrix transpose */
void helper_dma(CPURISCVState *env, target_ulong dst, target_ulong src, target_ulong grain)
{
    int grain_size;
    
    /* Determine matrix size based on grain parameter */
    switch (grain) {
        case 0:
            grain_size = 8;
            break;
        case 1:
            grain_size = 16;
            break;
        case 2:
            grain_size = 32;
            break;
        default:
            grain_size = 8;
            break;
    }
    
    /* Perform matrix transpose */
    uint32_t temp[grain_size * grain_size];
    
    /* Read source matrix */
    for (int i = 0; i < grain_size; i++) {
        for (int j = 0; j < grain_size; j++) {
            temp[i * grain_size + j] = cpu_ldl_data(env, src + (i * grain_size + j) * 4);
        }
    }
    
    /* Write transposed matrix to destination */
    for (int i = 0; i < grain_size; i++) {
        for (int j = 0; j < grain_size; j++) {
            cpu_stl_data(env, dst + (i * grain_size + j) * 4, temp[j * grain_size + i]);
        }
    }
}

/* Sort instruction - array sorting */
void helper_sort(CPURISCVState *env, target_ulong addr, target_ulong array_num, target_ulong sort_num)
{
    int arr_size = (int)array_num;
    int sort_size = (int)sort_num;
    
    if (sort_size > arr_size) {
        sort_size = arr_size;
    }
    
    /* Read array */
    uint32_t *arr = g_malloc(arr_size * sizeof(uint32_t));
    for (int i = 0; i < arr_size; i++) {
        arr[i] = cpu_ldl_data(env, addr + i * 4);
    }
    
    /* Bubble sort on first sort_num elements */
    for (int i = 0; i < sort_size - 1; i++) {
        for (int j = 0; j < sort_size - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                uint32_t temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
    
    /* Write sorted array back */
    for (int i = 0; i < arr_size; i++) {
        cpu_stl_data(env, addr + i * 4, arr[i]);
    }
    
    g_free(arr);
}

/* Crush instruction - pack low 4 bits */
void helper_crush(CPURISCVState *env, target_ulong dst, target_ulong src, target_ulong num)
{
    int n = (int)num;
    
    /* Pack pairs of 4-bit values into 8-bit values */
    for (int i = 0; i < n / 2; i++) {
        uint8_t val1 = cpu_ldub_data(env, src + i * 2) & 0x0F;
        uint8_t val2 = cpu_ldub_data(env, src + i * 2 + 1) & 0x0F;
        uint8_t packed = (val2 << 4) | val1;
        cpu_stb_data(env, dst + i, packed);
    }
}

/* Expand instruction - unpack to 4 bits */
void helper_expand(CPURISCVState *env, target_ulong dst, target_ulong src, target_ulong num)
{
    int n = (int)num;
    
    /* Unpack 8-bit values into pairs of 4-bit values */
    for (int i = 0; i < n; i++) {
        uint8_t val = cpu_ldub_data(env, src + i);
        uint8_t low = val & 0x0F;
        uint8_t high = (val >> 4) & 0x0F;
        cpu_stb_data(env, dst + i * 2, low);
        cpu_stb_data(env, dst + i * 2 + 1, high);
    }
}

