//Example for booting with a remote root filesystem (like a puppy sfs file
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sched.h>

void execute_background_fuse() {
    pid_t pid = fork();
    if (pid == 0) {
        setsid();
        char *args[] = {"/httpfs2", "-f", "http://your-server.com", "/mnt/http", NULL};
        execv(args, args);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // 1. Isolate the mount namespace for system safety
    unshare(CLONE_NEWNS);
    mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL);

    // 2. Start the FUSE streaming network engine
    execute_background_fuse();
    sleep(1); // Give kernel a brief moment to map network interfaces

    // 3. Connect the streamed EROFS file to the local kernel loop driver
    if (mount("/mnt/http/rootfs.erofs", "/mnt/erofs", "erofs", MS_RDONLY, "loop") < 0) {
        perror("EROFS stream target mount failed");
        return 1;
    }

    // 4. Attach standard kernel pseudo-filesystems inside the sysroot template
    mount("proc", "/sysroot/proc", "proc", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL);
    mount("sysfs", "/sysroot/sys", "sys", MS_NOSUID | MS_NOEXEC | MS_NODEV, NULL);
    mount("devtmpfs", "/sysroot/dev", "devtmpfs", MS_NOSUID | MS_NOEXEC, NULL);

    // 5. One-line OverlayFS Composition using native rootfs RAM space directly
    if (mount("overlay", "/sysroot", "overlay", 0, "lowerdir=/mnt/erofs,upperdir=/upper,workdir=/work") < 0) {
        perror("OverlayFS assembly failed");
        return 1;
    }

    // 6. Transition userspace smoothly into the clean sandbox
    chroot("/sysroot");
    chdir("/");

    // 7. Boot the actual OS initialization manager
    execv("/sbin/init", argv);

    perror("Handoff to permanent system init failed");
    return 1;
}
