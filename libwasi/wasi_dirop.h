struct path_info {
        char *hostpath;
        char *wasmpath;
};

struct timeval;
struct stat;

int wasi_host_open(const struct path_info *pi, int oflags, unsigned int mode);
int wasi_host_unlink(const struct path_info *pi);
int wasi_host_mkdir(const struct path_info *pi);
int wasi_host_rmdir(const struct path_info *pi);
int wasi_host_symlink(const char *target_buf, const struct path_info *pi);
int wasi_host_readlink(const struct path_info *pi, char *buf, size_t buflen);
int wasi_host_link(const struct path_info *pi1, const struct path_info *pi2);
int wasi_host_rename(const struct path_info *pi1, const struct path_info *pi2);
int wasi_host_stat(const struct path_info *pi, struct stat *stp);
int wasi_host_lstat(const struct path_info *pi, struct stat *stp);
int wasi_host_utimes(const struct path_info *pi, const struct timeval *tvp);
int wasi_host_lutimes(const struct path_info *pi, const struct timeval *tvp);