#include <efi.h>
#include <efilib.h>

#include <elf.h>

EFI_STATUS _relocate (long ldbase, Elf64_Dyn *dyn,
		      EFI_HANDLE image EFI_UNUSED,
		      EFI_SYSTEM_TABLE *systab EFI_UNUSED)
{
	long relsz = 0, relent = 0, gotsz = 0;
	Elf64_Rel *rel = 0;
	unsigned long *addr = 0;
	int i;

	for (i = 0; dyn[i].d_tag != DT_NULL; ++i) {
		switch (dyn[i].d_tag) {
			case DT_REL:
				rel = (Elf64_Rel*)
					((unsigned long)dyn[i].d_un.d_ptr
					 + ldbase);
				break;

			case DT_RELSZ:
				relsz = dyn[i].d_un.d_val;
				break;

			case DT_RELENT:
				relent = dyn[i].d_un.d_val;
				break;

			case DT_PLTGOT:
				addr = (unsigned long *)
					((unsigned long)dyn[i].d_un.d_ptr
					 + ldbase);
				break;

			case DT_MIPS_LOCAL_GOTNO:
				gotsz = dyn[i].d_un.d_val;
				break;

			default:
				break;
		}
	}

	if ((!rel && relent == 0) && (!addr && gotsz == 0))
		return EFI_SUCCESS;

	if ((!rel && relent != 0) || (!addr && gotsz != 0))
		return EFI_LOAD_ERROR;

	while (gotsz > 0) {
		*addr += ldbase;
		addr += 1;
		gotsz --;
	}

	while (relsz > 0) {
		switch (ELF64_R_TYPE (swap_uint64 (rel->r_info))) {
			case R_MIPS_NONE:
				break;

			case (R_MIPS_64 << 8) | R_MIPS_REL32:
				addr = (unsigned long *)
					(ldbase + rel->r_offset);
				*addr += ldbase;
				break;

			default:
				break;
		}
		rel = (Elf64_Rel*) ((char *) rel + relent);
		relsz -= relent;
	}
	return EFI_SUCCESS;
}
