ABSTAIN(1) - General Commands Manual

# NAME

**abstain** - execute programs with reduced
pledge(2)
promises

# SYNOPSIS

**abstain&nbsp;\[**-l**]**
\[**-v**&nbsp;*vice\[,vice,...]*]
*binary*&nbsp;\[*flags*&nbsp;*...*]

# DESCRIPTION

The
**abstain**
utility executes
*binary*
with
*flags*
using
pledge(2)
*execpromises*.
By default, it will do so with all possible
*execpromises*
included. Specifying
*vices*
with
**-v**
removes those from the
*execpromises*,
thereby restricting the allowed syscalls for the
*binary*.
Multiple
*vices*
can be specified separated by commas.
The
**-l**
flag prints all possible
*vices*
to the standard output.

**abstain**
always runs with the
'error'
promise.

# EXAMPLES

Execute
*hello\_world*
with
*arguments*,
prohibiting access to the syscalls of the
'wpath'
and
'cpath'
groups
(see
pledge(2)
for details)
:

	$ abstain -v wpath,cpath ./hello_world arguments

# EXIT STATUS

As
**abstain**
calls
execve(2),
it has no process to return to. If any error occurs prior to or when calling
execve(2),
**abstain**
will return -1.

# SEE ALSO

pledge(2)
execve(2)
unveilro(1)

# AUTHORS

Thomas Frohwein &lt;[thfr@openbsd.org](mailto:thfr@openbsd.org)&gt;

# CAVEATS

Some system calls when allowed still have restrictions applied to them. Refer to
pledge(2)
for details.

The default of running with
'error'
promise comes with risk that the program can enter inconsistent state, especially
setuid(2)
or
setgid(2)
programs.

OpenBSD 7.3 - September 4, 2023
