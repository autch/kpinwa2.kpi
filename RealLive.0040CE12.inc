/* モノラルか否か。ESP を変更後にアクセスするため、static の必要あり。 */
static unsigned char is_mono;
/* モノラルか否か調べる。 */
is_mono = sChannels == 1 ? 1 : 0;

/* RealLive.exe のコードを実行するための仮想スタック。 */
char virtual_stack[0x48];
/* 仮想スタックの値を埋める。 */
*(unsigned int *)&virtual_stack[0x10] = (unsigned int)&pOutput[0];
*(unsigned int *)&virtual_stack[0x14] = 0; /* 処理中のチャンネル。nChannel */
*(  signed int *)&virtual_stack[0x18] = *(signed short *)&pSrcBuffer[0]; // 処理中のチャンネルのサンプル
*(unsigned int *)&virtual_stack[0x20] = 0;                               // ランの長さ，nLength
*(unsigned int *)&virtual_stack[0x24] = nDstSamples;
*(  signed int *)&virtual_stack[0x28] = *(signed short *)&pSrcBuffer[2]; // 処理中のチャンネルの次のサンプル
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
	
	MOV		ESI, [pSrcBuffer]                  ; ESI = pSrcBuffer + 2; // そのチャンネルの次のサンプル
	ADD		ESI, 2
	
	TEST	is_mono, 1                         ; if(is_mono & 1)
	JNZ		L1                                 ; {
	ADD		ESI, 2                             ;   esi += 2;
L1:                                            ; }
	
	; スタックを待避し仮想スタックに切り替える。
	MOV		orig_stack_addr, ESP
	LEA		ESP, virtual_stack

	; --------------------------------------------
	; ここより RealLive.exe から抜き出したコード。
	; --------------------------------------------
	MOV		ECX, DWORD PTR SS:[ESP + 0x24]     ; ECX = nDstSamples
	MOV		DWORD PTR SS:[ESP + 0x2C], EAX     ; S2C = 0
	CMP		ECX, EAX					       ; if(ecx <= eax)
	JNG		L_0040D3B8                         ;   return;
	MOV		EAX, DWORD PTR SS:[ESP + 0x20]     ; eax = S20;
L_0040CE26:									   ; while() {
	MOV		ECX, DWORD PTR SS:[ESP + 0x14]     ; ecx = nChannel
	MOV		EBP, DWORD PTR SS:[ESP + 0x28]     ; ebp = *(short*)(pSrcBuffer + 2)
	TEST	ECX, ECX						   ; if(!ecx)
	JNZ		L_0040CE36                         ; {
	MOV		EBP, DWORD PTR SS:[ESP + 0x18]     ;   ebp = *(short*)(pSrcBuffer);
L_0040CE36:                                    ; }
	TEST	EAX, EAX                           ; if(eax)
	JZ		L_0040CE44                         ; {
	DEC		EAX                                ;   eax--;
	MOV		DWORD PTR SS:[ESP + 0x20], EAX     ;   S20 = eax;
	JMP		L_0040D073                         ;   break;
L_0040CE44:                                    ; }
	CMP		EDX, 8                             ; if(edx >= 8)
	JL		L_0040CE4D                         ; {
	INC		ESI                                ;   esi++;
	AND		EDX, 7                             ;   edx &= 7;
L_0040CE4D:                                    ; }              // ここから ReadBits, EDX = nShiftBits, ESI = pSrcBuffer
	XOR		EDI, EDI                           ; edi = 0;
	MOV		CL, DL                             ; cl = dl;
	MOV		DI, WORD PTR DS:[ESI]              ; di = *esi;
	ADD		EDX, 3                             ; edx += 3;
	SHR		EDI, CL                            ; edi >>= cl;
	AND		EDI, 7							   ; edi &= 7;
	CMP		EDI, 4                             ; if(edi < 4)
	JNL		L_0040CF84                         ; {
	TEST	EDI, EDI                           ;   if(edi == 0)
	JNZ		L_0040CEE0                         ;   {
	MOV		ECX, DWORD PTR SS:[ESP + 0x38]     ;     ecx = m_Header.m_bIsNWK;
	TEST	ECX, ECX                           ;     if(!ecx)
	JZ		L_0040D073                         ;       break;
	CMP		EDX, 8                             ;     if(edx >= 8)
	JL		L_0040CE7E                         ;     {
	INC		ESI                                ;       esi++;
	AND		EDX, 7                             ;       edx &= 7;
L_0040CE7E:                                    ;     }
	XOR		EAX, EAX                           ;     eax = 0;
	MOV		CL, DL                             ;     cl = dl;
	MOV		AX, WORD PTR DS:[ESI]              ;     di = *esi;
	SHR		EAX, CL                            ;     eax >>= cl;
	AND		EAX, 1                             ;     eax &= 1;
	INC		EDX                                ;     edx++;
	CMP		EAX, 1                             ;     S20 = eax;
	MOV		DWORD PTR SS:[ESP + 0x20], EAX     ;     if(eax != 1)
	JNZ		L_0040D073                         ;       break;
	CMP		EDX, 8                             ;     if(edx >= 8)
	JL		L_0040CEA1                         ;     {
	INC		ESI                                ;       esi++;
	AND		EDX, 7                             ;       edx &= 7;
L_0040CEA1:                                    ;     }
	XOR		EAX, EAX                           ;     eax = 0;
	MOV		CL, DL                             ;     cl = dl;
	MOV		AX, WORD PTR DS:[ESI]              ;     ax = *esi;
	ADD		EDX, 2                             ;     edx += 2;       
	SHR		EAX, CL                            ;     eax >>= cl;
	AND		EAX, 3                             ;     eax &= 3;
	CMP		EAX, 3                             ;     S20 = eax;
	MOV		DWORD PTR SS:[ESP + 0x20], EAX     ;     if(eax != 3)
	JNZ		L_0040D073                         ;       break;
	CMP		EDX, 8                             ;     if(edx >= 8)
	JL		L_0040CEC6						   ;     {
	INC		ESI                                ;      esi++;
	AND		EDX, 7                             ;      edx &= 7;
L_0040CEC6:                                    ;     }
	XOR		EAX,EAX                            ;     eax = 0;
	MOV		CL, DL                             ;     cl = dl;
	MOV		AX, WORD PTR DS:[ESI]              ;     ax = *esi;
	SHR		EAX, CL                            ;     eax >>= cl;
	AND		EAX, 0xFF                          ;     eax &= 0xff;
	ADD		EDX, 8                             ;     edx += 8;
	MOV		DWORD PTR SS:[ESP + 0x20], EAX     ;     S20 = eax;
	JMP		L_0040D073                         ;     break;
L_0040CEE0:                                    ;   }
	CMP		EDI, 1                             ;   if(edi == 1)
	JNZ		L_0040CF18                         ;   {
	CMP		EDX, 8                             ;     if(edx >= 8)
	JL		L_0040CEEE                         ;     {
	INC		ESI                                ;       esi++;
	AND		EDX, 7                             ;       edx &= 7;
L_0040CEEE:                                    ;     }
	XOR		EBX, EBX                           ;     ebx = 0;
	MOV		CL, DL                             ;     cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;     bx = *esi;
	ADD		EDX, 7                             ;     edx += 7;
	SHR		EBX, CL                            ;     ebx >>= cl;
	AND		EBX, 0x7F                          ;     ebx &= 0x7f;
	TEST	BL, 0x40                           ;     if(bl & 0x40)
	JZ		L_0040CF0F                         ;     {
	AND		EBX, 0x3F                          ;       ebx &= 0x3f;
	SHL		EBX, 2                             ;       ebx <<= 2;
	SUB		EBP, EBX                           ;       ebp -= ebx;
	JMP		L_0040D073                         ;       break;
L_0040CF0F:                                    ;     }
	LEA     EBP,DWORD PTR [EBP + EBX * 4]      ;     ebp += ebx * 4;
	JMP		L_0040D073                         ;     break;
L_0040CF18:									   ;   }
	CMP		EDI, 2                             ;   if(edi == 2)
	JNZ		L_0040CF50                         ;   {
	CMP		EDX, 8                             ;     if(edx >= 8)
	JL		L_0040CF26                         ;     {
	INC		ESI                                ;       esi++;
	AND		EDX, 7                             ;       edx &= 7;
L_0040CF26:                                    ;     }
	XOR		EBX, EBX                           ;     ebx = 0;
	MOV		CL, DL                             ;     cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;     bx = *esi;
	ADD		EDX, 7                             ;     edx += 7;
	SHR		EBX, CL                            ;     ebx >>= cl;
	AND		EBX, 0x7F                          ;     ebx &= 0x7f;
	TEST	BL, 0x40                           ;     if(bl & 0x40)
	JZ		L_0040CF47                         ;     {
	AND		EBX, 0x3F                          ;       ebx &= 0x3f;
	SHL		EBX, 3                             ;       ebx <<= 3;
	SUB		EBP, EBX                           ;       ebp -= ebx;
	JMP		L_0040D073                         ;       break;
L_0040CF47:                                    ;     }
	LEA		EBP, DWORD PTR SS:[EBP + EBX * 8]  ;     ebp += ebx * 8;
	JMP		L_0040D073                         ;     break;
L_0040CF50:                                    ;   }
	CMP		EDX, 8                             ;   if(edx >= 8)
	JL		L_0040CF59                         ;   {
	INC		ESI                                ;     esi++;
	AND		EDX, 7                             ;     edx &= 7;
L_0040CF59:                                    ;   }
	XOR		EBX, EBX                           ;   ebx = 0;
	MOV		CL, DL                             ;   cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;   bx = *esi;
	ADD		EDX, 7                             ;   edx += 7;
	SHR		EBX, CL                            ;   ebx >>= cl;
	AND		EBX, 0x7F                          ;   ebx &= 0x7f;
	TEST	BL, 0x40                           ;   if(bl & 0x40)
	JZ		L_0040CF7A                         ;   {
	AND		EBX, 0x3F                          ;     ebx &= 0x3f;
	SHL		EBX, 4                             ;     ebx <<= 4;
	SUB		EBP, EBX                           ;     ebp -= ebx;
	JMP		L_0040D073                         ;     break;
L_0040CF7A:                                    ;   }
	SHL		EBX, 4                             ;   ebx <<= 4;
	ADD		EBP, EBX                           ;   ebp += ebx;
	JMP		L_0040D073                         ;   break;
L_0040CF84:									   ; }
	JNZ		L_0040CFBA                         ; else if(edi == 4) {
	CMP		EDX, 8                             ;   if(edx >= 8)
	JL		L_0040CF8F                         ;   {
	INC		ESI                                ;     esi++;
	AND		EDX, 7                             ;     edx &= 7;
L_0040CF8F:                                    ;   }
	XOR		EBX, EBX                           ;   ebx = 0;
	MOV		CL, DL                             ;   cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;   bx = *esi;
	ADD		EDX, 7                             ;   edx += 7;
	SHR		EBX, CL                            ;   ebx >>= cl;
	AND		EBX, 0x7F                          ;   ebx &= 0x7f;
	TEST	BL, 0x40                           ;   if(bl & 0x40)
	JZ		L_0040CFB0                         ;   {
	AND		EBX, 0x3F                          ;     ebx &= 0x3f;
	SHL		EBX, 5                             ;     ebx <<= 5;
	SUB		EBP, EBX                           ;     ebp -= ebx;
	JMP		L_0040D073                         ;     break;
L_0040CFB0:                                    ;   }
	SHL		EBX, 5                             ;   ebx <<= 5;
	ADD		EBP, EBX                           ;   ebp += ebx;
	JMP		L_0040D073                         ;   break;
L_0040CFBA:                                    ; }
	CMP		EDI, 5                             ; if(edi == 5)
	JNZ		L_0040CFF3                         ; {
	CMP		EDX, 8                             ;   if(edx >= 8)
	JL		L_0040CFC8                         ;   {
	INC		ESI                                ;     esi++;
	AND		EDX, 7                             ;     edx &= 7;
L_0040CFC8:                                    ;   }
	XOR		EBX, EBX                           ;   ebx = 0;
	MOV		CL, DL                             ;   cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;   bx = *esi;
	ADD		EDX, 7                             ;   edx += 7;
	SHR		EBX, CL                            ;   ebx >>= cl;
	AND		EBX, 0x7F                          ;   ebx &= 0x7f;
	TEST	BL, 0x40                           ;   if(bl & 0x40)
	JZ		L_0040CFE9                         ;   {
	AND		EBX, 0x3F                          ;     ebx &= 0x3f;
	SHL		EBX, 6                             ;     ebx <<= 6;
	SUB		EBP, EBX                           ;     ebp -= ebx;
	JMP		L_0040D073                         ;     break;
L_0040CFE9:                                    ;   }
	SHL		EBX, 6                             ;   ebx <<= 6;
	ADD		EBP, EBX                           ;   ebp += ebx;
	JMP		L_0040D073                         ;   break;
L_0040CFF3:                                    ; }
	CMP		EDI, 6                             ; if(edi == 6)
	JNZ		L_0040D026                         ; {
	CMP		EDX, 8                             ;   if(edx >= 8)
	JL		L_0040D001                         ;   {
	INC		ESI                                ;     esi++;
	AND		EDX, 7                             ;     edx &= 7;
L_0040D001:                                    ;   }
	XOR		EBX, EBX                           ;   ebx = 0;
	MOV		CL, DL                             ;   cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;   bx = *esi;
	ADD		EDX, 7                             ;   edx += 7;
	SHR		EBX, CL                            ;   ebx >>= cl;
	AND		EBX, 0x7F                          ;   ebx &= 0x7f;
	TEST	BL, 0x40                           ;   if(bl & 0x40)
	JZ		L_0040D01F                         ;   {
	AND		EBX, 0x3F                          ;     ebx &= 0x3f;
	SHL		EBX, 7							   ;     ebx <<= 7;
	SUB		EBP, EBX                           ;     ebp -= ebx;
	JMP     L_0040D073                         ;     break;
L_0040D01F:                                    ;   }
	SHL		EBX, 7							   ;   ebx <<= 7;
	ADD		EBP, EBX                           ;   ebp += ebx;
	JMP		L_0040D073                         ;   break;
L_0040D026:                                    ; }
	CMP		EDX, 8                             ; if(edx >= 8)
	JL		L_0040D02F                         ; {
	INC		ESI                                ;   esi++;
	AND		EDX, 7							   ;   edx &= 7;
L_0040D02F:                                    ; }
	XOR		EDI, EDI                           ; edi = 0;
	MOV		CL, DL                             ; cl = dl;
	MOV		DI, WORD PTR DS:[ESI]              ; di = *esi;
	SHR		EDI, CL                            ; edi >>= cl;
	AND		EDI, 1                             ; edi &= 1;
	INC		EDX                                ; edx++;
	TEST	EDI, EDI                           ; if(!edi)
	JNZ		L_0040D071                         ; {
	CMP		EDX, 8                             ;   if(edx >= 8)
	JL		L_0040D049                         ;   {
	INC		ESI                                ;     esi++;
	AND		EDX, 7							   ;     edx &= 7;
L_0040D049:									   ;   }
	XOR		EBX, EBX						   ;   ebx = 0;
	MOV		CL, DL                             ;   cl = dl;
	MOV		BX, WORD PTR DS:[ESI]              ;   bx = *esi;
	ADD		EDX, 8                             ;   edx += 8;
	SHR		EBX, CL                            ;   ebx >>= cl;
	AND		EBX, 0xFF                          ;   ebx &= 0xff;
	TEST	BL, 0x80                           ;   if(bl & 0x80)
	JZ		L_0040D06A                         ;   {
	AND		EBX, 0x7F                          ;     ebx &= 0x7f;
	SHL		EBX, 9							   ;     ebx <<= 9;
	SUB		EBP, EBX                           ;     ebp -= ebx;
	JMP		L_0040D073                         ;     break;
L_0040D06A:                                    ;    }
	SHL		EBX, 9                             ;    ebx <<= 9;
	ADD		EBP, EBX                           ;    ebp += ebx;
	JMP		L_0040D073                         ;    break;
L_0040D071:                                    ; }
	XOR		EBP, EBP                           ; ebp = 0;
L_0040D073:									   ;
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
	TEST	is_mono, 1                               ; if(!isNWK)
	JZ		L2                                       ; {
	XOR		EBP, EBP                                 ;   ebp = 0;
L2:                                                  ; }
	;
	; ここまで追加。
	;
	CMP		EDI, ECX                                 ; S14 = EBP;
	MOV		DWORD PTR SS:[ESP + 0x14], EBP           ; S2C = EDI;
	MOV		DWORD PTR SS:[ESP + 0x2C], EDI           ; if(edi >= ecx)
	JL		L_0040CE26                               ;   goto
L_0040D3B8:                                          ; }
	; --------------------------------------------
	; ここまで RealLive.exe から抜き出したコード。
	; --------------------------------------------

	; 待避しておいたスタックに切り替える。
	MOV		ESP, orig_stack_addr
	
	; 後処理。
	POP		EBP
}
