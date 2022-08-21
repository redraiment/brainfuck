#ifndef __COMPILER_H_
#define __COMPILER_H_

#define DATA_SEGMENT_SIZE 30000

void SetUpCompiler(void);
void Compile(char*);

void WhileNotZero(void);
void WhileEnd(void);
void MovePointer(int);
void UpdateValue(int);
void InputValue(void);
void OutputValue(void);

#endif
