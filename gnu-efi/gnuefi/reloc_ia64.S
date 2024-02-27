#define MAX_FUNCTION_DESCRIPTORS	750

#define ST_VALUE_OFF	8

#define EFI_SUCCESS		0
#define EFI_LOAD_ERROR		1
#define EFI_BUFFER_TOO_SMALL	5

#define DT_NULL		0
#define DT_RELA		7
#define DT_RELASZ	8
#define DT_RELAENT	9
#define DT_SYMTAB	6
#define DT_SYMENT	11

#define R_IA64_NONE		0
#define R_IA64_REL64MSB		0x6e
#define R_IA64_REL64LSB		0x6f
#define R_IA64_DIR64MSB		0x26
#define R_IA64_DIR64LSB		0x27
#define R_IA64_FPTR64MSB	0x46
#define R_IA64_FPTR64LSB	0x47

#define	ldbase	in0
#define	dyn	in1

#define d_tag	r16
#define d_val	r17
#define rela	r18
#define relasz	r19
#define relaent	r20
#define addr	r21
#define r_info	r22
#define r_offset r23
#define r_addend r24
#define r_type	r25
#define r_sym	r25
#define fptr	r26
#define fptr_limit r27
#define symtab	f8
#define syment	f9
#define ftmp	f10

#define	target	r16
#define val	r17

#define NLOC	0

#define Pnull		p6
#define Prela		p7
#define Prelasz		p8
#define Prelaent	p9
#define Psymtab		p10
#define Psyment		p11

#define Pnone		p6
#define Prel		p7
#define Pfptr		p8

#define Pmore		p6

#define Poom		p6

	.global _relocate
	.proc _relocate
_relocate:
	alloc r2=ar.pfs,2,0,0,0
	movl	fptr = @gprel(fptr_mem_base)
	;;
	add	fptr = fptr, gp
	movl	fptr_limit = @gprel(fptr_mem_limit)
	;;
	add	fptr_limit = fptr_limit, gp

search_dynamic:
	ld8	d_tag = [dyn],8
	;;
	ld8	d_val = [dyn],8
	cmp.eq	Pnull,p0 = DT_NULL,d_tag
(Pnull)	br.cond.sptk.few apply_relocs
	cmp.eq	Prela,p0 = DT_RELA,d_tag
	cmp.eq	Prelasz,p0 = DT_RELASZ,d_tag
	cmp.eq	Psymtab,p0 = DT_SYMTAB,d_tag
	cmp.eq	Psyment,p0 = DT_SYMENT,d_tag
	cmp.eq	Prelaent,p0 = DT_RELAENT,d_tag
	;;
(Prela)	add rela = d_val, ldbase
(Prelasz) mov relasz = d_val
(Prelaent) mov relaent = d_val
(Psymtab) add val = d_val, ldbase
	;;
(Psyment) setf.sig syment = d_val
	;;
(Psymtab) setf.sig symtab = val
	br.sptk.few search_dynamic

apply_loop:
	ld8	r_offset = [rela]
	add	addr = 8,rela
	sub	relasz = relasz,relaent
	;;

	ld8	r_info = [addr],8
	;;
	ld8	r_addend = [addr]
	add	target = ldbase, r_offset

	add	rela = rela,relaent
	extr.u	r_type = r_info, 0, 32
	;;
	cmp.eq	Pnone,p0 = R_IA64_NONE,r_type
	cmp.eq	Prel,p0 = R_IA64_REL64LSB,r_type
	cmp.eq	Pfptr,p0 = R_IA64_FPTR64LSB,r_type
(Prel)	br.cond.sptk.few apply_REL64
	;;
	cmp.eq	Prel,p0 = R_IA64_DIR64LSB,r_type

(Pnone)	br.cond.sptk.few apply_relocs
(Prel)	br.cond.sptk.few apply_REL64
(Pfptr)	br.cond.sptk.few apply_FPTR64

	mov	r8 = EFI_LOAD_ERROR
	br.ret.sptk.few rp

apply_relocs:
	cmp.ltu	Pmore,p0=0,relasz
(Pmore)	br.cond.sptk.few apply_loop

	mov	r8 = EFI_SUCCESS
	br.ret.sptk.few rp

apply_REL64:
	ld8 val = [target]
	;;
	add val = val,ldbase
	;;
	st8 [target] = val
	br.cond.sptk.few apply_relocs

apply_FPTR64:
	st8	[target] = fptr
	extr.u	r_sym = r_info,32,32
	add	target = 8,fptr
	;;

	setf.sig ftmp = r_sym
	mov	r8=EFI_BUFFER_TOO_SMALL
	;;
	cmp.geu	Poom,p0 = fptr,fptr_limit

	xma.lu	ftmp = ftmp,syment,symtab
(Poom)	br.ret.sptk.few rp
	;;
	getf.sig addr = ftmp
	st8	[target] = gp
	;;
	add	addr = ST_VALUE_OFF, addr
	;;
	ld8	val = [addr]
	;;
	add	val = val,ldbase
	;;
	st8	[fptr] = val,16
	br.cond.sptk.few apply_relocs

	.endp _relocate

	.data
	.align 16
fptr_mem_base:
	.space  MAX_FUNCTION_DESCRIPTORS*16
fptr_mem_limit:
