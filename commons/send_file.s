	.file	"send_file.c"
	.section	.rodata
.LC0:
	.string	"[%Y-%m-%d  %H:%M:%S ] "
.LC1:
	.string	"\n < %s > "
.LC2:
	.string	"send_file.c"
.LC3:
	.string	"(%d) "
.LC4:
	.string	" : "
.LC5:
	.string	"SendFile:File %s Not found\n"
.LC6:
	.string	"a"
.LC7:
	.string	"Filename : %s, seq : %d"
	.text
	.globl	sendFile
	.type	sendFile, @function
sendFile:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%edi
	pushl	%ebx
	subl	$64, %esp
	movl	$0, 4(%esp)
	movl	12(%ebp), %eax
	movl	%eax, (%esp)
	.cfi_offset 3, -16
	.cfi_offset 7, -12
	call	open
	movl	%eax, -24(%ebp)
	movl	$0, -20(%ebp)
	movl	$59900, (%esp)
	call	malloc
	movl	%eax, -16(%ebp)
	movl	$0, -28(%ebp)
	cmpl	$0, -24(%ebp)
	jne	.L2
	movl	log_level, %eax
	testl	%eax, %eax
	jne	.L3
	movl	$log_mutex, (%esp)
	call	pthread_mutex_lock
	movl	$timer, (%esp)
	call	time
	movl	$timer, (%esp)
	call	localtime
	movl	%eax, tm_info
	movl	tm_info, %eax
	movl	%eax, %edx
	movl	$.LC0, %eax
	movl	%edx, 12(%esp)
	movl	%eax, 8(%esp)
	movl	$35, 4(%esp)
	movl	$fbuffer, (%esp)
	call	strftime
	movl	myIP, %edx
	movl	$.LC1, %eax
	movl	%edx, 12(%esp)
	movl	%eax, 8(%esp)
	movl	$27, 4(%esp)
	movl	$fbuf, (%esp)
	call	snprintf
	movl	$fbuffer, %eax
	movl	$70, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strncat
	movl	$.LC2, %eax
	movl	(%eax), %edx
	movl	%edx, fbuf1
	movl	4(%eax), %edx
	movl	%edx, fbuf1+4
	movl	8(%eax), %eax
	movl	%eax, fbuf1+8
	movl	$fbuf1, %eax
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strcat
	movl	$.LC3, %eax
	movl	$25, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$fbuf1, (%esp)
	call	sprintf
	movl	$fbuf1, %eax
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strcat
	movl	$.LC4, %ebx
	movl	$fbuf, %eax
	movl	$-1, -44(%ebp)
	movl	%eax, %edx
	movl	$0, %eax
	movl	-44(%ebp), %ecx
	movl	%edx, %edi
	repnz scasb
	movl	%ecx, %eax
	notl	%eax
	subl	$1, %eax
	leal	fbuf(%eax), %edx
	movl	(%ebx), %eax
	movl	%eax, (%edx)
	movl	$.LC5, %eax
	movl	(%eax), %edx
	movl	%edx, fbuf1
	movl	4(%eax), %edx
	movl	%edx, fbuf1+4
	movl	8(%eax), %edx
	movl	%edx, fbuf1+8
	movl	12(%eax), %edx
	movl	%edx, fbuf1+12
	movl	16(%eax), %edx
	movl	%edx, fbuf1+16
	movl	20(%eax), %edx
	movl	%edx, fbuf1+20
	movl	24(%eax), %eax
	movl	%eax, fbuf1+24
	movl	$fbuf1, %eax
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strcat
	movl	$.LC6, %edx
	movl	12(%ebp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fopen
	movl	%eax, log_fp
	movl	log_fp, %eax
	testl	%eax, %eax
	je	.L4
	movl	$fbuf, %edx
	movl	log_fp, %eax
	movl	12(%ebp), %ecx
	movl	%ecx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fprintf
	movl	log_fp, %eax
	movl	%eax, (%esp)
	call	fclose
.L4:
	movl	$log_mutex, (%esp)
	call	pthread_mutex_unlock
.L3:
	movl	$2, %eax
	jmp	.L5
.L2:
	cmpl	$0, 16(%ebp)
	je	.L6
	movl	16(%ebp), %eax
	movl	-16(%ebp), %edx
	addl	$4, %edx
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	strcpy
	jmp	.L7
.L6:
	movl	12(%ebp), %eax
	movl	-16(%ebp), %edx
	addl	$4, %edx
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	strcpy
.L7:
	movl	-16(%ebp), %eax
	movzwl	2(%eax), %eax
	movl	%eax, %edx
	orl	$1, %edx
	movl	-16(%ebp), %eax
	movw	%dx, 2(%eax)
.L14:
	movl	-16(%ebp), %eax
	addl	$204, %eax
	movl	$59600, %edx
	movl	%edx, 8(%esp)
	movl	$0, 4(%esp)
	movl	%eax, (%esp)
	call	memset
	movl	-16(%ebp), %eax
	addl	$204, %eax
	movl	$59600, 8(%esp)
	movl	%eax, 4(%esp)
	movl	-24(%ebp), %eax
	movl	%eax, (%esp)
	call	read
	movl	%eax, -20(%ebp)
	cmpl	$0, -20(%ebp)
	jg	.L8
	movl	$2, %eax
	jmp	.L5
.L8:
	cmpl	$0, -28(%ebp)
	jne	.L9
	movl	-16(%ebp), %eax
	movzwl	2(%eax), %eax
	movl	%eax, %edx
	orl	$4, %edx
	movl	-16(%ebp), %eax
	movw	%dx, 2(%eax)
.L9:
	movl	-20(%ebp), %eax
	cmpl	$59599, %eax
	ja	.L10
	movl	-16(%ebp), %eax
	movzwl	2(%eax), %eax
	movl	%eax, %edx
	orl	$8, %edx
	movl	-16(%ebp), %eax
	movw	%dx, 2(%eax)
.L10:
	movl	-16(%ebp), %eax
	movzwl	2(%eax), %eax
	movzwl	%ax, %eax
	movl	%eax, (%esp)
	call	htons
	movl	%eax, %edx
	movl	-16(%ebp), %eax
	movw	%dx, 2(%eax)
	movl	-20(%ebp), %eax
	addl	$204, %eax
	movl	%eax, 12(%esp)
	movl	-16(%ebp), %eax
	movl	%eax, 8(%esp)
	movl	$8, 4(%esp)
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	sendPayload
	movl	%eax, -12(%ebp)
	movl	log_level, %eax
	testl	%eax, %eax
	jne	.L11
	movl	$log_mutex, (%esp)
	call	pthread_mutex_lock
	movl	$timer, (%esp)
	call	time
	movl	$timer, (%esp)
	call	localtime
	movl	%eax, tm_info
	movl	tm_info, %eax
	movl	%eax, %edx
	movl	$.LC0, %eax
	movl	%edx, 12(%esp)
	movl	%eax, 8(%esp)
	movl	$35, 4(%esp)
	movl	$fbuffer, (%esp)
	call	strftime
	movl	myIP, %edx
	movl	$.LC1, %eax
	movl	%edx, 12(%esp)
	movl	%eax, 8(%esp)
	movl	$27, 4(%esp)
	movl	$fbuf, (%esp)
	call	snprintf
	movl	$fbuffer, %eax
	movl	$70, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strncat
	movl	$.LC2, %eax
	movl	(%eax), %edx
	movl	%edx, fbuf1
	movl	4(%eax), %edx
	movl	%edx, fbuf1+4
	movl	8(%eax), %eax
	movl	%eax, fbuf1+8
	movl	$fbuf1, %eax
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strcat
	movl	$.LC3, %eax
	movl	$53, 8(%esp)
	movl	%eax, 4(%esp)
	movl	$fbuf1, (%esp)
	call	sprintf
	movl	$fbuf1, %eax
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strcat
	movl	$.LC4, %ebx
	movl	$fbuf, %eax
	movl	$-1, -44(%ebp)
	movl	%eax, %edx
	movl	$0, %eax
	movl	-44(%ebp), %ecx
	movl	%edx, %edi
	repnz scasb
	movl	%ecx, %eax
	notl	%eax
	subl	$1, %eax
	leal	fbuf(%eax), %edx
	movl	(%ebx), %eax
	movl	%eax, (%edx)
	movl	$.LC7, %eax
	movl	(%eax), %edx
	movl	%edx, fbuf1
	movl	4(%eax), %edx
	movl	%edx, fbuf1+4
	movl	8(%eax), %edx
	movl	%edx, fbuf1+8
	movl	12(%eax), %edx
	movl	%edx, fbuf1+12
	movl	16(%eax), %edx
	movl	%edx, fbuf1+16
	movl	20(%eax), %eax
	movl	%eax, fbuf1+20
	movl	$fbuf1, %eax
	movl	%eax, 4(%esp)
	movl	$fbuf, (%esp)
	call	strcat
	movl	$.LC6, %edx
	movl	12(%ebp), %eax
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fopen
	movl	%eax, log_fp
	movl	log_fp, %eax
	testl	%eax, %eax
	je	.L12
	movl	-16(%ebp), %eax
	leal	4(%eax), %ebx
	movl	$fbuf, %edx
	movl	log_fp, %eax
	movl	-28(%ebp), %ecx
	movl	%ecx, 12(%esp)
	movl	%ebx, 8(%esp)
	movl	%edx, 4(%esp)
	movl	%eax, (%esp)
	call	fprintf
	movl	log_fp, %eax
	movl	%eax, (%esp)
	call	fclose
.L12:
	movl	$log_mutex, (%esp)
	call	pthread_mutex_unlock
.L11:
	cmpl	$1, -12(%ebp)
	je	.L13
	movl	-16(%ebp), %eax
	movl	%eax, (%esp)
	call	free
	movl	-12(%ebp), %eax
	jmp	.L5
.L13:
	movl	-16(%ebp), %eax
	movw	$0, 2(%eax)
	addl	$1, -28(%ebp)
	cmpl	$59600, -20(%ebp)
	je	.L14
	movl	-16(%ebp), %eax
	movl	%eax, (%esp)
	call	free
	movl	-24(%ebp), %eax
	movl	%eax, (%esp)
	call	close
	movl	$1, %eax
.L5:
	addl	$64, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%edi
	.cfi_restore 7
	popl	%ebp
	.cfi_def_cfa 4, 4
	.cfi_restore 5
	ret
	.cfi_endproc
.LFE0:
	.size	sendFile, .-sendFile
	.ident	"GCC: (Ubuntu/Linaro 4.6.3-1ubuntu5) 4.6.3"
	.section	.note.GNU-stack,"",@progbits
