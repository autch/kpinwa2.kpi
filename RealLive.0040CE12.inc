/* モノラルか否か。ESP を変更後にアクセスするため、static の必要あり。 */
static unsigned char is_mono;
/* モノラルか否か調べる。 */
is_mono = sChannels == 1 ? 1 : 0;

/* RealLive.exe のコードを実行するための仮想スタック。 */
char virtual_stack[0x48];
/* 仮想スタックの値を埋める。 */
*(unsigned int *)&virtual_stack[0x10] = (unsigned int)&pOutput[0];
*(unsigned int *)&virtual_stack[0x14] = 0; /* 処理中のチャンネル。 */
*(  signed int *)&virtual_stack[0x18] = *(signed short *)&pSrcBuffer[0];
*(unsigned int *)&virtual_stack[0x20] = 0;
*(unsigned int *)&virtual_stack[0x24] = nDstSamples;
*(  signed int *)&virtual_stack[0x28] = *(signed short *)&pSrcBuffer[2];
*(unsigned int *)&virtual_stack[0x2C] = 0;
*(unsigned int *)&virtual_stack[0x38] = is_mono; /* ここはとりあえず。実際は多分 mono フラグではない。 */
*(unsigned int *)&virtual_stack[0x3C] = 0;
*(unsigned int *)&virtual_stack[0x40] = nDstSamples;
*(unsigned int *)&virtual_stack[0x44] = 0; /* 生成したサンプル数。 */

/* ESP を待避させる変数。ESP を変更後にアクセスするため、static の必要あり。 */
static unsigned int orig_stack_addr;

__asm
{
	; 開始処理。
	PUSH	EBP
	
	; 各種レジスタを設定する。
	XOR		EAX, EAX
	XOR		EDX, EDX
	
	MOV		ESI, [pSrcBuffer]
	ADD		ESI, 2
	
	TEST	is_mono, 1
	JNZ		L1
	ADD		ESI, 2
L1:
	
	; スタックを待避し仮想スタックに切り替える。
	MOV		orig_stack_addr, ESP
	LEA		ESP, virtual_stack

	; --------------------------------------------
	; ここより RealLive.exe から抜き出したコード。
	; --------------------------------------------
	MOV		ECX, DWORD PTR SS:[ESP + 0x24]
	MOV		DWORD PTR SS:[ESP + 0x2C], EAX
	CMP		ECX, EAX
	JNG		L_0040D3B8
	MOV		EAX, DWORD PTR SS:[ESP + 0x20]
L_0040CE26:
	MOV		ECX, DWORD PTR SS:[ESP + 0x14]
	MOV		EBP, DWORD PTR SS:[ESP + 0x28]
	TEST	ECX, ECX
	JNZ		L_0040CE36
	MOV		EBP, DWORD PTR SS:[ESP + 0x18]
L_0040CE36:
	TEST	EAX, EAX
	JZ		L_0040CE44
	DEC		EAX
	MOV		DWORD PTR SS:[ESP + 0x20], EAX
	JMP		L_0040D073
L_0040CE44:
	CMP		EDX, 8
	JL		L_0040CE4D
	INC		ESI
	AND		EDX, 7
L_0040CE4D:
	XOR		EDI, EDI
	MOV		CL, DL
	MOV		DI, WORD PTR DS:[ESI]
	ADD		EDX, 3
	SHR		EDI, CL
	AND		EDI, 7
	CMP		EDI, 4
	JNL		L_0040CF84
	TEST	EDI, EDI
	JNZ		L_0040CEE0
	MOV		ECX, DWORD PTR SS:[ESP + 0x38]
	TEST	ECX, ECX
	JZ		L_0040D073
	CMP		EDX, 8
	JL		L_0040CE7E
	INC		ESI
	AND		EDX, 7
L_0040CE7E:
	XOR		EAX, EAX
	MOV		CL, DL
	MOV		AX, WORD PTR DS:[ESI]
	SHR		EAX, CL
	AND		EAX, 1
	INC		EDX
	CMP		EAX, 1
	MOV		DWORD PTR SS:[ESP + 0x20], EAX
	JNZ		L_0040D073
	CMP		EDX, 8
	JL		L_0040CEA1
	INC		ESI
	AND		EDX, 7
L_0040CEA1:
	XOR		EAX, EAX
	MOV		CL, DL
	MOV		AX, WORD PTR DS:[ESI]
	ADD		EDX, 2
	SHR		EAX, CL
	AND		EAX, 3
	CMP		EAX, 3
	MOV		DWORD PTR SS:[ESP + 0x20], EAX
	JNZ		L_0040D073
	CMP		EDX, 8
	JL		L_0040CEC6
	INC		ESI
	AND		EDX, 7
L_0040CEC6:
	XOR		EAX,EAX
	MOV		CL, DL
	MOV		AX, WORD PTR DS:[ESI]
	SHR		EAX, CL
	AND		EAX, 0xFF
	ADD		EDX, 8
	MOV		DWORD PTR SS:[ESP + 0x20], EAX
	JMP		L_0040D073
L_0040CEE0:
	CMP		EDI, 1
	JNZ		L_0040CF18
	CMP		EDX, 8
	JL		L_0040CEEE
	INC		ESI
	AND		EDX, 7
L_0040CEEE:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 7
	SHR		EBX, CL
	AND		EBX, 0x7F
	TEST	BL, 0x40
	JZ		L_0040CF0F
	AND		EBX, 0x3F
	SHL		EBX, 2
	SUB		EBP, EBX
	JMP		L_0040D073
L_0040CF0F:
	LEA     EBP,DWORD PTR [EBP + EBX * 4]
	JMP		L_0040D073
L_0040CF18:
	CMP		EDI, 2
	JNZ		L_0040CF50
	CMP		EDX, 8
	JL		L_0040CF26
	INC		ESI
	AND		EDX, 7
L_0040CF26:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 7
	SHR		EBX, CL
	AND		EBX, 0x7F
	TEST	BL, 0x40
	JZ		L_0040CF47
	AND		EBX, 0x3F
	SHL		EBX, 3
	SUB		EBP, EBX
	JMP		L_0040D073
L_0040CF47:
	LEA		EBP, DWORD PTR SS:[EBP + EBX * 8]
	JMP		L_0040D073
L_0040CF50:
	CMP		EDX, 8
	JL		L_0040CF59
	INC		ESI
	AND		EDX, 7
L_0040CF59:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 7
	SHR		EBX, CL
	AND		EBX, 0x7F
	TEST	BL, 0x40
	JZ		L_0040CF7A
	AND		EBX, 0x3F
	SHL		EBX, 4
	SUB		EBP, EBX
	JMP		L_0040D073
L_0040CF7A:
	SHL		EBX, 4
	ADD		EBP, EBX
	JMP		L_0040D073
L_0040CF84:
	JNZ		L_0040CFBA
	CMP		EDX, 8
	JL		L_0040CF8F
	INC		ESI
	AND		EDX, 7
L_0040CF8F:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 7
	SHR		EBX, CL
	AND		EBX, 0x7F
	TEST	BL, 0x40
	JZ		L_0040CFB0
	AND		EBX, 0x3F
	SHL		EBX, 5
	SUB		EBP, EBX
	JMP		L_0040D073
L_0040CFB0:
	SHL		EBX, 5
	ADD		EBP, EBX
	JMP		L_0040D073
L_0040CFBA:
	CMP		EDI, 5
	JNZ		L_0040CFF3
	CMP		EDX, 8
	JL		L_0040CFC8
	INC		ESI
	AND		EDX, 7
L_0040CFC8:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 7
	SHR		EBX, CL
	AND		EBX, 0x7F
	TEST	BL, 0x40
	JZ		L_0040CFE9
	AND		EBX, 0x3F
	SHL		EBX, 6
	SUB		EBP, EBX
	JMP		L_0040D073
L_0040CFE9:
	SHL		EBX, 6
	ADD		EBP, EBX
	JMP		L_0040D073
L_0040CFF3:
	CMP		EDI, 6
	JNZ		L_0040D026
	CMP		EDX, 8
	JL		L_0040D001
	INC		ESI
	AND		EDX, 7
L_0040D001:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 7
	SHR		EBX, CL
	AND		EBX, 0x7F
	TEST	BL, 0x40
	JZ		L_0040D01F
	AND		EBX, 0x3F
	SHL		EBX, 7
	SUB		EBP, EBX
	JMP     L_0040D073
L_0040D01F:
	SHL		EBX, 7
	ADD		EBP, EBX
	JMP		L_0040D073
L_0040D026:
	CMP		EDX, 8
	JL		L_0040D02F
	INC		ESI
	AND		EDX, 7
L_0040D02F:
	XOR		EDI, EDI
	MOV		CL, DL
	MOV		DI, WORD PTR DS:[ESI]
	SHR		EDI, CL
	AND		EDI, 1
	INC		EDX
	TEST	EDI, EDI
	JNZ		L_0040D071
	CMP		EDX, 8
	JL		L_0040D049
	INC		ESI
	AND		EDX, 7
L_0040D049:
	XOR		EBX, EBX
	MOV		CL, DL
	MOV		BX, WORD PTR DS:[ESI]
	ADD		EDX, 8
	SHR		EBX, CL
	AND		EBX, 0xFF
	TEST	BL, 0x80
	JZ		L_0040D06A
	AND		EBX, 0x7F
	SHL		EBX, 9
	SUB		EBP, EBX
	JMP		L_0040D073
L_0040D06A:
	SHL		EBX, 9
	ADD		EBP, EBX
	JMP		L_0040D073
L_0040D071:
	XOR		EBP, EBP
L_0040D073:
	MOV		EDI, DWORD PTR SS:[ESP + 0x2C]
	MOV		ECX, DWORD PTR SS:[ESP + 0x3C]
	CMP		EDI, ECX
	JL		L_0040D0AA
	MOV		EAX, DWORD PTR SS:[ESP + 0x40]
	MOV		ECX, DWORD PTR SS:[ESP + 0x44]
	CMP		ECX, EAX
	JNL		L_0040D3B8
	MOV		ECX, DWORD PTR SS:[ESP + 0x10]
	MOV		EAX, DWORD PTR SS:[ESP + 0x44]
	MOV		WORD PTR DS:[ECX], BP
	ADD		ECX, 2
	INC		EAX
	MOV		DWORD PTR SS:[ESP + 0x10], ECX
	MOV		DWORD PTR SS:[ESP + 0x44], EAX
	MOV		EAX, DWORD PTR SS:[ESP + 0x20]
L_0040D0AA:
	MOV		ECX, DWORD PTR SS:[ESP + 0x14]
	TEST	ECX,ECX
	JZ		L_0040D0B8
	MOV		DWORD PTR SS:[ESP + 0x28], EBP
	JMP		L_0040D0BC
L_0040D0B8:
	MOV		DWORD PTR SS:[ESP + 0x18], EBP
L_0040D0BC:
	MOV		EBP, DWORD PTR SS:[ESP + 0x14]
	MOV		ECX, DWORD PTR SS:[ESP + 0x24]
	XOR		EBP, 1
	INC		EDI
	;
	; ここより追加。
	;
	TEST	is_mono, 1
	JZ		L2
	XOR		EBP, EBP
L2:
	;
	; ここまで追加。
	;
	CMP		EDI, ECX
	MOV		DWORD PTR SS:[ESP + 0x14], EBP
	MOV		DWORD PTR SS:[ESP + 0x2C], EDI
	JL		L_0040CE26
L_0040D3B8:
	; --------------------------------------------
	; ここまで RealLive.exe から抜き出したコード。
	; --------------------------------------------

	; 待避しておいたスタックに切り替える。
	MOV		ESP, orig_stack_addr
	
	; 後処理。
	POP		EBP
}
