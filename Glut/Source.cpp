#include <GL/glut.h>
#include <math.h>

#define MAX_VECTORES 20
#define MAX_GRUPOS 10

typedef float Mat4[16];

typedef struct {
    float x0, y0;
    float x1, y1;
} Vector2D;

typedef struct {
    Vector2D vectores[MAX_VECTORES];
    int numVectores;
    Mat4 transformacion;
    int estado;       // 0 = rotar, 1 = trasladar, 2 = escalar
    int tiempo;       // Tiempo acumulado
    float angulo;     // Ángulo interno por grupo
} Grupo;

Grupo grupos[MAX_GRUPOS];
int numGrupos = 0;

const float PI = 3.14159;

void drawVector2D(float x0, float y0, float x1, float y1) {
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void multMat(Mat4 out, const Mat4 a, const Mat4 b);
void matIdentidad(Mat4 m);
void matRotacion(Mat4 m, float angulo);
void matTraslacion(Mat4 m, float tx, float ty);
void matEscalado(Mat4 m, float sx, float sy);

void aplicarTransformacionYdibujarGrupo(Grupo* grupo) {
    glPushMatrix();
    glMultMatrixf(grupo->transformacion);
    for (int i = 0; i < grupo->numVectores; i++) {
        drawVector2D(grupo->vectores[i].x0, grupo->vectores[i].y0,
            grupo->vectores[i].x1, grupo->vectores[i].y1);
    }
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    glColor3f(0.5f, 0.5f, 0.5f);
    drawVector2D(-1, 0, 1, 0);
    drawVector2D(0, -1, 0, 1);

    glColor3f(1, 0, 0);
    aplicarTransformacionYdibujarGrupo(&grupos[0]);

    glColor3f(0.5f, 0, 0);
    aplicarTransformacionYdibujarGrupo(&grupos[1]);

    glutSwapBuffers();
}

void update(int value) {
    for (int i = 0; i < numGrupos; i++) {
        Grupo* g = &grupos[i];
        g->tiempo += 50;

        Mat4 nueva;
        matIdentidad(nueva);

        if (g->estado == 0) {
            g->angulo += 2.0f;
            matRotacion(nueva, g->angulo);
        }
        else if (g->estado == 1) {
            float tx = sin(g->angulo * PI / 180.0f) * 0.5f;
            matTraslacion(nueva, tx, 0);
        }
        else if (g->estado == 2) {
            float scale = 0.5f + 0.5f * sin(g->angulo * PI / 180.0f);
            matEscalado(nueva, scale, scale);
        }

        for (int j = 0; j < 16; j++)
            g->transformacion[j] = nueva[j];

        if (g->tiempo > 5000) {
            g->tiempo = 0;
            g->estado = (g->estado + 1) % 3;
        }

        g->angulo += 2.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(50, update, 0);
}

void reshape(int width, int height) {
    if (height == 0) height = 1;
    float windowAspect = (float)width / (float)height;
    const float targetAspect = 16.0f / 9.0f;
    int vpX, vpY, vpWidth, vpHeight;

    if (windowAspect > targetAspect) {
        vpHeight = height;
        vpWidth = (int)(height * targetAspect);
        vpX = (width - vpWidth) / 2;
        vpY = 0;
    }
    else {
        vpWidth = width;
        vpHeight = (int)(width / targetAspect);
        vpX = 0;
        vpY = (height - vpHeight) / 2;
    }

    glViewport(vpX, vpY, vpWidth, vpHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspect = (float)vpWidth / (float)vpHeight;
    if (aspect >= 1.0f) {
        gluOrtho2D(-aspect, aspect, -1.0, 1.0);
    }
    else {
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void initGrupos() {
    Grupo* g0 = &grupos[0];
    g0->numVectores = 0;

    Vector2D v;
    v.x0 = -0.3f; v.y0 = -0.1f; v.x1 = 0.3f; v.y1 = -0.1f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = -0.3f; v.y0 = -0.1f; v.x1 = -0.5f; v.y1 = 0.1f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = 0.3f; v.y0 = -0.1f; v.x1 = 0.5f; v.y1 = 0.1f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = -0.5f; v.y0 = 0.1f; v.x1 = 0.5f; v.y1 = 0.1f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = -0.2f; v.y0 = 0.1f; v.x1 = 0; v.y1 = 0.3f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = 0.2f; v.y0 = 0.1f; v.x1 = 0; v.y1 = 0.3f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = 0; v.y0 = 0.1f; v.x1 = 0; v.y1 = 0.3f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = -0.3f; v.y0 = -0.1f; v.x1 = 0; v.y1 = 0.1f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = 0.3f; v.y0 = -0.1f; v.x1 = 0; v.y1 = 0.1f;
    g0->vectores[g0->numVectores++] = v;
    v.x0 = 0; v.y0 = 0.1f; v.x1 = 0; v.y1 = -0.1f;
    g0->vectores[g0->numVectores++] = v;
    matIdentidad(g0->transformacion);

    numGrupos = 1;
}

void init() {
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    initGrupos();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 450);
    glutCreateWindow("Animacion con Grupos de Vectores");

    init();
    reshape(800, 450);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(50, update, 0);
    glutMainLoop();
    return 0;
}

void multMat(Mat4 out, const Mat4 a, const Mat4 b) {
    Mat4 result;
    for (int fila = 0; fila < 4; fila++) {
        for (int col = 0; col < 4; col++) {
            result[col * 4 + fila] = 0;
            for (int i = 0; i < 4; i++) {
                result[col * 4 + fila] += a[i * 4 + fila] * b[col * 4 + i];
            }
        }
    }
    for (int i = 0; i < 16; i++) {
        out[i] = result[i];
    }
}

void matIdentidad(Mat4 m) {
    for (int i = 0; i < 16; i++)
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
}

void matRotacion(Mat4 m, float angulo) {
    matIdentidad(m);
    float rad = angulo * PI / 180.0f;
    m[0] = cos(rad);
    m[1] = sin(rad);
    m[4] = -sin(rad);
    m[5] = cos(rad);
}

void matTraslacion(Mat4 m, float tx, float ty) {
    matIdentidad(m);
    m[12] = tx;
    m[13] = ty;
}

void matEscalado(Mat4 m, float sx, float sy) {
    matIdentidad(m);
    m[0] = sx;
    m[5] = sy;
}