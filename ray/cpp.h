/*
 * Copyright (c) 2026 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#pragma once

// Macros for class definitions.

// Deletes the copy and move constructors and assignment operators.
#define DISALLOW_COPY_ASSIGN_AND_MOVE(_class_name)                                                 \
    _class_name(const _class_name &) = delete;                                                     \
    _class_name(_class_name &&) = delete;                                                          \
    _class_name &operator=(const _class_name &) = delete;                                          \
    _class_name &operator=(_class_name &&) = delete

// Deletes the copy constructor and assignment operator, leaving the class movable.
#define DISALLOW_COPY_AND_ASSIGN_ALLOW_MOVE(_class_name)                                           \
    _class_name(const _class_name &) = delete;                                                     \
    _class_name &operator=(const _class_name &) = delete
