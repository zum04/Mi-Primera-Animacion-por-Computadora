#include <GL/glut.h>
#include <math.h>

#define MAX_TRANSFORMACIONES 1000

typedef float Mat4[16];
Mat4 transformaciones[MAX_TRANSFORMACIONES];
int numTransformaciones = 0;

float angle = 0.0f;

void drawVector2D(float x0, float y0, float x1, float y1) {
    glBegin(GL_LINES);
    glVertex2f(x0, y0);
    glVertex2f(x1, y1);
    glEnd();
}

void multMat(Mat4 out, const Mat4 a, const Mat4 b);
void matIdentidad(Mat4 m);
void matRotacion(Mat4 m, float angulo);
void aplicarTransformaciones(Mat4* list, int count);
void update(int value);
void display();
void reshape(int width, int height);

void init() {
    glClearColor(0, 0, 0, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 450);  // 16:9 ratio inicial
    glutCreateWindow("Vector Giratorio con Aspecto 16:9");

    init();
    reshape(800, 450);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    // Ejes X e Y en verde
    glColor3f(0.5f, 0.5f, 0.5f);
    drawVector2D(-1, 0, 1, 0);  // eje X
    drawVector2D(0, -1, 0, 1);  // eje Y


    aplicarTransformaciones(transformaciones, numTransformaciones);

    // Vector rojo que rota
    glColor3f(1, 0, 0);
    drawVector2D(0.0f, 0.0f, 0.5f, 0.0f);

    glutSwapBuffers();
}

void update(int value) {
    angle += 2.0f;
    if (angle >= 360.0f) angle -= 360.0f;

    matRotacion(transformaciones[0], angle);
    numTransformaciones = 1;

    glutPostRedisplay();
    glutTimerFunc(8, update, 0);
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
    float rad = angulo * 3.1415926f / 180.0f;
    m[0] = cos(rad);
    m[1] = sin(rad);
    m[4] = -sin(rad);
    m[5] = cos(rad);
}

void aplicarTransformaciones(Mat4* list, int count) {
    Mat4 combined;
    matIdentidad(combined);

    for (int i = 0; i < count; i++)
        multMat(combined, combined, list[i]);

    glLoadMatrixf(combined);
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
