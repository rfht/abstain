ABSTAIN(1) - General Commands Manual

# NAME

**abstain** - selectively disallow
pledge(2)
promises for program execution

# SYNOPSIS

**abstain**
\[**-le**]
\[**-v**&nbsp;*vice*\[*,vice,...*]]
*binary*&nbsp;\[*arguments*&nbsp;*...*]

# DESCRIPTION

The
**abstain**
utility executes
*binary*
with
*arguments*
using
pledge(2)
*execpromises*.
By default, it will do so with all
*execpromises*
allowed.

Specify
*vices*
with
**-v**
in a comma-separated list.
They represent the exact same concept as
*promises*
in
pledge(2),
except that their effect is the reverse - they
prohibit
access to syscalls by removing the corresponding
pledge(2)
*promise*
from the
*execpromises*.

The
**-e**
flag adds the
'error'
*promise*
(which is not included by default)
so that a violation will lead to
`ENOSYS`
instead of
`SIGABRT`.

The
**-l**
flag prints all possible
*vices*
to the standard output.

The main use case that
**abstain**
is designed for is to gather empirical behavioral data on software for specific types of syscalls, building on
pledge(2)
*promise*
sets. Ideally, this is part of a comprehensive investigation that includes source code review and a study of kernel trace logs
(see
ktrace(1))
.

# EXAMPLES

Prohibit file system modification:

	$ abstain -v wpath,cpath,dpath,fattr,chown binary arguments

If you run with
*kern.audio.record*
and/or
*kern.video.record*
enabled
(see
sysctl(8),
not recommended as default)
,
you can selectively prohibit audio/video access, here with
video(1):

	$ abstain -v audio,video video
	...
	Abort trap (core dumped)

Use
**-e**
so that the program will receive
`ENOSYS`
instead of
`SIGABRT:`

	$ abstain -ev audio,video video
	...
	video: VIDIOC_QUERYCAP: Function not implemented
	video: ioctl STREAMOFF: Function not implemented

Prohibit network access, for example to assess if a build system truly runs in offline mode:

	$ abstain -v inet make
	...
	curl https://example.com/ -o /tmp/example.com.html
	Abort trap (core dumped)
	*** Error 134 in /tmp (Makefile:2 'all')

Same with
**-e**:

	$ abstain -ev inet make
	...
	curl https://example.com/ -o /tmp/example.com.html
	...
	curl: (7) Failed to connect to example.com port 443 after 23 ms: Couldn't connect to server
	*** Error 7 in /tmp (Makefile:2 'all')

Using
**abstain**
with a program that already calls
pledge(2)
returns an error:

	$ abstain -v cpath touch /tmp/test
	...
	touch: pledge: Operation not permitted
	$ file /tmp/test
	/tmp/test: cannot stat '/tmp/test' (No such file or directory)

It's the program's responsibility to handle
pledge(2)
errors.

# EXIT STATUS

As
**abstain**
calls
execvp(3),
it has no process to return to. If any error occurs prior to or when calling
execvp(3),
**abstain**
will return -1.

# SEE ALSO

pledge(2)
execvp(3)
unveilro(1)

# AUTHORS

Thomas Frohwein &lt;[thfr@openbsd.org](mailto:thfr@openbsd.org)&gt;

# CAVEATS

Avoid relying on
**abstain**
to restrain your programs. The use of
pledge(2)
modification inside the program source is a superior option wherever feasible. The intended use cases are investigation of external program behavior as part of a development process to add more robust restriction with
pledge(2),
or educational to understand the breadth of syscall use.

The usefulness for programs that already use
pledge(2)
is very limited. At best, earlier uses of
pledge(2)
can be identified. If
**abstain**
removes promises that the program uses, this will lead to an error. Depending on the program's error handling, this could cause an inconsistent state or potentially a failure to invoke a reduction in other
pledge(2)
promises.

Some system calls when allowed still have restrictions applied to them. Refer to
pledge(2)
for details.

Programs violating their
pledge(2)
restrictions by invoking syscalls from one of the
*vices*
are killed with uncatchable
`SIGABRT`.

When running with
**-e**,
the
'error'
*promise*
comes with risk that the program can enter inconsistent state, especially
setuid(2)
or
setgid(2)
programs.

OpenBSD 7.3 - September 5, 2023
