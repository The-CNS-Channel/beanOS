#include <efi.h>
#include <efilib.h>

#include <elf.h>

EFI_STATUS _relocate (long ldbase, Elf32_Dyn *dyn,
		      EFI_HANDLE image EFI_UNUSED,
		      EFI_SYSTEM_TABLE *systab EFI_UNUSED)
{
	long relsz = 0, relent = 0;
	Elf32_Rel *rel = 0;
	unsigned long *addr;
	int i;

	for (i = 0; dyn[i].d_tag != DT_NULL; ++i) {
		switch (dyn[i].d_tag) {
			case DT_REL:
				rel = (Elf32_Rel*)
					((unsigned long)dyn[i].d_un.d_ptr
					 + ldbase);
				break;

			case DT_RELSZ:
				relsz = dyn[i].d_un.d_val;
				break;

			case DT_RELENT:
				relent = dyn[i].d_un.d_val;
				break;

			default:
				break;
		}
	}

	if (!rel && relent == 0)
		return EFI_SUCCESS;

	if (!rel || relent == 0)
		return EFI_LOAD_ERROR;

	while (relsz > 0) {
		switch (ELF32_R_TYPE (rel->r_info)) {
			case R_ARM_NONE:
				break;

			case R_ARM_RELATIVE:
				addr = (unsigned long *)
					(ldbase + rel->r_offset);
				*addr += ldbase;
				break;

			default:
				break;
		}
		rel = (Elf32_Rel*) ((char *) rel + relent);
		relsz -= relent;
	}
	return EFI_SUCCESS;
}
