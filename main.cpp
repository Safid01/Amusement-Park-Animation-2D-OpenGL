// ============================================================================
// MERGED PROJECT -- 4 independent 2D OpenGL/GLUT amusement-park scenes
// combined into a single program. Scene switching only -- no transition
// animation -- exactly as requested.
//
//   Scene 1 = original main.cpp     (Pirate Ship)
//   Scene 2 = original main2.cpp    (Ferris Wheel)
//   Scene 3 = original project.cpp  (Water Park - traced shapes)
//   Scene 4 = original main1.cpp    (Sky Drop ride)
//
// Controls:
//   1 / 2 / 3 / 4   -> jump directly to that scene
//   Right Arrow     -> next scene
//   Left Arrow      -> previous scene
//
// IMPORTANT: none of the four original programs' drawing, vertex, or
// animation logic has been changed in any way. Each program's code is
// kept verbatim inside its own C++ namespace (Scene1..Scene4) below. The
// namespaces exist ONLY so that the four files' identically-named
// functions/variables (display(), init(), main(), isPaused, drawCircle(),
// etc.) -- and project.cpp/main1.cpp's clashing "PI" -- can coexist in one
// translation unit. Wrapping existing code in a namespace does not change
// what that code computes or draws.
// ============================================================================

#ifdef _WIN32
    #include <windows.h>
    #include <GL/glut.h>
#elif defined(__APPLE__)
    #define GL_SILENCE_DEPRECATION
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#else
    #include <GL/glut.h>
#endif
#include <cmath>
#include <cstdlib>

// Which scene is currently shown on screen: 1, 2, 3, or 4.
int currentScene = 1;

// ============================================================================
// SCENE 1 -- original main.cpp (verbatim, unmodified)
// ============================================================================
namespace Scene1 {

// ============================================================================
// Course: Computer Graphics (CSC 3224)
// Project: Interactive 2D Amusement Park Simulation with Swinging Pirate Ship
// Syllabus Backend: DDA Lines, 2D Transformations (Translation, Rotation,
//                   Scaling, Reflection, Shearing), Point Clipping, Bezier Curves.
// Coordinate System: First-Quadrant [0, 1280] x [0, 720]
// Numbering Scheme: Objects (Obj-101 to Obj-125) & Animations (A-101 to A-109)
// ============================================================================


#define PI 3.14159265

// GLOBAL STATE & ANIMATION VARIABLES
// ============================================================================

// [A-101]: Pirate Ship Pendulum Motion Variables[cite: 9]
float swingAngle = 0.0f;
float maxSwingAngle = 55.0f;
float swingSpeed = 1.6f;
float timeStep = 0.0f;

// [A-102]: Ferris Wheel Rotation Variable[cite: 9]
float ferrisWheelAngle = 0.0f;

// [A-103]: Day/Night Cycle Parameters (12.0s Total: 6.0s Day, 6.0s Night)
float cycleTimer = 0.0f;
const float totalCycleTime = 12.0f;
float nightFactor = 0.0f;

// [A-104]: Celestial Coordinates (Sun & Moon Arc Trajectories)[cite: 9]
float sunX = 640.0f, sunY = -100.0f;
float moonX = 640.0f, moonY = -100.0f;

// System Controls (Starts paused at theta = 0)
bool isPaused = false;

// Starfield Pool
const int numStars = 65;
float starX[numStars];
float starY[numStars];

// DATA STRUCTURES FOR ENTITIES
// ============================================================================

struct Point { float x, y; };
struct Cloud { float x, y, scale, speed; };
struct Bird  { float x, y, speed, wingAngle, dir; };
struct Human { float x, y, dir, speed, legAngle; float r, g, b; };

// Global Entity Instances
Cloud clouds[] = {
    {  40.0f, 540.0f, 1.2f, 0.35f},
    { 440.0f, 590.0f, 0.8f, 0.22f},
    { 790.0f, 500.0f, 1.4f, 0.40f},
    {1140.0f, 560.0f, 1.0f, 0.28f}
};
const int numClouds = 4;

Bird birds[] = {
    {190.0f, 470.0f, 1.6f, 0.0f, 1.0f},
    {150.0f, 495.0f, 1.6f, 1.5f, 1.0f},
    {840.0f, 580.0f, 1.2f, 0.8f, 1.0f}
};
const int numBirds = 3;

// Expanded Human Entities (12 Walking Park Visitors)
Human humans[] = {
    {  20.0f, 76.0f,  1.0f, 0.75f, 0.0f, 0.90f, 0.25f, 0.20f},
    { 140.0f, 74.0f,  1.0f, 0.60f, 2.0f, 0.20f, 0.60f, 0.90f},
    { 250.0f, 76.0f, -1.0f, 0.70f, 1.0f, 0.95f, 0.80f, 0.15f},
    { 370.0f, 74.0f,  1.0f, 0.65f, 3.5f, 0.30f, 0.80f, 0.40f},
    { 490.0f, 76.0f, -1.0f, 0.55f, 0.8f, 0.95f, 0.50f, 0.15f},
    { 610.0f, 74.0f,  1.0f, 0.70f, 4.2f, 0.70f, 0.30f, 0.80f},
    { 730.0f, 76.0f, -1.0f, 0.60f, 1.7f, 0.20f, 0.75f, 0.75f},
    { 850.0f, 74.0f,  1.0f, 0.68f, 5.0f, 0.85f, 0.35f, 0.45f},
    { 960.0f, 76.0f, -1.0f, 0.72f, 2.4f, 0.40f, 0.55f, 0.90f},
    {1080.0f, 74.0f,  1.0f, 0.58f, 3.1f, 0.80f, 0.60f, 0.20f},
    {1190.0f, 76.0f, -1.0f, 0.64f, 0.5f, 0.35f, 0.70f, 0.45f},
    {1290.0f, 74.0f, -1.0f, 0.66f, 4.0f, 0.85f, 0.40f, 0.25f}
};
const int numHumans = 12;

// ============================================================================
// MATHEMATICAL CORE FUNCTIONS (SYLLABUS IMPLEMENTATIONS)
// ============================================================================

// Linear Interpolation: Lerp(a, b, t) = a + (b - a) * t
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

// [Syllabus Ch 8: Point Clipping Algorithm][cite: 5]
// Checks if coordinates are inside the bounding box xwmin < x < xwmax and ywmin < y < ywmax
bool pointClipping(float x, float y, float xwmin, float xwmax, float ywmin, float ywmax) {
    return (x >= xwmin && x <= xwmax && y >= ywmin && y <= ywmax);
}

// [Obj-124: Syllabus Midterm - DDA Line Algorithm][cite: 10]
// Scan-converts a line using standard differential steps
void drawLineDDA_obj124(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float steps = (std::abs(dx) > std::abs(dy)) ? std::abs(dx) : std::abs(dy);
    float xInc = dx / steps;
    float yInc = dy / steps;
    float x = x1, y = y1;

    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i <= (int)steps; i++) {
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

// [Syllabus Ch 3: 2D Shearing Homogeneous Matrix][cite: 8]
void applyShearX_obj124(float shx) {
    GLfloat m[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        shx,  1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    glMultMatrixf(m);
}

// [Syllabus Ch 5: Quadratic Bézier Polynomial][cite: 6]
// B(t) = (1-t)^2 * P0 + 2*(1-t)*t * P1 + t^2 * P2
Point getBezierPoint(Point p0, Point p1, Point p2, float t) {
    float u = 1.0f - t;
    Point p;
    p.x = (u * u * p0.x) + (2.0f * u * t * p1.x) + (t * t * p2.x);
    p.y = (u * u * p0.y) + (2.0f * u * t * p1.y) + (t * t * p2.y);
    return p;
}

// Helper: Circle Drawing using GL_TRIANGLE_FAN
void drawCircle(float cx, float cy, float r, int segments, float red, float green, float blue, float alpha = 1.0f) {
    glColor4f(red, green, blue, alpha);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * (float)M_PI * (float)i / (float)segments;
        glVertex2f(cx + r * cosf(angle), cy + r * sinf(angle));
    }
    glEnd();
}

// Helper: Circle Outline using GL_LINE_LOOP
void drawCircleOutline(float cx, float cy, float r, int segments, float lineWidth, float red, float green, float blue) {
    glLineWidth(lineWidth);
    glColor3f(red, green, blue);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < segments; ++i) {
        float angle = 2.0f * (float)M_PI * (float)i / (float)segments;
        glVertex2f(cx + r * cosf(angle), cy + r * sinf(angle));
    }
    glEnd();
    glLineWidth(1.0f);
}

// ============================================================================
// OBJECT DRAWING FUNCTIONS (WITH NEW SUFFIX SCHEME)
// ============================================================================

// [Obj-101]: Sky Gradient
void drawSky_obj101() {
    float skyTopR = lerp(0.52f, 0.03f, nightFactor);
    float skyTopG = lerp(0.76f, 0.04f, nightFactor);
    float skyTopB = lerp(0.95f, 0.12f, nightFactor);
    float skyBotR = lerp(0.85f, 0.08f, nightFactor);
    float skyBotG = lerp(0.93f, 0.12f, nightFactor);
    float skyBotB = lerp(0.98f, 0.25f, nightFactor);

    glBegin(GL_QUADS);
    glColor3f(skyTopR, skyTopG, skyTopB);
    glVertex2f(-2000.0f, 2000.0f);
    glVertex2f( 3500.0f, 2000.0f);
    glColor3f(skyBotR, skyBotG, skyBotB);
    glVertex2f( 3500.0f, -500.0f);
    glVertex2f(-2000.0f, -500.0f);
    glEnd();
}

// [Obj-102]: Starfield (Using Chapter 8 Point Clipping)[cite: 5]
void drawStars_obj102() {
    if (nightFactor > 0.05f) {
        for (int i = 0; i < numStars; ++i) {
            if (pointClipping(starX[i], starY[i], -1000.0f, 2000.0f, 300.0f, 1000.0f)) {
                drawCircle(starX[i], starY[i], 1.5f, 6, 1.0f, 1.0f, 1.0f, nightFactor);
            }
        }
    }
}

// [Obj-103]: Sun
void drawSun_obj103() {
    if (sunY > 60.0f) {
        drawCircle(sunX, sunY, 40.0f, 24, 1.0f, 0.85f, 0.30f, 0.40f);
        drawCircle(sunX, sunY, 25.0f, 20, 1.0f, 0.95f, 0.50f, 1.0f);
    }
}

// [Obj-104]: Moon
void drawMoon_obj104() {
    if (moonY > 60.0f) {
        float skyTopR = lerp(0.52f, 0.03f, nightFactor);
        float skyTopG = lerp(0.76f, 0.04f, nightFactor);
        float skyTopB = lerp(0.95f, 0.12f, nightFactor);

        drawCircle(moonX, moonY, 30.0f, 20, 0.85f, 0.90f, 1.0f, 0.35f);
        drawCircle(moonX, moonY, 20.0f, 20, 0.98f, 0.96f, 0.85f, 1.0f);
        drawCircle(moonX + 8.0f, moonY + 4.0f, 17.0f, 20, skyTopR, skyTopG, skyTopB, 1.0f);
    }
}

// [Obj-105]: Drifting Clouds
void drawClouds_obj105() {
    for (int i = 0; i < numClouds; ++i) {
        glPushMatrix();
        glTranslatef(clouds[i].x, clouds[i].y, 0.0f);
        glScalef(clouds[i].scale, clouds[i].scale, 1.0f);

        float r = lerp(1.0f, 0.18f, nightFactor);
        float g = lerp(1.0f, 0.20f, nightFactor);
        float b = lerp(1.0f, 0.26f, nightFactor);
        drawCircle(0.0f, 0.0f, 25.0f, 18, r, g, b);
        drawCircle(-20.0f, -5.0f, 18.0f, 16, r, g, b);
        drawCircle(20.0f, -5.0f, 18.0f, 16, r, g, b);
        glPopMatrix();
    }
}

// [Obj-106]: Flying Birds (Using 2D Mirror Reflection)[cite: 8]
void drawBirds_obj106() {
    for (int i = 0; i < numBirds; ++i) {
        float wing = sinf(birds[i].wingAngle) * 6.0f;
        float dim = lerp(0.20f, 0.08f, nightFactor);

        glPushMatrix();
        glTranslatef(birds[i].x, birds[i].y, 0.0f);
        glScalef(birds[i].dir, 1.0f, 1.0f);

        glLineWidth(2.5f);
        glColor3f(dim, dim, dim);
        glBegin(GL_LINE_STRIP);
        glVertex2f(-10.0f, wing);
        glVertex2f(0.0f, 0.0f);
        glVertex2f(10.0f, wing);
        glEnd();

        glPopMatrix();
    }
}

// [Obj-107]: Distant Hills
void drawMountains_obj107() {
    glBegin(GL_POLYGON);
    glColor3f(lerp(0.40f, 0.05f, nightFactor), lerp(0.60f, 0.08f, nightFactor), lerp(0.55f, 0.14f, nightFactor));
    glVertex2f(-2000.0f, 100.0f); glVertex2f(-2000.0f, 280.0f);
    glVertex2f(    0.0f, 280.0f); glVertex2f(  100.0f, 200.0f);
    glVertex2f(  240.0f, 320.0f); glVertex2f(  690.0f, 190.0f);
    glVertex2f( 1140.0f, 300.0f); glVertex2f( 1280.0f, 220.0f);
    glVertex2f( 3500.0f, 220.0f); glVertex2f( 3500.0f, 100.0f);
    glEnd();
}

// [Obj-108]: Background Roller Coaster (Using DDA Lines for Pillars)[cite: 10]
void drawRollerCoaster_obj108() {
    float rideDim = lerp(1.0f, 0.30f, nightFactor);

    glLineWidth(3.5f);
    glColor3f(0.85f * rideDim, 0.25f * rideDim, 0.22f * rideDim);
    glBegin(GL_LINE_STRIP);
    glVertex2f(20.0f, 100.0f);  glVertex2f(120.0f, 340.0f);
    glVertex2f(190.0f, 190.0f); glVertex2f(260.0f, 290.0f);
    glVertex2f(320.0f, 150.0f); glVertex2f(380.0f, 300.0f);
    glVertex2f(450.0f, 100.0f);
    glEnd();

    glColor3f(0.70f * rideDim, 0.70f * rideDim, 0.75f * rideDim);
    drawLineDDA_obj124(120.0f, 340.0f, 120.0f, 100.0f);
    drawLineDDA_obj124(260.0f, 290.0f, 260.0f, 100.0f);
    drawLineDDA_obj124(380.0f, 300.0f, 380.0f, 100.0f);
}

// [Obj-109]: Rotating Ferris Wheel
void drawFerrisWheel_obj109() {
    float rideDim = lerp(1.0f, 0.30f, nightFactor);
    float fx = 1100.0f, fy = 320.0f, fr = 110.0f;

    // Static Support Legs
    glLineWidth(3.5f);
    glColor3f(0.35f * rideDim, 0.45f * rideDim, 0.65f * rideDim);
    glBegin(GL_LINES);
    glVertex2f(fx, fy); glVertex2f(fx - 70.0f, 100.0f);
    glVertex2f(fx, fy); glVertex2f(fx + 70.0f, 100.0f);
    glEnd();

    // Wheel Rings
    drawCircleOutline(fx, fy, fr, 36, 2.5f, 0.85f * rideDim, 0.65f * rideDim, 0.20f * rideDim);
    drawCircleOutline(fx, fy, fr * 0.65f, 28, 1.5f, 0.85f * rideDim, 0.65f * rideDim, 0.20f * rideDim);

    glPushMatrix();
    glTranslatef(fx, fy, 0.0f);
    glRotatef(ferrisWheelAngle * (180.0f / M_PI), 0.0f, 0.0f, 1.0f);

    glLineWidth(1.5f);
    glColor3f(0.75f * rideDim, 0.75f * rideDim, 0.80f * rideDim);
    glBegin(GL_LINES);
    for (int i = 0; i < 8; ++i) {
        float angle = (float)i * (float)M_PI / 4.0f;
        glVertex2f(0.0f, 0.0f);
        glVertex2f(fr * cosf(angle), fr * sinf(angle));
    }
    glEnd();

    for (int i = 0; i < 8; ++i) {
        float angle = (float)i * (float)M_PI / 4.0f;
        float gx = fr * cosf(angle);
        float gy = fr * sinf(angle);

        float cabR = (i % 2 == 0) ? 0.90f : 0.20f;
        float cabG = (i % 3 == 0) ? 0.80f : 0.60f;
        float cabB = (i % 2 != 0) ? 0.90f : 0.20f;
        drawCircle(gx, gy, 7.5f, 10, cabR * rideDim, cabG * rideDim, cabB * rideDim);
    }
    glPopMatrix();

    drawCircle(fx, fy, 10.0f, 14, 0.90f * rideDim, 0.75f * rideDim, 0.20f * rideDim);
}

// [Obj-110]: Circus Big Top Tents
void drawCircusTents_obj110() {
    float rideDim = lerp(1.0f, 0.30f, nightFactor);
    glBegin(GL_TRIANGLES);
    for (int i = -3; i < 3; ++i) {
        if (i % 2 == 0) glColor3f(0.88f * rideDim, 0.22f * rideDim, 0.20f * rideDim);
        else            glColor3f(0.95f * rideDim, 0.92f * rideDim, 0.85f * rideDim);
        glVertex2f(360.0f, 230.0f);
        glVertex2f(360.0f + i * 15.0f, 100.0f);
        glVertex2f(360.0f + (i + 1) * 15.0f, 100.0f);
    }
    glEnd();
}

// [Obj-111]: Park Trees
void drawTrees_obj111() {
    float positions[] = { 80.0f, 220.0f, 780.0f, 980.0f };
    float scales[]    = { 1.2f, 1.0f, 0.9f, 1.1f };
    float trunkDim = lerp(1.0f, 0.35f, nightFactor);
    float leafDim  = lerp(1.0f, 0.25f, nightFactor);

    for (int i = 0; i < 4; ++i) {
        float x = positions[i], s = scales[i];
        glColor3f(0.42f * trunkDim, 0.24f * trunkDim, 0.12f * trunkDim);
        glBegin(GL_QUADS);
        glVertex2f(x - 4.0f * s, 100.0f);
        glVertex2f(x + 4.0f * s, 100.0f);
        glVertex2f(x + 3.0f * s, 100.0f + 25.0f * s);
        glVertex2f(x - 3.0f * s, 100.0f + 25.0f * s);
        glEnd();

        drawCircle(x, 100.0f + 35.0f * s, 18.0f * s, 16, 0.18f * leafDim, 0.52f * leafDim, 0.18f * leafDim);
        drawCircle(x - 10.0f * s, 100.0f + 30.0f * s, 14.0f * s, 14, 0.14f * leafDim, 0.44f * leafDim, 0.15f * leafDim);
        drawCircle(x + 10.0f * s, 100.0f + 30.0f * s, 14.0f * s, 14, 0.14f * leafDim, 0.44f * leafDim, 0.15f * leafDim);
    }
}

// [Obj-112]: Lamp Posts
void drawLampPosts_obj112() {
    float lamps[] = { 320.0f, 490.0f, 790.0f, 970.0f };
    float postDim = lerp(1.0f, 0.35f, nightFactor);

    for (int i = 0; i < 4; ++i) {
        float x = lamps[i];
        glLineWidth(3.0f);
        glColor3f(0.20f * postDim, 0.22f * postDim, 0.25f * postDim);
        glBegin(GL_LINES);
        glVertex2f(x, 90.0f);
        glVertex2f(x, 135.0f);
        glEnd();

        float bulbR = lerp(0.85f, 1.0f, nightFactor);
        float bulbG = lerp(0.85f, 0.92f, nightFactor);
        float bulbB = lerp(0.70f, 0.30f, nightFactor);
        drawCircle(x, 138.0f, 3.5f, 10, bulbR, bulbG, bulbB);

        if (nightFactor > 0.1f) {
            drawCircle(x, 138.0f, 20.0f, 16, 1.0f, 0.88f, 0.30f, 0.25f * nightFactor);
        }
    }
}

// [Obj-113]: Ground & Promenade Walkway
void drawGround_obj113() {
    glBegin(GL_QUADS);
    glColor3f(lerp(0.24f, 0.08f, nightFactor), lerp(0.58f, 0.18f, nightFactor), lerp(0.26f, 0.10f, nightFactor));
    glVertex2f(-2000.0f, 100.0f); glVertex2f(3500.0f, 100.0f);
    glColor3f(lerp(0.14f, 0.04f, nightFactor), lerp(0.38f, 0.10f, nightFactor), lerp(0.15f, 0.05f, nightFactor));
    glVertex2f( 3500.0f, -500.0f); glVertex2f(-2000.0f, -500.0f);

    glColor3f(lerp(0.68f, 0.20f, nightFactor), lerp(0.65f, 0.20f, nightFactor), lerp(0.60f, 0.22f, nightFactor));
    glVertex2f(-2000.0f, 72.0f); glVertex2f(3500.0f, 72.0f);
    glColor3f(lerp(0.55f, 0.14f, nightFactor), lerp(0.52f, 0.14f, nightFactor), lerp(0.48f, 0.16f, nightFactor));
    glVertex2f( 3500.0f, 92.0f); glVertex2f(-2000.0f, 92.0f);
    glEnd();
}

// [Obj-114]: Walking Humans (Using 2D Mirror Reflection Matrix)[cite: 8]
void drawHumans_obj114() {
    for (int i = 0; i < numHumans; ++i) {
        float dim = lerp(1.0f, 0.45f, nightFactor);
        float swing = sinf(humans[i].legAngle);

        glPushMatrix();
        glTranslatef(humans[i].x, humans[i].y, 0.0f);
        glScalef(humans[i].dir, 1.0f, 1.0f);

        // 1. Back Arm
        glLineWidth(3.0f);
        glColor3f(humans[i].r * 0.65f * dim, humans[i].g * 0.65f * dim, humans[i].b * 0.65f * dim);
        glBegin(GL_LINES);
        glVertex2f(2.0f, 23.0f);
        glVertex2f(2.0f + swing * 7.5f, 13.0f);
        glEnd();
        drawCircle(2.0f + swing * 7.5f, 12.5f, 1.6f, 8, 0.82f * dim, 0.66f * dim, 0.52f * dim);

        // 2. Back Leg
        glLineWidth(4.0f);
        glColor3f(0.16f * dim, 0.19f * dim, 0.26f * dim);
        glBegin(GL_LINES);
        glVertex2f(3.0f, 14.0f);
        glVertex2f(3.0f - swing * 8.0f, 0.0f);
        glEnd();

        // 3. Torso
        glColor3f(humans[i].r * dim, humans[i].g * dim, humans[i].b * dim);
        glBegin(GL_QUADS);
        glVertex2f(-5.5f, 14.0f); glVertex2f(5.5f, 14.0f);
        glVertex2f(6.0f, 28.0f);  glVertex2f(-6.0f, 28.0f);
        glEnd();

        // 4. Front Leg
        glLineWidth(4.0f);
        glColor3f(0.22f * dim, 0.26f * dim, 0.35f * dim);
        glBegin(GL_LINES);
        glVertex2f(-3.0f, 14.0f);
        glVertex2f(-3.0f + swing * 8.0f, 0.0f);
        glEnd();

        // 5. Head
        drawCircle(0.0f, 34.0f, 6.0f, 16, 0.92f * dim, 0.76f * dim, 0.62f * dim);

        // 6. Front Arm
        glLineWidth(3.0f);
        glColor3f(humans[i].r * 0.85f * dim, humans[i].g * 0.85f * dim, humans[i].b * 0.85f * dim);
        glBegin(GL_LINES);
        glVertex2f(-2.0f, 23.0f);
        glVertex2f(-2.0f - swing * 7.5f, 13.0f);
        glEnd();
        drawCircle(-2.0f - swing * 7.5f, 12.5f, 1.8f, 8, 0.92f * dim, 0.76f * dim, 0.62f * dim);

        glPopMatrix();
    }
}

// [Obj-115]: Loading Platform & Railings
void drawPlatform_obj115() {
    float platR = lerp(0.65f, 0.24f, nightFactor);
    float platG = lerp(0.65f, 0.25f, nightFactor);
    float platB = lerp(0.68f, 0.28f, nightFactor);

    glBegin(GL_QUADS);
    glColor3f(platR, platG, platB);
    glVertex2f(450.0f, 95.0f);  glVertex2f(830.0f, 95.0f);
    glColor3f(platR * 0.6f, platG * 0.6f, platB * 0.6f);
    glVertex2f(850.0f, 72.0f);  glVertex2f(430.0f, 72.0f);
    glEnd();

    glColor3f(lerp(0.85f, 0.55f, nightFactor), lerp(0.72f, 0.42f, nightFactor), lerp(0.15f, 0.10f, nightFactor));
    glLineWidth(2.5f);
    glBegin(GL_LINES);
    glVertex2f(455.0f, 115.0f); glVertex2f(825.0f, 115.0f);
    glVertex2f(455.0f, 106.0f); glVertex2f(825.0f, 106.0f);
    for (float rx = 460.0f; rx <= 820.0f; rx += 30.0f) {
        glVertex2f(rx, 95.0f);  glVertex2f(rx, 117.0f);
    }
    glEnd();
}

// [Obj-116]: Operator Control Cabin (Using 2D Shearing Matrix)[cite: 8]
void drawCabin_obj116() {
    // Main Body
    glBegin(GL_QUADS);
    glColor3f(lerp(0.58f, 0.32f, nightFactor), lerp(0.32f, 0.16f, nightFactor), lerp(0.18f, 0.08f, nightFactor));
    glVertex2f(860.0f, 85.0f);  glVertex2f(930.0f, 85.0f);
    glVertex2f(930.0f, 150.0f); glVertex2f(860.0f, 150.0f);

    glColor3f(lerp(0.85f, 0.48f, nightFactor), lerp(0.22f, 0.10f, nightFactor), lerp(0.18f, 0.08f, nightFactor));
    glVertex2f(850.0f, 150.0f); glVertex2f(940.0f, 150.0f);
    glVertex2f(920.0f, 170.0f); glVertex2f(870.0f, 170.0f);
    glEnd();

    // Cabin Window - Using Shearing Transformation to make it a slanted parallelogram[cite: 8]
    glPushMatrix();
    glTranslatef(872.0f, 122.0f, 0.0f);
    applyShearX_obj124(0.25f);

    glColor3f(lerp(0.68f, 1.0f, nightFactor), lerp(0.88f, 0.90f, nightFactor), lerp(0.98f, 0.35f, nightFactor));
    glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);   glVertex2f(46.0f, 0.0f);
    glVertex2f(46.0f, 20.0f); glVertex2f(0.0f, 20.0f);
    glEnd();
    glPopMatrix();
}

// [Obj-117]: Static A-Frame Support Structure
void drawAFrameTower_obj117() {
    float colR = lerp(0.82f, 0.42f, nightFactor);
    float colG = lerp(0.60f, 0.28f, nightFactor);
    float colB = lerp(0.18f, 0.08f, nightFactor);

    glBegin(GL_QUADS);
    glColor3f(colR, colG, colB);
    glVertex2f(365.0f, 85.0f);  glVertex2f(395.0f, 85.0f);
    glVertex2f(620.0f, 540.0f); glVertex2f(602.0f, 540.0f);
    glColor3f(colR * 0.65f, colG * 0.65f, colB * 0.65f);
    glVertex2f(380.0f, 85.0f);  glVertex2f(395.0f, 85.0f);
    glVertex2f(620.0f, 540.0f); glVertex2f(612.0f, 540.0f);

    glColor3f(colR, colG, colB);
    glVertex2f(885.0f, 85.0f);  glVertex2f(915.0f, 85.0f);
    glVertex2f(678.0f, 540.0f); glVertex2f(660.0f, 540.0f);
    glColor3f(colR * 0.65f, colG * 0.65f, colB * 0.65f);
    glVertex2f(885.0f, 85.0f);  glVertex2f(900.0f, 85.0f);
    glVertex2f(668.0f, 540.0f); glVertex2f(660.0f, 540.0f);
    glEnd();

    glLineWidth(6.0f);
    glColor3f(lerp(0.72f, 0.42f, nightFactor), lerp(0.20f, 0.08f, nightFactor), lerp(0.15f, 0.06f, nightFactor));
    glBegin(GL_LINES);
    glVertex2f(450.0f, 220.0f); glVertex2f(830.0f, 220.0f);
    glVertex2f(510.0f, 330.0f); glVertex2f(770.0f, 330.0f);
    glVertex2f(565.0f, 435.0f); glVertex2f(715.0f, 435.0f);
    glEnd();

    glLineWidth(3.0f);
    glColor3f(colR * 0.75f, colG * 0.75f, colB * 0.75f);
    glBegin(GL_LINES);
    glVertex2f(450.0f, 220.0f); glVertex2f(770.0f, 330.0f);
    glVertex2f(830.0f, 220.0f); glVertex2f(510.0f, 330.0f);
    glVertex2f(510.0f, 330.0f); glVertex2f(715.0f, 435.0f);
    glVertex2f(770.0f, 330.0f); glVertex2f(565.0f, 435.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(lerp(0.35f, 0.20f, nightFactor), lerp(0.35f, 0.20f, nightFactor), lerp(0.37f, 0.22f, nightFactor));
    glVertex2f(350.0f, 80.0f);  glVertex2f(405.0f, 80.0f);
    glVertex2f(400.0f, 105.0f); glVertex2f(355.0f, 105.0f);
    glVertex2f(875.0f, 80.0f);  glVertex2f(930.0f, 80.0f);
    glVertex2f(925.0f, 105.0f); glVertex2f(870.0f, 105.0f);
    glEnd();

    if (nightFactor > 0.01f) {
        for (int i = 0; i <= 14; ++i) {
            float f = (float)i / 14.0f;
            float leftX = 380.0f + (611.0f - 380.0f) * f;
            float rightX = 900.0f + (669.0f - 900.0f) * f;
            float y = 85.0f + (540.0f - 85.0f) * f;
            float lr = 0.5f + 0.5f * sinf(timeStep * 4.0f + i * 0.4f);
            float lg = 0.5f + 0.5f * sinf(timeStep * 4.0f + i * 0.4f + 2.0f);
            float lb = 0.5f + 0.5f * sinf(timeStep * 4.0f + i * 0.4f + 4.0f);
            drawCircle(leftX, y, 3.5f, 8, lr, lg, lb, nightFactor);
            drawCircle(rightX, y, 3.5f, 8, lr, lg, lb, nightFactor);
        }
    }
}

// [Obj-118]: Pivot Axle & Wheel Hub
void drawPivotHub_obj118() {
    drawCircle(640.0f, 540.0f, 42.0f, 32, lerp(0.48f, 0.22f, nightFactor), lerp(0.26f, 0.10f, nightFactor), lerp(0.12f, 0.05f, nightFactor));
    drawCircleOutline(640.0f, 540.0f, 38.0f, 32, 5.0f, lerp(0.88f, 0.50f, nightFactor), lerp(0.72f, 0.38f, nightFactor), lerp(0.20f, 0.10f, nightFactor));
    drawCircle(640.0f, 540.0f, 16.0f, 20, lerp(0.25f, 0.15f, nightFactor), lerp(0.25f, 0.15f, nightFactor), lerp(0.28f, 0.18f, nightFactor));
    drawCircle(640.0f, 540.0f, 7.0f, 14, lerp(0.90f, 0.70f, nightFactor), lerp(0.85f, 0.65f, nightFactor), lerp(0.30f, 0.20f, nightFactor));

    glLineWidth(4.0f);
    glColor3f(lerp(0.88f, 0.50f, nightFactor), lerp(0.72f, 0.38f, nightFactor), lerp(0.20f, 0.10f, nightFactor));
    glBegin(GL_LINES);
    for (int i = 0; i < 8; ++i) {
        float angle = (float)i * (float)M_PI / 4.0f;
        glVertex2f(640.0f, 540.0f);
        glVertex2f(640.0f + 48.0f * cosf(angle), 540.0f + 48.0f * sinf(angle));
    }
    glEnd();
}

// [Obj-119]: Suspension Trusses (Connecting Pivot to Lowered Hull)
void drawSuspensionStruts_obj119(Point topP0, Point topP1, Point topP2) {
    Point attachL_outer = getBezierPoint(topP0, topP1, topP2, 0.18f);
    Point attachR_outer = getBezierPoint(topP0, topP1, topP2, 0.82f);
    Point attachL_inner = getBezierPoint(topP0, topP1, topP2, 0.37f);
    Point attachR_inner = getBezierPoint(topP0, topP1, topP2, 0.63f);

    glLineWidth(5.0f);
    glColor3f(lerp(0.78f, 0.38f, nightFactor), lerp(0.22f, 0.08f, nightFactor), lerp(0.18f, 0.06f, nightFactor));
    glBegin(GL_LINES);
    glVertex2f(-12.0f, -5.0f); glVertex2f(attachL_outer.x, attachL_outer.y);
    glVertex2f(12.0f, -5.0f);  glVertex2f(attachR_outer.x, attachR_outer.y);
    glVertex2f(-5.0f, -5.0f);  glVertex2f(attachL_inner.x, attachL_inner.y);
    glVertex2f(5.0f, -5.0f);   glVertex2f(attachR_inner.x, attachR_inner.y);
    glEnd();

    glLineWidth(2.5f);
    glColor3f(lerp(0.90f, 0.50f, nightFactor), lerp(0.75f, 0.40f, nightFactor), lerp(0.25f, 0.10f, nightFactor));
    glBegin(GL_LINES);
    for (float f = 0.12f; f <= 0.90f; f += 0.11f) {
        glVertex2f(-12.0f + (attachL_outer.x + 12.0f) * f, -5.0f + (attachL_outer.y + 5.0f) * f);
        glVertex2f(-5.0f  + (attachL_inner.x + 5.0f)  * f, -5.0f + (attachL_inner.y + 5.0f) * f);
        glVertex2f(12.0f  + (attachR_outer.x - 12.0f) * f, -5.0f + (attachR_outer.y + 5.0f) * f);
        glVertex2f(5.0f   + (attachR_inner.x - 5.0f)  * f, -5.0f + (attachR_inner.y + 5.0f) * f);
    }
    glEnd();
}

// [Obj-120]: Ship Mast & Crow's Nest (Lowered with Boat)
void drawMastAndSail_obj120(Point mastBase) {
    glLineWidth(6.0f);
    glColor3f(lerp(0.38f, 0.18f, nightFactor), lerp(0.20f, 0.08f, nightFactor), lerp(0.10f, 0.04f, nightFactor));
    glBegin(GL_LINES);
    glVertex2f(mastBase.x, mastBase.y);
    glVertex2f(0.0f, -195.0f);
    glVertex2f(-55.0f, -245.0f);
    glVertex2f(55.0f, -245.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(lerp(0.45f, 0.22f, nightFactor), lerp(0.25f, 0.10f, nightFactor), lerp(0.12f, 0.06f, nightFactor));
    glVertex2f(-14.0f, -220.0f);
    glVertex2f(14.0f, -220.0f);
    glVertex2f(10.0f, -232.0f);
    glVertex2f(-10.0f, -232.0f);
    glEnd();
}

// [Obj-121]: Parametric Bézier Ship Hull[cite: 6]
void drawShipHull_obj121(Point topP0, Point topP1, Point topP2, Point botP0, Point botP1, Point botP2) {
    const int segments = 36;
    float hullR = lerp(0.42f, 0.20f, nightFactor);
    float hullG = lerp(0.22f, 0.10f, nightFactor);
    float hullB = lerp(0.10f, 0.05f, nightFactor);

    glBegin(GL_QUAD_STRIP);
    glColor3f(hullR, hullG, hullB);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        Point topPt = getBezierPoint(topP0, topP1, topP2, t);
        Point botPt = getBezierPoint(botP0, botP1, botP2, t);
        glVertex2f(topPt.x, topPt.y);
        glVertex2f(botPt.x, botPt.y);
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    glColor3f(lerp(0.75f, 0.45f, nightFactor), lerp(0.18f, 0.08f, nightFactor), lerp(0.16f, 0.08f, nightFactor));
    for (int i = 2; i <= segments - 2; ++i) {
        float t = (float)i / (float)segments;
        Point topPt = getBezierPoint(topP0, topP1, topP2, t);
        Point botPt = getBezierPoint(botP0, botP1, botP2, t);
        glVertex2f(topPt.x, botPt.y + 0.35f * (topPt.y - botPt.y));
        glVertex2f(topPt.x, botPt.y + 0.55f * (topPt.y - botPt.y));
    }
    glEnd();

    glLineWidth(3.5f);
    glColor3f(lerp(0.92f, 0.65f, nightFactor), lerp(0.78f, 0.50f, nightFactor), lerp(0.24f, 0.12f, nightFactor));
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        Point pt = getBezierPoint(topP0, topP1, topP2, t);
        glVertex2f(pt.x, pt.y);
    }
    glEnd();

    glLineWidth(2.5f);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        Point pt = getBezierPoint(botP0, botP1, botP2, t);
        glVertex2f(pt.x, pt.y);
    }
    glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(lerp(0.92f, 0.65f, nightFactor), lerp(0.78f, 0.50f, nightFactor), lerp(0.24f, 0.12f, nightFactor));
    glVertex2f(topP0.x, topP0.y);
    glVertex2f(-205.0f, -295.0f);
    glVertex2f(botP0.x, botP0.y);

    glVertex2f(-205.0f, -295.0f);
    glVertex2f(-212.0f, -275.0f);
    glVertex2f(-190.0f, -290.0f);
    glEnd();

    glBegin(GL_QUADS);
    glColor3f(hullR * 0.8f, hullG * 0.8f, hullB * 0.8f);
    glVertex2f(145.0f, getBezierPoint(topP0, topP1, topP2, 0.91f).y);
    glVertex2f(topP2.x, topP2.y);
    glVertex2f(165.0f, -285.0f);
    glVertex2f(130.0f, -330.0f);

    glColor3f(lerp(0.95f, 1.0f, nightFactor), lerp(0.85f, 0.90f, nightFactor), lerp(0.35f, 0.35f, nightFactor));
    glVertex2f(148.0f, -322.0f);
    glVertex2f(165.0f, -300.0f);
    glVertex2f(160.0f, -292.0f);
    glVertex2f(143.0f, -314.0f);
    glEnd();
}

// [Obj-122]: Tiered Passenger Benches & Viking Shields[cite: 6]
void drawBenchesAndShields_obj122(Point topP0, Point topP1, Point topP2) {
    for (int i = 0; i < 9; ++i) {
        float t = 0.16f + (float)i * 0.085f;
        Point seatPt = getBezierPoint(topP0, topP1, topP2, t);

        glBegin(GL_QUADS);
        glColor3f(lerp(0.20f, 0.12f, nightFactor), lerp(0.20f, 0.12f, nightFactor), lerp(0.22f, 0.14f, nightFactor));
        glVertex2f(seatPt.x - 8.0f, seatPt.y - 14.0f);
        glVertex2f(seatPt.x + 8.0f, seatPt.y - 14.0f);
        glVertex2f(seatPt.x + 8.0f, seatPt.y + 2.0f);
        glVertex2f(seatPt.x - 8.0f, seatPt.y + 2.0f);
        glEnd();

        glLineWidth(2.5f);
        glColor3f(lerp(0.85f, 0.50f, nightFactor), lerp(0.85f, 0.50f, nightFactor), lerp(0.90f, 0.55f, nightFactor));
        glBegin(GL_LINES);
        glVertex2f(seatPt.x - 10.0f, seatPt.y - 6.0f);
        glVertex2f(seatPt.x + 10.0f, seatPt.y - 6.0f);
        glEnd();
    }

    for (int i = 0; i < 8; ++i) {
        float t = 0.12f + (float)i * 0.11f;
        Point shieldPt = getBezierPoint(topP0, topP1, topP2, t);

        float r1 = 0.2f, g1 = 0.4f, b1 = 0.7f;
        float r2 = 0.8f, g2 = 0.8f, b2 = 0.8f;
        if (i % 3 == 1) { r1 = 0.75f; g1 = 0.2f; b1 = 0.2f; r2 = 0.85f; g2 = 0.75f; b2 = 0.2f; }
        else if (i % 3 == 2) { r1 = 0.2f; g1 = 0.6f; b1 = 0.3f; r2 = 0.9f; g2 = 0.9f; b2 = 0.9f; }

        float dim = lerp(1.0f, 0.45f, nightFactor);
        drawCircle(shieldPt.x, shieldPt.y - 10.0f, 10.0f, 20, r1 * dim, g1 * dim, b1 * dim);
        drawCircle(shieldPt.x, shieldPt.y - 10.0f, 7.0f, 16, r2 * dim, g2 * dim, b2 * dim);
        drawCircle(shieldPt.x, shieldPt.y - 10.0f, 2.8f, 12, 0.85f * dim, 0.75f * dim, 0.2f * dim);
    }
}

// [Obj-123]: Dynamic Night LEDs along Boat Keel
void drawBoatKeelLEDs_obj123(Point botP0, Point botP1, Point botP2) {
    if (nightFactor > 0.01f) {
        for (int i = 0; i <= 8; ++i) {
            float t = (float)i / 8.0f;
            Point ledPt = getBezierPoint(botP0, botP1, botP2, t);

            float r = 0.5f + 0.5f * sinf(timeStep * 5.0f + i * 0.8f);
            float g = 0.5f + 0.5f * sinf(timeStep * 5.0f + i * 0.8f + 2.0f);
            float b = 0.5f + 0.5f * sinf(timeStep * 5.0f + i * 0.8f + 4.0f);
            drawCircle(ledPt.x, ledPt.y, 4.0f, 8, r, g, b, nightFactor);
        }
    }
}

// [Obj-125]: HIERARCHICAL SHIP ASSEMBLY
void drawPirateShipAssembly_obj125() {
    glPushMatrix();
    glTranslatef(640.0f, 540.0f, 0.0f);
    glRotatef(swingAngle, 0.0f, 0.0f, 1.0f);

    Point topP0 = { -175.0f, -315.0f };
    Point topP1 = {    0.0f, -385.0f };
    Point topP2 = {  175.0f, -315.0f };

    Point botP0 = { -175.0f, -365.0f };
    Point botP1 = {    0.0f, -475.0f };
    Point botP2 = {  175.0f, -365.0f };

    drawSuspensionStruts_obj119(topP0, topP1, topP2);
    drawMastAndSail_obj120(getBezierPoint(topP0, topP1, topP2, 0.5f));
    drawShipHull_obj121(topP0, topP1, topP2, botP0, botP1, botP2);
    drawBenchesAndShields_obj122(topP0, topP1, topP2);
    drawBoatKeelLEDs_obj123(botP0, botP1, botP2);

    glPopMatrix();
}

// =========================================================
// DISPLAY CALLBACK
// =========================================================

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glPushMatrix();

    // Render Environment & Still Objects
    drawSky_obj101();
    drawStars_obj102();
    drawSun_obj103();
    drawMoon_obj104();
    drawClouds_obj105();
    drawBirds_obj106();
    drawMountains_obj107();
    drawRollerCoaster_obj108();
    drawFerrisWheel_obj109();
    drawCircusTents_obj110();
    drawTrees_obj111();
    drawLampPosts_obj112();
    drawGround_obj113();
    drawHumans_obj114();
    drawPlatform_obj115();
    drawCabin_obj116();

    // Render Dynamic Rides
    drawPirateShipAssembly_obj125();
    drawAFrameTower_obj117();
    drawPivotHub_obj118();

    glPopMatrix();

    glutSwapBuffers();
}

// =========================================================
// ANIMATION TIMER CALLBACK[cite: 1]
// =========================================================

void timer(int value) {
    if (!isPaused) {
        float dt = 0.016f;
        timeStep += 0.025f;

        // [A-101]: Pendulum Simple Harmonic Motion
        swingAngle = maxSwingAngle * sinf(swingSpeed * timeStep);

        // [A-102]: Ferris Wheel Rotation Animation
        ferrisWheelAngle += 0.012f;
        if (ferrisWheelAngle >= 2.0f * (float)M_PI) {
            ferrisWheelAngle -= 2.0f * (float)M_PI;
        }

        // [A-103]: Day / Night Cycle Timer & Interpolation Factor
        cycleTimer += dt;
        if (cycleTimer >= totalCycleTime) {
            cycleTimer -= totalCycleTime;
        }

        if (cycleTimer < 5.0f) {
            nightFactor = 0.0f;
        } else if (cycleTimer < 6.0f) {
            nightFactor = cycleTimer - 5.0f;
        } else if (cycleTimer < 11.0f) {
            nightFactor = 1.0f;
        } else {
            nightFactor = 1.0f - (cycleTimer - 11.0f);
        }

        // [A-104]: Sun & Moon Arc Trajectories
        if (cycleTimer <= 6.0f) {
            float sunPhase = cycleTimer / 6.0f;
            float sunAngle = (1.0f - sunPhase) * (float)M_PI;
            sunX = 640.0f - 450.0f * cosf(sunAngle);
            sunY = 220.0f + 450.0f * sinf(sunAngle);
        } else {
            sunY = -100.0f;
        }

        if (cycleTimer > 6.0f) {
            float moonPhase = (cycleTimer - 6.0f) / 6.0f;
            float moonAngle = (1.0f - moonPhase) * (float)M_PI;
            moonX = 640.0f - 450.0f * cosf(moonAngle);
            moonY = 220.0f + 450.0f * sinf(moonAngle);
        } else {
            moonY = -100.0f;
        }

        // [A-106]: Cloud Drift Movement
        for (int i = 0; i < numClouds; ++i) {
            clouds[i].x += clouds[i].speed;
            if (clouds[i].x > 1450.0f) clouds[i].x = -150.0f;
        }

        // [A-107]: Bird Flight & Wing Flap
        for (int i = 0; i < numBirds; ++i) {
            birds[i].x += birds[i].speed;
            birds[i].wingAngle += 0.18f;
            if (birds[i].x > 1450.0f) birds[i].x = -150.0f;
        }

        // [A-108]: Walking Human Locomotion
        for (int i = 0; i < numHumans; ++i) {
            humans[i].x += humans[i].dir * humans[i].speed;
            humans[i].legAngle += 0.08f;

            if (humans[i].x > 1360.0f)     humans[i].dir = -1.0f;
            else if (humans[i].x < -80.0f) humans[i].dir =  1.0f;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// =========================================================
// INTERACTION CALLBACKS (MOUSE, RESHAPE, INIT, MAIN)
// =========================================================

// Mouse Interaction Handler: Left click to toggle pause/play[cite: 1]
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        isPaused = !isPaused;
    }
}

// Aspect-Ratio Preserving Reshape Callback[cite: 8]
void reshape(int width, int height) {
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const float baseAspect = 1280.0f / 720.0f;
    float windowAspect = (float)width / (float)height;

    if (windowAspect >= baseAspect) {
        float extraWidth = (720.0f * windowAspect - 1280.0f) / 2.0f;
        gluOrtho2D(-extraWidth, 1280.0f + extraWidth, 0.0, 720.0);
    } else {
        float extraHeight = (1280.0f / windowAspect - 720.0f) / 2.0f;
        gluOrtho2D(0.0, 1280.0, -extraHeight, 720.0f + extraHeight);
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Initialization
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    for (int i = 0; i < numStars; ++i) {
        starX[i] = -300.0f + (float)(rand() % 1900);
        starY[i] = 340.0f + (float)(rand() % 360);
    }

    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

} // namespace Scene1
#undef PI   // main.cpp's own "#define PI" must not leak into Scene4's "const float PI"

// ============================================================================
// SCENE 2 -- original main2.cpp (verbatim, unmodified)
// ============================================================================
namespace Scene2 {


float wheelAngle = 0.0f;
float cloudMove = 0.0f;

void init()
{
    glClearColor(0.529f, 0.808f, 0.922f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1280, 0, 720, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawSky201(){
    glColor3f(0.529f, 0.808f, 0.922f);
    glBegin(GL_POLYGON); // Obj-201(Sky Background)
    glVertex2f(0, 0);
    glVertex2f(1280, 0);
    glVertex2f(1280, 720);
    glVertex2f(0, 720);
    glEnd();
}

void drawGround202(){
    glColor3f(0.180f, 0.545f, 0.216f);
    glBegin(GL_POLYGON); //Obj-002 (Grass Ground)
    glVertex2f(0, 0);
    glVertex2f(1280, 0);
    glVertex2f(1280, 200);
    glVertex2f(0, 200);
    glEnd();
}

void drawClouds203(){
    glColor3f(0.902f, 0.957f, 0.988f);

    glBegin(GL_POLYGON); // Obj-203 (Cloud 1)
    glVertex2f(50 + cloudMove, 550);
    glVertex2f(131 + cloudMove, 550);
    glVertex2f(150 + cloudMove, 564);
    glVertex2f(170 + cloudMove, 570);
    glVertex2f(161 + cloudMove, 596);
    glVertex2f(137 + cloudMove, 613);
    glVertex2f(60 + cloudMove, 613);
    glVertex2f(21 + cloudMove, 591);
    glVertex2f(34 + cloudMove, 573);
    glVertex2f(16 + cloudMove, 561);
    glVertex2f(32 + cloudMove, 560);
    glEnd();

    glColor3f(0.902f, 0.957f, 0.988f);

    glBegin(GL_POLYGON); // Obj-204 (Cloud 2)
    glVertex2f(170 + cloudMove, 629);
    glVertex2f(250 + cloudMove, 629);
    glVertex2f(260 + cloudMove, 640);
    glVertex2f(280 + cloudMove, 640);
    glVertex2f(269 + cloudMove, 659);
    glVertex2f(279 + cloudMove, 671);
    glVertex2f(252 + cloudMove, 693);
    glVertex2f(235 + cloudMove, 706);
    glVertex2f(219 + cloudMove, 710);
    glVertex2f(190 + cloudMove, 693);
    glVertex2f(185 + cloudMove, 686);
    glVertex2f(174 + cloudMove, 682);
    glVertex2f(166 + cloudMove, 670);
    glVertex2f(156 + cloudMove, 661);
    glVertex2f(154 + cloudMove, 651);
    glVertex2f(154 + cloudMove, 637);
    glVertex2f(139 + cloudMove, 629);
    glEnd();
}

void drawTrees204(){
    // Obj-205/206 (Tree 1 - far left)
    glColor3f(0.090f, 0.506f, 0.278f);
    glBegin(GL_POLYGON);
    glVertex2f(82, 266); glVertex2f(82, 426); glVertex2f(120, 400); glVertex2f(108, 388);
    glVertex2f(131, 369); glVertex2f(120, 360); glVertex2f(140, 340); glVertex2f(120, 320);
    glVertex2f(140, 300); glVertex2f(116, 280); glVertex2f(143, 259);
    glEnd();

    glColor3f(0.055f, 0.318f, 0.247f);
    glBegin(GL_POLYGON);
    glVertex2f(82, 266); glVertex2f(22, 259); glVertex2f(49, 280); glVertex2f(25, 300);
    glVertex2f(45, 320); glVertex2f(25, 340); glVertex2f(45, 360); glVertex2f(34, 369);
    glVertex2f(56, 388); glVertex2f(45, 400); glVertex2f(82, 426);
    glEnd();

    glColor3f(0.510f, 0.271f, 0.125f);
    glBegin(GL_QUADS); //Obj-007 (Tree 1 trunk)
    glVertex2f(69, 195); glVertex2f(97, 194); glVertex2f(97, 264); glVertex2f(68, 264);
    glEnd();

    // Obj-208/209 (Tree 2 - left)
    glColor3f(0.090f, 0.506f, 0.278f);
    glBegin(GL_POLYGON);
    glVertex2f(219, 266); glVertex2f(219, 426); glVertex2f(257, 401); glVertex2f(246, 389);
    glVertex2f(268, 370); glVertex2f(257, 361); glVertex2f(277, 341); glVertex2f(257, 321);
    glVertex2f(277, 301); glVertex2f(253, 281); glVertex2f(280, 260);
    glEnd();

    glColor3f(0.055f, 0.318f, 0.247f);
    glBegin(GL_POLYGON);
    glVertex2f(219, 266); glVertex2f(159, 260); glVertex2f(186, 281); glVertex2f(162, 301);
    glVertex2f(182, 321); glVertex2f(162, 341); glVertex2f(182, 361); glVertex2f(171, 370);
    glVertex2f(193, 389); glVertex2f(182, 401); glVertex2f(219, 426);
    glEnd();

    glColor3f(0.510f, 0.271f, 0.125f);
    glBegin(GL_QUADS); //Obj-210 (Tree 2 trunk)
    glVertex2f(210, 196); glVertex2f(230, 196); glVertex2f(230, 265); glVertex2f(209, 265);
    glEnd();

    // Obj-211/212 (Tree 3 - right)
    glColor3f(0.090f, 0.506f, 0.278f);
    glBegin(GL_POLYGON);
    glVertex2f(1058, 275); glVertex2f(1058, 435); glVertex2f(1095, 410); glVertex2f(1084, 398);
    glVertex2f(1106, 378); glVertex2f(1095, 370); glVertex2f(1115, 350); glVertex2f(1095, 330);
    glVertex2f(1115, 310); glVertex2f(1091, 290); glVertex2f(1118, 269);
    glEnd();

    glColor3f(0.055f, 0.318f, 0.247f);
    glBegin(GL_POLYGON);
    glVertex2f(1058, 275); glVertex2f(997, 269); glVertex2f(1024, 290); glVertex2f(1000, 310);
    glVertex2f(1020, 330); glVertex2f(1000, 350); glVertex2f(1020, 370); glVertex2f(1009, 378);
    glVertex2f(1032, 398); glVertex2f(1020, 410); glVertex2f(1058, 435);
    glEnd();

    glColor3f(0.510f, 0.271f, 0.125f);
    glBegin(GL_QUADS); //Obj-213 (Tree 3 trunk)
    glVertex2f(1048, 196); glVertex2f(1068, 196); glVertex2f(1068, 274); glVertex2f(1048, 274);
    glEnd();

    // Obj-214/215 (Tree 4 - far right)
    glColor3f(0.090f, 0.506f, 0.278f);
    glBegin(GL_POLYGON);
    glVertex2f(1188, 274); glVertex2f(1188, 434); glVertex2f(1225, 408); glVertex2f(1214, 397);
    glVertex2f(1236, 377); glVertex2f(1225, 368); glVertex2f(1245, 348); glVertex2f(1225, 328);
    glVertex2f(1245, 308); glVertex2f(1221, 289); glVertex2f(1248, 268);
    glEnd();

    glColor3f(0.055f, 0.318f, 0.247f);
    glBegin(GL_POLYGON);
    glVertex2f(1188, 274); glVertex2f(1127, 268); glVertex2f(1154, 289); glVertex2f(1130, 308);
    glVertex2f(1150, 328); glVertex2f(1130, 348); glVertex2f(1150, 368); glVertex2f(1139, 377);
    glVertex2f(1162, 397); glVertex2f(1150, 408); glVertex2f(1188, 434);
    glEnd();

    glColor3f(0.510f, 0.271f, 0.125f);
    glBegin(GL_QUADS); //Obj-216 (Tree 4 trunk)
    glVertex2f(1180, 196); glVertex2f(1200, 196); glVertex2f(1200, 273); glVertex2f(1180, 273);
    glEnd();
}

void drawWheelSupport205(){
    glColor3f(0.800f, 0.520f, 0.180f);
    glBegin(GL_POLYGON); //Obj-217 (Left leg)
    glVertex2f(392, 215); glVertex2f(420, 215); glVertex2f(606, 474); glVertex2f(592, 485);
    glEnd();

    glColor3f(0.800f, 0.520f, 0.180f);
    glBegin(GL_POLYGON); //Obj218 (Right leg)
    glVertex2f(834, 219); glVertex2f(800, 219); glVertex2f(620, 474); glVertex2f(631, 491);
    glEnd();

    glColor3f(0.310f, 0.337f, 0.427f);
    glBegin(GL_QUADS); //Obj-219 (Left foot)
    glVertex2f(363, 196); glVertex2f(440, 196); glVertex2f(434, 215); glVertex2f(363, 215);
    glEnd();

    glColor3f(0.310f, 0.337f, 0.427f);
    glBegin(GL_QUADS); //Obj-220 (Right foot)
    glVertex2f(782, 193); glVertex2f(857, 193); glVertex2f(851, 219); glVertex2f(790, 219);
    glEnd();
}

void drawWheelSpokes206(){
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES); //Obj-221 (Wheel spokes & bracing)

    // hexagon outline
    glVertex2f(610, 320); glVertex2f(450, 402);
    glVertex2f(450, 402); glVertex2f(450, 560);
    glVertex2f(450, 560); glVertex2f(610, 641);
    glVertex2f(610, 641); glVertex2f(770, 560);
    glVertex2f(770, 560); glVertex2f(770, 400);
    glVertex2f(770, 400); glVertex2f(610, 320);

    // radial spokes to hub
    glVertex2f(610, 641); glVertex2f(611, 512);
    glVertex2f(450, 560); glVertex2f(594, 502);
    glVertex2f(450, 402); glVertex2f(592, 485);
    glVertex2f(610, 320); glVertex2f(610, 472);
    glVertex2f(770, 560); glVertex2f(629, 503);
    glVertex2f(770, 400); glVertex2f(631, 491);

    // cross bracing
    glVertex2f(536, 603); glVertex2f(601, 509);
    glVertex2f(592, 494); glVertex2f(450, 482);
    glVertex2f(684, 603); glVertex2f(622, 509);
    glVertex2f(770, 482); glVertex2f(631, 497);
    glEnd();

    glLineWidth(1.0f);

    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON); //Obj-222 (Wheel hub)
    glVertex2f(631, 492); glVertex2f(629, 500); glVertex2f(625, 506); glVertex2f(619, 510);
    glVertex2f(611, 512); glVertex2f(603, 510); glVertex2f(597, 506); glVertex2f(593, 500);
    glVertex2f(591, 492); glVertex2f(593, 484); glVertex2f(597, 478); glVertex2f(603, 474);
    glVertex2f(611, 472); glVertex2f(619, 474); glVertex2f(625, 478); glVertex2f(629, 484);
    glEnd();
}

void drawCabins207(){

    //Obj-223 (Bottom cabin)
    glColor3f(0.988f, 0.502f, 0.184f);
    glBegin(GL_QUADS);
    glVertex2f(636, 611);
    glVertex2f(636, 668);
    glVertex2f(580, 668);
    glVertex2f(580, 611);
    glEnd();

    glColor3f(0.984f, 0.941f, 0.875f);
    glBegin(GL_QUADS);
    glVertex2f(592, 620);
    glVertex2f(592, 656);
    glVertex2f(628, 656);
    glVertex2f(628, 620);
    glEnd();

    glColor3f(0.918f, 0.263f, 0.208f);
    glBegin(GL_QUADS);
    glVertex2f(580, 668);
    glVertex2f(636, 668);
    glVertex2f(647, 684);
    glVertex2f(568, 684);
    glEnd();


    //Obj-224 (Lower-right cabin)
    glColor3f(0.988f, 0.502f, 0.184f);
    glBegin(GL_QUADS);
    glVertex2f(796, 531);
    glVertex2f(796, 588);
    glVertex2f(740, 588);
    glVertex2f(740, 531);
    glEnd();

    glColor3f(0.984f, 0.941f, 0.875f);
    glBegin(GL_QUADS);
    glVertex2f(751, 541);
    glVertex2f(751, 577);
    glVertex2f(787, 577);
    glVertex2f(787, 541);
    glEnd();

    glColor3f(0.918f, 0.263f, 0.208f);
    glBegin(GL_QUADS);
    glVertex2f(741, 588);
    glVertex2f(796, 588);
    glVertex2f(807, 605);
    glVertex2f(728, 605);
    glEnd();


    //Obj-225 (Lower-left cabin)
    glColor3f(0.988f, 0.502f, 0.184f);
    glBegin(GL_QUADS);
    glVertex2f(479, 535);
    glVertex2f(479, 592);
    glVertex2f(424, 592);
    glVertex2f(424, 535);
    glEnd();

    glColor3f(0.984f, 0.941f, 0.875f);
    glBegin(GL_QUADS);
    glVertex2f(432, 543);
    glVertex2f(432, 580);
    glVertex2f(468, 580);
    glVertex2f(468, 543);
    glEnd();

    glColor3f(0.918f, 0.263f, 0.208f);
    glBegin(GL_QUADS);
    glVertex2f(424, 592);
    glVertex2f(479, 591);
    glVertex2f(491, 608);
    glVertex2f(412, 608);
    glEnd();


    //Obj-226 (Upper-right cabin)
    glColor3f(0.988f, 0.502f, 0.184f);
    glBegin(GL_QUADS);
    glVertex2f(796, 375);
    glVertex2f(796, 432);
    glVertex2f(741, 432);
    glVertex2f(741, 375);
    glEnd();

    glColor3f(0.984f, 0.941f, 0.875f);
    glBegin(GL_QUADS);
    glVertex2f(751, 384);
    glVertex2f(751, 420);
    glVertex2f(787, 420);
    glVertex2f(787, 384);
    glEnd();

    glColor3f(0.918f, 0.263f, 0.208f);
    glBegin(GL_QUADS);
    glVertex2f(741, 432);
    glVertex2f(796, 432);
    glVertex2f(808, 448);
    glVertex2f(728, 448);
    glEnd();


    //Obj-227 (Top cabin)
    glColor3f(0.988f, 0.502f, 0.184f);
    glBegin(GL_QUADS);
    glVertex2f(635, 300);
    glVertex2f(635, 357);
    glVertex2f(580, 357);
    glVertex2f(580, 300);
    glEnd();

    glColor3f(0.984f, 0.941f, 0.875f);
    glBegin(GL_QUADS);
    glVertex2f(588, 308);
    glVertex2f(588, 344);
    glVertex2f(624, 344);
    glVertex2f(624, 308);
    glEnd();

    glColor3f(0.918f, 0.263f, 0.208f);
    glBegin(GL_QUADS);
    glVertex2f(580, 357);
    glVertex2f(635, 356);
    glVertex2f(648, 372);
    glVertex2f(568, 372);
    glEnd();


    //Obj-228 (Upper-left cabin)
    glColor3f(0.988f, 0.502f, 0.184f);
    glBegin(GL_QUADS);
    glVertex2f(479, 371);
    glVertex2f(479, 428);
    glVertex2f(423, 428);
    glVertex2f(423, 371);
    glEnd();

    glColor3f(0.984f, 0.941f, 0.875f);
    glBegin(GL_QUADS);
    glVertex2f(432, 380);
    glVertex2f(432, 417);
    glVertex2f(468, 417);
    glVertex2f(468, 380);
    glEnd();

    glColor3f(0.918f, 0.263f, 0.208f);
    glBegin(GL_QUADS);
    glVertex2f(424, 428);
    glVertex2f(479, 427);
    glVertex2f(491, 444);
    glVertex2f(412, 444);
    glEnd();
}

void drawSun208(){
    glColor3f(0.976f, 0.749f, 0.373f);
    glBegin(GL_QUADS); //Obj-229 (Sun body)
    glVertex2f(1100, 550); glVertex2f(1100, 650); glVertex2f(1200, 650); glVertex2f(1200, 550);
    glEnd();

    glColor3f(0.976f, 0.647f, 0.129f);
    glBegin(GL_QUADS); //Obj-230 (Sun rays)
    glVertex2f(1140, 480); glVertex2f(1140, 540); glVertex2f(1160, 540); glVertex2f(1160, 480); //bottom
    glVertex2f(1144, 660); glVertex2f(1143, 711); glVertex2f(1160, 711); glVertex2f(1160, 660); //top
    glVertex2f(1208, 601); glVertex2f(1209, 612); glVertex2f(1260, 612); glVertex2f(1260, 600); //right
    glVertex2f(1012, 590); glVertex2f(1011, 609); glVertex2f(1079, 609); glVertex2f(1079, 591); //left
    glEnd();

    glColor3f(0.976f, 0.647f, 0.129f);
    glBegin(GL_POLYGON); //bottom-left diagonal ray
    glVertex2f(1090, 662); glVertex2f(1051, 700); glVertex2f(1040, 692); glVertex2f(1081, 655);
    glEnd();

    glBegin(GL_POLYGON); //top-right diagonal ray
    glVertex2f(1200, 660); glVertex2f(1209, 651); glVertex2f(1247, 683); glVertex2f(1237, 691);
    glEnd();

    glBegin(GL_POLYGON); //bottom-right diagonal ray
    glVertex2f(1212, 552); glVertex2f(1200, 540); glVertex2f(1245, 509); glVertex2f(1255, 527);
    glEnd();

    glBegin(GL_POLYGON); //top-left diagonal ray
    glVertex2f(1085, 549); glVertex2f(1028, 519); glVertex2f(1059, 489); glVertex2f(1097, 537);
    glEnd();
}

void drawTicketCounter209(){
    glColor3f(1.0f, 0.918f, 0.804f);
    glBegin(GL_QUADS); //Obj-231 (Booth wall)
    glVertex2f(1100, 29); glVertex2f(1100, 180); glVertex2f(1268, 180); glVertex2f(1269, 30);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS); //Obj-232 (Booth roof)
    glVertex2f(1129, 29); glVertex2f(1150, 109); glVertex2f(1225, 109); glVertex2f(1244, 30);
    glEnd();

    glColor3f(0.545f, 0.271f, 0.075f);
    glBegin(GL_QUADS); //Obj-233 (Booth door)
    glVertex2f(1172, 29); glVertex2f(1171, 81); glVertex2f(1205, 81); glVertex2f(1205, 30);
    glEnd();
}


// ROTATION PART
void update(int value)
{
    wheelAngle += 1.0f;

    if (wheelAngle >= 360.0f)
        wheelAngle -= 360.0f;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void updateClouds(int value)
{
    cloudMove += 1.5f;

    if (cloudMove > 1280.0f)
        cloudMove = -280.0f;

    glutPostRedisplay();
    glutTimerFunc(16, updateClouds, 0);
}

void display(){
    glClear(GL_COLOR_BUFFER_BIT);

    drawSky201();
    drawGround202();
    drawSun208();
    drawClouds203();

    drawTrees204();
    drawWheelSupport205();

    // ADDED FOR FERRIS WHEEL ROTATION
    glPushMatrix();

    glTranslatef(611.0f, 492.0f, 0.0f);
    glRotatef(wheelAngle, 0.0f, 0.0f, 1.0f);
    glTranslatef(-611.0f, -492.0f, 0.0f);

    drawWheelSpokes206();
    drawCabins207();

    glPopMatrix();

    drawTicketCounter209();

    glFlush();
}

} // namespace Scene2

// ============================================================================
// SCENE 3 -- original project.cpp (verbatim, unmodified)
// ============================================================================
namespace Scene3 {


GLfloat cloudX = 0.0f;
GLfloat birdX = 0.0f;
GLfloat birdY = 0.0f;
GLfloat birdScale = 1.0f;
int birdGrow = 1;
int birdDirX = 1;


GLfloat birdSpeedY = 0.1f;
GLfloat birdSpeedX = 0.05f;
GLfloat birdSpeedScale = 0.004f;
GLfloat birdSwayRange = 100.0f;


// umbrella
void shape01() {
   glColor3f(0.8863f, 0.3294f, 0.2745f);
    glBegin(GL_POLYGON);
        glVertex2f(139.27f, 317.46f);
        glVertex2f(147.30f, 340.58f);
        glVertex2f(160.93f, 353.48f);
        glVertex2f(191.35f, 366.38f);
        glVertex2f(224.69f, 373.92f);
        glVertex2f(259.24f, 376.60f);
        glVertex2f(287.96f, 374.16f);
        glVertex2f(326.76f, 364.05f);
        glVertex2f(347.11f, 354.76f);
        glVertex2f(359.50f, 346.13f);
        glVertex2f(369.68f, 334.63f);
        glVertex2f(375.37f, 315.39f);
        glVertex2f(364.97f, 313.40f);
        glVertex2f(268.29f, 312.96f);
        glVertex2f(249.93f, 313.18f);
        glVertex2f(173.17f, 315.39f);
        glVertex2f(157.02f, 316.50f);
        glVertex2f(145.95f, 317.61f);
    glEnd();
}


void shape02() {
glColor3f(0.8863f, 0.5294f, 0.1059f);
 glBegin(GL_POLYGON);
        glVertex2f(249.93f, 313.18f);
        glVertex2f(245.47f, 109.20f);
        glVertex2f(255.36f, 107.02f);
        glVertex2f(263.52f, 109.30f);
        glVertex2f(268.29f, 312.96f);
    glEnd();
}

// topwallaftergreen
void shape04() {
    glColor3f(0.278f, 0.678f, 0.282f);

    glBegin(GL_POLYGON);
        glVertex2f(67.16f, 403.57f);
        glVertex2f(65.40f, 417.63f);
        glVertex2f(68.35f, 421.61f);
        glVertex2f(68.72f, 441.28f);
        glVertex2f(78.73f, 441.03f);
        glVertex2f(78.73f, 421.00f);
        glVertex2f(86.43f, 420.76f);
        glVertex2f(86.55f, 436.76f);
        glVertex2f(91.19f, 437.12f);
        glVertex2f(91.27f, 420.30f);
        glVertex2f(86.34f, 420.64f);
        glVertex2f(86.61f, 436.90f);
        glVertex2f(91.11f, 437.16f);
        glVertex2f(106.56f, 437.38f);
        glVertex2f(106.56f, 417.92f);
        glVertex2f(118.32f, 417.92f);
        glVertex2f(118.08f, 437.03f);
        glVertex2f(122.26f, 437.01f);
        glVertex2f(132.71f, 437.42f);
        glVertex2f(133.36f, 417.73f);
        glVertex2f(129.30f, 412.14f);
        glVertex2f(127.21f, 404.75f);
        glVertex2f(120.57f, 396.40f);
        glVertex2f(65.95f, 399.62f);
    glEnd();
}

void shape05() {
        glColor3f(0.278f, 0.678f, 0.282f);
        glBegin(GL_POLYGON);
        glVertex2f(120.57f, 396.40f);
        glVertex2f(127.21f, 404.75f);
        glVertex2f(129.30f, 412.14f);
        glVertex2f(133.36f, 417.73f);
        glVertex2f(140.00f, 420.00f);
        glVertex2f(146.23f, 419.35f);
        glVertex2f(151.50f, 413.75f);
        glVertex2f(150.68f, 405.85f);
        glVertex2f(150.22f, 401.46f);
        glVertex2f(142.64f, 393.09f);
        glVertex2f(132.89f, 394.92f);
    glEnd();
}

void shape06() {
glColor3f(0.2627f, 0.4392f, 0.1725f);
    glBegin(GL_POLYGON);
        glVertex2f(142.64f, 393.09f);
        glVertex2f(150.22f, 401.46f);
        glVertex2f(150.68f, 405.85f);
        glVertex2f(151.50f, 413.75f);
        glVertex2f(158.92f, 411.71f);
        glVertex2f(159.70f, 392.62f);
    glEnd();
}

// slidemainrampe
void shape07() {

  glColor3f(0.7569f, 0.8078f, 0.7569f);
    glBegin(GL_POLYGON);
        glVertex2f(65.95f, 399.62f);
        glVertex2f(120.57f, 396.40f);
        glVertex2f(132.89f, 394.92f);
        glVertex2f(142.64f, 393.09f);
        glVertex2f(159.70f, 392.62f);
        glVertex2f(174.07f, 392.14f);
        glVertex2f(202.78f, 382.79f);
        glVertex2f(324.77f, 295.67f);
        glVertex2f(350.33f, 275.77f);
        glVertex2f(378.06f, 259.79f);
        glVertex2f(422.11f, 238.56f);
        glVertex2f(463.54f, 222.41f);
        glVertex2f(501.63f, 211.70f);
        glVertex2f(550.07f, 203.98f);
        glVertex2f(581.67f, 200.12f);
        glVertex2f(607.29f, 198.71f);
        glVertex2f(615.37f, 191.34f);
        glVertex2f(569.91f, 182.74f);
        glVertex2f(548.14f, 178.70f);
        glVertex2f(530.41f, 178.70f);
        glVertex2f(530.69f, 181.83f);
        glVertex2f(522.91f, 186.20f);
        glVertex2f(464.91f, 191.92f);
        glVertex2f(278.77f, 251.90f);
        glVertex2f(234.22f, 285.44f);
        glVertex2f(153.03f, 360.61f);
        glVertex2f(123.96f, 380.57f);
        glVertex2f(95.92f, 392.09f);
        glVertex2f(79.24f, 397.94f);
    glEnd();
}

// undertheslidepart1
void shape08() {
   glColor3f(0.255f, 0.361f, 0.345f);
    glBegin(GL_POLYGON);
        glVertex2f(72.91f, 381.04f);
        glVertex2f(67.69f, 383.66f);
        glVertex2f(65.95f, 399.62f);
        glVertex2f(97.94f, 388.69f);
        glVertex2f(118.24f, 377.87f);
        glVertex2f(135.05f, 365.27f);
        glVertex2f(154.66f, 348.64f);
        glVertex2f(221.62f, 286.06f);
        glVertex2f(134.50f, 313.88f);
        glVertex2f(120.45f, 322.87f);
        glVertex2f(72.95f, 352.38f);
    glEnd();
}

// undertheslidepart2
void shape09() {
   glColor3f(0.4784f, 0.5961f, 0.5765f);
    glBegin(GL_POLYGON);
        glVertex2f(67.69f, 383.66f);
        glVertex2f(67.21f, 388.09f);
        glVertex2f(87.64f, 382.39f);
        glVertex2f(109.41f, 372.95f);
        glVertex2f(130.00f, 360.00f);
        glVertex2f(150.00f, 343.14f);
        glVertex2f(147.30f, 340.58f);
        glVertex2f(139.27f, 317.46f);
        glVertex2f(138.03f, 313.85f);
        glVertex2f(129.40f, 322.64f);
        glVertex2f(126.09f, 326.77f);
        glVertex2f(120.39f, 340.66f);
        glVertex2f(109.28f, 354.93f);
        glVertex2f(72.91f, 381.04f);
    glEnd();
}

// undertheslidepartwall
void shape10() {
  glColor3f(0.4784f, 0.5961f, 0.5765f);
    glBegin(GL_POLYGON);
        glVertex2f(58.87f, 229.51f);
        glVertex2f(60.30f, 313.92f);
        glVertex2f(63.16f, 361.84f);
        glVertex2f(67.21f, 388.09f);
        glVertex2f(72.91f, 381.04f);
        glVertex2f(72.95f, 352.38f);
        glVertex2f(74.25f, 307.12f);
        glVertex2f(73.89f, 272.07f);
        glVertex2f(73.90f, 270.03f);
        glVertex2f(68.71f, 269.84f);
        glVertex2f(67.29f, 229.21f);
    glEnd();
}

// undertheslidepartwall2
void shape11() {
      glColor3f(0.255f, 0.361f, 0.345f);
    glBegin(GL_POLYGON);
        glVertex2f(2.39f, 224.96f);
        glVertex2f(8.08f, 225.08f);
        glVertex2f(9.14f, 366.32f);
        glVertex2f(63.16f, 361.84f);
        glVertex2f(58.87f, 229.51f);
    glEnd();
}

// wallsidearea
    void shape12(){
       glColor3f(0.373f, 0.545f, 0.196f);
       glBegin(GL_POLYGON);
        glVertex2f(59.17f, 464.12f);
        glVertex2f(33.76f, 480.92f);
        glVertex2f(35.00f, 485.00f);
        glVertex2f(37.93f, 483.18f);
        glVertex2f(40.41f, 483.25f);
        glVertex2f(38.55f, 486.08f);
        glVertex2f(39.38f, 488.49f);
        glVertex2f(41.24f, 488.56f);
        glVertex2f(41.10f, 490.14f);
        glVertex2f(46.61f, 489.93f);
        glVertex2f(46.75f, 481.81f);
        glVertex2f(52.19f, 482.01f);
        glVertex2f(51.85f, 484.97f);
        glVertex2f(53.02f, 485.18f);
        glVertex2f(52.60f, 487.73f);
        glVertex2f(55.57f, 487.59f);
        glVertex2f(55.84f, 484.22f);
        glVertex2f(56.32f, 490.07f);
        glVertex2f(58.39f, 490.07f);
        glVertex2f(58.60f, 486.01f);
        glVertex2f(60.00f, 485.00f);
        glVertex2f(62.04f, 485.18f);
        glVertex2f(61.83f, 487.38f);
        glVertex2f(66.31f, 487.25f);
        glVertex2f(66.24f, 482.01f);
        glVertex2f(70.97f, 482.03f);
        glVertex2f(74.98f, 481.15f);
        glVertex2f(75.06f, 483.99f);
        glVertex2f(76.67f, 484.25f);
        glVertex2f(76.59f, 487.39f);
        glVertex2f(77.90f, 487.43f);
        glVertex2f(77.68f, 490.09f);
        glVertex2f(80.47f, 489.96f);
        glVertex2f(80.60f, 487.74f);
        glVertex2f(81.77f, 487.74f);
        glVertex2f(81.91f, 484.73f);
        glVertex2f(88.14f, 484.64f);
        glVertex2f(88.18f, 483.16f);
        glVertex2f(93.37f, 483.33f);
        glVertex2f(93.28f, 485.43f);
        glVertex2f(94.55f, 485.34f);
        glVertex2f(99.78f, 485.73f);
        glVertex2f(99.82f, 487.26f);
        glVertex2f(104.05f, 487.17f);
        glVertex2f(104.00f, 486.00f);
        glVertex2f(106.00f, 486.00f);
        glVertex2f(106.05f, 487.21f);
        glVertex2f(107.84f, 487.26f);
        glVertex2f(109.32f, 485.97f);
        glVertex2f(109.40f, 483.11f);
        glVertex2f(107.78f, 482.67f);
        glVertex2f(107.20f, 481.13f);
        glVertex2f(105.59f, 479.30f);
        glVertex2f(105.81f, 476.59f);
        glVertex2f(104.41f, 473.95f);
        glVertex2f(105.88f, 471.75f);
        glVertex2f(106.03f, 467.87f);
        glVertex2f(107.64f, 466.69f);
        glVertex2f(109.25f, 464.86f);
        glVertex2f(110.99f, 464.80f);
        glVertex2f(110.87f, 473.82f);
        glVertex2f(114.26f, 473.90f);
        glVertex2f(117.10f, 474.02f);
        glVertex2f(117.16f, 469.08f);
        glVertex2f(118.22f, 468.90f);
        glVertex2f(118.36f, 467.64f);
        glVertex2f(119.50f, 468.96f);
        glVertex2f(119.44f, 470.30f);
        glVertex2f(121.14f, 470.36f);
        glVertex2f(122.02f, 472.53f);
        glVertex2f(123.63f, 471.30f);
        glVertex2f(124.68f, 468.81f);
        glVertex2f(125.01f, 466.44f);
        glVertex2f(127.40f, 466.53f);
        glVertex2f(127.35f, 470.07f);
        glVertex2f(131.33f, 470.04f);
        glVertex2f(131.53f, 465.97f);
        glVertex2f(136.74f, 470.69f);
        glVertex2f(136.65f, 471.45f);
        glVertex2f(140.63f, 471.30f);
        glVertex2f(141.93f, 470.11f);
        glVertex2f(141.68f, 466.42f);
        glVertex2f(140.00f, 465.00f);
        glVertex2f(139.64f, 460.87f);
        glVertex2f(137.09f, 459.53f);
        glVertex2f(137.28f, 456.22f);
        glVertex2f(140.00f, 455.00f);
        glVertex2f(140.85f, 451.63f);
        glVertex2f(143.66f, 451.75f);
        glVertex2f(143.46f, 453.54f);
        glVertex2f(147.48f, 453.79f);
        glVertex2f(147.80f, 451.82f);
        glVertex2f(151.18f, 452.07f);
        glVertex2f(154.43f, 451.94f);
        glVertex2f(155.00f, 455.00f);
        glVertex2f(157.49f, 455.77f);
        glVertex2f(157.43f, 457.55f);
        glVertex2f(160.23f, 458.38f);
        glVertex2f(163.04f, 456.98f);
        glVertex2f(162.08f, 454.69f);
        glVertex2f(160.68f, 453.67f);
        glVertex2f(160.00f, 450.00f);
        glVertex2f(162.91f, 448.80f);
        glVertex2f(162.99f, 441.84f);
        glVertex2f(157.75f, 441.84f);
        glVertex2f(158.92f, 411.71f);
        glVertex2f(151.50f, 413.75f);
        glVertex2f(146.23f, 419.35f);
        glVertex2f(140.00f, 420.00f);
        glVertex2f(133.36f, 417.73f);
        glVertex2f(132.71f, 437.42f);
        glVertex2f(122.26f, 437.01f);
        glVertex2f(118.08f, 437.03f);
        glVertex2f(118.32f, 417.92f);
        glVertex2f(106.56f, 417.92f);
        glVertex2f(106.56f, 437.38f);
        glVertex2f(91.11f, 437.16f);
        glVertex2f(86.61f, 436.90f);
        glVertex2f(86.34f, 420.64f);
        glVertex2f(78.73f, 421.00f);
        glVertex2f(78.73f, 441.03f);
        glVertex2f(68.72f, 441.28f);
        glVertex2f(68.50f, 460.29f);
    glEnd();
}

  // topwallxaxis
   void shape13() {
    glColor3f(0.439f, 0.220f, 0.129f);
    glBegin(GL_POLYGON);
        glVertex2f(0.31f, 503.98f);
        glVertex2f(33.76f, 480.92f);
        glVertex2f(59.17f, 464.12f);
        glVertex2f(68.50f, 460.29f);
        glVertex2f(68.72f, 441.28f);
        glVertex2f(68.35f, 421.61f);
        glVertex2f(65.40f, 417.63f);
        glVertex2f(62.33f, 413.10f);
        glVertex2f(55.28f, 409.73f);
        glVertex2f(40.00f, 410.00f);
        glVertex2f(31.74f, 417.22f);
        glVertex2f(30.00f, 430.00f);
        glVertex2f(34.22f, 437.77f);
        glVertex2f(38.85f, 440.56f);
        glVertex2f(45.00f, 440.00f);
        glVertex2f(50.25f, 433.97f);
        glVertex2f(52.23f, 434.54f);
        glVertex2f(47.77f, 443.07f);
        glVertex2f(46.00f, 444.00f);
        glVertex2f(38.00f, 444.00f);
        glVertex2f(33.31f, 441.16f);
        glVertex2f(27.30f, 433.38f);
        glVertex2f(20.00f, 410.00f);
        glVertex2f(16.20f, 374.27f);
        glVertex2f(0.47f, 374.01f);
        glVertex2f(5.76f, 374.40f);
        glVertex2f(11.98f, 428.11f);
        glVertex2f(20.91f, 463.61f);
        glVertex2f(55.13f, 452.22f);
        glVertex2f(59.90f, 454.26f);
        glVertex2f(59.99f, 462.35f);
        glVertex2f(10.75f, 481.56f);
        glVertex2f(1.23f, 486.96f);
    glEnd();
}

   void shape14() {
    glColor3f(0.894f, 0.137f, 0.141f);
    glBegin(GL_POLYGON);
        glVertex2f(375.64f, 310.46f);
        glVertex2f(381.68f, 306.84f);
        glVertex2f(387.65f, 306.84f);
        glVertex2f(392.90f, 303.64f);
        glVertex2f(608.56f, 244.41f);
        glVertex2f(615.07f, 245.17f);
        glVertex2f(621.09f, 237.38f);
        glVertex2f(623.22f, 221.37f);
        glVertex2f(639.53f, 216.21f);
        glVertex2f(647.79f, 206.07f);
        glVertex2f(648.54f, 201.38f);
        glVertex2f(622.70f, 197.61f);
        glVertex2f(607.29f, 198.71f);
        glVertex2f(581.45f, 203.33f);
        glVertex2f(385.00f, 263.42f);
        glVertex2f(350.33f, 275.77f);
        glVertex2f(324.77f, 295.67f);
        glVertex2f(381.04f, 283.55f);
    glEnd();
}

//redslideinnerpart
void shape15() {
   glColor3f(0.9725f, 0.8157f, 0.5608f);
   glBegin(GL_POLYGON);
        glVertex2f(381.04f, 283.55f);
        glVertex2f(615.89f, 213.50f);
        glVertex2f(624.69f, 204.82f);
        glVertex2f(627.73f, 198.34f);
        glVertex2f(622.70f, 197.61f);
        glVertex2f(617.40f, 204.82f);
        glVertex2f(611.72f, 207.18f);
        glVertex2f(382.97f, 276.07f);
    glEnd();
}

// main slide part last portion
void shape16() {
     glColor3f(0.7569f, 0.8078f, 0.7569f);
    glBegin(GL_POLYGON);
        glVertex2f(381.46f, 256.09f);
        glVertex2f(408.58f, 236.20f);
        glVertex2f(465.35f, 215.14f);
        glVertex2f(506.38f, 204.05f);
        glVertex2f(552.06f, 197.84f);
        glVertex2f(602.61f, 195.36f);
        glVertex2f(607.76f, 189.90f);
        glVertex2f(569.91f, 182.74f);
        glVertex2f(564.29f, 188.36f);
        glVertex2f(529.41f, 191.98f);
        glVertex2f(466.46f, 201.12f);
        glVertex2f(413.08f, 214.31f);
        glVertex2f(382.97f, 229.28f);
        glVertex2f(381.04f, 243.55f);
    glEnd();
}

// slide part below
void shape18() {
     glColor3f(0.8706f, 0.1529f, 0.1373f);
     glBegin(GL_POLYGON);
        glVertex2f(385.75f, 218.66f);
        glVertex2f(386.12f, 212.62f);
        glVertex2f(457.91f, 190.30f);
        glVertex2f(464.33f, 185.38f);
        glVertex2f(468.90f, 178.47f);
        glVertex2f(471.33f, 173.46f);
        glVertex2f(448.35f, 169.95f);
        glVertex2f(439.15f, 184.07f);
        glVertex2f(431.32f, 190.07f);
        glVertex2f(417.28f, 195.17f);
        glVertex2f(387.44f, 207.72f);
        glVertex2f(388.03f, 211.58f);
    glEnd();
}

//slide round deep red
void shape19() {
   glColor3f(0.6157f, 0.1294f, 0.1294f);
    glBegin(GL_POLYGON);
        glVertex2f(3.41f, 294.89f);
        glVertex2f(3.61f, 323.09f);
        glVertex2f(81.87f, 304.44f);
        glVertex2f(248.49f, 247.45f);
        glVertex2f(417.28f, 195.17f);
        glVertex2f(431.32f, 190.07f);
        glVertex2f(439.15f, 184.07f);
        glVertex2f(448.35f, 169.95f);
        glVertex2f(442.73f, 168.44f);
        glVertex2f(435.87f, 169.84f);
        glVertex2f(431.39f, 177.98f);
        glVertex2f(425.28f, 181.90f);
        glVertex2f(80.89f, 296.46f);
        glVertex2f(80.00f, 280.00f);
        glVertex2f(91.57f, 274.59f);
        glVertex2f(393.34f, 180.73f);
        glVertex2f(91.16f, 274.52f);
    glEnd();
}

// slide support wall
void shape20() {
  glColor3f(0.4235f, 0.5059f, 0.4431f);
    glBegin(GL_POLYGON);
        glVertex2f(67.29f, 229.21f);
        glVertex2f(67.42f, 206.89f);
        glVertex2f(101.48f, 206.73f);
        glVertex2f(109.83f, 204.78f);
        glVertex2f(124.03f, 202.67f);
        glVertex2f(126.53f, 207.22f);
        glVertex2f(115.68f, 217.62f);
        glVertex2f(114.51f, 234.19f);
        glVertex2f(99.81f, 246.05f);
        glVertex2f(93.97f, 270.75f);
        glVertex2f(73.90f, 270.03f);
        glVertex2f(68.71f, 269.84f);
    glEnd();
}

// house side 1st cloud
void shape21() {
  glColor3f(0.9725f, 0.8157f, 0.5608f);
    glBegin(GL_POLYGON);
        glVertex2f(525.28f, 426.40f);
        glVertex2f(532.13f, 428.69f);
        glVertex2f(537.50f, 427.13f);
        glVertex2f(544.28f, 431.89f);
        glVertex2f(551.06f, 431.44f);
        glVertex2f(558.41f, 435.47f);
        glVertex2f(565.85f, 434.83f);
        glVertex2f(572.06f, 429.79f);
        glVertex2f(580.00f, 430.00f);
        glVertex2f(582.52f, 427.22f);
        glVertex2f(584.92f, 427.53f);
        glVertex2f(585.26f, 425.71f);
        glVertex2f(587.41f, 425.05f);
        glVertex2f(586.96f, 423.07f);
        glVertex2f(589.00f, 423.45f);
        glVertex2f(588.77f, 420.81f);
        glVertex2f(590.81f, 421.03f);
        glVertex2f(590.87f, 419.32f);
        glVertex2f(593.81f, 419.26f);
        glVertex2f(594.04f, 420.97f);
        glVertex2f(596.76f, 420.81f);
        glVertex2f(596.99f, 422.46f);
        glVertex2f(604.69f, 421.96f);
        glVertex2f(605.74f, 419.42f);
        glVertex2f(607.42f, 419.42f);
        glVertex2f(608.14f, 420.97f);
        glVertex2f(611.62f, 420.80f);
        glVertex2f(611.49f, 419.38f);
        glVertex2f(614.20f, 419.38f);
        glVertex2f(614.00f, 418.00f);
        glVertex2f(615.23f, 417.91f);
        glVertex2f(616.22f, 416.66f);
        glVertex2f(620.86f, 416.91f);
        glVertex2f(622.23f, 418.29f);
        glVertex2f(625.07f, 417.79f);
        glVertex2f(626.81f, 418.94f);
        glVertex2f(626.77f, 420.88f);
        glVertex2f(627.88f, 420.36f);
        glVertex2f(628.38f, 421.96f);
        glVertex2f(630.68f, 421.59f);
        glVertex2f(630.83f, 423.27f);
        glVertex2f(633.05f, 422.89f);
        glVertex2f(633.78f, 424.50f);
        glVertex2f(636.81f, 424.61f);
        glVertex2f(640.02f, 424.27f);
        glVertex2f(640.68f, 423.23f);
        glVertex2f(643.20f, 423.19f);
        glVertex2f(645.43f, 422.41f);
        glVertex2f(647.57f, 420.58f);
        glVertex2f(649.14f, 420.88f);
        glVertex2f(649.37f, 419.20f);
        glVertex2f(651.44f, 419.09f);
        glVertex2f(651.67f, 417.60f);
        glVertex2f(654.16f, 417.67f);
        glVertex2f(654.28f, 416.52f);
        glVertex2f(655.54f, 416.37f);
        glVertex2f(655.35f, 414.06f);
        glVertex2f(657.34f, 414.21f);
        glVertex2f(657.15f, 409.47f);
        glVertex2f(655.17f, 407.86f);
        glVertex2f(651.92f, 407.71f);
        glVertex2f(652.00f, 406.00f);
        glVertex2f(649.85f, 406.27f);
        glVertex2f(649.95f, 404.59f);
        glVertex2f(648.12f, 405.12f);
        glVertex2f(647.97f, 403.30f);
        glVertex2f(645.06f, 404.11f);
        glVertex2f(645.01f, 402.15f);
        glVertex2f(642.40f, 402.39f);
        glVertex2f(642.75f, 401.09f);
        glVertex2f(639.89f, 401.23f);
        glVertex2f(640.00f, 400.00f);
        glVertex2f(636.04f, 399.70f);
        glVertex2f(635.25f, 398.45f);
        glVertex2f(632.24f, 398.93f);
        glVertex2f(630.47f, 397.25f);
        glVertex2f(628.55f, 398.60f);
        glVertex2f(625.74f, 397.06f);
        glVertex2f(624.50f, 397.44f);
        glVertex2f(623.52f, 396.05f);
        glVertex2f(617.70f, 395.72f);
        glVertex2f(617.40f, 394.47f);
        glVertex2f(607.79f, 394.66f);
        glVertex2f(607.79f, 393.41f);
        glVertex2f(592.95f, 392.89f);
        glVertex2f(573.34f, 392.96f);
        glVertex2f(523.29f, 424.37f);
    glEnd();
}

// most small cloud
void shape22() {
      glColor3f(0.9725f, 0.8157f, 0.5608f);
    glBegin(GL_POLYGON);
        glVertex2f(761.99f, 430.23f);
        glVertex2f(756.83f, 431.32f);
        glVertex2f(753.63f, 433.08f);
        glVertex2f(751.68f, 430.78f);
        glVertex2f(745.97f, 431.18f);
        glVertex2f(744.16f, 433.22f);
        glVertex2f(741.65f, 432.40f);
        glVertex2f(737.19f, 434.03f);
        glVertex2f(720.47f, 434.03f);
        glVertex2f(716.71f, 435.52f);
        glVertex2f(714.21f, 439.45f);
        glVertex2f(716.55f, 442.20f);
        glVertex2f(720.76f, 442.62f);
        glVertex2f(721.87f, 445.27f);
        glVertex2f(724.45f, 444.76f);
        glVertex2f(724.07f, 446.86f);
        glVertex2f(726.85f, 446.30f);
        glVertex2f(727.04f, 448.39f);
        glVertex2f(729.29f, 447.97f);
        glVertex2f(729.24f, 450.49f);
        glVertex2f(730.96f, 450.26f);
        glVertex2f(731.11f, 452.03f);
        glVertex2f(732.83f, 451.80f);
        glVertex2f(732.78f, 453.57f);
        glVertex2f(735.75f, 453.05f);
        glVertex2f(736.04f, 454.59f);
        glVertex2f(742.31f, 454.54f);
        glVertex2f(743.13f, 452.73f);
        glVertex2f(745.90f, 453.05f);
        glVertex2f(746.67f, 451.89f);
        glVertex2f(750.00f, 452.00f);
        glVertex2f(750.00f, 454.00f);
        glVertex2f(751.27f, 453.24f);
        glVertex2f(751.60f, 454.78f);
        glVertex2f(753.95f, 454.59f);
        glVertex2f(754.00f, 456.00f);
        glVertex2f(757.85f, 455.65f);
        glVertex2f(758.00f, 454.00f);
        glVertex2f(760.40f, 454.43f);
        glVertex2f(760.00f, 456.00f);
        glVertex2f(761.52f, 455.86f);
        glVertex2f(761.62f, 458.17f);
        glVertex2f(762.92f, 458.41f);
        glVertex2f(762.85f, 460.01f);
        glVertex2f(764.14f, 459.60f);
        glVertex2f(764.04f, 460.97f);
        glVertex2f(765.44f, 460.76f);
        glVertex2f(765.47f, 462.36f);
        glVertex2f(768.06f, 462.33f);
        glVertex2f(768.00f, 462.00f);
        glVertex2f(768.10f, 463.62f);
        glVertex2f(775.52f, 463.55f);
        glVertex2f(775.66f, 462.36f);
        glVertex2f(778.60f, 462.26f);
        glVertex2f(778.60f, 460.90f);
        glVertex2f(781.26f, 460.97f);
        glVertex2f(781.01f, 459.50f);
        glVertex2f(783.50f, 459.71f);
        glVertex2f(783.65f, 458.43f);
        glVertex2f(785.09f, 458.43f);
        glVertex2f(785.45f, 456.79f);
        glVertex2f(786.90f, 456.55f);
        glVertex2f(787.68f, 454.32f);
        glVertex2f(790.00f, 455.00f);
        glVertex2f(789.49f, 452.39f);
        glVertex2f(792.62f, 452.04f);
        glVertex2f(795.51f, 451.45f);
        glVertex2f(796.41f, 449.87f);
        glVertex2f(799.06f, 449.93f);
        glVertex2f(798.70f, 448.05f);
        glVertex2f(802.37f, 448.47f);
        glVertex2f(802.19f, 447.06f);
        glVertex2f(805.50f, 446.53f);
        glVertex2f(805.75f, 445.30f);
        glVertex2f(807.85f, 445.30f);
        glVertex2f(807.91f, 443.78f);
        glVertex2f(809.54f, 442.61f);
        glVertex2f(808.82f, 440.03f);
        glVertex2f(810.74f, 439.85f);
        glVertex2f(810.98f, 435.16f);
        glVertex2f(810.74f, 433.64f);
        glVertex2f(807.97f, 433.46f);
        glVertex2f(807.79f, 431.82f);
        glVertex2f(803.58f, 432.41f);
        glVertex2f(803.46f, 431.18f);
        glVertex2f(799.24f, 431.18f);
        glVertex2f(799.18f, 429.95f);
        glVertex2f(780.00f, 430.00f);
    glEnd();
}

// house top cloud
void shape23() {
     glColor3f(0.9725f, 0.8157f, 0.5608f);
    glBegin(GL_POLYGON);
        glVertex2f(404.13f, 604.96f);
        glVertex2f(402.26f, 608.79f);
        glVertex2f(406.89f, 613.10f);
        glVertex2f(418.84f, 615.00f);
        glVertex2f(428.00f, 616.00f);
        glVertex2f(432.63f, 617.30f);
        glVertex2f(437.78f, 617.81f);
        glVertex2f(451.06f, 617.91f);
        glVertex2f(462.39f, 616.70f);
        glVertex2f(468.36f, 614.90f);
        glVertex2f(471.96f, 614.40f);
        glVertex2f(479.28f, 612.90f);
        glVertex2f(498.53f, 603.48f);
        glVertex2f(513.97f, 597.77f);
        glVertex2f(526.54f, 586.05f);
        glVertex2f(526.33f, 581.24f);
        glVertex2f(523.55f, 578.13f);
        glVertex2f(508.21f, 575.43f);
        glVertex2f(486.28f, 578.93f);
        glVertex2f(461.67f, 583.34f);
        glVertex2f(419.04f, 594.16f);
    glEnd();
}

// lest side cloud MOst big one
void shape24() {
      glColor3f(0.9725f, 0.8157f, 0.5608f);
    glBegin(GL_POLYGON);
        glVertex2f(60.14f, 586.14f);
        glVertex2f(43.23f, 583.94f);
        glVertex2f(33.67f, 588.13f);
        glVertex2f(29.36f, 592.16f);
        glVertex2f(24.42f, 592.01f);
        glVertex2f(21.55f, 595.11f);
        glVertex2f(18.04f, 596.82f);
        glVertex2f(18.20f, 599.30f);
        glVertex2f(15.49f, 601.63f);
        glVertex2f(16.29f, 610.63f);
        glVertex2f(14.06f, 613.73f);
        glVertex2f(13.90f, 621.18f);
        glVertex2f(17.40f, 626.45f);
        glVertex2f(23.62f, 628.93f);
        glVertex2f(23.62f, 636.53f);
        glVertex2f(30.00f, 640.00f);
        glVertex2f(29.29f, 650.56f);
        glVertex2f(32.86f, 653.34f);
        glVertex2f(35.99f, 654.21f);
        glVertex2f(36.44f, 663.52f);
        glVertex2f(38.91f, 664.68f);
        glVertex2f(38.68f, 666.09f);
        glVertex2f(41.34f, 666.20f);
        glVertex2f(41.57f, 667.95f);
        glVertex2f(44.35f, 667.50f);
        glVertex2f(44.87f, 669.24f);
        glVertex2f(53.84f, 668.62f);
        glVertex2f(54.12f, 667.27f);
        glVertex2f(56.38f, 668.06f);
        glVertex2f(57.60f, 670.48f);
        glVertex2f(58.46f, 670.31f);
        glVertex2f(62.40f, 673.12f);
        glVertex2f(72.23f, 673.07f);
        glVertex2f(76.22f, 671.38f);
        glVertex2f(78.36f, 670.20f);
        glVertex2f(81.37f, 670.08f);
        glVertex2f(82.00f, 667.95f);
        glVertex2f(86.52f, 668.00f);
        glVertex2f(87.67f, 665.30f);
        glVertex2f(91.32f, 665.13f);
        glVertex2f(91.84f, 663.72f);
        glVertex2f(95.77f, 662.66f);
        glVertex2f(95.89f, 659.67f);
        glVertex2f(101.55f, 658.10f);
        glVertex2f(101.73f, 655.90f);
        glVertex2f(103.81f, 655.28f);
        glVertex2f(104.91f, 653.03f);
        glVertex2f(105.00f, 655.00f);
        glVertex2f(111.50f, 654.21f);
        glVertex2f(113.87f, 652.81f);
        glVertex2f(114.22f, 650.84f);
        glVertex2f(116.01f, 650.78f);
        glVertex2f(115.84f, 649.20f);
        glVertex2f(122.90f, 648.92f);
        glVertex2f(123.13f, 650.44f);
        glVertex2f(126.25f, 650.61f);
        glVertex2f(127.52f, 649.20f);
        glVertex2f(128.92f, 648.78f);
        glVertex2f(129.08f, 650.26f);
        glVertex2f(130.33f, 650.20f);
        glVertex2f(130.10f, 651.57f);
        glVertex2f(131.19f, 651.72f);
        glVertex2f(131.34f, 653.01f);
        glVertex2f(133.72f, 653.05f);
        glVertex2f(133.75f, 654.53f);
        glVertex2f(137.50f, 654.29f);
        glVertex2f(137.72f, 653.28f);
        glVertex2f(140.59f, 652.92f);
        glVertex2f(140.59f, 651.97f);
        glVertex2f(141.75f, 651.85f);
        glVertex2f(142.15f, 650.49f);
        glVertex2f(142.94f, 650.46f);
        glVertex2f(143.25f, 649.15f);
        glVertex2f(144.34f, 649.15f);
        glVertex2f(144.65f, 647.81f);
        glVertex2f(145.66f, 647.90f);
        glVertex2f(145.87f, 645.40f);
        glVertex2f(147.44f, 645.43f);
        glVertex2f(147.15f, 642.68f);
        glVertex2f(148.64f, 642.63f);
        glVertex2f(148.35f, 639.76f);
        glVertex2f(149.98f, 641.35f);
        glVertex2f(154.78f, 641.20f);
        glVertex2f(161.54f, 637.28f);
        glVertex2f(169.50f, 637.43f);
        glVertex2f(175.82f, 633.62f);
        glVertex2f(180.48f, 623.38f);
        glVertex2f(187.99f, 622.94f);
        glVertex2f(193.10f, 620.31f);
        glVertex2f(198.97f, 625.58f);
        glVertex2f(204.83f, 622.36f);
        glVertex2f(206.48f, 616.95f);
        glVertex2f(212.04f, 617.09f);
        glVertex2f(214.90f, 613.14f);
        glVertex2f(223.17f, 612.41f);
        glVertex2f(236.39f, 601.00f);
        glVertex2f(244.96f, 600.42f);
        glVertex2f(254.43f, 593.25f);
        glVertex2f(269.46f, 587.25f);
        glVertex2f(273.52f, 582.28f);
        glVertex2f(273.97f, 578.04f);
        glVertex2f(271.42f, 574.82f);
        glVertex2f(259.39f, 573.51f);
        glVertex2f(256.84f, 569.26f);
        glVertex2f(246.32f, 565.31f);
        glVertex2f(230.08f, 567.07f);
        glVertex2f(215.05f, 571.46f);
        glVertex2f(208.44f, 567.65f);
        glVertex2f(196.26f, 567.07f);
        glVertex2f(184.68f, 570.43f);
        glVertex2f(175.21f, 566.05f);
        glVertex2f(159.28f, 565.75f);
        glVertex2f(146.50f, 570.00f);
        glVertex2f(137.79f, 570.14f);
        glVertex2f(115.09f, 570.43f);
        glVertex2f(109.83f, 574.09f);
        glVertex2f(104.26f, 574.82f);
        glVertex2f(97.05f, 574.53f);
        glVertex2f(83.67f, 576.72f);
        glVertex2f(79.31f, 574.09f);
        glVertex2f(70.14f, 576.87f);
    glEnd();
}

// shiri
void shape25() {
    glColor3f(0.9216f, 0.6706f, 0.4902f);
    glBegin(GL_POLYGON);
        glVertex2f(165.26f, 392.84f);
        glVertex2f(168.15f, 444.81f);
        glVertex2f(181.41f, 444.49f);
        glVertex2f(182.61f, 443.12f);
        glVertex2f(182.50f, 423.44f);
        glVertex2f(190.22f, 423.34f);
        glVertex2f(190.70f, 422.03f);
        glVertex2f(195.18f, 421.94f);
        glVertex2f(195.35f, 441.97f);
        glVertex2f(202.85f, 441.72f);
        glVertex2f(202.92f, 443.89f);
        glVertex2f(209.69f, 443.95f);
        glVertex2f(209.68f, 458.64f);
        glVertex2f(216.04f, 458.44f);
        glVertex2f(216.19f, 464.83f);
        glVertex2f(219.99f, 464.98f);
        glVertex2f(220.24f, 463.63f);
        glVertex2f(221.78f, 463.48f);
        glVertex2f(221.78f, 462.53f);
        glVertex2f(226.70f, 462.33f);
        glVertex2f(226.86f, 458.49f);
        glVertex2f(231.74f, 458.49f);
        glVertex2f(232.26f, 456.78f);
        glVertex2f(286.98f, 508.91f);
        glVertex2f(289.23f, 508.75f);
        glVertex2f(289.53f, 476.70f);
        glVertex2f(237.75f, 427.63f);
        glVertex2f(237.37f, 438.20f);
        glVertex2f(226.63f, 437.46f);
        glVertex2f(224.69f, 373.92f);
        glVertex2f(202.78f, 382.79f);
        glVertex2f(174.07f, 392.14f);
    glEnd();
}

// shiri below
void shape26() {
  glColor3f(0.5725f, 0.3725f, 0.2431f);
    glBegin(GL_POLYGON);
        glVertex2f(236.51f, 376.43f);
        glVertex2f(237.75f, 427.63f);
        glVertex2f(289.53f, 476.70f);
        glVertex2f(313.78f, 471.41f);
        glVertex2f(263.48f, 420.38f);
        glVertex2f(237.15f, 420.58f);
    glEnd();
}

// house
void shape27() {
   glColor3f(0.9216f, 0.6706f, 0.4902f);
    glBegin(GL_POLYGON);
        glVertex2f(289.23f, 508.75f);
        glVertex2f(286.98f, 508.91f);
        glVertex2f(285.64f, 512.44f);
        glVertex2f(286.08f, 548.94f);
        glVertex2f(395.76f, 556.42f);
        glVertex2f(471.79f, 531.56f);
        glVertex2f(468.98f, 350.10f);
        glVertex2f(469.40f, 284.94f);
        glVertex2f(392.90f, 303.64f);
        glVertex2f(393.23f, 440.96f);
        glVertex2f(395.03f, 468.54f);
        glVertex2f(382.43f, 467.23f);
        glVertex2f(330.79f, 476.61f);
        glVertex2f(323.68f, 480.21f);
        glVertex2f(315.16f, 479.10f);
        glVertex2f(289.53f, 476.70f);
    glEnd();
}

// house bottom window
void shape28() {
   glColor3f(0.2706f, 0.4235f, 0.2392f);
    glBegin(GL_POLYGON);
        glVertex2f(413.48f, 386.26f);
        glVertex2f(394.64f, 416.52f);
        glVertex2f(396.35f, 448.65f);
        glVertex2f(460.86f, 437.68f);
        glVertex2f(460.26f, 396.47f);
        glVertex2f(463.03f, 393.58f);
        glVertex2f(451.34f, 380.89f);
        glVertex2f(441.41f, 380.06f);
        glVertex2f(431.48f, 395.72f);
        glVertex2f(422.23f, 384.89f);
    glEnd();
}

// top right side window
void shape29() {
     glColor3f(0.5451f, 0.8078f, 0.9412f);
    glBegin(GL_POLYGON);
        glVertex2f(395.76f, 556.42f);
        glVertex2f(395.03f, 468.54f);
        glVertex2f(470.60f, 454.82f);
        glVertex2f(471.79f, 531.56f);
    glEnd();
}

// Top lest side window
void shape30() {
   glColor3f(0.2235f, 0.3725f, 0.4471f);
    glBegin(GL_POLYGON);
        glVertex2f(345.66f, 510.84f);
        glVertex2f(345.46f, 545.47f);
        glVertex2f(379.69f, 546.80f);
        glVertex2f(379.69f, 502.28f);
        glVertex2f(376.17f, 503.04f);
        glVertex2f(375.78f, 541.67f);
        glVertex2f(350.00f, 540.00f);
        glVertex2f(351.33f, 508.75f);
    glEnd();
}

// bottom window
void shape31() {
    glColor3f(0.6078f, 0.4235f, 0.3412f);
    glBegin(GL_POLYGON);
        glVertex2f(289.23f, 508.75f);
        glVertex2f(288.83f, 524.22f);
        glVertex2f(297.39f, 524.22f);
        glVertex2f(297.98f, 512.23f);
        glVertex2f(304.74f, 512.81f);
        glVertex2f(305.54f, 526.55f);
        glVertex2f(313.69f, 525.19f);
        glVertex2f(313.49f, 514.55f);
        glVertex2f(322.44f, 513.39f);
        glVertex2f(323.04f, 527.13f);
        glVertex2f(329.60f, 526.16f);
        glVertex2f(330.79f, 513.19f);
        glVertex2f(337.95f, 511.84f);
        glVertex2f(338.55f, 524.03f);
        glVertex2f(344.91f, 522.87f);
        glVertex2f(345.66f, 510.84f);
        glVertex2f(351.33f, 508.75f);
        glVertex2f(353.74f, 508.49f);
        glVertex2f(353.51f, 519.21f);
        glVertex2f(360.00f, 520.00f);
        glVertex2f(361.21f, 506.93f);
        glVertex2f(367.90f, 504.68f);
        glVertex2f(367.38f, 516.95f);
        glVertex2f(376.05f, 514.70f);
        glVertex2f(376.17f, 503.04f);
        glVertex2f(382.53f, 500.96f);
        glVertex2f(382.98f, 513.05f);
        glVertex2f(389.94f, 511.55f);
        glVertex2f(389.43f, 467.96f);
        glVertex2f(382.43f, 467.23f);
        glVertex2f(330.79f, 476.61f);
        glVertex2f(323.68f, 480.21f);
        glVertex2f(315.16f, 479.10f);
        glVertex2f(289.53f, 476.70f);
    glEnd();
}

// window inner color
void shape32() {
   glColor3f(0.5529f, 0.6549f, 0.7059f);
    glBegin(GL_POLYGON);
        glVertex2f(395.32f, 503.31f);
        glVertex2f(400.00f, 500.00f);
        glVertex2f(401.55f, 490.79f);
        glVertex2f(408.80f, 488.75f);
        glVertex2f(407.53f, 495.59f);
        glVertex2f(403.01f, 495.70f);
        glVertex2f(403.61f, 540.09f);
        glVertex2f(429.86f, 532.23f);
        glVertex2f(429.98f, 495.59f);
        glVertex2f(428.28f, 494.85f);
        glVertex2f(428.49f, 484.82f);
        glVertex2f(424.52f, 485.84f);
        glVertex2f(418.87f, 486.17f);
        glVertex2f(418.87f, 495.40f);
        glVertex2f(418.66f, 497.70f);
        glVertex2f(415.39f, 497.91f);
        glVertex2f(417.52f, 536.98f);
        glVertex2f(403.21f, 539.92f);
        glVertex2f(402.80f, 495.68f);
        glVertex2f(400.74f, 495.56f);
        glVertex2f(399.76f, 500.02f);
    glEnd();
}

// window inner color
void shape33() {
    glColor3f(0.5529f, 0.6549f, 0.7059f);
    glBegin(GL_POLYGON);
        glVertex2f(428.28f, 494.85f);
        glVertex2f(437.00f, 492.74f);
        glVertex2f(437.13f, 530.35f);
        glVertex2f(462.03f, 522.45f);
        glVertex2f(462.44f, 489.40f);
        glVertex2f(467.39f, 487.52f);
        glVertex2f(460.24f, 488.19f);
        glVertex2f(460.00f, 480.00f);
        glVertex2f(454.32f, 480.70f);
        glVertex2f(452.81f, 484.58f);
        glVertex2f(452.54f, 489.26f);
        glVertex2f(450.61f, 492.07f);
        glVertex2f(443.87f, 492.07f);
        glVertex2f(443.05f, 483.24f);
        glVertex2f(436.03f, 483.91f);
        glVertex2f(436.05f, 492.66f);
    glEnd();
}

// house
   void shape34() {
      glColor3f(0.9216f, 0.6706f, 0.4902f);
      glBegin(GL_POLYGON);
        glVertex2f(286.08f, 548.94f);
        glVertex2f(280.52f, 547.69f);
        glVertex2f(279.86f, 586.36f);
        glVertex2f(295.63f, 587.16f);
        glVertex2f(295.14f, 571.50f);
        glVertex2f(300.88f, 571.66f);
        glVertex2f(301.54f, 584.92f);
        glVertex2f(311.23f, 584.12f);
        glVertex2f(311.72f, 573.90f);
        glVertex2f(320.92f, 574.38f);
        glVertex2f(321.25f, 585.72f);
        glVertex2f(330.44f, 585.72f);
        glVertex2f(331.43f, 575.49f);
        glVertex2f(341.61f, 576.77f);
        glVertex2f(341.28f, 587.16f);
        glVertex2f(351.46f, 586.84f);
        glVertex2f(350.00f, 580.00f);
        glVertex2f(360.99f, 578.69f);
        glVertex2f(360.17f, 588.44f);
        glVertex2f(372.48f, 589.56f);
        glVertex2f(372.81f, 580.61f);
        glVertex2f(382.17f, 581.73f);
        glVertex2f(382.34f, 604.10f);
        glVertex2f(395.80f, 603.94f);
        glVertex2f(403.68f, 601.70f);
        glVertex2f(403.52f, 578.69f);
        glVertex2f(408.77f, 577.57f);
        glVertex2f(408.61f, 588.12f);
        glVertex2f(416.82f, 586.20f);
        glVertex2f(417.97f, 575.33f);
        glVertex2f(424.21f, 572.30f);
        glVertex2f(423.72f, 581.89f);
        glVertex2f(431.44f, 580.13f);
        glVertex2f(431.93f, 569.74f);
        glVertex2f(437.02f, 567.98f);
        glVertex2f(436.53f, 577.09f);
        glVertex2f(444.57f, 575.81f);
        glVertex2f(445.40f, 566.07f);
        glVertex2f(450.65f, 563.83f);
        glVertex2f(450.32f, 573.10f);
        glVertex2f(454.76f, 571.98f);
        glVertex2f(455.58f, 577.89f);
        glVertex2f(460.00f, 580.00f);
        glVertex2f(470.03f, 578.05f);
        glVertex2f(472.49f, 576.29f);
        glVertex2f(471.79f, 531.56f);
        glVertex2f(395.76f, 556.42f);
        glVertex2f(285.62f, 548.66f);
        glVertex2f(280.43f, 547.82f);
    glEnd();
}

// house side structure
void shape35() {
  glColor3f(0.8314f, 0.5216f, 0.3608f);
    glBegin(GL_POLYGON);
        glVertex2f(469.54f, 386.52f);
        glVertex2f(681.84f, 286.13f);
        glVertex2f(659.83f, 269.47f);
        glVertex2f(536.10f, 264.31f);
        glVertex2f(469.40f, 284.94f);
    glEnd();
}

// house side structure
void shape36() {
   glColor3f(0.6902f, 0.4863f, 0.2667f);
    glBegin(GL_POLYGON);
        glVertex2f(469.47f, 334.79f);
        glVertex2f(479.06f, 332.24f);
        glVertex2f(491.65f, 329.52f);
        glVertex2f(500.91f, 312.51f);
        glVertex2f(505.46f, 274.58f);
        glVertex2f(519.80f, 269.35f);
        glVertex2f(522.24f, 296.52f);
        glVertex2f(528.35f, 313.36f);
        glVertex2f(538.32f, 325.95f);
        glVertex2f(550.73f, 325.95f);
        glVertex2f(557.19f, 317.10f);
        glVertex2f(565.06f, 293.29f);
        glVertex2f(565.24f, 264.89f);
        glVertex2f(577.12f, 264.89f);
        glVertex2f(582.19f, 301.11f);
        glVertex2f(588.83f, 314.72f);
        glVertex2f(597.75f, 319.14f);
        glVertex2f(605.09f, 317.78f);
        glVertex2f(614.88f, 293.97f);
        glVertex2f(615.40f, 265.74f);
        glVertex2f(625.36f, 266.08f);
        glVertex2f(628.16f, 290.57f);
        glVertex2f(633.75f, 301.11f);
        glVertex2f(641.97f, 302.82f);
        glVertex2f(650.00f, 300.00f);
        glVertex2f(658.57f, 285.30f);
        glVertex2f(659.83f, 269.47f);
        glVertex2f(536.10f, 264.31f);
        glVertex2f(469.40f, 284.94f);
    glEnd();
}

// house beside ride
void shape37() {
   glColor3f(0.8039f, 0.8863f, 0.8784f);
    glBegin(GL_POLYGON);
        glVertex2f(470.54f, 396.79f);
        glVertex2f(470.53f, 449.99f);
        glVertex2f(472.82f, 449.49f);
        glVertex2f(472.61f, 455.83f);
        glVertex2f(706.67f, 307.14f);
        glVertex2f(686.07f, 289.61f);
    glEnd();
}

// most right side green
void shape38() {
glColor3f(0.2627f, 0.4392f, 0.1725f);
    glBegin(GL_POLYGON);
        glVertex2f(923.03f, 336.31f);
        glVertex2f(927.50f, 334.48f);
        glVertex2f(931.47f, 336.63f);
        glVertex2f(937.46f, 336.95f);
        glVertex2f(938.10f, 343.61f);
        glVertex2f(941.96f, 346.32f);
        glVertex2f(947.09f, 346.73f);
        glVertex2f(949.34f, 342.46f);
        glVertex2f(957.15f, 342.15f);
        glVertex2f(956.40f, 338.30f);
        glVertex2f(953.30f, 337.57f);
        glVertex2f(953.40f, 335.07f);
        glVertex2f(955.44f, 334.66f);
        glVertex2f(955.12f, 330.70f);
        glVertex2f(956.94f, 329.24f);
        glVertex2f(956.72f, 325.60f);
        glVertex2f(958.54f, 324.45f);
        glVertex2f(963.46f, 322.99f);
        glVertex2f(965.00f, 325.00f);
        glVertex2f(968.03f, 325.49f);
        glVertex2f(967.77f, 328.19f);
        glVertex2f(969.75f, 328.39f);
        glVertex2f(969.35f, 330.70f);
        glVertex2f(971.33f, 330.57f);
        glVertex2f(978.03f, 332.10f);
        glVertex2f(980.67f, 333.31f);
        glVertex2f(993.17f, 336.94f);
        glVertex2f(998.41f, 334.80f);
        glVertex2f(1000.89f, 329.70f);
        glVertex2f(1006.96f, 326.47f);
        glVertex2f(1010.00f, 320.84f);
        glVertex2f(1013.31f, 334.80f);
        glVertex2f(1018.27f, 346.34f);
        glVertex2f(1024.62f, 343.92f);
        glVertex2f(1024.62f, 338.82f);
        glVertex2f(1032.90f, 336.68f);
        glVertex2f(1044.57f, 336.63f);
        glVertex2f(1050.47f, 335.76f);
        glVertex2f(1055.30f, 344.11f);
        glVertex2f(1057.62f, 340.80f);
        glVertex2f(1064.41f, 339.76f);
        glVertex2f(1068.38f, 343.26f);
        glVertex2f(1072.68f, 345.08f);
        glVertex2f(1074.22f, 342.83f);
        glVertex2f(1079.40f, 346.47f);
        glVertex2f(1065.58f, 339.22f);
        glVertex2f(1063.02f, 336.32f);
        glVertex2f(1065.35f, 332.69f);
        glVertex2f(1065.35f, 327.10f);
        glVertex2f(1058.05f, 326.80f);
        glVertex2f(1053.39f, 324.68f);
        glVertex2f(1052.30f, 321.06f);
        glVertex2f(1053.24f, 317.73f);
        glVertex2f(1060.22f, 314.10f);
        glVertex2f(1064.88f, 308.82f);
        glVertex2f(1066.44f, 303.83f);
        glVertex2f(1012.37f, 301.69f);
        glVertex2f(1010.71f, 286.10f);
        glVertex2f(1015.38f, 285.10f);
        glVertex2f(1011.32f, 281.89f);
        glVertex2f(1008.53f, 278.68f);
        glVertex2f(987.34f, 278.81f);
        glVertex2f(971.74f, 282.14f);
        glVertex2f(957.91f, 297.57f);
        glVertex2f(925.31f, 331.52f);
        glVertex2f(922.52f, 332.26f);
    glEnd();
}

   void canal(){
  glColor3f(0.1922f, 0.4235f, 0.6314f);
    glBegin(GL_POLYGON);
        glVertex2f(0.28f, 94.19f);
        glVertex2f(37.15f, 86.98f);
        glVertex2f(103.49f, 76.03f);
        glVertex2f(168.26f, 65.38f);
        glVertex2f(180.00f, 80.00f);
        glVertex2f(147.42f, 85.04f);
        glVertex2f(94.28f, 93.22f);
        glVertex2f(50.00f, 100.00f);
        glVertex2f(14.83f, 106.46f);
        glVertex2f(0.00f, 110.00f);
        glVertex2f(787.53f, 109.99f);
        glVertex2f(815.33f, 123.76f);
        glVertex2f(853.65f, 144.83f);
        glVertex2f(882.29f, 156.75f);
        glVertex2f(890.84f, 163.41f);
        glVertex2f(890.55f, 178.38f);
        glVertex2f(883.71f, 171.72f);
        glVertex2f(869.33f, 165.21f);
        glVertex2f(844.68f, 154.95f);
        glVertex2f(806.64f, 133.60f);
        glVertex2f(781.19f, 122.80f);
        glVertex2f(841.34f, 200.65f);
        glVertex2f(850.00f, 205.00f);
        glVertex2f(869.95f, 209.20f);
        glVertex2f(885.75f, 210.84f);
        glVertex2f(938.89f, 220.34f);
        glVertex2f(945.68f, 224.73f);
        glVertex2f(945.22f, 232.08f);
        glVertex2f(941.10f, 228.59f);
        glVertex2f(933.39f, 225.84f);
        glVertex2f(912.24f, 222.13f);
        glVertex2f(879.34f, 218.56f);
        glVertex2f(856.28f, 214.55f);
        glVertex2f(839.26f, 208.98f);
        glVertex2f(837.12f, 205.04f);
    glEnd();
}

void Waterfall(){
     glColor3f(0.2784f, 0.6863f, 0.2745f);
    glBegin(GL_POLYGON);
        glVertex2f(627.23f, 242.68f);
        glVertex2f(636.13f, 238.73f);
        glVertex2f(659.16f, 232.70f);
        glVertex2f(702.10f, 226.87f);
        glVertex2f(730.93f, 224.99f);
        glVertex2f(810.44f, 224.23f);
        glVertex2f(852.81f, 226.30f);
        glVertex2f(874.67f, 230.07f);
        glVertex2f(887.24f, 233.83f);
        glVertex2f(794.72f, 310.14f);
        glVertex2f(713.82f, 311.51f);
        glVertex2f(626.85f, 245.69f);
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(0.2863f, 0.3098f, 0.3020f);
        glVertex2f(751.84f, 310.62f);
        glVertex2f(758.64f, 310.63f);
        glVertex2f(758.79f, 400.78f);
        glVertex2f(752.57f, 400.57f);
    glEnd();
}

void Waterfall1(){
    glColor3f(0.9961f, 0.8588f, 0.0039f);
    glBegin(GL_POLYGON);
        glVertex2f(722.31f, 401.03f);
        glVertex2f(790.00f, 400.00f);
        glVertex2f(786.87f, 410.91f);
        glVertex2f(772.70f, 419.25f);
        glVertex2f(746.28f, 421.46f);
        glVertex2f(728.79f, 414.14f);
    glEnd();
}

void SIGNBOARD(){
    glColor3f(0.7451f, 0.4471f, 0.2196f);
    glBegin(GL_POLYGON);
        glVertex2f(917.88f, 87.08f);
        glVertex2f(940.44f, 108.39f);
        glVertex2f(1033.45f, 108.65f);
        glVertex2f(1033.97f, 72.96f);
        glVertex2f(1027.38f, 68.60f);
        glVertex2f(937.93f, 68.72f);
    glEnd();

    glBegin(GL_POLYGON);
        glVertex2f(973.65f, 68.67f);
        glVertex2f(990.00f, 68.65f);
        glVertex2f(990.14f, 13.33f);
        glVertex2f(974.05f, 13.60f);
    glEnd();
}

void housearch(){
glColor3f(0.2745f, 0.3922f, 0.3843f);
glBegin(GL_POLYGON);
    glVertex2f(469.47f, 334.79f);
    glVertex2f(479.06f, 332.24f);
    glVertex2f(491.65f, 329.52f);
    glVertex2f(500.91f, 312.51f);
    glVertex2f(505.46f, 274.58f);
    glVertex2f(519.80f, 269.35f);
    glVertex2f(536.10f, 264.31f);
    glVertex2f(469.47f, 334.79f);
glEnd();

glBegin(GL_POLYGON);
    glVertex2f(505.46f, 274.58f);
    glVertex2f(528.35f, 313.36f);
    glVertex2f(538.32f, 325.95f);
    glVertex2f(550.73f, 325.95f);
    glVertex2f(557.19f, 317.10f);
    glVertex2f(565.06f, 293.29f);
    glVertex2f(565.24f, 264.89f);
    glVertex2f(536.10f, 264.31f);
glEnd();

glBegin(GL_POLYGON);
    glVertex2f(565.24f, 264.89f);
    glVertex2f(577.12f, 264.89f);
    glVertex2f(582.19f, 301.11f);
    glVertex2f(588.83f, 314.72f);
    glVertex2f(597.75f, 319.14f);
    glVertex2f(605.09f, 317.78f);
    glVertex2f(614.88f, 293.97f);
    glVertex2f(615.40f, 265.74f);
    glVertex2f(625.36f, 266.08f);
    glVertex2f(628.16f, 290.57f);
    glVertex2f(633.75f, 301.11f);
    glVertex2f(641.97f, 302.82f);
    glVertex2f(650.00f, 300.00f);
    glVertex2f(658.57f, 285.30f);
    glVertex2f(659.83f, 269.47f);
glEnd();
}

void housewindow(){
glColor3f(0.5647f, 0.2980f, 0.1490f);
glBegin(GL_TRIANGLES);
    glVertex2f(396.35f, 448.65f);
    glVertex2f(460.86f, 437.68f);
    glVertex2f(413.48f, 386.26f);

    glVertex2f(460.86f, 437.68f);
    glVertex2f(413.48f, 386.26f);
    glVertex2f(431.48f, 395.72f);

    glVertex2f(460.86f, 437.68f);
    glVertex2f(431.48f, 395.72f);
    glVertex2f(460.26f, 396.47f);

    glVertex2f(460.26f, 396.47f);
    glVertex2f(431.48f, 395.72f);
    glVertex2f(422.23f, 384.89f);
glEnd();

glColor3f(0.4196f, 0.2941f, 0.2314f);
glBegin(GL_TRIANGLES);
    glVertex2f(395.78f, 437.92f);
    glVertex2f(448.00f, 437.92f);
    glVertex2f(442.19f, 416.46f);

    glVertex2f(395.78f, 437.92f);
    glVertex2f(442.19f, 416.46f);
    glVertex2f(416.07f, 385.85f);
glEnd();

glColor3f(0.0000f, 0.5000f, 0.0000f);
glBegin(GL_TRIANGLES);
    glVertex2f(431.48f, 395.72f);
    glVertex2f(442.19f, 416.46f);
    glVertex2f(422.23f, 384.89f);
glEnd();
}

void STAIR_WALL(){
   glColor3f(0.6549f, 0.3647f, 0.2078f);
    glBegin(GL_POLYGON);
        glVertex2f(236.51f, 376.43f);
        glVertex2f(237.15f, 420.58f);
        glVertex2f(289.53f, 476.70f);
        glVertex2f(315.16f, 479.10f);
        glVertex2f(323.68f, 480.21f);
        glVertex2f(324.82f, 479.63f);
        glVertex2f(330.79f, 476.61f);
        glVertex2f(330.00f, 470.00f);
        glVertex2f(329.88f, 447.68f);
        glVertex2f(329.43f, 362.84f);
        glVertex2f(314.82f, 367.16f);
    glEnd();
}

void stairwall(){
glColor3f(0.9333f, 0.6549f, 0.4510f);
    glBegin(GL_POLYGON);
        glVertex2f(236.51f, 376.43f);
        glVertex2f(237.15f, 420.58f);
        glVertex2f(289.53f, 476.70f);
        glVertex2f(315.16f, 479.10f);
        glVertex2f(323.68f, 480.21f);
        glVertex2f(315.51f, 468.28f);
        glVertex2f(315.51f, 447.58f);
        glVertex2f(329.43f, 362.84f);
        glVertex2f(314.82f, 367.16f);
    glEnd();

    glColor3f(0.4500f, 0.2500f, 0.1200f);
    glBegin(GL_POLYGON);
        glVertex2f(315.51f, 447.58f);
        glVertex2f(329.88f, 447.68f);
        glVertex2f(330.00f, 470.00f);
        glVertex2f(315.51f, 468.28f);
    glEnd();

    glColor3f(0.6500f, 0.4000f, 0.2200f);
    glBegin(GL_POLYGON);
        glVertex2f(316.26f, 470.49f);
        glVertex2f(322.86f, 472.10f);
        glVertex2f(330.00f, 470.00f);
        glVertex2f(315.51f, 468.28f);
    glEnd();
}

void FILL_GAP_SECTION() {
  glColor3f(0.9333f, 0.6549f, 0.4510f);
    glBegin(GL_POLYGON);
        glVertex2f(329.43f, 362.84f);
        glVertex2f(329.53f, 383.06f);
        glVertex2f(329.86f, 443.22f);
        glVertex2f(329.88f, 447.68f);
        glVertex2f(330.00f, 470.00f);
        glVertex2f(330.79f, 476.61f);
        glVertex2f(382.43f, 467.23f);
        glVertex2f(389.43f, 467.96f);
        glVertex2f(395.03f, 468.54f);
        glVertex2f(393.23f, 440.96f);
        glVertex2f(393.00f, 345.13f);
        glVertex2f(392.90f, 303.64f);
        glVertex2f(387.65f, 306.84f);
        glVertex2f(381.68f, 306.84f);
        glVertex2f(375.64f, 310.46f);
    glEnd();
}

void VERTICAL_PILLAR() {
    glColor3f(0.8500f, 0.7200f, 0.5200f);
    glBegin(GL_POLYGON);
        glVertex2f(236.51f, 376.43f);
        glVertex2f(237.75f, 427.63f);
        glVertex2f(237.37f, 438.20f);
        glVertex2f(226.63f, 437.46f);
    glEnd();
}

void Bird() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
        glVertex2f(520.22f, 617.16f);
        glVertex2f(526.21f, 617.16f);
        glVertex2f(526.37f, 615.34f);
        glVertex2f(528.82f, 615.34f);
        glVertex2f(530.16f, 611.86f);
        glVertex2f(531.33f, 610.45f);
        glVertex2f(532.13f, 612.95f);
        glVertex2f(533.04f, 615.24f);
        glVertex2f(535.00f, 615.00f);
        glVertex2f(535.45f, 616.80f);
        glVertex2f(537.96f, 617.78f);
        glVertex2f(540.57f, 617.68f);
        glVertex2f(535.75f, 616.49f);
        glVertex2f(535.37f, 614.82f);
        glVertex2f(532.98f, 612.81f);
        glVertex2f(531.64f, 609.53f);
        glVertex2f(529.56f, 611.56f);
        glVertex2f(528.46f, 614.98f);
        glVertex2f(525.98f, 614.74f);
    glEnd();
}

void DRAW_SECOND_BIRD() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
        glVertex2f(553.76f, 605.39f);
        glVertex2f(559.94f, 605.47f);
        glVertex2f(563.01f, 602.11f);
        glVertex2f(565.42f, 599.56f);
        glVertex2f(566.53f, 602.15f);
        glVertex2f(567.94f, 603.38f);
        glVertex2f(570.53f, 604.92f);
        glVertex2f(574.49f, 606.04f);
        glVertex2f(570.46f, 604.72f);
        glVertex2f(568.39f, 604.57f);
        glVertex2f(565.76f, 599.17f);
        glVertex2f(564.43f, 598.21f);
        glVertex2f(562.73f, 600.91f);
        glVertex2f(560.90f, 603.25f);
        glVertex2f(559.66f, 605.26f);
        glVertex2f(559.76f, 603.29f);
        glVertex2f(561.53f, 601.45f);
        glVertex2f(563.79f, 598.32f);
        glVertex2f(564.23f, 598.62f);
        glVertex2f(563.01f, 601.14f);
        glVertex2f(562.00f, 602.00f);
        glVertex2f(561.38f, 603.56f);
        glVertex2f(560.09f, 603.66f);
    glEnd();
}

void DRAW_THIRD_BIRD() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
        glVertex2f(589.63f, 586.03f);
        glVertex2f(596.60f, 585.85f);
        glVertex2f(598.00f, 584.00f);
        glVertex2f(599.10f, 582.78f);
        glVertex2f(600.56f, 581.13f);
        glVertex2f(602.00f, 580.00f);
        glVertex2f(603.18f, 582.58f);
        glVertex2f(604.55f, 583.94f);
        glVertex2f(606.13f, 585.27f);
        glVertex2f(610.95f, 586.49f);
        glVertex2f(606.49f, 586.13f);
        glVertex2f(605.04f, 584.83f);
        glVertex2f(603.62f, 583.55f);
        glVertex2f(602.32f, 582.22f);
        glVertex2f(601.37f, 579.83f);
        glVertex2f(600.54f, 578.64f);
        glVertex2f(599.39f, 581.12f);
        glVertex2f(597.87f, 582.69f);
        glVertex2f(596.61f, 584.04f);
        glVertex2f(589.59f, 585.75f);
    glEnd();
}

void DRAW_FOURTH_BIRD() {
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_POLYGON);
        glVertex2f(610.51f, 617.28f);
        glVertex2f(618.24f, 617.07f);
        glVertex2f(620.22f, 615.34f);
        glVertex2f(621.71f, 613.91f);
        glVertex2f(622.82f, 612.69f);
        glVertex2f(624.07f, 611.19f);
        glVertex2f(625.51f, 608.81f);
        glVertex2f(626.98f, 612.79f);
        glVertex2f(628.49f, 613.99f);
        glVertex2f(629.60f, 615.34f);
        glVertex2f(632.30f, 616.59f);
        glVertex2f(638.87f, 617.86f);
        glVertex2f(630.01f, 616.23f);
        glVertex2f(628.61f, 615.03f);
        glVertex2f(627.33f, 613.73f);
        glVertex2f(626.02f, 612.45f);
        glVertex2f(624.94f, 608.36f);
        glVertex2f(623.76f, 610.93f);
        glVertex2f(622.62f, 612.51f);
        glVertex2f(621.25f, 613.60f);
        glVertex2f(619.85f, 613.82f);
        glVertex2f(618.15f, 614.73f);
        glVertex2f(615.11f, 616.53f);
        glVertex2f(617.93f, 616.92f);
        glVertex2f(617.73f, 615.02f);
        glVertex2f(619.88f, 615.05f);
        glVertex2f(621.57f, 612.28f);
        glVertex2f(622.66f, 611.22f);
        glVertex2f(622.91f, 611.05f);
        glVertex2f(624.32f, 606.27f);
        glVertex2f(625.92f, 611.53f);
        glVertex2f(627.39f, 612.54f);
        glVertex2f(628.73f, 613.61f);
        glVertex2f(629.00f, 614.00f);
        glVertex2f(630.13f, 615.08f);
        glVertex2f(632.78f, 616.45f);
        glVertex2f(632.43f, 617.72f);
        glVertex2f(632.00f, 618.00f);
        glVertex2f(629.68f, 616.61f);
        glVertex2f(628.26f, 615.31f);
        glVertex2f(626.98f, 614.04f);
        glVertex2f(625.72f, 612.69f);
        glVertex2f(625.56f, 611.59f);
        glVertex2f(624.49f, 608.74f);
        glVertex2f(624.36f, 606.21f);
        glVertex2f(622.97f, 611.31f);
        glVertex2f(621.69f, 612.56f);
        glVertex2f(620.30f, 614.11f);
        glVertex2f(618.09f, 615.10f);
    glEnd();
}

void DRAW_RIDE_SECTION() {
    glColor3f(0.9373f, 0.5020f, 0.4235f);
    glBegin(GL_POLYGON);
        glVertex2f(72.95f, 352.38f);
        glVertex2f(74.25f, 307.12f);
        glVertex2f(81.87f, 304.44f);
        glVertex2f(120.45f, 322.87f);
        glVertex2f(134.50f, 313.88f);
        glVertex2f(221.62f, 286.06f);
        glVertex2f(248.49f, 247.45f);
        glVertex2f(278.77f, 251.90f);
        glVertex2f(417.28f, 195.17f);
        glVertex2f(464.91f, 191.92f);
        glVertex2f(472.14f, 184.78f);
        glVertex2f(473.56f, 182.10f);
    glEnd();
}

void DRAW_RED_RIDE() {
    glColor3f(0.85f, 0.15f, 0.15f);
    glBegin(GL_POLYGON);
        glVertex2f(81.87f, 304.44f);
        glVertex2f(97.32f, 325.43f);
        glVertex2f(249.16f, 278.0f);
        glVertex2f(248.49f, 247.45f);
    glEnd();
}

void DRAW_BLUE_SECTION() {
    glColor3f(0.19f, 0.42f, 0.63f);
    glBegin(GL_POLYGON);
        glVertex2f(0.44f, 28.34f);
        glVertex2f(0.59f, 49.25f);
        glVertex2f(29.30f, 75.99f);
        glVertex2f(30.00f, 60.00f);
        glVertex2f(37.15f, 86.98f);
    glEnd();
}


void DRAW_WATER_SECTION() {
    glColor3f(0.4902f, 0.8314f, 0.9412f);
    glBegin(GL_POLYGON);
        glVertex2f(722.31f, 401.03f);
        glVertex2f(752.57f, 400.57f);
        glVertex2f(751.84f, 310.62f);
        glVertex2f(713.82f, 311.51f);
        glVertex2f(714.88f, 321.10f);
        glVertex2f(712.19f, 322.56f);
        glVertex2f(715.63f, 335.68f);
        glVertex2f(717.54f, 347.49f);
        glVertex2f(718.31f, 359.56f);
        glVertex2f(717.89f, 364.62f);
        glVertex2f(719.12f, 376.95f);
    glEnd();
}

void DRAW_RIGHT_WATER_SECTION() {
    glColor3f(0.4902f, 0.8314f, 0.9412f);
    glBegin(GL_POLYGON);

    glVertex2f(758.79f, 400.78f);
    glVertex2f(790.00f, 400.00f);
    glVertex2f(794.72f, 310.14f);
    glVertex2f(751.84f, 310.62f);
    glVertex2f(758.79f, 400.78f);
    glEnd();
}


void drawSkyGradient() {
    glBegin(GL_QUADS);
        glColor3f(0.95f, 0.45f, 0.05f);
        glVertex2f(0.0f, 720.0f);
        glVertex2f(1080.0f, 720.0f);
        glColor3f(1.0f, 0.80f, 0.35f);
        glVertex2f(1080.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);
    glEnd();
}

void drawAllTracedShapes() {

    shape04();
    shape07();
    shape08();
    shape09();
    shape10();
    shape11();
    shape12();
    shape13();
    shape15();
    shape16();
    shape20();


    glPushMatrix();
        glTranslatef(cloudX, 0.0f, 0.0f);
        shape21();
        shape22();
        shape23();
        shape24();
    glPopMatrix();


    shape25();
    shape26();
    shape27();
    shape28();
    shape29();
    shape30();
    shape31();
    shape32();
    shape33();
    shape34();
    shape35();
    shape36();
    shape37();
    shape38();
    canal();
    Waterfall();
    Waterfall1();
    SIGNBOARD();
    housearch();
    housewindow();
    STAIR_WALL();
    stairwall();
    FILL_GAP_SECTION();
    VERTICAL_PILLAR();

    glPushMatrix();
        glTranslatef(birdX, birdY, 0.0f);
        glScalef(birdScale, birdScale, 1.0f);
        Bird();
        DRAW_SECOND_BIRD();
        DRAW_THIRD_BIRD();
        DRAW_FOURTH_BIRD();
    glPopMatrix();
    DRAW_RIDE_SECTION();
    DRAW_BLUE_SECTION();
    DRAW_WATER_SECTION() ;
    DRAW_RIGHT_WATER_SECTION();
    shape14();
    shape18();
    shape19();
    DRAW_RED_RIDE();
    shape01();
    shape02();
    shape05();
    shape06();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawSkyGradient();
    drawAllTracedShapes();
    glutSwapBuffers();
    glFlush();
}

void update(int value) {
    cloudX += 0.3f;
    if (cloudX > 300.0f) {
        cloudX = -1080.0f;
    }

    birdY += birdSpeedY;
    if (birdY > 150.0f) {
        birdY = 0.0f;
    }

    birdX += birdDirX * birdSpeedX;
    if (birdX > birdSwayRange) {
        birdDirX = -1;
    }
    if (birdX < -birdSwayRange) {
        birdDirX = 1;
    }

    if (birdGrow == 1) {
        birdScale += birdSpeedScale;
        if (birdScale >= 1.2f) {
            birdGrow = 0;
        }
    } else {
        birdScale -= birdSpeedScale;
        if (birdScale <= 0.8f) {
            birdGrow = 1;
        }
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}


void init() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1080.0, 0.0, 720.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_LINE_SMOOTH);
}

} // namespace Scene3

// ============================================================================
// SCENE 4 -- original main1.cpp (verbatim, unmodified)
// ============================================================================
namespace Scene4 {


  const int WIN_W = 1280;
  const int WIN_H = 720;
  const float PI = 3.14159265f;

  float carY = 0.0f;
  const float CAR_MIN  = 0.0f;
  const float CAR_MAX  = 330.0f;
  const float CAR_STEP = 10.0f;

  int carDir = 1;
  const int TIMER_MS = 30;
  bool isPaused = false;

  void setColor(float r, float g, float b)
  {
      glColor3f(r, g, b);
  }

  void drawRect(float x, float y, float w, float h)
  {
      glBegin(GL_QUADS);
          glVertex2f(x,     y);
          glVertex2f(x + w, y);
          glVertex2f(x + w, y + h);
          glVertex2f(x,     y + h);
      glEnd();
  }

  void drawPolygon(const float pts[][2], int n)
  {
      glBegin(GL_POLYGON);
          for (int i = 0; i < n; i++)
              glVertex2f(pts[i][0], pts[i][1]);
      glEnd();
  }

  void drawLine(float x1, float y1, float x2, float y2, float width)
  {
      glLineWidth(width);
      glBegin(GL_LINES);
          glVertex2f(x1, y1);
          glVertex2f(x2, y2);
      glEnd();
  }

  void drawCircle(float cx, float cy, float r, int segments)
  {
      glBegin(GL_POLYGON);
          for (int i = 0; i < segments; i++)
          {
              float theta = 2.0f * PI * (float)i / (float)segments;
              glVertex2f(cx + r * cosf(theta), cy + r * sinf(theta));
          }
      glEnd();
  }

  void drawSkyAndMountain()
  {
      setColor(0.310f, 0.690f, 0.902f);
      drawRect(0, 0, 1280, 720);

      const float mtn[8][2] = {
          {0, 220}, {192, 380}, {448, 316}, {640, 380},
          {833, 316}, {1089, 380}, {1280, 316}, {1280, 220}
      };
      setColor(0.290f, 0.620f, 0.322f);
      drawPolygon(mtn, 8);
  }

  void drawGrassTuft(float x, float y)
  {
      setColor(0.180f, 0.478f, 0.227f);
      glBegin(GL_TRIANGLES);
          glVertex2f(x,      y);
          glVertex2f(x + 4,  y + 14);
          glVertex2f(x + 8,  y);
      glEnd();
      glBegin(GL_TRIANGLES);
          glVertex2f(x + 4,  y);
          glVertex2f(x + 8,  y + 10);
          glVertex2f(x + 12, y);
      glEnd();
  }

  void drawGrassAndRoad()
  {
      setColor(0.306f, 0.796f, 0.361f);
      drawRect(0, 60, 1280, 160);

      const float tuftX[6] = {110, 293, 952, 1098, 860, 220};
      const float tuftY[6] = {100, 130, 130, 100, 160, 160};
      for (int i = 0; i < 6; i++)
          drawGrassTuft(tuftX[i], tuftY[i]);

      setColor(0.663f, 0.663f, 0.663f);
      drawRect(0, 0, 1280, 60);
  }

  void drawHexagonCrown()
  {
      const float neck[4][2] = { {618,605}, {662,605}, {650,625},
  {630,625} };
      setColor(0.176f, 0.141f, 0.314f);
      drawPolygon(neck, 4);

      const float hexPts[6][2] = {
          {640, 700}, {672, 681}, {672, 644}, {640, 625}, {608, 644},
  {608, 681}
      };

      setColor(0.310f, 0.690f, 0.902f);
      drawPolygon(hexPts, 6);

      glLineWidth(2.0f);
      setColor(0.910f, 0.353f, 0.165f);
      glBegin(GL_LINE_LOOP);
          for (int i = 0; i < 6; i++)
              glVertex2f(hexPts[i][0], hexPts[i][1]);
      glEnd();
  }

  void drawWindows()
  {
      setColor(0.310f, 0.690f, 0.902f);
      drawRect(618, 560, 44, 45);

      setColor(0.176f, 0.141f, 0.314f);
      const float leftArm[4][2]  = { {618,560}, {618,605}, {568,605},
  {548,560} };
      const float rightArm[4][2] = { {662,560}, {662,605}, {712,605},
  {732,560} };
      drawPolygon(leftArm, 4);
      drawPolygon(rightArm, 4);

      setColor(0.310f, 0.690f, 0.902f);
      const float leftHollow[4][2] = { {615,565}, {615,600}, {570,600},
  {552,565} };
      const float rightHollow[4][2] = { {665,565}, {665,600}, {708,600},
  {728,565} };
      drawPolygon(leftHollow, 4);
      drawPolygon(rightHollow, 4);

      setColor(0.0f, 0.0f, 0.0f);
      drawLine(618, 560, 568, 605, 3.0f);
      drawLine(548, 560, 618, 605, 3.0f);
      drawLine(662, 560, 712, 605, 3.0f);
      drawLine(732, 560, 662, 605, 3.0f);
  }

  void drawShaft()
  {
      setColor(0.753f, 0.224f, 0.169f);
      drawRect(630, 280, 20, 280);

      setColor(0.949f, 0.710f, 0.235f);
      drawRect(630, 520, 20, 40);

      setColor(0.180f, 0.373f, 0.541f);
      drawRect(630, 280, 20, 100);

      setColor(0.992f, 0.965f, 0.890f);
      for (float y = 295; y <= 545; y += 25)
          drawLine(630, y, 650, y, 2.0f);

      setColor(0.176f, 0.141f, 0.314f);
      drawRect(618, 280, 12, 280);
      drawRect(650, 280, 12, 280);
  }

  void drawTowerBase()
  {
      setColor(0.753f, 0.224f, 0.169f);
      drawRect(630, 100, 20, 180);

      setColor(0.176f, 0.141f, 0.314f);
      drawRect(618, 100, 12, 180);
      drawRect(650, 100, 12, 180);

      setColor(0.176f, 0.141f, 0.314f);
      drawLine(624, 100, 656, 190, 4.0f);
      drawLine(656, 100, 624, 190, 4.0f);
      drawLine(624, 190, 656, 280, 4.0f);
      drawLine(656, 190, 624, 280, 4.0f);

      setColor(0.851f, 0.725f, 0.541f);
      drawRect(590, 80, 100, 25);

      setColor(0.925f, 0.812f, 0.612f);
      drawRect(575, 60, 130, 20);

      setColor(0.663f, 0.663f, 0.663f);
      for (float x = 585; x <= 695; x += 20)
          drawCircle(x, 70, 3, 10);
  }

  void drawAwnings()
  {
      setColor(0.247f, 0.624f, 0.639f);
      const float left[4][2]  = { {540,165}, {620,165}, {600,185},
  {560,185} };
      const float right[4][2] = { {660,165}, {740,165}, {720,185},
  {680,185} };
      drawPolygon(left, 4);
      drawPolygon(right, 4);
  }

  void drawPlatform()
  {
      setColor(0.925f, 0.812f, 0.612f);
      drawRect(530, 150, 220, 15);

      setColor(0.851f, 0.725f, 0.541f);
      drawRect(530, 125, 220, 25);
  }

  void drawSeat(float x, float yBase)
  {
      setColor(0.820f, 0.208f, 0.165f);
      drawRect(x - 4, yBase, 20, 26);

      setColor(0.102f, 0.082f, 0.188f);
      drawRect(x + 2, yBase + 26, 16, 8);
  }

  void drawSeatsRow()
  {
      const float seatX[9] = {540, 562, 584, 606, 628, 650, 672, 694,
  716};
      for (int i = 0; i < 9; i++)
          drawSeat(seatX[i], 165);
  }

  void display()
  {
      glClear(GL_COLOR_BUFFER_BIT);

      drawSkyAndMountain();
      drawGrassAndRoad();

      drawHexagonCrown();
      drawWindows();
      drawShaft();
      drawTowerBase();

      glPushMatrix();
          glTranslatef(0.0f, carY, 0.0f);
          drawAwnings();
          drawPlatform();
          drawSeatsRow();
      glPopMatrix();

      glutSwapBuffers();
  }

  void specialKeys(int key, int x, int y)
  {
      if (key == GLUT_KEY_UP)
          carY += CAR_STEP;
      else if (key == GLUT_KEY_DOWN)
          carY -= CAR_STEP;

      if (carY < CAR_MIN) carY = CAR_MIN;
      if (carY > CAR_MAX) carY = CAR_MAX;

      glutPostRedisplay();
  }

  void keyboard(unsigned char key, int x, int y)
  {
      if (key == 'p' || key == 'P')
      {
          isPaused = !isPaused;
      }

      if (key == 'u' || key == 'U')
          carY += CAR_STEP;
      else if (key == 'd' || key == 'D')
          carY -= CAR_STEP;

      if (carY < CAR_MIN) carY = CAR_MIN;
      if (carY > CAR_MAX) carY = CAR_MAX;

      glutPostRedisplay();
  }

  void timer(int value)
  {
      if (!isPaused)
      {
          carY += carDir * CAR_STEP;

          if (carY >= CAR_MAX)
          {
              carY = CAR_MAX;
              carDir = -1;
          }
          else if (carY <= CAR_MIN)
          {
              carY = CAR_MIN;
              carDir = 1;
          }
          glutPostRedisplay();
      }

      glutTimerFunc(TIMER_MS, timer, 0);
  }

  void reshape(int width, int height)
  {
      if (height == 0) height = 1;

      glViewport(0, 0, width, height);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();

      const float baseAspect = 1280.0f / 720.0f;
      float windowAspect = (float)width / (float)height;

      if (windowAspect >= baseAspect)
      {
          float extraWidth = (720.0f * windowAspect - 1280.0f) / 2.0f;
          gluOrtho2D(-extraWidth, 1280.0f + extraWidth, 0.0, 720.0);
      }
      else
      {
          float extraHeight = (1280.0f / windowAspect - 720.0f) / 2.0f;
          gluOrtho2D(0.0, 1280.0, -extraHeight, 720.0f + extraHeight);
      }

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
  }

  void initGL()
  {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  }

} // namespace Scene4

// ============================================================================
// MERGE GLUE CODE
// This section is new code written to combine the four programs above --
// it is not part of, and does not alter, any of the four scenes' own
// drawing/animation logic. It only decides which scene's existing
// display()/timer functions get called, and reads the scene-switching
// keys the assignment asks for.
// ============================================================================

// Each original program set its own background clear color once, in its
// own init(). Now that all four scenes share one OpenGL context, we
// re-apply the correct original color for whichever scene is about to
// draw itself, using the exact same values each scene's own init() used.
void display() {
    switch (currentScene) {
        case 1:
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);        // Scene1's own init() color
            Scene1::display();
            break;
        case 2:
            glClearColor(0.529f, 0.808f, 0.922f, 1.0f);  // Scene2's own init() color
            Scene2::display();
            // main2.cpp was originally written for GLUT_SINGLE mode, so its
            // own display() only calls glFlush() and never swaps buffers.
            // The merged window runs GLUT_DOUBLE (needed by Scenes 1/3/4),
            // so we swap here instead of touching Scene2's own code.
            glutSwapBuffers();
            break;
        case 3:
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);        // Scene3's own init() color
            Scene3::display();
            break;
        case 4:
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);        // Scene4's own initGL() color
            Scene4::display();
            break;
    }
}

// '1'..'4' jump straight to a scene. Any other key is forwarded to the
// currently active scene's own keyboard handler, if it has one (only
// Scene4/main1.cpp defines one: 'p' pause, 'u'/'d' move the car).
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1': currentScene = 1; glutPostRedisplay(); return;
        case '2': currentScene = 2; glutPostRedisplay(); return;
        case '3': currentScene = 3; glutPostRedisplay(); return;
        case '4': currentScene = 4; glutPostRedisplay(); return;
    }
    if (currentScene == 4) Scene4::keyboard(key, x, y);
}

// Right/Left arrow keys step to the next/previous scene, wrapping around.
// Any other special key is forwarded to the active scene's own handler,
// if it has one (only Scene4/main1.cpp defines one: Up/Down moves the car).
void specialKeys(int key, int x, int y) {
    if (key == GLUT_KEY_RIGHT) {
        currentScene = (currentScene % 4) + 1;
        glutPostRedisplay();
        return;
    }
    if (key == GLUT_KEY_LEFT) {
        currentScene = (currentScene == 1) ? 4 : currentScene - 1;
        glutPostRedisplay();
        return;
    }
    if (currentScene == 4) Scene4::specialKeys(key, x, y);
}

// Forwards left-click to Scene1's own pause/play toggle (original
// main.cpp behaviour), only while Scene1 is the one on screen.
void mouseClick(int button, int state, int x, int y) {
    if (currentScene == 1) Scene1::mouse(button, state, x, y);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(50, 50);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("Merged Amusement Park Animation");

    // Run every scene's own initialization exactly as its original
    // program did (seeds star positions, enables blending, etc.) --
    // completely untouched.
    Scene1::init();
    Scene2::init();
    Scene3::init();
    Scene4::initGL();

    // Fixed 1280x720 window, ortho2d x:[0,1280] y:[0,720] for the whole
    // merged program, as requested. (This is applied last so it is the
    // ortho actually used, regardless of what any individual scene's own
    // init() set it to -- e.g. Scene3/project.cpp originally used a
    // 1080-wide ortho.)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1280.0, 0.0, 720.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseClick);

    // Start every scene's own animation timer chain exactly as its
    // original program did, so each scene's animation logic -- and its
    // own recursive glutTimerFunc rescheduling -- runs completely
    // unmodified in the background regardless of which scene is visible.
    glutTimerFunc(0,  Scene1::timer, 0);
    glutTimerFunc(16, Scene2::updateClouds, 0);
    glutTimerFunc(16, Scene2::update, 0);
    glutTimerFunc(16, Scene3::update, 0);
    glutTimerFunc(Scene4::TIMER_MS, Scene4::timer, 0);

    glutMainLoop();
    return 0;
}
