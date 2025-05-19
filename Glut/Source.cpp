#include <GL/glut.h>
#include <math.h>

#define MAX_VECTORES 200
#define MAX_GRUPOS 10

typedef float Mat4[16];

typedef struct {
    float tiempoInicio;
    float tiempoFin;
    float anguloActual;
    float desplazamiento;
    float fase;
} EstadoAnimacion;

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

EstadoAnimacion estadosAnimacion[MAX_GRUPOS];
Grupo grupos[MAX_GRUPOS];
int numGrupos = 0;
int tiempoEstado = 0;
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
void actualizarAnimacion(int);
void reshape(int, int);

// Se añade el parámetro tiempoActual para decidir qué vectores dibujar
void aplicarTransformacionYdibujarGrupo(Grupo* grupo, int tiempoActual) {
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
    aplicarTransformacionYdibujarGrupo(&grupos[0], tiempoEstado);

    glColor3f(0.5f, 0.5f, 1);
    aplicarTransformacionYdibujarGrupo(&grupos[1], tiempoEstado);

    glutSwapBuffers();
}

void actualizarAnimacion(int tiempoGlobal) {
    for (int i = 0; i < numGrupos; i++) {
        Grupo* grupo = &grupos[i];
        EstadoAnimacion* estado = &estadosAnimacion[i];

        // Para grupo 1 (onda seno), solo dibujar si ya empezó
        if (i == 1) {
            if (tiempoGlobal < estado->tiempoInicio) {
                grupo->numVectores = 0;
                matIdentidad(grupo->transformacion);
                continue;
            }

            // tiempoParaFase no puede superar el tiempoFin
            int tiempoParaFase = tiempoGlobal;
            if (tiempoParaFase > estado->tiempoFin)
                tiempoParaFase = estado->tiempoFin;

            float tiempoSegundos = tiempoParaFase / 1000.0f;
            float A = 0.05f;
            float f = 8.0f;
            float fase = tiempoSegundos * 4.0f;

            float xStart = -1.0f;
            float xEnd = 1.0f;
            int numSegmentos = 200;

            grupo->numVectores = 0;
            for (int j = 0; j < numSegmentos; j++) {
                float x0 = xStart + (xEnd - xStart) * j / numSegmentos;
                float x1 = xStart + (xEnd - xStart) * (j + 1) / numSegmentos;
                float y0 = A * sinf(f * x0 - fase) - 1.5f;
                float y1 = A * sinf(f * x1 - fase) - 0.2f;

                if (grupo->numVectores < MAX_VECTORES) {
                    grupo->vectores[grupo->numVectores].x0 = x0;
                    grupo->vectores[grupo->numVectores].y0 = y0;
                    grupo->vectores[grupo->numVectores].x1 = x1;
                    grupo->vectores[grupo->numVectores].y1 = y1;
                    grupo->numVectores++;
                }
            }
            matIdentidad(grupo->transformacion);
            continue;
        }

        // Para otros grupos, calcular tiempo local sin modificar tiempoGlobal
        if (tiempoGlobal < estado->tiempoInicio) {
            // No iniciar animación
            matIdentidad(grupo->transformacion);
            continue;
        }

        int tiempoLocal = tiempoGlobal;
        if (tiempoLocal > estado->tiempoFin)
            tiempoLocal = estado->tiempoFin;

        float t = (tiempoLocal - estado->tiempoInicio) / (float)(estado->tiempoFin - estado->tiempoInicio);
        float angulo = t * 360.0f;

        matIdentidad(grupo->transformacion);
        matRotacion(grupo->transformacion, angulo);
    }
}


void update(int value) {
    tiempoEstado += 50;
    actualizarAnimacion(tiempoEstado);
    glutPostRedisplay();
    glutTimerFunc(50, update, 0);
}

void initGrupos() {
    // ----- Grupo 0: figura compuesta -----
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

    estadosAnimacion[0].tiempoInicio = 0;
    estadosAnimacion[0].tiempoFin = 5000;
    estadosAnimacion[0].anguloActual = 0.0f;
    estadosAnimacion[0].desplazamiento = 0.0f;

    // ----- Grupo 1: curva seno -----
    Grupo* g1 = &grupos[1];
    g1->numVectores = 0;
    matIdentidad(g1->transformacion);

    // Nota: aquí solo inicializamos vectores, pero serán actualizados dinámicamente
    estadosAnimacion[1].tiempoInicio = 1000;  // Ejemplo: empieza en 2s
    estadosAnimacion[1].tiempoFin = 8000;
    estadosAnimacion[1].anguloActual = 0.0f;
    estadosAnimacion[1].desplazamiento = 0.0f;

    numGrupos = 2;
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
    glutInitWindowSize(900, 900);
    glutCreateWindow("Animacion con Grupos de Vectores");

    init();
    reshape(900, 900);
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

void reshape(int width, int height) {
    if (height == 0) height = 1;
    float aspect = (float)width / (float)height;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (aspect >= 1.0f) {
        // Ventana más ancha que alta
        gluOrtho2D(-1.0 * aspect, 1.0 * aspect, -1.0, 1.0);
    }
    else {
        // Ventana más alta que ancha
        gluOrtho2D(-1.0, 1.0, -1.0 / aspect, 1.0 / aspect);
    }

    glMatrixMode(GL_MODELVIEW);
}