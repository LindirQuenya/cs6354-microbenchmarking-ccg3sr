	.file	"harness.c"
	.text
	.p2align 4
	.globl	emptyFunction
	.type	emptyFunction, @function
emptyFunction:
.LFB5681:
	.cfi_startproc
	ret
	.cfi_endproc
.LFE5681:
	.size	emptyFunction, .-emptyFunction
	.p2align 4
	.globl	int_stats_mean
	.type	int_stats_mean, @function
int_stats_mean:
.LFB5682:
	.cfi_startproc
	testq	%rsi, %rsi
	je	.L6
	leaq	(%rdi,%rsi,4), %rax
	pxor	%xmm0, %xmm0
	.p2align 4,,10
	.p2align 3
.L5:
	pxor	%xmm1, %xmm1
	addq	$4, %rdi
	cvtsi2sdl	-4(%rdi), %xmm1
	addsd	%xmm1, %xmm0
	cmpq	%rdi, %rax
	jne	.L5
	pxor	%xmm1, %xmm1
	cvtsi2sdq	%rsi, %xmm1
	divsd	%xmm1, %xmm0
	ret
	.p2align 4,,10
	.p2align 3
.L6:
	pxor	%xmm1, %xmm1
	pxor	%xmm0, %xmm0
	cvtsi2sdq	%rsi, %xmm1
	divsd	%xmm1, %xmm0
	ret
	.cfi_endproc
.LFE5682:
	.size	int_stats_mean, .-int_stats_mean
	.p2align 4
	.globl	int_stats_sd
	.type	int_stats_sd, @function
int_stats_sd:
.LFB5683:
	.cfi_startproc
	pxor	%xmm4, %xmm4
	cvtsi2sdq	%rsi, %xmm4
	testq	%rsi, %rsi
	je	.L16
	pxor	%xmm3, %xmm3
	leaq	(%rdi,%rsi,4), %rdx
	movq	%rdi, %rax
	movapd	%xmm3, %xmm0
	.p2align 4,,10
	.p2align 3
.L10:
	pxor	%xmm1, %xmm1
	addq	$4, %rax
	cvtsi2sdl	-4(%rax), %xmm1
	addsd	%xmm1, %xmm0
	cmpq	%rax, %rdx
	jne	.L10
	divsd	%xmm4, %xmm0
	movapd	%xmm3, %xmm2
	.p2align 4,,10
	.p2align 3
.L12:
	pxor	%xmm1, %xmm1
	addq	$4, %rdi
	cvtsi2sdl	-4(%rdi), %xmm1
	subsd	%xmm0, %xmm1
	mulsd	%xmm1, %xmm1
	addsd	%xmm1, %xmm2
	cmpq	%rdi, %rdx
	jne	.L12
.L9:
	divsd	%xmm4, %xmm2
	ucomisd	%xmm2, %xmm3
	movapd	%xmm2, %xmm0
	ja	.L24
	sqrtsd	%xmm0, %xmm0
	ret
	.p2align 4,,10
	.p2align 3
.L16:
	pxor	%xmm3, %xmm3
	movapd	%xmm3, %xmm2
	jmp	.L9
.L24:
	jmp	sqrt
	.cfi_endproc
.LFE5683:
	.size	int_stats_sd, .-int_stats_sd
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC1:
	.string	"Cycles: %llu\n"
	.text
	.p2align 4
	.globl	measureFunction
	.type	measureFunction, @function
measureFunction:
.LFB5684:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	pushq	%rbx
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	subq	$8, %rsp
	.cfi_def_cfa_offset 32
	lfence
	rdtsc
	lfence
	lfence
	movq	%rax, %rbp
	salq	$32, %rdx
	orq	%rdx, %rbp
	rdtsc
	movl	$.LC1, %edi
	lfence
	movq	%rax, %rbx
	salq	$32, %rdx
	xorl	%eax, %eax
	orq	%rdx, %rbx
	movq	%rbx, %rsi
	subq	%rbp, %rsi
	call	printf
	movl	%ebx, %eax
	addq	$8, %rsp
	.cfi_def_cfa_offset 24
	subl	%ebp, %eax
	popq	%rbx
	.cfi_def_cfa_offset 16
	popq	%rbp
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE5684:
	.size	measureFunction, .-measureFunction
	.section	.rodata.str1.1
.LC2:
	.string	"Average cycles: %f\n"
.LC3:
	.string	"Standard deviation: %f\n"
	.section	.text.startup,"ax",@progbits
	.p2align 4
	.globl	main
	.type	main, @function
main:
.LFB5685:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$24, %rsp
	.cfi_offset 15, -24
	.cfi_offset 14, -32
	.cfi_offset 13, -40
	.cfi_offset 12, -48
	.cfi_offset 3, -56
	cmpl	$2, %edi
	jg	.L28
	movl	$10000, -64(%rbp)
	subq	$40000, %rsp
	movl	$1000, %ebx
	movq	$10000, -56(%rbp)
	movq	%rsp, %r13
.L29:
	xorl	%r12d, %r12d
	.p2align 4,,10
	.p2align 3
.L31:
	lfence
	rdtsc
	lfence
	lfence
	movq	%rax, %rcx
	salq	$32, %rdx
	orq	%rdx, %rcx
	rdtsc
	movl	$.LC1, %edi
	lfence
	salq	$32, %rdx
	addl	$1, %r12d
	orq	%rdx, %rax
	subq	%rcx, %rax
	movq	%rax, %rsi
	xorl	%eax, %eax
	call	printf
	cmpl	%ebx, %r12d
	jl	.L31
.L32:
	movl	-64(%rbp), %eax
	movq	%r13, %r15
	subl	$1, %eax
	leaq	4(%r13,%rax,4), %r12
	.p2align 4,,10
	.p2align 3
.L35:
	lfence
	rdtsc
	lfence
	lfence
	movq	%rax, %r14
	salq	$32, %rdx
	orq	%rdx, %r14
	rdtsc
	movl	$.LC1, %edi
	lfence
	movq	%rax, %rbx
	salq	$32, %rdx
	xorl	%eax, %eax
	addq	$4, %r15
	orq	%rdx, %rbx
	movq	%rbx, %rsi
	subl	%r14d, %ebx
	subq	%r14, %rsi
	call	printf
	movl	%ebx, -4(%r15)
	cmpq	%r15, %r12
	jne	.L35
.L36:
	cmpq	$0, -56(%rbp)
	pxor	%xmm0, %xmm0
	je	.L34
	movq	-56(%rbp), %rdi
	movq	%r13, %rax
	pxor	%xmm0, %xmm0
	leaq	0(%r13,%rdi,4), %rdx
	.p2align 4,,10
	.p2align 3
.L37:
	pxor	%xmm1, %xmm1
	addq	$4, %rax
	cvtsi2sdl	-4(%rax), %xmm1
	addsd	%xmm1, %xmm0
	cmpq	%rax, %rdx
	jne	.L37
.L34:
	pxor	%xmm2, %xmm2
	movq	-56(%rbp), %rsi
	movq	%r13, %rdi
	cvtsi2sdl	-64(%rbp), %xmm2
	divsd	%xmm2, %xmm0
	movsd	%xmm0, -64(%rbp)
	call	int_stats_sd
	movsd	-64(%rbp), %xmm1
	movl	$.LC2, %edi
	movl	$1, %eax
	movsd	%xmm0, -56(%rbp)
	movapd	%xmm1, %xmm0
	call	printf
	movsd	-56(%rbp), %xmm2
	movl	$.LC3, %edi
	movl	$1, %eax
	movapd	%xmm2, %xmm0
	call	printf
	leaq	-40(%rbp), %rsp
	xorl	%eax, %eax
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	.cfi_remember_state
	.cfi_def_cfa 7, 8
	ret
.L28:
	.cfi_restore_state
	movq	16(%rsi), %rdi
	movl	$10, %edx
	xorl	%esi, %esi
	call	strtol
	movl	$10, %esi
	movslq	%eax, %rdi
	movq	%rax, %rcx
	movl	%eax, -64(%rbp)
	leaq	15(,%rdi,4), %rax
	movq	%rdi, -56(%rbp)
	andq	$-16, %rax
	subq	%rax, %rsp
	movl	%ecx, %eax
	cltd
	movq	%rsp, %r13
	idivl	%esi
	movl	%eax, %ebx
	cmpl	$9, %ecx
	jg	.L29
	testl	%ecx, %ecx
	jle	.L36
	jmp	.L32
	.cfi_endproc
.LFE5685:
	.size	main, .-main
	.ident	"GCC: (GNU) 11.4.0"
	.section	.note.GNU-stack,"",@progbits
