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
    glClearColor(0.12f, 0.12f, 0.15f, 1.0f); // Fondo azul claro
    glLoadIdentity();
    for (int i = 0; i < numGrupos; i++) {
        EstadoAnimacion* estado = &estadosAnimacion[i];
        if (tiempoEstado >= estado->tiempoInicio && tiempoEstado <= estado->tiempoFin) {
            // Cambia color según grupo o como quieras
            if (i == 0) glColor3f(1, 0, 0);
            else if (i == 1) glColor3f(0.5f, 0.5f, 1);
            else if (i == 2) glColor3f(0, 1, 0);
            else glColor3f(1, 1, 0);

            aplicarTransformacionYdibujarGrupo(&grupos[i], tiempoEstado);
        }
    }

    glutSwapBuffers();
}

float easeInOut(float t) {
    return t * t * (3 - 2 * t);  // Hermite curve
}

void actualizarAnimacion(int tiempoGlobal) {
    for (int i = 0; i < numGrupos; i++) {
        Grupo* grupo = &grupos[i];
        EstadoAnimacion* estado = &estadosAnimacion[i];
        if (tiempoGlobal < estado->tiempoInicio || tiempoGlobal > estado->tiempoFin) {
            grupo->numVectores = 0;
            matIdentidad(grupo->transformacion);
            continue;
        }
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
            float A = 0.02f;

            // Frecuencia variable en el tiempo
            float fMin = 5.0f;
            float fMax = 12.0f;
            float ciclo = 5.0f;  // segundos para completar una oscilación de frecuencia
            float f = fMax;

            if (tiempoGlobal >= 3000 && tiempoGlobal <= 5000) {
                float t = (tiempoGlobal - 3000) / 2000.0f;  // Normalizado de 0.0 a 1.0
                t = easeInOut(t);  // Aplicamos easing
                f = fMax + (fMin - fMax) * t;  // Interpolación con curva
            }
            else if (tiempoGlobal > 5000) {
                f = fMin;
            }

            float fase = tiempoSegundos * 4.0f;

            float xStart = -1.0f;
            float xEnd = 2.0f;
            int numSegmentos = 220;

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

        if (i == 0) {
            float desplazamientoInicialY = -0.2f;

            if (tiempoGlobal < estado->tiempoInicio || tiempoGlobal > estado->tiempoFin) {
                matIdentidad(grupo->transformacion);
                continue;
            }

            float tTambaleoSeg = (tiempoGlobal - estado->tiempoInicio) / 1000.0f;
            float angulo = sinf(tTambaleoSeg * 2.0f) * 3.0f;

            // Parámetros para escalar y trasladar, con desfase de 3 segundos
            int inicioEscalaYtrasl = estado->tiempoInicio + 3000;
            int finEscalaYtrasl = inicioEscalaYtrasl + 5000;

            float tMov = 0.0f;
            if (tiempoGlobal >= inicioEscalaYtrasl) {
                float tRaw = (tiempoGlobal - inicioEscalaYtrasl) / (float)(finEscalaYtrasl - inicioEscalaYtrasl);
                if (tRaw > 1.0f) tRaw = 1.0f;
                if (tRaw < 0.0f) tRaw = 0.0f;

                // Ease in-out (curva suave tiRecuerdpo S)
                tMov = tRaw * tRaw * (3 - 2 * tRaw);
                if (tMov > 1.0f) tMov = 1.0f;
            }

            float desplazamientoX = 0.0f;

            if (tiempoGlobal >= 10000 && tiempoGlobal <= 12000) {
                float t = (tiempoGlobal - 10000) / 2000.0f;  // Normalizado de 0.0 a 1.0
                t = easeInOut(t);  // Transición suave
                desplazamientoX = 2.0f * t;  // De x = 0 a x = 2.0
            }
            else if (tiempoGlobal > 12000) {
                desplazamientoX = 2.0f;
            }

            // Movimiento de izquierda a centro (x = -0.7 a x = 0)
            float posX = -0.7f + tMov * 0.7f;

            posX += desplazamientoX;

            // Movimiento vertical adicional (de -0.2 a 0.3 por ejemplo)
            float posY = -0.2f + tMov * 0.05f;  // Subir 0.5 unidades mientras avanza

            // Escala de 0.3x a 1.0x
            float escala = 0.3f + tMov * 0.7f;

            // Construir transformación final: T(x) * T(-y) * R(ángulo) * T(y) * S(escala)
            matIdentidad(grupo->transformacion);

            Mat4 t1, r, t2, s, tpos, temp1, temp2, temp3;

            matTraslacion(t1, 0.0f, -0.1f);         // Mover al centro para rotar
            matRotacion(r, angulo);                // Tambaleo constante
            matTraslacion(t2, 0.0f, 0.1f);          // Regresar a posición original
            matEscalado(s, escala, escala);        // Escalado progresivo
            matTraslacion(tpos, posX, posY);        // Traslación progresiva

            // Componer la matriz: T(posX) * T2 * R * T1 * S
            multMat(temp1, t1, s);                 // T1 * S
            multMat(temp2, r, temp1);              // R * (T1 * S)
            multMat(temp3, t2, temp2);             // T2 * R * T1 * S
            multMat(grupo->transformacion, tpos, temp3); // Tpos * todo lo anterior

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
    estadosAnimacion[0].tiempoFin = 20000;
    estadosAnimacion[0].anguloActual = 0.0f;
    estadosAnimacion[0].desplazamiento = 0.0f;

    // ----- Grupo 1: curva seno -----
    Grupo* g1 = &grupos[1];
    g1->numVectores = 0;
    matIdentidad(g1->transformacion);

    // Nota: aquí solo inicializamos vectores, pero serán actualizados dinámicamente
    estadosAnimacion[1].tiempoInicio = 0;  // Ejemplo: empieza en 2s
    estadosAnimacion[1].tiempoFin = 20000;
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