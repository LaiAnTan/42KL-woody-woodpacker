#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#define W 80
#define H 22

float A = 0, B = 0;

float clamp(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

/* ---------------- DONUT ---------------- */
void render_donut(char *b, float *z) {
    for (float j = 0; j < 6.28; j += 0.07) {
        for (float i = 0; i < 6.28; i += 0.02) {

            float c = sin(i), d = cos(j);
            float e = sin(A), f = sin(j);
            float g = cos(A), h = d + 2;

            float D = 1 / (c * h * e + f * g + 5);
            float l = cos(i);
            float t = c * h * g - f * e;

            int x = 40 + 30 * D * (l * h * cos(B) - t * sin(B));
            int y = 12 + 15 * D * (l * h * sin(B) + t * cos(B));

            int o = x + W * y;

            int N = 8 * (f * e - c * d * g);

            if (x >= 0 && x < W && y >= 0 && y < H && D > z[o]) {
                z[o] = D;
                b[o] = ".,-~:;=!*#$@"[(int)clamp(N, 0, 11)];
            }
        }
    }
}

/* ---------------- CUBE ---------------- */
void render_cube(char *b, float *z) {

    float size = 1.2;

    for (float x = -size; x <= size; x += 0.25) {
        for (float y = -size; y <= size; y += 0.25) {

            for (int s = 0; s < 6; s++) {

                float X = x, Y = y, Z;

                if (s == 0) Z = -size;
                if (s == 1) Z =  size;
                if (s == 2) { Z = y; Y = -size; }
                if (s == 3) { Z = y; Y =  size; }
                if (s == 4) { Z = x; X = -size; }
                if (s == 5) { Z = x; X =  size; }

                float cx = cos(A), sx = sin(A);
                float cy = cos(B), sy = sin(B);

                float x1 = X * cy - Z * sy;
                float z1 = X * sy + Z * cy;

                float y1 = Y * cx - z1 * sx;
                float z2 = Y * sx + z1 * cx + 3;

                float ooz = 1 / z2;

                int xp = 40 + (int)(30 * ooz * x1);
                int yp = 12 + (int)(15 * ooz * y1);

                int o = xp + W * yp;

                if (xp >= 0 && xp < W && yp >= 0 && yp < H && ooz > z[o]) {
                    z[o] = ooz;
                    b[o] = "#@%&*+=-."[s];
                }
            }
        }
    }
}

/* ---------------- SPHERE ---------------- */
void render_sphere(char *b, float *z) {

    for (float t = 0; t < 6.28; t += 0.07) {
        for (float p = 0; p < 6.28; p += 0.03) {

            float r = 1;

            float x = r * sin(t) * cos(p);
            float y = r * cos(t);
            float z0 = r * sin(t) * sin(p);

            float cx = cos(A), sx = sin(A);
            float cy = cos(B), sy = sin(B);

            float x1 = x * cy - z0 * sy;
            float z1 = x * sy + z0 * cy;

            float y1 = y * cx - z1 * sx;
            float z2 = y * sx + z1 * cx + 3;

            float ooz = 1 / z2;

            int xp = 40 + (int)(30 * ooz * x1);
            int yp = 12 + (int)(15 * ooz * y1);

            int o = xp + W * yp;

            if (xp >= 0 && xp < W && yp >= 0 && yp < H && ooz > z[o]) {
                z[o] = ooz;
                b[o] = ".:-=+*#%@"[(int)(p * 3) % 9];
            }
        }
    }
}

/* ---------------- MAIN ---------------- */
int main(int argc, char **argv) {

    char *mode = (argc > 1) ? argv[1] : "donut";

    void (*render)(char*, float*) = render_donut;

    if (strcmp(mode, "cube") == 0) render = render_cube;
    else if (strcmp(mode, "sphere") == 0) render = render_sphere;

    while (1) {
        char buf[W * H];
        float z[W * H];

        memset(buf, ' ', sizeof(buf));
        memset(z, 0, sizeof(z));

        render(buf, z);

        printf("\x1b[H");
        for (int i = 0; i < W * H; i++) {
            putchar(i % W ? buf[i] : '\n');
        }

        A += 0.04;
        B += 0.02;

        usleep(30000);
    }
}