#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>

#define GLYPH_COUNT 256

typedef struct
{
	short		width;					// number of pixels wide - pitch
	short		height;					// number of scan lines - height
	short		horizAdvance;			// number of pixels to advance to the next char - xSkip
	short		horizOffset;			// x offset into space to render glyph - ignored?
	int			baseline;				// y offset - ignored?
	float		s;						// x start tex coord
	float		t;						// y start tex coord
	float		s2;						// x end tex coord
	float		t2;						// y end tex coord
} glyphInfojk2_t;

typedef struct dfontdat_s
{
	glyphInfojk2_t		mGlyphs[GLYPH_COUNT];

	short			mPointSize;
	short			mHeight;				// max height of font
	short			mAscender;
	short			mDescender;

	short			mKoreanHack;
} dfontdat_t;

#define check_failure(ret, err, msg)		\
	do {					\
		if ((ret) == (err)) {		\
			perror(msg);		\
			exit(EXIT_FAILURE);	\
		}				\
	} while(0)

// Globals

dfontdat_t jk2font;
const char *progName;

// Procedures

void PrintHelpExit()
{
	printf("  Usage: %s [-u] FILE\n", progName);
	printf("Options:\n"
	       "  -u replace lowercase glyph data with uppercase\n");
	exit(EXIT_SUCCESS);
}

void LoadFontData(const char *file)
{
	FILE *fd;
	int size;

	if (file[0] == '\0') {
		PrintHelpExit();
	} else if (file[0] == '-' && file[1] == '\0') {
		fd = stdin;
	} else {
		fd = fopen(file, "rb");
		check_failure(fd, NULL, "Error opening file: ");
		printf("File: %s\n\n", file);
	}

	// Little endian only
	size = fread(&jk2font, 1, sizeof(jk2font), fd);

	if (size < sizeof(jk2font)) {
		printf("Font data too short.\n");
		exit(EXIT_FAILURE);
	} else if (size > sizeof(jk2font)) {
		printf("Font data too long.\n");
		exit(EXIT_FAILURE);
	}
}

void ConvertToUppercase()
{
	printf("Replacing lowercase glyphs...\n");

	for (int i = 'a'; i <= 'z'; i++) {
		jk2font.mGlyphs[i] = jk2font.mGlyphs[i - 'a' + 'A'];
	}
	for (int i = 0xe0; i < 0xf6; i++) {
		jk2font.mGlyphs[i] = jk2font.mGlyphs[i - 0xe0 + 0xc0];
	}
	for (int i = 0xf8; i < 0xfe; i++) {
		jk2font.mGlyphs[i] = jk2font.mGlyphs[i - 0xf8 + 0xd8];
	}

	printf("Done\n");
}

void PrintFontData()
{
	glyphInfojk2_t *glyph;
	short height, ascent, descent, offset, width, advance;

	printf("Global parameters:\n\n");
	printf("mPointSize: %d mHeight: %d mAscender: %d mDescender: %d mKoreanHack: %d\n\n",
	       jk2font.mPointSize, jk2font.mHeight, jk2font.mAscender, jk2font.mDescender, jk2font.mKoreanHack);

	printf("Glyph parameters:\n\n");

	for (int i = 0; i < GLYPH_COUNT; i++) {
		glyph = &jk2font.mGlyphs[i];

		height = glyph->height;
		ascent = glyph->baseline;
		descent = height - glyph->baseline;
		offset = glyph->horizOffset;
		width = glyph->width;
		advance = glyph->horizAdvance;

		printf("0x%02x ", i);

		if (isprint(i))
			printf("'%c'", i);
		else
			printf("   ");

		printf("   Height: %3d Ascent: %3d Descent: %3d         Offset: %3d Width: %3d Advance: %3d\n",
		       height, ascent, descent, offset, width, advance );
	}
}

#define FLAG_UPPERCASE 0x01

int main (int argc, char *argv[])
{
	const char *file;
	int opt;
	int flags = 0;

	progName = argv[0];

	while ((opt = getopt(argc, argv, "u")) != -1) {
		switch (opt) {
		case 'u':
			flags |= FLAG_UPPERCASE;
			break;
		case '?':
			PrintHelpExit();
		}
	}

	if (optind >= argc) {
		PrintHelpExit();
	}

	file = argv[optind];
	LoadFontData(file);

	if (flags & FLAG_UPPERCASE) {
		ConvertToUppercase();
	} else {
		PrintFontData();
	}

	return 0;
}
