#define MAXCOLORTC;

#include <wingraph.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <array>
#include <map>

using namespace std;

struct TPCXHeaderStruct {
	unsigned char ID;
	unsigned char Version;
	unsigned char Coding;
	unsigned char BitPerPixel;
	unsigned short XMin;
	unsigned short YMin;
	unsigned short XMax;
	unsigned short YMax;
	unsigned short HRes;
	unsigned short VRes;
	unsigned char Palette[48];
	unsigned char Reserved;
	unsigned char Planes;
	unsigned short BytePerLine;
	unsigned short PaletteInfo;
	unsigned short HScreenSize;
	unsigned short VScreenSize;
	unsigned char Filler[54];
} TPCXHeader;

struct pixel {
	array <unsigned char, 3> RGB;
	int frequence;
};

unsigned char palette_256[768], palette_16[48];
pixel *pixels, palette[16];
int xSize, ySize, fileSize;
int x = 0, y = 0;
array <unsigned char, 3> currentPixel;
vector <unsigned char> rastr;
vector <unsigned char> new_rastr;
map <array <unsigned char, 3>, int> colorsFrequence;

void get_pixel_color(int color_number, int *R, int *G, int *B) {
	*R = (int) palette_256[color_number * 3];
	*G = (int) palette_256[color_number * 3 + 1];
	*B = (int) palette_256[color_number * 3 + 2];
}

void decode(ifstream &in) {
	unsigned char byte;
	int repeat = 0, R, G, B;
	while (in.tellg() != fileSize - 769) {
		in.read((char*) &byte, 1);
	    
		if ((byte & 0xC0) == 0xC0) {
			repeat = byte & 0x3F;
			in.read((char*) &byte, 1);
		} else 
			repeat = 1;
				
		get_pixel_color((int) byte, &R, &G, &B);
		
		while (repeat > 0) {
			rastr.push_back(R);
			rastr.push_back(G);
			rastr.push_back(B);
			repeat--;
		}
	}
}

bool sort_comparator(const pixel &p_1, const pixel &p_2) {
	return p_1.frequence > p_2.frequence;
}

void calculate_frequence_pixel() {
	int index = 0;
	
	for (int i = 0; i < rastr.size(); i += 3) {
		for (int j = 0; j < 3; j++) 
			currentPixel[j] = rastr[index++];
		colorsFrequence[currentPixel]++;
	}
	
	pixels = new pixel[colorsFrequence.size()];
	index = 0;
	
	for (auto iter = colorsFrequence.begin(); iter != colorsFrequence.end(); iter++) {
		pixels[index].RGB = iter->first;
		pixels[index].frequence = iter->second;
		index++;
	}
	
	sort(pixels, pixels + colorsFrequence.size(), sort_comparator);
}

int delta(array <unsigned char, 3> p_1, array <unsigned char, 3> p_2) {
	return pow((p_1[0] - p_2[0]), 2) + pow((p_1[1] - p_2[1]), 2) + pow((p_1[2] - p_2[2]), 2);
}

void create_palette() {
	int count_color_in_palette = 1, count_diff_colors = 0;
	size_t count_pixels = colorsFrequence.size();
	//236
	palette[0] = pixels[0];
	
	for (int i = 1; i < count_pixels; i++) {
		if (i == count_pixels / 2 || count_color_in_palette == 16) 
			return;
			
		for (int j = 0; j < count_color_in_palette; j++) {
			if (delta(pixels[i].RGB, palette[j].RGB) > 500) {
				count_diff_colors++;
			}
		}
		
		if (count_diff_colors == count_color_in_palette) {
			palette[count_color_in_palette++] = pixels[i];
		}
		
		count_diff_colors = 0;
	}
}

void create_new_rastr() {
	int min_difference = INT_MAX, color = 0, index = 0, index_new_rastr = 0, k = 0;
	
	for (int i = 0; i < 16; i++) {
		palette_16[i * 3] = palette[i].RGB[0];
		palette_16[i * 3 + 1] = palette[i].RGB[1];
		palette_16[i * 3 + 2] = palette[i].RGB[2];
	}
	
	for (int i = 0; i < rastr.size(); i += 3) {
		for (int j = 0; j < 3; j++) 
			currentPixel[j] = rastr[index++];
			
		min_difference = INT_MAX;
		
		for (int j = 0; j < 16; j++) {
			int diff = delta(currentPixel, palette[j].RGB);
			
			if (diff < min_difference) {
				min_difference = diff;
				color = j;
			}
		}
		
		new_rastr.push_back(palette[color].RGB[0]);
		new_rastr.push_back(palette[color].RGB[1]);
		new_rastr.push_back(palette[color].RGB[2]);
	}
}

void show_image() {
	for (int i = 0; i < rastr.size(); i += 3) {
		if (x + ((xSize % 2 == 0) ? 1 : 0) == xSize) {
			x = 0;
			y++;
		} else
			putpixel(x++, y, RGB(rastr[i], rastr[i + 1], rastr[i + 2]));
	}
}

void show_converted_picture() {
	x = 0; 
	y = 0;
	for (int i = 0; i < new_rastr.size(); i += 3) {
		if (x + ((xSize % 2 == 0) ? 1 : 0) == xSize) {
			x = 0;
			y++;
		} else
			putpixel(x++ + (xSize + 30), y, RGB(new_rastr[i], new_rastr[i + 1], new_rastr[i + 2]));
	}
}

int main() {
	unsigned char byte;
	ifstream in("CAT256.PCX", ios::binary);
	
    in.seekg(0, ios_base::end);
	fileSize = in.tellg();
	in.seekg(0, ios_base::beg);
	
	in.seekg(-769, ios_base::end);
    in.read((char*) &byte, 1);
    in.read((char*) palette_256, 768);
    in.seekg(0, ios_base::beg);
    in.read((char*) &TPCXHeader, 128);
	
	xSize = TPCXHeader.XMax - TPCXHeader.XMin + 1;
	ySize = TPCXHeader.YMax - TPCXHeader.YMin + 1;
    resize(xSize * 2 + 40, ySize);
        
	decode(in);		
	calculate_frequence_pixel();
	create_palette();
	create_new_rastr();
	show_image();
	show_converted_picture();
	
	in.close();
}
