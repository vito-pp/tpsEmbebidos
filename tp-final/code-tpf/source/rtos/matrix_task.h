#ifndef MATRIX_TASK_H_
#define MATRIX_TASK_H_

typedef enum
{
    MATRIX_INDICATOR_NONE,
    MATRIX_INDICATOR_VALID,
    MATRIX_INDICATOR_INVALID,
    MATRIX_INDICATOR_TIMEOUT
} MatrixIndicator_t;

void MatrixTask_Create(void);
void MatrixTask_RequestRefresh(void);
void MatrixTask_RequestIndicator(MatrixIndicator_t indicator);
void MatrixTask_FrameDoneFromISR(void);

#endif /* MATRIX_TASK_H_ */
