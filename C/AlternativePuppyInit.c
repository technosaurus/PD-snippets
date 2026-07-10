//untested scratch work, do not use
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define STATE_VESA 0
#define STATE_XORG 1
#define STATE_EXIT 2

/* Minimal global config extracted from kernel arguments */
char pmedia_val[64]  = {0};
char psubdir_val[64] = {0};
int  pfix_ram        = 0;
int  pfix_nox        = 0;

/* Helper to safely compare strings without pulling in large libc runtime routines */
int micro_strncmp(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++; s2++; n--;
    }
    if (n == 0) return 0;
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

/* Parse raw parameters sent by the kernel via bootloader (envp & argv) */
void parse_kernel_arguments(int argc, char *argv[], char *envp[]) {
    int i;
    
    // 1. Scan the environment pointers (Variables containing '=')
    for (i = 0; envp[i] != NULL; i++) {
        if (micro_strncmp(envp[i], "pmedia=", 7) == 0) {
            size_t len = 0;
            char *val = envp[i] + 7;
            while (val[len] && len < 63) { pmedia_val[len] = val[len]; len++; }
            pmedia_val[len] = '\0';
        }
        if (micro_strncmp(envp[i], "psubdir=", 8) == 0) {
            size_t len = 0;
            char *val = envp[i] + 8;
            while (val[len] && len < 63) { psubdir_val[len] = val[len]; len++; }
            psubdir_val[len] = '\0';
        }
    }

    // 2. Scan the direct argument array flags
    for (i = 1; i < argc; i++) {
        if (micro_strncmp(argv[i], "pfix=ram", 8) == 0) {
            pfix_ram = 1;
        }
        if (micro_strncmp(argv[i], "pfix=nox", 8) == 0) {
            pfix_nox = 1;
        }
    }
}

/* Non-blocking child process auditor to reap background zombies */
void audit_background_workers() {
    int status;
    // -1 checks any child; WNOHANG prevents the supervisor from blocking/stalling
    while (syscall(__NR_wait4, -1, &status, WNOHANG, NULL) > 0) {
        // Child reaped smoothly by the kernel
    }
}

/* Concurrent mounting routine executed inside a separate background thread */
void execute_background_mount_pipeline() {
    // 1. Mount physical drive (Adjust sda1 or use pmedia target details)
    syscall(__NR_mount, "/dev/sda1", "/mnt/disk", "ext4", 0, NULL);

    // 2. Loop mount the core read-only regular Puppy Linux SquashFS file
    // (Note: Assumes your base file layout houses an abstracted loop mounter 
    // or targets a directory mount path like Technosaurus handles natively)
    
    // 3. Mount persistent compressed Btrfs image if 'pfix=ram' was not issued
    if (!pfix_ram) {
        syscall(__NR_mount, "/mnt/disk/puppy_save.img", "/mnt/btrfs_rw", "btrfs", 0, "compress=zstd");
    }

    // 4. Assemble OverlayFS layers (Exposing Puppy arrays right on top of Nanosaurus base)
    syscall(__NR_mount, "overlay", "/usr", "overlay", 0, 
            "lowerdir=/mnt/puppy_ro/usr:/usr,upperdir=/mnt/btrfs_rw/upper,workdir=/mnt/btrfs_rw/work");
            
    syscall(__NR_mount, "overlay", "/root", "overlay", 0, 
            "lowerdir=/mnt/puppy_ro/root:/root,upperdir=/mnt/btrfs_rw/upper_root,workdir=/mnt/btrfs_rw/work_root");

    // 5. Handshake: Touch a signal file to alert the running UI that files are ready
    int fd = syscall(__NR_openat, AT_FDCWD, "/tmp/mounts_complete", O_CREAT | O_WRONLY, 0644);
    if (fd >= 0) {
        syscall(__NR_close, fd);
    }
}

int main(int argc, char *argv[], char *envp[]) {
    // 1. Early core OS nodes initialization (Required before doing anything)
    syscall(__NR_mount, "proc", "/proc", "proc", 0, NULL);
    syscall(__NR_mount, "sysfs", "/sys", "sysfs", 0, NULL);
    syscall(__NR_mount, "tmpfs", "/tmp", "tmpfs", 0, NULL);
    syscall(__NR_mount, "devtmpfs", "/dev", "devtmpfs", 0, NULL);

    // 2. Process Puppy Linux parameters passed into cmdline
    parse_kernel_arguments(argc, argv, envp);

    // 3. Command Line Override check: Did user ask for no GUI?
    if (pfix_nox) {
        // Fallback to basic busybox shell and bypass graphical supervisor completely
        char *shell_args[] = {"/bin/sh", NULL};
        syscall(__NR_execve, "/bin/sh", shell_args, envp);
        return 0; 
    }

    // 4. Fork the heavy disk/mount calculations to a parallel runtime process
    pid_t mount_pid = syscall(__NR_clone, SIGCHLD, 0, NULL, NULL, 0);
    if (mount_pid == 0) {
        execute_background_mount_pipeline();
        syscall(__NR_exit, 0); // Terminate background child process when done
    }

    // 5. State Machine: Initialize sub-second Xvesa desktop mode
    int current_state = STATE_VESA;

    while (current_state != STATE_EXIT) {
        pid_t x_server_pid = syscall(__NR_clone, SIGCHLD, 0, NULL, NULL, 0);

        if (x_server_pid == 0) {
            // --- CHILD PROCESS: Fire up Graphical Engines ---
            if (current_state == STATE_VESA) {
                // Instantly spin up sub-second minimal graphics layer
                char *vesa_args[] = {"/usr/bin/xinit", "/etc/X11/xinit/nanosaurus.xinitrc", "--", "/usr/bin/Xvesa", "-screen", "1024x768x16", ":0", NULL};
                syscall(__NR_execve, "/usr/bin/xinit", vesa_args, envp);
            } 
            else if (current_state == STATE_XORG) {
                // Background overlay structures are guaranteed bound and visible by now
                char *xorg_args[] = {"/usr/bin/xinit", "/root/.xinitrc", "--", "/usr/bin/Xorg", ":0", NULL};
                syscall(__NR_execve, "/usr/bin/xinit", xorg_args, envp);
            }
            syscall(__NR_exit, 1); // Safety exit if execution layout failure happens
        }

        // --- PARENT PROCESS (PID 1 SUPERVISOR): Maintain Active Uptime ---
        while (1) {
            int status;
            
            // Non-blocking check: Has the active X server died/exited?
            pid_t check_x = syscall(__NR_wait4, x_server_pid, &status, WNOHANG, NULL);
            if (check_x == x_server_pid) {
                break; // Window manager session closed! Break loop to evaluate transition.
            }

            // Non-blocking check: Sweep for any background worker zombies (like the mount process)
            audit_background_workers();

            // Native micro-sleep (20ms) to ensure CPU cores keep running cool
            struct timespec ts;
            ts.tv_sec = 0;
            ts.tv_nsec = 20000000;
            syscall(__NR_nanosleep, &ts, NULL);
        }

        // 6. Transition Analysis: Evaluate why the graphical server dropped
        struct stat st;
        // Check if the dialog prompt inside the UI dropped an acceleration switch file
        if (syscall(__NR_newfstatat, AT_FDCWD, "/tmp/graphics_mode_choice", &st, 0) == 0) {
            current_state = STATE_XORG;
            syscall(__NR_unlinkat, AT_FDCWD, "/tmp/graphics_mode_choice", 0); // Clear state token file
        } else {
            // Normal desktop exit or total session shutdown requested
            current_state = STATE_EXIT;
        }
    }

    // 7. Clean Machine Halt Safety Net
    char *poweroff_args[] = {"/bin/poweroff", NULL};
    syscall(__NR_execve, "/bin/poweroff", poweroff_args, envp);

    return 0;
}
