#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "bele.h"
#include "elf.h"
#include "dat.h"

static int readelf32ehdr(FILE*, Fhdr*);
static int readelf32shdr(FILE*, Fhdr*);
static int readelf32phdr(FILE*, Fhdr*);
static int readelf32syment(uint8_t*, Fhdr*);

static int readelf64ehdr(FILE*, Fhdr*);
static int readelf64shdr(FILE*, Fhdr*);
static int readelf64phdr(FILE*, Fhdr*);
static int readelf64syment(uint8_t*, Fhdr*);

static st_dataInterface_t dataInterface[] = {
	{
		ELFDATANONE,
		"invalid",
		NULL,
		NULL,
		NULL,
		NULL
	},
	{
		ELFDATA2LSB,
		"little-endian",
		le8get,
		le16get,
		le32get,
		le64get
	},
	{
		ELFDATA2MSB,
		"big-endian",
		be8get,
		be16get,
		be32get,
		be64get
	}
};

static st_classInterface_t classInterface[] = {
	{
		ELFCLASSNONE,
		"invalid",
		0,
		0,
		0,
		NULL,
		NULL,
		NULL,
		NULL,
	},
	{
		ELFCLASS32,
		"32-bit",
		sizeof(Elf32_Ehdr),
		sizeof(Elf32_Shdr),
		sizeof(Elf32_Phdr),
		readelf32ehdr,
		readelf32shdr,
		readelf32phdr,
		readelf32syment,
	},
	{
		ELFCLASS64,
		"64-bit",
		sizeof(Elf64_Ehdr),
		sizeof(Elf64_Shdr),
		sizeof(Elf64_Phdr),
		readelf64ehdr,
		readelf64shdr,
		readelf64phdr,
		readelf64syment,
	}
};


/*
 * Read ELF ident
 */
static int
readident(FILE *f, Fhdr *fp)
{
	uint8_t buf[EI_NIDENT];
	unsigned int i;
	uint8_t *p;

	if (fseek(f, 0, SEEK_SET) < 0)
		return -1;

	p = buf;
	
	if (fread(p, EI_NIDENT, 1, f) != 1)
		return -1;
	p += EI_NIDENT;

	if (buf[EI_MAG0] != ELFMAG0)
		return -1;
	if (buf[EI_MAG1] != ELFMAG1)
		return -1;
	if (buf[EI_MAG2] != ELFMAG2)
		return -1;
	if (buf[EI_MAG3] != ELFMAG3)
		return -1;

	if(buf[EI_VERSION] != EV_CURRENT) {
		fprintf(stderr, "unsupported file version %d\n", buf[EI_VERSION]);
		return -1;
	}

	for (i = 0; i < nelem(classInterface); i++) {
		if (buf[EI_CLASS] != classInterface[i].type)
			continue;
		if (classInterface[i].readelfehdr == NULL)
			return -1;
		fp->classInterface = classInterface + i;
		fp->ehsize = classInterface[i].ehsize;
		fp->shentsize = classInterface[i].shentsize;
		fp->phentsize = classInterface[i].phentsize;
		break;
	}

	if (i == nelem(classInterface))
		return -1;

	for (i = 0; i < nelem(dataInterface); i++) {
		if (buf[EI_DATA] != dataInterface[i].type)
			continue;
		if (dataInterface[i].get8 == NULL)
			return -1;
		fp->dataInterface = dataInterface + i;
		break;
	}

	if (i == nelem(dataInterface))
		return -1;

	fp->elfclass = buf[EI_CLASS];
	fp->elfdata = buf[EI_DATA];
	fp->elfversion = buf[EI_VERSION];
	fp->osabi = buf[EI_OSABI];
	fp->abiversion = buf[EI_ABIVERSION];

	return (int)(p - buf);
}

/*
 * Read ELF32 Header
 */
static int
readelf32ehdr(FILE *f, Fhdr *fp)
{
	uint8_t buf[Eh32sz];
	Elf32_Ehdr e;
	uint8_t *p;

	p = buf;

	if (fseek(f, 0, SEEK_SET) < 0)
		return -1;

	if (fread(p, fp->ehsize, 1, f) != 1)
		return -1;

	memmove(&e.ident, p, sizeof(e.ident));
	p += sizeof(e.ident);
	p += fp->dataInterface->get16(p, &e.type);
	p += fp->dataInterface->get16(p, &e.machine);
	p += fp->dataInterface->get32(p, &e.version);
	p += fp->dataInterface->get32(p, &e.entry);
	p += fp->dataInterface->get32(p, &e.phoff);
	p += fp->dataInterface->get32(p, &e.shoff);
	p += fp->dataInterface->get32(p, &e.flags);
	p += fp->dataInterface->get16(p, &e.ehsize);
	p += fp->dataInterface->get16(p, &e.phentsize);
	p += fp->dataInterface->get16(p, &e.phnum);
	p += fp->dataInterface->get16(p, &e.shentsize);
	p += fp->dataInterface->get16(p, &e.shnum);
	p += fp->dataInterface->get16(p, &e.shstrndx);

	if (e.type != ET_REL && e.type != ET_EXEC && 
		e.type != ET_DYN && e.type != ET_CORE) {
		fprintf(stderr, "unsupported file type %d\n", e.type);
		return -1;
	}

	if (fp->ehsize != e.ehsize) {
		fprintf(stderr, "ehsize mismatch; want %u; got %u\n", fp->ehsize, e.ehsize);
		return -1;
	}

	if (fp->shentsize != e.shentsize) {
		fprintf(stderr, "shentsize mismatch; want %u; got %u\n", fp->ehsize, e.ehsize);
		return -1;
	}

	if (fp->phentsize != e.phentsize) {
		fprintf(stderr, "phentsize mismatch; want %u; got %u\n", fp->ehsize, e.ehsize);
		return -1;
	}

	fp->type = e.type;
	fp->machine = e.machine;
	fp->version = e.version;
	fp->entry = e.entry;
	fp->shoff = e.shoff;
	fp->phoff = e.phoff;
	fp->phnum = e.phnum;
	fp->shnum = e.shnum;
	fp->shstrndx = e.shstrndx;

	return (int)(p - buf);
}

/*
 * Read ELF64 Header
 */
static int
readelf64ehdr(FILE *f, Fhdr *fp)
{
	uint8_t buf[Eh64sz];
	Elf64_Ehdr e;
	uint8_t *p;

	p = buf;

	if (fseek(f, 0, SEEK_SET) < 0)
		return -1;

	if (fread(p, fp->ehsize , 1, f) != 1)
		return -1;

	memmove(&e.ident, p, sizeof(e.ident));
	p += sizeof(e.ident);
	p += fp->dataInterface->get16(p, &e.type);
	p += fp->dataInterface->get16(p, &e.machine);
	p += fp->dataInterface->get32(p, &e.version);
	p += fp->dataInterface->get64(p, &e.entry);
	p += fp->dataInterface->get64(p, &e.phoff);
	p += fp->dataInterface->get64(p, &e.shoff);
	p += fp->dataInterface->get32(p, &e.flags);
	p += fp->dataInterface->get16(p, &e.ehsize);
	p += fp->dataInterface->get16(p, &e.phentsize);
	p += fp->dataInterface->get16(p, &e.phnum);
	p += fp->dataInterface->get16(p, &e.shentsize);
	p += fp->dataInterface->get16(p, &e.shnum);
	p += fp->dataInterface->get16(p, &e.shstrndx);

	if (e.type != ET_REL && e.type != ET_EXEC && 
		e.type != ET_DYN && e.type != ET_CORE) {
		fprintf(stderr, "unsupported file type %d\n", e.type);
		return -1;
	}

	if (fp->ehsize != e.ehsize) {
		fprintf(stderr, "ehsize mismatch; want %u; got %u\n", fp->ehsize, e.ehsize);
		return -1;
	}

	if (fp->shentsize != e.shentsize) {
		fprintf(stderr, "shentsize mismatch; want %u; got %u\n", fp->ehsize, e.ehsize);
		return -1;
	}

	if (fp->phentsize != e.phentsize) {
		fprintf(stderr, "phentsize mismatch; want %u; got %u\n", fp->ehsize, e.ehsize);
		return -1;
	}

	fp->type = e.type;
	fp->machine = e.machine;
	fp->version = e.version;
	fp->entry = e.entry;
	fp->shoff = e.shoff;
	fp->phoff = e.phoff;
	fp->phnum = e.phnum;
	fp->shnum = e.shnum;
	fp->shstrndx = e.shstrndx;

	return (int)(p - buf);
}

/*
 * Unpack ELF32 Section Header
 */
static int
unpackelf32shdr(uint8_t *buf, int len, Elf32_Shdr *sh, Fhdr *fp)
{
	uint8_t *p;

	if (len < Sh32sz)
		return -1;

	p = buf;

	p += fp->dataInterface->get32(p, &sh->name);
	p += fp->dataInterface->get32(p, &sh->type);
	p += fp->dataInterface->get32(p, &sh->flags);
	p += fp->dataInterface->get32(p, &sh->addr);
	p += fp->dataInterface->get32(p, &sh->offset);
	p += fp->dataInterface->get32(p, &sh->size);
	p += fp->dataInterface->get32(p, &sh->link);
	p += fp->dataInterface->get32(p, &sh->info);
	p += fp->dataInterface->get32(p, &sh->addralign);
	p += fp->dataInterface->get32(p, &sh->entsize);

	return p - buf;
}

/*
 * Read ELF32 Section Header
 */
static int
readelf32shdr(FILE *f, Fhdr *fp)
{
	uint8_t buf[Sh32sz];
	Elf32_Shdr sh;

	if (fread(buf, fp->shentsize, 1, f) != 1)
		return -1;

	if (unpackelf32shdr(buf, sizeof(buf), &sh, fp) < 0)
		return -1;

	fp->name = sh.name;
	fp->offset = sh.offset;
	fp->size = sh.size;
	fp->addr = sh.addr;

	return 0;
}

/*
 * Unpack ELF64 Section Header
 */
static int
unpackelf64shdr(uint8_t *buf, int len, Elf64_Shdr *sh, Fhdr *fp)
{
	uint8_t *p;

	if (len < Sh64sz)
		return -1;

	p = buf;

	p += fp->dataInterface->get32(p, &sh->name);
	p += fp->dataInterface->get32(p, &sh->type);
	p += fp->dataInterface->get64(p, &sh->flags);
	p += fp->dataInterface->get64(p, &sh->addr);
	p += fp->dataInterface->get64(p, &sh->offset);
	p += fp->dataInterface->get64(p, &sh->size);
	p += fp->dataInterface->get32(p, &sh->link);
	p += fp->dataInterface->get32(p, &sh->info);
	p += fp->dataInterface->get64(p, &sh->addralign);
	p += fp->dataInterface->get64(p, &sh->entsize);

	return p - buf;
}

/*
 * Read ELF64 Section Header
 */
static int
readelf64shdr(FILE *f, Fhdr *fp)
{
	uint8_t buf[Sh64sz];
	Elf64_Shdr sh;

	if (fread(buf, fp->shentsize, 1, f) != 1)
		return -1;

	if (unpackelf64shdr(buf, sizeof(buf), &sh, fp) < 0)
		return -1;

	fp->name = sh.name;
	fp->offset = sh.offset;
	fp->size = sh.size;
	fp->addr = sh.addr;

	return 0;
}

/*
 * Iter ELF Section Headers
 */
static int
iterelfshdrs(FILE *f, Fhdr *fp, iterFunc func)
{
	unsigned int i;

	if (fseek(f, fp->shoff, SEEK_SET) < 0)
		return -1;

	for (i = 0; i < fp->shnum; i++) {
		if (fp->classInterface->readelfshdr(f, fp) < 0)
			return -1;

		if(func(fp)) break;
	}

	return 0;
}

/*
 * Unpack ELF32 Program Header
 */
static int
unpackelf32phdr(uint8_t *buf, int len, Elf32_Phdr *ph, Fhdr *fp)
{
	uint8_t *p;

	if (len < Ph32sz)
		return -1;

	p = buf;

	p += fp->dataInterface->get32(p, &ph->type);
	p += fp->dataInterface->get32(p, &ph->offset);
	p += fp->dataInterface->get32(p, &ph->vaddr);
	p += fp->dataInterface->get32(p, &ph->paddr);
	p += fp->dataInterface->get32(p, &ph->filesz);
	p += fp->dataInterface->get32(p, &ph->memsz);
	p += fp->dataInterface->get32(p, &ph->flags);
	p += fp->dataInterface->get32(p, &ph->align);

	return p - buf;
}

/*
 * Read ELF32 Program Header
 */
static int
readelf32phdr(FILE *f, Fhdr *fp)
{
	uint8_t buf[Ph32sz];
	Elf32_Phdr ph;

	if (fread(buf, fp->phentsize, 1, f) != 1)
		return -1;

	if (unpackelf32phdr(buf, sizeof(buf), &ph, fp) < 0)
		return -1;

	// Do something here
	// ...

	return 0;
}

/*
 * Unpack ELF64 Program Header
 */
static int
unpackelf64phdr(uint8_t *buf, int len, Elf64_Phdr *ph, Fhdr *fp)
{
	uint8_t *p;

	if (len < Ph64sz)
		return -1;

	p = buf;

	p += fp->dataInterface->get32(p, &ph->type);
	p += fp->dataInterface->get32(p, &ph->flags);
	p += fp->dataInterface->get64(p, &ph->offset);
	p += fp->dataInterface->get64(p, &ph->vaddr);
	p += fp->dataInterface->get64(p, &ph->paddr);
	p += fp->dataInterface->get64(p, &ph->filesz);
	p += fp->dataInterface->get64(p, &ph->memsz);
	p += fp->dataInterface->get64(p, &ph->align);

	return p - buf;
}

/*
 * Read ELF64 Program Header
 */
static int
readelf64phdr(FILE *f, Fhdr *fp)
{
	uint8_t buf[Ph64sz];
	Elf64_Phdr ph;

	if (fread(buf, fp->phentsize, 1, f) != 1)
		return -1;

	if (unpackelf64phdr(buf, sizeof(buf), &ph, fp) < 0)
		return -1;

	// Do something here
	// ...
	
	return 0;
}

/*
 * Read ELF Program Headers
 */
static int
iterelfphdrs(FILE *f, Fhdr *fp, iterFunc func)
{
	unsigned int i;

	if (fseek(f, fp->phoff, SEEK_SET) < 0)
		return -1;

	for (i = 0; i < fp->phnum; i++) {
		if (fp->classInterface->readelfphdr(f, fp) < 0)
			return -1;

		if(func(fp)) break;
	}

	return 0;
}

/*
 * New Section
 */
static uint8_t*
newsection(FILE *f, uint64_t offset, uint64_t size)
{
	uint8_t *sect;

	if (size == 0)
		return NULL;

	sect = malloc(size);
	if (sect == NULL)
		return NULL;

	if (fseek(f, offset, SEEK_SET) < 0) {
		free(sect);
		return NULL;
	}

	if (fread(sect, size, 1, f) != 1) {
		free(sect);
		return NULL;
	}

	return sect;
}

/*
 * Read ELF SH String Table
 */
static int
readelfshstrtab(FILE *f, Fhdr *fp)
{
	if (fp->shstrndx == SHN_UNDEF) {
		fprintf(stderr, "missing string table\n");
		return -1;
	}

	if (fp->shstrndx >= fp->shnum) {
		fprintf(stderr, "shstrndx out of range\n");
		return -1;
	}

	if (fseek(f, fp->shoff + (uint64_t)fp->shstrndx * 
		fp->shentsize, SEEK_SET) < 0)
		return -1;

	if (fp->classInterface->readelfshdr(f, fp) < 0)
		return -1;

	fp->shstrtabsize = fp->size;
	fp->shstrtab = newsection(f, fp->offset, fp->shstrtabsize);
	if (fp->shstrtab == NULL)
		return -1;

	fp->shstrtab[fp->shstrtabsize - 1] = '\0';

	return 0;
}

/*
 * Get string from index in String Table
 */
static char * 
getshstr(Fhdr *fp, uint32_t i)
{
	if (fp->shstrtab == NULL)
		return NULL;

	if (i >= fp->shstrtabsize)
		return NULL;

	return (char*)&fp->shstrtab[i];
}

/*
 * Read ELF Section Headers
 */
uint8_t *
readelfsect(FILE *f, char *name, Fhdr *fp)
{
	unsigned int i;
	char *n;

	if (fseek(f, fp->shoff, SEEK_SET) < 0)
		return NULL;

	for (i = 0; i < fp->shnum; i++) {
		if (fp->classInterface->readelfshdr(f, fp) < 0)
			return NULL;
		n = getshstr(fp, fp->name);
		if (n == NULL)
			return NULL;
		if (strcmp(n, name) == 0)
			return newsection(f, fp->offset, fp->size);
	}

	fprintf(stderr, "section %s not found\n", name);

	return NULL;
}

/*
 * Read ELF String Table
 */
static int
readelfstrtab(FILE *f, Fhdr *fp)
{
	fp->strtabsize = fp->size;
	fp->strtab = readelfsect(f, ".strtab", fp);
	if (fp->strtab == NULL)
		return -1;
	
	return 0;
}

/*
 * Read ELF Symbol Table
 */
static int
readelfsymtab(FILE *f, Fhdr *fp)
{
	fp->symtabsize = fp->size;
	fp->symtab = readelfsect(f, ".symtab", fp);
	if (fp->symtab == NULL)
		return -1;
	
	return 0;
}

/*
 * Read ELF32 Symbol
 */
static int
readelf32syment(uint8_t *buf, Fhdr *fp)
{
	Elf32_Sym sym;
	uint8_t *p;

	p = buf;

	p += fp->dataInterface->get32(p, &sym.name);
	p += fp->dataInterface->get32(p, &sym.value);
	p += fp->dataInterface->get32(p, &sym.size);
	p += fp->dataInterface->get8(p, &sym.info);
	p += fp->dataInterface->get8(p, &sym.other);
	p += fp->dataInterface->get16(p, &sym.shndx);

	fp->symname = sym.name;
	fp->value = sym.value;

	return p - buf;
}

/*
 * Read ELF64 Symbol
 */
static int
readelf64syment(uint8_t *buf, Fhdr *fp)
{
	Elf64_Sym sym;
	uint8_t *p;

	p = buf;

	p += fp->dataInterface->get32(p, &sym.name);
	p += fp->dataInterface->get8(p, &sym.info);
	p += fp->dataInterface->get8(p, &sym.other);
	p += fp->dataInterface->get16(p, &sym.shndx);
	p += fp->dataInterface->get64(p, &sym.value);
	p += fp->dataInterface->get64(p, &sym.size);

	fp->symname = sym.name;
	fp->value = sym.value;

	return p - buf;
}

/*
 * Get string from index in String Table
 */
static char * 
getstr(Fhdr *fp, uint32_t i)
{
	if (fp->strtab == NULL)
		return NULL;

	if (i >= fp->strtabsize)
		return NULL;

	return (char*)&fp->strtab[i];
}

/*
 * Read ELF Symbol
 */
int
readelfsym(FILE *f, char *name, Fhdr *fp)
{
	unsigned int i, num;
	char *n;
	
	if ((fp->strtab == NULL) || (fp->symtab == NULL))
		return -1;

	for (i = 0; i < fp->symtabsize;) {
		num = fp->classInterface->readelfsyment(fp->symtab + i, fp);
		if (num < 0)
			return -1;
		i += num;
		n = getstr(fp, fp->symname);
		printf("%s %d %d %d\r\n", n, fp->symname, num, i);
		if (n == NULL)
			return -1;
		if (strcmp(n, name) == 0)
			return 0;
	}
}

/*
 * Read ELF File
 */
int
readelf(FILE *f, Fhdr *fp)
{
	memset(fp, 0, sizeof(*fp));

	if (readident(f, fp) < 0)
		return -1;

	if (fp->classInterface->readelfehdr(f, fp) < 0)
		return -1;

	if (readelfshstrtab(f, fp) < 0)
		return -1;

	if (readelfstrtab(f, fp) < 0)
		goto SHSTR;

	if (readelfsymtab(f, fp) < 0)
		goto STR;

	return 0;

STR:
	free(fp->strtab);
SHSTR:
	free(fp->shstrtab);

	return -1;
}

/*
 * Free String Table
 */
void freeelf(Fhdr *fp)
{
	if (fp->shstrtab != NULL)
		free(fp->shstrtab);
	if (fp->strtab != NULL)
		free(fp->strtab);
	if (fp->symtab != NULL)
		free(fp->symtab);
}
