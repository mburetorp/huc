/*	File code.c: 2.2 (84/08/31,10:05:13) */
/*% cc -O -c %
 *
 */

#include <stdio.h>
#include <time.h>

#include "defs.h"
#include "data.h"
#include "code.h"
#include "error.h"
#include "function.h"
#include "io.h"
#include "main.h"
#include "optimize.h"

/* locals */
long segment;

/* externs */
extern long arg_stack_flag;

/*
 *	print all assembler info before any code is generated
 *
 */
void gdata (void )
{
	if (segment == 1) {
		segment  = 0;
		ol (".bss");
	}
}

void gtext (void )
{
	if (segment == 0) {
		segment  = 1;
		ol (".code");
	}
}

void header (void )
{
time_t today;

	outstr("; Small C HuC6280 (v0.2, 08/Nov/97)\n;\n");
	outstr("; Taken to HuC (22/Feb/00)\n;\n");
	outstr(";\n");
	outstr("; This file generated by ");
	outstr(HUC_VERSION);
	outstr("\n");
	outstr("; on ");
	time(&today);
	outstr(ctime(&today));
	outstr(";\n");
	outstr("\n");
	outstr("HUC\t= 1");
	nl ();
}

void asmdefines (void)
{
	outstr(asmdefs);
}

void inc_startup (void )
{
	if (startup_incl == 0)
	{
		startup_incl = 1;

		nl();
		outstr("\t.include \"startup.asm\"\n");
		outstr("\t.data\n");
		outstr("\t.bank DATA_BANK\n\n");
		gtext ();
		nl ();
	}
}

/*
 *	print pseudo-op  to define a byte
 *
 */
void defbyte (void )
{
	ot (".db\t");
}

/*
 *	print pseudo-op to define storage
 *
 */
void defstorage (void )
{
	ot (".ds\t");
}

/*
 *	print pseudo-op to define a word
 *
 */
void defword (void )
{
	ot (".dw\t");
}

/*
 *	output instructions
 *
 */
void out_ins(long code, long type, long data)
{
	INS tmp;

	tmp.code = code;
	tmp.type = type;
	tmp.data = data;
	gen_ins(&tmp);
}

void out_ins_ex(long code, long type, long data, long imm)
{
	INS tmp;

	tmp.code = code;
	tmp.type = type;
	tmp.data = data;
	tmp.imm  = imm;
	gen_ins(&tmp);
}

void out_ins_sym(long code, long type, long data, SYMBOL *sym)
{
	INS tmp;

	tmp.code = code;
	tmp.type = type;
	tmp.data = data;
	tmp.sym  = sym;
	gen_ins(&tmp);
}

void gen_ins(INS *tmp)
{
	if (optimize)
		push_ins(tmp);
	else {
		if (arg_stack_flag)
			arg_push_ins(tmp);
		else
			gen_code(tmp);
	}
}

/*
 *	gen assembly code
 *
 */
void gen_code(INS *tmp)
{
	long code;
	long type;
	long data;
	long imm;

	code = tmp->code;
	type = tmp->type;
	data = tmp->data;
	imm  = tmp->imm;

	if (type == T_NOP)
		return;

	switch (code) {
	case I_FARPTR:
		ot("__farptr\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		}
		outstr(",");
		outsymbol((char *)tmp->arg[0]);
		outstr(",");
		outsymbol((char *)tmp->arg[1]);
		nl();
		break;

	case I_FARPTR_I:
		ot("__farptr_i\t");
		outsymbol((char *)data);
		outstr(",");
		outsymbol((char *)tmp->arg[0]);
		outstr(",");
		outsymbol((char *)tmp->arg[1]);
		nl();
		break;

	case I_FARPTR_GET:
		ot("__farptr_get\t");
		outsymbol((char *)tmp->arg[0]);
		outstr(",");
		outsymbol((char *)tmp->arg[1]);
		nl();
		break;

	case I_FGETB:
		ot("__farptr_i\t");
		outsymbol((char *)data);
		nl();
		ol("__fgetb");
		break;

	case I_FGETUB:
		ot("__farptr_i\t");
		outsymbol((char *)data);
		nl();
		ol("__fgetub");
		break;

	case I_FGETW:
		ot("__farptr_i\t");
		outsymbol((char *)data);
		nl();
		ol("  jsr\t_farpeekw.fast");
		break;

	case I_VGETW:
		ol("__ldw\tvideo_data");
		break;

	case I_VPUTW:
		ol("__stw\tvideo_data");
		break;

	case I_NARGS:
		ot("  ldy\t#");
		outdec(data);
		nl();
		break;

	case I_LDB:
		ot("__ldb\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		default:
			outsymbol((char *)data);
			break;
		}
		nl();
		break;

	case I_LDUB:
		ot("__ldub\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		default:
			outsymbol((char *)data);
			break;
		}
		nl();
		break;

	case I_LDBP:
		ol("__ldbp\t__ptr");
		break;

	case I_LDUBP:
		ol("__ldubp\t__ptr");
		break;

	case I_LDW:
		ot("__ldw\t");

		switch (type) {
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_LABEL:
			outlabel(data);
			break;
		case T_STACK:
			outstr("__stack");
			break;
		}
		nl();
		break;

	case I_LDWI:
		ot("__ldwi\t");

		switch (type) {
		case T_VALUE:
			outdec(data);
			break;
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_STRING:
			outlabel(litlab);
			outbyte('+');
			outdec(data);
			break;
		case T_BANK:
			outstr("BANK(");
			outstr((char *)data);
			outstr(")");
			break;
		case T_VRAM:
			outstr("VRAM(");
			outstr((char *)data);
			outstr(")");
			break;
		case T_PAL:
			outstr("PAL(");
			outstr((char *)data);
			outstr(")");
			break;
		}
		nl();
		break;

	case I_LDWP:
		ol("__ldwp\t__ptr");
		break;

	case I_STB:
		ot("  stx\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		default:
			outsymbol((char *)data);
			break;
		}
		nl();
		break;

	case I_STBPS:
		ol("__stbps");
		break;

	case I_STW:
		ot("__stw\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_PTR:
			outstr("__ptr");
			break;
		}
		nl();
		break;
	
	case I_STWPS:
		ol("__stwps");
		break;

	case I_ADDW:
		ot("__addw\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_PTR:
			outstr("__ptr");
			break;
		}
		nl();
		break;

	case I_ADDWI:
	case I_ADDBI:
		if (code == I_ADDBI)
			ot("__addbi\t");
		else
			ot("__addwi\t");

		switch (type) {
		case T_VALUE:
			outdec(data);
			break;
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_STRING:
			outlabel(litlab);
			outbyte('+');
			outdec(data);
			break;
		case T_BANK:
			outstr("BANK(");
			outstr((char *)data);
			outstr(")");
			break;
		case T_VRAM:
			outstr("VRAM(");
			outstr((char *)data);
			outstr(")");
			break;
		case T_PAL:
			outstr("PAL(");
			outstr((char *)data);
			outstr(")");
			break;
		}
		nl();
		break;

	case I_ADDWS:
		ol("__addws");
		break;

	case I_ADDBS:
		ol("__addbs");
		break;

	case I_ADDMI:
		ot("__addmi\t");
		outdec(data);
		outstr(",__stack");
		nl();
		break;

	case I_SUBW:
		ot("__subw\t");

		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_PTR:
			outstr("__ptr");
			break;
		}
		nl();
		break;

	case I_SUBWI:
		ot("__subwi\t");
		outdec(data);
		nl();
		break;

	case I_SUBWS:
		ol("__subws");
		break;

	case I_ORWI:
		ot("__orwi\t");
		outdec(data);
		nl();
		break;

	case I_STBIPP:
		ot("__stbipp\t");
		outdec(data);
		nl();
		break;

	case I_STWIPP:
		ot("__stwipp\t");
		outdec(data);
		nl();
		break;

	case I_ORWS:
		ol("__orws");
		break;

	case I_EORWS:
		ol("__eorws");
		break;

	case I_ANDWI:
		ot("__andwi\t");
		outdec(data);
		nl();
		break;

	case I_ANDWS:
		ol("__andws");
		break;

	case I_ASLW:
		ol("__aslw");
		break;

	case I_ASLWS:
		ol("__aslws");
		break;

	case I_ASRW:
		ol("__asrw");
		break;

	case I_COMW:
		ol("__comw");
		break;

	case I_NEGW:
		ol("__negw");
		break;

	case I_SWAPW:
		ol("__swapw");
		break;

	case I_EXTW:
		ol("__extw");
		break;

	case I_BOOLW:
		ol("__boolw");
		break;

	case I_NOTW:
		ol("__notw");
		break;

	case I_JMP:
		ot("  jmp\t");
		outsymbol((char *)data);
		nl();
		break;

	case I_JSR:
		ot("  jsr\t");

		switch (type) {
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_LIB:
			outstr((char *)data);
			break;
		}
		nl();
		break;

	case I_CALL:
		switch (type) {
		case T_SYMBOL:
			ot("  call\t");
			outsymbol((char *)data);
			if (imm) {
				outstr(".");
				outdec(imm);
			}
			break;
		case T_LIB:
			ot("  jsr\t");
			outstr((char *)data);
			break;
		}
		nl();
		break;

	case I_CALLS:
		ot("__calls\t");
		outdec(data);
		nl();
		break;

	case I_RTS:
		ol("  rts");
		break;

	case I_PUSHW:
		ol("__pushw");
		break;

	case I_PHB:
		ot("__phb\t");
		outdec(data);
		if (imm) {
			outstr(",");
			outdec(imm);
		}
		nl();
		break;

	case I_PHW:
		ot("__phw\t");
		outdec(data);
		if (imm) {
			outstr(",");
			outdec(imm);
		}
		nl();
		break;

	case I_POPW:
		ol("__popw");
		break;

	case I_TSTW:
		ol("__tstw");
		break;

	case I_LBRA:
		ot("__lbra\t");
		outlabel(data);
		nl();
		break;

	case I_LBEQ:
		ot("__lbeq\t");
		outlabel(data);
		nl();
		break;

	case I_LBNE:
		ot("__lbne\t");
		outlabel(data);
		nl();
		break;

	case I_BANK:
		ot(".bank\t");
		switch (type) {
		case T_VALUE:
			outdec(data);
			break;
		default:
			error("bank type switching not handled");
			break;
		}
		nl();
		break;

	case I_OFFSET:
		ot(".org\t");
		switch (type) {
		case T_VALUE:
			outhex((unsigned short)data);
			break;
		default:
			error("org type setting not handled");
			break;
		}
		nl();
		break;

	case I_INCW:
		ot("incw\t");
		switch (type) {
		case T_LABEL:
			outlabel(data);
			break;
		case T_SYMBOL:
			outsymbol((char *)data);
			break;
		case T_PTR:
			outstr("__ptr");
			break;
		}
		nl();
		break;

	case I_LABEL:
		outlabel(data);
		col();
		nl();
		break;

	case I_MULWI:
		ot("__mulwi\t");
		outdec(data);
		nl();
		break;

	default:
		gen_asm(tmp);
		break;
	}
}
