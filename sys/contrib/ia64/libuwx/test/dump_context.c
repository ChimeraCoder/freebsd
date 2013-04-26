
/*
 * You may redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
	/*  0 */ "ip", "sp", "bsp", "cfm",
	/*  4 */ "rp", "psp", "pfs", "preds",
	/*  8 */ "priunat", "ar.bspstore", "ar.rnat", "ar.unat",
	/* 12 */ "ar.fpsr", "ar.lc", "ar.pfs", "(pad)",
	/* 16 */ "gr4", "gr5", "gr6", "gr7",
	/* 20 */ "br1", "br2", "br3", "br4", "br5"    };
    static int col1[] = {
	IP,
	SP,
	BSP,
	CFM,
	RP,
	PSP,
	PFS,
	AR_RNAT,
	AR_UNAT,
	AR_FPSR,
	AR_LC,
	AR_PFS,
    };
    static int col2[] = {
	PREDS,
	PRIUNAT,
	GR4,
	GR5,
	GR6,
	GR7,
	BR1,
	BR2,
	BR3,
	BR4,
	BR5,
    };

#define NCOL1 (sizeof(col1)/sizeof(int))
#define NCOL2 (sizeof(col2)/sizeof(int))
#define NPRINT (NCOL1 > NCOL2 ? NCOL1 : NCOL2)

    valid = (unsigned int)(context[0] >> 32);
    printf("  valid_regs (%08lx):", valid);
    for (i = 0; i <= BR5; i++) {
	if (valid & 1) printf(" %s", names[i]);
	valid >>= 1;
    }
    printf("\n");
    for (i = 0; i < NPRINT; i++) {
	if (i < NCOL1) {
	    j = col1[i];
	    val = context[j+1];
	    printf("  %-8s %08x %08x", names[j],
			(unsigned int)(val >> 32),
			(unsigned int)val);
	}
	else
	    printf("                            ");
	if (i < NCOL2) {
	    j = col2[i];
	    val = context[j+1];
	    printf("      %-8s %08x %08x", names[j],
			(unsigned int)(val >> 32),
			(unsigned int)val);
	}
	putchar('\n');
    }
}