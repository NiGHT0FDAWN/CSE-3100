/*
 * TOPICS COVERED IN ONE PROGRAM:
 *
 * - Threads               (pthread_create, pthread_join)
 * - Resource sharing       (global variables accessed by multiple threads)
 * - Mutex                 (pthread_mutex_lock / unlock)
 * - Condition variables   (pthread_cond_wait / signal)
 * - Synchronization       (mutex + condvar to coordinate server/client startup;
 *                           mutex + condvar for a reporting thread)
 * - Sockets               (socket, bind, listen, accept, connect, send, recv)
 * - Client and Server     (a TCP echo server and a TCP client)
 * - Protocols             (TCP via SOCK_STREAM)
 *
 * HOW TO COMPILE & RUN:
 *    gcc -pthread -o all_in_one all_in_one.c
 *    ./all_in_one
 *
 * The server binds to port 0 (OS picks a free port) and signals the client
 * thread when the port is known. The client sends 10 messages, each one
 * echoed back. A reporter thread prints the total messages processed so far
 * every 5 messages. All shared data is protected by mutexes and condition
 * variables.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>   // for inet_pton

/* ------------------------------------------------------------------ */
/*  Shared resources and synchronisation primitives                    */
/* ------------------------------------------------------------------ */

/* ----- For server / client startup handshake ----- */
pthread_mutex_t startup_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  startup_cond  = PTHREAD_COND_INITIALIZER;
int server_port = 0;          // the actual port the server is listening on
int server_ready = 0;         // flag: 0 = not ready, 1 = ready

/* ----- For the global message counter (shared resource) ----- */
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  counter_cond  = PTHREAD_COND_INITIALIZER;
int total_messages = 0;        // number of messages processed by the server

/* ----- For the reporter thread ----- */
int last_printed_multiple = 0; // reporter remembers the last printed multiple of 5

/* ------------------------------------------------------------------ */
/*  Worker thread: handles one client connection (echo server)        */
/* ------------------------------------------------------------------ */
void *worker_thread(void *arg) {
    int client_fd = *(int *)arg;
    free(arg);                     // we allocated the fd on the heap
    char buffer[1024];
    ssize_t n;

    /* Receive data until client closes or an error occurs */
    while ((n = recv(client_fd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[n] = '\0';

        /* --- Mutex: protect the shared counter --- */
        pthread_mutex_lock(&counter_mutex);
        total_messages++;

        /* If we just hit a multiple of 5, wake up the reporter thread */
        if (total_messages % 5 == 0) {
            pthread_cond_signal(&counter_cond);  // wake one reporter
        }
        pthread_mutex_unlock(&counter_mutex);
        /* --- end of critical section --- */

        /* Echo the data back to the client */
        send(client_fd, buffer, n, 0);
    }

    close(client_fd);
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Reporter thread: prints a summary every 5 messages                */
/* ------------------------------------------------------------------ */
void *reporter_thread(void *arg) {
    (void)arg;
    while (1) {
        pthread_mutex_lock(&counter_mutex);

        /*
         * Condition variable wait:
         *   - atomically unlocks the mutex and blocks
         *   - re-acquires the mutex before returning
         *   - we use a while loop to guard against spurious wakeups
         */
        while (total_messages / 5 <= last_printed_multiple) {
            /*
             * The predicate "total_messages / 5 > last_printed_multiple"
             * is not yet true, so we wait. The worker signals this
             * condition variable whenever total_messages % 5 == 0.
             */
            pthread_cond_wait(&counter_cond, &counter_mutex);
        }

        /* We are now in a new "multiple of 5" region */
        last_printed_multiple = total_messages / 5;
        printf("[REPORTER] Total messages processed so far: %d\n",
               total_messages);

        pthread_mutex_unlock(&counter_mutex);

        /* Stop when we have seen 10 messages (2 * 5) */
        if (total_messages >= 10)
            break;
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Server thread: binds, listens, accepts and spawns workers         */
/* ------------------------------------------------------------------ */
void *server_thread(void *arg) {
    (void)arg;
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    /* ----- SOCKET ----- */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);   // TCP socket (SOCK_STREAM)
    if (server_fd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* ----- BIND ----- */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;   // listen on all interfaces
    server_addr.sin_port        = htons(0);     // let OS choose a free port

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* ----- Get the actual port assigned by the OS ----- */
    socklen_t addr_len = sizeof(server_addr);
    if (getsockname(server_fd, (struct sockaddr *)&server_addr, &addr_len) < 0) {
        perror("getsockname");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    int actual_port = ntohs(server_addr.sin_port);

    /* ----- Notify the client thread that the server is ready ----- */
    /*
     * This is a classic use of mutex + condition variable for
     * synchronisation between threads. The client thread is waiting
     * for server_ready to become 1.
     */
    pthread_mutex_lock(&startup_mutex);
    server_port = actual_port;
    server_ready = 1;
    pthread_cond_signal(&startup_cond);   // wake up the client thread
    pthread_mutex_unlock(&startup_mutex);

    printf("[SERVER] Listening on port %d\n", actual_port);

    /* ----- LISTEN ----- */
    if (listen(server_fd, 1) < 0) {   // backlog = 1 (only one client expected)
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* ----- ACCEPT (blocking call) ----- */
    int *client_fd = malloc(sizeof(int));   // will be freed by worker
    *client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (*client_fd < 0) {
        perror("accept");
        free(client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Client connected\n");

    /* ----- Create worker thread to handle the client ----- */
    pthread_t worker;
    if (pthread_create(&worker, NULL, worker_thread, client_fd) != 0) {
        perror("pthread_create (worker)");
        free(client_fd);
        close(*client_fd);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Wait for the worker to finish before closing the server socket */
    pthread_join(worker, NULL);

    close(server_fd);
    printf("[SERVER] Shutting down.\n");
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Client thread: connects to the server and sends messages          */
/* ------------------------------------------------------------------ */
void *client_thread(void *arg) {
    (void)arg;

    /* ----- Wait until the server is ready (synchronisation) ----- */
    pthread_mutex_lock(&startup_mutex);
    while (!server_ready) {
        /*
         * The server thread will set server_ready = 1, store the port,
         * and signal the condition variable.
         */
        pthread_cond_wait(&startup_cond, &startup_mutex);
    }
    int port = server_port;   // copy the port out of the shared variable
    pthread_mutex_unlock(&startup_mutex);

    printf("[CLIENT] Server is ready on port %d. Connecting...\n", port);

    /* ----- SOCKET ----- */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("client socket");
        exit(EXIT_FAILURE);
    }

    /* ----- CONNECT ----- */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("[CLIENT] Connected. Sending %d messages...\n", 10);

    /* ----- Send messages and receive echoes ----- */
    char send_buf[50];
    char recv_buf[50];
    for (int i = 1; i <= 10; i++) {
        snprintf(send_buf, sizeof(send_buf), "Message #%d\n", i);

        /* send() returns the number of bytes sent; we send the whole string */
        if (send(sock, send_buf, strlen(send_buf), 0) < 0) {
            perror("send");
            break;
        }

        /* Receive the echo */
        ssize_t n = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
        if (n <= 0) {
            if (n == 0)
                printf("[CLIENT] Server closed connection.\n");
            else
                perror("recv");
            break;
        }
        recv_buf[n] = '\0';
        printf("[CLIENT] Echo received: %s", recv_buf);
    }

    close(sock);
    printf("[CLIENT] Finished.\n");
    return NULL;
}

/* ------------------------------------------------------------------ */
/*  Main: orchestrates all threads                                    */
/* ------------------------------------------------------------------ */
int main(void) {
    pthread_t server_tid, client_tid, reporter_tid;

    /* Start the reporter thread early (it will wait on the condition) */
    if (pthread_create(&reporter_tid, NULL, reporter_thread, NULL) != 0) {
        perror("pthread_create reporter");
        return EXIT_FAILURE;
    }

    /* Start the server thread (it will bind and signal the client) */
    if (pthread_create(&server_tid, NULL, server_thread, NULL) != 0) {
        perror("pthread_create server");
        return EXIT_FAILURE;
    }

    /* Start the client thread (it will wait for the server to be ready) */
    if (pthread_create(&client_tid, NULL, client_thread, NULL) != 0) {
        perror("pthread_create client");
        return EXIT_FAILURE;
    }

    /* Wait for all threads to finish */
    pthread_join(server_tid, NULL);
    pthread_join(client_tid, NULL);
    pthread_join(reporter_tid, NULL);

    /* Clean up mutexes and condition variables (good practice) */
    pthread_mutex_destroy(&startup_mutex);
    pthread_cond_destroy(&startup_cond);
    pthread_mutex_destroy(&counter_mutex);
    pthread_cond_destroy(&counter_cond);

    printf("[MAIN] All threads finished. Program exiting.\n");
    return 0;
}

// Here’s an in‑depth look at how these building blocks actually work under the hood, focusing on typical Linux / POSIX implementations (NPTL threads, futex‑based locks, and the kernel’s networking stack).
// 1. Threads – How They Work Internally
// Kernel threads, not just userspace

// In modern Linux with NPTL (Native POSIX Thread Library), every pthread_create calls clone() with flags that make the new thread a schedulable entity managed directly by the kernel. The result is a 1:1 thread model: one kernel task (a task_struct) per pthread.

// The clone flags are crucial:

//     CLONE_VM – share the same virtual memory address space (page tables uncopied). All threads see the same memory map.

//     CLONE_FILES, CLONE_FS, CLONE_SIGHAND, etc. – share file descriptor table, filesystem root/working directory, signal handlers.

//     No CLONE_VM would create a separate process (like fork). Threads share everything except:

//         Thread ID (PID, actually TID)

//         Registers (each thread has its own CPU context)

//         Stack (a separate memory region, allocated by the library)

//         Signal mask

//         Errno (often via thread‑local storage)

//         Some scheduling parameters

// The kernel’s scheduler treats threads just like processes: each has its own task_struct and competes for CPU time slices (timeslices) via the Completely Fair Scheduler (CFS), with equal weight unless priorities are set. A context switch between threads of the same process is cheaper than between processes because the memory map (TLB) doesn’t need to be flushed entirely – the page table root (pgd) stays the same (shared), so only minimal TLB shoot‑downs are needed. However, registers, stack pointer, and kernel stack are swapped.
// Thread creation – the full path

//     pthread_create allocates stack memory (default 8 MB) and sets up a struct containing the start function and argument.

//     It calls clone(CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, ...).

//     Inside the kernel, a new task_struct is created, page tables are copied but then shared because of CLONE_VM. The thread gets a unique TID (returned to userspace as the pid_t from clone).

//     CLONE_SETTLS installs a new Thread‑Local Storage (TLS) segment; the gs/fs register on x86‑64 is set to point to the thread‑control block (TCB), which contains things like the thread pointer for __thread variables and the location of errno.

//     The new task is marked as runnable and eventually scheduled. The parent thread returns from clone with the new TID. The child’s execution begins in a small assembly trampoline that calls the user’s thread function.

// Switching & scheduling

// Each thread has its own kernel stack (8 KB typically) and its own thread_struct holding saved registers (RIP, RSP, FPU state). When a timer interrupt or preemption occurs, the current thread’s registers are saved onto the kernel stack, the scheduler selects another task, and its state is restored. Because the address space is shared, the page tables don’t change; the CPU’s CR3 register remains the same, avoiding the need to flush the Translation‑Lookaside Buffer (TLB) completely, which is the main performance benefit over process context switches.


// 2. Resource Sharing – The Hardware Reality
// Shared address space = shared everything

// All global variables, static variables, malloc‑ed heap, and mapped files are at identical virtual addresses in every thread. A write by one thread is immediately visible to others … in theory. In practice, hardware caches and store buffers can delay visibility unless proper ordering is enforced.
// Race conditions in silicon

// A simple counter++ in C compiles to three instructions:
// asm

// mov    eax, [counter]
// inc    eax
// mov    [counter], eax

// If two threads execute this concurrently:

//     Thread A loads counter (say, 0) into a register.

//     Timer interrupt, context switch to Thread B.

//     Thread B loads 0, increments, stores back 1.

//     Thread A resumes (still has 0 in register), increments to 1, stores back 1.

// The result is 1 instead of 2 – the classic “lost update” race. On a multi‑core machine, this can happen even without a context switch because both cores may load the same value from their private L1 caches.
// Memory ordering and cache coherence

// Caches are kept coherent by the MESI protocol, so eventually a write from one core becomes visible to another. But without memory barriers, a thread may see operations in a different order than they were performed. Compiler optimisations can reorder loads/stores, and the CPU’s out‑of‑order execution can do the same. This is why locks and atomic operations include memory barriers: they enforce a happens‑before relationship. Atomic operations (like lock cmpxchg on x86) implicitly flush the store buffer and force ordering.


// 3. Mutex – The Futex‑Based Reality

// A pthread_mutex_t is not just a simple spin‑lock. It’s designed to be fast in the uncontended case and fall back to kernel‑mediated blocking only when needed. Linux implements this using futex (fast userspace mutex).
// Internal data structure (simplified)

// A mutex is an aligned int whose value encodes the lock state:

//     0 – unlocked.

//     1 – locked, but no other thread waiting (a private bit).

//     A special “waiters” bit – mentioned later in futex ops, often using FUTEX_WAITERS flag or other conventions.
//     In NPTL, the mutex uses a 32‑bit word where bits track ownership and any waiting threads (exact layout is more complex, but the principle holds).

// Fast path (uncontended lock)

// The pthread_mutex_lock function first tries an atomic compare‑and‑swap (CAS):
// c

// if (__sync_bool_compare_and_swap(&mutex->__data.__lock, 0, 1)) {
//     return; // acquired without kernel involvement
// }

// If the mutex was 0, it becomes 1, and the thread holds the lock. Because CAS on x86 uses the LOCK CMPXCHG instruction, it guarantees atomicity and enforces a full memory barrier, making all prior memory operations visible.
// Slow path (contended or already locked)

// If the CAS fails, the lock is held. The thread then goes into the slow path:

//     It atomically sets the “waiters” flag using an atomic OR (now the mutex value indicates that there are waiters).

//     It calls futex(FUTEX_WAIT, &mutex_word, expected_value, ...). FUTEX_WAIT checks if the mutex word still equals expected_value (the value with the waiters bit set and the lock bit set). If it does, the kernel suspends the thread, putting it on a futex wait queue associated with that user‑space address. If it doesn’t (for example, between the OR and the WAIT the lock was released), the call returns immediately with EAGAIN, and the thread retries the fast path.

//     While waiting, the thread is removed from the scheduler’s run queue. It yields the CPU completely, no spinning.

// Unlock

// pthread_mutex_unlock performs:
// c

// if (__sync_bool_compare_and_swap(&mutex_word, 1, 0)) {
//     return; // no waiters, just clear the lock
// }
// // else: there are waiters, or we need to wake someone
// __sync_lock_release(&mutex_word);  // store 0 with release semantics
// futex(FUTEX_WAKE, &mutex_word, 1, ...);

// FUTEX_WAKE tells the kernel to wake up exactly one thread from the wait queue for that address. The woken thread then re‑runs the lock acquisition logic (fast path) and eventually returns to the user’s code. This two‑phase design means that an uncontended mutex costs just a single CAS (a few CPU cycles), while contention triggers a context switch – but only when necessary.

// Priority inheritance (for PTHREAD_PRIO_INHERIT) is an additional kernel‑managed protocol using futex, where the kernel can temporarily boost the mutex owner’s priority to avoid priority inversion.


// 4. Conditional Variables – The Signal‑Wait Contract

// A condition variable is always paired with a mutex. Internally, it uses its own futex word (and sometimes a separate low‑level mutex) to coordinate sleep and wake‑up.
// How pthread_cond_wait works atomically

// The call must do three things without missing a wake‑up:

//     Atomically release the user’s mutex.

//     Put the calling thread to sleep on the condition variable.

//     On wake‑up, reacquire the mutex.

// Linux implements this by:

//     The thread first re‑queues itself from the mutex’s wait queue to the condition’s futex queue, all inside a single futex syscall – FUTEX_WAIT_REQUEUE_PI (or simpler ones). A simplified model:

//         The condition variable has an associated “cond” futex integer.

//         pthread_cond_wait atomically:

//             Adds the thread to the cond’s wait queue.

//             Unlocks the mutex.

//             Then issues a FUTEX_WAIT on the cond futex.

//         Because the mutex unlock and the cond wait are not two separate system calls but are combined via the kernel’s futex requeue mechanism, there is no gap where a signal could be lost. If a pthread_cond_signal occurs in between, the kernel wakes the thread from the cond futex (which then re‑acquires the mutex) – no missed wake‑up.

//     The kernel manages wait queues for both the mutex and the cond variable. The requeue operation moves a waiting thread from the cond’s queue to the mutex’s queue, effectively transferring the wait so that when the thread wakes, it already queue‑waits for the mutex.

// Spurious wake‑ups

// The POSIX standard allows that a thread waiting on a condvar may return even if nobody signalled. This happens because the underlying futex wake can sometimes be triggered by process‑wide signals (e.g., SIGCONT), or the kernel may optimistically wake tasks. That’s why while(!condition) around pthread_cond_wait is mandatory – the condition itself (user‑defined) is re‑checked after every return, and the thread goes straight back to waiting if it’s a spurious wake‑up.


// 5. Synchronization – Atomics, Barriers, and Ordering

// Synchronization is the broader principle. Under the hood, it boils down to atomic operations and memory ordering.
// Atomic operations (CPU level)

// The CPU provides instructions like LOCK CMPXCHG, LOCK XADD, or LOCK BTS that guarantee an entire read‑modify‑write cycle appears instantaneous to all other cores. The LOCK prefix locks the cache line (on modern CPUs, via cache coherency protocol) for the duration of the instruction, preventing other agents from accessing that line.
// Memory barriers (fences)

//     Acquire barrier: Ensures all loads/stores after the barrier aren’t reordered before it. Gained when taking a lock.

//     Release barrier: Ensures all loads/stores before the barrier aren’t reordered after it. Applied when unlocking.

//     Full barrier: Prevents reordering in both directions.

// On x86, most locked atomics already enforce sequential consistency (very strong), so explicit barriers (mfence) are rarely needed in lock-unlock code. On ARM/Power, the memory model is weaker; the pthread library inserts appropriate dmb, sync instructions.
// Semantics of locks

// A mutex unlock synchronises with a subsequent lock that sees the release. This creates a happens‑before relationship:

//     All memory writes by the unlocker become visible to the next locker.

//     The compiler and CPU must respect that ordering. The atomic CAS in mutex_lock that sees 0 acts as an acquire operation; the atomic store of 0 in unlock is a release operation.

// All other primitives (read‑write locks, barriers, semaphores) are built using combinations of futex and atomic operations. For example, a sem_t can be implemented with a counter, a futex for sleeping, and atomic decrement+test; pthread_barrier_t uses a mutex and condition variable internally, atomically counting threads.


// 6. Sockets – The Kernel’s Networking Machinery
// File descriptor abstraction

// A socket is represented inside the kernel by two structures:

//     struct file – the generic VFS file object, containing the file descriptor’s reference count, current offset (usually unused for sockets), and a pointer to the “ops” function table.

//     struct socket – the networking‑specific part, which holds the protocol family (AF_INET), type (SOCK_STREAM/DGRAM), and protocol state. It in turn points to a struct sock.

// The socket() system call allocates a struct socket, picks the appropriate protocol handler (e.g., TCP), creates the corresponding struct sock, binds them, and installs a file descriptor in the process’s file table. All subsequent operations (bind, listen, sendmsg, etc.) are routed through the file descriptor to the socket’s proto_ops.
// Bind and address assignment

// bind() assigns a local IP address and port to the socket. The kernel records this in the struct inet_sock (the INET‑specific part of sock). It then inserts the socket into a hash table (inet_hash) keyed by (source port, address) so that incoming packets can be rapidly demultiplexed.
// Listen and the TCP state machine

// listen() does two things:

//     Sets the socket state to TCP_LISTEN (for TCP).

//     Allocates the accept queue (also called the backlog). This is actually two queues:

//         Incomplete connection queue – SYN‑Rcvd state (connections still going through three‑way handshake).

//         Completed connection queue – ESTABLISHED connections that have completed the handshake but have not yet been accepted by the application. The backlog parameter limits the sum of both queues.

// The kernel TCP stack now processes incoming SYN packets for this port:

//     SYN arrives → TCP creates a request socket (incomplete queue), replies with SYN‑ACK, starts a timer.

//     ACK of SYN‑ACK arrives → the request socket is removed from the incomplete queue, a full struct sock (child socket) is created, and it is moved to the completed queue. The listen socket is notified, waking any thread blocked in accept().

// Accept

// accept() dequeues the first completed connection from the listen socket’s completed queue. If the queue is empty and the socket is blocking, the calling thread is put to sleep on the socket’s wait queue (sk_sleep). When a new connection finishes the handshake, the kernel wakes one of those threads. The new file descriptor returned by accept points to the child struct sock; it shares the same protocol, but has its own state (ESTABLISHED), its own send/receive buffers, and its own sequence numbers.
// Data transfer: send/recv internals

// send() / write():

//     The system call copies user data into the socket’s send buffer (a sk_buff queue in struct sock). If the buffer is full, the process sleeps (or returns EAGAIN for non‑blocking).

//     TCP breaks the buffer into segments (based on MSS), creates a sk_buff for each, sets the TCP header fields (sequence number, acknowledgment, window size, etc.), and hands them to the IP layer.

//     The IP layer does routing, fragments if necessary, and passes to the network device driver, which transmits the frame.

//     The data remains in the send buffer until acknowledged (retransmission queue). Only when the ACK arrives does TCP remove the segment and free the send buffer space.

// recv() / read():

//     The system call checks the receive buffer (a queue of sk_buff fragments waiting to be read). If data is present, it copies as much as requested to user space and removes the consumed segments.

//     If the receive buffer is empty, the process sleeps on the socket’s wait queue.

//     Inside the kernel’s receive‑side softirq (NET_RX_SOFTIRQ), an incoming TCP segment is processed: the TCP header is validated, the packet is placed in the receive buffer, and if the segment’s sequence number is exactly the next expected one, any sleeping reader thread is woken. Out‑of‑order packets are queued separately but still wake the reader only when the hole is filled.

// Blocking vs non‑blocking

// If a socket is set to O_NONBLOCK, the kernel never puts the process to sleep; it returns EAGAIN immediately when an operation cannot be completed. Under the hood, the socket’s wait queue is still used for polling (epoll), but the send/recv paths check the file status flag and return instead of sleeping.


// 7. Client and Server – How Accept, Connect, and Multiplexing Work
// The three‑way handshake (connect/accept)

// Client connect():

//     Sends a SYN to the server’s address. The kernel creates a struct sock, puts it in TCP_SYN_SENT state, and blocks until the handshake completes (or returns EINPROGRESS if non‑blocking).

//     Upon receiving the SYN‑ACK, the client moves to ESTABLISHED and sends the final ACK.

//     The thread waiting in connect() wakes up.

// Server accept():

//     The listen socket is in LISTEN. When the final ACK arrives, the completed connection is ready. The kernel wakes a thread blocked in accept(), which removes the new sock from the completed queue and returns its file descriptor.

// Backlog / SYN flood protection

// The two‑queue design prevents a simple flooding attack from exhausting resources. Incomplete connections are stored as lightweight request sockets (not full struct sock). The kernel may also use SYN cookies: when the incomplete queue overflows, it doesn’t store anything; it sends a specially crafted SYN‑ACK whose sequence number encodes the needed state (cryptographically). When the matching ACK returns, the cookie is verified and a full socket is created directly – without using any queue space during the SYN‑RCVD state.
// I/O multiplexing (select, poll, epoll)

// These are simply ways to sleep on multiple sockets’ wait queues simultaneously.

//     epoll: a file descriptor that holds a red‑black tree of watched fds. The kernel registers a callback (ep_poll_callback) on each socket’s wait queue. When data arrives on a socket, that callback places the socket into an epoll‑ready list and wakes the epoll wait queue. The application can then retrieve the ready list in one batch, eliminating the linear scan of poll/select.


// 8. Protocols – How TCP and UDP Differ Under the Hood
// TCP (SOCK_STREAM)

//     Stateful: A TCP connection is defined by the tuple (src IP, src port, dst IP, dst port) and by the sequence/acknowledgement numbers. The kernel maintains a Transmission Control Block (TCB) inside struct tcp_sock containing current send/receive windows, timers, retransmission queue, and congestion control state.

//     Reliable byte stream: The protocol acknowledges segments. If an ACK isn’t received within the round‑trip time (RTT), the segment is retransmitted. This means the kernel manages a retransmission timer and a duplicate ACK counter for fast retransmit.

//     Ordering: TCP reconstructs the byte stream using sequence numbers. Out‑of‑order segments are kept in an out‑of‑order queue until the gap is filled.

//     Flow control: The receiver advertises a window, preventing the sender from overflowing the receive buffer.

//     Congestion control: Algorithms like CUBIC adjust the congestion window to avoid swamping routers. This is entirely inside the TCP layer – sockets don’t see it.

//     Connection lifecycle: Explicit handshake (SYN, SYN‑ACK, ACK) and teardown (FIN, ACK, FIN, ACK). The kernel handles TIME_WAIT on the side that sends the first FIN, preventing stray segments from confusing future connections.

// UDP (SOCK_DGRAM)

//     Stateless: The kernel only keeps the local bound address and any connected peer address. There is no connection state machine, no retransmission, no congestion window.

//     Datagram boundaries: Each sendto/recvfrom preserves the message length; readers get exactly one datagram, never part of a stream. The kernel buffers the entire user message in a single sk_buff and passes it to IP. If the datagram is larger than the local MTU, IP fragments it.

//     No guarantees: Delivery is best‑effort. If a datagram is lost or arrives corrupted, it’s simply dropped. recvfrom will block until a datagram arrives, but may never get anything if the network drops it.

//     Binding: bind() on UDP just associates a local port; the socket is immediately available for recvfrom. connect() on UDP simply sets the default destination address so you can use read/write instead of sendto/recvfrom, and filters out packets from other peers. No handshake packets are sent.

// Why this matters to the programmer

//     With TCP, send() may block if the send buffer is full (send buffer limited by congestion window and receive window). The data is just copied into the kernel; actual transmission happens asynchronously.

//     With UDP, sendto() usually doesn’t block (the kernel just forms a datagram and hands it to IP). It might block only momentarily for memory allocation, but there’s no back‑pressure from the network.

//     The choice of SOCK_STREAM vs SOCK_DGRAM triggers entirely different kernel code paths, from the socket’s proto_ops down to the protocol handler.

// All in all, the simple C APIs compile into a massive web of kernel logic: threads via clone, synchronization via atomic futex sleep/wake mechanisms, and networking via the kernel’s TCP/IP stack. Seeing the interplay between userspace primitives and the kernel’s implementation is key to understanding performance and correctness.




// -----




// 1. Threads in C (pthreads)

// A thread is a lightweight unit of execution within a process. All threads share the same address space (global variables, heap, file descriptors), which makes communication easy but introduces the risk of race conditions when multiple threads access shared data concurrently.

// Creating and joining threads

// ```c
// #include <pthread.h>
// #include <stdio.h>
// #include <stdlib.h>

// void* thread_func(void* arg) {
//     int* num = (int*)arg;
//     printf("Thread received: %d\n", *num);
//     return NULL;
// }

// int main() {
//     pthread_t tid;
//     int value = 42;

//     // int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
//     //                    void *(*start_routine)(void*), void *arg);
//     if (pthread_create(&tid, NULL, thread_func, &value) != 0) {
//         perror("pthread_create");
//         return 1;
//     }

//     // wait for the thread to finish
//     if (pthread_join(tid, NULL) != 0) {
//         perror("pthread_join");
//         return 1;
//     }

//     return 0;
// }
// ```

// Compile with -pthread (e.g., gcc -pthread program.c).
// Important: All threads of a process terminate when the main thread exits, even if they haven’t finished, unless you pthread_join them or detach them.

// The need for synchronization

// Consider two threads incrementing a global counter:

// ```c
// int counter = 0;            // shared

// void* increment(void* arg) {
//     for (int i = 0; i < 1000000; i++)
//         counter++;          // race condition: read‑modify‑write not atomic
//     return NULL;
// }
// ```

// counter++ translates to several machine instructions (load, add, store). When two threads do this without coordination, some increments are lost. The final count is unpredictable. A mutex fixes this.

// ---

// 2. Mutex (Mutual Exclusion)

// A mutex ensures that only one thread can access a critical section at a time.

// pthread_mutex_t API

// ```c
// pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;   // static init

// // dynamic init: pthread_mutex_init(&mutex, NULL);
// // destroy:      pthread_mutex_destroy(&mutex);

// pthread_mutex_lock(&mutex);    // block until lock acquired
// pthread_mutex_unlock(&mutex);  // release the lock
// ```

// Fixed counter example

// ```c
// pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
// int counter = 0;

// void* safe_increment(void* arg) {
//     for (int i = 0; i < 1000000; i++) {
//         pthread_mutex_lock(&mtx);
//         counter++;                // protected
//         pthread_mutex_unlock(&mtx);
//     }
//     return NULL;
// }
// ```

// Now the final counter is always the expected 2,000,000.

// Mutex types (via attributes)

// By default, mutexes are normal:

// · Deadlock if a thread locks again without unlocking.
// · Undefined behaviour if a non‑owner unlocks.
// · No speed‑up for recursive locking.

// Other types set with pthread_mutexattr_settype:

// · PTHREAD_MUTEX_ERRORCHECK – returns error on double lock / non‑owner unlock.
// · PTHREAD_MUTEX_RECURSIVE – a thread can lock multiple times; must unlock equally many times.
// · PTHREAD_MUTEX_ADAPTIVE_NP (Linux) – optimised for short critical sections, spins briefly before sleeping.

// Example using recursive mutex:

// ```c
// pthread_mutex_t mtx;
// pthread_mutexattr_t attr;
// pthread_mutexattr_init(&attr);
// pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
// pthread_mutex_init(&mtx, &attr);
// ```

// Best practices

// · Always unlock on every exit path (including after errors or return). Use goto cleanup or pthread_cleanup_push/pop.
// · Keep critical sections as short as possible.
// · Avoid calling unknown code while holding a lock to prevent deadlocks.
// · Be consistent with lock ordering in different threads.

// ---

// 3. Reader‑Writer Lock (pthread_rwlock_t)

// A reader‑writer lock allows multiple readers to hold the lock simultaneously, but a writer requires exclusive access. This is ideal when a shared resource is read much more often than modified.

// pthread_rwlock_t API

// ```c
// pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// // dynamic init: pthread_rwlock_init(&rwlock, NULL);
// // destroy:      pthread_rwlock_destroy(&rwlock);

// pthread_rwlock_rdlock(&rwlock);   // lock for reading (shared)
// pthread_rwlock_wrlock(&rwlock);   // lock for writing (exclusive)
// pthread_rwlock_unlock(&rwlock);   // unlock (same call for both)
// ```

// Semantics (typically):

// · Any number of readers can hold the lock as long as no writer holds it.
// · A writer must wait for all current readers and writers to finish.
// · New readers may be blocked if a writer is waiting, to prevent writer starvation (this behaviour is implementation‑defined; most modern implementations prefer reader or writer fairness or avoid writer starvation).

// Example: cached price table

// ```c
// #include <pthread.h>
// #include <stdio.h>

// typedef struct {
//     double price;
//     char ticker[16];
// } Stock;

// Stock table[100];                    // shared data
// int table_size = 0;
// pthread_rwlock_t rw = PTHREAD_RWLOCK_INITIALIZER;

// // Many reader threads
// void* query_price(void* arg) {
//     char* target = (char*)arg;
//     pthread_rwlock_rdlock(&rw);          // multiple readers OK
//     for (int i = 0; i < table_size; i++) {
//         if (strcmp(table[i].ticker, target) == 0) {
//             printf("%s: %f\n", target, table[i].price);
//             break;
//         }
//     }
//     pthread_rwlock_unlock(&rw);
//     return NULL;
// }

// // Infrequent writer thread
// void* update_price(void* arg) {
//     Stock* update = (Stock*)arg;
//     pthread_rwlock_wrlock(&rw);          // exclusive access
//     // search and update or add new entry
//     for (int i = 0; i < table_size; i++) {
//         if (strcmp(table[i].ticker, update->ticker) == 0) {
//             table[i].price = update->price;
//             pthread_rwlock_unlock(&rw);
//             return NULL;
//         }
//     }
//     // not found, add new
//     if (table_size < 100)
//         table[table_size++] = *update;
//     pthread_rwlock_unlock(&rw);
//     return NULL;
// }
// ```

// Reader‑writer lock trade‑offs

// · Overhead: rwlock is heavier than a plain mutex because of more complex state tracking. If the critical section is very short, a mutex may be faster even when there are many readers.
// · Starvation: Without careful handling, writers can be starved by a continuous stream of readers (or vice versa). POSIX leaves fairness details to the implementation. On Linux/glibc, the default is typically writer‑preferring to avoid writer starvation, but exact behaviour can depend on the version.
// · When to prefer:
//   · Read operations are lengthy (e.g., searching a large data structure).
//   · Many more readers than writers.
//   · The lock is held for a non‑trivial amount of time.

// ---

// 4. Mutex vs. Reader‑Writer Lock: Quick comparison

// Aspect Mutex (pthread_mutex_t) Reader‑Writer Lock (pthread_rwlock_t)
// Access type Exclusive only Shared for readers, exclusive for writers
// Parallelism None – serialises all threads Multiple readers can proceed simultaneously
// Overhead Lower Higher (more bookkeeping)
// Starvation risk None if used correctly Can starve writers (or readers) if not careful
// Recursive locking Possible with recursive type Not specified by POSIX (generally not recursive)
// Destruction constraint Must be unlocked before destroy Same; also no threads must be waiting

// ---

// 5. Important additional topics

// trylock variants

// Both mutex and rwlock have non‑blocking versions that return EBUSY if the lock cannot be acquired immediately:

// · pthread_mutex_trylock()
// · pthread_rwlock_tryrdlock(), pthread_rwlock_trywrlock()

// Use them to avoid blocking or to implement deadlock avoidance (e.g., back‑off and retry).

// Deadlocks and lock ordering

// A classic deadlock:

// ```c
// // Thread A:                 // Thread B:
// pthread_mutex_lock(&m1);     pthread_mutex_lock(&m2);
// pthread_mutex_lock(&m2);     pthread_mutex_lock(&m1);
// ```

// Both wait forever. Prevention: all threads always acquire locks in a globally consistent order. For rwlocks, same rule applies.

// Condition variables (brief)

// Often used with mutexes to wait for some condition to become true:

// ```c
// pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
// pthread_mutex_lock(&mtx);
// while (shared_flag == 0)
//     pthread_cond_wait(&cond, &mtx);  // atomically unlocks mutex and waits
// // use shared data...
// pthread_mutex_unlock(&mtx);
// ```

// A reader‑writer lock cannot be used directly with a condition variable because pthread_cond_wait requires a pthread_mutex_t *, not an rwlock. If you need to wait on a condition while using an rwlock, you usually fall back to a mutex or restructure your design.

// ---

// 6. Platform and linking

// · Includes: #include <pthread.h>
// · Compile/Link: Always use -pthread (or -lpthread on older systems). This flag ensures proper macro definitions and linking of the threads library.
// · C11 threads (<threads.h>, mtx_t, cnd_t) are an optional part of the C standard, but they are not widely supported, especially on Linux (glibc). POSIX pthreads remains the portable choice.

// ---

// By understanding these concepts and their APIs, you can write correct, efficient multithreaded programs in C. Start with simple mutex protection, then consider reader‑writer locks when profiling shows contention that can benefit from read sharing—always watching for starvation issues and performance overhead.
