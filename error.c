#include <errno.h>
#include <sqlite3.h>

char *errno_str(void) {
	switch (errno) {
	case EPERM:
		return "operation not permitted";
	case ENOENT:
		return "no such file or directory";
	case ESRCH:
		return "no such process";
	case EINTR:
		return "interrupted system call";
	case EIO:
		return "input output error";
	case ENXIO:
		return "device not configured";
	case E2BIG:
		return "argument list too long";
	case ENOEXEC:
		return "exec format error";
	case EBADF:
		return "bad file descriptor";
	case ECHILD:
		return "no child processes";
	case EDEADLK:
		return "resource deadlock avoided";
	case ENOMEM:
		return "cannot allocate memory";
	case EACCES:
		return "permission denied";
	case EFAULT:
		return "bad address";
	case ENOTBLK:
		return "block device required";
	case EBUSY:
		return "device resource busy";
	case EEXIST:
		return "file exists";
	case EXDEV:
		return "cross device link";
	case ENODEV:
		return "operation not supported by device";
	case ENOTDIR:
		return "not a directory";
	case EISDIR:
		return "is a directory";
	case EINVAL:
		return "invalid argument";
	case ENFILE:
		return "too many open files in system";
	case EMFILE:
		return "too many open files";
	case ENOTTY:
		return "inappropriate ioctl for device";
	case ETXTBSY:
		return "text file busy";
	case EFBIG:
		return "file too large";
	case ENOSPC:
		return "no space left on device";
	case ESPIPE:
		return "illegal seek";
	case EROFS:
		return "read only file system";
	case EMLINK:
		return "too many links";
	case EPIPE:
		return "broken pipe";
	case EDOM:
		return "numerical argument out of domain";
	case ERANGE:
		return "result too large";
	case EAGAIN:
		return "resource temporarily unavailable";
	case EINPROGRESS:
		return "operation now in progress";
	case EALREADY:
		return "operation already in progress";
	case ENOTSOCK:
		return "socket operation on non socket";
	case EDESTADDRREQ:
		return "destination address required";
	case EMSGSIZE:
		return "message too long";
	case EPROTOTYPE:
		return "protocol wrong type for socket";
	case ENOPROTOOPT:
		return "protocol not available";
	case EPROTONOSUPPORT:
		return "protocol not supported";
	case ESOCKTNOSUPPORT:
		return "socket type not supported";
	case ENOTSUP:
		return "operation not supported";
	case EPFNOSUPPORT:
		return "protocol family not supported";
	case EAFNOSUPPORT:
		return "address family not supported by protocol family";
	case EADDRINUSE:
		return "address already in use";
	case EADDRNOTAVAIL:
		return "can not assign requested address";
	case ENETDOWN:
		return "network is down";
	case ENETUNREACH:
		return "network is unreachable";
	case ENETRESET:
		return "network dropped connection on reset";
	case ECONNABORTED:
		return "software caused connection abort";
	case ECONNRESET:
		return "connection reset by peer";
	case ENOBUFS:
		return "no buffer space available";
	case EISCONN:
		return "socket is already connected";
	case ENOTCONN:
		return "socket is not connected";
	case ESHUTDOWN:
		return "can not send after socket shutdown";
	case ETOOMANYREFS:
		return "too many references can not splice";
	case ETIMEDOUT:
		return "operation timed out";
	case ECONNREFUSED:
		return "connection refused";
	case ELOOP:
		return "too many levels of symbolic links";
	case ENAMETOOLONG:
		return "file name too long";
	case EHOSTDOWN:
		return "host is down";
	case EHOSTUNREACH:
		return "no route to host";
	case ENOTEMPTY:
		return "directory not empty";
	case EPROCLIM:
		return "too many processes";
	case EUSERS:
		return "too many users";
	case EDQUOT:
		return "disc quota exceeded";
	default:
		return "???";
	}
}
