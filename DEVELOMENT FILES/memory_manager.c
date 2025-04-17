#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Biến toàn cục
static int mem_size = 0;
static block* free1 = NULL;
static int free1_count = 0;
static int free1_capacity = 0;
static block* alloc = NULL;
static int alloc_count = 0;
static int alloc_capacity = 0;
static proj* proces = NULL;
static int proces_count = 0;
static int proces_capacity = 0;

// Hàm so sánh để sắp xếp
static int compareByStart(const void* a, const void* b) {
    block* blockA = (block*)a;
    block* blockB = (block*)b;
    return blockA->start - blockB->start;
}

static int compareBySize(const void* a, const void* b) {
    block* blockA = (block*)a;
    block* blockB = (block*)b;
    return blockA->size - blockB->size;
}

// Hàm thêm block vào mảng động
static void add_block(block** arr, int* count, int* capacity, block b) {
    if (*count >= *capacity) {
        *capacity = (*capacity == 0) ? 10 : *capacity * 2;
        *arr = (block*)realloc(*arr, *capacity * sizeof(block));
    }
    (*arr)[*count] = b;
    (*count)++;
}

// Hàm xóa block khỏi mảng động
static void remove_block(block** arr, int* count, int index) {
    for (int i = index; i < *count - 1; i++) {
        (*arr)[i] = (*arr)[i + 1];
    }
    (*count)--;
}

// Hàm khởi tạo một proj
static proj create_proj(const char* name, int num_seg) {
    proj p;
    strncpy(p.name, name, sizeof(p.name) - 1);
    p.name[sizeof(p.name) - 1] = '\0';
    p.num_seg = num_seg;
    p.allocated = 0;
    p.segs_count = 0;
    p.segs_capacity = 10;
    p.segs = (block*)malloc(p.segs_capacity * sizeof(block));
    return p;
}

// Thuật toán First Fit
static void first_fit(proj* p) {
    int counter = 0;
    block* first_vec = (block*)malloc(free1_count * sizeof(block));
    memcpy(first_vec, free1, free1_count * sizeof(block));
    int first_vec_count = free1_count;

    for (int n = 0; n < p->num_seg; n++) {
        int found = 0;
        for (int i = 0; i < first_vec_count; i++) {
            if (first_vec[i].size >= p->segs[n].size) {
                p->segs[n].start = first_vec[i].start;
                first_vec[i].size -= p->segs[n].size;
                first_vec[i].start += p->segs[n].size;
                p->segs[n].end = p->segs[n].start + p->segs[n].size - 1;
                counter++;
                if (first_vec[i].size < 1) {
                    remove_block(&first_vec, &first_vec_count, i);
                }
                found = 1;
                break;
            }
        }
        if (!found) break;
    }

    if (counter == p->num_seg && !p->allocated) {
        p->allocated = 1;
        free(free1);
        free1 = first_vec;
        free1_count = first_vec_count;
        for (int n = 0; n < p->num_seg; n++) {
            add_block(&alloc, &alloc_count, &alloc_capacity, p->segs[n]);
        }
    } else {
        free(first_vec);
    }
}

// Thuật toán Best Fit
static void best_fit(proj* p) {
    int counter = 0;
    block* first_vec = (block*)malloc(free1_count * sizeof(block));
    memcpy(first_vec, free1, free1_count * sizeof(block));
    int first_vec_count = free1_count;

    qsort(first_vec, first_vec_count, sizeof(block), compareBySize);

    for (int n = 0; n < p->num_seg; n++) {
        int found = 0;
        for (int i = 0; i < first_vec_count; i++) {
            if (first_vec[i].size >= p->segs[n].size) {
                p->segs[n].start = first_vec[i].start;
                first_vec[i].size -= p->segs[n].size;
                first_vec[i].start += p->segs[n].size;
                p->segs[n].end = p->segs[n].start + p->segs[n].size - 1;
                counter++;
                if (first_vec[i].size < 1) {
                    remove_block(&first_vec, &first_vec_count, i);
                }
                qsort(first_vec, first_vec_count, sizeof(block), compareBySize);
                found = 1;
                break;
            }
        }
        if (!found) break;
    }

    if (counter == p->num_seg && !p->allocated) {
        p->allocated = 1;
        printf("hello %d\n", counter);
        free(free1);
        free1 = first_vec;
        free1_count = first_vec_count;
        for (int n = 0; n < p->num_seg; n++) {
            add_block(&alloc, &alloc_count, &alloc_capacity, p->segs[n]);
        }
    } else {
        free(first_vec);
    }
}

// Hàm giải phóng bộ nhớ cho một block
static void de_allocate(block seg) {
    qsort(alloc, alloc_count, sizeof(block), compareByStart);
    qsort(free1, free1_count, sizeof(block), compareByStart);

    for (int i = 0; i < alloc_count; i++) {
        if (seg.start == alloc[i].start) {
            printf("yes i am %s go to delete %d %d %d\n", seg.name, seg.start, seg.size, seg.end);

            // Trường hợp toàn bộ bộ nhớ
            if (seg.size == mem_size) {
                add_block(&free1, &free1_count, &free1_capacity, seg);
                remove_block(&alloc, &alloc_count, i);
                break;
            }
            // Trường hợp đầu bộ nhớ
            else if (seg.start == 0) {
                if (i + 1 < alloc_count && seg.end + 1 != alloc[i + 1].start) {
                    free1[0].start = 0;
                    free1[0].size = free1[0].end - free1[0].start + 1;
                    remove_block(&alloc, &alloc_count, i);
                    break;
                } else {
                    add_block(&free1, &free1_count, &free1_capacity, seg);
                    remove_block(&alloc, &alloc_count, i);
                    break;
                }
            }
            // Trường hợp cuối bộ nhớ
            else if (seg.end == mem_size - 1) {
                if (i > 0 && seg.start - 1 != alloc[i - 1].end) {
                    free1[free1_count - 1].end = mem_size - 1;
                    free1[free1_count - 1].size = free1[free1_count - 1].end - free1[free1_count - 1].start + 1;
                    remove_block(&alloc, &alloc_count, i);
                    break;
                } else {
                    add_block(&free1, &free1_count, &free1_capacity, seg);
                    remove_block(&alloc, &alloc_count, i);
                    break;
                }
            }
            // Trường hợp giữa bộ nhớ: alloc-alloc
            else if (i > 0 && i + 1 < alloc_count && seg.start - 1 == alloc[i - 1].end && seg.end + 1 == alloc[i + 1].start) {
                block x = { "", seg.start, seg.size, seg.end };
                strncpy(x.name, "", sizeof(x.name));
                add_block(&free1, &free1_count, &free1_capacity, x);
                remove_block(&alloc, &alloc_count, i);
                break;
            }
            // Trường hợp giữa bộ nhớ: hole-alloc
            else if (i > 0 && i + 1 < alloc_count && seg.start - 1 != alloc[i - 1].end && seg.end + 1 == alloc[i + 1].start) {
                for (int j = 0; j < free1_count; j++) {
                    if (seg.start - 1 == free1[j].end) {
                        free1[j].end = seg.end;
                        free1[j].size = free1[j].end - free1[j].start + 1;
                        remove_block(&alloc, &alloc_count, i);
                        break;
                    }
                }
                break;
            }
            // Trường hợp giữa bộ nhớ: alloc-hole
            else if (i > 0 && i + 1 < alloc_count && seg.start - 1 == alloc[i - 1].end && seg.end + 1 != alloc[i + 1].start) {
                for (int j = 0; j < free1_count; j++) {
                    if (seg.end + 1 == free1[j].start) {
                        free1[j].start -= seg.size;
                        free1[j].size = free1[j].end - free1[j].start + 1;
                        remove_block(&alloc, &alloc_count, i);
                        break;
                    }
                }
                break;
            }
            // Trường hợp giữa bộ nhớ: hole-hole
            else if (i > 0 && i + 1 < alloc_count && seg.start - 1 != alloc[i - 1].end && seg.end + 1 != alloc[i + 1].start) {
                int before = -1, after = -1;
                for (int j = 0; j < free1_count; j++) {
                    if (seg.start - 1 == free1[j].end) {
                        before = j;
                        break;
                    }
                }
                for (int j = 0; j < free1_count; j++) {
                    if (seg.end + 1 == free1[j].start) {
                        after = j;
                        break;
                    }
                }
                if (before != -1 && after != -1) {
                    free1[before].size += (free1[after].size + seg.size);
                    free1[before].end = free1[after].end;
                    remove_block(&free1, &free1_count, after);
                    remove_block(&alloc, &alloc_count, i);
                }
                break;
            }
        }
    }
}

// Hàm giải phóng toàn bộ tiến trình
static void de_allocate_process(proj p) {
    for (int i = 0; i < p.num_seg; i++) {
        qsort(free1, free1_count, sizeof(block), compareByStart);
        qsort(alloc, alloc_count, sizeof(block), compareByStart);
        de_allocate(p.segs[i]);
    }
}

// Khởi tạo bộ nhớ
void init_memory(int size) {
    mem_size = size;
    free1_count = 0;
    free1_capacity = 0;
    alloc_count = 0;
    alloc_capacity = 0;
    proces_count = 0;
    proces_capacity = 0;
    free(free1);
    free(alloc);
    for (int i = 0; i < proces_count; i++) {
        free(proces[i].segs);
    }
    free(proces);
    free1 = NULL;
    alloc = NULL;
    proces = NULL;

    block neo;
    neo.start = 0;
    neo.size = mem_size;
    neo.end = mem_size - 1;
    strncpy(neo.name, "alloc for system", sizeof(neo.name));
    add_block(&alloc, &alloc_count, &alloc_capacity, neo);
}

// Thêm lỗ trống
void add_hole(int start, int size) {
    block x;
    strncpy(x.name, "hole", sizeof(x.name));
    x.start = start;
    x.size = size;
    x.end = start + size - 1;

    qsort(alloc, alloc_count, sizeof(block), compareByStart);
    qsort(free1, free1_count, sizeof(block), compareByStart);

    if (free1_count == 0) {
        if (start == 0) {
            add_block(&free1, &free1_count, &free1_capacity, x);
            alloc[alloc_count - 1].start = x.end + 1;
            alloc[alloc_count - 1].size = alloc[alloc_count - 1].end - alloc[alloc_count - 1].start + 1;
        } else {
            block alloc_new;
            strncpy(alloc_new.name, "alloc for system", sizeof(alloc_new.name));
            alloc_new.start = alloc[alloc_count - 1].start;
            alloc_new.end = start - 1;
            alloc_new.size = alloc_new.end - alloc_new.start + 1;
            alloc[alloc_count - 1].start = x.end + 1;
            alloc[alloc_count - 1].size = alloc[alloc_count - 1].end - alloc[alloc_count - 1].start + 1;
            add_block(&alloc, &alloc_count, &alloc_capacity, alloc_new);
            add_block(&free1, &free1_count, &free1_capacity, x);
        }
    } else {
        if (free1[free1_count - 1].end + 1 == start) {
            free1[free1_count - 1].end = x.end;
            free1[free1_count - 1].size = free1[free1_count - 1].end - free1[free1_count - 1].start + 1;
            alloc[alloc_count - 1].start = x.end + 1;
            alloc[alloc_count - 1].size = alloc[alloc_count - 1].end - alloc[alloc_count - 1].start + 1;
        } else {
            block alloc_new;
            strncpy(alloc_new.name, "alloc for system", sizeof(alloc_new.name));
            alloc_new.start = alloc[alloc_count - 1].start;
            alloc_new.end = start - 1;
            alloc_new.size = alloc_new.end - alloc_new.start + 1;
            alloc[alloc_count - 1].start = x.end + 1;
            alloc[alloc_count - 1].size = alloc[alloc_count - 1].end - alloc[alloc_count - 1].start + 1;
            add_block(&alloc, &alloc_count, &alloc_capacity, alloc_new);
            add_block(&free1, &free1_count, &free1_capacity, x);
        }
    }
    qsort(alloc, alloc_count, sizeof(block), compareByStart);
    qsort(free1, free1_count, sizeof(block), compareByStart);
}

// Thêm tiến trình
void add_process(const char* name, int num_seg) {
    for (int i = 0; i < proces_count; i++) {
        if (strcmp(proces[i].name, name) == 0) {
            return; // Tránh trùng tên
        }
    }
    if (proces_count >= proces_capacity) {
        proces_capacity = (proces_capacity == 0) ? 10 : proces_capacity * 2;
        proces = (proj*)realloc(proces, proces_capacity * sizeof(proj));
    }
    proces[proces_count] = create_proj(name, num_seg);
    proces_count++;
}

// Thêm segment vào tiến trình
void add_segment_to_process(const char* proc_name, const char* seg_name, int seg_size) {
    for (int i = 0; i < proces_count; i++) {
        if (strcmp(proces[i].name, proc_name) == 0) {
            if (proces[i].segs_count + 1 <= proces[i].num_seg) {
                block new_seg;
                strncpy(new_seg.name, seg_name, sizeof(new_seg.name));
                new_seg.size = seg_size;
                new_seg.start = 0;
                new_seg.end = 0;
                add_block(&proces[i].segs, &proces[i].segs_count, &proces[i].segs_capacity, new_seg);
            }
            break;
        }
    }
}

// Cấp phát tiến trình
void allocate_process(const char* proc_name, const char* algo) {
    for (int i = 0; i < proces_count; i++) {
        if (strcmp(proces[i].name, proc_name) == 0) {
            if (proces[i].num_seg > proces[i].segs_count) {
                break;
            }
            if (strcmp(algo, "first") == 0) {
                first_fit(&proces[i]);
            } else if (strcmp(algo, "best") == 0) {
                best_fit(&proces[i]);
            }
            break;
        }
    }
}

// Giải phóng tiến trình
void deallocate_process(const char* proc_name) {
    for (int i = 0; i < proces_count; i++) {
        if (strcmp(proces[i].name, proc_name) == 0) {
            de_allocate_process(proces[i]);
            free(proces[i].segs);
            for (int j = i; j < proces_count - 1; j++) {
                proces[j] = proces[j + 1];
            }
            proces_count--;
            break;
        }
    }
}

// Giải phóng block
void deallocate_block(int start) {
    for (int i = 0; i < alloc_count; i++) {
        if (alloc[i].start == start && strcmp(alloc[i].name, "alloc for system") == 0) {
            de_allocate(alloc[i]);
            break;
        }
    }
}

// Lấy dữ liệu để hiển thị
int get_free_blocks(block** blocks) {
    *blocks = free1;
    return free1_count;
}

int get_alloc_blocks(block** blocks) {
    *blocks = alloc;
    return alloc_count;
}

int get_processes(proj** processes) {
    *processes = proces;
    return proces_count;
}

// Giải phóng bộ nhớ
void free_memory() {
    for (int i = 0; i < proces_count; i++) {
        free(proces[i].segs);
    }
    free(proces);
    free(alloc);
    free(free1);
    proces = NULL;
    alloc = NULL;
    free1 = NULL;
    proces_count = 0;
    alloc_count = 0;
    free1_count = 0;
}