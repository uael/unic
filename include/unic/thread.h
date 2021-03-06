/*
 * Copyright (C) 2010-2017 Alexander Saprykin <xelfium@gmail.com>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*!@file unic/thread.h
 * @brief Multithreading support
 * @author Alexander Saprykin
 *
 * A thread is a system execution unit which is managed independently by the
 * scheduler of the operating system. It allows to do things in parallel or
 * concurrently.
 *
 * #thread_t provides a convinient way of multithreading support using native
 * routines to provide the best performance on the target system.
 *
 * To create the thread use the u_thread_create() or u_thread_create_full()
 * routines. Joinable threads allow to wait until their execution is finished
 * before proceeding further. Thus you can synchronize threads' execution within
 * the main thread.
 *
 * A reference counter mechanism is used to keep track of a #thread_t structure.
 * It means that the structure will be freed automatically when the reference
 * counter becomes zero. Use u_thread_ref() to hold the structure and
 * u_thread_unref() to decrement the counter back. A running thread holds a
 * reference to itself structure, so you do not require to hold a reference
 * to the thread while it is running.
 *
 * Priorities (if supported) allow to tune scheduler behavior: threads with
 * higher priority will be executed more frequently. Be careful that improper
 * priorities may lead to negative effects when some threads may receive almost
 * zero execution time.
 *
 * Thread priorities are unreliable: not all operating systems respect thread
 * priorities in favour of process ones. Priorities may be ignored for bound
 * threads (every thread bound to a kernel light-weight thread as 1:1), other
 * systems may require administrative privileges to change the thread priority
 * (i.e. Linux). Windows always respects thread priorities.
 *
 * To put the current thread (even if it was not created using the #thread_t
 * routines) in a sleep state use u_thread_sleep().
 *
 * You can give a hint to the scheduler that the current thread do not need an
 * execution time with the u_thread_yield() routine. This is useful when some
 * of the threads are in an idle state so you do not want to waste a CPU time.
 * This only tells to the scheduler to skip the current scheduling cycle for the
 * calling thread, though the scheduler can ingnore it.
 *
 * A thread local storage (TLS) is provided. The TLS key's value can be accessed
 * through a reference key defined as a #thread_tKey. A TLS reference key is
 * some sort of a token which has an associated value. But every thread has its
 * own token value though using the same token object.
 *
 * After creating the TLS reference key every thread can use it to access a
 * local-specific value. Use the u_thread_local_new() call to create the TLS
 * reference key and pass it to every thread which needs local-specific values.
 * You can also provide a destroy notification function which would be called
 * upon a TLS key removal which is usually performed on the thread exit.
 *
 * There are two calls to set a TLS key's value: u_thread_set_local() and
 * u_thread_replace_local(). The only difference is that the former one calls
 * the provided destroy notification function before replacing the old value.
 */
#ifndef U_THREAD_H__
# define U_THREAD_H__

#include "unic/macros.h"
#include "unic/types.h"

/*!@brief Typedef for a #thread_t running method. */
typedef ptr_t (*thread_fn_t)(ptr_t arg);

/*!@brief Thread opaque data type. */
typedef struct thread thread_t;

/*!@brief TLS key opaque data type. */
typedef struct thread_key thread_key_t;

/*!@brief Thread priority. */
enum thread_prio {

  /*!@brief Inherits the caller thread priority. Default priority. */
  U_THREAD_PRIORITY_INHERIT = 0,

  /*!@brief Scheduled only when no other threads are running. */
  U_THREAD_PRIORITY_IDLE = 1,

  /*!@brief Scheduled less often than #U_THREAD_PRIORITY_LOW. */
  U_THREAD_PRIORITY_LOWEST = 2,

  /*!@brief Scheduled less often than #U_THREAD_PRIORITY_NORMAL. */
  U_THREAD_PRIORITY_LOW = 3,

  /*!@brief Operating system's default priority. */
  U_THREAD_PRIORITY_NORMAL = 4,

  /*!@brief Scheduled more often than #U_THREAD_PRIORITY_NORMAL. */
  U_THREAD_PRIORITY_HIGH = 5,

  /*!@brief Scheduled more often than #U_THREAD_PRIORITY_HIGH. */
  U_THREAD_PRIORITY_HIGHEST = 6,

  /*!@brief Scheduled as often as possible. */
  U_THREAD_PRIORITY_TIMECRITICAL = 7
};

typedef enum thread_prio thread_prio_t;

/*!@brief Creates a new #thread_t and starts it.
 * @param func Main thread function to run.
 * @param data Pointer to pass into the thread main function, may be NULL.
 * @param joinable Whether to create a joinable thread or not.
 * @param prio Thread priority.
 * @param stack_size Thread stack size, in bytes. Leave zero to use a default
 * value.
 * @return Pointer to #thread_t in case of success, NULL otherwise.
 * @since 0.0.1
 * @note Unreference the returned value after use with u_thread_unref(). You do
 * not need to call u_thread_ref() explicitly on the returned value.
 */
U_API thread_t *
u_thread_create_full(thread_fn_t func, ptr_t data, bool joinable,
  thread_prio_t prio, size_t stack_size);

/*!@brief Creates a #thread_t and starts it. A short version of
 * u_thread_create_full().
 * @param func Main thread function to run.
 * @param data Pointer to pass into the thread main function, may be NULL.
 * @param joinable Whether to create a joinable thread or not.
 * @return Pointer to #thread_t in case of success, NULL otherwise.
 * @since 0.0.1
 * @note Unreference the returned value after use with u_thread_unref(). You do
 * not need to call u_thread_ref() explicitly on the returned value.
 */
U_API thread_t *
u_thread_create(thread_fn_t func, ptr_t data, bool joinable);

/*!@brief Exits from the currently running (caller) thread.
 * @param code Exit code.
 * @since 0.0.1
 */
U_API void
u_thread_exit(int code);

/*!@brief Waits for the selected thread to become finished.
 * @param thread Thread to wait for.
 * @return Thread exit code in case of success, -1 otherwise.
 * @since 0.0.1
 * @note Thread must be joinable to return the non-negative result.
 */
U_API int
u_thread_join(thread_t *thread);

/*!@brief Sleeps the current thread (caller) for a specified amount of time.
 * @param msec Milliseconds to sleep.
 * @return 0 in case of success, -1 otherwise.
 * @since 0.0.1
 */
U_API int
u_thread_sleep(u32_t msec);

/*!@brief Sets a thread priority.
 * @param thread Thread to set the priority for.
 * @param prio Priority to set.
 * @return true in case of success, false otherwise.
 * @since 0.0.1
 */
U_API bool
u_thread_set_priority(thread_t *thread, thread_prio_t prio);

/*!@brief Tells the scheduler to skip the current (caller) thread in the current
 * planning stage.
 * @since 0.0.1
 *
 * The scheduler shouldn't give time ticks for the current thread during the
 * current period, but it may ignore this call.
 */
U_API void
u_thread_yield(void);

/*!@brief Gets an ID of the current (caller) thread.
 * @return The ID of the current thread.
 * @since 0.0.1
 *
 * This is a platform-dependent type. You shouldn't treat it as a number, it
 * only gives you the uniquer ID of the thread accross the system.
 */
U_API U_HANDLE
u_thread_current_id(void);

/*!@brief Gets a thread structure of the current (caller) thread.
 * @return The thread structure of the current thread.
 * @since 0.0.1
 * @note This call doesn't not increment the reference counter of the returned
 * structure.
 *
 * A thread structure will be returned even for the thread which was created
 * outside the library. But you should not use thread manipulation routines over
 * that structure.
 */
U_API thread_t *
u_thread_current(void);

/*!@brief Gets the ideal number of threads for the system based on the number of
 * avaialble CPUs and cores (physical and logical).
 * @return Ideal number of threads, 1 in case of failed detection.
 * @since 0.0.3
 */
U_API int
u_thread_ideal_count(void);

/*!@brief Increments a thread reference counter
 * @param thread #thread_t to increment the reference counter.
 * @since 0.0.1
 * @note The #thread_t object will not be removed until the reference counter is
 * positive.
 */
U_API void
u_thread_ref(thread_t *thread);

/*!@brief Decrements a thread reference counter
 * @param thread #thread_t to decrement the reference counter.
 * @since 0.0.1
 * @note When the reference counter becomes zero the #thread_t is removed from
 * the memory.
 */
U_API void
u_thread_unref(thread_t *thread);

/*!@brief Create a new TLS reference key.
 * @param free_func TLS key destroy notification call, leave NULL if not need.
 * @return New TLS reference key in case of success, NULL otherwise.
 * @since 0.0.1
 */
U_API thread_key_t *
u_thread_local_new(destroy_fn_t free_func);

/*!@brief Frees a TLS reference key.
 * @param key TLS reference key to free.
 * @since 0.0.1
 *
 * It doesn't remove the TLS key itself but only removes a reference used to
 * access the TLS slot.
 */
U_API void
u_thread_local_free(thread_key_t *key);

/*!@brief Gets a TLS value.
 * @param key TLS reference key to get the value for.
 * @return TLS value for the given key.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal use or program behavior,
 * the NULL value will be returned to tolerance the failure.
 */
U_API ptr_t
u_thread_get_local(thread_key_t *key);

/*!@brief Sets a TLS value.
 * @param key TLS reference key to set the value for.
 * @param value TLS value to set.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal use or program behavior.
 *
 * It doesn't call the destructor notification function provided with
 * u_thread_local_new().
 */
U_API void
u_thread_set_local(thread_key_t *key, ptr_t value);

/*!@brief Replaces a TLS value.
 * @param key TLS reference key to replace the value for.
 * @param value TLS value to set.
 * @since 0.0.1
 * @note This call may fail only in case of abnormal use or program behavior.
 *
 * This call does perform the notification function provided with
 * u_thread_local_new() on the old TLS value. This is the only difference with
 * u_thread_set_local().
 */
U_API void
u_thread_replace_local(thread_key_t *key, ptr_t value);

#endif /* !U_THREAD_H__ */
