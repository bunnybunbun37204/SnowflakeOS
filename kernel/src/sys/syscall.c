#include <kernel/syscall.h>
#include <kernel/proc.h>
#include <kernel/timer.h>
#include <kernel/fb.h>
#include <kernel/wm.h>
#include <kernel/sys.h> // for UNUSED macro

#include <stdio.h>
#include <stdlib.h>

static void syscall_handler(registers_t* regs);

static void syscall_yield(registers_t* regs);
static void syscall_exit(registers_t* regs);
static void syscall_wait(registers_t* regs);
static void syscall_putchar(registers_t* regs);
static void syscall_sbrk(registers_t* regs);
static void syscall_get_framebuffer_info(registers_t* regs);
static void syscall_wm_open_window(registers_t* regs);
static void syscall_wm_close_window(registers_t* regs);
static void syscall_wm_render_window(registers_t* regs);

handler_t syscall_handlers[SYSCALL_NUM] = { 0 };

void init_syscall() {
	isr_register_handler(48, &syscall_handler);

	syscall_handlers[0] = syscall_yield;
	syscall_handlers[1] = syscall_exit;
	syscall_handlers[2] = syscall_wait;
	syscall_handlers[3] = syscall_putchar;
	syscall_handlers[4] = syscall_sbrk;
	syscall_handlers[6] = syscall_get_framebuffer_info;

	// TODO: convert those to a single syscall with parameters
	syscall_handlers[7] = syscall_wm_open_window;
	syscall_handlers[8] = syscall_wm_close_window;
	syscall_handlers[9] = syscall_wm_render_window;
}

static void syscall_handler(registers_t* regs) {
	if (syscall_handlers[regs->eax]) {
		handler_t handler = syscall_handlers[regs->eax];
		handler(regs);
	} else {
		printf("Unknown syscall %d\n", regs->eax);
	}
}

/* Convention:
 * - Arguments shall be passed in this order: ecx, edx
 * - If more are needed, pack them into a struct pointer
 * - Values shall be returned in eax, ecx then edx.
 */

static void syscall_yield(registers_t* regs) {
	UNUSED(regs);

	proc_switch_process();
}

static void syscall_exit(registers_t* regs) {
	UNUSED(regs);

	// TODO: exit status?
	proc_exit_current_process();
}

static void syscall_wait(registers_t* regs) {
	UNUSED(regs);

	// TODO
	/* This must be implemented by the scheduler, not here:
	 * - IRQs don't fire while in a syscall, so we can't rely on the timer
	 *   increasing
	 * - We can't both task switch and come back to this handler to check
	 *   the time
	 */
}

static void syscall_putchar(registers_t* regs) {
	putchar((char) regs->ecx);
}

static void syscall_sbrk(registers_t* regs) {
	uint32_t size = regs->ecx;
	regs->eax = (uint32_t) proc_sbrk(size);
}

static void syscall_get_framebuffer_info(registers_t* regs) {
	fb_t* fb = (fb_t*) regs->ecx;
	*fb = fb_get_info();
}

static void syscall_wm_open_window(registers_t* regs) {
	regs->eax = wm_open_window((fb_t*) regs->ecx, regs->edx);
}

static void syscall_wm_close_window(registers_t* regs) {
	wm_close_window(regs->ecx);
}

static void syscall_wm_render_window(registers_t* regs) {
	wm_render_window(regs->ecx);
}