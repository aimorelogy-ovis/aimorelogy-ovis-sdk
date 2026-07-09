#ifndef OSAL_ERRNO_H_
#define OSAL_ERRNO_H_

#define	OSAL_EPERM		 (-1)	/* Operation not permitted */
#define	OSAL_ENOENT		 (-2)	/* No such file or directory */
#define	OSAL_ESRCH		 (-3)	/* No such process */
#define	OSAL_EINTR		 (-4)	/* Interrupted system call */
#define	OSAL_EIO		 (-5)	/* I/O error */
#define	OSAL_ENXIO		 (-6)	/* No such device or address */
#define	OSAL_E2BIG		 (-7)	/* Argument list too long */
#define	OSAL_ENOEXEC		 (-8)	/* Exec format error */
#define	OSAL_EBADF		 (-9)	/* Bad file number */
#define	OSAL_ECHILD		(-10)	/* No child processes */
#define	OSAL_EAGAIN		(-11)	/* Try again */
#define	OSAL_ENOMEM		(-12)	/* Out of memory */
#define	OSAL_EACCES		(-13)	/* Permission denied */
#define	OSAL_EFAULT		(-14)	/* Bad address */
#define	OSAL_ENOTBLK		(-15)	/* Block device required */
#define	OSAL_EBUSY		(-16)	/* Device or resource busy */
#define	OSAL_EEXIST		(-17)	/* File exists */
#define	OSAL_EXDEV		(-18)	/* Cross-device link */
#define	OSAL_ENODEV		(-19)	/* No such device */
#define	OSAL_ENOTDIR		(-20)	/* Not a directory */
#define	OSAL_EISDIR		(-21)	/* Is a directory */
#define	OSAL_EINVAL		(-22)	/* Invalid argument */
#define	OSAL_ENFILE		(-23)	/* File table overflow */
#define	OSAL_EMFILE		(-24)	/* Too many open files */
#define	OSAL_ENOTTY		(-25)	/* Not a typewriter */
#define	OSAL_ETXTBSY		(-26)	/* Text file busy */
#define	OSAL_EFBIG		(-27)	/* File too large */
#define	OSAL_ENOSPC		(-28)	/* No space left on device */
#define	OSAL_ESPIPE		(-29)	/* Illegal seek */
#define	OSAL_EROFS		(-30)	/* Read-only file system */
#define	OSAL_EMLINK		(-31)	/* Too many links */
#define	OSAL_EPIPE		(-32)	/* Broken pipe */
#define	OSAL_EDOM		(-33)	/* Math argument out of domain of func */
#define	OSAL_ERANGE		(-34)	/* Math result not representable */

#define	OSAL_EDEADLK		(-35)	/* Resource deadlock would occur */
#define	OSAL_ENAMETOOLONG	(-36)	/* File name too long */
#define	OSAL_ENOLCK		(-37)	/* No record locks available */

#define	OSAL_ENOSYS		(-38)	/* Invalid system call number */

#define	OSAL_ENOTEMPTY		(-39)	/* Directory not empty */
#define	OSAL_ELOOP		(-40)	/* Too many symbolic links encountered */
#define	OSAL_EWOULDBLOCK	OSAL_EAGAIN	/* Operation would block */
#define	OSAL_ENOMSG		(-42)	/* No message of desired type */
#define	OSAL_EIDRM		(-43)	/* Identifier removed */
#define	OSAL_ECHRNG		(-44)	/* Channel number out of range */
#define	OSAL_EL2NSYNC		(-45)	/* Level 2 not synchronized */
#define	OSAL_EL3HLT		(-46)	/* Level 3 halted */
#define	OSAL_EL3RST		(-47)	/* Level 3 reset */
#define	OSAL_ELNRNG		(-48)	/* Link number out of range */
#define	OSAL_EUNATCH		(-49)	/* Protocol driver not attached */
#define	OSAL_ENOCSI		(-50)	/* No CSI structure available */
#define	OSAL_EL2HLT		(-51)	/* Level 2 halted */
#define	OSAL_EBADE		(-52)	/* Invalid exchange */
#define	OSAL_EBADR		(-53)	/* Invalid request descriptor */
#define	OSAL_EXFULL		(-54)	/* Exchange full */
#define	OSAL_ENOANO		(-55)	/* No anode */
#define	OSAL_EBADRQC		(-56)	/* Invalid request code */
#define	OSAL_EBADSLT		(-57)	/* Invalid slot */

#define	OSAL_EDEADLOCK		OSAL_EDEADLK

#define	OSAL_EBFONT		(-59)	/* Bad font file format */
#define	OSAL_ENOSTR		(-60)	/* Device not a stream */
#define	OSAL_ENODATA		(-61)	/* No data available */
#define	OSAL_ETIME		(-62)	/* Timer expired */
#define	OSAL_ENOSR		(-63)	/* Out of streams resources */
#define	OSAL_ENONET		(-64)	/* Machine is not on the network */
#define	OSAL_ENOPKG		(-65)	/* Package not installed */
#define	OSAL_EREMOTE		(-66)	/* Object is remote */
#define	OSAL_ENOLINK		(-67)	/* Link has been severed */
#define	OSAL_EADV		(-68)	/* Advertise error */
#define	OSAL_ESRMNT		(-69)	/* Srmount error */
#define	OSAL_ECOMM		(-70)	/* Communication error on send */
#define	OSAL_EPROTO		(-71)	/* Protocol error */
#define	OSAL_EMULTIHOP		(-72)	/* Multihop attempted */
#define	OSAL_EDOTDOT		(-73)	/* RFS specific error */
#define	OSAL_EBADMSG		(-74)	/* Not a data message */
#define	OSAL_EOVERFLOW		(-75)	/* Value too large for defined data type */
#define	OSAL_ENOTUNIQ		(-76)	/* Name not unique on network */
#define	OSAL_EBADFD		(-77)	/* File descriptor in bad state */
#define	OSAL_EREMCHG		(-78)	/* Remote address changed */
#define	OSAL_ELIBACC		(-79)	/* Can not access a needed shared library */
#define	OSAL_ELIBBAD		(-80)	/* Accessing a corrupted shared library */
#define	OSAL_ELIBSCN		(-81)	/* .lib section in a.out corrupted */
#define	OSAL_ELIBMAX		(-82)	/* Attempting to link in too many shared libraries */
#define	OSAL_ELIBEXEC		(-83)	/* Cannot exec a shared library directly */
#define	OSAL_EILSEQ		(-84)	/* Illegal byte sequence */
#define	OSAL_ERESTART		(-85)	/* Interrupted system call should be restarted */
#define	OSAL_ESTRPIPE		(-86)	/* Streams pipe error */
#define	OSAL_EUSERS		(-87)	/* Too many users */
#define	OSAL_ENOTSOCK		(-88)	/* Socket operation on non-socket */
#define	OSAL_EDESTADDRREQ	(-89)	/* Destination address required */
#define	OSAL_EMSGSIZE		(-90)	/* Message too long */
#define	OSAL_EPROTOTYPE		(-91)	/* Protocol wrong type for socket */
#define	OSAL_ENOPROTOOPT	(-92)	/* Protocol not available */
#define	OSAL_EPROTONOSUPPORT	(-93)	/* Protocol not supported */
#define	OSAL_ESOCKTNOSUPPORT	(-94)	/* Socket type not supported */
#define	OSAL_EOPNOTSUPP		(-95)	/* Operation not supported on transport endpoint */
#define	OSAL_EPFNOSUPPORT	(-96)	/* Protocol family not supported */
#define	OSAL_EAFNOSUPPORT	(-97)	/* Address family not supported by protocol */
#define	OSAL_EADDRINUSE		(-98)	/* Address already in use */
#define	OSAL_EADDRNOTAVAIL	(-99)	/* Cannot assign requested address */
#define	OSAL_ENETDOWN		(-100)	/* Network is down */
#define	OSAL_ENETUNREACH	(-101)	/* Network is unreachable */
#define	OSAL_ENETRESET		(-102)	/* Network dropped connection because of reset */
#define	OSAL_ECONNABORTED	(-103)	/* Software caused connection abort */
#define	OSAL_ECONNRESET		(-104)	/* Connection reset by peer */
#define	OSAL_ENOBUFS		(-105)	/* No buffer space available */
#define	OSAL_EISCONN		(-106)	/* Transport endpoint is already connected */
#define	OSAL_ENOTCONN		(-107)	/* Transport endpoint is not connected */
#define	OSAL_ESHUTDOWN		(-108)	/* Cannot send after transport endpoint shutdown */
#define	OSAL_ETOOMANYREFS	(-109)	/* Too many references: cannot splice */
#define	OSAL_ETIMEDOUT		(-110)	/* Connection timed out */
#define	OSAL_ECONNREFUSED	(-111)	/* Connection refused */
#define	OSAL_EHOSTDOWN		(-112)	/* Host is down */
#define	OSAL_EHOSTUNREACH	(-113)	/* No route to host */
#define	OSAL_EALREADY		(-114)	/* Operation already in progress */
#define	OSAL_EINPROGRESS	(-115)	/* Operation now in progress */
#define	OSAL_ESTALE		(-116)	/* Stale file handle */
#define	OSAL_EUCLEAN		(-117)	/* Structure needs cleaning */
#define	OSAL_ENOTNAM		(-118)	/* Not a XENIX named type file */
#define	OSAL_ENAVAIL		(-119)	/* No XENIX semaphores available */
#define	OSAL_EISNAM		(-120)	/* Is a named type file */
#define	OSAL_EREMOTEIO		(-121)	/* Remote I/O error */
#define	OSAL_EDQUOT		(-122)	/* Quota exceeded */

#define	OSAL_ENOMEDIUM		(-123)	/* No medium found */
#define	OSAL_EMEDIUMTYPE	(-124)	/* Wrong medium type */
#define	OSAL_ECANCELED		(-125)	/* Operation Canceled */
#define	OSAL_ENOKEY		(-126)	/* Required key not available */
#define	OSAL_EKEYEXPIRED	(-127)	/* Key has expired */
#define	OSAL_EKEYREVOKED	(-128)	/* Key has been revoked */
#define	OSAL_EKEYREJECTED	(-129)	/* Key was rejected by service */

/* for rOSAL_obust mutexes */
#define	OSAL_EOWNERDEAD		(-130)	/* Owner died */
#define	OSAL_ENOTRECOVERABLE	(-131)	/* State not recoverable */

#define OSAL_ERFKILL		(-132)	/* Operation not possible due to RF-kill */

#define OSAL_EHWPOISON		(-133)	/* Memory page has hardware error */

#define OSAL_ERESTARTSYS	(-512)
#define OSAL_ERESTARTNOINTR	(-513)
#define OSAL_ERESTARTNOHAND	(-514)	/* restart if no handler.. */
#define OSAL_ENOIOCTLCMD	(-515)	/* No ioctl command */
#define OSAL_ERESTART_RESTARTBLOCK (-516) /* restart by calling sys_restart_syscall */
#define OSAL_EPROBE_DEFER	(-517)	/* Driver requests probe retry */
#define OSAL_EOPENSTALE		(-518)	/* open found a stale dentry */

/* DefinOSAL_ed for the NFSv3 protocol */
#define OSAL_EBADHANDLE		(-521)	/* Illegal NFS file handle */
#define OSAL_ENOTSYNC		(-522)	/* Update synchronization mismatch */
#define OSAL_EBADCOOKIE		(-523)	/* Cookie is stale */
#define OSAL_ENOTSUPP		(-524)	/* Operation is not supported */
#define OSAL_ETOOSMALL		(-525)	/* Buffer or request is too small */
#define OSAL_ESERVERFAULT	(-526)	/* An untranslatable error occurred */
#define OSAL_EBADTYPE		(-527)	/* Type not supported by server */
#define OSAL_EJUKEBOX		(-528)	/* Request initiated, but will not complete before timeout */
#define OSAL_EIOCBQUEUED	(-529)	/* iocb queued, will get completion event */
#define OSAL_ERECALLCONFLICT	(-530)	/* conflict with recalled state */

#define OSAL_NONEOS_DEFAULT_RET (-200) /* NoneOS's default return value */
#ifndef EOK
#define EOK 0
#endif

#endif /* OSAL_ERRNO_H_ */
