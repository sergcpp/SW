#include "Matrices_old.h"

#include <cstdio>
#include <cmath>
#include <cstring>

#define PI 3.1415926f
#define PI_OVER_360 0.0087266

#define I(_i, _j) ((_j)+4*(_i))

#define my_normalize(x, y, z)                  \
{                                               \
        float norm = 1.0f / sqrtf(x*x+y*y+z*z);  \
        x *= norm; y *= norm; z *= norm;        \
}

void matrixSetIdentityM(float *m) {
    memset((void *) m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void matrixSetRotateM(float *m, float a, float x, float y, float z) {
    float s, c;

    memset((void *) m, 0, 15 * sizeof(float));
    m[15] = 1.0f;

    a *= PI / 180.0f;
    s = (float) sin(a);
    c = (float) cos(a);

    if (1.0f == x && 0.0f == y && 0.0f == z) {
        m[5] = c;
        m[10] = c;
        m[6] = s;
        m[9] = -s;
        m[0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        m[0] = c;
        m[10] = c;
        m[8] = s;
        m[2] = -s;
        m[5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        m[0] = c;
        m[5] = c;
        m[1] = s;
        m[4] = -s;
        m[10] = 1;
    } else {
        my_normalize(x, y, z);
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        m[0] = x * x * nc + c;
        m[4] = xy * nc - zs;
        m[8] = zx * nc + ys;
        m[1] = xy * nc + zs;
        m[5] = y * y * nc + c;
        m[9] = yz * nc - xs;
        m[2] = zx * nc - ys;
        m[6] = yz * nc + xs;
        m[10] = z * z * nc + c;
    }
}

void matrixMultiplyMM(float *m, float *lhs, float *rhs) {
    float t[16];
    for (int i = 0; i < 4; i++) {
        register const float rhs_i0 = rhs[I(i, 0)];
        register float ri0 = lhs[I(0, 0)] * rhs_i0;
        register float ri1 = lhs[I(0, 1)] * rhs_i0;
        register float ri2 = lhs[I(0, 2)] * rhs_i0;
        register float ri3 = lhs[I(0, 3)] * rhs_i0;
        for (int j = 1; j < 4; j++) {
            register const float rhs_ij = rhs[I(i, j)];
            ri0 += lhs[I(j, 0)] * rhs_ij;
            ri1 += lhs[I(j, 1)] * rhs_ij;
            ri2 += lhs[I(j, 2)] * rhs_ij;
            ri3 += lhs[I(j, 3)] * rhs_ij;
        }
        t[I(i, 0)] = ri0;
        t[I(i, 1)] = ri1;
        t[I(i, 2)] = ri2;
        t[I(i, 3)] = ri3;
    }
    memcpy(m, t, sizeof(t));
}

void matrixScaleM(float *m, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
        m[i] *= x;
        m[4 + i] *= y;
        m[8 + i] *= z;
    }
}

void matrixTranslateM(float *m, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
        m[12 + i] += m[i] * x + m[4 + i] * y + m[8 + i] * z;
    }
}

void matrixTranslateXM(float *m, float x) {
    for (int i = 0; i < 4; i++) {
        m[12 + i] += m[i] * x;
    }
}

void matrixTranslateYM(float *m, float y) {
    for (int i = 0; i < 4; i++) {
        m[12 + i] += m[4 + i] * y;
    }
}

void matrixTranslateZM(float *m, float z) {
    for (int i = 0; i < 4; i++) {
        m[12 + i] += m[8 + i] * z;
    }
}

void matrixSetTranslateM(float *m, float x, float y, float z) {
    for (int i = 0; i < 4; i++) {
        m[12 + i] = m[i] * x + m[4 + i] * y + m[8 + i] * z;
    }
}

void matrixRotateM(float *m, float a, float x, float y, float z) {
    float rot[16], res[16];
    matrixSetRotateM(rot, a, x, y, z);
    matrixMultiplyMM(res, m, rot);
    memcpy(m, res, 16 * sizeof(float));
}

void matrixLookAtM(float *m,
                   float eyeX, float eyeY, float eyeZ,
                   float cenX, float cenY, float cenZ,
                   float upX, float upY, float upZ) {
    float fx = cenX - eyeX;
    float fy = cenY - eyeY;
    float fz = cenZ - eyeZ;
    my_normalize(fx, fy, fz);
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;
    my_normalize(sx, sy, sz);
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    m[0] = sx;
    m[1] = ux;
    m[2] = -fx;
    m[3] = 0.0f;
    m[4] = sy;
    m[5] = uy;
    m[6] = -fy;
    m[7] = 0.0f;
    m[8] = sz;
    m[9] = uz;
    m[10] = -fz;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
    matrixTranslateM(m, -eyeX, -eyeY, -eyeZ);
}


void matrixLookAtM2(float *m,
                    float eyeX, float eyeY, float eyeZ,
                    float cenX, float cenY, float cenZ,
                    float upX, float upY, float upZ,
                    float *out_side, float *out_up) {
    float fx = cenX - eyeX;
    float fy = cenY - eyeY;
    float fz = cenZ - eyeZ;
    my_normalize(fx, fy, fz);
    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;

    out_side[0] = sx;
    out_side[1] = sy;
    out_side[2] = sz;

    my_normalize(sx, sy, sz);
    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    out_up[0] = ux;
    out_up[1] = uy;
    out_up[2] = uz;

    m[0] = sx;
    m[1] = ux;
    m[2] = -fx;
    m[3] = 0.0f;
    m[4] = sy;
    m[5] = uy;
    m[6] = -fy;
    m[7] = 0.0f;
    m[8] = sz;
    m[9] = uz;
    m[10] = -fz;
    m[11] = 0.0f;
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;
    matrixTranslateM(m, -eyeX, -eyeY, -eyeZ);
}

void matrixFrustumM(float *m, float left, float right, float bottom, float top, float nnear, float ffar) {
    float r_width = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth = 1.0f / (nnear - ffar);
    float x = 2.0f * (nnear * r_width);
    float y = 2.0f * (nnear * r_height);
    float A = ((right + left) * r_width); // ��� ���� 2.0f*...
    float B = (top + bottom) * r_height;
    float C = (ffar + nnear) * r_depth;
    float D = 2.0f * (ffar * nnear * r_depth);

    memset((void *) m, 0, 16 * sizeof(float));

    m[0] = x;
    m[5] = y;
    m[8] = A;
    m[9] = B;
    m[10] = C;
    m[14] = D;
    m[11] = -1.0f;
}

void matrixFrustumM2(float *m, float left, float right, float bottom, float top, float nnear, float ffar) {
    float r_width = 1.0f / (right - left);
    float r_height = 1.0f / (top - bottom);
    float r_depth = 1.0f / (nnear - ffar);

    memset((void *) m, 0, 16 * sizeof(float));

    m[0] = 2.0f * r_width;
    m[5] = 2.0f * r_height;
    m[10] = -1.0f * r_depth;
    m[12] = -(right + left) * r_width;
    m[13] = -(top + bottom) * r_height;
    m[14] = -nnear * r_depth;
    m[15] = 1.0f;
}

void BuildPerspProjMat(float *m, float fov, float aspect,
                       float znear, float zfar) {
    float xymax = znear * tanf(fov * (float) PI_OVER_360);
    float ymin = -xymax;
    float xmin = -xymax;

    float width = xymax - xmin;
    float height = xymax - ymin;

    float depth = zfar - znear;
    float q = -(zfar + znear) / depth;
    float qn = -2 * (zfar * znear) / depth;

    float w = 2 * znear / width;
    w = w / aspect;
    float h = 2 * znear / height;

    m[0] = w;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;

    m[4] = 0;
    m[5] = h;
    m[6] = 0;
    m[7] = 0;

    m[8] = 0;
    m[9] = 0;
    m[10] = q;
    m[11] = -1;

    m[12] = 0;
    m[13] = 0;
    m[14] = qn;
    m[15] = 0;
}

void MatVecMult(const float *vec3, const float *mat, float *res) {

    res[0] = vec3[0] * mat[0] + vec3[1] * mat[4] + vec3[2] * mat[8] + mat[12];
    res[1] = vec3[0] * mat[1] + vec3[1] * mat[5] + vec3[2] * mat[9] + mat[13];
    res[2] = vec3[0] * mat[2] + vec3[1] * mat[6] + vec3[2] * mat[10] + mat[14];
    float r_w = 1.0f / (vec3[0] * mat[3] + vec3[1] * mat[7] + vec3[2] * mat[11] + mat[15]);
    res[0] *= r_w;
    res[1] *= r_w;
    res[2] *= r_w;

}

void InvMat(float *mat, float *res) {

    /*res[0] = mat[0];
    res[1] = mat[4];
    res[2] = mat[8];

    res[3] = mat[1];
    res[4] = mat[5];
    res[5] = mat[9];

    res[6] = mat[2];
    res[7] = mat[6];
    res[8] = mat[10];

    res[9] = mat[3];
    res[10] = mat[7];
    res[11] = mat[11];

    res[12] = -mat[12];
    res[13] = -mat[13];
    res[14] = -mat[14];

    res[15] = mat[15];*/

    res[0] = mat[0];
    res[1] = mat[4];
    res[2] = mat[8];
    res[3] = mat[12];

    res[4] = mat[1];
    res[5] = mat[5];
    res[6] = mat[9];
    res[7] = mat[13];

    res[8] = mat[2];
    res[9] = mat[6];
    res[10] = mat[10];
    res[11] = mat[14];

    res[12] = mat[3];
    res[13] = mat[7];
    res[14] = mat[11];
    res[15] = mat[15];

}

bool gluInvertMatrix(const float m[16], float invOut[16]) {
    float inv[16], det;
    int i;

    inv[0] = m[5] * m[10] * m[15] -
             m[5] * m[11] * m[14] -
             m[9] * m[6] * m[15] +
             m[9] * m[7] * m[14] +
             m[13] * m[6] * m[11] -
             m[13] * m[7] * m[10];

    inv[4] = -m[4] * m[10] * m[15] +
             m[4] * m[11] * m[14] +
             m[8] * m[6] * m[15] -
             m[8] * m[7] * m[14] -
             m[12] * m[6] * m[11] +
             m[12] * m[7] * m[10];

    inv[8] = m[4] * m[9] * m[15] -
             m[4] * m[11] * m[13] -
             m[8] * m[5] * m[15] +
             m[8] * m[7] * m[13] +
             m[12] * m[5] * m[11] -
             m[12] * m[7] * m[9];

    inv[12] = -m[4] * m[9] * m[14] +
              m[4] * m[10] * m[13] +
              m[8] * m[5] * m[14] -
              m[8] * m[6] * m[13] -
              m[12] * m[5] * m[10] +
              m[12] * m[6] * m[9];

    inv[1] = -m[1] * m[10] * m[15] +
             m[1] * m[11] * m[14] +
             m[9] * m[2] * m[15] -
             m[9] * m[3] * m[14] -
             m[13] * m[2] * m[11] +
             m[13] * m[3] * m[10];

    inv[5] = m[0] * m[10] * m[15] -
             m[0] * m[11] * m[14] -
             m[8] * m[2] * m[15] +
             m[8] * m[3] * m[14] +
             m[12] * m[2] * m[11] -
             m[12] * m[3] * m[10];

    inv[9] = -m[0] * m[9] * m[15] +
             m[0] * m[11] * m[13] +
             m[8] * m[1] * m[15] -
             m[8] * m[3] * m[13] -
             m[12] * m[1] * m[11] +
             m[12] * m[3] * m[9];

    inv[13] = m[0] * m[9] * m[14] -
              m[0] * m[10] * m[13] -
              m[8] * m[1] * m[14] +
              m[8] * m[2] * m[13] +
              m[12] * m[1] * m[10] -
              m[12] * m[2] * m[9];

    inv[2] = m[1] * m[6] * m[15] -
             m[1] * m[7] * m[14] -
             m[5] * m[2] * m[15] +
             m[5] * m[3] * m[14] +
             m[13] * m[2] * m[7] -
             m[13] * m[3] * m[6];

    inv[6] = -m[0] * m[6] * m[15] +
             m[0] * m[7] * m[14] +
             m[4] * m[2] * m[15] -
             m[4] * m[3] * m[14] -
             m[12] * m[2] * m[7] +
             m[12] * m[3] * m[6];

    inv[10] = m[0] * m[5] * m[15] -
              m[0] * m[7] * m[13] -
              m[4] * m[1] * m[15] +
              m[4] * m[3] * m[13] +
              m[12] * m[1] * m[7] -
              m[12] * m[3] * m[5];

    inv[14] = -m[0] * m[5] * m[14] +
              m[0] * m[6] * m[13] +
              m[4] * m[1] * m[14] -
              m[4] * m[2] * m[13] -
              m[12] * m[1] * m[6] +
              m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] +
             m[1] * m[7] * m[10] +
             m[5] * m[2] * m[11] -
             m[5] * m[3] * m[10] -
             m[9] * m[2] * m[7] +
             m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] -
             m[0] * m[7] * m[10] -
             m[4] * m[2] * m[11] +
             m[4] * m[3] * m[10] +
             m[8] * m[2] * m[7] -
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] +
              m[0] * m[7] * m[9] +
              m[4] * m[1] * m[11] -
              m[4] * m[3] * m[9] -
              m[8] * m[1] * m[7] +
              m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] -
              m[0] * m[6] * m[9] -
              m[4] * m[1] * m[10] +
              m[4] * m[2] * m[9] +
              m[8] * m[1] * m[6] -
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return false;

    det = 1.0f / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

#undef PI
#undef PI_OVER_360
#undef I
#undef my_normalize
