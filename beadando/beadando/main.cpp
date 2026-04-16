#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <vector>
#include <cmath>
#include <iostream>

// Ablak mérete
const int WIDTH = 800;
const int HEIGHT = 600;

// Kontrollpont sugara pixelben
const float POINT_RADIUS = 6.0f;

// Saját PI definíció
const float PI = 3.14159265359f;

std::vector<glm::vec2> controlPoints;
int selectedPoint = -1;
bool dragging = false;

// Színek
glm::vec3 curveColor(1.0f, 0.0f, 0.0f);    // piros
glm::vec3 polygonColor(0.0f, 0.0f, 1.0f);  // kék
glm::vec3 pointColor(0.0f, 0.8f, 0.0f);    // zöld

// Képernyő koordináta -> NDC
glm::vec2 screenToNDC(double x, double y) {
    return glm::vec2(
        (2.0f * x) / WIDTH - 1.0f,
        1.0f - (2.0f * y) / HEIGHT
    );
}

// Binomiális együttható
int binomial(int n, int k) {
    if (k == 0 || k == n) return 1;
    int res = 1;
    for (int i = 1; i <= k; ++i)
        res = res * (n - k + i) / i;
    return res;
}

// Bézier pont számítása
glm::vec2 bezierPoint(float t) {
    int n = static_cast<int>(controlPoints.size()) - 1;
    glm::vec2 point(0.0f);

    for (int i = 0; i <= n; ++i) {
        float coeff = binomial(n, i) *
            pow(1.0f - t, n - i) *
            pow(t, i);
        point += coeff * controlPoints[i];
    }
    return point;
}

// Kontrollpont keresése
int findPoint(double x, double y) {
    glm::vec2 p = screenToNDC(x, y);
    for (size_t i = 0; i < controlPoints.size(); ++i) {
        if (glm::distance(p, controlPoints[i]) < 0.05f)
            return static_cast<int>(i);
    }
    return -1;
}

// Kör kirajzolása
void drawCircle(const glm::vec2& center) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(center.x, center.y);
    for (int i = 0; i <= 40; ++i) {
        float angle = 2.0f * PI * i / 40.0f;
        float dx = cos(angle) * (POINT_RADIUS / WIDTH) * 2.0f;
        float dy = sin(angle) * (POINT_RADIUS / HEIGHT) * 2.0f;
        glVertex2f(center.x + dx, center.y + dy);
    }
    glEnd();
}

// Egérgomb események
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        int idx = findPoint(xpos, ypos);
        if (idx != -1) {
            selectedPoint = idx;
            dragging = true;
        }
        else {
            controlPoints.push_back(screenToNDC(xpos, ypos));
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        dragging = false;
        selectedPoint = -1;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        int idx = findPoint(xpos, ypos);
        if (idx != -1) {
            controlPoints.erase(controlPoints.begin() + idx);
        }
    }
}

// Egér mozgatás (drag & drop)
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (dragging && selectedPoint != -1) {
        controlPoints[selectedPoint] = screenToNDC(xpos, ypos);
    }
}

// Kontrollpoligon rajzolása
void drawControlPolygon() {
    if (controlPoints.size() < 2) return;

    glColor3f(polygonColor.r, polygonColor.g, polygonColor.b);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (const auto& p : controlPoints)
        glVertex2f(p.x, p.y);
    glEnd();
}

// Bézier-görbe rajzolása
void drawBezierCurve() {
    if (controlPoints.size() < 2) return;

    glColor3f(curveColor.r, curveColor.g, curveColor.b);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);

    const int segments = 300; // részletesség
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        glm::vec2 p = bezierPoint(t);
        glVertex2f(p.x, p.y);
    }
    glEnd();
}

// Kontrollpontok rajzolása
void drawControlPoints() {
    glColor3f(pointColor.r, pointColor.g, pointColor.b);
    for (const auto& p : controlPoints)
        drawCircle(p);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW inicializálási hiba!" << std::endl;
        return -1;
    }

    // Kompatibilis OpenGL profil az immediate mode használatához
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Bezier Curve Editor", NULL, NULL);
    if (!window) {
        std::cerr << "Ablak létrehozási hiba!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW inicializálási hiba!" << std::endl;
        return -1;
    }

    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glViewport(0, 0, WIDTH, HEIGHT);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // fehér háttér

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        drawControlPolygon();
        drawBezierCurve();
        drawControlPoints();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}