#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/

/* Hardware & software constants */
#define PAGESIZE		  4096			/* page size in bytes	*/
#define WORDLEN			  4				  /* word size in bytes	*/
#define BYTELEN	8


/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR		0x10000000
#define RAMBASESIZE		0x10000004
#define TODLOADDR		  0x1000001C
#define INTERVALTMR		0x10000020	
#define TIMESCALEADDR	0x10000024


/* utility constants */
#define	TRUE			    1
#define	FALSE			    0
#define HIDDEN			  static
#define EOS				    '\0'
#define ON			    1
#define OFF                        0
#define NULL 			    ((void *)0xFFFFFFFF)

/* device interrupts */
#define DISKINT			  3
#define FLASHINT 		  4
#define NETWINT 		  5
#define PRNTINT 		  6
#define TERMINT			  7

#define DEVINTNUM		  5		  /* interrupt lines used by devices */
#define DEVPERINT		  8		  /* devices per interrupt line */
#define DEVREGLEN		  4		  /* device register field length in bytes, and regs per dev */	
#define DEVREGSIZE	  16 		/* device register size in bytes */

/* device register field number for non-terminal devices */
#define STATUS			  0
#define COMMAND			  1
#define DATA0			    2
#define DATA1			    3

/* device register field number for terminal devices */
#define RECVSTATUS  		0
#define RECVCOMMAND 		1
#define TRANSTATUS  		2
#define TRANCOMMAND 		3

/* device common STATUS codes */
#define UNINSTALLED		0
#define READY			    1
#define BUSY			    3
#define CHARRECV 5

/* device common COMMAND codes */
#define RESET			    0
#define ACK				    1

/* Memory related constants */
#define KSEG0           	0x00000000
#define KSEG1           	0x20000000
#define KSEG2           	0x40000000
#define KUSEG           	0x80000000
#define RAMSTART        	0x20000000
#define BIOSDATAPAGE    	0x0FFFF000
#define PASSUPVECTOR	 	0x0FFFF900
#define STACKADDRESS 	0x20001000
#define EXCEPTSTATEADDR 	0x0FFFF000
#define BUSADDRESS 	0x10000000
#define POOLSTARTADDR 0x20020000


/* Pcb Constants */
#define MAXPROC		20

/* operations */
#define	MIN(A,B)		((A) < (B) ? A : B)
#define MAX(A,B)		((A) < (B) ? B : A)
#define	ALIGNED(A)		(((unsigned)A & 0x3) == 0)

/* Macro to load the Interval Timer */
#define LDIT(T)	((* ((cpu_t *) INTERVALTMR)) = (T) * (* ((cpu_t *) TIMESCALEADDR))) 

/* Macro to read the TOD clock */
#define STCK(T) ((T) = ((* ((cpu_t *) TODLOADDR)) / (* ((cpu_t *) TIMESCALEADDR))))

/*Time Constants*/
#define ITSECOND 	100000
#define TIMESLICE 	5000

/*BIT Patterns*/
#define ALLOFF 	0
#define IMON 	0x0000FF00
#define TEON 	0x08000000
#define IECON 	0x00000001
#define IEPON 	0x00000004
#define KUPON 	0x00000008
#define EXMASK 	0x0000007C
#define ITINTERRUPT 	0x00000400
#define PLTINTERRUPT 	0x00000200
#define DISKINTERRUPT 	0x00000800
#define FLASHINTERRUPT 	0x00001000
#define NETWORKINTERRUPT	0x00002000
#define PRINTERINTERRUPT 	0x00004000
#define TERMINTERRUPT 	0x00008000

/*LineNo Constants*/
#define PROCESSOR 0
#define INTERVALTIMER 1
#define PLTIMER 2
#define DISK 3
#define FLASH 4
#define NETWORK 5
#define PRINTER 6
#define TERMINAL 7

/*Device Interrupt Masks*/
#define DEV0 	0x00000001
#define DEV1 	0x00000002
#define DEV2 	0x00000004
#define DEV3 	0x00000008
#define DEV4 	0x00000010
#define DEV5 	0x00000020
#define DEV6 	0x00000040
#define DEV7 	0x00000080

/*Exception related Constants*/
#define PGFAULTEXCEPT 	0
#define GENERALEXCEPT 	1
#define SYSCALLEXCEPT 	8
#define IOEXCEPT 	0
#define TLBREFILLEXCEPT 3


/*SYSCALL CODES*/
#define CREATEPROCESS 	1
#define TERMPROCESS 	2
#define PASSEREN 	3
#define VERHOGEN 	4
#define WAITFORIO 	5
#define GETCPUT 	6
#define WAITFORCLOCK 	7
#define GETSUPPORTT 	8
#define UTERMINATE 9
#define GETTOD 10
#define PRNTRW 11
#define TERMW 12
#define TERMR 13

/*Mneumonic Constants*/
#define PCINCREMENT 	4
#define SEMCOUNT 	49
#define ERRORCODE -1
#define SUCCESS 0
#define NONPERIPHERALDEV 3
#define DEVPERLINE 8

/*Support Level Constants*/
#define UPROCMAX 1
#define POOLSIZE 20
#define PGMAX 32
#define STARTPGNO 0x80000000
#define DRON 0x00000400
#define PRINTCHR	2
#define RECVD	5
#define TERMSTATMASK	0xFF
#define ENTRYHISHIFT 5
#define DIRTYON 0x00000400
#define CHARTRANSM 5
#define TEXTADDR 0x800000B0
#define STACKPAGEADDR 0xC0000000
#define TLBMOD 1
#define READBLK 2
#define WRITEBLK 3
#define VALIDON 0x00000200
#define READ 1
#define WRITE 0
#define VALIDMASK 0x00000200
#define VPNMASK 0xFFFFF000
#define VALIDOFF 0xFFFFFDFF
#define STARTVPN 0x02000000
#define VPNSHIFT 12
#define IECOFF 0xFFFFFFFE
#define POOLMAX 20
#define FRAMESHIFT 8
#endif
