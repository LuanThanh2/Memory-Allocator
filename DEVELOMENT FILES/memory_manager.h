#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

// Định nghĩa cấu trúc block
typedef struct {
    char name[50]; // Dùng mảng char thay vì std::string trong C
    int size;
    int start;
    int end;
} block;

// Định nghĩa cấu trúc proj (process)
typedef struct {
    char name[50];
    int num_seg;
    int allocated;
    block* segs; // Mảng động thay vì vector
    int segs_count;
    int segs_capacity;
} proj;

// Khai báo các hàm
#ifdef __cplusplus
extern "C" {
#endif

void init_memory(int size);
void add_hole(int start, int size);
void add_process(const char* name, int num_seg);
void add_segment_to_process(const char* proc_name, const char* seg_name, int seg_size);
void allocate_process(const char* proc_name, const char* algo);
void deallocate_process(const char* proc_name);
void deallocate_block(int start);

// Hàm để lấy dữ liệu từ C để hiển thị trên giao diện
int get_free_blocks(block** blocks);
int get_alloc_blocks(block** blocks);
int get_processes(proj** processes);
void free_memory();

#ifdef __cplusplus
}
#endif

#endif // MEMORY_MANAGER_H