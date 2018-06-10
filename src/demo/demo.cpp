#include <string.h>
#include <math.h>
#include "demo.h"
#include "vga.h"
#include "vgatext.h"

#define M_PI 3.14159265358979323846
#define random(howbig) (howbig == 0 ? 0 : (rand() % howbig))

float zOff = 150;
float xOff = 0;
float yOff = 0;
float cSize = 50;
float view_plane = 95;
float angle = M_PI / 60;

float cube3d[8][3] = {
    {xOff - cSize, yOff + cSize, zOff - cSize},
    {xOff + cSize, yOff + cSize, zOff - cSize},
    {xOff - cSize, yOff - cSize, zOff - cSize},
    {xOff + cSize, yOff - cSize, zOff - cSize},
    {xOff - cSize, yOff + cSize, zOff + cSize},
    {xOff + cSize, yOff + cSize, zOff + cSize},
    {xOff - cSize, yOff - cSize, zOff + cSize},
    {xOff + cSize, yOff - cSize, zOff + cSize}};

unsigned char cube2d[8][2];

namespace VgaDemo
{
    void printcube();

    void zrotate(float q);

    void yrotate(float q);

    void xrotate(float q);

   	void draw_cube(uint8_t c);
}

void VgaDemo::DemoSetup()
{
    //random cube forever.
    Vga::clear_screen(0x0C00);
    Vga::printAt(5, 23, "Random Cube Rotation", 0x0C00);
    srand(Vga::millis());
}

void VgaDemo::DemoLoop()
{
    int rsteps = random(60 - 10) + 9;
    switch (random(6))
    {
    case 0:
        for (int i = 0; i < rsteps; i++)
        {
            zrotate(angle);
            printcube();
        }
        break;
    case 1:
        for (int i = 0; i < rsteps; i++)
        {
            zrotate(2 * M_PI - angle);
            printcube();
        }
        break;
    case 2:
        for (int i = 0; i < rsteps; i++)
        {
            xrotate(angle);
            printcube();
        }
        break;
    case 3:
        for (int i = 0; i < rsteps; i++)
        {
            xrotate(2 * M_PI - angle);
            printcube();
        }
        break;
    case 4:
        for (int i = 0; i < rsteps; i++)
        {
            yrotate(angle);
            printcube();
        }
        break;
    case 5:
        for (int i = 0; i < rsteps; i++)
        {
            yrotate(2 * M_PI - angle);
            printcube();
        }
        break;
    }
}

void VgaDemo::printcube()
{
    draw_cube(PIXEL_CLEAR);

    //calculate 2d points
    for (uint8_t i = 0; i < 8; i++)
    {
        cube2d[i][0] = (unsigned char)((cube3d[i][0] * view_plane / cube3d[i][2]) + (Vga::hres() / 2));
        cube2d[i][1] = (unsigned char)((cube3d[i][1] * view_plane / cube3d[i][2]) + (Vga::vres() / 2));
    }

    draw_cube(PIXEL_SET);
}

void VgaDemo::zrotate(float q)
{
    float tx, ty, temp;
    for (uint8_t i = 0; i < 8; i++)
    {
        tx = cube3d[i][0] - xOff;
        ty = cube3d[i][1] - yOff;
        temp = tx * cos(q) - ty * sin(q);
        ty = tx * sin(q) + ty * cos(q);
        tx = temp;
        cube3d[i][0] = tx + xOff;
        cube3d[i][1] = ty + yOff;
    }
}

void VgaDemo::yrotate(float q)
{
    float tx, tz, temp;
    for (uint8_t i = 0; i < 8; i++)
    {
        tx = cube3d[i][0] - xOff;
        tz = cube3d[i][2] - zOff;
        temp = tz * cos(q) - tx * sin(q);
        tx = tz * sin(q) + tx * cos(q);
        tz = temp;
        cube3d[i][0] = tx + xOff;
        cube3d[i][2] = tz + zOff;
    }
}

void VgaDemo::xrotate(float q)
{
    float ty, tz, temp;
    for (uint8_t i = 0; i < 8; i++)
    {
        ty = cube3d[i][1] - yOff;
        tz = cube3d[i][2] - zOff;
        temp = ty * cos(q) - tz * sin(q);
        tz = ty * sin(q) + tz * cos(q);
        ty = temp;
        cube3d[i][1] = ty + yOff;
        cube3d[i][2] = tz + zOff;
    }
}

void VgaDemo::draw_cube(uint8_t c)
{
    Vga::draw_line(cube2d[0][0], cube2d[0][1], cube2d[1][0], cube2d[1][1], c);
    Vga::draw_line(cube2d[0][0], cube2d[0][1], cube2d[2][0], cube2d[2][1], c);
    Vga::draw_line(cube2d[0][0], cube2d[0][1], cube2d[4][0], cube2d[4][1], c);
    Vga::draw_line(cube2d[1][0], cube2d[1][1], cube2d[5][0], cube2d[5][1], c);
    Vga::draw_line(cube2d[1][0], cube2d[1][1], cube2d[3][0], cube2d[3][1], c);
    Vga::draw_line(cube2d[2][0], cube2d[2][1], cube2d[6][0], cube2d[6][1], c);
    Vga::draw_line(cube2d[2][0], cube2d[2][1], cube2d[3][0], cube2d[3][1], c);
    Vga::draw_line(cube2d[4][0], cube2d[4][1], cube2d[6][0], cube2d[6][1], c);
    Vga::draw_line(cube2d[4][0], cube2d[4][1], cube2d[5][0], cube2d[5][1], c);
    Vga::draw_line(cube2d[7][0], cube2d[7][1], cube2d[6][0], cube2d[6][1], c);
    Vga::draw_line(cube2d[7][0], cube2d[7][1], cube2d[3][0], cube2d[3][1], c);
    Vga::draw_line(cube2d[7][0], cube2d[7][1], cube2d[5][0], cube2d[5][1], c);
}
