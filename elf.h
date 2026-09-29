#ifndef __ELF_H__
#define __ELF_H__

typedef struct Fhdr Fhdr;
typedef int (*iterFunc)(Fhdr*);

typedef struct {
	int type;
	char *name;
	unsigned int (*get8)(void*, uint8_t*);
	unsigned int (*get16)(void*, uint16_t*);
	unsigned int (*get32)(void*, uint32_t*);
	unsigned int (*get64)(void*, uint64_t*);
} st_dataInterface_t;

typedef struct {
	int type;
	char *name;
	int ehsize;
	int shentsize;
	int phentsize;
	int (*readelfehdr)(FILE*, Fhdr*);
	int (*readelfshdr)(FILE*, Fhdr*);
	int (*readelfphdr)(FILE*, Fhdr*);
	int (*readelfsyment)(uint8_t*, Fhdr*);
} st_classInterface_t;

/*
 * Portable ELF file header
 */
struct Fhdr {
	/* ELF Data */
	st_dataInterface_t *dataInterface;

	/* ELF Class */
	st_classInterface_t *classInterface;

	/* ELF Identification */
	uint8_t		elfclass;		/* File class */
	uint8_t		elfdata;		/* Data encoding */
	uint8_t		elfversion;		/* File version */
	uint8_t		osabi;			/* Operating system/ABI identification */
	uint8_t		abiversion;		/* ABI version */

	/* ELF Header */
	uint16_t	type;
	uint16_t	machine;
	uint32_t	version;
	uint64_t	entry;
	uint64_t	phoff;
	uint64_t	shoff;
	uint16_t	ehsize;		/* ELF Header size */
	uint16_t	phentsize;	/* Section Header size */
	uint16_t	phnum;
	uint16_t	shentsize;	/* Program Header size */
	uint16_t	shnum;
	uint16_t	shstrndx;

	/* Section Header */
	uint32_t	name;
	uint64_t	offset;
	uint64_t	size;
	uint64_t	addr;

	/* Program Header */
	// ...
	
	/* SH String Table */
	uint64_t	shstrtabsize;	/* String Table size */
	uint8_t		*shstrtab;		/* Copy of String Table */

	/* String Table */
	uint64_t	strtabsize;
	uint8_t		*strtab;

	/* Symbol Table */
	uint64_t	symtabsize;
	uint8_t		*symtab;

	/* Symbol */
	uint32_t	symname;
	uint64_t	value;
};

/* Read */
uint8_t *readelfsect(FILE *f, char *name, Fhdr *fp);
int readelfsym(FILE *f, char *name, Fhdr *fp);
int readelf(FILE *f, Fhdr *fp);
void freeelf(Fhdr *fp);

#endif /* __ELF_H__ */