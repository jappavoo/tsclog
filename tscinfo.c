// from: https://stackoverflow.com/questions/35123379/getting-tsc-rate-from-x86-kernel

#include <asm/unistd.h>
#include <inttypes.h>
#include <linux/perf_event.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

static uint64_t mul_u64_u32_shr(uint64_t cyc, uint32_t mult, uint32_t shift)
{
    __uint128_t x = cyc;
    x *= mult;
    x >>= shift;
    return x;
}

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
           int cpu, int group_fd, unsigned long flags)
{
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

int main(int argc, char **argv)
{
    struct perf_event_attr pe = {
        .type = PERF_TYPE_HARDWARE,
        .size = sizeof(struct perf_event_attr),
        .config = PERF_COUNT_HW_INSTRUCTIONS,
        .disabled = 1,
        .exclude_kernel = 1,
        .exclude_hv = 1
    };
    int fd = perf_event_open(&pe, 0, -1, -1, 0);
    int x=1000;
    if (argc > 1) { x=atoi(argv[1]); }
    
    if (fd == -1) {
        perror("perf_event_open failed");
        return 1;
    }
    void *addr = mmap(NULL, 4*1024, PROT_READ, MAP_SHARED, fd, 0);
    if (!addr) {
        perror("mmap failed");
        return 1;
    }
    struct perf_event_mmap_page *pc = addr;
    if (pc->cap_user_time != 1) {
        fprintf(stderr, "Perf system doesn't support user time\n");
        return 1;
    }
    printf("%16s   %5s\n", "mult", "shift");
    printf("%16" PRIu32 "   %5" PRIu16 "\n", pc->time_mult, pc->time_shift);
    printf("tsc->ns: %" PRIu64 "\n", mul_u64_u32_shr(x, pc->time_mult, pc->time_shift));
    close(fd);
}
