#ifndef MATRICES_H
#define MATRICES_H

void matrixSetIdentityM(float *m);

void matrixSetRotateM(float *m, float a, float x, float y, float z);

void matrixMultiplyMM(float *m, float *lhs, float *rhs);

void matrixScaleM(float *m, float x, float y, float z);

void matrixTranslateM(float *m, float x, float y, float z);
void matrixTranslateXM(float *m, float x);
void matrixTranslateYM(float *m, float y);
void matrixTranslateZM(float *m, float z);

void matrixSetTranslateM(float *m, float x, float y, float z);

void matrixRotateM(float *m, float a, float x, float y, float z);

void matrixLookAtM(float *m,
                float eyeX, float eyeY, float eyeZ,
                float cenX, float cenY, float cenZ,
                float  upX, float  upY, float  upZ);

void matrixLookAtM2(float *m,
                float eyeX, float eyeY, float eyeZ,
                float cenX, float cenY, float cenZ,
                float  upX, float  upY, float  upZ,
				float* out_side, float* out_up);

void matrixFrustumM(float *m, float left, float right, float bottom, float top, float near, float far);

void matrixFrustumM2(float *m, float left, float right, float bottom, float top, float near, float far);

void BuildPerspProjMat(float *m, float fov, float aspect, float znear, float zfar);

void MatVecMult( const float* vec3, const float* matr, float* res );

void InvMat( float* mat, float* res );

bool gluInvertMatrix(const float m[16], float invOut[16]);

#endif
