#pragma once

#include <windows.h>
#include <vector>

#define JMP_TO_NEXT_HANDLER \
    0x41, 0x8B, 0x45, 0x00,   /* mov eax, dword ptr [r13] */ \
    0x49, 0x83, 0xC5, 0x04,   /* add r13, 0x04 */ \
    0x4C, 0x01, 0xF0,         /* add rax, r14 */ \
    0xFF, 0xE0,               /* jmp rax */

/*

afaik, stack collision check is only required for
handlers that push values onto stack, or modify the vsp. They
shouldn't be required on other handlers since they operate
within a safe range (I think).

*/

#define CONTEXT_COLLISION_CHECK \
	0x90,


enum VMHandlerTypes
{
	ENTER, EXIT,

	PUSHI64, PUSHI32, PUSHI16, PUSHI8,

	PUSHR64, PUSHR32, PUSHR16, PUSHR8,

	PUSHRSP64, PUSHRSP32, PUSHRSP16, PUSHRSP8,

	POPRSP64, POPRSP32, POPRSP16, POPRSP8,

	POPR64, POPR32, POPR16, POPR8,

	READ64, READ32, READ16, READ8,

	WRITE64, WRITE32, WRITE16, WRITE8,

	ADD64, ADD32, ADD16, ADD8,

	SUB64, SUB32, SUB16, SUB8,

	XOR64, XOR32, XOR16, XOR8,

	AND64, AND32, AND16, AND8,

	OR64, OR32, OR16, OR8,

	NAND64, NAND32, NAND16, NAND8,

	NOR64, NOR32, NOR16, NOR8,

	SHL64, SHL32, SHL16, SHL8,

	JMP,

	JNE,
};

class VMHandler
{
public:
	VMHandlerTypes getType();
	DWORD getRva();
	DWORD getFileOffset();
	std::vector<BYTE> getBytes();

	void setRva(DWORD rva);
	void setFileOffset(DWORD fileOffset);
protected:
	VMHandlerTypes type;
	DWORD rva;
	DWORD fileOffset;
	std::vector<BYTE> bytes;
};

class Enter : public VMHandler
{
public:
	Enter()
	{
		type = ENTER;
		bytes =
		{
			0x41, 0x57,                                          // push r15
			0x41, 0x56,                                          // push r14
			0x41, 0x55,                                          // push r13
			0x41, 0x54,                                          // push r12
			0x41, 0x53,                                          // push r11
			0x41, 0x52,                                          // push r10
			0x41, 0x51,                                          // push r9
			0x41, 0x50,                                          // push r8
			0x55,                                                // push rbp
			0x57,                                                // push rdi
			0x56,                                                // push rsi
			0x52,                                                // push rdx
			0x51,                                                // push rcx
			0x53,                                                // push rbx
			0x50,                                                // push rax
			0x9C,                                                // pushfq
			0x49, 0x89, 0xE7,                                    // mov r15, rsp
			0x48, 0x81, 0xEC, 0x0, 0x1, 0x0, 0x0,                // sub rsp, 0x100
			0x65, 0x4C, 0x8B, 0x34, 0x25, 0x60, 0x0, 0x0, 0x0,   // mov r14, qword ptr gs:[0x60]
			0x4D, 0x8B, 0x76, 0x10,                              // mov r14, qword ptr [r13+0x10]
			0x4C, 0x89, 0xB4, 0x24, 0x88, 0x00, 0x00, 0x00,      // mov qword ptr [rsp+0x88], r14
			0x45, 0x8B, 0xAF, 0x80, 0x0, 0x0, 0x0,               // mov r13d, dword ptr [r15+0x80]
			0x4D, 0x01, 0xF5,                                    // add r13, r14
			JMP_TO_NEXT_HANDLER
		};
	};
private:

};
class Exit : public VMHandler
{
public:
	Exit()
	{
		type = EXIT;
		bytes =
		{
			0x4C, 0x89, 0xFC,   // mov rsp, r15
			0x9D,               // popfq
			0x58,               // pop rax
			0x5B,               // pop rbx
			0x59,               // pop rcx
			0x5A,               // pop rdx
			0x5E,               // pop rsi
			0x5F,               // pop rdi
			0x5D,               // pop rbp
			0x41, 0x58,         // pop r8
			0x41, 0x59,         // pop r9
			0x41, 0x5A,         // pop r10
			0x41, 0x5B,         // pop r11
			0x41, 0x5C,         // pop r12
			0x41, 0x5D,         // pop r13
			0x41, 0x5E,         // pop r14
			0x41, 0x5F,         // pop r15
			0xC3,               // ret
		};
	};
private:

};

class PushR64 : public VMHandler
{
public:
	PushR64()
	{
		type = PUSHR64;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x48, 0x8B, 0x04, 0xC4,               // mov rax, qword ptr [rsp+rax*0x08]
			0x49, 0x83, 0xEF, 0x08,               // sub r15, 0x08
			0x49, 0x89, 0x07,                     // mov qword ptr [r15], rax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushR32 : public VMHandler
{
public:
	PushR32()
	{
		type = PUSHR32;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x8B, 0x04, 0xC4,                    // mov eax, dword ptr [rsp+rax*0x08]
			0x49, 0x83, 0xEF, 0x04,               // sub r15, 0x04
			0x41, 0x89, 0x07,                     // mov dword ptr [r15], eax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushR16 : public VMHandler
{
public:
	PushR16()
	{
		type = PUSHR16;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x66, 0x8B, 0x04, 0xC4,               // mov ax, word ptr [rsp+rax*0x08]
			0x49, 0x83, 0xEF, 0x02,               // sub r15, 0x02
			0x66, 0x41, 0x89, 0x07,               // mov word ptr [r15], ax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushR8 : public VMHandler
{
public:
	PushR8()
	{
		type = PUSHR8;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x8A, 0x04, 0xC4,                     // mov al, byte ptr [rsp+rax*0x08]
			0x49, 0x83, 0xEF, 0x01,               // sub r15, 0x01
			0x41, 0x88, 0x07,                     // mov byte ptr [r15], al
			JMP_TO_NEXT_HANDLER
		};
	}
};

class PushRSP64 : public VMHandler
{
public:
	PushRSP64()
	{
		type = PUSHRSP64;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x4C, 0x89, 0xF8,         // mov rax, r15
			0x49, 0x83, 0xEF, 0x08,   // sub r15, 0x08
			0x49, 0x89, 0x07,         // mov qword ptr [r15], rax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushRSP32 : public VMHandler
{
public:
	PushRSP32()
	{
		type = PUSHRSP32;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x44, 0x89, 0xF8,         // mov eax, r15d
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x89, 0x07,         // mov dword ptr [r15], eax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushRSP16 : public VMHandler
{
public:
	PushRSP16()
	{
		type = PUSHRSP16;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x66, 0x44, 0x89, 0xF8,   // mov ax, r15w
			0x49, 0x83, 0xEF, 0x02,   // sub r15, 0x02
			0x66, 0x41, 0x89, 0x07,   // mov word ptr [r15], ax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushRSP8 : public VMHandler
{
public:
	PushRSP8()
	{
		type = PUSHRSP8;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x44, 0x88, 0xF8,         // mov al, r15b
			0x49, 0x83, 0xEF, 0x01,   // sub r15, 0x01
			0x41, 0x88, 0x07,         // mov byte ptr [r15], al
			JMP_TO_NEXT_HANDLER
		};
	}
};

class PopRSP64 : public VMHandler
{
public:
	PopRSP64()
	{
		type = POPRSP64;
		bytes =
		{
			0x4D, 0x8B, 0x3F,   // mov r15, qword ptr [r15]
			CONTEXT_COLLISION_CHECK
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PopRSP32 : public VMHandler
{
public:
	PopRSP32()
	{
		type = POPRSP32;
		bytes =
		{
			0x45, 0x8B, 0x3F,   // mov r15d, dword ptr [r15]
			CONTEXT_COLLISION_CHECK
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PopRSP16 : public VMHandler
{
public:
	PopRSP16()
	{
		type = POPRSP16;
		bytes =
		{
			0x66, 0x45, 0x8B, 0x3F,   // mov r15w, word ptr [r15]
			CONTEXT_COLLISION_CHECK
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PopRSP8 : public VMHandler
{
public:
	PopRSP8()
	{
		type = POPRSP8;
		bytes =
		{
			0x45, 0x8A, 0x3F,   // mov r15b, byte ptr [r15]
			CONTEXT_COLLISION_CHECK
			JMP_TO_NEXT_HANDLER
		};
	}
};

class PopR64 : public VMHandler
{
public:
	PopR64()
	{
		type = POPR64;
		bytes =
		{
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x49, 0x8B, 0x1F,                     // mov rbx, qword ptr [r15]
			0x49, 0x83, 0xC7, 0x08,               // add r15, 0x08
			0x48, 0x89, 0x1C, 0xC4,               // mov qword ptr [rsp+rax*8], rbx
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PopR32 : public VMHandler
{
public:
	PopR32()
	{
		type = POPR32;
		bytes =
		{
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x41, 0x8B, 0x1F,                     // mov ebx, dword ptr [r15]
			0x49, 0x83, 0xC7, 0x04,               // add r15, 0x04
			0x89, 0x1C, 0xC4,                     // mov dword ptr [rsp+rax*8], ebx
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PopR16 : public VMHandler
{
public:
	PopR16()
	{
		type = POPR16;
		bytes =
		{
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x66, 0x41, 0x8B, 0x1F,               // mov bx, word ptr [r15]
			0x49, 0x83, 0xC7, 0x02,               // add r15, 0x02
			0x66, 0x89, 0x1C, 0xC4,               // mov word ptr [rsp+rax*8], bx
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PopR8 : public VMHandler
{
public:
	PopR8()
	{
		type = POPR8;
		bytes =
		{
			0x49, 0x8B, 0x45, 0x00,               // mov rax, qword ptr [r13]
			0x48, 0x25, 0xFF, 0x00, 0x00, 0x00,   // and rax, 0xFF
			0x49, 0xFF, 0xC5,                     // inc r13
			0x41, 0x8A, 0x1F,                     // mov bl, byte ptr [r15]
			0x49, 0x83, 0xC7, 0x01,               // add r15, 0x01
			0x88, 0x1C, 0xC4,                     // mov byte ptr [rsp+rax*8], bl
			JMP_TO_NEXT_HANDLER
		};
	}
};

class PushI64 : public VMHandler
{
public:
	PushI64()
	{
		type = PUSHI64;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x49, 0x8B, 0x45, 0x00,   // mov rax, qword ptr [r13]
			0x49, 0x83, 0xC5, 0x08,   // add r13, 0x08
			0x49, 0x83, 0xEF, 0x08,   // sub r15, 0x08
			0x49, 0x89, 0x07,         // mov qword ptr [r15], rax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushI32 : public VMHandler
{
public:
	PushI32()
	{
		type = PUSHI32;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x41, 0x8B, 0x45, 0x00,   // mov eax, dword ptr [r13]
			0x49, 0x83, 0xC5, 0x04,   // add r13, 0x04
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x89, 0x07,         // mov dword ptr [r15], eax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushI16 : public VMHandler
{
public:
	PushI16()
	{
		type = PUSHI16;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x66, 0x41, 0x8B, 0x45, 0x00,   // mov ax, word ptr [r13]
			0x49, 0x83, 0xC5, 0x02,         // add r13, 0x02
			0x49, 0x83, 0xEF, 0x02,         // sub r15, 0x02
			0x66, 0x41, 0x89, 0x07,         // mov word ptr [r15], ax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class PushI8 : public VMHandler
{
public:
	PushI8()
	{
		type = PUSHI8;
		bytes =
		{
			CONTEXT_COLLISION_CHECK
			0x41, 0x8A, 0x45, 0x00,   // mov al, byte ptr [r13]
			0x49, 0x83, 0xC5, 0x01,   // add r13, 0x01
			0x49, 0x83, 0xEF, 0x01,   // sub r15, 0x01
			0x41, 0x88, 0x07,         // mov byte ptr [r15], al
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Read64 : public VMHandler
{
public:
	Read64()
	{
		type = READ64;
		bytes =
		{
			0x49, 0x8B, 0x07,   // mov rax, qword ptr [r15]
			0x48, 0x8B, 0x00,   // mov rax, qword ptr [rax]
			0x49, 0x89, 0x07,   // mov qword ptr [r15], rax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Read32 : public VMHandler
{
public:
	Read32()
	{
		type = READ32;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x8B, 0x00,               // mov eax, dword ptr [rax]
			0x49, 0x83, 0xC7, 0x04,   // add r15, 0x04
			0x41, 0x89, 0x07,         // mov dword ptr [r15], eax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Read16 : public VMHandler
{
public:
	Read16()
	{
		type = READ16;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x66, 0x8B, 0x00,         // mov ax, word ptr [rax]
			0x49, 0x83, 0xC7, 0x06,   // add r15, 0x06
			0x66, 0x41, 0x89, 0x07,   // mov word ptr [r15], ax
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Read8 : public VMHandler
{
public:
	Read8()
	{
		type = READ8;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x8A, 0x00,               // mov al, byte ptr [rax]
			0x49, 0x83, 0xC7, 0x07,   // add r15, 0x07
			0x41, 0x88, 0x07,         // mov byte ptr [r15], al
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Write64 : public VMHandler
{
public:
	Write64()
	{
		type = WRITE64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x8B, 0x5F, 0x08,   // mov rbx, qword ptr [r15+0x08]
			0x49, 0x83, 0xC7, 0x10,   // add r15, 0x10
			0x48, 0x89, 0x18,         // mov qword ptr [rax], rbx
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Write32 : public VMHandler
{
public:
	Write32()
	{
		type = WRITE32;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x41, 0x8B, 0x5F, 0x08,   // mov ebx, dword ptr [r15+0x08]
			0x49, 0x83, 0xC7, 0x0C,   // add r15, 0x0C
			0x89, 0x18,               // mov dword ptr [rax], ebx
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Write16 : public VMHandler
{
public:
	Write16()
	{
		type = WRITE16;
		bytes =
		{
			0x49, 0x8B, 0x07,               // mov rax, qword ptr [r15]
			0x66, 0x41, 0x8B, 0x5F, 0x08,   // mov bx, word ptr [r15+0x08]
			0x49, 0x83, 0xC7, 0x0A,         // add r15, 0x0A
			0x66, 0x89, 0x18,               // mov word ptr [rax], bx
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Write8 : public VMHandler
{
public:
	Write8()
	{
		type = WRITE8;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x41, 0x8A, 0x5F, 0x08,   // mov bl, byte ptr [r15+0x08]
			0x49, 0x83, 0xC7, 0x09,   // add r15, 0x9
			0x88, 0x18,               // mov byte ptr [rax], bl
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Add64 : public VMHandler
{
public:
	Add64()
	{
		type = ADD64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x01, 0x47, 0x08,   // add qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Add32 : public VMHandler
{
public:
	Add32()
	{
		type = ADD32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x01, 0x47, 0x08,   // add dword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Add16 : public VMHandler
{
public:
	Add16()
	{
		type = ADD16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x41, 0x01, 0x47, 0x08,   // add word ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Add8 : public VMHandler
{
public:
	Add8()
	{
		type = ADD8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, byte ptr [r15]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x41, 0x00, 0x47, 0x08,   // add byte ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Sub64 : public VMHandler
{
public:
	Sub64()
	{
		type = SUB64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x29, 0x47, 0x08,   // sub qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Sub32 : public VMHandler
{
public:
	Sub32()
	{
		type = SUB32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x29, 0x47, 0x08,   // sub dword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Sub16 : public VMHandler
{
public:
	Sub16()
	{
		type = SUB16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x41, 0x29, 0x47, 0x08,   // sub word ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Sub8 : public VMHandler
{
public:
	Sub8()
	{
		type = SUB8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, byte ptr [r15]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x41, 0x28, 0x47, 0x08,   // sub byte ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Xor64 : public VMHandler
{
public:
	Xor64()
	{
		type = XOR64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x31, 0x47, 0x08,   // xor qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Xor32 : public VMHandler
{
public:
	Xor32()
	{
		type = XOR32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x31, 0x47, 0x08,   // xor dword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Xor16 : public VMHandler
{
public:
	Xor16()
	{
		type = XOR16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x41, 0x31, 0x47, 0x08,   // xor word ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Xor8 : public VMHandler
{
public:
	Xor8()
	{
		type = XOR8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, byte ptr [r15]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x41, 0x30, 0x47, 0x08,   // xor byte ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class And64 : public VMHandler
{
public:
	And64()
	{
		type = AND64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x21, 0x47, 0x08,   // and qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class And32 : public VMHandler
{
public:
	And32()
	{
		type = AND32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x21, 0x47, 0x08,   // and dword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class And16 : public VMHandler
{
public:
	And16()
	{
		type = AND16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x41, 0x21, 0x47, 0x08,   // and word ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class And8 : public VMHandler
{
public:
	And8()
	{
		type = AND8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, byte ptr [r15]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x41, 0x20, 0x47, 0x08,   // and byte ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Or64 : public VMHandler
{
public:
	Or64()
	{
		type = OR64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x09, 0x47, 0x08,   // or qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Or32 : public VMHandler
{
public:
	Or32()
	{
		type = OR32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x41, 0x09, 0x47, 0x08,   // or dword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Or16 : public VMHandler
{
public:
	Or16()
	{
		type = OR16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x41, 0x09, 0x47, 0x08,   // or word ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Or8 : public VMHandler
{
public:
	Or8()
	{
		type = OR8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, byte ptr [r15]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x41, 0x08, 0x47, 0x08,   // or byte ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Nand64 : public VMHandler
{
public:
	Nand64()
	{
		type = NAND64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x8B, 0x5F, 0x08,   // mov rbx, qword ptr [r15+0x08]
			0x48, 0x21, 0xD8,         // not rax, rbx
			0x48, 0xF7, 0xD0,         // not rax
			0x49, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Nand32 : public VMHandler
{
public:
	Nand32()
	{
		type = NAND32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x41, 0x8B, 0x5F, 0x04,   // mov ebx, dword ptr [r15+0x04]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x21, 0xD8,               // and eax, ebx
			0xF7, 0xD0,               // not eax
			0x41, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Nand16 : public VMHandler
{
public:
	Nand16()
	{
		type = NAND16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x66, 0x41, 0x8B, 0x5F, 0x02,   // mov bx, word ptr [r15+0x02]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x21, 0xD8,               // and ax, bx
			0x66, 0xF7, 0xD0,               // not ax
			0x66, 0x41, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Nand8 : public VMHandler
{
public:
	Nand8()
	{
		type = NAND8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, word ptr [r15]
			0x41, 0x8A, 0x5F, 0x01,   // mov bl, word ptr [r15+0x01]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x20, 0xD8,               // and al, bl
			0xF6, 0xD0,               // not al
			0x41, 0x88, 0x47, 0x08,   // mov qword ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Nor64 : public VMHandler
{
public:
	Nor64()
	{
		type = NOR64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x49, 0x8B, 0x5F, 0x08,   // mov rbx, qword ptr [r15+0x08]
			0x48, 0x09, 0xD8,         // or rax, rbx
			0x48, 0xF7, 0xD0,         // not rax
			0x49, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Nor32 : public VMHandler
{
public:
	Nor32()
	{
		type = NOR32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x41, 0x8B, 0x5F, 0x04,   // mov ebx, dword ptr [r15+0x04]
			0x49, 0x83, 0xEF, 0x04,   // sub r15, 0x04
			0x09, 0xD8,               // or eax, ebx
			0xF7, 0xD0,               // not eax
			0x41, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Nor16 : public VMHandler
{
public:
	Nor16()
	{
		type = NOR16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x66, 0x41, 0x8B, 0x5F, 0x02,   // mov bx, word ptr [r15+0x02]
			0x49, 0x83, 0xEF, 0x06,         // sub r15, 0x06
			0x66, 0x09, 0xD8,               // or ax, bx
			0x66, 0xF7, 0xD0,               // not ax
			0x66, 0x41, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Nor8 : public VMHandler
{
public:
	Nor8()
	{
		type = NOR8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, word ptr [r15]
			0x41, 0x8A, 0x5F, 0x01,   // mov bl, word ptr [r15+0x01]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x08, 0xD8,               // or al, bl
			0xF6, 0xD0,               // not al
			0x41, 0x88, 0x47, 0x08,   // mov qword ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Shl64 : public VMHandler
{
public:
	Shl64()
	{
		type = SHL64;
		bytes =
		{
			0x49, 0x8B, 0x07,         // mov rax, qword ptr [r15]
			0x41, 0x8A, 0x4F, 0x08,   // mov cl, byte ptr [r15+0x08]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0x48, 0xD3, 0xE0,         // shl rax, cl
			0x49, 0x89, 0x47, 0x08,   // mov qword ptr [r15+0x08], rax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Shl32 : public VMHandler
{
public:
	Shl32()
	{
		type = SHL32;
		bytes =
		{
			0x41, 0x8B, 0x07,         // mov eax, dword ptr [r15]
			0x41, 0x8A, 0x4F, 0x04,   // mov cl, byte ptr [r15+0x04]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0xD3, 0xE0,               // shl eax, cl
			0x41, 0x89, 0x47, 0x08,   // mov dword ptr [r15+0x08], eax
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Shl16 : public VMHandler
{
public:
	Shl16()
	{
		type = SHL16;
		bytes =
		{
			0x66, 0x41, 0x8B, 0x07,         // mov ax, word ptr [r15]
			0x41, 0x8A, 0x4F, 0x02,         // mov cl, byte ptr [r15+0x02]
			0x49, 0x83, 0xEF, 0x07,         // sub r15, 0x07
			0x66, 0xD3, 0xE0,               // shl ax, cl
			0x66, 0x41, 0x89, 0x47, 0x08,   // mov word ptr [r15+0x08], ax
			0x9C,                           // pushfq
			0x41, 0x8F, 0x07,               // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};
class Shl8 : public VMHandler
{
public:
	Shl8()
	{
		type = SHL8;
		bytes =
		{
			0x41, 0x8A, 0x07,         // mov al, byte ptr [r15]
			0x41, 0x8A, 0x4F, 0x01,   // mov cl, byte ptr [r15+0x01]
			0x49, 0x83, 0xEF, 0x07,   // sub r15, 0x07
			0xD2, 0xE0,               // shl al, cl
			0x41, 0x88, 0x47, 0x08,   // mov byte ptr [r15+0x08], al
			0x9C,                     // pushfq
			0x41, 0x8F, 0x07,         // pop qword ptr [r15]
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Jmp : public VMHandler
{
public:
	Jmp()
	{
		type = JMP;
		bytes =
		{
			0x45, 0x8B, 0x6D, 0x00,   // mov r13d, dword ptr [r13]
			0x4D, 0x01, 0xF5,         // add r13, r14
			JMP_TO_NEXT_HANDLER
		};
	}
};

class Jne : public VMHandler
{
public:
	Jne()
	{
		type = JNE;
		bytes =
		{
			0x41, 0x8B, 0x45, 0x00,   // mov eax, dword ptr [r13]
			0x49, 0x83, 0xC5, 0x04,   // add r13, 0x4
			0x4C, 0x01, 0xF0,         // add rax, r14
			0x41, 0xFF, 0x37,         // push qword ptr [r15]
			0x9D,                     // popfq
			0x4D, 0x8D, 0x7F, 0x08,   // lea r15, qword ptr [r15+0x08]
			0x4C, 0x0F, 0x45, 0xE8,   // cmovne r13, rax
			JMP_TO_NEXT_HANDLER
		};
	}
};