ABSTAIN(1) - General Commands Manual

# NAME

**abstain** - selectively disallow
pledge(2)
promises for program execution

# SYNOPSIS

**abstain**
\[**-lev**]
\[**-p**&nbsp;*promise*\[*,promise,...*]]
\[**-p**&nbsp;*promise*\[*,promise,...*]]
*binary*&nbsp;\[*arguments*&nbsp;*...*]

# DESCRIPTION

**abstain**
executes a
*binary*
with
(optional)
*arguments*
using
pledge(2)
*execpromises*.
Unlike
pledge(2)'s
syntax of specifying arguments for
*addition*,
**abstain**'s
*promises*
are listed for
*subtraction*.
This means that without any
**-p**
*promises*,
the maximal number of
pledge(2)
*promises*
are permitted.
(This doesn't mean that no restrictions are imposed by
pledge(2).)
To disallow
*promises*,
specify them with
**-p**
in a comma-separated list and/or with multiple
**-p**
arguments.

**abstain**
imposes a
pledge(2)
*promise set*
on the application
*as a whole*.
Depending on the nature of the application, it could fail to launch or abort execution at a later stage when a
pledge(2)
violation occurs.

Due to the nature of
*execpromises*,
the restrictions will propagate to children of the application and new processes spawned by
execve(2)
(see the
make(1)
example below)
.

The
'error'
*promise*
is disabled by default, but can be added with
**-e**.

The
**-l**
flag prints all possible
*promises*
to the standard output.

Use
**-v**
to display the command passed to
execvp(3)
and the
*execpromises*.

# EXAMPLES

Prohibit file system modification:

	$ abstain -p wpath,cpath,dpath,fattr,chown binary arguments

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

	$ abstain -p audio,video video
	Abort trap (core dumped)

Use
**-e**
so that the program will receive
`ENOSYS`
instead of
`SIGABRT:`

	$ abstain -ep audio,video video
	video: VIDIOC_QUERYCAP: Function not implemented
	video: ioctl STREAMOFF: Function not implemented

Prohibit network access, for example to assess if a build system truly runs in offline mode:

	$ abstain -p inet make
	curl https://example.com/ -o /tmp/example.com.html
	Abort trap (core dumped)
	*** Error 134 in /tmp (Makefile:2 'all')

And now with
**-e**:

	$ abstain -ep inet make
	curl https://example.com/ -o /tmp/example.com.html
	...
	curl: (7) Failed to connect to example.com port [...]
	*** Error 7 in /tmp (Makefile:2 'all')

If
**abstain**
encounters a less restrictive
pledge(2)
call,
an error is thrown by
pledge(2)
that needs to be handled by the calling program:

	$ abstain -p cpath touch /tmp/test
	touch: pledge: Operation not permitted
	$ file /tmp/test
	/tmp/test: cannot stat '/tmp/test' (No such file or directory)

# EXIT STATUS

As
**abstain**
calls
execvp(3),
it has no process to return to. If any error occurs prior to
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

Due to the nature of
pledge(2)
*execpromises*,
the restrictions are applied to the entire program invoked by
execvp(3),
including future processes spawned by other calls to
execve(2)
and related syscalls. The only way to apply more fine-grained
pledge(2)
is direct source modification of the program in question.

If a program calls
pledge(2)
with
*promises*
prohibited by
**abstain**,
this will lead to an error. Depending on the program's error handling, this could cause an inconsistent state and potentially a failure to invoke a reduction in other
pledge(2)
promises.

Use of
**-e**
can lead to an inconsistent state of the program and therefore should be used judiciously.

OpenBSD 7.3 - September 7, 2023
