struct regs {
	int     eax;
	int     ebx;
	int     ecx;
	int     edx;

	int     esi;
	int     edi;
	int     ebp;

	// We still want an IRET frame.
	// Syscall depends on it. IRET will be used to get back in.
	// ESP however does not get autosaved, that we have to deal with later.
	int;
};

//
// Linux has a task structure which it uses for thread and processes
// or something. This does not really matter here.
//
// clone and clone3 are used to create new threads.
//
// This is stored in a std::vector-style reallocating array. PID is the index.
// Threads are valid processes so they get a PID.
//
struct thread {
	struct regs *   regs;
	int             fpu_context[108/4];
	struct thread * next, prev;
};

struct proc {
	int stdio_fds[3];
};

struct thread *lib_sched_get_current_thread_r();

// void lib_sched_get_
